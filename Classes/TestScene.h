// (C) 2015 Turnro.com

#ifndef TestScene_hpp
#define TestScene_hpp
#include "DDLightningNode.h"
#include "DDRectEffectNode.h"
#include "TRBaseScene.h"

class TestScene:public TRBaseScene
{
public:
    virtual bool init() override;
    CREATE_FUNC(TestScene);

protected:
    cocos2d::Layer* _layer;
    
    cocos2d::ParticleSystemQuad* _emitter;

    DDLightningNode* _lightningInter;
    DDRectEffectNode* _rectEffectNode;

    void emit();
    void lightn();
    void rect();

    void update(float dt) override;
};

#endif /* TestScene_hpp */
