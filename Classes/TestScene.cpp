// (C) 2015 Turnro.com

#include "TestScene.h"
#include "GameScene.h"
#include "TRLocale.h"

USING_NS_CC;
bool TestScene::init()
{
    assert(TRBaseScene::init());

    _layer = Layer::create();
    this->addChild(_layer);

    _lightningInter = DDLightningNode::create();
    _lightningInter->setPosition(genPos({0.5,0.5}));
    _defaultLayer->addChild(_lightningInter);
    _lightningInter->configWidth(1.f);

    _rectEffectNode = DDRectEffectNode::create();
    _rectEffectNode->setPosition(genPos({0.5,0.5}));
    _defaultLayer->addChild(_rectEffectNode);

    addCommonBtn({0.5,0.1}, "t", [this](){
//        lightn();
        rect();
//        this->emit();
    });

    scheduleUpdate();
    return true;
}

void TestScene::update(float dt)
{
    static int cnt = 0;
    cnt++;
    if (cnt == 3) {
        cnt = 0;
//        lightn();
        rect();
    }
    _lightningInter->step(dt);
    _rectEffectNode->step(dt);
}

void TestScene::lightn()
{
    Vec2 p = {static_cast<float>(random(-300, 300)),static_cast<float>(random(-500, 500))};
    int N = random(1, 5);
    for (int i = 0; i < N; i++) {
        Vec4 color = {rand_0_1(),rand_0_1(),rand_0_1(),1.0};
        _lightningInter->configLightning({0,0}, p, 0.1f, color);
    }
}

void TestScene::rect()
{
    _rectEffectNode->configRectEffect({rand_0_1(),rand_0_1(),rand_0_1(),1.f}, 500*Vec2{rand_0_1()-0.5f, rand_0_1()-0.5f}, rand_0_1()*300, rand_0_1()*1, 0.3f);
}

void TestScene::emit()
{
    auto _emitter = ParticleSystemQuad::createWithTotalParticles(100);
    addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage("images/test_lizi.png"));
    _emitter->setDuration(1.f);

    // gravity
    _emitter->setGravity(Vec2::ZERO);

    // angle
    _emitter->setAngle(90);
    _emitter->setAngleVar(0);

    // speed of particles
    _emitter->setSpeed(260);
    _emitter->setSpeedVar(0);

    // radial
    _emitter->setRadialAccel(-260);
    _emitter->setRadialAccelVar(0);

    // tagential
    _emitter->setTangentialAccel(00);
    _emitter->setTangentialAccelVar(0);

    // emitter position
    //    _emitter->setPosition( Vec2(160,240) );
    _emitter->setPosVar({2,10});

    // life of particles
    _emitter->setLife(1);
    _emitter->setLifeVar(0);

    // spin of particles
    _emitter->setStartSpin(10000);
    _emitter->setStartSizeVar(360);
    _emitter->setEndSpin(0);
    _emitter->setEndSpinVar(360);

    // color of particles
    Color4F startColor(0.f, 0.5f, 0.f, 1.0f);
    _emitter->setStartColor(startColor);

    Color4F startColorVar(0.f, 0.1f, 0.f, 1.0f);
    _emitter->setStartColorVar(startColorVar);

    Color4F endColor(0.1f, 0.5f, 0.1f, 1.0f);
    _emitter->setEndColor(endColor);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(5.0f);
    _emitter->setStartSizeVar(1.0f);
    _emitter->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

    // emits per second
    _emitter->setEmissionRate(_emitter->getTotalParticles()/_emitter->getLife());

    // additive
    _emitter->setBlendAdditive(true);
    //addChild(_emitter);
    _emitter->setPosition(genPos({0.5,0.5}));
    //    _emitter->runAction(RotateBy::create(10.f, 1000));
}