// (C) 2015 Turnro.com

#include "BattleField.h"
#include "format.h"
#include "DDMapData.h"
#include "TRLocale.h"
#include "DDUpgradeLoad.h"
#include "DDConfig.h"

USING_NS_CC;


// LightManager
void LightNodeManager::init()
{
    for (int i = 0; i < DDConfig::NUM_LIGHT; i++) {
        _lightSet.push_back(&_lights[i]);
    }
}

LightNode* LightNodeManager::brrow()
{
    if (!_lightSet.empty()) {
        LightNode* ret = _lightSet.back();
        _lightSet.pop_back();
        return  ret;
    } else {
        return nullptr;
    }
}

void LightNodeManager::giveback(LightNode *node)
{
    assert(node != nullptr);
    node->quality = 0;
    _lightSet.push_back(node);
}

LightNode* LightNodeManager::getLights()
{
    return _lights;
}

// BattleField
void BattleField::init(cocos2d::Layer *battleLayer)
{
    _battleLayer = battleLayer;

    _lightManager.init();
    _floatNumberManager.init(_battleLayer);
    
    _doorLightTop = _lightManager.brrow();
    _doorLightTop->height = 2;
    _doorLightTop->color = Vec4{1,1,1,1};
    _doorLightTop->agentFloatPos = {0,DDConfig::MINMAP_EXPAND};
    _doorLightBottom = _lightManager.brrow();
    _doorLightBottom->height = 2;
    _doorLightBottom->color = Vec4{1,1,1,1};
    _doorLightBottom->agentFloatPos = {0,-DDConfig::MINMAP_EXPAND};
    _doorLightLeft = _lightManager.brrow();
    _doorLightLeft->height = 2;
    _doorLightLeft->color = Vec4{1,1,1,1};
    _doorLightLeft->agentFloatPos = {-DDConfig::MINMAP_EXPAND,0};
    _doorLightRight = _lightManager.brrow();
    _doorLightRight->height = 2;
    _doorLightRight->color = Vec4{1,1,1,1};
    _doorLightRight->agentFloatPos = {DDConfig::MINMAP_EXPAND,0};

    initMovingCover();

    _backgroundNode = DDBackgroundNode::create();
    _backgroundNode->setZOrder(Z_BACKGROUND);
    _backgroundNode->setPosition({0,0});
    _battleLayer->addChild(_backgroundNode);

    auto doors = Sprite::create("images/doors.png");
    doors->setScale(DDConfig::battleAreaRect().size.width/doors->getContentSize().width);
    _battleLayer->addChild(doors);
    doors->setZOrder(Z_DOORS);
    doors->setPosition({0,0});

/*
    auto testLow = BattleLowNode::create();
    testLow->setBuildingBg("images/test_building_bg.png");
    testLow->setFieldBg("images/test_field_bg.png");
    testLow->setPosition({0,0});
    testLow->setZOrder(BattleField::Z_LOW_NODE);
    _battleLayer->addChild(testLow);

    auto testMiddle = BattleMiddleNode::create();
    testMiddle->configShadowFile("images/test_middle_shadow.png");
    testMiddle->setPosition({0,0});
    testMiddle->setZOrder(BattleField::Z_MIDDLE_NODE);
    _battleLayer->addChild(testMiddle);
    _testMiddleNode = testMiddle;
    _testMiddleNode->retain();
    testMiddle->configSelection(1);
    testMiddle->configScopeLine({1,1,1,1});

    auto testHigh = BattleHighNode::create();
    testHigh->configBuildingAnimation("images/test_high_building_animation.png");
    testHigh->configBuildingNormalMap("images/test_high_building_normalmap.png");
    testHigh->setZOrder(BattleField::Z_HIGH_NODE);
    _battleLayer->addChild(testHigh);
    _testHighNode = testHigh;*/

    for (int i = 0; i < DDConfig::BATTLE_NUM; i++) {
        for (int j = 0; j < DDConfig::BATTLE_NUM; j++) {
            AgentPos agentPos = {i - DDConfig::BATTLE_NUM/2,j - DDConfig::BATTLE_NUM/2};
            _posAgentNodeMap[agentPos] = &_rawAgentNodes[j*DDConfig::BATTLE_NUM+i];
            AgentNode* pNode = _posAgentNodeMap[agentPos];
            pNode->init(_battleLayer);
            pNode->configAgentPos(agentPos);
            pNode->configAgentFloatPos(BattleField::help_agentPos2agentFloatPos(agentPos));
            pNode->configActive(false);
        }
    }

    _shadowCover = ShadowCover::create();
    _shadowCover->setZOrder(Z_SHADOW_COVER);
    _shadowCover->setPosition({0,0});
    _battleLayer->addChild(_shadowCover);
    _shadowCover->retain();
    _shadowCover->setVisible(false); // 弃用shadow cover

    _redGreenCover = RedGreenCover::create();
    _redGreenCover->setZOrder(Z_REDGREEN_COVER);
    _redGreenCover->setPosition({0,0});
    _battleLayer->addChild(_redGreenCover);
    _redGreenCover->dismiss();

    _distanceBox = DistanceBox::create();
    _distanceBox->setZOrder(Z_DISTANCE_BOX);
    _distanceBox->setPosition({0,0});
    _battleLayer->addChild(_distanceBox);
    _distanceBox->dismiss();

    _floatEffectManager.init(_battleLayer, &_lightManager);

    addTestLights();

    initTapMessgeThings();
    initMovingNewBuildThings();
    initSelectionThings();
}

