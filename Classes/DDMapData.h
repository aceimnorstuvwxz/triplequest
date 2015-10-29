
#ifndef DDMapData_hpp
#define DDMapData_hpp


#include <unordered_map>
#include "DDAgent.h"
#include "DDConfig.h"
#include "DDProtocals.h"
#include "DDUpgradeLoad.h"




class DDMinMap
{
public:
    enum State{
        T_ACTIVABLE,//可被激活
        T_ACTIVE,//活动的
        T_NON_ACTIVE //还不可激活
    };
    static std::shared_ptr<DDMinMap> create();
    static std::shared_ptr<DDMinMap> createMinMap(const MapPos& mappos);
    static std::shared_ptr<DDMinMap> createCoreMinMap();
    virtual ~DDMinMap();
    bool blocked; //被阻止的
    MapPos pos;
    int state;
    int weight = -1; ////距离核心的距离
    DDElementType mainElementType;
    DDElementType secondaryElementType;

    bool isCore() const{return pos.x == 0 && pos.y == 0;}
    bool isPosEmpty(const AgentPos& agentPos) const;
    /*
    std::vector<DDMine*> agentMines;//矿
    std::vector<DDStoneBarrier*> agentStones;//石头
    std::vector<DDTreeBarrier*> agentTrees;//树
    std::vector<DDWaterPoolAgent*> agentWaterPools;//水塘
    std::vector<DDVolcanoAgent*> agentVolcanos;//火山
    std::vector<DDFriendMinerAgent*> agentMiners;//矿厂
    std::vector<DDFriendWallAgent*> agentWalls;//墙
    std::vector<DDFriendLightHouseAgent*> agentLightHouses;//灯塔
    std::vector<DDFriendArrowTowerAgent*> agentArrowTowers;//箭塔
    std::vector<DDFriendCannonTowerAgent*> agentCannonTowers;//炮塔
    std::vector<DDFriendCureTowerAgent*> agentCureTowers;//治疗塔
    DDFriendCoreAgent* agentCore = nullptr;//核心（只有核心才有的建筑）
    std::vector<DDEnemyNestAgent*> agentEnemyNests;//母巢
    std::vector<DDEnemyFarMonsterAgent*> agentFarEnemies;//远程怪
    std::vector<DDEnemyNearMonsterAgent*> agentNearEnemies;//近战怪*/

    // 分别用pos和aid对agents进行索引，这俩个容器的内容是一致的。
    std::unordered_map<AgentPos, DDAgent*> posAgentMap;
    std::unordered_map<int, DDAgent*> aidAgentMap;


    // TreatMap用来为怪物寻路服务，加速！
    std::unordered_map<AgentPos, int> generalTreatMap;
    std::unordered_map<AgentPos, int> wallTreatMap;

protected:
    DDMinMap(){};
};



enum class DDBigmapObserveMessage
{
    MINE_EXAUSTED,//一个矿被挖空
};

// 运行时数据结构
class DDMapData
{
public:
    static DDMapData* s() {return &_instance;};
    std::unordered_map<MapPos, std::shared_ptr<DDMinMap>>& getMinMaps(){return _presentingMinmaps;}
    std::shared_ptr<DDMinMap>& getBattleObservedMinMap() {
        return _presentingMinmaps[battleFieldObservedMinMap];
    }
    static int nextAgentId(){return _agentIdIndex++;};
    int resourceMineAmount;
    int resourceGasAmount;
    
    void store();
    void recover();

    // 保存当前的参数作为本攻略游戏结束的参数。
//    void saveAsGameOverData();

    // 在重新进入地下城之前清空数据。
    void clearAllDataForNewGame();

