// (C) 2015 Turnro.com

#include "Bigmap.h"
#include "DDMapData.h"
#include "format.h"
#include "GameScene.h"

USING_NS_CC;

void Bigmap::init(cocos2d::Layer *bigmapLayer, cocos2d::Layer* uiLayer)
{
    _bigmapLayer = bigmapLayer;
    _uiLayer = uiLayer;

    _bigmapBaseImage = Sprite::create(fmt::sprintf("mapdata/%d.png", DDMapData::s()->getTemplateIndex()));
    _bigmapBaseImage->setScale(DDConfig::bigmapCubePixelScaler());
    _bigmapBaseImage->setZOrder(Z_BASE_IMAGE);
    _bigmapLayer->addChild(_bigmapBaseImage);
//    _bigmapBaseImage->setPositionY(1);

    _agentsNode = BigmapAgentsNode::create();
    _agentsNode->setScale(DDConfig::bigmapCubePixelScaler());
    _agentsNode->setZOrder(Z_BIGMAP_NODE);
    _bigmapLayer->addChild(_agentsNode);

    _coverNode = BigmapCoverNode::create();
    _coverNode->setScale(DDConfig::bigmapCubePixelScaler());
    _coverNode->setZOrder(Z_BIGMAP_COVER);
    _bigmapLayer->addChild(_coverNode);

    _centerMinMapPos = {0,0};

    initMessageThings();
    initTouchThings();
    initResourceThings();
}

void Bigmap::configProtocals(DDBattleFieldProtocal *battleProtocal)
{
    _battleProtocal = battleProtocal;
}

void Bigmap::step(float dt)
{
    _agentsNode->configAsDirty();//todo
    _coverNode->configAsDirty();
    tryRefreshResourceCnt();
}

void Bigmap::initTouchThings()
{
    static MapPos move_start_mappos;
    static bool touch_moved;
    _selectionIcon = Sprite::create("images/bigmap_select.png");

    _selectionIcon->setScale(DDConfig::bigmapCubeWidth()/_selectionIcon->getContentSize().width);
    _selectionIcon->setVisible(false);
    _bigmapLayer->addChild(_selectionIcon);
    _selectionIcon->setZOrder(Z_SELECTION_IMAGE);

    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event){
        auto point = touch->getLocation();
        move_start_mappos = _centerMinMapPos;
        touch_moved = false;
        auto rect = DDConfig::bigmapAreaRect();
        return !DDPropertyFieldProtocal::flagIsTappingExclusive && rect.containsPoint(point);
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event){
        touch_moved = true;
        if (DDConfig::bigmapAreaRect().containsPoint(touch->getLocation())) {
            auto delta = touch->getLocation() - touch->getStartLocation();
            int diffx = delta.x/DDConfig::bigmapCubeWidth();
            int diffy = delta.y/DDConfig::bigmapCubeWidth();
            help_bigmapMoveTo({move_start_mappos.x-diffx, move_start_mappos.y-diffy});
        }
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event){
        if (DDConfig::bigmapAreaRect().containsPoint(touch->getLocation()) && touch_moved == false) {
            //点击选择新的Minmap
            auto point = touch->getLocation();
            auto delta = point - DDConfig::bigmapAreaCenter();
            MapPos clickPos = _centerMinMapPos;
            clickPos.x += (std::abs(delta.x)+DDConfig::bigmapCubeWidth()*0.5f)/DDConfig::bigmapCubeWidth() * (delta.x > 0 ? 1:-1);
            clickPos.y += (std::abs(delta.y)+DDConfig::bigmapCubeWidth()*0.5f)/DDConfig::bigmapCubeWidth() * (delta.y > 0 ? 1:-1);
            help_tryToSwitchTo(clickPos, false);
        }
    };

    listener->onTouchCancelled = [this](Touch* touch, Event* event){
    };

    _bigmapLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _bigmapLayer);
}

void Bigmap::help_bigmapMoveTo(const MapPos& mappos)
{
    _centerMinMapPos = mappos;
    CCLOG("center %d %d", _centerMinMapPos.x, _centerMinMapPos.y);
    _bigmapLayer->setPosition(DDConfig::bigmapAreaCenter() + DDConfig::bigmapCubeWidth()*Vec2{static_cast<float>(-mappos.x), -static_cast<float>(mappos.y)});
}

