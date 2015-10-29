// (C) 2015 Turnro.com

#include "BattleField.h"
#include "DDConfig.h"

USING_NS_CC;

bool ShadowCover::init()
{
    assert(Node::init());

    if (_programState == nullptr) {
        prepareVertexData();
        prepareShaders();
    }

    _textureShadow = Director::getInstance()->getTextureCache()->addImage("images/shadow_cover.png");

    return true;
}

inline int help_agentPos2arrawIndexForShadow(const AgentPos& pos)
{
    int y = pos.y + (DDConfig::MINMAP_EXPAND+1);
    int x = pos.x + (DDConfig::MINMAP_EXPAND+1);
    return (y * (DDConfig::BATTLE_NUM+2) + x );
}

void ShadowCover::prepareVertexData()
{

    for (int i = -(DDConfig::MINMAP_EXPAND+1); i <= (DDConfig::MINMAP_EXPAND+1); i++) {
        for (int j = -(DDConfig::MINMAP_EXPAND+1); j <= (DDConfig::MINMAP_EXPAND+1); j++){
            ShadowCoverVertexFormat* pdata = &_vertexData[help_agentPos2arrawIndexForShadow({i,j})];
            pdata->startPosition = BattleField::help_agentPos2agentFloatPos({i,j});
            pdata->endPosition = pdata->startPosition;
            pdata->startRadio = pdata->endRadio = j == DDConfig::MINMAP_EXPAND+1 ? 0.f : 1.f; // 特别处理，最上方的一条不能挡住bigmap部分所以在图形上不显示。
            pdata->startTime = pdata->endTime = -1.f;
            _currentVisionMap[{i,j}] = false;
        }
    }

    if (Configuration::getInstance()->supportsShareableVAO())
    {
        glGenVertexArrays(1, &_vao);
        GL::bindVAO(_vao);
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(ShadowCoverVertexFormat)*(DDConfig::BATTLE_NUM+2)*(DDConfig::BATTLE_NUM+2), _vertexData, GL_STREAM_DRAW);
        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ShadowCoverVertexFormat), (GLvoid *)offsetof(ShadowCoverVertexFormat, startPosition));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ShadowCoverVertexFormat), (GLvoid *)offsetof(ShadowCoverVertexFormat, endPosition));

        // radio
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ShadowCoverVertexFormat), (GLvoid *)offsetof(ShadowCoverVertexFormat, startRadio));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ShadowCoverVertexFormat), (GLvoid *)offsetof(ShadowCoverVertexFormat, endRadio));

        // time
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ShadowCoverVertexFormat), (GLvoid *)offsetof(ShadowCoverVertexFormat, startTime));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ShadowCoverVertexFormat), (GLvoid *)offsetof(ShadowCoverVertexFormat, endTime));

        GL::bindVAO(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }
    else
    {
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(ShadowCoverVertexFormat)*(DDConfig::BATTLE_NUM+2)*(DDConfig::BATTLE_NUM+2), _vertexData, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    CHECK_GL_ERROR_DEBUG();
}

void ShadowCover::prepareShaders()
{
    auto fileUtils = FileUtils::getInstance();
    auto vertSource = fileUtils->getStringFromFile("3d/shadow_cover.vsh");
    auto fragSource = fileUtils->getStringFromFile("3d/shadow_cover.fsh");

    auto glprogram = GLProgram::createWithByteArrays(vertSource.c_str(), fragSource.c_str());
    glprogram->bindAttribLocation("a_start_positioin", 0);
    glprogram->bindAttribLocation("a_end_position", 1);
    glprogram->bindAttribLocation("a_start_radio", 2);
    glprogram->bindAttribLocation("a_end_radio", 3);
    glprogram->bindAttribLocation("a_start_time", 4);
    glprogram->bindAttribLocation("a_end_time", 5);

    glprogram->link();
    glprogram->updateUniforms();

    _programState = GLProgramState::getOrCreateWithGLProgram(glprogram);
    _programState->retain();
}



void ShadowCover::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    _command.init(_globalZOrder, transform, flags);
    _command.func = CC_CALLBACK_0(ShadowCover::onDraw, this, transform, flags);
    renderer->addCommand(&_command);
}

