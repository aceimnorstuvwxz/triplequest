// (C) 2015 Turnro.com

#include "BattleField.h"
#include "format.h"
#include "DDMapData.h"
#include "TRLocale.h"

USING_NS_CC;

// Floating Effects
/*
 2，floating 具体动画（远程怪物，进程怪物 攻击，我方塔的攻击）
 floating的设定：
 金木水火土
 炮塔cannon tower 实际表现为雷电塔，一股大的雷电会击中主目标，然后从主目标会溅射散出较小的雷电去攻击溅射的目标。攻击速度较慢，攻击范围较大，攻击力一般，群攻。 普通闪电为蓝白色，可以附魔为金
 箭塔arrow tower 实际表现为箭塔，为暗影的小箭，攻击范围一般，攻击力小，攻击速度快。可以附魔为木，土
 魔法塔 magic tower 实际表现为法师塔，为光影的小球，攻击范围一般，攻击力较大，攻击速度慢。可以附魔为水（冰），火
 治疗塔 cure tower 实际表现为治疗塔，为光波，可以附魔为全部5行。

 floating的实现：
 雷电，雷电，可以改变元素色彩和粗细及复杂度（手动实现），有绑定光源。
 暗影的小箭，帧动画贴图+粒子，无绑定光源。
 光影的小球，帧动画贴图+粒子，有绑定光源。
 治疗光波。。。纯粹的粒子实现。
 */

void FloatingEffectManager::init(cocos2d::Layer* layer, LightNodeManager* lightManager)
{
    _layer = layer;
    _lightManager = lightManager;

    _lightningNode = DDLightningNode::create();
    _lightningNode->setPosition({0,0});
    _layer->addChild(_lightningNode);
    _lightningNode->configWidth(1.5f);
    _lightningNode->setZOrder(BattleField::Z_FLOATING_EFFECT_LIGHTNING);

    for (int i = 0; i < NUM_IMGSPS; i++) {
        auto sp = Sprite::create("images/fe_arrow.png");
        sp->setZOrder(BattleField::Z_FLOATING_EFFECT_IMGS);
        _layer->addChild(sp);
        sp->setVisible(false);
        _imgsps.push_back(sp);
    }
}

void FloatingEffectManager::toast(int type, const AgentPos& srcPos, const AgentPos& desPos, DDElementType elementType, int level)
{
    Vec2 fsrc = BattleField::help_agentPos2agentFloatPos(srcPos);
    Vec2 fdes = BattleField::help_agentPos2agentFloatPos(desPos);
    switch (type) {
        case FE_CURE:
            toast_cure(fsrc, fdes, elementType, level);
            break;
        case FE_CANNON_ATTACK:
        case FE_CANNON_ATTACK_SPUTT:
        case FE_CORE_ATTACK:
            toast_lightning(fsrc, fdes, elementType, level);
            break;

        case FE_ARROW_ATTACK:
            toast_arrow(fsrc, fdes, elementType, level);
            break;
            
        case FE_MAGIC_ATTACK:
            toast_magic(fsrc, fdes, elementType, level);
            break;
        case FE_VOLCANO_ATTACK:
            toast_volcano(fsrc, fdes, elementType, level);
            break;
        case FE_ENEMY_FAR_ATTACK:
            toast_enemy_far(fsrc, fdes, elementType, level);
            break;

        default:
            break;
    }
}

static inline float vector2angel(cocos2d::Vec2 v)
{
    if (v.length() == 0) {
        return 0;
    }

    float pir = std::acos(v.x/v.length());
    if (v.y < 0) {
        pir = 2*3.1415926 - pir;
    }
    return  pir*(180.f/3.1415926);
}

void FloatingEffectManager::toast_cure(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level)
{

    const float speed = 300;
    float len = (desPos-srcPos).length();
    if (len == 0) {
        return;
    }
    auto _emitter = ParticleSystemQuad::createWithTotalParticles(500);
    _emitter->setAutoRemoveOnFinish(true);
    _layer->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage("images/test_lizi.png"));
    _emitter->setDuration(len/speed);

    // gravity
    _emitter->setGravity(Vec2::ZERO);

    // angle
    _emitter->setAngle(vector2angel(desPos-srcPos));
    _emitter->setAngleVar(0.5f* std::atan(DDConfig::battleCubeWidth()/len) * 180.f/3.1415926f);

    // speed of particles
    _emitter->setSpeed(speed);
    _emitter->setSpeedVar(0);

    // radial
    _emitter->setRadialAccel(-(speed/(2*len/speed)));
    _emitter->setRadialAccelVar(0);

    // tagential
    _emitter->setTangentialAccel(00);
    _emitter->setTangentialAccelVar(0);

    // emitter position
    //    _emitter->setPosition( Vec2(160,240) );
    _emitter->setPosVar({0,0});

    // life of particles
    _emitter->setLife(2*len/speed);
    _emitter->setLifeVar(0);

    // spin of particles
    _emitter->setStartSpin(720);
    _emitter->setStartSizeVar(360);
    _emitter->setEndSpin(0);
    _emitter->setEndSpinVar(360);

    // color of particles
    Color4F startColor(0.f, 0.5f, 0.f, 0.8f);
    _emitter->setStartColor(startColor);

    Color4F startColorVar(0.f, 0.1f, 0.f, 0.1f);
    _emitter->setStartColorVar(startColorVar);

    Color4F endColor(0.1f, 0.5f, 0.1f, 0.8f);
    _emitter->setEndColor(endColor);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.1f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(3.0f);
    _emitter->setStartSizeVar(1.0f);
    _emitter->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

    // emits per second
    _emitter->setEmissionRate(_emitter->getTotalParticles()/_emitter->getLife());

    // additive
    _emitter->setBlendAdditive(true);

    _emitter->setPosition(srcPos);
}

