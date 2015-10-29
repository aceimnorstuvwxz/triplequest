// (C) 2015 Turnro.com

#ifndef BattleField_hpp
#define BattleField_hpp

#include "cocos2d.h"
#include "DDConfig.h"
#include "DDAgent.h"
#include "DDProtocals.h"
#include "DDMapData.h"
#include "DDLightningNode.h"

class LightNode;


inline int help_agentPos2arrawIndex(const AgentPos& pos)
{
    int y = pos.y + DDConfig::BATTLE_NUM/2;
    int x = pos.x + DDConfig::BATTLE_NUM/2;
    return (y * DDConfig::BATTLE_NUM + x );
}

// DistanceBox用来反应action distance，机能范围！
struct DistanceBoxVertexPormat
{
    cocos2d::Vec2 position;
    cocos2d::Vec4 edge; // 上，顺时针
    cocos2d::Vec4 corner; //上右角，顺时针
};

class DistanceBox:public cocos2d::Node
{
public:
    CREATE_FUNC(DistanceBox);
    virtual bool init()override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;

    void configDistance(int distance);
    void configCenter(const AgentPos& center, bool isgood);
    void show();
    void dismiss();
    void step(float dt);//真实时间，用来驱动动画。

protected:
    void prepareVertexData(); //初始化VAO/VBO
    GLuint _vao; //共享的VAO
    GLuint _vbo; //共享的VBO
    void prepareShaders(); //初始化shader program
    cocos2d::GLProgramState* _programState = nullptr;//共享的PROGRAM
    cocos2d::CustomCommand _command;
    constexpr static int NUM_MAX_VERTEXS = DDConfig::BATTLE_NUM*DDConfig::BATTLE_NUM*5; //防止不够
    DistanceBoxVertexPormat _vertexData[NUM_MAX_VERTEXS];
    int _count;
    const cocos2d::Vec4 COLOR_GOOD = {152/255.0, 245/255.0, 255/255.0, 1.0};
    const cocos2d::Vec4 COLOR_BAD = {255/255.0, 38/255.0, 38/255.0, 1.0};

    cocos2d::Vec4 _color;
    bool _dirty = true;
    int _distance = -1;
};
// 红绿层，用来表示那些区域可以放入新建筑
struct RedGreenCoverVertexFormat
{
    cocos2d::Vec2 position;
    float state; // 0 不显示， 1 绿 2 红
};

class RedGreenCover:public cocos2d::Node
{
public:
    enum COVER_STATE
    {
        CS_NONE = 0,
        CS_GREEN = 1,
        CS_RED = 2,
    };
    CREATE_FUNC(RedGreenCover);
    virtual bool init()override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;

    void configCover(const AgentPos& pos, int state);
    void show();
    void dismiss();
    void step(float dt);//真实时间，用来驱动动画。

protected:
    constexpr static float ANI_STEP_PER_SEC = 1.f;
//    void setRadio(const AgentPos& pos, float radio);
//    float getRadio(const AgentPos& pos);
//    constexpr static float COVER_SCALER = 1.25f;//每个cube之间会有重叠
    void prepareVertexData(); //初始化VAO/VBO
    GLuint _vao; //共享的VAO
    GLuint _vbo; //共享的VBO
    void prepareShaders(); //初始化shader program
    cocos2d::GLProgramState* _programState = nullptr;//共享的PROGRAM
    cocos2d::CustomCommand _command;
    RedGreenCoverVertexFormat _vertexData[DDConfig::BATTLE_NUM*DDConfig::BATTLE_NUM];

    bool _dirty = true;
};


// 在视野外区域覆盖的shadow cover，这个是一张图渲染全部区域的。
// 只有一个draw call，但是当视野变动时，每次都要glBufferData传数据到显卡。
// 迷雾，具有扩散/聚集的动画效果。
struct ShadowCoverVertexFormat
{
    cocos2d::Vec2 startPosition;
    cocos2d::Vec2 endPosition;
    float startRadio;
    float endRadio;
    float startTime;
    float endTime;
};

class ShadowCover: public cocos2d::Node
{
public:
    CREATE_FUNC(ShadowCover);
    virtual bool init()override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;


    void config(const std::unordered_map<AgentPos, bool>& newVisionMap, const std::vector<std::tuple<AgentPos, int, bool>>& toCalc); // bool 指is to open


