// (C) 2015 Turnro.com

#include "SimplePixelNode.h"

USING_NS_CC;

bool SimplePixelNode::init()
{
    assert(Node::init());

    if (_programState == nullptr) {
        prepareVertexData();
        prepareShaders();
    }

    return true;
}



void SimplePixelNode::prepareVertexData()
{

    _count = 0;
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        glGenVertexArrays(1, &_vao);
        GL::bindVAO(_vao);
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(SimplePixelVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);
        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimplePixelVertexPormat), (GLvoid *)offsetof(SimplePixelVertexPormat, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SimplePixelVertexPormat), (GLvoid *)offsetof(SimplePixelVertexPormat, color));

        GL::bindVAO(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }
    else
    {
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(SimplePixelVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    CHECK_GL_ERROR_DEBUG();
}

void SimplePixelNode::prepareShaders()
{
    auto fileUtils = FileUtils::getInstance();
    auto vertSource = fileUtils->getStringFromFile("3d/simple_pixel.vsh");
    auto fragSource = fileUtils->getStringFromFile("3d/simple_pixel.fsh");

    auto glprogram = GLProgram::createWithByteArrays(vertSource.c_str(), fragSource.c_str());
    glprogram->bindAttribLocation("a_positioin", 0);
    glprogram->bindAttribLocation("a_color", 1);

    glprogram->link();
    glprogram->updateUniforms();

    _programState = GLProgramState::getOrCreateWithGLProgram(glprogram);
    _programState->retain();
}



void SimplePixelNode::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    _command.init(_globalZOrder, transform, flags);
    _command.func = CC_CALLBACK_0(SimplePixelNode::onDraw, this, transform, flags);
    renderer->addCommand(&_command);
}

void SimplePixelNode::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{

    auto glProgram = _programState->getGLProgram();
    glProgram->use();

    glProgram->setUniformsForBuiltins(transform);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    if (_dirty) {
        _dirty = false;
        glBufferData(GL_ARRAY_BUFFER, sizeof(SimplePixelVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);
    }
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(_vao);
    }
    else
    {
        // TODO
    }
//    glEnable (GL_BLEND);
//    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    // shadow cover打开depth test同时在fsh中对a为0的进行discard，以保证重合交叠处不会交叠而加深。
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);
    glDrawArrays(GL_TRIANGLES, 0, _count);


    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glEnable (GL_BLEND);

    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_count);
    CHECK_GL_ERROR_DEBUG();
}

void SimplePixelNode::config(const std::vector<PixelUnit>& data)
{
    _dirty = true;
    _count = 0;

    // 8 point
    const float half_step = 0.5f;

    cocos2d::Vec3 cornors[8] = {{1,1,1}, {-1,1,1}, {-1,-1,1}, {1,-1,1}, {1,1,-1}, {-1,1,-1}, {-1,-1,-1}, {1,-1,-1}};

    auto genface = [this, &cornors, half_step](int a, int b, int c, int d, Color3B color, cocos2d::Vec3 relativePos){
        _vertexData[_count].position = cornors[a] * half_step + relativePos;
        _vertexData[_count++].color = {color.r/255.f, color.g/255.f, color.b/255.f};
        _vertexData[_count].position = cornors[b] * half_step + relativePos;

        _vertexData[_count++].color = {color.r/255.f, color.g/255.f, color.b/255.f};
        _vertexData[_count].position = cornors[c]  * half_step + relativePos;

        _vertexData[_count++].color = {color.r/255.f, color.g/255.f, color.b/255.f};

        _vertexData[_count].position = cornors[a] * half_step + relativePos;

        _vertexData[_count++].color = {color.r/255.f, color.g/255.f, color.b/255.f};
        _vertexData[_count].position = cornors[c] * half_step + relativePos;

        _vertexData[_count++].color = {color.r/255.f, color.g/255.f, color.b/255.f};
        _vertexData[_count].position = cornors[d] * half_step + relativePos;

        _vertexData[_count++].color = {color.r/255.f, color.g/255.f, color.b/255.f};
    };

    for (auto pix : data) {
        cocos2d::Vec3 relativePos = {static_cast<float>(pix.pos.x), static_cast<float>(pix.pos.y), 0.f};
        genface(0,1,2,3,pix.color, relativePos);
        genface(4,0,3,7,pix.color, relativePos);
        genface(5,4,7,6,pix.color, relativePos);
        genface(1,5,6,2,pix.color, relativePos);
        genface(4,5,1,0,pix.color, relativePos);
        genface(6,7,3,2,pix.color, relativePos);
    }
}
