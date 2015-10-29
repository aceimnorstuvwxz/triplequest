
#include "DDMapData.h"
#include <memory>
#include <cmath>
#include "DDConfig.h"

USING_NS_CC;

int DDMapData::_agentIdIndex;
DDMapData DDMapData::_instance;

void DDMapData::store()
{
    CCLOG("mapdata store");
    UserDefault::getInstance()->setBoolForKey("has_saved_game", _hasSavedGame);
    UserDefault::getInstance()->setBoolForKey("has_finish_tutorial", _hasFinishTutorial);
    UserDefault::getInstance()->setIntegerForKey("template_index", _templateIndex);
    UserDefault::getInstance()->setIntegerForKey("agent_id_index", _agentIdIndex);
}

void DDMapData::recover()
{
    CCLOG("mapdata recover");
    _hasSavedGame = UserDefault::getInstance()->getBoolForKey("has_saved_game");//默认false
    _hasFinishTutorial = UserDefault::getInstance()->getBoolForKey("has_finish_tutorial");//默认false
    _templateIndex = UserDefault::getInstance()->getIntegerForKey("template_index", 0);
    _agentIdIndex = UserDefault::getInstance()->getIntegerForKey("agent_id_index", 10000);

    // 地图数据的恢复并不在这里。

}

//void DDMapData::registerBattleFieldObserver(std::function<void(DDBattleObserveMessage message, int aid_src, int aid_des)> battleFieldObserver)
//{
//    _battleFieldObserver = battleFieldObserver;
//}
//
//void DDMapData::registerBigmapFieldObserver(std::function<void(DDBigmapObserveMessage message, MapPos mappos)> bigmapFieldObserver)
//{
//    _bigmapFieldObserver = bigmapFieldObserver;
//}

void DDMapData::configProtocals(DDBattleFieldProtocal* battleProtocal, DDBigmapFieldProtocal* bigmapProtocal)
{
    _battleFieldProtocal = battleProtocal;
    _bigmapFieldProtocal = bigmapProtocal;
}

bool DDMapData::isMinMapActive(const MapPos& mappos)
{
    return _presentingMinmaps.count(mappos) > 0 &&
        _presentingMinmaps[mappos]->state == DDMinMap::T_ACTIVE;
}


//不同的威胁程度，导致怪物会先攻击那些建筑，而后再去门继而走出。

inline int fetchThreatMaxOfAgentType(int agentType)
{
    int res = 0;
    switch (agentType) {
        case DDAgent::AT_FRIEND_CORE:
            res = DDMapData::THREAT_FULL_MAP;
            break;

        case DDAgent::AT_FRIEND_CURE_TOWER:
            res = DDMapData::THREAT_FULL_MAP;
            break;

        case DDAgent::AT_FRIEND_LIGHT_TOWER:
            res = DDMapData::THREAT_FULL_MAP;
            break;

        case DDAgent::AT_FRIEND_ARROW_TOWER:
        case DDAgent::AT_FRIEND_CONNON_TOWER:
        case DDAgent::AT_FRIEND_MAGIC_TOWER:
            res = DDMapData::THREAT_FULL_MAP;
            break;
        case DDAgent::AT_FRIEND_MINER:
            res = DDMapData::THREAT_FULL_MAP;
            break;
        case DDAgent::AT_FRIEND_WALL:
            res = -1;

        default:
            break;
    }
    return res;
}