    void step(float dt);//真实时间，用来驱动动画。

protected:
    constexpr static float COVER_SCALER = 1.25f;//每个cube之间会有重叠

    float _lastTime = 0.f;
    void prepareVertexData(); //初始化VAO/VBO
    GLuint _vao; //共享的VAO
    GLuint _vbo; //共享的VBO
    void prepareShaders(); //初始化shader program
    cocos2d::GLProgramState* _programState = nullptr;//共享的PROGRAM
    cocos2d::CustomCommand _command;
    ShadowCoverVertexFormat _vertexData[(DDConfig::BATTLE_NUM+2)*(DDConfig::BATTLE_NUM+2)]; // +2 各边有1个单位的offset迷雾，永远cover!
    cocos2d::Texture2D* _textureShadow = nullptr;
    float _time = 0.f;
    bool _dirty = true;

    std::unordered_map<AgentPos, bool> _currentVisionMap;
};




class BattleNode:public cocos2d::Node
{
public:
    enum ATTRIB_INDEX{
        MY_VERTEX_ATTRIB_POSITION,
        MY_VERTEX_ATTRIB_TEXCOORD
    };
};

// 单元底层（BattleField背景，建筑背景）
class BattleLowNode:public BattleNode
{
public:
    CREATE_FUNC(BattleLowNode);
    virtual bool init()override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;
    void setFieldBg(const std::string& fieldBgFile);
    void setBuildingBg(const std::string& buildingBgFile);


protected:
    static void prepareVertexData(); //初始化VAO/VBO
    static GLuint _vao; //共享的VAO
    static GLuint _vbo; //共享的VBO
    static void prepareShaders(); //初始化shader program
    static cocos2d::GLProgramState* _programState;//共享的PROGRAM
    cocos2d::BlendFunc _blendFunc;
    cocos2d::CustomCommand _command;
    cocos2d::Texture2D* _textureFieldBg = nullptr;
    cocos2d::Texture2D* _textureBuildingBg = nullptr;
};


// 单元中层 （扩展的，阴影，机能范围线，建筑可放性，选中光环，BOSS光环，低血光环）
class BattleMiddleNode:public BattleNode
{
public:
    CREATE_FUNC(BattleMiddleNode);
    virtual bool init()override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;

    void configShadowFile(const std::string& shadowFile);
    void configHeight(float height);
    void configScopeLine(const cocos2d::Vec4& opacitys);//top/right/bottom/left
    void configPlacement(float opacity, bool isOk);
    void configSelection(float opacity);
    void configBoss(float opacity);
    void configLowBlood(float opacity);
    void updateLights(LightNode* lights);

protected:

    static void prepareVertexData(); //初始化VAO/VBO
    static GLuint _vao; //共享的VAO
    static GLuint _vbo; //共享的VBO
    static void prepareShaders(); //初始化shader program
    static cocos2d::GLProgramState* _programState;//共享的PROGRAM
    cocos2d::BlendFunc _blendFunc;
    cocos2d::CustomCommand _command;
    cocos2d::Texture2D* _textureShadow = nullptr;//阴影
    cocos2d::Texture2D* _textureActionScopeLine = nullptr;//机能范围线
    cocos2d::Texture2D* _texturePlacement = nullptr;//建筑可放性（灰度）
    cocos2d::Texture2D* _textureSelection = nullptr;//选中光环
    cocos2d::Texture2D* _textureBOSS = nullptr;//BOSS光环
    cocos2d::Texture2D* _textureLowBlood = nullptr;//低血光环
    cocos2d::Vec4 _paraActionScopeLine = {0,0,0,0};//top,right,below,left
    cocos2d::Vec4 _paraPlacement = {0,0,0,0};//{r,g,b,isshow}
    cocos2d::Vec4 _paraSelectionBossLowBlood = {0,0,0,0};//{isselect,isboss,islowblood, not used}
    cocos2d::Vec4 _lights[DDConfig::NUM_LIGHT];

};

// 单元高层 （普通扩展的，上层建筑，受攻击，受治疗，扩展-方便可以延伸一些光圈等效果）
class BattleHighNode:public BattleNode
{
public:
    CREATE_FUNC(BattleHighNode);
    virtual bool init()override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;

