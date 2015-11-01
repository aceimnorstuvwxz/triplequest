// (C) 2015 Arisecbf


#include "StealthTestScene.h"
#include "format.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

bool StealthTestScene::init()
{
    assert(TRBaseScene::init());

    addCommonBtn({0.5,0.9}, "t", [this](){_simplePixelNode->_stencil = !_simplePixelNode->_stencil;});

    initMainLayer();
    return true;
}

void StealthTestScene::initMainLayer()
{
    auto layer = Layer::create();
    this->addChild(layer);

    auto size = Director::getInstance()->getVisibleSize();

    auto camera = Camera::createPerspective(60, size.width/size.height, 0.1, 1000000);
    camera->setPosition3D({0,0,200});
    camera->lookAt({0,0,0});
    camera->setCameraFlag(CameraFlag::USER1);
    layer->addChild(camera);
    camera->setCameraMask((unsigned short)CameraFlag::USER1);

    _mainLayer = layer;
    _mainCamera = camera;

    //test 3d obj

//    auto sp = Sprite3D::create("3d/shieldx.c3b");
//    sp->setPosition3D({0,0,0});
//    _mainLayer->addChild(sp);
//    sp->setCameraMask(_mainCamera->getCameraMask());
//    sp->runAction(RepeatForever::create(RotateBy::create(10, {0,1000,0})));

    // scatpixel test


    auto _bg2 = SimplePixelNode::create();
    _bg2->setPosition3D({0,0,0});
    _bg2->setCameraMask(_mainCamera->getCameraMask());
    _mainLayer->addChild(_bg2);
    _bg2->config(loadScatPixelFile("images/scatpixel/bg.png.sopx"));



    _simplePixelNode = SimplePixelNode::create();
    _simplePixelNode->setPosition3D({0,0,10});
    _simplePixelNode->setCameraMask(_mainCamera->getCameraMask());
    _mainLayer->addChild(_simplePixelNode);
    _simplePixelNode->config(loadScatPixelFile("images/scatpixel/c.png.sopx"));
    _simplePixelNode->_stencil = true;
    _simplePixelNode->runAction(RepeatForever::create(Sequence::create(MoveBy::create(5, {0,50,0}),MoveBy::create(5, {0,-50,0}), NULL)));




    _bg = SimplePixelNode::create();
    _bg->setPosition3D({0,10,10});
    _bg->setCameraMask(_mainCamera->getCameraMask());
    _mainLayer->addChild(_bg);
    _bg->config(loadScatPixelFile("images/scatpixel/bg.png.sopx"));
    _bg->_stenciled = true;
//    _bg->runAction(RepeatForever::create(RotateBy::create(100, {0,1000,0})));


}