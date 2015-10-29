// (C) 2015 Turnro.com

#include "PropertyField.h"
#include "format.h"
#include "DDMapData.h"
#include "TRLocale.h"

USING_NS_CC;

void PropertyField::init(cocos2d::Layer *propertyLayer)
{
    _propertyLayer = propertyLayer;

    _headImage = Sprite::create("images/property/idle_head.png");
    _headImage->setPosition(DDConfig::relativePos(0.13, 0.13));
    _headImage->setScale(DDConfig::relativeScaler(1.f));
    _headImage->setZOrder(Z_HEAD_IMAGE);
    _propertyLayer->addChild(_headImage);

    _titleLabel = Label::createWithTTF("Idle title", TRLocale::s().font(), 30*DDConfig::fontSizeRadio());
    _titleLabel->setPosition(DDConfig::relativePos(0.62, 0.12));
    _titleLabel->setWidth(DDConfig::relativeScaler(1.f)*500);
    _titleLabel->setAlignment(TextHAlignment::LEFT);
    _titleLabel->setZOrder(Z_TEXT);
    _propertyLayer->addChild(_titleLabel);

    _describeLabel = Label::createWithTTF("Describe text string...", TRLocale::s().font(), 25*DDConfig::fontSizeRadio());
    _describeLabel->setPosition(DDConfig::relativePos(0.62, 0.095));
    _describeLabel->setWidth(DDConfig::relativeScaler(1.f)*500);
    _describeLabel->setAlignment(TextHAlignment::LEFT);
    _describeLabel->setZOrder(Z_TEXT);
    _propertyLayer->addChild(_describeLabel);

    float x_now = 0.31;
    float x_step = 0.12;
    float y_image = 0.045;
    float y_num_diff = -0.012;
    float y_cost_diff = -0.028;
    float x_cost_diff = 0.005;
    float x_text_width = 60*DDConfig::relativeScaler(1.0);

    auto genPropertyNode = [this, y_num_diff, y_cost_diff,x_cost_diff, x_text_width](float x, float y){
        PropertyNode ret;
        auto image = Sprite::create("images/property/idle_property.png");
        image->setPosition(DDConfig::relativePos(x, y));
        image->setScale(DDConfig::relativeScaler(0.7f));
        image->setZOrder(Z_PROPERTY_IMAGE);
        image->setVisible(false);
        _propertyLayer->addChild(image);
        ret.image = image;

        auto num = Label::createWithTTF("2/10", TRLocale::s().font(), 18*DDConfig::fontSizeRadio());
        num->setPosition(DDConfig::relativePos(x, y + y_num_diff));
        num->setZOrder(Z_PROPERTY_NUM);
        num->setWidth(x_text_width);
        num->setAlignment(TextHAlignment::RIGHT);
        num->setVisible(false);
        _propertyLayer->addChild(num);
        ret.numLabel = num;


        auto cost = Label::createWithTTF("200", TRLocale::s().font(), 18*DDConfig::fontSizeRadio());
        cost->setPosition(DDConfig::relativePos(x+x_cost_diff, y + y_cost_diff));
        cost->setZOrder(Z_PROPERTY_COST);
        cost->setWidth(x_text_width);
        cost->setAlignment(TextHAlignment::CENTER);
        cost->setVisible(false);
        _propertyLayer->addChild(cost);
        ret.costLabel = cost;
        return  ret;
    };

    // 下方基本图标序列
    for (int i = 0; i < NUM_PROPERTY_MAX; i++) {
        _propertyNodes[i] = genPropertyNode(x_now, y_image);
        x_now += x_step;
    }

    // 五行属性图标
    _elementTypeNode = genPropertyNode(x_now, 0.105);

    // 移除售卖
    _removeNode = genPropertyNode(x_now, y_image);

    _removeNodeConfirmCover = Sprite::create("images/property/sell_confirm_cover.png");
    _removeNodeConfirmCover->setPosition(DDConfig::relativePos(x_now, y_image));
    _removeNodeConfirmCover->setScale(DDConfig::relativeScaler(0.7f));
    _removeNodeConfirmCover->setZOrder(Z_PROPERTY_CONFIRM_IMAGE);
    _removeNodeConfirmCover->setVisible(false);
    _propertyLayer->addChild(_removeNodeConfirmCover);


    initTouchThings();
    initFiveSelectionThings();
}