    void configBuildingTexture(const std::string& buildingFile);
    void configBuildingAnimationIndex(int index);
    void configBeAttacked(float opacity);
    void configBeCured(float opacity);
    void configAmbientColor(const cocos2d::Vec4& color);
    void updateLights(LightNode* lights);

protected:

    static void prepareVertexData(); //初始化VAO/VBO
    static GLuint _vao; //共享的VAO
    static GLuint _vbo; //共享的VBO
    static void prepareShaders(); //初始化shader program
    static cocos2d::GLProgramState* _programState;//共享的PROGRAM
    cocos2d::BlendFunc _blendFunc;
    cocos2d::CustomCommand _command;
    
    cocos2d::Texture2D* _textureBuildingTexture = nullptr;//以帧动画形式出现的上层建筑，其中包含每一帧对应的法向贴图。
    cocos2d::Texture2D* _textureBeAttacked = nullptr;//被攻击贴图
    cocos2d::Texture2D* _textureBeCured = nullptr;//被治疗贴图

    int _buildingAnimationIndex = 0;//建筑的帧动画索引
    cocos2d::Vec4 _paraAmbientColor = {1,1,1,0.5};
    cocos2d::Vec4 _paraAttackedCured = {0,0,0,0};
    cocos2d::Vec4 _lights[DDConfig::NUM_LIGHT];// TODO 这个数据变成全局的，而不是对每个node的，因为都一样!
    cocos2d::Vec4 _lightsColor[DDConfig::NUM_LIGHT];//光源颜色
};

// 光源
class LightNode
{
public:
    float quality = 0;//以battle cube为单位的距离表示强度
    cocos2d::Vec2 agentFloatPos = {0,0};//以battleField中心为原点，以battle cube为单位的位置。传给shader时，需要计算从光源到对方点的相对坐标！
    float height;//以battle cube为单位的高度  降低复杂度而不考虑高度，所有光源在一个统一的高度。为了更一致的阴影计算，考虑高度，高度将与quality混合传输。
    cocos2d::Vec4 color = {1,1,1,1};//颜色
    // 因为const mediump int gl_MaxFragmentUniformVectors    >= 16;
    // 所以要节约uniform，一个vec4对应一个光源Vec4{pos.x, pos.y, qualityHeight, encodedColor}
    // qualityHeight, xxx.yyy，xxx表示xx.x的quantity .yyy表示yy.y的高度。
    // 暂时设成10个，GLSL内用uniform array!!
    float _targetQuality = 0;
    void setQuality(float targetQuality, bool isanimation) {
        if (isanimation) {
            _targetQuality = targetQuality;
        } else {
            quality = _targetQuality = targetQuality;
        }
    }
    void step(float dt) {
        const float step = 20.f;
        if (quality < _targetQuality) {
            quality += dt* step;
            if  (quality > _targetQuality) quality = _targetQuality;
        } else if (quality > _targetQuality) {
            quality -= dt*step;
            if (quality < _targetQuality ) quality = _targetQuality;
        }

    }
};

class LightNodeManager
{
public:
    void init();
    LightNode* getLights();
    LightNode* brrow();
    void giveback(LightNode* node);
    void step(float dt) {
        for ( auto& node : _lights) {
            node.step(dt);
        }
    }

protected:
    LightNode _lights[DDConfig::NUM_LIGHT];
    std::vector<LightNode*> _lightSet;
};

enum class DDAnimationType:int
{
    IDLE = 0,
    APPEAR = 1,
    DISAPPEAR = 2,
    DIE = 3,
    ACTION = 4,
    MOVE = 5,
    MAX = 6
};

class DDAnimationManager
{
public:

    // key 是agentType * 1000 + animationType。
    static std::unordered_map<int, std::pair<int, int>> _animationMap;

    // 在初始化游戏时，读取各个类型的动作配置文件。
    static void init();

    // 给出agent类型和animationType，返回该动作的start-end。
    static std::pair<int, int> fetch(int agentType, DDAnimationType animationType);
};


class AgentNode
{
public:
    void init(cocos2d::Layer* battleLayer);

    //会实际改变Node的位置
    void configAgentFloatPos(const cocos2d::Vec2& pos);

    //并不会实际改变位置
    void configAgentPos(const AgentPos& pos);