Vec2 BattleField::help_battleLayerPos2agentFloatPos(const cocos2d::Vec2& srcPos)
{
    return {srcPos.x / DDConfig::battleCubeWidth(), srcPos.y / DDConfig::battleCubeWidth()};
}

cocos2d::Vec2 BattleField::help_agentPos2agentFloatPos(const AgentPos& agentPos)
{
    return {agentPos.x * DDConfig::battleCubeWidth(), agentPos.y * DDConfig::battleCubeWidth()};
}

void BattleField::step(float dt)
{
    _lightManager.step(dt);
    if (_needToRefreshCover) {
        _needToRefreshCover = false;
        calcVisionMapAndRefreshCover();
    }
    for (auto p: _posAgentNodeMap) {
        p.second->updateLights(_lightManager.getLights());
        if (p.second->isActive()) {
            p.second->anistep(dt);
        }
    }
    _backgroundNode->updateLights(_lightManager.getLights());
    _shadowCover->step(dt);
    _floatEffectManager.step(dt);
}


void BattleField::initMovingCover()
{
    /*
    for (int i = 0; i < 3; i++) {
        auto sp = Sprite::create(fmt::sprintf("images/moving_cover_left_%d.png", i));
        sp->setScale(DDConfig::battleAreaRect().size.width/sp->getContentSize().width);
        sp->setPosition({0,0});
        sp->setZOrder(Z_MOVING_COVER);
        _leftMovingCover[i] = sp;
        _battleLayer->addChild(sp);
    }
    for (int i = 0; i < 3; i++) {
        auto sp = Sprite::create(fmt::sprintf("images/moving_cover_right_%d.png", i));
        sp->setScale(DDConfig::battleAreaRect().size.width/sp->getContentSize().width);
        sp->setPosition({0,0});
        sp->setZOrder(Z_MOVING_COVER);
        _rightMovingCover[i] = sp;
        _battleLayer->addChild(sp);
    }*/

    for (int y = -DDConfig::MINMAP_EXPAND; y <= DDConfig::MINMAP_EXPAND; y++) {
        for (int x = -DDConfig::MINMAP_EXPAND; x <= DDConfig::MINMAP_EXPAND; x++) {
            _allCoverredvisionMap[{x,y}] = false;
        }
    }
}

void BattleField::movingCoverIn()
{
    _isMovingCovered = true;
    // moving cover 现在用shadow cover取代。当movein时，将shadow cover全部cover。
    std::vector<std::tuple<AgentPos, int, bool>> visionDiff;
    if (_hasLastBattleObserving) {
        auto& minmap = DDMapData::s()->getMinMaps()[_lastObservingMinMap];
        // 算灭失的
        for (auto posAgent : minmap->posAgentMap) {
            if (posAgent.second->isFriend()) {
                visionDiff.push_back({posAgent.first, posAgent.second->actionDistance, false});
            }
        }
    }
    // 关上4个门
    visionDiff.push_back({AgentPos{0, DDConfig::MINMAP_EXPAND}, DOOR_VISION_DISTANCE, false});
    visionDiff.push_back({AgentPos{0, -DDConfig::MINMAP_EXPAND}, DOOR_VISION_DISTANCE, false});
    visionDiff.push_back({AgentPos{ DDConfig::MINMAP_EXPAND,0}, DOOR_VISION_DISTANCE, false});
    visionDiff.push_back({AgentPos{-DDConfig::MINMAP_EXPAND,0}, DOOR_VISION_DISTANCE, false});


    _shadowCover->config(_allCoverredvisionMap, visionDiff);
    _currentVisionCoverMeta.clear();
}

