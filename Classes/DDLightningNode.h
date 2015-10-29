// (C) 2015 Turnro.com

#ifndef DDLightningNode_hpp
#define DDLightningNode_hpp

#include "DDAgent.h"

struct DDLightningNodeVertexPormat
{
    cocos2d::Vec2 position;
    cocos2d::Vec4 color;
    float startTime;
    float endTime;
};


class DDLightningNode:public cocos2d::Node
{
public:
    CREATE_FUNC(DDLightningNode);
    virtual bool init() override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;

    void configWidth(float width);
    void reset();
    float configLightning(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, float delay, const cocos2d::Vec4& color);

    void step(float dt);//真实时间，用来驱动动画。

protected:
    void prepareVertexData(); //初始化VAO/VBO
    GLuint _vao; //共享的VAO
    GLuint _vbo; //共享的VBO
    void prepareShaders(); //初始化shader program
    cocos2d::GLProgramState* _programState = nullptr;//共享的PROGRAM
    cocos2d::CustomCommand _command;
    constexpr static int NUM_MAX_VERTEXS = 10000;
    DDLightningNodeVertexPormat _vertexData[NUM_MAX_VERTEXS];
    int _count = 0;

    float _lineWidth = 1;
    float _time;
    cocos2d::Vec4 _color;
    bool _dirty = true;
    int _distance = -1;
    float _lastTime;
};

#endif /* DDLightningNode_hpp */
