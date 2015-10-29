// (C) 2015 Turnro.com

#include "BattleField.h"
#include "format.h"
#include "DDAgent.h"
#include "DDMapData.h"

USING_NS_CC;

void AgentNode::init(cocos2d::Layer * battleLayer)
{
    _battleLayer = battleLayer;

    auto testLow = BattleLowNode::create();
    testLow->setBuildingBg("images/test_building_bg.png");
    testLow->setFieldBg("images/test_field_bg.png");
    testLow->setPosition({0,0});
    testLow->setZOrder(BattleField::Z_LOW_NODE);
//    _battleLayer->addChild(testLow);
    _lowNode = testLow;
    _lowNode->retain();

    auto testMiddle = BattleMiddleNode::create();
    testMiddle->configShadowFile("images/test_middle_shadow.png");
    testMiddle->setPosition({0,0});
    testMiddle->setZOrder(BattleField::Z_MIDDLE_NODE);
//    _battleLayer->addChild(testMiddle);
    testMiddle->retain();
    _middleNode = testMiddle;
//    testMiddle->configSelection(1);
//    testMiddle->configScopeLine({1,1,1,1});

    auto testHigh = BattleHighNode::create();
    testHigh->configBuildingTexture("images/test_high_building_animation.png");
    testHigh->setZOrder(BattleField::Z_HIGH_NODE);
    _battleLayer->addChild(testHigh);
    _highNode = testHigh;
    _highNode->retain();

    const float ftz = 25;
    _lbAction = Label::createWithTTF("00", "fonts/fz.ttf", ftz);
    _lbAction->setZOrder(BattleField::Z_SMALL_NUMBER);
//    _lbAction->setTextColor(Color4B::RED);
    _battleLayer->addChild(_lbAction);
    _lbDistance = Label::createWithTTF("00", "fonts/fz.ttf", ftz);
    _lbDistance->setZOrder(BattleField::Z_SMALL_NUMBER);
    _battleLayer->addChild(_lbDistance);
    _lbBlood = Label::createWithTTF("00", "fonts/fz.ttf", ftz);
    _lbBlood->setZOrder(BattleField::Z_SMALL_NUMBER);
    _battleLayer->addChild(_lbBlood);
//    _lbBlood->setTextColor(Color4B::BLUE);

}

//void AgentNode::config(const std::string& buildingFile)
//{
//    _highNode->configBuildingAnimation(buildingFile);
//}

void AgentNode::configAgentFloatPos(const Vec2& pos)
{
    _lowNode->setPosition(pos);
    _middleNode->setPosition(pos);
    _highNode->setPosition(pos);
    const float cubeWith = DDConfig::battleCubeWidth();
    const float offset = 0.35f;
    _lbBlood->setPosition(pos + Vec2{cubeWith*offset, cubeWith*offset});
    _lbAction->setPosition(pos + Vec2{cubeWith*-offset, cubeWith*-offset});
    _lbDistance->setPosition(pos + Vec2{cubeWith*-offset, cubeWith*offset});
}

void AgentNode::animationAgentFloatPos(const cocos2d::Vec2& moveTo)
{
    _lowNode->runAction(MoveTo::create(0.1, moveTo));
    _middleNode->runAction(MoveTo::create(0.1, moveTo));
    _highNode->runAction(MoveTo::create(0.1, moveTo));

    const float cubeWith = DDConfig::battleCubeWidth();
    const float offset = 0.35f;

    _lbBlood->setPosition(moveTo + Vec2{cubeWith*offset, cubeWith*offset});
    _lbAction->setPosition(moveTo + Vec2{cubeWith*-offset, cubeWith*-offset});
    _lbDistance->setPosition(moveTo + Vec2{cubeWith*-offset, cubeWith*offset});
}

void AgentNode::configAgentPos(const AgentPos &pos)
{
    _agentPos = pos;
}

void AgentNode::updateLights(LightNode* lights)
{
    _middleNode->updateLights(lights);
    _highNode->updateLights(lights);
}

void AgentNode::configActive(bool enable)
{
    _active = enable;
//    _lowNode->setVisible(enable);
//    _middleNode->setVisible(enable);
    _highNode->setVisible(enable);
    if (!enable) {
        _lbAction->setVisible(false);
        _lbBlood->setVisible(false);
        _lbDistance->setVisible(false);
    }
    resetAnimation();
}

bool AgentNode::isActive()
{
    return _active;
}

int AgentNode::playAnimation(DDAnimationType type)
{

    _currentAnimationType = type;
    _currentAnimationStartEnd = DDAnimationManager::fetch(_agentType, type);
    _currentAnimationDirection = _currentAnimationStartEnd.second >= _currentAnimationStartEnd.first ? 1:-1;
    _currentAnimationFrameIndex = _currentAnimationDirection > 0 ? _currentAnimationStartEnd.first-1 : _currentAnimationStartEnd.first+1;
    return std::abs(_currentAnimationStartEnd.first - _currentAnimationStartEnd.second) + 1;
}

