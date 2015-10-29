// (C) 2015 Turnro.com

#include "DDRectEffectNode.h"
#include "BattleField.h"
#include "DDConfig.h"

USING_NS_CC;

bool DDRectEffectNode::init()
{
    assert(Node::init());

    if (_programState == nullptr) {
        prepareVertexData();
        prepareShaders();
    }

    return true;
}



void DDRectEffectNode::prepareVertexData()
{

    _count = 0;
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        glGenVertexArrays(1, &_vao);
        GL::bindVAO(_vao);
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DDRectEffectNodeVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DDRectEffectNodeVertexPormat), (GLvoid *)offsetof(DDRectEffectNodeVertexPormat, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DDRectEffectNodeVertexPormat), (GLvoid *)offsetof(DDRectEffectNodeVertexPormat, color));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(DDRectEffectNodeVertexPormat), (GLvoid *)offsetof(DDRectEffectNodeVertexPormat, expandConfig));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(DDRectEffectNodeVertexPormat), (GLvoid *)offsetof(DDRectEffectNodeVertexPormat, radioConfig));

        GL::bindVAO(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }
    else
    {
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DDRectEffectNodeVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    CHECK_GL_ERROR_DEBUG();
}

void DDRectEffectNode::prepareShaders()
{
    auto fileUtils = FileUtils::getInstance();
    auto vertSource = fileUtils->getStringFromFile("3d/rect_effect.vsh");
    auto fragSource = fileUtils->getStringFromFile("3d/rect_effect.fsh");

    auto glprogram = GLProgram::createWithByteArrays(vertSource.c_str(), fragSource.c_str());
    glprogram->bindAttribLocation("a_positioin", 0);
    glprogram->bindAttribLocation("a_color", 1);
    glprogram->bindAttribLocation("a_expand_config", 2);
    glprogram->bindAttribLocation("a_radio_config", 3);


    glprogram->link();
    glprogram->updateUniforms();

    _programState = GLProgramState::getOrCreateWithGLProgram(glprogram);
    _programState->retain();
}



void DDRectEffectNode::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    _command.init(_globalZOrder, transform, flags);
    _command.func = CC_CALLBACK_0(DDRectEffectNode::onDraw, this, transform, flags);
    renderer->addCommand(&_command);
}

void DDRectEffectNode::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{

    auto glProgram = _programState->getGLProgram();
    glProgram->use();

    auto loc = glProgram->getUniformLocation("u_time");
    glProgram->setUniformLocationWith1f(loc, _time);

    glProgram->setUniformsForBuiltins(transform);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    if (_dirty) {
        _dirty = false;
        glBufferData(GL_ARRAY_BUFFER, sizeof(DDRectEffectNodeVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);
    }
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(_vao);
    }
    else
    {
        // TODO
    }
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //    glDisable(GL_BLEND);
    // shadow cover打开depth test同时在fsh中对a为0的进行discard，以保证重合交叠处不会交叠而加深。
    //    glEnable(GL_DEPTH_TEST);
    //    glDepthMask(true);

    glDrawArrays(GL_POINTS, 0, _count);

    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);

    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_count);
    CHECK_GL_ERROR_DEBUG();
}

void DDRectEffectNode::reset()
{
    _count = 0;
}

void DDRectEffectNode::configRectEffect(const cocos2d::Vec4& color, const cocos2d::Vec2& position, float expand, float interval, float delay) // 位置，初始的大小，持续时间, 延迟时间
{
    auto thenode = &_vertexData[_count++];
    thenode->position = position;
    thenode->color = color;
    thenode->expandConfig = {expand, DDConfig::bigmapCubeWidth(), _time + delay, _time + delay + interval};
    float radioEnd = _time + delay +interval*2; // 留存的时间同interval??待实际测试 TODO
    thenode->radioConfig = {_time + delay + interval, radioEnd};
    if (_lastTime < radioEnd ) _lastTime = radioEnd;
    _dirty = true;
}

void DDRectEffectNode::step(float dt)
{
    if (_count) {
        _time += dt;
    }
    if (_time > _lastTime || _count > (NUM_MAX_VERTEXS - 100)) {
        reset();
        _time = 0.f;
    }
}