void PropertyField::op_dispBuildingTemplate(int agentType) // 显示building区域某个某个选择的信息。
{
    if (_showState != SS_NONE) {
        op_dismissAllAgent();
        op_dismissTemplate();
    }
    _showState = SS_TEMPLATE;
    help_dispBaseInfo(agentType, 0);
    auto supportedProps = DDUpgradeRoadMap::getUpdradableProperties(agentType);
    assert(supportedProps.size() <= NUM_PROPERTY_MAX);
    for (int i = 0; i < supportedProps.size(); i++) {

        auto node =  &_propertyNodes[i];

        if (supportedProps[i] == PropertyType::NONE) { //为了让升级的属性位置统一而NONE，而特殊处理
            node->enable = false;
            continue;
        }
        node->enable = true;
        node->isShowCost = false;
        node->isCoste = false;
        node->clickable = false;

        node->propType = supportedProps[i];
        auto upgrade = DDUpgradeRoadMap::getPropertyUpgradeMap(agentType, node->propType);
        std::string iconstr = fmt::sprintf("%d", upgrade.start);
        node->iconstr = iconstr;
        node->cost = upgrade.cost;
    }
    help_showPropNodes();
}

inline std::string help_elementType2iconStr(DDElementType elet)
{
    switch (elet) {
        case DDElementType::None:
        return TRLocale::s()["et_none"];

        case DDElementType::Metal:
        return TRLocale::s()["et_metal"];

        case DDElementType::Wood:
        return TRLocale::s()["et_wood"];

        case DDElementType::Water:
        return TRLocale::s()["et_water"];

        case DDElementType::Fire:
        return TRLocale::s()["et_fire"];

        case DDElementType::Earth:
        return TRLocale::s()["et_earth"];

        default:
        assert(false);
        break;
    }
}

/*
 属性数值的显示策略。
 对于 矿/矿总量，血/血最大值  采用 10/20的显示方式。需要更新。
 对于 攻击力，范围，挖矿速度等，直接显示数字，9，如果是我军建筑，并且达到了升级的最大值，则显示MAX。通过cost数值，来防止达到MAX。

 */

void PropertyField::op_dispBattlingAgent(int aid) // 显示当前battle field正在Observed的minmap内的某个agent。
{
    if (_showState != SS_NONE) {
        op_dismissAllAgent();
        op_dismissTemplate();
    }
    _showState = SS_AGENT;
    DDAgent* agent = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid];
    help_dispBaseInfo(agent->type, agent->level);
    _battleAgentAid = agent->aid;
    help_calcPropNodesData(agent);
    help_showPropNodes();


}

void PropertyField::op_refreshAgentProperty(int aid) // 选择的agent属性变更通知到来更新这里的表现。
{
    //check
    if (_showState == SS_AGENT &&_battleAgentAid == aid) {
        DDAgent* agent = DDMapData::s()->getBattleObservedMinMap()->aidAgentMap[aid];
        help_dispBaseInfo(agent->type, agent->level); // 更新，可能升级后会变化
        help_calcPropNodesData(agent); // 更新各节点数据
        help_refreshPropNodes(); // 更新各节点图形
    }

}

void PropertyField::op_refreshWithCosts() // 当资源变更时，调用到此来更新一些花费的node。
{
    
}

