
#include "DDMapData.h"
#include <memory>
#include <cmath>
#include "DDConfig.h"
#include <algorithm>
#include "DDUpgradeLoad.h"

USING_NS_CC;

// 专门用来处理计算gameTick()

void DDMapData::gameTick()
{
//    CCLOG("tick step");
    for (auto posmin : DDMapData::s()->getMinMaps()) {
        auto minmap = posmin.second;
        if (minmap->state == DDMinMap::T_ACTIVE && !minmap->blocked)
        {
            tickStepPerMinMap(minmap);
        }
    }
}

void DDMapData::tickStepPerMinMap(const std::shared_ptr<DDMinMap>& minmap)
{
    // 如果直接遍历，那么过程中对posAgentMap无法进行增删，需要引入暂存和事后导入，但这又给isPosEmpty等判断碎片化。导致太琐碎不可靠。
    // 所以这里采用先获取key，然后用keys来去遍历，但要注意某个key可能已经被删除，所以要做好检查。
    std::vector<int> aids;
    aids.reserve(minmap->aidAgentMap.size());
    for (auto aidagent: minmap->aidAgentMap) {
        aids.push_back(aidagent.first);
    }

    for (auto aid : aids) {
        if (minmap->aidAgentMap.count(aid)) {
            auto agent = minmap->aidAgentMap[aid];
            bool battleObserving = isBattleFieldObserving && minmap->pos == battleFieldObservedMinMap;
            switch (agent->type) {
                case DDAgent::AT_3RD_WATER:
                    tickStepPerMinMap_waterPool(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_3RD_VOLCANO:
                    tickStepPerMinMap_volcano(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_FRIEND_MINER:
                    tickStepPerMinMap_miner(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_FRIEND_ARROW_TOWER:
                case DDAgent::AT_FRIEND_MAGIC_TOWER:
                    tickStepPerMinMap_arrowMagicTower(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_FRIEND_CONNON_TOWER:
                    tickStepPerMinMap_cannonTower(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_FRIEND_CURE_TOWER:
                    tickStepPerMinMap_cureTower(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_FRIEND_CORE:
                    tickStepPerMinMap_core(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_ENEMY_NEST:
                    tickStepPerMinMap_enemyNest(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_ENEMY_NEAR:
                    tickStepPerMinMap_enemy(minmap, agent, battleObserving);
                    break;
                case DDAgent::AT_ENEMY_FAR:
                    tickStepPerMinMap_enemy(minmap, agent, battleObserving);
                    break;
                    
                default:
                    break;
            }
        }
    }
}



void DDMapData::help_computePeriodIndex(DDAgent* agent)
{
    // 对于母巢有relax计算
    if (agent->type == DDAgent::AT_ENEMY_NEST && agent->nestActionRelaxIndex > 0) {
        agent->nestActionRelaxIndex--;
    } else {
        agent->actionPeriodIndex--;
        if (agent->actionPeriodIndex < 0) {
            agent->actionPeriodIndex = agent->actionPeriod;
        }
    }
}

/*
void DDMapData::help_computeUnbeatableAmount(DDAgent* activeAgent, bool battleObserving)
{
    if (activeAgent->unbeatableAmount > 0) {
        activeAgent->unbeatableAmount--;
        if (activeAgent->unbeatableAmount <= 0) {
            if (battleObserving) sendBattleFieldMessage(DDBattleObserveMessage::UNBETABLE_LOSS, activeAgent->aid, -1);
        }
    }
}*/

bool DDMapData::help_isFirstStageAction(DDAgent* activeAgent)
{
    // 1的时候机能
    return activeAgent->actionPeriodIndex == 1;
}

bool DDMapData::help_isSecondStageAction(DDAgent* activeAgent)
{
    // 0的时候自疗
    return activeAgent->actionPeriodIndex == 0;
}
/*
bool DDMapData::help_computeSelfCureIndex(DDAgent* activeAgent)
{
    if (activeAgent->selfCureAmount > 0 || activeAgent->selfCureAmount < 0) {
        activeAgent->selfCureAmount--;
        activeAgent->blood = std::min(activeAgent->blood + activeAgent->selfCure, activeAgent->    bloodMax);
        return true;
    }
    return false;
}*/
/*
template <class T>
std::vector<T*> help_findAllInCertainScope(const std::vector<T*>& agents, const AgentPos& mypos, int scope) {
    std::vector<T*> res;
    for (T* agent : agents) {
        if (agent->pos.distance(mypos) <= scope) {
            res.push_back(agent);
        }
    }
    return res;
}

template <class T>
std::vector<T*> help_findAllAtCertainDistance(const std::vector<T*>& agents, const AgentPos& mypos, int distance) {
    std::vector<T*> res;
    for (T* agent : agents) {
        if (agent->pos.distance(mypos) == distance) {
            res.push_back(agent);
        }
    }
    return res;
}
*/
inline std::vector<AgentPos> help_findAllEmptyAtCertainDistance(const std::shared_ptr<DDMinMap>& minmap, const AgentPos& mypos, int distance)
{
    std::vector<AgentPos> ret;
    for (int dx = -distance; dx <= distance; dx++) {
        int dy = distance - std::abs(dx);
        if (dy != 0) {
            AgentPos a = {mypos.x + dx, mypos.y + dy};
            AgentPos b = {mypos.x + dx, mypos.y - dy};
            if (a.isLegal() && minmap->posAgentMap.count(a) == 0)
                ret.push_back(a);
            if (b.isLegal() && minmap->posAgentMap.count(b) == 0)
                ret.push_back(b);
        } else {
            AgentPos a = {mypos.x + dx, mypos.y};
            if (a.isLegal() && minmap->posAgentMap.count(a) == 0)
                ret.push_back(a);
        }
    }
    return ret;
}

inline std::pair<bool, AgentPos> help_findNearestRandomEmptyInScope(const std::shared_ptr<DDMinMap>& minmap, const AgentPos& mypos, int myscope)
{
    bool isfound = false;
    AgentPos res;
    for (int d = 1; d <= myscope; d++) {
        auto founds = help_findAllEmptyAtCertainDistance(minmap, mypos, d);
        if (!founds.empty()) {
            isfound = true;
            res = founds[static_cast<int>(rand_0_1()*100)%founds.size()];
            break;
        }
    }
    return {isfound, res};
}

inline std::vector<AgentPos> help_findAllAtCertainDistance(const std::shared_ptr<DDMinMap>& minmap, const AgentPos& mypos, int distance, std::function<bool(DDAgent*)> filter)
{
    std::vector<AgentPos> ret;
    for (int dx = -distance; dx <= distance; dx++) {
        int dy = distance - std::abs(dx);
        if (dy != 0) {
            AgentPos a = {mypos.x + dx, mypos.y + dy};
            AgentPos b = {mypos.x + dx, mypos.y - dy};
            if (a.isLegal() && minmap->posAgentMap.count(a) > 0 && filter(minmap->posAgentMap[a]))
                ret.push_back(a);
            if (b.isLegal() && minmap->posAgentMap.count(b) > 0 && filter(minmap->posAgentMap[b]))
                ret.push_back(b);
        } else {
            AgentPos a = {mypos.x + dx, mypos.y};
            if (a.isLegal() && minmap->posAgentMap.count(a) > 0 && filter(minmap->posAgentMap[a]))
                ret.push_back(a);
        }
    }
    return ret;
}

inline std::pair<bool, AgentPos> help_findNearestRandomInScope(const std::shared_ptr<DDMinMap>& minmap, const AgentPos& mypos, int myscope, std::function<bool(DDAgent*)> filter)
{
    bool isfound = false;
    AgentPos res;
    for (int d = 1; d <= myscope; d++) {
        auto founds = help_findAllAtCertainDistance(minmap, mypos, d, filter);
        if (!founds.empty()) {
            isfound = true;
            res = founds[static_cast<int>(rand_0_1()*100)%founds.size()];
            break;
        }
    }
    return {isfound, res};
}

inline std::vector<AgentPos> help_findAllInScope(const std::shared_ptr<DDMinMap>& minmap, const AgentPos& mypos, int distance, std::function<bool(DDAgent*)> filter)
{
    std::vector<AgentPos> ret;
    for (int dx = -distance; dx <= distance; dx++) {
        int ysc = distance - std::abs(dx);
        for (int dy = -ysc; dy <= ysc; dy++) {
            if (dx == 0 && dy == 0) {
                continue;
            } else {
                AgentPos a = {mypos.x+dx, mypos.y+dy};
                if (a.isLegal() && minmap->posAgentMap.count(a) > 0 && filter(minmap->posAgentMap[a]))
                    ret.push_back(a);
            }
        }
    }
    return ret;
}

inline std::pair<bool, AgentPos> help_findTreatMapNextMove(const std::shared_ptr<DDMinMap>& minmap,const std::unordered_map<AgentPos, int>& treatMap, const AgentPos& mypos, int currentThreat) // 根据威胁图，找到下一步的移动位置，并且这个位置不能有别的敌人
{
    std::vector<AgentPos> candidates;
    std::vector<AgentPos> founds;
    AgentPos around[4] = {AgentPos{mypos.x+1, mypos.y},AgentPos{mypos.x-1, mypos.y},AgentPos{mypos.x, mypos.y+1},AgentPos{mypos.x, mypos.y-1}};
    for (auto& pos : around) {
        if (pos.isLegal() && minmap->posAgentMap.count(pos) == 0) {
            candidates.push_back(pos);
        }
    }

    for (auto& candidate : candidates) {
        if (treatMap.at(candidate) > currentThreat) {
            founds.push_back(candidate);
        }
    }

    return {!candidates.empty(), (!founds.empty()) ? founds[static_cast<int>(rand_0_1()*100)%founds.size()] :  (!candidates.empty())? candidates[static_cast<int>(rand_0_1()*100)%candidates.size()] : AgentPos{0,0}};
}

/*
template <class T>
void help_deleteAgentFromMinmap(const std::vector<T*>& agents, std::unordered_map<AgentPos, DDAgent*>& agentMap, AgentPos agentpos)
{
    for (auto iter = agents.begin(); iter != agents.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = agents.erase(iter);
            break;
        }
    }
    delete agentMap[agentpos];
    agentMap.erase(agentpos);
}

void help_deleteAgentFromMinmap(std::shared_ptr<DDMinMap> minmap, AgentPos agentpos)
{
    for (auto iter = minmap->agentMines.begin(); iter != minmap->agentMines.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentMines.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentWaterPools.begin(); iter != minmap->agentWaterPools.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentWaterPools.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentMiners.begin(); iter != minmap->agentMiners.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentMiners.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentWalls.begin(); iter != minmap->agentWalls.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentWalls.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentLightHouses.begin(); iter != minmap->agentLightHouses.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentLightHouses.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentArrowTowers.begin(); iter != minmap->agentArrowTowers.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentArrowTowers.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentCannonTowers.begin(); iter != minmap->agentCannonTowers.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentCannonTowers.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentCureTowers.begin(); iter != minmap->agentCureTowers.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentCureTowers.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentFarEnemies.begin(); iter != minmap->agentFarEnemies.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentFarEnemies.erase(iter);
            break;
        }
    }
    for (auto iter = minmap->agentNearEnemies.begin(); iter != minmap->agentNearEnemies.end(); iter++) {
        if ((*iter)->pos == agentpos) {
            iter = minmap->agentNearEnemies.erase(iter);
            break;
        }
    }
    delete minmap->agentMap[agentpos];
    minmap->agentMap.erase(agentpos);
}*/

void DDMapData::tickStepPerMinMap_waterPool(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{
    // 水池会给范围内所有治疗。
    help_computePeriodIndex(agent);
    if (help_isFirstStageAction(agent)) {

        auto founds = help_findAllInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent*a){return a->isEnemy() || a->isFriend();});

        for (auto apos : founds) {
            auto des = minmap->posAgentMap[apos];
            if (des->blood < des->bloodMax) {
                tickStep_help_cure(minmap, agent, des, battleObserving);
            }
        }
    }

}
void DDMapData::tickStepPerMinMap_volcano(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{
    // 火山给范围内所有单位伤害
    help_computePeriodIndex(agent);
    if (help_isFirstStageAction(agent)) {

        auto founds = help_findAllInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent*a){return a->isEnemy() || a->isFriend();});

        for (auto apos : founds) {
            auto des = minmap->posAgentMap[apos];
            tickStep_help_attack(minmap, agent, des, battleObserving);
        }
    }
}

void DDMapData::tickStepPerMinMap_arrowMagicTower(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{
    // arrow/magic 的攻击形式是一致的。只是设定上，arrow的攻击低，攻速快。magic的攻击高，攻速低。

    help_computePeriodIndex(agent);
    if (help_isFirstStageAction(agent)) {

        // 如果有锁定，并且锁定的敌人存在，则攻击。否则寻找敌人锁定，攻击。
        bool willAttack = false;
        if (agent->hasLockedTarget && minmap->aidAgentMap.count(agent->lockedTargetAid)) {
            willAttack = true;
        } else {
            auto found = help_findNearestRandomInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent* a){return  a->isEnemy();});
            if (found.first) {
                agent->hasLockedTarget = true;
                agent->lockedTargetAid = minmap->posAgentMap[found.second]->aid;
                willAttack = true;
            }
        }

        if (willAttack) {
            // 生0.5克2
            auto enemy = minmap->aidAgentMap[agent->lockedTargetAid];
            bool overcoming = elet_isovercoming(agent->elementType, enemy->elementType);
            bool generating = elet_isgenerating(agent->elementType, enemy->elementType);

            int oldBlood = enemy->blood;
            enemy->blood = std::max(0, enemy->blood - static_cast<int>(agent->attack*(overcoming?2:generating?0.5f:1)));
            int bloodNum = oldBlood-enemy->blood;
            if (battleObserving) {
                sendBattleFieldMessage(DDBattleObserveMessage::ATTACK, agent->aid, enemy->aid, bloodNum, overcoming?2:generating?0:1);
            }
            if (enemy->blood == 0) {
                if (battleObserving) {
                    sendBattleFieldMessage(DDBattleObserveMessage::ENEMY_DIE, enemy->aid, -1);
                }
                tickStep_help_agent_gone(minmap, enemy);
            }
        }
    }
}

void DDMapData::tickStepPerMinMap_cannonTower(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{
    // 炮塔，会打击一个主要目标，然后对该目标周围的敌人造成溅射伤害。
    help_computePeriodIndex(agent);
    if (help_isFirstStageAction(agent)) {

        // 如果有锁定，并且锁定的敌人存在，则攻击。否则寻找敌人锁定，攻击。
        bool willAttack = false;
        if (agent->hasLockedTarget && minmap->aidAgentMap.count(agent->lockedTargetAid)) {
            willAttack = true;
        } else {
            auto found = help_findNearestRandomInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent* a){return  a->isEnemy();});
            if (found.first) {
                agent->hasLockedTarget = true;
                agent->lockedTargetAid = minmap->posAgentMap[found.second]->aid;
                willAttack = true;
            }
        }

        if (willAttack) {
            // 攻击主要敌人
            auto enemy = minmap->aidAgentMap[agent->lockedTargetAid];
            bool overcoming = elet_isovercoming(agent->elementType, enemy->elementType);
            bool generating = elet_isovercoming(agent->elementType, enemy->elementType);
            int myattack = static_cast<int>(agent->attack * (overcoming ? 2 : generating ? 0.5f:1));
            int data2 = overcoming ? 2:generating?0:1;
            AgentPos enemyCenterPos = enemy->pos;
            int oldBlood = enemy->blood;
            enemy->blood = std::max(0, enemy->blood - myattack);
            int bloodNum = oldBlood-enemy->blood;
            if (battleObserving) {
                sendBattleFieldMessage(DDBattleObserveMessage::ATTACK, agent->aid, enemy->aid, bloodNum, data2);
            }


            //溅射伤害周围敌人
            auto around = help_findAllInScope(minmap, enemyCenterPos, agent->cannonSputtDistance, [](DDAgent* a){return  a->isEnemy();});
            int spattack = agent->cannonSputtDamageRadio * 0.1f * myattack;
            for (auto epos : around) {
                auto spenemy = minmap->posAgentMap[epos];
                int eoldBlood = spenemy->blood;
                spenemy->blood = std::max(0, spenemy->blood - spattack);
                int ebloodNum = eoldBlood - spenemy->blood;
                if (battleObserving) {
                    sendBattleFieldMessage(DDBattleObserveMessage::ATTACK_SPUTT, enemy->aid, spenemy->aid, ebloodNum, data2);
                }
                if (spenemy->blood == 0) {
                    if (battleObserving) {
                        sendBattleFieldMessage(DDBattleObserveMessage::ENEMY_DIE, spenemy->aid, -1);
                    }
                    tickStep_help_agent_gone(minmap, spenemy);
                }

            }


            if (enemy->blood == 0) {
                if (battleObserving) {
                    sendBattleFieldMessage(DDBattleObserveMessage::ENEMY_DIE, enemy->aid, -1);
                }
                tickStep_help_agent_gone(minmap, enemy);
            }
        }
    }

}

void DDMapData::tickStepPerMinMap_core(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{
    // core 像火山一样，能够对范围内所有敌人进行同时攻击！
    help_computePeriodIndex(agent);
    if (help_isFirstStageAction(agent)) {

        auto founds = help_findAllInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent*a){return a->isEnemy();});

        for (auto apos : founds) {
            auto des = minmap->posAgentMap[apos];
            tickStep_help_attack(minmap, agent, des, battleObserving);
        }
    }
}

void DDMapData::tickStepPerMinMap_cureTower(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{
    // 水池会给范围内（只有1）所有治疗。
    help_computePeriodIndex(agent);
    if (help_isFirstStageAction(agent)) {

        auto founds = help_findAllInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent*a){return a->isFriend() && a->blood < a->bloodMax;});

        std::vector<DDAgent*> foundAgents;
        for (auto& ap : founds) {
            foundAgents.push_back(minmap->posAgentMap[ap]);
        }
        if (foundAgents.size() <= agent->cureNum) {
        } else {
            std::sort(foundAgents.begin(), foundAgents.end(), [](DDAgent* a, DDAgent*b)->bool{return a->blood < b->blood;});
        }
        for (int i = 0; i < std::min(static_cast<int>(foundAgents.size()), agent->cureNum); i++) {
            tickStep_help_cure(minmap, agent, foundAgents[i], battleObserving);
        }
    }
}


void DDMapData::tickStepPerMinMap_enemyNest(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{
    // 母巢会在周边位置生成敌人，敌人的初始位置在母巢的周边2及内。
    help_computePeriodIndex(agent);
//    help_computeUnbeatableAmount(agent, battleObserving);
    if (help_isFirstStageAction(agent)) {
        // 机能
        auto ifpos = help_findNearestRandomEmptyInScope(minmap, agent->pos, 2);
        if (ifpos.first) {
            AgentPos agentpos = ifpos.second;
            CCLOG("nest create enemy at %d %d", agentpos.x, agentpos.y);

            // 制作敌人
            this->appearAgent_enemy(minmap, agentpos, agent, battleObserving);

        } else {
//            CCLOG("nest, no empty place for new enemy");
        }
        // 可能会进入睡眠
        if (rand_0_1() < agent->nestChanceToRelax) {
            CCLOG("nest in relax");
            agent->nestActionRelaxIndex = agent->nestActionRelaxPeriod;
        }
    }
    /*else if (help_isSecondStageAction(agent)) {
        // 自我治疗
        if (help_computeSelfCureIndex(miner)) {
            if (battleObserving) _battleFieldObserver(DDBattleObserveMessage::SELF_CURE, miner->aid, -1);
        }
    }
    */

}

void DDMapData::tickStep_help_agent_gone(const std::shared_ptr<DDMinMap>& minmap, DDAgent* des) // 将某个agent正确移出，如果agent会影响threatmap则会重新计算，在数据结构上移出，包括释放内存。
{

    // gas在此计数
    if (des->isEnemy()) {
        injectResourceGas(1, true);
    }

    minmap->posAgentMap.erase(des->pos);
    minmap->aidAgentMap.erase(des->aid);
    if (!des->isEnemy()) {
        help_calcMinMapThreatMapAll(minmap->pos);
    }
    delete des;
}


void DDMapData::tickStep_help_attack(const std::shared_ptr<DDMinMap>& minmap, DDAgent* src, DDAgent* des, bool battleObserving)// 通用的攻击计算过程，会自动锁定，对方死了会自动解锁
{
    // 克 *2 生*0.5
    bool overcoming = elet_isovercoming(src->elementType, des->elementType);
    bool generating = elet_isgenerating(src->elementType, des->elementType);
    src->hasLockedTarget = true;
    src->lockedTargetAid = des->aid;

    int oldBlood = des->blood;
    des->blood -= static_cast<int>(src->attack * (overcoming ? 2: generating? 0.5f:1));
    if (des->blood < 0) {
        des->blood = 0;
    }
    int bloodCost = oldBlood - des->blood;
    if (battleObserving)
        sendBattleFieldMessage(DDBattleObserveMessage::ATTACK, src->aid, des->aid, bloodCost, overcoming?2:generating?0:1);

    if (des->type == DDAgent::AT_FRIEND_CORE) {
        _bigmapFieldProtocal->op_showMessage(BigmapMessageType::CORE_ATTACK, "msg_core_attack", false, true, {0,0});
    }

    // 处理战死
    if (des->blood == 0) {
        if (battleObserving)
            sendBattleFieldMessage(des->isFriend() ? DDBattleObserveMessage::FRIEND_DIE : DDBattleObserveMessage::ENEMY_DIE, des->aid, -1);
        if (des->type == DDAgent::AT_FRIEND_CORE) {
            _battleFieldProtocal->op_gameOverProcess();
            _bigmapFieldProtocal->op_showMessage(BigmapMessageType::CORE_DIE, "msg_core_die", false, true, {0,0});
        } else if (des->isFriend()) {
            _bigmapFieldProtocal->op_showMessage(BigmapMessageType::MOBAD, "msg_friend_die", false, true, minmap->pos);
        }

        tickStep_help_agent_gone(minmap, des);
        src->hasLockedTarget = false;
    }

}

void DDMapData::tickStep_help_enemy_move(const std::shared_ptr<DDMinMap>& minmap, DDAgent* enemy, const AgentPos& moveto, bool battleObserving) // 怪物的通用移动
{

    minmap->posAgentMap.erase(enemy->pos);
    enemy->pos = moveto;
    minmap->posAgentMap[moveto] = enemy;
    if (battleObserving) {
        sendBattleFieldMessage(DDBattleObserveMessage::ENEMY_MOVE, enemy->aid, -1);
    }

}

void DDMapData::tickStep_help_cure(const std::shared_ptr<DDMinMap>& minmap, DDAgent* src, DDAgent* des, bool battleObserving) // 通用治疗
{
    // 生*2 克*0.5
    int oldBlood = des->blood;
    bool generating = elet_isgenerating(src->elementType, des->elementType);
    bool overcoming = elet_isovercoming(src->elementType, des->elementType);
    des->blood = std::min(des->bloodMax, des->blood+ static_cast<int>(src->cure*(generating?2: overcoming?0.5f:1)));
    int bloodNum = des->blood - oldBlood;
    if (battleObserving) {
        sendBattleFieldMessage(DDBattleObserveMessage::CURE, src->aid, des->aid, bloodNum, generating?2:overcoming?0:1);
    }
}


void DDMapData::tickStepPerMinMap_enemy(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{
    help_computePeriodIndex(agent);
//    help_computeUnbeatableAmount(agent, battleObserving);
    if (help_isFirstStageAction(agent)) {
        // 机能
        int threat = minmap->generalTreatMap[agent->pos];
        // 由于移出口肯定会有FULL_MAP的威胁，所以threat为0肯定是这个位置被四周被堵死了。
        // 堵死分为两种情况，1-天然堵死 2-由于人造的墙堵死
        bool willAttackWall = threat == 0; //当堵死时，会攻击墙！

        //TODO 在传送点，并且willAttackWall == true，则移出到上游！
        if (threat == THREAT_DOOR) {
            // 由于threat的梯度，所以threat为此值的，位置一定在门口。

            MapPos desMinMapPos = {0,0};
            AgentPos desAgentPos = {0,0};
            bool willTryMoveOut = false;
            if (agent->pos.x == -DDConfig::MINMAP_EXPAND) {
                // 左
                willTryMoveOut = true;
                desMinMapPos = {minmap->pos.x -1, minmap->pos.y};
                desAgentPos = {-agent->pos.x, agent->pos.y};
            } else if (agent->pos.x == DDConfig::MINMAP_EXPAND) {
                // 右
                willTryMoveOut = true;
                desMinMapPos = {minmap->pos.x +1, minmap->pos.y};
                desAgentPos = {-agent->pos.x, agent->pos.y};
            } else if (agent->pos.y == DDConfig::MINMAP_EXPAND) {
                // 上
                willTryMoveOut = true;
                desMinMapPos = {minmap->pos.x, minmap->pos.y+1};
                desAgentPos = {agent->pos.x, -agent->pos.y};
            } else if (agent->pos.y == -DDConfig::MINMAP_EXPAND) {
                // 下
                willTryMoveOut = true;
                desMinMapPos = {minmap->pos.x, minmap->pos.y-1};
                desAgentPos = {agent->pos.x, -agent->pos.y};
            }
            if (willTryMoveOut) {
                // 检查对方minmap此位置是否空。
                if (_presentingMinmaps[desMinMapPos]->posAgentMap.count(desAgentPos) == 0) {
                    CCLOG("enemy move out from %d %d to %d %d", minmap->pos.x, minmap->pos.y, desMinMapPos.x, desMinMapPos.y);
                    if (battleObserving) {
                        sendBattleFieldMessage(DDBattleObserveMessage::ENEMY_OUT, agent->aid, -1);
                    }
                    // 移动
                    minmap->posAgentMap.erase(agent->pos);
                    minmap->aidAgentMap.erase(agent->aid);

                    auto theMinmap = _presentingMinmaps[desMinMapPos];
                    agent->pos = desAgentPos;
                    theMinmap->posAgentMap[agent->pos] = agent;
                    theMinmap->aidAgentMap[agent->aid] = agent;

                    // 通知对方
                    if (isBattleFieldObserving && desMinMapPos == battleFieldObservedMinMap) {
                        sendBattleFieldMessage(DDBattleObserveMessage::ENEMY_IN, agent->aid, -1);
                    }
                }
                return;//即使因为对方没空而没有移出，也在此位置等待，而走下面流程移动。
            }

        }

        // 先尝试进攻
        if (agent->hasLockedTarget && minmap->aidAgentMap.count(agent->lockedTargetAid) && (willAttackWall == (minmap->aidAgentMap[agent->lockedTargetAid]->type == DDAgent::AT_FRIEND_WALL))) {
            // 如果已有锁定攻击目标(如果在不是需要对墙攻击的情况下，攻击的是墙，那么需要重新选择攻击目标)，并且锁定目标依然存在，那么直接攻击！(这些描述指上面的复杂判断)
            // TODO 进攻 已锁定的！！
            tickStep_help_attack(minmap, agent, minmap->aidAgentMap[agent->lockedTargetAid], battleObserving);

        } else {
            if (willAttackWall) {

                // 找一个墙来进攻，找到则锁定，并进攻！找不到则按墙的威胁图走一步。
                //CCLOG("wall attack or move %d %d", threat, minmap->generalTreatMap.size());

                // 一般情况下这个找的过程是需要遍历的，这可能造成性能问题？
                // 可以使用建筑的威胁图（将TREAT设为相同，而来使得威胁图可以算出怪到最近的建筑的距离，来判断是否需要搜索）加速，但建筑在找怪的操作则由于怪物一直会动而无法通过类似威胁图来加速，因为怪一直在动。
                // 暂时假设不会造成性能问题。进行搜索
                auto res = help_findNearestRandomInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent* a){return a->type == DDAgent::AT_FRIEND_WALL;});
                if (res.first) {
                    tickStep_help_attack(minmap, agent, minmap->posAgentMap[res.second], battleObserving);
                } else {
                    int wallTreatNow = minmap->wallTreatMap[agent->pos];
                    if (wallTreatNow == 0) {
                        // 没可见的墙！
                        // 没办法了，这个怪在死胡同里面
                        // TODO 随便移动一步！
                    } else {
                        auto gotoPos = help_findTreatMapNextMove(minmap, minmap->wallTreatMap, agent->pos, wallTreatNow);
                        if (gotoPos.first) {
                            tickStep_help_enemy_move(minmap, agent, gotoPos.second, battleObserving);
                        }
                    }
                }

            } else {
                // 找一个非墙目标来进攻，找到则锁定，并进攻！找不到则按非墙的威胁图走一步。
                //CCLOG("un wall attack or move");
                auto res = help_findNearestRandomInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent* a){ return  a->isFriend() && a->type != DDAgent::AT_FRIEND_WALL;});
                if (res.first) {
                    tickStep_help_attack(minmap, agent, minmap->posAgentMap[res.second], battleObserving);
                } else {
                    if (threat == 0) {
                        // 没有能触及的可攻击目标。
                    } else {
                        auto gotoPos = help_findTreatMapNextMove(minmap, minmap->generalTreatMap, agent->pos, threat);
                        if (gotoPos.first) {
                            tickStep_help_enemy_move(minmap, agent, gotoPos.second, battleObserving);
                        }
                    }
                }
            }
        }

    }
}