    // 激活某一个minmap。会计算 导航weight。
    void activeMinMap(const MapPos& mappos);
    // 选择一个随机的minmap templates
    void randomChooseTemplates() { _templateIndex = 0;}//TODO
    // 从模板中读取每个minmap的模板数据
    void loadMinmapTemplates();
    // 从游戏存档中读取游戏数据
    void loadPresentingMinmapsFromSavedGame();
    // 新开局设定已活动minmaps
    void loadPresentingMinmapsForNew();

    // 新建Agent（包括nest产怪，玩家放置友军），都是从调用新放agent接口开始的，由mapdata发消息给battle field进行后续动作。（即，即使是看似由battle field发起的动作，battle field上新出现的agent也是由mapdata发消息给Battle field后，由battle field处理消息才表现出来的）

    // 新产生敌人，根据nest内配置参数，在minmap/agentpos位置产生一个敌人，同时还有消息发送。
    void appearAgent_enemy(const std::shared_ptr<DDMinMap>& minmap, const AgentPos& agentpos, DDAgent* nest, bool battleObserving);

    // 新产生友军建筑，总是在battleObserved的minmap，总是会observing。
    void appearAgent_friend(const AgentPos& agentpos, int friendAgentType);

    // 卖出友军建筑，或者，移出3RD的石头和树，总是在battleObserved的minmap，总是会observing。
    void removeAgent(const AgentPos& agentpos);

    void tellCurrentAllVisionOpen(); // 告知当前的minmap的所有视野开了，需要激活周边。还要通知bigmap。


    bool hasSavedGame(){ return _hasSavedGame;}
    int getTemplateIndex(){ return _templateIndex;}


    bool isBattleFieldObserving = false;
    MapPos battleFieldObservedMinMap = {0,0};
    
    bool isBigmapObserving = false;

    void configProtocals(DDBattleFieldProtocal* battleProtocal, DDBigmapFieldProtocal* bigmapProtocal);

    bool isMinMapActive(const MapPos& mappos);


//    void registerBattleFieldObserver(std::function<void(DDBattleObserveMessage message, int aid_src, int aid_des)> battleFieldObserver);
//    void registerBigmapFieldObserver(std::function<void(DDBigmapObserveMessage message, MapPos mappos)> bigmapFieldObserver);

    void gameTick(); //驱动一个游戏内帧Period。

    constexpr static const int THREAT_FULL_MAP = 40;//给agents用的
    constexpr static const int THREAT_DOOR = 20;//给DOOR用

    bool gamingRunning = false;

    int _cntOccupied = 0;//本次攻略，占领的地下城数目
    int _cntOccupiedMax;//历史上，占领地下城数目的最大值。
    int _cntMineGot;//本次攻略，所攫取的矿物量
    int _cntGasGot;
    int _cntEnemyKilled;//本次攻略，消灭的敌人的量
    int _cntMineLeft;//当前剩余矿物量。
    int _cntGasLeft;//当前剩余气量。


    // 资源变化接口
    void injectResourceMine(int mineDiff, bool ifcnt);
    void injectResourceGas(int gasDiff, bool ifcnt);


    void agentUpgrade(int aid, PropertyType propType); // 升级，总是当前的minmap内的，会自动扣资源，然后通知各方
    void agentRemove(int aid, PropertyType propType); // 移除某个agent，分为挖掉石头/树，和，变卖建筑2种。
    void agentSetElementType(int aid, DDElementType newElementType); // 设置，并扣资源，通知各方。
protected:
    DDBattleFieldProtocal* _battleFieldProtocal;
    DDBigmapFieldProtocal* _bigmapFieldProtocal;
    int _templateIndex;//模板的id，用来支持多个template地图，暂时开发中，只有默认的一个地图。
    bool _hasSavedGame;
    bool _hasFinishTutorial;
    static int _agentIdIndex; //从10000开始，之前的0-10000留给离线生成的agent
    static DDMapData _instance;

