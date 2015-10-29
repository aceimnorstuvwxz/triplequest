// (C) 2015 Turnro.com

#ifndef PropertyField_hpp
#define PropertyField_hpp

#include "cocos2d.h"
#include "DDConfig.h"
#include "DDAgent.h"
#include "DDProtocals.h"
#include "DDUpgradeLoad.h"
#include "DDMapData.h"

struct PropertyNode
{

    enum PropState{
        PS_NO_GAS,
        PS_GAS_DISABLE,
        PS_GAS_ENABLE
    };
    cocos2d::Sprite* image;
    cocos2d::Label* numLabel;
    cocos2d::Label* costLabel;

    bool enable = false; //是否显示此位置
    bool isShowCost = false; //是否显示下面的费用
    bool isCoste = false; //下面的费用数字是否要更新，（卖出建筑的价格不需要随资源量而更新，而升级、挖掘、五行选择等需要）
    bool clickable = false; //是否可以点击
    PropertyType propType;
    std::string iconstr;
    int cost;
    int getCostState() {
        int gasLeft = DDMapData::s()->_cntGasLeft;
        return (!isShowCost) ? PS_NO_GAS :
        (gasLeft > cost) ? PS_GAS_ENABLE : PS_GAS_DISABLE;
    }
};

inline std::string properType2string(PropertyType type)
{
    std::string ret;
    switch (type) {
        case PropertyType::ATTACK:
            ret = "ATTACK";
            break;
        case PropertyType::CURE:
            ret = "CURE";
            break;
        case PropertyType::CURE_NUM:
            ret = "CURE_NUM";
            break;
        case PropertyType::MINE_SPEED:
            ret = "MINE_SPEED";
            break;
        case PropertyType::ACTION_DISTANCE:
            ret = "ACTION_DISTANCE";
            break;
        case PropertyType::BLOOD_MAX:
            ret = "BLOOD_MAX";
            break;
        case PropertyType::MINE_AMOUNT:
            ret = "MINE_AMOUNT";
            break;
        case PropertyType::BOSS:
            ret = "BOSS";
            break;
        case PropertyType::SPUTT_DISTANCE:
            ret = "SPUTT_DISTANCE";
            break;
        case PropertyType::SPUTT_RADIO:
            ret = "SPUTT_RADIO";
            break;
        case PropertyType::ELEMENT_NONE:
            ret = "ELEMENT_NONE";
            break;
        case PropertyType::ELEMENT_METAL:
            ret = "ELEMENT_METAL";
            break;
        case PropertyType::ELEMENT_WOOD:
            ret = "ELEMENT_WOOD";
            break;
        case PropertyType::ELEMENT_WATER:
            ret = "ELEMENT_WATER";
            break;
        case PropertyType::ELEMENT_FIRE:
            ret = "ELEMENT_FIRE";
            break;
        case PropertyType::ELEMENT_EARTH:
            ret = "ELEMENT_EARTH";
            break;
        case PropertyType::REMOVE_3RD:
            ret = "REMOVE_3RD";
            break;
        case PropertyType::SELL_FRIEND:
            ret = "SELL_FRIEND";
            break;
        case PropertyType::NONE:
            ret = "NONE";
            break;

        default:
            assert(false);
            break;
    }
    return ret;
}

struct FiveNode
{
    std::string file;
    cocos2d::Vec2 relativePos;
    cocos2d::Sprite* image;
    DDElementType elementType;
};

// 下方头像，属性区域！！
class PropertyField:public DDPropertyFieldProtocal
{
public:
    enum ZORDER{
        //先画
        Z_HEAD_IMAGE,
        Z_TEXT,
        Z_PROPERTY_IMAGE,
        Z_PROPERTY_CONFIRM_IMAGE,
        Z_PROPERTY_NUM,
        Z_PROPERTY_COST,
        Z_FIVE_RELATION,
        Z_FIVE_NODE
        //后画
    };
    enum ShowState{
        SS_TEMPLATE,
        SS_AGENT,
        SS_NONE
    };
    void init(cocos2d::Layer* propertyLayer);
    virtual void op_dispBuildingTemplate(int agentType); // 显示building区域某个某个选择的信息。
    virtual void op_dispBattlingAgent(int aid); // 显示当前battle field正在Observed的minmap内的某个agent。
//    virtual void op_dismissCertainAgent(int aid); // 当前battle field某个agent gone了，通知到此，如果正在显示它，则dismiss，template不受影响.
    virtual void op_dismissAllAgent(); // 当转移minmap，暂停，go等情况时，如果在显示agent，则dismiss，template不受影响。
    virtual void  op_dismissTemplate();
    virtual void op_refreshWithCosts(); // 当资源变更时，调用到此来更新一些花费的node。
    virtual void op_refreshAgentProperty(int aid); // 选择的agent属性变更通知到来更新这里的表现。


protected:
    cocos2d::Layer* _propertyLayer;

    cocos2d::Sprite* _headImage; //头像
    cocos2d::Label* _titleLabel; //主字符串
    cocos2d::Label* _describeLabel; //描述字符串
    constexpr static int NUM_PROPERTY_MAX = 5;
    PropertyNode _propertyNodes[NUM_PROPERTY_MAX]; //属性们
    PropertyNode _removeNode; //移除/出售 按钮
    PropertyNode _elementTypeNode; //element属性按钮
    cocos2d::Sprite* _removeNodeConfirmCover;

    bool _tappingEnable = false; // 属性按钮点击有效性总开关
    int _showState = SS_NONE; // 正在显示的是啥
    int _battleAgentAid;
    void help_dispBaseInfo(int agentType, int level); // 显示基本信息，头像，名称，描述
    void help_dismissBaseInfo();
    void help_calcPropNodesData(DDAgent* agent);
    void help_showPropNodes(); // 根据节点配置，尝试显示所有节点，从不显示到显示，有动画。
    void help_showPropNodes_ast(PropertyNode* propNode);
    void help_refreshPropNodes(); // 与showPropNodes()不同，已显示，只是从新从配置到图形，没有动画。
    void help_refreshPropNodes_ast(PropertyNode* propNode);
    void help_dismissPropNodes(); // 关闭所有节点。
    void help_dismissPropNodes_ast(PropertyNode* propNode);
    void initTouchThings();
    bool help_checkIfPointInNode(cocos2d::Sprite* node, const cocos2d::Vec2& point);
    void ansClickAgentUpgrade(int aid, PropertyType propType);
    void ansClickElementIcon(int aid, PropertyType propType);
    void ansClickRemoveIcon(int aid, PropertyType propType);
    void ansClickGasRunOut(); //气不够，又点升级时，通知消息，气不够，以及让加号晃动，加红点等。TODO 在bigmap加个统一接口，当请求但资源不够时，给出提示，及促销等动作！！

    FiveNode _fiveSelectionNodes[5];
    cocos2d::Sprite* _fiveSelectionRelation;
    void initFiveSelectionThings();
    void showFiveSelection(); //打开五行选择面板
    void dissmissFiveSelection(bool delayed); //关闭五行选择

};

#endif /* PropertyField_hpp */