void PropertyField::help_calcPropNodesData(DDAgent* agent)
{
    if (agent->isFriend()) {
        _tappingEnable = true;
        // 基本属性
        auto supportedProps = DDUpgradeRoadMap::getUpdradableProperties(agent->type);
        for (int i = 0; i < supportedProps.size(); i++) {
            auto node =  &_propertyNodes[i];
            node->propType = supportedProps[i];
            if (supportedProps[i] == PropertyType::NONE) { //为了让升级的属性位置统一而NONE，而特殊处理
                node->enable = false;
                continue;
            }

            auto upgradeMap = DDUpgradeRoadMap::getPropertyUpgradeMap(agent->type, node->propType);

            auto vpair = help_fetchAgentPropertyValue(agent, node->propType, upgradeMap.end);
            auto upgradeConfig = DDUpgradeRoadMap::fetchUpgradeConfigs(agent->type, node->propType, vpair.first);

            node->enable = true;
            node->isShowCost = upgradeConfig.upgradable;
            node->isCoste = upgradeConfig.upgradable;
            node->clickable = upgradeConfig.upgradable;

            node->iconstr = vpair.second;

            node->cost = upgradeConfig.cost;
        }

        // 五行
        _elementTypeNode.enable = true;
        _elementTypeNode.isShowCost = true;
        _elementTypeNode.isCoste = true;
        _elementTypeNode.clickable = true;
        _elementTypeNode.propType = help_elementType2propType(agent->elementType);
        _elementTypeNode.iconstr = help_elementType2iconStr(agent->elementType);
        _elementTypeNode.cost = -1; //-1不显示。

        // 变卖
        _removeNode.enable = true;
        _removeNode.isShowCost = true;
        _removeNode.isCoste = false;
        _removeNode.clickable = true;
        _removeNode.propType = PropertyType::SELL_FRIEND;
        _removeNode.iconstr = TRLocale::s()["sell_friend"];
        _removeNode.cost = DDUpgradeRoadMap::fetchSellFriendGot(agent); // TODO 变卖物价处理 对核心的变卖要进行特殊处理！！ 要把老家也卖了？ 另外，变卖是需要确认的！
    } else if (agent->isEnemy()) {
        _tappingEnable = false;

        // 基本属性
        // 血
        {
            auto node = &_propertyNodes[0];
            node->enable = true;
            node->isShowCost = false;
            node->isCoste = false;
            node->clickable = false;

            node->propType = PropertyType::BLOOD_MAX;
            node->iconstr = help_fetchAgentPropertyValue(agent, PropertyType::BLOOD_MAX, -1).second;
            node->cost = -1;
        }
        if (agent->type != DDAgent::AT_ENEMY_NEST) {
            // 攻击力
            {
                auto node = &_propertyNodes[1];
                node->enable = true;
                node->isShowCost = false;
                node->isCoste = false;
                node->clickable = false;

                node->propType = PropertyType::ATTACK;
                node->iconstr = fmt::sprintf("%d", agent->attack);
                node->cost = -1;
            }
            // 攻击范围
            {
                auto node = &_propertyNodes[2];
                node->enable = true;
                node->isShowCost = false;
                node->isCoste = false;
                node->clickable = false;

                node->propType = PropertyType::ACTION_DISTANCE;
                node->iconstr = fmt::sprintf("%d", agent->actionDistance);
                node->cost = -1;
            }
            if (agent->isBoss) {
                auto node = &_propertyNodes[3];
                node->enable = true;
                node->isShowCost = false;
                node->isCoste = false;
                node->clickable = false;

                node->propType = PropertyType::BOSS;
                node->iconstr = "";
                node->cost = -1;
            }
        }
        // 五行
        _elementTypeNode.enable = true;
        _elementTypeNode.isShowCost = false;
        _elementTypeNode.isCoste = false;
        _elementTypeNode.clickable = false;
        _elementTypeNode.propType = help_elementType2propType(agent->elementType);
        _elementTypeNode.iconstr = help_elementType2iconStr(agent->elementType);
        _elementTypeNode.cost = -1;//-1不显示。
    } else if (agent->type == DDAgent::AT_3RD_MINE) {
        // 矿量
        {
            auto node = &_propertyNodes[0];
            node->enable = true;
            node->isShowCost = false;
            node->isCoste = false;
            node->clickable = false;

            node->propType = PropertyType::MINE_AMOUNT;
            node->iconstr = help_fetchAgentPropertyValue(agent, PropertyType::MINE_AMOUNT, -1).second;
            node->cost = -1;
        }
    } else if (agent->type == DDAgent::AT_3RD_STONE || agent->type == DDAgent::AT_3RD_TREE) {
        _tappingEnable = true;
        // 移除
        // 变卖
        _removeNode.enable = true;
        _removeNode.isShowCost = true;
        _removeNode.isCoste = true;
        _removeNode.clickable = true;
        _removeNode.propType = PropertyType::REMOVE_3RD;
        _removeNode.iconstr = TRLocale::s()["remove_3rd"];
        _removeNode.cost = DDUpgradeRoadMap::fetchRemove3RdCost(agent); // TODO 移除价格
    } else if (agent->type == DDAgent::AT_3RD_VOLCANO){
        // 攻击力
        {
            auto node = &_propertyNodes[1];
            node->enable = true;
            node->isShowCost = false;
            node->isCoste = false;
            node->clickable = false;

            node->propType = PropertyType::ATTACK;
            node->iconstr = fmt::sprintf("%d", agent->attack);
            node->cost = -1;
        }
        // 范围
        {
            auto node = &_propertyNodes[2];
            node->enable = true;
            node->isShowCost = false;
            node->isCoste = false;
            node->clickable = false;

            node->propType = PropertyType::ACTION_DISTANCE;
            node->iconstr = fmt::sprintf("%d", agent->actionDistance);
            node->cost = -1;
        }

        // 五行
        _elementTypeNode.enable = true;
        _elementTypeNode.isShowCost = false;
        _elementTypeNode.isCoste = false;
        _elementTypeNode.clickable = false;
        _elementTypeNode.propType = help_elementType2propType(DDElementType::Fire);
        _elementTypeNode.iconstr = help_elementType2iconStr(DDElementType::Fire);
        _elementTypeNode.cost = -1;//-1不显示。

    } else if (agent->type == DDAgent::AT_3RD_WATER) {

        // 治疗
        {
            auto node = &_propertyNodes[1];
            node->enable = true;
            node->isShowCost = false;
            node->isCoste = false;
            node->clickable = false;

            node->propType = PropertyType::CURE;
            node->iconstr = fmt::sprintf("%d", agent->cure);
            node->cost = -1;
        }
        // 范围
        {
            auto node = &_propertyNodes[2];
            node->enable = true;
            node->isShowCost = false;
            node->isCoste = false;
            node->clickable = false;

            node->propType = PropertyType::ACTION_DISTANCE;
            node->iconstr = fmt::sprintf("%d", agent->actionDistance);
            node->cost = -1;
        }

        // 五行
        _elementTypeNode.enable = true;
        _elementTypeNode.isShowCost = false;
        _elementTypeNode.isCoste = false;
        _elementTypeNode.clickable = false;
        _elementTypeNode.propType = help_elementType2propType(DDElementType::Water);
        _elementTypeNode.iconstr = help_elementType2iconStr(DDElementType::Water);
        _elementTypeNode.cost = -1;//-1不显示。
    }
}