void ShadowCover::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{

    auto glProgram = _programState->getGLProgram();
    glProgram->use();

    //    auto loc = glProgram->getUniformLocation("u_color");
    //    glProgram->setUniformLocationWith4fv(loc, &_color.r, 1);
    //    loc = glProgram->getUniformLocation("u_factor");
    //    glProgram->setUniformLocationWith1f(loc, _factor);

    //    loc = glProgram->getUniformLocation("u_texture_shadow");
    //    glProgram->setUniformLocationWith1i(loc, 1);
    //    loc = glProgram->getUniformLocation("u_light_color");
    //    glProgram->setUniformLocationWith3fv(loc, &_lightColor.r, 1);
    //    loc = glProgram->getUniformLocation("u_light_direction");
    //    glProgram->setUniformLocationWith3fv(loc, &_lightDirection.x, 1);
    //    loc = glProgram->getUniformLocation("u_speed");
    //    glProgram->setUniformLocationWith1f(loc, _speed*25);
    //    GL::bindTexture2DN(1, _textureShadow->getName());
    auto loc = glProgram->getUniformLocation("u_texture_shadow");
    glProgram->setUniformLocationWith1i(loc, 0);
    GL::bindTexture2DN(0, _textureShadow->getName());

    loc = glProgram->getUniformLocation("u_shadow_time");
    glProgram->setUniformLocationWith1f(loc, _time);

    loc = glProgram->getUniformLocation("u_unit_width");
    glProgram->setUniformLocationWith1f(loc, DDConfig::battleCubeWidth()*1.5);

    /*
    loc = glProgram->getUniformLocation("u_texture_be_attacked");
    glProgram->setUniformLocationWith1i(loc, 2);
    GL::bindTexture2DN(2, _textureBeAttacked->getName());

    loc = glProgram->getUniformLocation("u_texture_be_cured");
    glProgram->setUniformLocationWith1i(loc, 3);
    GL::bindTexture2DN(3, _textureBeCured->getName());

    loc = glProgram->getUniformLocation("u_para_ambient_color");
    glProgram->setUniformLocationWith4fv(loc, &_paraAmbientColor.x, 1);
    loc = glProgram->getUniformLocation("u_para_attacked_cured");
    glProgram->setUniformLocationWith4fv(loc, &_paraAttackedCured.x, 1);

    loc = glProgram->getUniformLocation("u_lights");
    glProgram->setUniformLocationWith4fv(loc, &_lights[0].x, DDConfig::NUM_LIGHT);
    loc = glProgram->getUniformLocation("u_lights_color");
    glProgram->setUniformLocationWith4fv(loc, &_lightsColor[0].x, DDConfig::NUM_LIGHT);

    loc = glProgram->getUniformLocation("u_building_animation_index");
    glProgram->setUniformLocationWith1i(loc, _buildingAnimationIndex);
*/
    glProgram->setUniformsForBuiltins(transform);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    if (_dirty) {
        _dirty = false;
        glBufferData(GL_ARRAY_BUFFER, sizeof(ShadowCoverVertexFormat)*(DDConfig::BATTLE_NUM+2)*(DDConfig::BATTLE_NUM+2), _vertexData, GL_STREAM_DRAW);
    }
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(_vao);
    }
    else
    {
        // TODO
    }
//    glDisable(GL_BLEND);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // shadow cover打开depth test同时在fsh中对a为0的进行discard，以保证重合交叠处不会交叠而加深。
//    glEnable(GL_DEPTH_TEST);
//    glDepthMask(true);

    // TODO 画两次，第一次，较淡的色彩，且灰度，同时position 缩小一些，造成广泛的偏移，来作为迷雾的投影。 第二次，正常的画。


    glDrawArrays(GL_POINTS, 0, (DDConfig::BATTLE_NUM+2)*(DDConfig::BATTLE_NUM+2));

    if (Configuration::getInstance()->supportsShareableVAO())
    {
        GL::bindVAO(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glDisable(GL_DEPTH_TEST);
//    glDepthMask(false);

    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, (DDConfig::BATTLE_NUM+2)*(DDConfig::BATTLE_NUM+2));
    CHECK_GL_ERROR_DEBUG();
}

void ShadowCover::step(float dt)
{
    // 与Flightning效果一样，我们用float来标记时间时，当_time持续走大到一定大小时，它的浮点精度开始变大，而运行一段时候后，vertexData里面的end/start time开始失灵。
    // 所以要防止_time在非工作时间段持续变大。
    if (_time <= _lastTime) {
        _time += dt;
    }
}

inline std::pair<bool, AgentPos> help_findAroundDirection(const std::unordered_map<AgentPos, bool>& versionMap, bool iscover, const AgentPos& mypos)
{
    AgentPos around[4] = {{mypos.x+1, mypos.y},{mypos.x-1, mypos.y},{mypos.x, mypos.y+1},{mypos.x, mypos.y-1}};
    std::vector<AgentPos> founds;
    AgentPos notInThree;
    for (auto pos : around) {
        if (iscover) {
            if (versionMap.at(pos) == false) founds.push_back(pos);
            else notInThree = pos;
        } else {
            if (versionMap.at(pos) == true) founds.push_back(pos);
            else notInThree = pos;
        }
    }
    if (founds.size() == 4) {
        return {false, {0,0}};
    } else if (founds.size() > 0 && founds.size() == 3) {
        int x = mypos.x == notInThree.x ? mypos.x : notInThree.x < mypos.x ? mypos.x+1: mypos.x -1;
        int y = mypos.y == notInThree.y ? mypos.y : notInThree.y < mypos.y ? mypos.y +1:mypos.y -1;
        return {true, {x,y}};
    } else if (founds.size() > 0) {
        return {true, founds[static_cast<int>(rand_0_1()*100)%founds.size()]};
    } else {
        return {false, {0,0}};
    }
}