void BattleField::movingCoverOut()
{
    _isMovingCovered = false;
    _lastObservingMinMap = DDMapData::s()->battleFieldObservedMinMap;
    _hasLastBattleObserving = true;
    // 实际直接让battleField refresh visionmap即可！
}

void BattleField::op_switch2minmap()
{
    dismissTapMessage();
    DDMapData::s()->isBattleFieldObserving = false;
    CCLOG("turn2newMinMap");
    float turnInWait = 0.f;
    if (!_isMovingCovered) {
        movingCoverIn();
        turnInWait += 0.5f;
    }
    // 关4个门灯
    _doorLightTop->setQuality(0, true);
    _doorLightBottom->setQuality(0, true);
    _doorLightRight->setQuality(0, true);
    _doorLightLeft->setQuality(0, true);

    // 提前clean //原先在switchMinMap()函数内
    for (int y = 0-DDConfig::BATTLE_NUM/2; y <= DDConfig::BATTLE_NUM/2; y++) {
        for (int x = 0-DDConfig::BATTLE_NUM/2; x <= DDConfig::BATTLE_NUM/2; x++) {
            AgentPos pos = {x,y};
            _posAgentNodeMap[pos]->configActive(false);
            _posAgentNodeMap[pos]->configLight(&_lightManager, false);

        }
    }


    // 激活检查
    MapPos mappos = DDMapData::s()->battleFieldObservedMinMap;
    if(DDMapData::s()->getMinMaps()[mappos]->blocked) {
        showTapMessage(false);
    } else if (DDMapData::s()->getMinMaps()[mappos]->state == DDMinMap::T_ACTIVABLE) {
        showTapMessage(true);
    } else {
        _battleLayer->scheduleOnce([this](float dt){
            op_switch2minmap_continue();
        }, turnInWait, "turnOut");
    }

    op_dismissSelection();
}

void BattleField::op_switch2minmap_continue()
{
    this->switchMinMap();
    this->movingCoverOut();
    DDMapData::s()->isBattleFieldObserving = true;
}

void BattleField::switchMinMap()
{
    MapPos mappos = DDMapData::s()->battleFieldObservedMinMap;
    CCLOG("switchMinMap to %d %d", mappos.x, mappos.y);


    // build up
    auto minmap = DDMapData::s()->getBattleObservedMinMap();

    for (auto pa : minmap->posAgentMap) {
        auto agent = pa.second;
        _posAgentNodeMap[pa.first]->configActive(true);
        _posAgentNodeMap[pa.first]->configAgent(agent->aid, agent->type, agent->level);
        _posAgentNodeMap[pa.first]->configLight(&_lightManager, true);
        _posAgentNodeMap[pa.first]->configSmallNumber(agent);
        _posAgentNodeMap[pa.first]->resetAnimation();
        _aidAgentNodeMap[pa.second->aid] = _posAgentNodeMap[pa.first];
    }

    // 计算视野
    calcVisionMapAndRefreshCover();
}