//void PropertyField::op_dismissCertainAgent(int aid) // 当前battle field某个agent gone了，通知到此，如果正在显示它，则dismiss，template不受影响.{}

void PropertyField::op_dismissAllAgent() // 当转移minmap，暂停，go，battle选择的agent gone等情况时，如果在显示agent，则dismiss，template不受影响。
{
    if (_showState != SS_AGENT) return;

    _tappingEnable = false;
    help_dismissBaseInfo();
    help_dismissPropNodes();
    _showState = SS_NONE;
}

void PropertyField::op_dismissTemplate()
{
    if (_showState != SS_TEMPLATE) return;

    _tappingEnable = false;
    help_dismissBaseInfo();
    help_dismissPropNodes();
    _showState = SS_NONE;
}

void PropertyField::help_dispBaseInfo(int agentType, int level) // 显示基本信息，头像，名称，描述
{
    auto typestr = agentType2agentTypeString(agentType);
    _headImage->setVisible(true);
    _headImage->setTexture(fmt::sprintf("images/heads/head_%s_m%d.png", typestr, level)); // TODO 不同级别不同的头像

    _titleLabel->setVisible(true);
    _titleLabel->setString(TRLocale::s()[fmt::sprintf("prop_title_%s_m%d", typestr, level)]);

    _describeLabel->setVisible(true);
    _describeLabel->setString(TRLocale::s()[fmt::sprintf("prop_desc_%s_m%d", typestr, level)]);
    // TODO 动画 label
}