void DDMapData::help_calcMinMapThreatMapAll(const MapPos& mappos) //为minmap计算TreatMap，当初始化、建筑灭失时需要调用此方法进行完全计算。
{
    auto minmap = _presentingMinmaps[mappos];
    help_clearMinMapThreatMap(&(minmap->generalTreatMap));
    help_clearMinMapThreatMap(&(minmap->wallTreatMap));

    // 门 // 3格门
    MapPos poss[4] = {{mappos.x, mappos.y+1}, {mappos.x, mappos.y-1}, {mappos.x-1, mappos.y}, {mappos.x+1, mappos.y}};
    // 上
    {
        int dex = 0;
        if (_presentingMinmaps.count(poss[dex]) && _presentingMinmaps[poss[dex]]->state == DDMinMap::T_ACTIVE ) {
            int theweight = _presentingMinmaps[poss[dex]]->weight;
            if (theweight < minmap->weight) {

                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {0, DDConfig::MINMAP_EXPAND}, THREAT_DOOR);
                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {-1, DDConfig::MINMAP_EXPAND}, THREAT_DOOR);
                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {1, DDConfig::MINMAP_EXPAND}, THREAT_DOOR);
            }
        }
    }
    // 下
    {
        int dex = 1;
        if (_presentingMinmaps.count(poss[dex]) && _presentingMinmaps[poss[dex]]->state == DDMinMap::T_ACTIVE ) {
            int theweight = _presentingMinmaps[poss[dex]]->weight;
            if (theweight < minmap->weight) {

                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {0, -DDConfig::MINMAP_EXPAND}, THREAT_DOOR);
                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {-1, -DDConfig::MINMAP_EXPAND}, THREAT_DOOR);
                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {1, -DDConfig::MINMAP_EXPAND}, THREAT_DOOR);
            }
        }
    }
    // 左
    {
        int dex = 2;
        if (_presentingMinmaps.count(poss[dex]) && _presentingMinmaps[poss[dex]]->state == DDMinMap::T_ACTIVE ) {
            int theweight = _presentingMinmaps[poss[dex]]->weight;
            if (theweight < minmap->weight) {

                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {-DDConfig::MINMAP_EXPAND, 0}, THREAT_DOOR);
                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {-DDConfig::MINMAP_EXPAND, 1}, THREAT_DOOR);
                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {-DDConfig::MINMAP_EXPAND, -1}, THREAT_DOOR);
            }
        }
    }
    // 右
    {
        int dex = 3;
        if (_presentingMinmaps.count(poss[dex]) && _presentingMinmaps[poss[dex]]->state == DDMinMap::T_ACTIVE ) {
            int theweight = _presentingMinmaps[poss[dex]]->weight;
            if (theweight < minmap->weight) {

                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {DDConfig::MINMAP_EXPAND, 0}, THREAT_DOOR);
                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {DDConfig::MINMAP_EXPAND, 1}, THREAT_DOOR);
                help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), {DDConfig::MINMAP_EXPAND, -1}, THREAT_DOOR);
            }
        }
    }

    // 内部agents
    for (auto posagent: minmap->posAgentMap) {
        int threat = fetchThreatMaxOfAgentType(posagent.second->type);
        if (threat > 0) {
            help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), posagent.first, threat);
        } else if (threat == -1) {
            help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->wallTreatMap), posagent.first, THREAT_FULL_MAP);
        }
    }
//    help_printMinMapThreatMap(&(minmap->generalTreatMap));
//    help_printMinMapThreatMap(&(minmap->wallTreatMap));
}

void DDMapData::help_calcMinMapThreatMapSingle(const MapPos& mappos, const AgentPos& agentpos) //当新增建筑时，要为threat map补充这个新建筑的量。
{
    auto minmap = _presentingMinmaps[mappos];
    int threat = fetchThreatMaxOfAgentType(minmap->posAgentMap[agentpos]->type);
    if (threat > 0) {
        help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->generalTreatMap), agentpos, threat);
        help_printMinMapThreatMap(&(minmap->generalTreatMap));
    } else if (threat == -1) {
        help_calcMinMapThreatMapSingle_start(&(minmap->posAgentMap), &(minmap->wallTreatMap), agentpos, THREAT_FULL_MAP);
        help_printMinMapThreatMap(&(minmap->wallTreatMap));
    }
}

inline bool checkIfAgentCutThreat(DDAgent* agent)
{
    return !agent->isEnemy();
}