void BattleField::initTapMessgeThings()
{
    _messageBackgroundImage = Sprite::create("images/battle_message_bg.png");
    _messageBackgroundImage->setPosition({0,0});
    _messageBackgroundImage->setScale(DDConfig::battleAreaRect().size.width/_messageBackgroundImage->getContentSize().width);
    _messageBackgroundImage->setVisible(false);
    _messageBackgroundImage->setZOrder(Z_MESSAGE_BACKGROUND);
    _battleLayer->addChild(_messageBackgroundImage);

    _messageLabel = Label::createWithTTF("test message", TRLocale::s().font(), 20*DDConfig::fontSizeRadio());
    _messageLabel->setPosition({0,0});
    _messageLabel->setZOrder(Z_MESSAGE_LABEL);
    _messageLabel->setVisible(false);
    _battleLayer->addChild(_messageLabel);

    _processLabel = Label::createWithTTF("", TRLocale::s().font(), 20*DDConfig::fontSizeRadio());
    _processLabel->setPosition({0,0});
    _processLabel->setZOrder(Z_MESSAGE_LABEL);
    _battleLayer->addChild(_processLabel);
    _processLabel->setVisible(false);

    _gameOverLabel = Label::createWithTTF("", TRLocale::s().font(), 20*DDConfig::fontSizeRadio());
    _gameOverLabel->setPosition({0,0});
    _gameOverLabel->setZOrder(Z_MESSAGE_LABEL);
    _battleLayer->addChild(_gameOverLabel);
    _gameOverLabel->setVisible(false);

    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event){
        auto point = touch->getLocation();
        auto rect = DDConfig::battleAreaRect();
        return !DDPropertyFieldProtocal::flagIsTappingExclusive && (_waitingToTappingAsActive || _waitingToTappingAsRestEnough || _waitingToTappingAsReturnInvade) && rect.containsPoint(point);
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event){
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event){
        if (DDConfig::battleAreaRect().containsPoint(touch->getLocation())) {
            if (_waitingToTappingAsActive) {
                // 入侵此MinMap
                _waitingToTappingAsActive = false;
                DDMapData::s()->activeMinMap(DDMapData::s()->battleFieldObservedMinMap);
                this->dismissTapMessage();
                this->op_switch2minmap_continue();
            } else if (_waitingToTappingAsRestEnough) {
                _waitingToTappingAsRestEnough = false;
                _waitingToTappingAsReturnInvade = true;
                this->_gameOverLabel->setString(TRLocale::s()["go_return_invade"]);
            } else if (_waitingToTappingAsReturnInvade) {
                _waitingToTappingAsReturnInvade = false;
                _gameOverLabel->stopAllActions();
                _gameOverLabel->setVisible(false);
                _messageBackgroundImage->setVisible(false);
                op_newGameProcess();
            }
        }
    };

    listener->onTouchCancelled = [this](Touch* touch, Event* event){
    };

    _battleLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _battleLayer);

}

void BattleField::showTapMessage(bool isToActive)
{
    _messageBackgroundImage->setVisible(true);
    _messageLabel->stopAllActions();
    if (isToActive) {
        _messageLabel->runAction(Blink::create(1000.f, 1000*2));
    }
    _waitingToTappingAsActive = isToActive;
    _messageLabel->setString(TRLocale::s()[isToActive?"battle_tap_to_invade":"battle_its_blocked_minmap"]);

}

void BattleField::dismissTapMessage()
{
    _waitingToTappingAsActive = false;
    _messageBackgroundImage->setVisible(false);
    _messageLabel->setVisible(false);
    _messageLabel->stopAllActions();
}