void DDMapData::tickStepPerMinMap_miner(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving)
{

    help_computePeriodIndex(agent);
    if (help_isFirstStageAction(agent)) {

        // 如果有锁定，并且锁定的矿还存在，那么就去挖，否则找其它矿挖，并锁定，如果没有矿可以挖了，那么就idle了。
        if (agent->mineDone) {
            return;
        }
        bool willMine = false;

        if (agent->hasLockedTarget && minmap->aidAgentMap.count(agent->lockedTargetAid)) {
            willMine = true;
        } else {
            auto found = help_findNearestRandomInScope(minmap, agent->pos, agent->actionDistance, [](DDAgent* a){return a->type == DDAgent::AT_3RD_MINE;});
            if (!found.first) {
                agent->mineDone = true;
                if (battleObserving) {
                    sendBattleFieldMessage(DDBattleObserveMessage::MINE_IDLE, agent->aid, -1);
                }
            } else {
                agent->hasLockedTarget = true;
                agent->lockedTargetAid = minmap->posAgentMap[ found.second]->aid;
                willMine = true;
            }
        }

        if (willMine) {
            // 矿 矿属土，木克土，*2
            bool overcoming = elet_isovercoming(agent->elementType, DDElementType::Earth);
            auto mine = minmap->aidAgentMap[agent->lockedTargetAid];
            auto oldAmount = mine->mineAmount;
            mine->mineAmount = std::max(0, mine->mineAmount-agent->mineSpeed*(overcoming?2:1));
            int mineNum = oldAmount - mine->mineAmount;
            injectResourceMine(mineNum, true);

            if (battleObserving) {
                sendBattleFieldMessage(DDBattleObserveMessage::MINE, agent->aid, mine->aid, mineNum, overcoming ?2:1);
            }

            if (mine->mineAmount == 0) {
                agent->hasLockedTarget = false;
                if (battleObserving) {
                    sendBattleFieldMessage(DDBattleObserveMessage::MINE_GONE, mine->aid, -1);
                }
                tickStep_help_agent_gone(minmap, mine);
                _bigmapFieldProtocal->op_showMessage(BigmapMessageType::INFO, "msg_mine_gone", false, true, minmap->pos);
            }
        }
    }
}