void FloatingEffectManager::toast_lightning(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level)
{
    // TODO color
    Vec4 col = {0.5,0.5,1.0,1.0};
    int N = random(3,5);
    for (int i = 0; i < N; i++) {
        _lightningNode->configLightning(srcPos, desPos, 0.f, col);
    }
}

cocos2d::Sprite* FloatingEffectManager::borrowImgsps()
{
    auto sp = _imgsps.front();
    _imgsps.pop_front();
    _imgsps.push_back(sp);
    sp->stopAllActions();
    return sp;
}

#define TIME_MOVE_TOAST  0.4
void FloatingEffectManager::toast_arrow(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level)
{
    auto sp = borrowImgsps();
    sp->setTexture("images/fe_arrow.png");
    sp->setRotation(360-vector2angel(desPos-srcPos));
    sp->setPosition(srcPos);
    auto action = Sequence::create(Show::create(), MoveTo::create(TIME_MOVE_TOAST, desPos), Hide::create(), NULL);
    sp->runAction(action);

//    toast_help_tail(srcPos, desPos, Color4F::GRAY);
}

void FloatingEffectManager::toast_magic(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level)
{
    auto sp = borrowImgsps();
    sp->setTexture("images/fe_magic.png");
    sp->setRotation(360-vector2angel(desPos-srcPos));
    sp->setPosition(srcPos);
    auto action = Sequence::create(Show::create(), MoveTo::create(TIME_MOVE_TOAST, desPos), Hide::create(), NULL);
    sp->runAction(action);
}
void FloatingEffectManager::toast_enemy_far(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level)
{
    auto sp = borrowImgsps();
    sp->setTexture("images/fe_enemy_far.png");
    sp->setRotation(360-vector2angel(desPos-srcPos));
    sp->setPosition(srcPos);
    auto action = Sequence::create(Show::create(), MoveTo::create(TIME_MOVE_TOAST, desPos), Hide::create(), NULL);
    sp->runAction(action);
}
void FloatingEffectManager::toast_volcano(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, DDElementType elementType, int level)
{
    auto sp = borrowImgsps();
    sp->setTexture("images/fe_volcano.png");
    sp->setRotation(360-vector2angel(desPos-srcPos));
    sp->setPosition(srcPos);
    auto action = Sequence::create(Show::create(), MoveTo::create(TIME_MOVE_TOAST, desPos), Hide::create(), NULL);
    sp->runAction(action);
}

void FloatingEffectManager::step(float dt)
{
    _lightningNode->step(dt);
}

void FloatingEffectManager::toast_help_tail(const cocos2d::Vec2& srcPos, const cocos2d::Vec2& desPos, const cocos2d::Color4F& color)
{
    const float time = 0.2;
    float len = (desPos-srcPos).length();
    if (len == 0) {
        return;
    }

    const float speed = len/time;
    auto _emitter = ParticleSystemQuad::createWithTotalParticles(500);
    _emitter->setAutoRemoveOnFinish(true);
    _layer->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage("images/test_lizi.png"));
    _emitter->setDuration(time*5);

    // gravity
    _emitter->setGravity(Vec2::ZERO);

    // angle
    _emitter->setAngle(vector2angel(srcPos-desPos));
    _emitter->setAngleVar(3);

    // speed of particles
    _emitter->setSpeed(speed/5);
    _emitter->setSpeedVar(0);

    // radial
    _emitter->setRadialAccel(0);
    _emitter->setRadialAccelVar(0);

    // tagential
    _emitter->setTangentialAccel(0);
    _emitter->setTangentialAccelVar(0);

    // emitter position
    //    _emitter->setPosition( Vec2(160,240) );
    _emitter->setPosVar({0,0});

    // life of particles
    _emitter->setLife(0.2);
    _emitter->setLifeVar(0);

    // spin of particles
    _emitter->setStartSpin(720);
    _emitter->setStartSizeVar(360);
    _emitter->setEndSpin(0);
    _emitter->setEndSpinVar(360);

    // color of particles
    _emitter->setStartColor(color);

    Color4F startColorVar(0.f, 0.1f, 0.f, 0.1f);
    _emitter->setStartColorVar(startColorVar);

    _emitter->setEndColor(color);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.1f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(2.0f);
    _emitter->setStartSizeVar(1.0f);
    _emitter->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

    // emits per second
    _emitter->setEmissionRate(_emitter->getTotalParticles()/_emitter->getLife());

    // additive
    _emitter->setBlendAdditive(true);
    
    _emitter->setPosition(srcPos);
    _emitter->runAction(MoveTo::create(0.1, desPos));
}