void BattleField::op_newGameProcess() // 进行新游戏的图形过程。
{
    DDMapData::s()->clearAllDataForNewGame();
    DDMapData::s()->randomChooseTemplates();
    DDMapData::s()->loadMinmapTemplates();
    DDMapData::s()->loadPresentingMinmapsForNew();
    DDMapData::s()->gamingRunning = true;

    _messageBackgroundImage->setVisible(true);
    _processLabel->setVisible(true);
    _processLabel->stopAllActions();
    _processLabel->setString("");
    _newGameProcessIndex = 0;

    const float step_time = 0.05;
    // 进度条
    _battleLayer->schedule([this](float dt){

        _newGameProcessIndex++;
        if (_newGameProcessIndex <= PROCESS_INDEX_MAX) {
            std::stringstream ss;
            ss << "[";
            for (int i = 0; i < _newGameProcessIndex; i++) {
                ss << "|";
            }
            for (int i = _newGameProcessIndex; i < PROCESS_INDEX_MAX; i++) {
                ss << " ";
            }
            ss << "]";
            _processLabel->setString(ss.str());
        } else if(_newGameProcessIndex == PROCESS_INDEX_MAX+4){
            _messageBackgroundImage->setVisible(false);
            _processLabel->setVisible(false);

            // 让bigmap选择核心。然后开始游戏。

        }

    }, step_time, PROCESS_INDEX_MAX+5, 0, "new game process");

    // 一些消息
    const int N_MSGS = 8;
    for (int i = 0; i < N_MSGS; i++) {
        _battleLayer->scheduleOnce([this, i](float dt){
            _bigmapProtocal->op_showMessage(BigmapMessageType::GOOD, fmt::sprintf("new_game_process_m%d", i));
            if (i == 5) {
                //注入基础资源
                DDMapData::s()->injectResourceMine(DDConfig::NEW_GAME_INJECT_MINE, false);
                DDMapData::s()->injectResourceGas(DDConfig::NEW_GAME_INJECT_GAS, false);

            }
        }, step_time * PROCESS_INDEX_MAX * ((1.f/N_MSGS)*i + rand_0_1()*(1.f/N_MSGS)), fmt::sprintf("new game process m %d", i));
    }

    // 开启核心
    _battleLayer->scheduleOnce([this](float dt){
        _bigmapProtocal->op_selectMapPos({0,0});
        _bigmapProtocal->op_showMessage(BigmapMessageType::MOGOOD, "tut_defence_core");
    }, step_time * PROCESS_INDEX_MAX *1.5, "tut_defence_core");

    // 开启核心
    _battleLayer->scheduleOnce([this](float dt){
        _bigmapProtocal->op_showMessage(BigmapMessageType::EXGOOD,"tut_defence_core");
    }, step_time * PROCESS_INDEX_MAX *1.7, "tut_defence_core2");
}

void BattleField::op_gameOverProcess()
{
    /*
     击败核心过程。游戏主要流程。battle显示消息，核心被摧毁，您被驱逐出了地下城。本次入侵共占领了N块地下城，攫取了N多矿物，消灭了N多敌人，剩余N多矿物，N多气体。 显示2秒。『大人是否休息够了/修整完毕?』闪烁『 返回地下城』。闪，点击后进行新游戏过程。剩余资源会保留到新游戏里面。
     */

    _messageBackgroundImage->setVisible(true);
    _gameOverLabel->setVisible(true);
    _gameOverLabel->stopAllActions();
    _gameOverLabel->setString(TRLocale::s()["go_drive_out"]);

    // battle部分
    const float disp_result_time = 3.f;
    _battleLayer->scheduleOnce([this](float dt){
        _gameOverLabel->setString(TRLocale::s()["go_rest_enough"]);
        _gameOverLabel->runAction(RepeatForever::create( Blink::create(5.f, 10)));
        _waitingToTappingAsRestEnough = true;
    }, disp_result_time, "go_0");

    // bigmap的消息
    _battleLayer->scheduleOnce([this](float dt){
        _bigmapProtocal->op_showMessage(BigmapMessageType::MOGOOD, "go_result_start");
    }, disp_result_time * 0.1, "disp result 0");
    _battleLayer->scheduleOnce([this](float dt){
        _bigmapProtocal->op_showMessage(BigmapMessageType::MOGOOD, fmt::sprintf(TRLocale::s()["go_mine"], DDMapData::s()->_cntMineGot), true);
    }, disp_result_time * 0.3, "disp result 1");
    _battleLayer->scheduleOnce([this](float dt){
        _bigmapProtocal->op_showMessage(BigmapMessageType::MOGOOD,fmt::sprintf(TRLocale::s()["go_kill"], DDMapData::s()->_cntEnemyKilled), true);
    }, disp_result_time * 0.5, "disp result 2");
    _battleLayer->scheduleOnce([this](float dt){
        _bigmapProtocal->op_showMessage(BigmapMessageType::MOGOOD, fmt::sprintf(TRLocale::s()["go_mine_left"], DDMapData::s()->_cntMineLeft), true);
    }, disp_result_time * 0.6, "disp result 3");
    _battleLayer->scheduleOnce([this](float dt){
        _bigmapProtocal->op_showMessage(BigmapMessageType::MOGOOD, fmt::sprintf(TRLocale::s()["go_gas_left"], DDMapData::s()->_cntGasLeft), true);
    }, disp_result_time * 0.7, "disp result 4");
    _battleLayer->scheduleOnce([this](float dt){
        _bigmapProtocal->op_showMessage(BigmapMessageType::MOGOOD, "go_result_end");
    }, disp_result_time * 0.9, "disp result 5");

}



 int BattleField::help_getWhichAgentTypeMoveIn(int buildingIndex) // 返回building field的序号对应的friend agent的类型。
{
    int t = DDAgent::AT_FRIEND_WALL;
    switch (buildingIndex) {
        case 0:
            t = DDAgent::AT_FRIEND_WALL;
            break;
        case 1:
            t = DDAgent::AT_FRIEND_MINER;
            break;
        case 2:
            t = DDAgent::AT_FRIEND_ARROW_TOWER;
            break;
        case 3:
            t = DDAgent::AT_FRIEND_MAGIC_TOWER;
            break;
        case 4:
            t = DDAgent::AT_FRIEND_CONNON_TOWER;
            break;
        case 5:
            t = DDAgent::AT_FRIEND_CURE_TOWER;
            break;
        case 6:
            t = DDAgent::AT_FRIEND_LIGHT_TOWER;
            break;
        default:
            assert(false);
            break;
    }
    return t;
}