void DDMapData::appearAgent_enemy(const std::shared_ptr<DDMinMap>& minmap, const AgentPos& agentpos, DDAgent* nest, bool battleObserving)
{
    // 新产生敌人。
    auto enemy = new DDAgent();
    enemy->aid = nextAgentId();
    enemy->pos = agentpos;

    bool isNear = rand_0_1() < nest->nestChanceToNear;
    bool isBoss = rand_0_1() < nest->nestChanceToBoss;

    enemy->type = isNear ? DDAgent::AT_ENEMY_NEAR : DDAgent::AT_ENEMY_FAR;
    enemy->level = 0;//TODO
    enemy->bloodMax = enemy->blood = nest->nestBlood * (isBoss ? 2:1);
    enemy->attack = nest->nestAttack;
    enemy->actionDistance = isNear ? nest->nestAttackDistanceNear : nest->nestAttackDistanceFar;
    enemy->actionPeriod = enemy->actionPeriodIndex = nest->nestAttackPeriod;
    enemy->elementType = nest->nestElementType;

    minmap->posAgentMap[agentpos] = enemy;
    minmap->aidAgentMap[enemy->aid] = enemy;

    // 通知
    if (battleObserving) {
        sendBattleFieldMessage(DDBattleObserveMessage::ENEMY_NEW, nest->aid, enemy->aid);
    }
}

