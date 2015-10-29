// (C) 2015 Turnro.com

#include "GameScene.h"
#include "DDConfig.h"
#include "ScatOrcScene.h"
#include "DDMapData.h"


USING_NS_CC;

bool GameScene::init()
{
    assert(TRBaseScene::init());

    initBaseUI();
    initBigmap();
    initBattleField();
    initBuildingField();
    initPropertyField();

    _bigmap.configProtocals(&_battleField);
    DDMapData::s()->configProtocals(&_battleField, &_bigmap);

    DDMapData::s()->isBattleFieldObserving = true;
    _battleField.configProtocals(&_bigmap, &_propertyField, &_buildingField);

    _buildingField.configProtocals(&_propertyField, &_battleField);

    /*
    addCommonBtn({0.5,0.1}, "battle dlg", [this](){
        this->battleDialog("this is message", "action", [](){
            CCLOG("test dlg func");
        });
    });
    addCommonBtn({0.1,0.1}, "reset", [this](){
        Director::getInstance()->replaceScene(ScatOrcScene::create());
    });*/

    scheduleUpdate();

//

    if (DDMapData::s()->hasSavedGame()) {
        CCLOG("has saved game");
        //DDMapData::s()->loadMinmapTemplates();
        //DDMapData::s()->loadPresentingMinmapsFromSavedGame();
//        showOldGameLoadView(); // 进行旧游戏的载入过程。（包括图形和数据）
    } else {
        CCLOG("no saved game, init game");


        _battleField.op_newGameProcess();
        DDMapData::s()->gamingRunning = true;
    }


    return true;
}

void GameScene::initBigmap()
{
    _bigmapLayer = Layer::create();
    this->addChild(_bigmapLayer);
    _bigmapLayer->setPosition(DDConfig::bigmapAreaCenter());
    _bigmapLayer->setZOrder(LZ_BIGMAP);
    _bigmap.init(_bigmapLayer, _generalLayer);
}


void GameScene::initBaseUI()
{
    _generalLayer = Layer::create();
    this->addChild(_generalLayer);
    _generalLayer->setZOrder(LZ_GENERAL);


    auto spBaseCover = Sprite::create("images/cover.png");
    spBaseCover->setPosition(genPos({0.5,0.5}));
    _generalLayer->addChild(spBaseCover);
}

void GameScene::initBuildingField()
{
    _buildingLayer = Layer::create();
    this->addChild(_buildingLayer);
    _buildingLayer->setZOrder(LZ_BUILDING);
    _buildingField.init(_buildingLayer);
}

void GameScene::initPropertyField()
{
    _propertyLayer = Layer::create();
    this->addChild(_propertyLayer);
    _propertyLayer->setZOrder(LZ_PROPERTY);
    _propertyField.init(_propertyLayer);
}

void GameScene::update(float dt)
{
    static int CNT = 0;
    _battleField.step(dt);
    _bigmap.update(dt);
    CNT++;
    if (CNT%10 == 0) {
        _bigmap.step(dt);
    }
    if (CNT%3 == 0 && DDMapData::s()->gamingRunning) {
        DDMapData::s()->gameTick();
    }
}