std::pair<bool, AgentPos> BattleField::help_touchPoint2agentPos(const cocos2d::Vec2& point) // 计算触摸到的battle field内部的坐标
{
    bool isok = false;
    AgentPos retpos;
    auto rect = DDConfig::battleAreaRect();
    if (rect.containsPoint(point)) {
        isok = true;

        float cubewidth = rect.size.width / DDConfig::BATTLE_NUM;
        int rx = (point.x - rect.origin.x)/cubewidth;
        int ry = (point.y - rect.origin.y)/cubewidth;
        retpos = {rx - DDConfig::MINMAP_EXPAND, ry - DDConfig::MINMAP_EXPAND};
    }
    return {isok, retpos};
}


void BattleField::initMovingNewBuildThings()
{

    static bool touch_moved = false;
    static int which_is_selected = 0;
    static int agent_type = 0;
    static int agent_distance = 0;
    static bool ispuutable = false;
    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event){
        auto point = touch->getLocation();
        auto rect = DDConfig::buildingAreaRect();
        touch_moved = false;
        if (!DDPropertyFieldProtocal::flagIsTappingExclusive && rect.containsPoint(touch->getLocation())) {
            //算出选中了哪一个
            auto point = touch->getLocation();
            float diffX = point.x - rect.origin.x;
            float widthStep = DDConfig::buildingAreaSelectionWidth();
            which_is_selected = diffX / widthStep;
            if (which_is_selected > 6 || which_is_selected < 0) {
                return false;
            }
            agent_type = help_getWhichAgentTypeMoveIn(which_is_selected);
            //资源是否够
            int cost = DDUpgradeRoadMap::fetchFriendCost(agent_type);
            if (cost > DDMapData::s()->_cntMineLeft) {
                // TODO tell no enough resource
                return false;
            }
            agent_distance = DDUpgradeRoadMap::getPropertyUpgradeMap(agent_type, PropertyType::ACTION_DISTANCE).start;
            CCLOG("buildingfiled select %d distance %d", which_is_selected, agent_distance);
            return true;
        } else {
            return false;
        }
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event){
        touch_moved = true;
        auto rect = DDConfig::battleAreaRect();
        auto ifagentpos = help_touchPoint2agentPos(touch->getLocation());
        if (ifagentpos.first) {
            //CCLOG("moving in at %d %d", ifagentpos.second.x, ifagentpos.second.y);
            // TODO 在视野地图的基础上，显示 红绿层
            if (!_redGreenCover->isVisible()) {
                this->calcRedGreenCoverAndShow();
            }
            if (!_distanceBox->isVisible()) {
                _distanceBox->show();
            }
            ispuutable = _redGreenMap[ifagentpos.second] == RedGreenCover::CS_GREEN;
            _distanceBox->configDistance(agent_distance);

            _distanceBox->configCenter(ifagentpos.second, ispuutable);
        } else {
            if (_redGreenCover->isVisible()) {
                _redGreenCover->dismiss();
            }
            if (_distanceBox->isVisible()) {
                _distanceBox->dismiss();
            }
        }
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event){
        if (_redGreenCover->isVisible()) {
            _redGreenCover->dismiss();
        }
        if (_distanceBox->isVisible()) {
            _distanceBox->dismiss();
        }
        auto ifagentpos = help_touchPoint2agentPos(touch->getLocation());
        if (ifagentpos.first && _redGreenMap[ifagentpos.second]== RedGreenCover::CS_GREEN) {
            DDMapData::s()->appearAgent_friend(ifagentpos.second, agent_type);
        }
    };

    listener->onTouchCancelled = [this](Touch* touch, Event* event){
    };

    _battleLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _battleLayer);
}


