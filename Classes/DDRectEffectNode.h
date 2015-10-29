// (C) 2015 Turnro.com

#ifndef DDRectEffectNode_hpp
#define DDRectEffectNode_hpp

#include "DDAgent.h"

struct DDRectEffectNodeVertexPormat
{
    cocos2d::Vec2 position; //中心位置
    cocos2d::Vec4 color;
    cocos2d::Vec4 expandConfig;
    cocos2d::Vec2 radioConfig; // radio是默认的从1-0，所以仅需要start/end time
    /*
    float expandStartValue;
    float expandEndValue;
    float expandStartTime;
    float expandEndTime;
    float radioStartValue;
    float radioEndValue;
    float radioStartTime;
    float radioEndTime;
     */
};


class DDRectEffectNode:public cocos2d::Node
{
public:
    CREATE_FUNC(DDRectEffectNode);
    virtual bool init() override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;

    void reset();
    void configRectEffect(const cocos2d::Vec4& color, const cocos2d::Vec2& position, float expand, float interval, float delay); // 位置，初始的大小，持续时间, 延迟时间
    void step(float dt);//真实时间，用来驱动动画。

protected:
    void prepareVertexData();
    GLuint _vao;
    GLuint _vbo;
    void prepareShaders();
    cocos2d::GLProgramState* _programState = nullptr;
    cocos2d::CustomCommand _command;
    constexpr static int NUM_MAX_VERTEXS = 1000; //因为每个框采用一个vertex，所以需要量很小
    DDRectEffectNodeVertexPormat _vertexData[NUM_MAX_VERTEXS];
    int _count = 0;
    float _time;
    cocos2d::Vec4 _color;
    bool _dirty = true;
    int _distance = -1;
    float _lastTime;
};

#endif /* DDRectEffectNode_hpp */
