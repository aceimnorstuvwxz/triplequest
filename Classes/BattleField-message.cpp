// (C) 2015 Turnro.com

#include "BattleField.h"
#include "format.h"
#include "DDMapData.h"
#include "TRLocale.h"

USING_NS_CC;

// 处理来自其它地方的消息!!

void BattleField::op_mapdata_message(DDBattleObserveMessage message, int aid_src, int aid_des, int data, int data2) // mapdata 来通知battle field各种消息！
{
    CCLOG("mapdata message %s %d %d %d %d", battleObserveMessage2stringKey(message).c_str(), aid_src, aid_des, data, data2);

    // 消息分发到处理过程
    switch (message) {
        case DDBattleObserveMessage::MINE:
            msg_mine(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::MINE_IDLE:
//            msg_mine_idle(aid_src, aid_des, data, data2);
            break;

        case DDBattleObserveMessage::MINE_GONE:
            msg_mine_gone(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::CURE:
            msg_cure(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::ATTACK:
            msg_attack(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::ATTACK_SPUTT:
            msg_attack_sputt(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::ENEMY_IN:
            msg_enemy_in(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::ENEMY_OUT:
            msg_enemy_out(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::ENEMY_NEW:
            msg_enemy_new(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::ENEMY_DIE:
            msg_enemy_die(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::ENEMY_MOVE:
            msg_enemy_move(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::FRIEND_UPGRADE:
            msg_friend_upgrade(aid_src, aid_des, data, data2);
            break;

        case DDBattleObserveMessage::FRIEND_NEW:
            msg_friend_new(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::FRIEND_DIE:
            msg_friend_die(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::FRIEND_SELL:
            msg_friend_sell(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::REMOVE_3RD:
            msg_remove_3rd(aid_src, aid_des, data, data2);
            break;
        case DDBattleObserveMessage::FRIEND_ELET:
            msg_friend_elet(aid_src, aid_des, data, data2);
            break;

        default:
            break;
    }

}

void BattleField::msg_common_agentsAddMinusMove() //当agent出现/消失时均要调用！
{
    // 红绿图
    if (_redGreenCover->isVisible())
        calcRedGreenCoverAndShow();
}

void BattleField::msg_common_friendAddMinus() //当friend出现/消失时均要调用！
{
//    calcVisionMapAndRefreshCover();
    _needToRefreshCover = true; // 由于在DIE时消息收到时，MAPDATA的数据结构还未改，所以要延时一个节拍计算COVER才正确。
}

void BattleField::msg_common_agentGone(int aid)
{
    if (_selectedAid == aid) {
        op_dismissSelection();
    }
}

void BattleField::msg_attack(int aid_src, int aid_des, int data, float data2)
{
    auto srcPos = _aidAgentNodeMap[aid_src]->getAgentPos();
    auto desPos = _aidAgentNodeMap[aid_des]->getAgentPos();
    DDElementType elet = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_src]->elementType;
    int level = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_src]->level;
    int srcAgentType = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_src]->type;

    // agent动画
    _aidAgentNodeMap[aid_src]->playAnimation(DDAnimationType::ACTION);

    // 具体动画
    switch (srcAgentType) {
        case DDAgent::AT_FRIEND_ARROW_TOWER:
            _floatEffectManager.toast(FloatingEffectManager::FE_ARROW_ATTACK, srcPos, desPos, elet, level);
            break;
        case DDAgent::AT_FRIEND_MAGIC_TOWER:
            _floatEffectManager.toast(FloatingEffectManager::FE_MAGIC_ATTACK, srcPos, desPos, elet, level);
            break;
        case DDAgent::AT_FRIEND_CORE:
            _floatEffectManager.toast(FloatingEffectManager::FE_CORE_ATTACK, srcPos, desPos, elet, level);
            break;
        case DDAgent::AT_3RD_VOLCANO:
            _floatEffectManager.toast(FloatingEffectManager::FE_VOLCANO_ATTACK, srcPos, desPos, elet, level);
            break;
        case DDAgent::AT_FRIEND_CONNON_TOWER:
            _floatEffectManager.toast(FloatingEffectManager::FE_CANNON_ATTACK, srcPos, desPos, elet, level);
            break;

        default:
            break;
    }
    // 浮动数字
    _floatNumberManager.toast(FloatingNumberManager::ATTACK, data, desPos);

    // 基本数字
    _aidAgentNodeMap[aid_des]->configSmallNumber(DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_des]);


    // 低血
}

void BattleField::msg_attack_sputt(int aid_src, int aid_des, int data, float data2)
{

    auto srcPos = _aidAgentNodeMap[aid_src]->getAgentPos();
    auto desPos = _aidAgentNodeMap[aid_des]->getAgentPos();

    // todo 传攻击者的aid作为data2（data2改成int），然后从此获得elet 和 level
    _floatEffectManager.toast(FloatingEffectManager::FE_CANNON_ATTACK_SPUTT, srcPos, desPos, DDElementType::None, 0);

    // 浮动数字
    _floatNumberManager.toast(FloatingNumberManager::ATTACK, data, desPos);

    // 基本数字
    _aidAgentNodeMap[aid_des]->configSmallNumber(DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_des]);

}

void BattleField::msg_cure(int aid_src, int aid_des, int data, float data2)
{
    // 具体动画

    _floatEffectManager.toast(FloatingEffectManager::FE_CURE, _aidAgentNodeMap[aid_src]->getAgentPos(), _aidAgentNodeMap[aid_des]->getAgentPos(), DDElementType::None, 1);

    // 浮动数字
    _floatNumberManager.toast(FloatingNumberManager::CURE, data, _aidAgentNodeMap[aid_des]->getAgentPos());
    // 基本数字

    // 低血
}

void BattleField::msg_mine(int aid_src, int adi_des, int data, float data2)
{
    // 浮动数字
    _floatNumberManager.toast(FloatingNumberManager::MINE, data, _aidAgentNodeMap[adi_des]->getAgentPos());
}

void BattleField::msg_mine_gone(int aid_src, int adi_des, int data, float data2)
{
    auto node = _aidAgentNodeMap[aid_src];

    // 效果
    node->playAnimation(DDAnimationType::DIE);

    // TODO 时序问题，效果还没播放，就已经invisible了！！
    // 数据结构
    node->configActive(false);

    _aidAgentNodeMap.erase(aid_src);

    // 更新
    msg_common_agentsAddMinusMove();
    msg_common_agentGone(aid_src);
}

void BattleField::msg_enemy_in(int aid_src, int adi_des, int data, float data2)
{
    // 敌人移入
    auto agent = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_src];
    _posAgentNodeMap[agent->pos]->configAgent(aid_src, agent->type, agent->level);
    _posAgentNodeMap[agent->pos]->configActive(true);
    _posAgentNodeMap[agent->pos]->configSmallNumber(agent);
    _posAgentNodeMap[agent->pos]->playAnimation(DDAnimationType::APPEAR);
    _aidAgentNodeMap[agent->aid] = _posAgentNodeMap[agent->pos];

    // 其它
    msg_common_agentsAddMinusMove();
}

void BattleField::msg_enemy_out(int aid_src, int adi_des, int data, float data2)
{
    auto node = _aidAgentNodeMap[aid_src];

    // 效果
    node->playAnimation(DDAnimationType::DISAPPEAR);

    // TODO 时序问题，效果还没播放，就已经invisible了！！
    // 数据结构
    node->configActive(false);

    _aidAgentNodeMap.erase(aid_src);

    // 更新
    msg_common_agentsAddMinusMove();
    msg_common_agentGone(aid_src);

}

void BattleField::msg_enemy_new(int aid_src, int aid_des, int data, float data2)
{
    // 母巢播放 动画
    _aidAgentNodeMap[aid_src]->playAnimation(DDAnimationType::ACTION);

    // TODO 可能右一条红色闪电，连接母巢和敌人

    // 敌人诞生
    auto agent = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_des];
    _posAgentNodeMap[agent->pos]->configAgent(aid_des, agent->type, agent->level);
    _posAgentNodeMap[agent->pos]->configActive(true);
    _posAgentNodeMap[agent->pos]->configSmallNumber(agent);
    _posAgentNodeMap[agent->pos]->playAnimation(DDAnimationType::APPEAR);
    _aidAgentNodeMap[agent->aid] = _posAgentNodeMap[agent->pos];

    // 其它
    msg_common_agentsAddMinusMove();
}

void BattleField::msg_enemy_move(int aid_src, int aid_des, int data, float data2)
{
    auto newPos = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_src]->pos;
    assert(newPos.isLegal());
    auto swaptoNode = _posAgentNodeMap[newPos];
    auto oldPos = _aidAgentNodeMap[aid_src]->getAgentPos();
    _aidAgentNodeMap[aid_src]->configAgentPos(newPos);
    swaptoNode->configAgentPos(oldPos);
    swaptoNode->configAgentFloatPos(help_agentPos2agentFloatPos(oldPos));
    _aidAgentNodeMap[aid_src]->animationAgentFloatPos(help_agentPos2agentFloatPos(newPos));
    _posAgentNodeMap[newPos] = _aidAgentNodeMap[aid_src];
    _posAgentNodeMap[oldPos] = swaptoNode;

    // 唯一要移动选择框的情况
    if (aid_src == _selectedAid) {
        _selectionIcon->setPosition(help_agentPos2agentFloatPos(newPos));
        _distanceBox->configCenter(newPos, true);
    }

    msg_common_agentsAddMinusMove();
}


void BattleField::msg_enemy_die(int aid_src, int aid_des, int data, float data2)
{
    auto node = _aidAgentNodeMap[aid_src];

    // 效果
    node->playAnimation(DDAnimationType::DIE);

    // TODO 时序问题，效果还没播放，就已经invisible了！！
    // 数据结构
    node->configActive(false);

    _aidAgentNodeMap.erase(aid_src);

    // 更新
    msg_common_agentsAddMinusMove();
    msg_common_agentGone(aid_src);

}

void BattleField::msg_friend_new(int aid_src, int aid_des, int data, float data2)
{
    auto agent = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_src];

    _posAgentNodeMap[agent->pos]->configAgent(aid_src, agent->type, agent->level);
    _posAgentNodeMap[agent->pos]->configLight(&_lightManager, true);
    _posAgentNodeMap[agent->pos]->configActive(true);
    _posAgentNodeMap[agent->pos]->configSmallNumber(agent);
    _posAgentNodeMap[agent->pos]->playAnimation(DDAnimationType::APPEAR);
    _aidAgentNodeMap[agent->aid] = _posAgentNodeMap[agent->pos];

    // 其它
    msg_common_agentsAddMinusMove();
    msg_common_friendAddMinus();

    // 新移入的建筑，自动选中
    showSelectionAt(agent->pos);
}

void BattleField::msg_friend_upgrade(int aid_src, int aid_des, int data, float data2)
{
    auto agent = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid_src];

//    _aidAgentNodeMap[aid_src]->playAnimation(DDAnimationType::ACTION) //todo 没动画...

    _aidAgentNodeMap[aid_src]->configSmallNumber(agent); // 更新agent上数字


    msg_common_friendAddMinus(); // 更新视野
    _distanceBox->configDistance(agent->actionDistance); // 更新视野框

    _propertyProtocal->op_refreshAgentProperty(aid_src); // 通知property field

    _aidAgentNodeMap[aid_src]->refreshLightQuality(agent->actionDistance);
}

void BattleField::msg_friend_die(int aid_src, int aid_des, int data, float data2)
{
    auto node = _aidAgentNodeMap[aid_src];

    // 效果
    node->playAnimation(DDAnimationType::DIE);

    // TODO 时序问题，效果还没播放，就已经invisible了！！
    // 数据结构
    node->configActive(false);
    node->configLight(&_lightManager, false);

    _aidAgentNodeMap.erase(aid_src);

    // 更新
    msg_common_agentsAddMinusMove();
    msg_common_friendAddMinus();
    msg_common_agentGone(aid_src);

}

void BattleField::msg_friend_sell(int aid_src, int aid_des, int data, float data2)
{
    auto node = _aidAgentNodeMap[aid_src];

    // 效果
    node->playAnimation(DDAnimationType::DISAPPEAR);

    // TODO 时序问题，效果还没播放，就已经invisible了！！
    // 数据结构
    node->configActive(false);
    node->configLight(&_lightManager, false);

    _aidAgentNodeMap.erase(aid_src);

    // 更新
    msg_common_agentsAddMinusMove();
    msg_common_friendAddMinus();
    msg_common_agentGone(aid_src);
}

void BattleField::msg_friend_elet(int aid_src, int aid_des, int data, float data2)
{
    _propertyProtocal->op_refreshAgentProperty(aid_src); // 通知property field
}

void BattleField::msg_remove_3rd(int aid_src, int aid_des, int data, float data2)
{
    auto node = _aidAgentNodeMap[aid_src];

    // 效果
    node->playAnimation(DDAnimationType::DISAPPEAR);

    // TODO 时序问题，效果还没播放，就已经invisible了！！
    // 数据结构
    node->configActive(false);
    node->configLight(&_lightManager, false);

    _aidAgentNodeMap.erase(aid_src);

    // 更新
    msg_common_agentsAddMinusMove();
    msg_common_agentGone(aid_src);
}