void BattleField::initSelectionThings()
{
    _selectionIcon = Sprite::create("images/battle_select.png");
    _selectionIcon->setScale(DDConfig::battleCubeWidth()/_selectionIcon->getContentSize().width);
    _selectionIcon->setVisible(false);
    _selectionIcon->setZOrder(Z_SELECTION_ICON);
    _battleLayer->addChild(_selectionIcon);


    static bool touch_moved = false;

    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event){
        auto rect = DDConfig::battleAreaRect();
        touch_moved = false;
        return !DDPropertyFieldProtocal::flagIsTappingExclusive && rect.containsPoint(touch->getLocation());
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event){
        touch_moved = true;
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event){
        if (!touch_moved) {
            auto ifagentpos = help_touchPoint2agentPos(touch->getLocation());
            if (ifagentpos.first && _visionMap[ifagentpos.second] &&_posAgentNodeMap[ifagentpos.second]->isActive()) {
                this->showSelectionAt(ifagentpos.second);
                return;
            }
        }
        op_dismissSelection();
    };

    listener->onTouchCancelled = [this](Touch* touch, Event* event){
    };

    _battleLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _battleLayer);

}

void BattleField::showSelectionAt(const AgentPos& agentpos)
{
    _selectionIcon->setVisible(true);
    _selectionIcon->setPosition(help_agentPos2agentFloatPos(agentpos));

    _selectedAid = _posAgentNodeMap[agentpos]->getAid();
    _buildingProtocal->op_cancelSelection();
    _propertyProtocal->op_dispBattlingAgent(_selectedAid);
    _distanceBox->configDistance(DDMapData::s()->getBattleObservedMinMap()->posAgentMap[agentpos]->actionDistance);
    _distanceBox->configCenter(agentpos, true);
    _distanceBox->show();
}

void BattleField::op_dismissSelection()// 通知取消选择
{
    _selectedAid = -1;
    _selectionIcon->setVisible(false);
    _distanceBox->dismiss();
    _propertyProtocal->op_dismissAllAgent();
}

