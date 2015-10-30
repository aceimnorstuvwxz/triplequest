// (C) 2015 Arisecbf
#ifndef BeatQuestScene_hpp
#define BeatQuestScene_hpp

#include <stdio.h>

#include "TRBaseScene.h"
#include "format.h"
#include "BeatDef.hpp"
#include "SOCommon.h"

// Beat Quest Gameplay test


struct BeatNode
{
    cocos2d::Sprite* image;
    BeatType type;
    int level; // 等级（血瓶）
    int value; // 数值

    // uniform的度量体系
    float beatSpeed;
    float beatAccerate;
    cocos2d::Vec2 beatPos;
    void update(float dt){
        beatPos.y += beatSpeed * dt;
        image->setPosition(soGenPos({beatPos.x, 0.1f+beatPos.y*0.8f}));
    }
    void gen(cocos2d::Layer* layer) {
        image = cocos2d::Sprite::create(fmt::sprintf("images/beat/icon_%s.png", beattype2string(type)));
        layer->addChild(image);
        image->setScale(8);
    }
    void clear() {
        image->getParent()->removeChild(image);
        image = nullptr;
    }
};

class BeatQuestScene:public TRBaseScene
{
public:
    CREATE_FUNC(BeatQuestScene);
    virtual bool init() override;

protected:
    cocos2d::Sprite* _spCover;
    cocos2d::Label* _lbEnemyStatus;
    cocos2d::Label* _lbFriendStatus;

    int _enemyBlood;
    int _enemyAttack;
    int _enemyShieldCnt;

    int _friendBlood;
    int _friendAttack;
    int _friendShieldCnt;
    int _friendComboCnt;
    void newFriend();
    void newEnemy();
    BeatNode genRandomNode();

    std::list<BeatNode> _runningBeats;

    void update(float dt) override;
};

#endif /* BeatQuestScene_hpp */