    void sendBattleFieldMessage(DDBattleObserveMessage message, int aid_src, int aid_des, int data = 0, int data2 = 0) { _battleFieldProtocal->op_mapdata_message(message, aid_src, aid_des, data, data2);}

//    void sendBigmapFieldMessage(DDBigmapObserveMessage message, MapPos mappos) { _bigmapFieldObserver(message, mappos); }

    void tickStepPerMinMap(const std::shared_ptr<DDMinMap>& minmap);

    void tickStepPerMinMap_waterPool(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);
    void tickStepPerMinMap_volcano(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);
    void tickStepPerMinMap_miner(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);
    void tickStepPerMinMap_arrowMagicTower(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);
    void tickStepPerMinMap_cannonTower(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);
    void tickStepPerMinMap_cureTower(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);
    void tickStepPerMinMap_core(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);
    void tickStepPerMinMap_enemyNest(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);
    void tickStepPerMinMap_enemy(const std::shared_ptr<DDMinMap>& minmap, DDAgent* agent, bool battleObserving);

    void tickStep_help_attack(const std::shared_ptr<DDMinMap>& minmap, DDAgent* src, DDAgent* des, bool battleObserving); // 通用的攻击计算过程
    void tickStep_help_agent_gone(const std::shared_ptr<DDMinMap>& minmap, DDAgent* des); // 将某个agent正确移出，在数据结构上移出，包括释放内存。
    void tickStep_help_enemy_move(const std::shared_ptr<DDMinMap>& minmap, DDAgent* enemy, const AgentPos& moveto, bool battleObserving); // 怪物的通用移动
    void tickStep_help_cure(const std::shared_ptr<DDMinMap>& minmap, DDAgent* src, DDAgent* des, bool battleObserving); // 通用治疗


    void help_computePeriodIndex(DDAgent* activeAgent);
    bool help_isFirstStageAction(DDAgent* activeAgent);
    bool help_isSecondStageAction(DDAgent* activeAgent);
//    bool help_computeSelfCureIndex(DDAgent* activeAgent);
//    void help_computeUnbeatableAmount(DDAgent* activeAgent, bool battleObserving);

    int _unblockedMinmapTotal; // 非blocked的minmap总数(占领通关值）
    std::unordered_map<MapPos, std::shared_ptr<DDMinMap>> _templateMinmaps;

    std::unordered_map<MapPos, std::shared_ptr<DDMinMap>> _presentingMinmaps; // 在表演的minmap们，包括可激活的和活动

    bool help_movefromTemplate2preseting(const MapPos& mappos); //将某个从templates移到preseting中，自动设置成待激活(实际并不会从templates中移走)，核心会设成已激活。
    void help_makeArroundActivable(const MapPos& mappos); //将某个周围的变成待激活

    void help_calcMinMapThreatMapAll(const MapPos& mappos); //为minmap计算TreatMap，当初始化、建筑新灭，矿挖空时需要调用此方法进行完全计算。

    void help_calcMinMapThreatMapSingle(const MapPos& mappos, const AgentPos& agentpos); //当新增建筑时，要为threat map补充这个新建筑的量。错误，并不行！新增建筑也需要全部重算！

    void help_calcMinMapThreatMapSingle_start(std::unordered_map<AgentPos, DDAgent*>* posAgents, std::unordered_map<AgentPos, int>* threatMap, const AgentPos& pos, int threat); //递归的点-降渲染threat map

    void help_calcMinMapThreatMapSingle_ast(std::unordered_map<AgentPos, DDAgent*>* posAgents, std::unordered_map<AgentPos, int>* threatMap, const AgentPos& pos, int threat); //递归的点-降渲染threat map

    void help_clearMinMapThreatMap(std::unordered_map<AgentPos, int>* threatMap); //初始化 或 清0 威胁图
    void help_printMinMapThreatMap(std::unordered_map<AgentPos, int>* threatMap); //打印威胁图

    void help_calcMinMapWeight(const MapPos& mappos); //迭代的计算更新minmap的weight


};



#endif /* DDMapData_hpp */