void AgentNode::configSmallNumber(DDAgent* agent)
{

    // blood
    bool isblood = false;
    int blood = 0;
    if (agent->isEnemy() || agent->isFriend()) {
        isblood = true;
        blood = agent->blood;
    } else if (agent->type == DDAgent::AT_3RD_MINE) {
        isblood = true;
        blood = agent->mineAmount;
    }

    // action
    bool isaction = false;
    int action = 0;
    if (agent->isEnemy() && agent->type != DDAgent::AT_ENEMY_NEST) {
        isaction = true;
        action = agent->attack;
    } else if (agent->type == DDAgent::AT_FRIEND_MINER) {
        isaction = true;
        action = agent->mineSpeed;
    } else if (agent->type == DDAgent::AT_3RD_WATER || agent->type == DDAgent::AT_FRIEND_CURE_TOWER) {
        isaction = true;
        action = agent->cure;
    } else if (agent->type == DDAgent::AT_FRIEND_ARROW_TOWER ||
               agent->type == DDAgent::AT_FRIEND_MAGIC_TOWER ||
               agent->type == DDAgent::AT_FRIEND_CONNON_TOWER ||
               agent->type == DDAgent::AT_FRIEND_CORE) {
        isaction = true;
        action = agent->attack;
    }

    _lbBlood->setVisible(isblood);
    _lbBlood->setString(fmt::sprintf("%2d", blood));

    _lbAction->setVisible(isaction);
    _lbAction->setString(fmt::sprintf("%2d", action));
    /*
    // distance
    bool isdistance = false;
    int distance = 0;
    if (agent->type == DDAgent::AT_3RD_WATER ||
        agent->type == DDAgent::AT_3RD_VOLCANO ||
        agent->type == DDAgent::AT_ENEMY_FAR ||
        agent->type == DDAgent::AT_ENEMY_NEAR ||
        (agent->isFriend() && agent->type != DDAgent::AT_FRIEND_WALL)) {
        isdistance = true;
        distance = agent->actionDistance;
    }


    _lbDistance->setVisible(isdistance);
    _lbDistance->setString(fmt::sprintf("%2d", distance));*/
}

void AgentNode::resetAnimation()
{
    playAnimation(DDAnimationType::IDLE);
}

int AgentNode::nextFrameIndex()
{
    if (_currentAnimationType == DDAnimationType::IDLE) {
        if (_currentAnimationDirection > 0) {
            _currentAnimationFrameIndex++;
            if (_currentAnimationFrameIndex > _currentAnimationStartEnd.second) {
                _currentAnimationFrameIndex = _currentAnimationStartEnd.first;
            }
        } else {
            _currentAnimationFrameIndex--;
            if (_currentAnimationFrameIndex < _currentAnimationStartEnd.second) {
                _currentAnimationFrameIndex = _currentAnimationStartEnd.first;
            }
        }
        return _currentAnimationFrameIndex;
    }

    if (_currentAnimationDirection > 0) {
        _currentAnimationFrameIndex++;
        if (_currentAnimationFrameIndex > _currentAnimationStartEnd.second) {
            resetAnimation();
            return nextFrameIndex();
        }
    } else {
        _currentAnimationFrameIndex--;
        if (_currentAnimationFrameIndex < _currentAnimationStartEnd.second) {
            resetAnimation();
            return nextFrameIndex();
        }
    }
    return _currentAnimationFrameIndex;
}

void AgentNode::configAgent(int aid, int agentType, int agentLevel)
{
    _aid = aid;
    _agentType = agentType;
    _agentLevel = agentLevel;


    std::string typeKey = agentType2agentTypeString(agentType);
    CCLOG("build agent node: %s", typeKey.c_str());
    _middleNode->configShadowFile(fmt::sprintf("agents/%s.shadow.png", typeKey));
    _middleNode->setVisible(true);

    _highNode->configBuildingTexture(fmt::sprintf("agents/%s%d.json.png", typeKey, agentLevel));
    _highNode->setVisible(true);
}

void AgentNode::configLight(LightNodeManager* lightManager, bool haslight)
{
    if (haslight && _lightNode == nullptr && (_agentType == DDAgent::AT_FRIEND_LIGHT_TOWER || _agentType == DDAgent::AT_3RD_VOLCANO)) {
        // 灯塔和火山 具有绑定光源
        auto lightnode= lightManager->brrow();
        if (lightnode) {
            _lightNode = lightnode;
            _lightNode->agentFloatPos = {static_cast<float>(_agentPos.x), static_cast<float>(_agentPos.y)};
            _lightNode->height = 2;
            _lightNode->color = _agentType == DDAgent::AT_FRIEND_LIGHT_TOWER ? Vec4{1,1,1,1} : Vec4{1,0,0,1};
            _lightNode->setQuality(DDConfig::actionDistance2lightQuality(DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[_aid]->actionDistance), true);
        } else {
            CCLOG("not enough lgiht node");
        }
    }
    if (!haslight && _lightNode) {
        lightManager->giveback(_lightNode);
        _lightNode->setQuality(0, true);
        _lightNode = nullptr;
    }
}

void AgentNode::refreshLightQuality(int actionDistance)
{
    if (_lightNode) {
        _lightNode->setQuality(DDConfig::actionDistance2lightQuality(actionDistance), true);
    }
}

void AgentNode::anistep(float dt)
{
    static int cnt = 0;
    cnt++;
    if (cnt % 3 == 0) {
        cnt = 0;
        _highNode->configBuildingAnimationIndex(nextFrameIndex());

    }
}