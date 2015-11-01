// (C) 2015 Turnro.com

#ifndef SimplePixelNode_hpp
#define SimplePixelNode_hpp

#include "cocos2d.h"
#include "ScatPixelDef.hpp"

struct SimplePixelVertexPormat
{
    cocos2d::Vec3 position;
    cocos2d::Vec3 color;
};


class SimplePixelNode:public cocos2d::Node
{
public:
    CREATE_FUNC(SimplePixelNode);
    virtual bool init() override;
    void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
    void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)override;

    void config(const std::vector<PixelUnit>& data);
    bool _stencil = false;
    bool _stenciled = false;

protected:
    // TODO 共享

    void prepareVertexData(); //初始化VAO/VBO
    GLuint _vao; //共享的VAO
    GLuint _vbo; //共享的VBO
    void prepareShaders(); //初始化shader program
    cocos2d::GLProgramState* _programState = nullptr;//共享的PROGRAM
    cocos2d::CustomCommand _command;
//    constexpr static int NUM_MAX_VERTEXS = 1000000;
//    SimplePixelVertexPormat _vertexData[NUM_MAX_VERTEXS];
    int _count = 0;
//    bool _dirty = false;

};

#endif /* SimplePixelNode_hpp */
