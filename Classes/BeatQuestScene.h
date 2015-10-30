// (C) 2015 Arisecbf
#ifndef BeatQuestScene_hpp
#define BeatQuestScene_hpp

#include <stdio.h>

#include "TRBaseScene.h"
#include "format.h"
#include "BeatDef.hpp"

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
    void newFriend();
    void newEnemy();

    void update(float dt) override;
};

#endif /* BeatQuestScene_hpp */