void Bigmap::help_tryToSwitchTo(const MapPos& mappos, bool force)
{
    if (!force) {
        if (_selectedMapPos == mappos) { return;}
    }
    // 检查是否可以选择
    if (DDMapData::s()->getMinMaps().find(mappos) != DDMapData::s()->getMinMaps().end())
    {
        _selectionIcon->setVisible(true);
        _selectionIcon->runAction(MoveTo::create(0.25f, DDConfig::bigmapCubeWidth()*Vec2{static_cast<float>(mappos.x), static_cast<float>(mappos.y)}));

        // battle field和big map要互相照顾好。
        DDMapData::s()->battleFieldObservedMinMap = mappos;
        _battleProtocal->op_switch2minmap();
        _selectedMapPos = mappos;
    }
}

void Bigmap::op_selectMapPos(const MapPos& mappos)
{
    help_tryToSwitchTo(mappos, true);
}


void Bigmap::initResourceThings()
{
    _lbMine = Label::createWithTTF("0000", "fonts/fz.ttf", 27*DDConfig::fontSizeRadio());
    _lbMine->setZOrder(GameScene::Z_BIGMAP_RESOURCE_ABOVE);
    auto size = DDConfig::bigmapAreaRect().size;
    _lbMine->setPosition(_bigmapLayer->getPosition() + Vec2{size.width*0.22f, size.height*-0.39f});
    _uiLayer->addChild(_lbMine);
    _lbMine->setTextColor(Color4B::WHITE);


    _lbGas = Label::createWithTTF("0000", "fonts/fz.ttf", 27*DDConfig::fontSizeRadio());
    _lbGas->setZOrder(GameScene::Z_BIGMAP_RESOURCE_ABOVE);
    _lbGas->setPosition(_bigmapLayer->getPosition() + Vec2{size.width*0.37f, size.height*-0.39f});
    _uiLayer->addChild(_lbGas);
    _lbGas->setTextColor(Color4B::WHITE);

    float shadow_offset = 1*DDConfig::relativeScaler(1.f);

    _lbMineShadow = Label::createWithTTF("0000", "fonts/fz.ttf", 27*DDConfig::fontSizeRadio());
    _lbMineShadow->setZOrder(GameScene::Z_BIGMAP_RESOURCE_SHADOW);
    _lbMineShadow->setPosition(_bigmapLayer->getPosition() + Vec2{size.width*0.22f, size.height*-0.39f + shadow_offset});
    _uiLayer->addChild(_lbMineShadow);
    _lbMineShadow->setTextColor(Color4B::GRAY);


    _lbGasShadow = Label::createWithTTF("0000", "fonts/fz.ttf", 27*DDConfig::fontSizeRadio());
    _lbGasShadow->setZOrder(GameScene::Z_BIGMAP_RESOURCE_SHADOW);
    _lbGasShadow->setPosition(_bigmapLayer->getPosition() + Vec2{size.width*0.37f, size.height*-0.39f + shadow_offset});
    _uiLayer->addChild(_lbGasShadow);
    _lbGasShadow->setTextColor(Color4B::GRAY);
}

void Bigmap::tryRefreshResourceCnt()
{
    _targetMine =  DDMapData::s()->_cntMineLeft;
    _targetGas = DDMapData::s()->_cntGasLeft;
}

void Bigmap::update(float dt) // 与帧率同步的更新器，用来资源字符串动画等
{
    static const int DEFAULT_STEP = 3;
    bool changed = false;
    // mine
    int step = std::max(DEFAULT_STEP, std::abs(_targetMine-_currentMine)/30);
    if (_targetMine > _currentMine) {
        changed = true;
        _currentMine += step;
        if (_currentMine > _targetMine) {
            _currentMine = _targetMine;
        }
    } else if (_targetMine < _currentMine) {
        changed = true;
        _currentMine -= step;
        if (_currentMine < _targetMine) {
            _currentMine = _targetMine;
        }
    }

    // gas
    step = std::max(DEFAULT_STEP, std::abs(_targetGas-_currentGas)/30);

    if (_targetGas > _currentGas) {
        changed = true;
        _currentGas += step;
        if (_currentGas > _targetGas) {
            _currentGas = _targetGas;
        }
    } else if (_targetGas < _currentGas) {
        changed = true;
        _currentGas -= step;
        if (_currentGas < _targetGas) {
            _currentGas = _targetGas;
        }
    }
    if (changed) {
        _lbMine->setString(fmt::sprintf("%04d", _currentMine));
        _lbMineShadow->setString(fmt::sprintf("%04d", _currentMine));
        _lbGas->setString(fmt::sprintf("%04d", _currentGas));
        _lbGasShadow->setString(fmt::sprintf("%04d", _currentGas));
    }

    _rectEffectNode->step(dt);
}