void DDMapData::help_calcMinMapThreatMapSingle_start(std::unordered_map<AgentPos, DDAgent*>* posAgents, std::unordered_map<AgentPos, int>* threatMap, const AgentPos& pos, int threat)
{
    help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, pos, threat);
    if (threat > 1) {
        int nextThreat = threat -1;
        help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, {pos.x-1, pos.y}, nextThreat);
        help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, {pos.x+1, pos.y}, nextThreat);
        help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, {pos.x, pos.y-1}, nextThreat);
        help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, {pos.x, pos.y+1}, nextThreat);
    }
}


void DDMapData::help_calcMinMapThreatMapSingle_ast(std::unordered_map<AgentPos, DDAgent*>* posAgents, std::unordered_map<AgentPos, int>* threatMap, const AgentPos& pos, int threat)
{
    if (posAgents->count(pos) > 0 && checkIfAgentCutThreat((*posAgents)[pos])) {
        (*threatMap)[pos] = 0;
        return;
    }
    if (std::abs(pos.x) <= DDConfig::MINMAP_EXPAND && std::abs(pos.y) <= DDConfig::MINMAP_EXPAND) {
        if ((*threatMap)[pos] < threat) {
            (*threatMap)[pos] = threat;
            if (threat > 1) {
                int nextThreat = threat -1;
                help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, {pos.x-1, pos.y}, nextThreat);
                help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, {pos.x+1, pos.y}, nextThreat);
                help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, {pos.x, pos.y-1}, nextThreat);
                help_calcMinMapThreatMapSingle_ast(posAgents, threatMap, {pos.x, pos.y+1}, nextThreat);
            }
        }
    }
}

void DDMapData::help_clearMinMapThreatMap(std::unordered_map<AgentPos, int>* threatMap) //初始化 或 清0 威胁图
{
    for (int x = -DDConfig::MINMAP_EXPAND; x <= DDConfig::MINMAP_EXPAND; x++) {
        for (int y = -DDConfig::MINMAP_EXPAND; y <= DDConfig::MINMAP_EXPAND; y++) {
            (*threatMap)[{x,y}] = 0;
        }
    }
}

void DDMapData::help_printMinMapThreatMap(std::unordered_map<AgentPos, int>* threatMap) //初始化 或 清0 威胁图
{
    for (int y = -DDConfig::MINMAP_EXPAND; y <= DDConfig::MINMAP_EXPAND; y++) {
        std::stringstream ss;
        for (int x = -DDConfig::MINMAP_EXPAND; x <= DDConfig::MINMAP_EXPAND; x++) {
            ss << (*threatMap)[{x,y}] << " ";
        }
        CCLOG("%s", ss.str().c_str());
    }
}

void DDMapData::help_calcMinMapWeight(const MapPos& mappos) //迭代的计算更新minmap的weight
{
    if (_presentingMinmaps.count(mappos)) {
        auto minmap = _presentingMinmaps[mappos];
        if (minmap->state == DDMinMap::T_ACTIVE) {
            int oldWeight = minmap->weight;
            MapPos poss[4] = {{mappos.x, mappos.y+1}, {mappos.x, mappos.y-1}, {mappos.x-1, mappos.y}, {mappos.x+1, mappos.y}};

            if (minmap->isCore()) {
                minmap->weight = 0;
            } else {
                // 从周围找最低的+1
                int minWeight = 100000;
                for (MapPos& thepos : poss) {
                    if (_presentingMinmaps.count(thepos) && _presentingMinmaps[thepos]->state == DDMinMap::T_ACTIVE) {
                        if (minWeight > _presentingMinmaps[thepos]->weight) {
                            minWeight = _presentingMinmaps[thepos]->weight;
                        }
                    }
                }
                // 至少会有一个的
                minmap->weight = minWeight + 1;
            }
            if (oldWeight != minmap->weight) {
                CCLOG("minmap weight change %d %d from %d to %d", mappos.x, mappos.y, oldWeight, minmap->weight);
                // 如果改变，则还要更新周边。
                for (MapPos& thepos : poss) {
                    help_calcMinMapWeight(thepos);
                }
            }
        }
    }
}