void PropertyField::help_dismissBaseInfo()
{
    _headImage->setTexture("images/heads/idle_head.png");
    _titleLabel->setVisible(false);
    _describeLabel->setVisible(false);
}

void PropertyField::help_showPropNodes() // 根据节点配置，尝试显示所有节点。
{
    // 5个基本的序列
    for (int i = 0; i < NUM_PROPERTY_MAX; i++) {
        help_showPropNodes_ast(&_propertyNodes[i]);
    }
    // 2个特别按钮
    help_showPropNodes_ast(&_removeNode);
    help_showPropNodes_ast(&_elementTypeNode);
}

void PropertyField::help_showPropNodes_ast(PropertyNode* propNode)
{
    if (!propNode->enable) {
        return;
    }

    // icon
    int state = propNode->getCostState();
    propNode->image->setTexture(fmt::sprintf("images/property/prop_%s_%d.png", properType2string(propNode->propType), state));
    propNode->image->setVisible(true);

    // num
    propNode->numLabel->setVisible(true);
    propNode->numLabel->setString(propNode->iconstr);

    //cost
    if (state == PropertyNode::PS_NO_GAS) {
        propNode->costLabel->setVisible(false);
    } else {
        propNode->costLabel->setVisible(true);
        propNode->costLabel->setString(propNode->cost == -1 ? "" : fmt::sprintf("%d", propNode->cost));
        propNode->costLabel->setTextColor(state == PropertyNode::PS_GAS_ENABLE ? Color4B::WHITE:Color4B::GRAY);
    }

    // TODO 动画进入
}

void PropertyField::help_refreshPropNodes() // 与showPropNodes()不同，已显示，只是从新从配置到图形，没有动画。
{
    // 5个基本的序列
    for (int i = 0; i < NUM_PROPERTY_MAX; i++) {
        help_refreshPropNodes_ast(&_propertyNodes[i]);
    }
    // 2个特别按钮
    help_refreshPropNodes_ast(&_removeNode);
    help_refreshPropNodes_ast(&_elementTypeNode);
}

void PropertyField::help_refreshPropNodes_ast(PropertyNode* propNode)
{
    if (!propNode->enable) {
        return;
    }

    // icon
    int state = propNode->getCostState();
    propNode->image->setTexture(fmt::sprintf("images/property/prop_%s_%d.png", properType2string(propNode->propType), state));
    propNode->image->setVisible(true);

    // num
    propNode->numLabel->setVisible(true);
    propNode->numLabel->setString(propNode->iconstr);

    //cost
    if (state == PropertyNode::PS_NO_GAS) {
        propNode->costLabel->setVisible(false);
    } else {
        propNode->costLabel->setVisible(true);
        propNode->costLabel->setString(propNode->cost == -1 ? "" : fmt::sprintf("%d", propNode->cost));
        propNode->costLabel->setTextColor(state == PropertyNode::PS_GAS_ENABLE ? Color4B::WHITE:Color4B::GRAY);
    }
}
void PropertyField::help_dismissPropNodes() // 关闭所有节点。
{
    // 5个基本的序列
    for (int i = 0; i < NUM_PROPERTY_MAX; i++) {
        help_dismissPropNodes_ast(&_propertyNodes[i]);
    }
    // 2个特别按钮
    help_dismissPropNodes_ast(&_removeNode);
    help_dismissPropNodes_ast(&_elementTypeNode);

}

void PropertyField::help_dismissPropNodes_ast(PropertyNode* propNode)
{
    propNode->enable = false;
    propNode->isCoste = false;
    propNode->isShowCost = false;
    propNode->iconstr = "";
    propNode->cost = -1;
    propNode->clickable = false;

    propNode->numLabel->setVisible(false);
    propNode->costLabel->setVisible(false);
    propNode->image->setVisible(false);
}

