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
    _friendComboCnt = 0;
}


void BeatQuestScene::update(float dt)
{
    static float _nextBeatTimeLeft = 0.f;

    //refresh status
    _lbEnemyStatus->setString(fmt::sprintf("%03d  %03d  %03d", _enemyBlood, _enemyAttack, _enemyShieldCnt));

    _lbFriendStatus->setString(fmt::sprintf("%03d  %03d  %03d", _friendBlood, _friendAttack, _friendShieldCnt));

    _nextBeatTimeLeft -= dt;
    if (_nextBeatTimeLeft <= 0) {
        _nextBeatTimeLeft = random(0.3,1.0);
        _runningBeats.push_back(genRandomNode());
    }

    for (auto& node : _runningBeats) {
        node.update(dt);
    }
}

BeatNode BeatQuestScene::genRandomNode()
{
    BeatNode node;
    float ran = rand_0_1();
    if (ran < 0.6) {
        node.type = BeatType::SWORD;
        node.level = 0;
        node.value = 0;
    } else if (ran < 0.8) {
        node.type = BeatType::BLOOD;
        node.level = 0;
        node.value = random(5, 10);
    } else {
        node.type = BeatType::SHIELD;
        node.level = 0;
        node.value = 0;
    }
    node.beatSpeed = -random(1.f/0.8f, 1.f/1.2f);
    node.beatPos = {rand_0_1(), 1.f};
    node.beatAccerate = 0;
    node.gen(_defaultLayer);

    return node;
}