inline std::vector<AgentPos> help_findAllPositionAtDistance(const std::unordered_map<AgentPos, bool>& oldVersionMap, bool isOpenOld, const std::unordered_map<AgentPos, bool>& newVersionMap, bool isOpenNew, const AgentPos& mypos, int distance)
{
    std::vector<AgentPos> ret;
    for (int dx = -distance; dx <= distance; dx++) {
        int dy = distance - std::abs(dx);
        if (dy != 0) {
            AgentPos a = {mypos.x + dx, mypos.y + dy};
            AgentPos b = {mypos.x + dx, mypos.y - dy};
            if (a.isLegal() && oldVersionMap.at(a) == isOpenOld && newVersionMap.at(a) == isOpenNew)
                ret.push_back(a);
            if (b.isLegal() && oldVersionMap.at(b) == isOpenOld && newVersionMap.at(b) == isOpenNew)
                ret.push_back(b);
        } else {
            AgentPos a = {mypos.x + dx, mypos.y};
            if (a.isLegal() && oldVersionMap.at(a) == isOpenOld && newVersionMap.at(a) == isOpenNew)
                ret.push_back(a);
        }
    }
    return ret;
}

void ShadowCover::config(const std::unordered_map<AgentPos, bool>& newVisionMap, const std::vector<std::tuple<AgentPos, int, bool>>& toCalc) // bool 指is to open
{
    // 与currentVisionMap比对，对vertexData进行重置。
    // 只对需要重新设置的变化的单位进行设置，而不进行全盘的更新。
    // 这保证了原先那些还未完成的动画依旧能够正确执行，而不会瞬间被置为end状态的表现。

    // 遍历，查找不同的节点，对每个不同节点，设置对应的vertexdata，置dirty，更新current vision map。

    // TODO迷雾逐步移动效果。
    _dirty = true;

    std::vector<std::tuple<int, AgentPos, bool>> _execUnits; // bool 指isOpen


    for (auto posDis : toCalc) {
        int turnCount = 0;
        if (std::get<2>(posDis)) {
            for (int dis = 0; dis <= std::get<1>(posDis); dis++) {
                auto founds = help_findAllPositionAtDistance(_currentVisionMap, false, newVisionMap, true, std::get<0>(posDis), dis);
                for (auto toExecPos : founds) {
                    _execUnits.push_back({turnCount, toExecPos, std::get<2>(posDis)});
                }
                if (founds.size() > 0) {
                    turnCount++;
                }
            }
        } else {
            for (int dis = std::get<1>(posDis); dis >= 0; dis--) {
                auto founds = help_findAllPositionAtDistance(_currentVisionMap, true, newVisionMap, false, std::get<0>(posDis), dis);
                for (auto toExecPos : founds) {
                    _execUnits.push_back({turnCount, toExecPos, std::get<2>(posDis)});
                }
                if (founds.size() > 0) {
                    turnCount++;
                }
            }
        }
    }

    std::sort(_execUnits.begin(), _execUnits.end(), [](const std::tuple<int, AgentPos, bool>& a, const std::tuple<int, AgentPos, bool>& b){return std::get<0>(a) < std::get<0>(b);});



//    assert(_execUnits.size());


    const float step_time = 0.1f;
    const float turn_time = 0.1f;

    int turn = 0;
    std::vector<std::tuple<int, AgentPos, bool>> toPushIn;
    for (int i = 0; i < _execUnits.size(); i++) {
        if (turn != std::get<0>(_execUnits[i])) {
            turn = std::get<0>(_execUnits[i]);
            for (auto pi : toPushIn) {
                _currentVisionMap[std::get<1>(pi)] = std::get<2>(pi);
            }
            toPushIn.clear();
        }
        toPushIn.push_back(_execUnits[i]);
        auto res = help_findAroundDirection(_currentVisionMap, std::get<2>(_execUnits[i]), std::get<1>(_execUnits[i]));
        Vec2 myPosition = BattleField::help_agentPos2agentFloatPos(std::get<1>(_execUnits[i]));
        Vec2 taPosition = myPosition;
        if ( res.first ) {
            taPosition = BattleField::help_agentPos2agentFloatPos(res.second);
        }
        auto pdata = &_vertexData[help_agentPos2arrawIndexForShadow(std::get<1>(_execUnits[i]))];
        bool isOpen =std::get<2>(_execUnits[i]);
        pdata->startPosition =  myPosition ;
        pdata->endPosition = myPosition;
        pdata->startRadio = isOpen ? 1.0 :0.0;
        pdata->endRadio = isOpen ? 0.0:1.0;
        pdata->startTime = _time + turn * step_time;
        pdata->endTime = pdata->startTime + turn_time;
        _lastTime = pdata->endTime;

    }
    for (auto pi : toPushIn) {
        _currentVisionMap[std::get<1>(pi)] = std::get<2>(pi);
    }


    for (int y = DDConfig::MINMAP_EXPAND; y >= -DDConfig::MINMAP_EXPAND; y--) {
        std::stringstream ss;
        for (int x = -DDConfig::MINMAP_EXPAND; x <= DDConfig::MINMAP_EXPAND; x++) {
            ss << (_currentVisionMap[{x,y}] ? "V":"_");
        }
        CCLOG("%s", ss.str().c_str());
    }

}