    //动画的改变实际位置
    void animationAgentFloatPos(const cocos2d::Vec2& moveTo);

    // 更新灯光源
    void updateLights(LightNode* lights);

    // 标记此agent是否active，非active的agent不会显示任何内容，同时anistep也不会被驱动。
    void configActive(bool enable);
    bool isActive();

    // 配置基本数据
    void configAgent(int aid, int agentType, int agentLevel);

    // 根据节点的类型决定，是否要 获得/移出 光源
    void configLight(LightNodeManager* lightManager, bool isadd);

    void refreshLightQuality(int actionDistance);

    // 播放某个动画，当播放完毕后会自动切换会循环的播放idle的状态
    int playAnimation(DDAnimationType type); // 播放某个动画
    void resetAnimation();

    // 驱动动画
    void anistep(float dt);

    AgentPos getAgentPos(){ return _agentPos;}
    int getAid(){ return  _aid;}

    void configSmallNumber(DDAgent* agent);

protected:

    bool _active = false;
    DDAnimationType _currentAnimationType;
    std::pair<int, int> _currentAnimationStartEnd;
    int _currentAnimationDirection = 1;
    int _currentAnimationFrameIndex;
    int nextFrameIndex();

    int _aid;
    int _agentType;
    int _agentLevel;
    bool _boss;
    bool _lowBlood;

    cocos2d::Layer* _battleLayer;
    AgentPos _agentPos;
    cocos2d::Vec2 _agentFloatPos;
    float _agentHeight;
    BattleHighNode* _highNode;
    BattleMiddleNode* _middleNode;
    BattleLowNode* _lowNode;
    LightNode* _lightNode = nullptr;
    cocos2d::Label* _lbAction;
    cocos2d::Label* _lbDistance;
    cocos2d::Label* _lbBlood;
};

class FloatingNumberManager
{
    // 各种机能时，伤害，治疗，挖矿的 数值在agent上方浮动显示。
public:
    enum floatingNumberType
    {
        ATTACK,
        CURE,
        MINE
    };

    void init(cocos2d::Layer* layer);
    void toast(int type, int num, const AgentPos& pos, int state = 1); // state 0 较小 1 中等 2 较大

private:
    constexpr static int NUM_LABELS = 32;
    cocos2d::Layer* _layer;
    std::deque<cocos2d::Label*> _labels;
};


class FloatingEffectManager
{
    // 各种攻击，治疗等的效果实现。
public:
    enum floatingEffectType
    {
        FE_CURE,
        FE_ENEMY_FAR_ATTACK,
        FE_ARROW_ATTACK,
        FE_CANNON_ATTACK,
        FE_CANNON_ATTACK_SPUTT,
        FE_CORE_ATTACK,
        FE_MAGIC_ATTACK,
        FE_MINE,
        FE_NEST_NEW,
        FE_VOLCANO_ATTACK
    };
    void init(cocos2d::Layer* layer, LightNodeManager* lightManager);

    void toast(int type, const AgentPos& srcPos, const AgentPos& desPos, DDElementType elementType, int level);

    void step(float dt);

protected:
    cocos2d::Layer* _layer;
    LightNodeManager* _lightManager;
    DDLightningNode* _lightningNode;

    void toast_cure(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level);
    void toast_lightning(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level);

    constexpr static int NUM_IMGSPS = 64; // 多少个预备的sprite。
    std::deque< cocos2d::Sprite*> _imgsps;
    cocos2d::Sprite* borrowImgsps();
    void toast_help_tail(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, const cocos2d::Color4F& color);
    void toast_arrow(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level);
    void toast_magic(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level);
    void toast_enemy_far(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level);
    void toast_volcano(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level);


};

    class DDBackgroundNode:public cocos2d::Node
    {
    public:
        CREATE_FUNC(DDBackgroundNode);
        virtual bool init()override;
        void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
        void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;
        void configAmbientColor(const cocos2d::Vec4& color);
        void configBackgroundMap(const std::string& file);
        void updateLights(LightNode* lights);

    protected:
        void prepareVertexData(); //初始化VAO/VBO
        GLuint _vao; //共享的VAO
        GLuint _vbo; //共享的VBO
        void prepareShaders(); //初始化shader program
        cocos2d::GLProgramState* _programState;//共享的PROGRAM
        cocos2d::CustomCommand _command;
        cocos2d::Texture2D* _textureBackground = nullptr;//背景的normal MAP
        cocos2d::Vec4 _paraAmbientColor = {0.5,0.5,0.5,0};
        cocos2d::Vec4 _lights[DDConfig::NUM_LIGHT];// TODO 这个数据变成全局的，而不是对每个node的，因为都一样!
        cocos2d::Vec4 _lightsColor[DDConfig::NUM_LIGHT];//光源颜色
    };