// 新产生友军建筑，总是在battleObserved的minmap，总是会observing。
void DDMapData::appearAgent_friend(const AgentPos& agentpos, int friendAgentType)
{
    CCLOG("add friend type = %d", friendAgentType);
    auto agent = DDUpgradeRoadMap::generateInitFriendAgents(friendAgentType);
    int cost = DDUpgradeRoadMap::fetchFriendCost(friendAgentType);
    injectResourceMine(-cost, false);
    agent->aid = nextAgentId();
    agent->pos = agentpos;
    getBattleObservedMinMap()->posAgentMap[agentpos] = agent;
    getBattleObservedMinMap()->aidAgentMap[agent->aid] = agent;

    help_calcMinMapThreatMapAll(battleFieldObservedMinMap);
//    help_calcMinMapThreatMapSingle(battleFieldObservedMinMap, agentpos);

    sendBattleFieldMessage(DDBattleObserveMessage::FRIEND_NEW, agent->aid, -1);
}

// 卖出友军建筑，或者，移出3RD的石头和树，总是在battleObserved的minmap，总是会observing。
void DDMapData::removeAgent(const AgentPos& agentpos)
{

}

void DDMapData::tellCurrentAllVisionOpen() // 告知当前的minmap的所有视野开了，需要激活周边。还要通知bigmap。
{
    help_makeArroundActivable(battleFieldObservedMinMap);
}