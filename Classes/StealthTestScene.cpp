// (C) 2015 Arisecbf


#include "StealthTestScene.h"
#include "format.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

bool StealthTestScene::init()
{
    assert(TRBaseScene::init());

    addCommonBtn({0.5,0.9}, "On/Off", [this](){
        _shield->_stencil = !_shield->_stencil;
        _bg->_stenciled = !_bg->_stenciled;
        _bg->setVisible(_bg->_stenciled);
    });

    initMainLayer();
    return true;
}

void StealthTestScene::initMainLayer()
{
    auto layer = Layer::create();
    this->addChild(layer);

    auto size = Director::getInstance()->getVisibleSize();

    auto camera = Camera::createPerspective(60, size.width/size.height, 0.1, 1000000);
    camera->setPosition3D({0,-150,150});
    camera->lookAt({0,0,0});
    camera->setCameraFlag(CameraFlag::USER1);
    layer->addChild(camera);
    camera->setCameraMask((unsigned short)CameraFlag::USER1);

    _mainLayer = layer;
    _mainCamera = camera;


    // object stealth
    _shield = SimplePixelNode::create();
    _shield->setPosition3D({0,-50,5});
    _shield->setCameraMask(_mainCamera->getCameraMask());
    _mainLayer->addChild(_shield);
    _shield->config(loadScatPixelFile("images/scatpixel/c.png.sopx"));
    _shield->_stencil = true;
    _shield->runAction(RepeatForever::create(Sequence::create(MoveBy::create(5, {0,100,0}),MoveBy::create(5, {0,-100,0}), NULL)));

    // bg with offset
    _bg = SimplePixelNode::create();
    _bg->setPosition3D({0,0,5});
    _bg->setCameraMask(_mainCamera->getCameraMask());
    _mainLayer->addChild(_bg);
    _bg->config(loadScatPixelFile("images/scatpixel/bg.png.sopx"));
    _bg->_stenciled = true;

    // real showed bg
    auto _bg2 = SimplePixelNode::create();
    _bg2->setPosition3D({0,0,0});
    _bg2->setCameraMask(_mainCamera->getCameraMask());
    _mainLayer->addChild(_bg2);
    _bg2->config(loadScatPixelFile("images/scatpixel/bg.png.sopx"));


}