class BattleField:public DDBattleFieldProtocal
{
public:
   
    enum ZORDER{
        //先画
        Z_TEST_BG,
        Z_BACKGROUND,
        Z_DOORS,
        Z_LOW_NODE,
        Z_REDGREEN_COVER,
        Z_MIDDLE_NODE,
        Z_SELECTION_ICON,
        Z_HIGH_NODE,
        Z_SMALL_NUMBER,
        Z_FLOATING_EFFECT_IMGS_TAIL,
        Z_FLOATING_EFFECT_IMGS,
        Z_FLOATING_EFFECT_LIGHTNING,
        Z_FLOATING_NUMBER,
        Z_SHADOW_COVER,
        Z_DISTANCE_BOX,
        Z_TEST_LIGHT,
        Z_MOVING_COVER,
        Z_MESSAGE_BACKGROUND,
        Z_MESSAGE_LABEL,

        //后画
    };
    void init(cocos2d::Layer* battleLayer);
    void step(float dt);


    void configProtocals(DDBigmapFieldProtocal* bigmapProtocal, DDPropertyFieldProtocal* propertyProtocal,     DDBuildingFieldProtocal* buildingProtocal) { _bigmapProtocal = bigmapProtocal; _propertyProtocal = propertyProtocal; _buildingProtocal = buildingProtocal;}

    //转场到新的地图，有过场动画。
    virtual void op_switch2minmap() override;// bigmap 来通知battle field切换minmap!

    virtual void op_mapdata_message(DDBattleObserveMessage message, int aid_src, int aid_des, int data, int data2) override; // mapdata 来通知battle field各种消息！

    virtual void op_newGameProcess() override; // 进行新游戏的图形过程。
    virtual void op_gameOverProcess() override; // 进行游戏结束的图形过程。

    void op_dismissSelection() override; // 通知取消选择


    static cocos2d::Vec2 help_battleLayerPos2agentFloatPos(const cocos2d::Vec2& srcPos);
    static cocos2d::Vec2 help_agentPos2agentFloatPos(const AgentPos& agentPos);
    static int help_getWhichAgentTypeMoveIn(int buildingIndex); // 返回building field的序号对应的friend agent的类型。

protected:
    DDPropertyFieldProtocal* _propertyProtocal;
    DDBigmapFieldProtocal* _bigmapProtocal;
    DDBuildingFieldProtocal* _buildingProtocal;
    cocos2d::Layer* _battleLayer;
    LightNodeManager _lightManager;

    LightNode* _doorLightTop;
    LightNode* _doorLightBottom;
    LightNode* _doorLightRight;
    LightNode* _doorLightLeft;


    constexpr static int NUM_TEST_LIGHT = 5;
    BattleMiddleNode* _testMiddleNode;
    BattleHighNode* _testHighNode;
    cocos2d::Sprite* _testLightIcon[NUM_TEST_LIGHT];
    LightNode* _testLight[NUM_TEST_LIGHT];
    int _testMovingIndex = -1;
    void addTestLights();

    AgentNode _rawAgentNodes[DDConfig::BATTLE_NUM * DDConfig::BATTLE_NUM];
    std::unordered_map<AgentPos, AgentNode*> _posAgentNodeMap;
    std::unordered_map<int, AgentNode*> _aidAgentNodeMap;
    ShadowCover* _shadowCover;

    bool _isMovingCovered = true;
//    cocos2d::Sprite* _leftMovingCover[3];
//    cocos2d::Sprite* _rightMovingCover[3];
    std::unordered_map<AgentPos, bool> _allCoverredvisionMap;
    void initMovingCover();
    void movingCoverIn();
    void movingCoverOut();
    bool _hasLastBattleObserving = false;
    MapPos _lastObservingMinMap = {0,0};
    void op_switch2minmap_continue();
    void switchMinMap();//将新的选中的minmap渲染出来