void BattleField::calcVisionMapAndRefreshCover() //计算视野地图，同时更新cover层。
{
    // 清空
    for (int x = -(DDConfig::MINMAP_EXPAND); x <= (DDConfig::MINMAP_EXPAND); x++) {
        for (int y = -(DDConfig::MINMAP_EXPAND); y <= (DDConfig::MINMAP_EXPAND); y++) {
            _visionMap[{x, y}] = false;
        }
    }

    std::vector<std::tuple<AgentPos, int, bool>> visionDiff;

    // 计算4个门
    MapPos current = DDMapData::s()->battleFieldObservedMinMap;
    if (DDMapData::s()->isMinMapActive({current.x, current.y+1})) {
        calcVisionMap_ast({0, DDConfig::MINMAP_EXPAND}, DOOR_VISION_DISTANCE);
        visionDiff.push_back({AgentPos{0, DDConfig::MINMAP_EXPAND}, DOOR_VISION_DISTANCE, true});
        _doorLightTop->setQuality(DOOR_VISION_DISTANCE, true);
    } else {
        _doorLightTop->setQuality(0, true);
    }
    if (DDMapData::s()->isMinMapActive({current.x, current.y-1})) {
        calcVisionMap_ast({0, -DDConfig::MINMAP_EXPAND}, DOOR_VISION_DISTANCE);
        visionDiff.push_back({AgentPos{0, -DDConfig::MINMAP_EXPAND}, DOOR_VISION_DISTANCE, true});
        _doorLightBottom->setQuality(DOOR_VISION_DISTANCE, true);
    } else {
        _doorLightBottom->setQuality(0, true);
    }
    if (DDMapData::s()->isMinMapActive({current.x+1, current.y})) {
        calcVisionMap_ast({DDConfig::MINMAP_EXPAND, 0}, DOOR_VISION_DISTANCE);
        visionDiff.push_back({AgentPos{DDConfig::MINMAP_EXPAND, 0}, DOOR_VISION_DISTANCE, true});
        _doorLightRight->setQuality(DOOR_VISION_DISTANCE, true);
    } else {
        _doorLightRight->setQuality(0, true);
    }
    if (DDMapData::s()->isMinMapActive({current.x-1, current.y})) {
        calcVisionMap_ast({-DDConfig::MINMAP_EXPAND, 0}, DOOR_VISION_DISTANCE);
        visionDiff.push_back({AgentPos{-DDConfig::MINMAP_EXPAND, 0}, DOOR_VISION_DISTANCE, true});
        _doorLightLeft->setQuality(DOOR_VISION_DISTANCE, true);
    } else {
        _doorLightLeft->setQuality(0, true);
    }

    // 计算里面所有的我军agent导致的视野
    auto& minmap = DDMapData::s()->getBattleObservedMinMap();
    for (auto pair : minmap->posAgentMap) {
        if (pair.second->isFriend()) {
            calcVisionMap_ast(pair.first, pair.second->actionDistance);
        }
    }

    // 检查是否全开。
    bool isallopen = true;
    for (auto pp : _visionMap) {
        if (!pp.second) {
            isallopen = false;
            break;
        }
    }
    if (isallopen) {
        DDMapData::s()->tellCurrentAllVisionOpen();
    }


    // 计算shadow cover动画数据。

    // 算灭失的
    for (auto iter = _currentVisionCoverMeta.begin(); iter != _currentVisionCoverMeta.end(); ) {
        if (minmap->posAgentMap.count((*iter).first) == 0) {
            visionDiff.push_back({(*iter).first, (*iter).second, false});
            iter = _currentVisionCoverMeta.erase(iter);
        } else {
            iter++;
        }
    }

    // 算新增的
    for (auto posAgent : minmap->posAgentMap) {
        if (posAgent.second->isFriend()) {
            if (_currentVisionCoverMeta.count(posAgent.first) == 0 || _currentVisionCoverMeta[posAgent.first] < posAgent.second->actionDistance) {
                visionDiff.push_back({posAgent.first, posAgent.second->actionDistance, true});
                _currentVisionCoverMeta[posAgent.first] = posAgent.second->actionDistance;
            }
        }
    }

    _shadowCover->config(_visionMap, visionDiff);

}


void BattleField::calcVisionMap_ast(const AgentPos& agentpos, int visionDistance)
{
    for (int x = agentpos.x - visionDistance; x <= agentpos.x + visionDistance; x++) {
        for (int y = agentpos.y - visionDistance; y <= agentpos.y + visionDistance; y++) {
            AgentPos pos = {x,y};
            if (pos.isLegal() && pos.distance(agentpos) <= visionDistance) {
                _visionMap[pos] = true;
            }
        }
    }

}

bool BattleField::help_isDoorPosition(const AgentPos& agentpos)
{
    // 3格门
    return (std::abs(agentpos.x) <= 1 && std::abs(agentpos.y) == DDConfig::MINMAP_EXPAND) || (std::abs(agentpos.y) <= 1 && std::abs(agentpos.x) == DDConfig::MINMAP_EXPAND);
}


void BattleField::calcRedGreenCoverAndShow()
{
    auto& minmap = DDMapData::s()->getBattleObservedMinMap();
    for (auto pair : _visionMap) {
        if (pair.second && minmap->posAgentMap.count(pair.first) == 0 && !help_isDoorPosition(pair.first)) {
            // 视野内，没有agent占位，不再门口
            _redGreenMap[pair.first] = RedGreenCover::CS_GREEN;
        } else if (pair.second) {
            _redGreenMap[pair.first] = RedGreenCover::CS_RED;
        } else {
            _redGreenMap[pair.first] = RedGreenCover::CS_NONE;
        }
    }
    for (auto pair:_redGreenMap) {
        _redGreenCover->configCover(pair.first, pair.second);
    }
    _redGreenCover->show();
}