// (C) 2015 Turnro.com

#ifndef DDProtocals_hpp
#define DDProtocals_hpp


struct MapPos
{
    int x, y;
    MapPos(int x, int y):x(x),y(y){};
    MapPos(){x = y = 0;};
    float length(){return std::abs(x)+std::abs(y);};//根据地图的特性，并不是sqrt的。
};

bool operator==(const MapPos& r, const MapPos& l);

namespace std{
    template<>
    class hash<MapPos> {
        public :
        size_t operator()(const MapPos &p) const {
            return ((p.y+100) * 1000 + p.x + 100);
        }
    };
}

enum class DDBattleObserveMessage:int
{
    MINE,//矿机挖到了矿（矿机，矿，挖到的矿量）。
    MINE_IDLE,//矿机无矿可挖(矿机）
    MINE_GONE,//矿被挖空后被移除（矿）

    CURE,//治疗(治疗者, 被治疗者，加血量)
//    CURE_SELF,//Agent自我治疗(Agent)

    ATTACK,//攻击 （攻击者，被攻击者, 失血量）
    ATTACK_SPUTT,//攻击(溅射) （溅射中心，被溅射者，失血量）

    ENEMY_IN, //怪物的进入 (怪物)
    ENEMY_OUT, //怪物的出去 (怪物)
    ENEMY_NEW, //母巢产生新敌人 (母巢，新怪物)
    ENEMY_DIE, //敌人死亡 (怪物)
    ENEMY_MOVE, //敌人移动 (怪物)

    FRIEND_UPGRADE, //建筑的升级 (我军)
    FRIEND_NEW, //放入新我军 (新我军, -1)
    FRIEND_DIE, //我军死亡 (我军)
    FRIEND_SELL, //我军被卖 (我军)

    REMOVE_3RD, //第三方被挖走 (第三方)
    FRIEND_ELET, //设置属性
};

inline std::string battleObserveMessage2stringKey(const DDBattleObserveMessage& message)
{
    std::string r;
    switch (message) {
        case DDBattleObserveMessage::MINE:
            r = "MINE";
            break;
        case DDBattleObserveMessage::MINE_IDLE:
            r = "MINE_IDLE";
            break;
        case DDBattleObserveMessage::MINE_GONE:
            r = "MINE_GONE";
            break;
        case DDBattleObserveMessage::CURE:
            r = "CURE";
            break;
        case DDBattleObserveMessage::ATTACK:
            r = "ATTACK";
            break;
        case DDBattleObserveMessage::ATTACK_SPUTT:
            r = "ATTACK_SPUTT";
            break;
        case DDBattleObserveMessage::ENEMY_IN:
            r = "ENEMY_IN";
            break;
        case DDBattleObserveMessage::ENEMY_OUT:
            r = "ENEMY_OUT";
            break;
        case DDBattleObserveMessage::ENEMY_NEW:
            r = "ENEMY_NEW";
            break;
        case DDBattleObserveMessage::ENEMY_DIE:
            r = "ENEMY_DIE";
            break;
        case DDBattleObserveMessage::ENEMY_MOVE:
            r = "ENEMY_MOVE";
            break;
        case DDBattleObserveMessage::FRIEND_UPGRADE:
            r = "FRIEND_UPGRADE";
            break;
        case DDBattleObserveMessage::FRIEND_NEW:
            r = "FRIEND_NEW";
            break;
        case DDBattleObserveMessage::FRIEND_DIE:
            r = "FRIEND_DIE";
            break;
        case DDBattleObserveMessage::FRIEND_SELL:
            r = "FRIEND_SELL";
            break;
        case DDBattleObserveMessage::REMOVE_3RD:
            r = "REMOVE_3RD";
            break;
        case DDBattleObserveMessage::FRIEND_ELET:
            r = "FRIEND_ELET";
            break;


        default:
            assert(false);
            break;
    }
    return r;
}

enum class BigmapMessageType
{
    EXGOOD, // 最好的消息，通关
    MOGOOD, // 极好的消息， 比如占领了
    GOOD, // 较好的消息，比如开局
    INFO, // 普通信息
    BAD, // 较坏的消息，塔被灭
    MOBAD, // 很坏的消息，核心被攻击
    EXBAD, //极坏的消息，核心被灭了


    CORE_ATTACK, //核心受攻击
    CORE_DIE, //核心毁灭
};

struct DDRectEffectConfig
{
    float expand;//大小
    float interval; //速度
    int number; //数量
    float delayStep; //间隔
};

class DDBigmapFieldProtocal
{
public:
    // 让bigmap发某个消息。
    // 这个text始终是字符串索引。
    virtual void op_showMessage(BigmapMessageType messageType, const std::string& text,  bool direct = false, bool rectEffect = false, const MapPos& mappos = {0,0}) = 0;

    // 让Bigmap去选择一个MinMap，继而它会通知battle field来切换到新的。
    virtual void op_selectMapPos(const MapPos& mappos) = 0;

    // 被通知资源变化 由step()来主动轮训更新了，而不是通知
//    virtual void op_tellResourceChange() = 0;

    std::pair<cocos2d::Color4B, cocos2d::Color4B> messageTypeToColor(BigmapMessageType mtp) {
        // TODO different color
        return {cocos2d::Color4B::WHITE, cocos2d::Color4B::GRAY};
    }

    float messageTypeToScaler(BigmapMessageType mtp) {
        // TODO different scaler
        return 1.f;
    }
    DDRectEffectConfig messageTypeToRectConfig(BigmapMessageType messageType) {
        return {500, 0.8, 2, 0.2};
    }

};

class DDBattleFieldProtocal
{
public:
    virtual void op_switch2minmap() = 0; // bigmap 来通知battle field切换minmap!
    virtual void op_mapdata_message(DDBattleObserveMessage message, int aid_src, int aid_des, int data = 0, int data2 = 0) = 0; // mapdata 来通知battle field各种消息！
    virtual void op_newGameProcess() = 0;
    virtual void op_gameOverProcess() = 0;
    virtual void op_dismissSelection() = 0; // 通知取消选择

};

class DDBuildingFieldProtocal
{
public:
    virtual void op_cancelSelection() = 0; // 在其它地方点击后，通知取消BuildingField区域的选择框！
    virtual void op_tellResourceChanged() = 0; // 在资源变化时，告知BuildingField可能需要改变各建筑的是否够资源修建性！
};


class DDPropertyFieldProtocal
{
public:
    static bool flagIsTappingExclusive;
    virtual void op_dispBuildingTemplate(int agentType) = 0; // 显示building区域某个某个选择的信息。
    virtual void op_dispBattlingAgent(int aid) = 0; // 显示当前battle field正在Observed的minmap内的某个agent。
//    virtual void op_dismissCertainAgent(int aid) = 0; // 当前battle field某个agent gone了，通知到此，如果正在显示它，则dismiss，template不受影响.
    virtual void op_dismissAllAgent() = 0; // 当转移minmap，暂停，go等情况时，如果在显示agent，则dismiss，template不受影响。
    virtual void op_dismissTemplate() = 0; // 当很多时候，要取消template的选择。
    virtual void op_refreshWithCosts() = 0; // 当资源变更时，调用到此来更新一些花费的node。
    virtual void op_refreshAgentProperty(int aid) = 0; // 选择的agent属性变更通知到来更新这里的表现。
};



#endif /* DDProtocals_hpp */