// 在重新进入地下城之前清空数据。
void DDMapData::clearAllDataForNewGame()
{
    gamingRunning = false;//必须停止游戏
    _presentingMinmaps.clear();
    _templateMinmaps.clear();
    _cntEnemyKilled = 0;
    _cntMineGot = 0;
    _cntOccupied = 0;
}


void DDMapData::injectResourceMine(int mineDiff, bool ifcnt)
{
    _cntMineLeft += mineDiff;
    if (_cntMineLeft < 0) {
        _cntMineLeft = 0;
    }

    if (ifcnt) {
        _cntMineGot += mineDiff;
    }
}
void DDMapData::injectResourceGas(int gasDiff, bool ifcnt)
{
    _cntGasLeft += gasDiff;
    if (_cntGasLeft < 0) {
        _cntGasLeft = 0;
    }

    if (ifcnt) {
        _cntGasGot += gasDiff;
    }
}

void DDMapData::agentUpgrade(int aid, PropertyType propType)// 升级，总是当前的minmap内的，会自动扣资源，然后通知各方
{
    assert(getBattleObservedMinMap()->aidAgentMap.count(aid) >0);

    auto agent = getBattleObservedMinMap()->aidAgentMap[aid];
    auto upgradeConfig = DDUpgradeRoadMap::fetchUpgradeConfigs(agent->type, propType, help_fetchAgentPropertyValue(agent, propType, -1).first);

    assert(upgradeConfig.upgradable);
    assert(_cntGasLeft >= upgradeConfig.cost);

    injectResourceGas(-upgradeConfig.cost, false);
    help_upgradeAgentProperty(agent, propType, upgradeConfig.valueAfter);

    // 告知其它模块
    if (isBattleFieldObserving) {
        sendBattleFieldMessage(DDBattleObserveMessage::FRIEND_UPGRADE, agent->aid, -1);
    }
}

void DDMapData::agentRemove(int aid, PropertyType propType) // 移除某个agent，分为挖掉石头/树，和，变卖建筑2种。
{
    assert(getBattleObservedMinMap()->aidAgentMap.count(aid));

    auto minmap = getBattleObservedMinMap();
    auto agent = getBattleObservedMinMap()->aidAgentMap[aid];
    if (propType == PropertyType::REMOVE_3RD) {
        // 挖走第三方
        int cost = DDUpgradeRoadMap::fetchRemove3RdCost(agent);
        assert(_cntGasLeft >= cost);
        injectResourceGas(-cost, false);
        if (isBattleFieldObserving) {
            sendBattleFieldMessage(DDBattleObserveMessage::REMOVE_3RD, aid, -1);
        }
        tickStep_help_agent_gone(minmap, agent);
    } else if (propType == PropertyType::SELL_FRIEND) {
        // 变卖建筑
        int cost  = DDUpgradeRoadMap::fetchSellFriendGot(agent);
        injectResourceGas(cost, false);
        if (isBattleFieldObserving) {
            sendBattleFieldMessage(DDBattleObserveMessage::FRIEND_SELL, aid, -1);
        }
        if (agent->type == DDAgent::AT_FRIEND_CORE) {
            // 如果强制卖掉 核心！那么会go
            _battleFieldProtocal->op_gameOverProcess();
        }
        tickStep_help_agent_gone(minmap, agent);
    }

}

void DDMapData::agentSetElementType(int aid, DDElementType newElementType) // 设置，并扣资源，通知各方。
{
    auto minmap = getBattleObservedMinMap();
    auto agent = getBattleObservedMinMap()->aidAgentMap[aid];
    agent->elementType = newElementType;
    injectResourceGas(-DDUpgradeRoadMap::fetchElementCost(), false);
    if (isBattleFieldObserving)
        sendBattleFieldMessage(DDBattleObserveMessage::FRIEND_ELET, aid, -1);
}