void PropertyField::initTouchThings()
{
    static bool touch_moved = false;
    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event){
        auto point = touch->getLocation();
        auto rect = DDConfig::propertyAreaRect();
        touch_moved = false;
        return !DDPropertyFieldProtocal::flagIsTappingExclusive && rect.containsPoint(point) && _tappingEnable && _showState==SS_AGENT;
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event){
        touch_moved = true;
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event){
        auto rect = DDConfig::propertyAreaRect();
        if (rect.containsPoint(touch->getLocation()) && touch_moved == false) {

            auto point = touch->getLocation();
            // 遍历各个propNode

            // 5个常规node
            for (int i= 0; i < NUM_PROPERTY_MAX; i++) {
                auto node = &_propertyNodes[i];
                if (node->enable && node->clickable && help_checkIfPointInNode(node->image, point)) {
                    // 常规属性升级
                    if (node->cost <= DDMapData::s()->_cntGasLeft) {
                        ansClickAgentUpgrade(_battleAgentAid, node->propType);
                    } else {
                        ansClickGasRunOut();
                    }
                    return;
                }
            }

            // 五行
            if (_elementTypeNode.enable && _elementTypeNode.clickable && help_checkIfPointInNode(_elementTypeNode.image, point)) {
                ansClickElementIcon(_battleAgentAid, _elementTypeNode.propType);
                return;
            }

            // 移除按钮
            if (_removeNode.enable && _removeNode.clickable && help_checkIfPointInNode(_removeNode.image, point)) {
                ansClickRemoveIcon(_battleAgentAid, _removeNode.propType);
                return;
            }
        }
    };

    listener->onTouchCancelled = [this](Touch* touch, Event* event){
    };

    _propertyLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _propertyLayer);
}

bool PropertyField::help_checkIfPointInNode(cocos2d::Sprite* node, const cocos2d::Vec2& point)
{
    Vec2 center = _propertyLayer->getPosition() + node->getPosition();
    cocos2d::Size size = node->getContentSize()*node->getScale();
    Rect rect = {center.x - size.width/2, center.y - size.height/2, size.width, size.height};
    return rect.containsPoint(point);
}



void PropertyField::ansClickAgentUpgrade(int aid, PropertyType propType)
{
    CCLOG("upg");
    // 通知给MapData升级，并扣除资源量，同时通知Battle,bigmap,property，各方面更新和表现。
    DDMapData::s()->agentUpgrade(aid, propType);
}

void PropertyField::ansClickElementIcon(int aid, PropertyType propType)
{
    CCLOG("elet");
    showFiveSelection();
}

void PropertyField::ansClickRemoveIcon(int aid, PropertyType propType)
{
    CCLOG("remv");
    if (_removeNodeConfirmCover->isVisible()) {
        DDMapData::s()->agentRemove(aid, propType);
    } else {
        _removeNodeConfirmCover->runAction(Sequence::create(Show::create(),DelayTime::create(1.f),Hide::create(), NULL));
    }
}

void PropertyField::ansClickGasRunOut()//气不够，又点升级时，通知消息，气不够，以及让加号晃动，加红点等。TODO 在bigmap加个统一接口，当请求但资源不够时，给出提示，及促销等动作！！
{

}

