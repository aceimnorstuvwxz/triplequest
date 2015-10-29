// (C) 2015 Arisecbf

#ifndef DDUpgradeLoad_hpp
#define DDUpgradeLoad_hpp

#include "DDConfig.h"
#include "DDAgent.h"
#include "format.h"

// 怪物，第三方 的属性由 地图数据 离线生成。
// 而我军的属性配置都在这里了。


enum class PropertyType:int
{
    ATTACK = 0,
    CURE = 1,
    CURE_NUM = 2,
    MINE_SPEED = 3,
    ACTION_DISTANCE = 4,
    BLOOD_MAX = 5,
    MINE_AMOUNT = 6,
    BOSS = 7,
    SPUTT_DISTANCE = 8,
    SPUTT_RADIO = 9,
    ELEMENT_NONE = 10,
    ELEMENT_METAL = 11,
    ELEMENT_WOOD = 12,
    ELEMENT_WATER = 13,
    ELEMENT_FIRE = 14,
    ELEMENT_EARTH = 15,
    REMOVE_3RD = 16,
    SELL_FRIEND = 17,
    NONE = 18,
};

inline PropertyType help_elementType2propType(DDElementType elet)
{
    switch (elet) {
        case DDElementType::None:
        return PropertyType::ELEMENT_NONE;

        case DDElementType::Metal:
        return PropertyType::ELEMENT_METAL;

        case DDElementType::Wood:
        return PropertyType::ELEMENT_WOOD;

        case DDElementType::Water:
        return PropertyType::ELEMENT_WATER;

        case DDElementType::Fire:
        return PropertyType::ELEMENT_FIRE;

        case DDElementType::Earth:
        return PropertyType::ELEMENT_EARTH;

        default:
        assert(false);
        break;
    }
}

inline std::pair<int, std::string> help_fetchAgentPropertyValue(DDAgent* agent, PropertyType propType, int max) {
    bool twoval = false;
    int v = 0;
    int v1 = 0;
    switch (propType) {
        case PropertyType::ATTACK:
        v = agent->attack;
        break;

        case PropertyType::CURE:
        v =  agent->cure;
        break;

        case PropertyType::CURE_NUM:
        v =   agent->cureNum;
        break;

        case PropertyType::MINE_SPEED:
        v =   agent->mineSpeed;
        break;

        case PropertyType::ACTION_DISTANCE:
        v =  agent->actionDistance;
        break;

        case PropertyType::BLOOD_MAX:
        twoval = true;
        v1 =  agent->blood;
        v = agent->bloodMax;
        break;

        case PropertyType::MINE_AMOUNT:
        twoval = true;
        v1 =   agent->mineAmount;
        v = agent->mineCapacity;
        break;

        case PropertyType::BOSS:
        v =  agent->isBoss ? 1:0;
        break;

        case PropertyType::SPUTT_DISTANCE:
        v =  agent->cannonSputtDistance;
        break;

        case PropertyType::SPUTT_RADIO:
        v =  agent->cannonSputtDamageRadio;
        break;

        case PropertyType::ELEMENT_NONE:
        v =  agent->elementType == DDElementType::None ? 1:0;
        break;

        case PropertyType::ELEMENT_EARTH:
        v =  agent->elementType == DDElementType::Earth ? 1:0;
        break;

        case PropertyType::ELEMENT_FIRE:
        v =  agent->elementType == DDElementType::Fire ? 1:0;
        break;

        case PropertyType::ELEMENT_METAL:
        v =  agent->elementType == DDElementType::Metal ? 1:0;
        break;

        case PropertyType::ELEMENT_WATER:
        v =  agent->elementType == DDElementType::Water ? 1:0;
        break;

        case PropertyType::ELEMENT_WOOD:
        v =  agent->elementType == DDElementType::Wood ? 1:0;
        break;

        case PropertyType::REMOVE_3RD:
        v =  (agent->type == DDAgent::AT_3RD_STONE || agent->type == DDAgent::AT_3RD_TREE) ? 1:0;
        break;

        case PropertyType::SELL_FRIEND:
        v =  (agent->isFriend() && agent->type != DDAgent::AT_FRIEND_CORE )? 1:0;
        break;

        default:
        assert(false);
        break;
    }
    if (!twoval) {
        if (v == max) {
            return {v, "max"};
        } else {
            return {v, fmt::sprintf("%d", v)};
        }
    } else {
        return {v, fmt::sprintf("%d/%d", v1, v)};
    }
}

