// (C) 2015 Turnro.com

#include "DDLightningNode.h"
#include "BattleField.h"
#include "DDConfig.h"

USING_NS_CC;

bool DDLightningNode::init()
{
    assert(Node::init());

    if (_programState == nullptr) {
        prepareVertexData();
        prepareShaders();
    }

    return true;
}



void DDLightningNode::prepareVertexData()
{

    _count = 0;
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        glGenVertexArrays(1, &_vao);
        GL::bindVAO(_vao);
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DDLightningNodeVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);

        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DDLightningNodeVertexPormat), (GLvoid *)offsetof(DDLightningNodeVertexPormat, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DDLightningNodeVertexPormat), (GLvoid *)offsetof(DDLightningNodeVertexPormat, color));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(DDLightningNodeVertexPormat), (GLvoid *)offsetof(DDLightningNodeVertexPormat, startTime));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(DDLightningNodeVertexPormat), (GLvoid *)offsetof(DDLightningNodeVertexPormat, endTime));

        GL::bindVAO(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }
    else
    {
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DDLightningNodeVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    CHECK_GL_ERROR_DEBUG();
}

void DDLightningNode::prepareShaders()
{
    auto fileUtils = FileUtils::getInstance();
    auto vertSource = fileUtils->getStringFromFile("3d/lightning.vsh");
    auto fragSource = fileUtils->getStringFromFile("3d/lightning.fsh");

    auto glprogram = GLProgram::createWithByteArrays(vertSource.c_str(), fragSource.c_str());
    glprogram->bindAttribLocation("a_positioin", 0);
    glprogram->bindAttribLocation("a_color", 1);
    glprogram->bindAttribLocation("a_start_time", 2);
    glprogram->bindAttribLocation("a_end_time", 3);


    glprogram->link();
    glprogram->updateUniforms();

    _programState = GLProgramState::getOrCreateWithGLProgram(glprogram);
    _programState->retain();
}



void DDLightningNode::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    _command.init(_globalZOrder, transform, flags);
    _command.func = CC_CALLBACK_0(DDLightningNode::onDraw, this, transform, flags);
    renderer->addCommand(&_command);
}

void DDLightningNode::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{

    auto glProgram = _programState->getGLProgram();
    glProgram->use();

    auto loc = glProgram->getUniformLocation("u_time");
    glProgram->setUniformLocationWith1f(loc, _time);

    glProgram->setUniformsForBuiltins(transform);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    if (_dirty) {
        _dirty = false;
        glBufferData(GL_ARRAY_BUFFER, sizeof(DDLightningNodeVertexPormat)*_count, _vertexData, GL_STREAM_DRAW);
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
    // 画描边
    loc = glProgram->getUniformLocation("u_is_outer");
    glProgram->setUniformLocationWith1f(loc, 1.f);
    glLineWidth(_lineWidth*2);
    glDrawArrays(GL_LINES, 0, _count);
    // 画中间
    glProgram->setUniformLocationWith1f(loc, 0.f);
    glLineWidth(_lineWidth);
    glDrawArrays(GL_LINES, 0, _count);


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

void DDLightningNode::reset()
{
    _count = 0;
}

void DDLightningNode::configWidth(float width)
{
    _lineWidth = width;
}


static inline void configLightning_ast(Vec2* poss, int begin, int last, float vary)
{
    if (std::abs(last-begin) <= 1) {
        return;
    }
    const float varyRadio = 0.4f;
    Vec2 displace = {(rand_0_1()-0.5f)*vary, (rand_0_1()-0.5f)*vary};

    int middle = (last + begin)/2;
    poss[middle] = 0.5f * (poss[begin] + poss[last]) + displace;
    configLightning_ast(poss, begin, middle, varyRadio* vary);
    configLightning_ast(poss, middle, last, varyRadio* vary);
}

float DDLightningNode::configLightning(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, float delay, const cocos2d::Vec4& color)
{
    _dirty = true;
    constexpr static int N = 16+1;
    constexpr static float time_step = 0.0125;
    constexpr static float time_left = 0.2;
    Vec2 computingPositions[N];
    auto beginPos = srcPos;
    computingPositions[0] = beginPos;
    auto endPos = desPos;
    computingPositions[N-1] = endPos;
    float vary = 0.5f* (endPos-beginPos).length();
    configLightning_ast(computingPositions, 0, N-1, vary);

    for (int i = 0, j = 1; j < N; i++,j++) {
        float st =             _time + delay + i * time_step;
        float et =            _time + delay + i * time_step  + time_left  + random(0,20)*time_step;// + (N-i+random(0,20))*time_step;
        if (et > _lastTime) {
            _lastTime = et;
        }

        _vertexData[_count++] = {
            computingPositions[i],
            color,
            st,
            et
        };
        _vertexData[_count++] = {
            computingPositions[j],
            color,
            st,
            et
        };
    }
    return 0.f;
}



void DDLightningNode::step(float dt)
{
    /*
     for (int i = 0; i < DDConfig::BATTLE_NUM; i++) {
     for (int j = 0; j < DDConfig::BATTLE_NUM; j++) {
     float dir =  _cubeAniDir[i*DDConfig::BATTLE_NUM+j];
     if (dir == 0.f) continue;
     AgentPos pos = {j-DDConfig::BATTLE_NUM/2, i-DDConfig::BATTLE_NUM/2};
     float oldRadio = getRadio(pos);
     if (dir < 0 && oldRadio > 0.f){
     float newRadio = std::max(0.f, oldRadio - ANI_STEP_PER_SEC*dt);
     if (newRadio == 0.f) {
     _cubeAniDir[i*DDConfig::BATTLE_NUM+j] = 0;
     }
     setRadio(pos, newRadio);
     } else if (dir >0 && oldRadio < 1.f) {
     float newRadio = std::min(1.f, oldRadio + ANI_STEP_PER_SEC*dt);
     if (newRadio == 1.f) {
     _cubeAniDir[i*DDConfig::BATTLE_NUM+j] = 0;
     }
     setRadio(pos, newRadio);
     }
     }
     }*/
    _time += dt;
    if (_time > _lastTime || _count > (NUM_MAX_VERTEXS - 200)) {
        reset();
        _time = 0.f;
    }
}