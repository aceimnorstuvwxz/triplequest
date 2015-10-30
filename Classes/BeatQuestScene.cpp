// (C) 2015 Arisecbf


#include "BeatQuestScene.h"
#include "format.h"

USING_NS_CC;


bool BeatQuestScene::init()
{
    assert(TRBaseScene::init());


//    addCommonBtn({0.5,0.9}, "t", [](){});

    _spCover = Sprite::create("images/beat/cover.png");
    auto size = Director::getInstance()->getWinSize();
    _spCover->setScale(size.width/_spCover->getContentSize().width      );
    _spCover->setPosition(genPos({0.5,0.5}));
    _defaultLayer->addChild(_spCover);

    _lbEnemyStatus = Label::createWithTTF("", "fonts/pixel.ttf", 80);
    _lbEnemyStatus->setPosition(genPos({0.6, 0.95}));
    _lbEnemyStatus->setTextColor(Color4B::BLACK);
    _defaultLayer->addChild(_lbEnemyStatus);


    _lbFriendStatus = Label::createWithTTF("", "fonts/pixel.ttf", 80);
    _lbFriendStatus->setPosition(genPos({0.6, 0.05}));
    _lbFriendStatus->setTextColor(Color4B::BLACK);
    _defaultLayer->addChild(_lbFriendStatus);

    newFriend();
    newEnemy();

    scheduleUpdate();

    return true;
}

void BeatQuestScene::newEnemy()
{
    _enemyBlood = random(30, 50);
    _enemyAttack = random(5, 10);
    _enemyShieldCnt = 0;
}

void BeatQuestScene::newFriend()
{
    _friendAttack = random(5, 10);
    _friendBlood = 30;
    _friendShieldCnt = 0;
}


void BeatQuestScene::update(float dt)
{

    //refresh status
    _lbEnemyStatus->setString(fmt::sprintf("%03d  %03d  %03d", _enemyBlood, _enemyAttack, _enemyShieldCnt));

    _lbFriendStatus->setString(fmt::sprintf("%03d  %03d  %03d", _friendBlood, _friendAttack, _friendShieldCnt));

}