    // background
    DDBackgroundNode* _backgroundNode;


    // 通用对话 （新游戏，游戏结束，游戏通关，激活，blocked）
    cocos2d::Sprite* _messageBackgroundImage;
    cocos2d::Label* _messageLabel;
    bool _waitingToTappingAsActive = false;
    bool _waitingToTappingAsRestEnough = false;
    bool _waitingToTappingAsReturnInvade = false;
    void initTapMessgeThings();
    void showTapMessage(bool isActivable);
    void dismissTapMessage();
    cocos2d::Label* _processLabel;
    cocos2d::Label* _gameOverLabel;
    constexpr static int PROCESS_INDEX_MAX = 100;
    int _newGameProcessIndex;

    // 移入新建筑
    std::pair<bool, AgentPos> help_touchPoint2agentPos(const cocos2d::Vec2& point); // 计算触摸到的battle field内部的坐标
    void initMovingNewBuildThings();

    // 选择
    int _selectedAid; // -1时表示没有选择到
    cocos2d::Sprite* _selectionIcon;
    void initSelectionThings();
    void showSelectionAt(const AgentPos& agentpos);

    // 视野图
    std::unordered_map<AgentPos, bool> _visionMap; //视野地图，当有新建筑，或者建筑毁灭，或者切换到新的minmap，都需要重新计算它，然后据此设置cover。( * 与ShadowCover不合作，各边界均*不*是扩展单位的。）
    bool _needToRefreshCover = false; // 由于在DIE时消息收到时，MAPDATA的数据结构还未改，所以要延时一个节拍计算COVER才正确。
    std::unordered_map<AgentPos, int> _currentVisionCoverMeta; // 当前的视野情况，用它来与最新的视野情况进行对比来给shadow cover提供改动数据。
    void calcVisionMapAndRefreshCover(); //计算视野地图，同时更新cover层。附带的会检查是否攻占成功（即视野全开！）
    void calcVisionMap_ast(const AgentPos& agentpos, int visionDistance);

    // 红绿图
    // 每次要显示这个红绿图的时候会重新计算一边。
    // 如果minmap内agent移动或者生灭的时候，如果红绿图的visible的，那么也要更新。
    std::unordered_map<AgentPos, int> _redGreenMap;
    bool help_isDoorPosition(const AgentPos& agentpos);
    RedGreenCover* _redGreenCover;
    void calcRedGreenCoverAndShow();


    // Distance Box
    DistanceBox* _distanceBox;

    // 浮动数值
    FloatingNumberManager _floatNumberManager;

    // 浮动特效
    FloatingEffectManager _floatEffectManager;

    // 处理消息

    void msg_attack(int aid_src, int aid_des, int data, float data2);
    void msg_attack_sputt(int aid_src, int aid_des, int data, float data2);

    void msg_cure(int aid_src, int aid_des, int data, float data2);
    void msg_mine(int aid_src, int adi_des, int data, float data2);
    void msg_mine_gone(int aid_src, int adi_des, int data, float data2);
    void msg_enemy_in(int aid_src, int adi_des, int data, float data2);
    void msg_enemy_out(int aid_src, int adi_des, int data, float data2);

    void msg_enemy_new(int aid_src, int aid_des, int data, float data2);
    void msg_enemy_move(int aid_src, int aid_des, int data, float data2);
    void msg_enemy_die(int aid_src, int aid_des, int data, float data2);

    void msg_friend_upgrade(int aid_src, int aid_des, int data, float data2);

    void msg_friend_new(int aid_src, int aid_des, int data, float data2);
    void msg_friend_die(int aid_src, int aid_des, int data, float data2);
    void msg_friend_sell(int aid_src, int aid_des, int data, float data2);
    void msg_remove_3rd(int aid_src, int aid_des, int data, float data2);
    void msg_friend_elet(int aid_src, int aid_des, int data, float data2);


    void msg_common_agentsAddMinusMove(); //当agent出现/消失/移动时均要调用！
    void msg_common_friendAddMinus(); //当friend出现/消失时均要调用！
    void msg_common_agentGone(int aid); //当agent消失时调用

};

#endif /* BattleField_hpp */