inline void help_upgradeAgentProperty(DDAgent* agent, PropertyType propType, int newValue)
{
    switch (propType) {
        case PropertyType::ATTACK:
            agent->attack = newValue;
            break;

        case PropertyType::CURE:
            agent->cure= newValue;
            break;

        case PropertyType::CURE_NUM:
            agent->cureNum= newValue;
            break;

        case PropertyType::MINE_SPEED:
            agent->mineSpeed= newValue;
            break;

        case PropertyType::ACTION_DISTANCE:
            agent->actionDistance= newValue;
            break;

        case PropertyType::BLOOD_MAX:
            agent->blood += newValue-agent->bloodMax; //升级最大血量时，现有的血也会跟着上升
            agent->bloodMax= newValue;
            break;


        case PropertyType::SPUTT_DISTANCE:
            agent->cannonSputtDistance= newValue;
            break;
            
        case PropertyType::SPUTT_RADIO:
            agent->cannonSputtDamageRadio= newValue;
            break;
        default:
            assert(false);
    }

}

namespace std{
        template<>
        class hash<PropertyType> {
            public :
            size_t operator()(const PropertyType &p) const {
                return static_cast<int>(p);
            }
        };
}

// 我军建筑的升级路线图
// friend agents的升级路线图 定义{类型-》可升级属性-》初始等级，最高等级，各等级的升级花费}
struct PropertyUpgradeMap
{
    int start;
    int end; // -1表示无终点
    int cost;
};

struct UpgradeConfig
{
     bool upgradable;
     int cost;
     int valueAfter;
};

class DDUpgradeRoadMap
{
public:
    static std::unordered_map<int, std::unordered_map<PropertyType, PropertyUpgradeMap>> _agentUpgradeMaps;
    static std::unordered_map<int, int> _agentActionPeriodMap;
    static void build();
    //获取某个类型的agent的可以升级的属性列表。
    static std::vector<PropertyType> getUpdradableProperties(int agentType)
    {
        std::vector<PropertyType> ret;
        switch (agentType) {
            case DDAgent::AT_FRIEND_WALL:
                ret = {PropertyType::BLOOD_MAX};
                break;
            case DDAgent::AT_FRIEND_MINER:
                ret = {PropertyType::BLOOD_MAX, PropertyType::MINE_SPEED, PropertyType::ACTION_DISTANCE};
                break;
            case DDAgent::AT_FRIEND_ARROW_TOWER:
            case DDAgent::AT_FRIEND_MAGIC_TOWER:
                ret = {PropertyType::BLOOD_MAX, PropertyType::ATTACK, PropertyType::ACTION_DISTANCE};
                break;
            case DDAgent::AT_FRIEND_CONNON_TOWER:
                ret = {PropertyType::BLOOD_MAX, PropertyType::ATTACK, PropertyType::ACTION_DISTANCE, PropertyType::SPUTT_RADIO, PropertyType::SPUTT_DISTANCE};
                break;
            case DDAgent::AT_FRIEND_CURE_TOWER:
                ret = {PropertyType::BLOOD_MAX, PropertyType::CURE, PropertyType::ACTION_DISTANCE, PropertyType::CURE_NUM};
                break;
            case DDAgent::AT_FRIEND_LIGHT_TOWER:
                ret = {PropertyType::BLOOD_MAX, PropertyType::NONE, PropertyType::ACTION_DISTANCE};
                break;

            default:
                ret = {};
                break;
        }
        return ret;
    }


    static PropertyUpgradeMap& getPropertyUpgradeMap(int agentType, PropertyType propertyType)
    {

        return _agentUpgradeMaps[agentType][propertyType];
    }

    // 产生各种我军的初始状态。
    static DDAgent* generateInitFriendAgents(int agentType);

    // agent类型，属性科目，当前值， 返回是否能够升级，升级的花费，升级后的值
    static  UpgradeConfig fetchUpgradeConfigs(int agentType, PropertyType propType, int currentValue);

    static int fetchRemove3RdCost(DDAgent* agent) {return 500;};
    static int fetchSellFriendGot(DDAgent* agent) {return 100;};
    static int fetchElementCost() {return 300;}
    static int fetchFriendCost(int agenttype) {return 20;}
};


#endif /* DDAgent_hpp */
