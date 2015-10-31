// (C) 2015 Arisecbf


#include "ScatPixelScene.h"
#include "format.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

bool ScatPixelScene::init()
{
    assert(TRBaseScene::init());

    addCommonBtn({0.5,0.9}, "t", [](){});

    initMainLayer();
    return true;
}

void ScatPixelScene::initMainLayer()
{
    auto layer = Layer::create();
    this->addChild(layer);

    auto size = Director::getInstance()->getVisibleSize();

    auto camera = Camera::createPerspective(60, size.width/size.height, 0.1, 1000000);
    camera->setPosition3D({0,0,50});
    camera->lookAt({0,0,0});
    camera->setCameraFlag(CameraFlag::USER1);
    layer->addChild(camera);
    camera->setCameraMask((unsigned short)CameraFlag::USER1);

    _mainLayer = layer;
    _mainCamera = camera;

    //test 3d obj

    auto sp = Sprite3D::create("3d/shieldx.c3b");
    sp->setPosition3D({0,0,0});
    _mainLayer->addChild(sp);
    sp->setCameraMask(_mainCamera->getCameraMask());
}