void PropertyField::initFiveSelectionThings()
{
    _fiveSelectionNodes[0].elementType = DDElementType::Metal;
    _fiveSelectionNodes[0].relativePos = {0.32f, 0.35f};
    _fiveSelectionNodes[0].file = "metal";

    _fiveSelectionNodes[1].elementType = DDElementType::Wood;
    _fiveSelectionNodes[1].relativePos = {0.5f, 0.65f};
    _fiveSelectionNodes[1].file = "wood";

    _fiveSelectionNodes[2].elementType = DDElementType::Water;
    _fiveSelectionNodes[2].relativePos = {0.22f, 0.55f};
    _fiveSelectionNodes[2].file = "water";

    _fiveSelectionNodes[3].elementType = DDElementType::Fire;
    _fiveSelectionNodes[3].relativePos = {0.78f, 0.55f};
    _fiveSelectionNodes[3].file = "fire";

    _fiveSelectionNodes[4].elementType = DDElementType::Earth;
    _fiveSelectionNodes[4].relativePos = {0.68f, 0.35f};
    _fiveSelectionNodes[4].file = "earth";

    auto size = DDConfig::getWinSize();

    for (int i = 0; i < 5; i++) {
        auto image = Sprite::create(fmt::sprintf("images/five/%s_0.png", _fiveSelectionNodes[i].file));
        image->setScale(DDConfig::relativeScaler(0.5f));
        image->setZOrder(Z_FIVE_NODE);
        image->setVisible(false);
        _propertyLayer->addChild(image);
        image->setPosition({size.width * _fiveSelectionNodes[i].relativePos.x, size.height * _fiveSelectionNodes[i].relativePos.y});
        _fiveSelectionNodes[i].image = image;
    }

    _fiveSelectionRelation = Sprite::create("images/five/relation.png");
    _fiveSelectionRelation->setScale(DDConfig::relativeScaler(1.f));
    _fiveSelectionRelation->setZOrder(Z_FIVE_RELATION);
    _fiveSelectionRelation->setVisible(false);
    _fiveSelectionRelation->setPosition({size.width*0.5f, size.height*0.5f});
    _propertyLayer->addChild(_fiveSelectionRelation);

    static bool touch_moved = false;
    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event){
        touch_moved = false;
        return flagIsTappingExclusive;
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event){
        touch_moved = true;
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event){
        if (!touch_moved) {
            for (int i = 0; i < 5; i++) {
                if (help_checkIfPointInNode(_fiveSelectionNodes[i].image, touch->getLocation())) {
                    // 选了
                    if (DDMapData::s()->_cntGasLeft < DDUpgradeRoadMap::fetchElementCost()) {
                        ansClickGasRunOut();
                    } else {
                        DDMapData::s()->agentSetElementType(_battleAgentAid, _fiveSelectionNodes[i].elementType);
                        dissmissFiveSelection(true);
                    }
                    return;
                }
            }
            dissmissFiveSelection(false);
        }

    };

    listener->onTouchCancelled = [this](Touch* touch, Event* event){
    };

    _propertyLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _propertyLayer);
}

void PropertyField::showFiveSelection() //打开五行选择面板
{
    flagIsTappingExclusive = true;
    auto size = DDConfig::getWinSize();
    auto center = Vec2{size.width/2, size.height/2};
    for (int i = 0; i < 5; i++) {
        _fiveSelectionNodes[i].image->cocos2d::Node::setPosition(center);
        _fiveSelectionNodes[i].image->setOpacity(0);
        _fiveSelectionNodes[i].image->runAction(Sequence::create(Show::create(), FadeIn::create(0.2), MoveTo::create(0.3, {size.width*_fiveSelectionNodes[i].relativePos.x, size.height*_fiveSelectionNodes[i].relativePos.y}), NULL));
    }
    _fiveSelectionRelation->setVisible(true);
    _fiveSelectionRelation->setOpacity(0);
    _fiveSelectionRelation->runAction(Sequence::create(DelayTime::create(0.4), FadeIn::create(0.3), NULL));
}

void PropertyField::dissmissFiveSelection(bool delayed) //关闭五行选择
{

    auto size = DDConfig::getWinSize();
    auto center = Vec2{size.width/2, size.height/2};
    flagIsTappingExclusive = false;
    _fiveSelectionRelation->runAction(Sequence::create(DelayTime::create(delayed?0.3:0.f), FadeOut::create(0.3), Hide::create(), NULL));
    for (int i = 0; i < 5; i++) {
        _fiveSelectionNodes[i].image->runAction(Sequence::create(DelayTime::create(delayed?0.3:0.f), DelayTime::create(0.2), MoveTo::create(0.3, center), FadeOut::create(0.2), NULL));
    }
}
