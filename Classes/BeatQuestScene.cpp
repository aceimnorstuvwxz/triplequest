// (C) 2015 Arisecbf


#include "BeatQuestScene.h"
#include "format.h"
#include "SimpleAudioEngine.h"

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


    _lbEnemyInfo = Label::createWithTTF("", "fonts/pixel.ttf", 120);
    _lbEnemyInfo->setPosition(genPos({0.5, 0.7}));
    _lbEnemyInfo->setTextColor(Color4B::RED);
    _defaultLayer->addChild(_lbEnemyInfo);


    _lbFriendInfo = Label::createWithTTF("", "fonts/pixel.ttf", 120);
    _lbFriendInfo->setPosition(genPos({0.5, 0.3}));
    _lbFriendInfo->setTextColor(Color4B::RED);
    _defaultLayer->addChild(_lbFriendInfo);

    initTouchThings();

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

    checkBeats();
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
    node.beatPos = {random(0.1f,0.9f), 1.f};
    node.beatAccerate = 0;
    node.gen(_defaultLayer);

    return node;
}

void BeatQuestScene::checkBeats()
{
    float friendHitLine = uniform2real({0.f, friend_hit_line}).y;

    float enemyHitLine = uniform2real({0.f, enemy_hit_line}).y;
    for (auto iter = _runningBeats.begin(); iter != _runningBeats.end(); ) {
        float y = uniform2real(iter->beatPos).y;
        if ( iter->beatSpeed <= 0 && y < friendHitLine) {
            if (dealFriendHit(&(*iter))) {
                iter->clear();
                iter = _runningBeats.erase(iter);
                continue;
            }
        }
        if (iter->beatSpeed >= 0 && y > enemyHitLine) {
            if (dealEnemyHit(&(*iter))) {
                iter->clear();
                iter = _runningBeats.erase(iter);
                continue;
            }
        }
        iter++;
    }

}

void BeatQuestScene::initTouchThings()
{
    static bool touch_moved = false;
    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event){
        float touch_start = uniform2real({0,friend_start_line} ).y;
        float touch_end = uniform2real({0, friend_end_line}).y;
        Vec2 p = touch->getLocation();
        touch_moved = false;
        CCLOG("in");
        return true;//(p.y > touch_start && p.y < touch_end);
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event){
        touch_moved = true;
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event){
        if (!touch_moved) {
            tryHitBeat(touch->getLocation());
        }
    };

    listener->onTouchCancelled = [this](Touch* touch, Event* event){
    };

    _defaultLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _defaultLayer);
}

// 注意坐标的层次结构
inline bool beatCheckSpriteContain(cocos2d::Sprite* sp, cocos2d::Vec2 point)
{
    auto center = sp->getPosition();
    cocos2d::Size size = {sp->getContentSize().width * sp->getScaleX()*1.5f, sp->getContentSize().height * sp->getScaleY()*1.5f};
    CCLOG("%f, %f", sp->getScaleX(), sp->getScaleY());
    cocos2d::Rect rect;
    rect = {center.x - size.width/2, center.y - size.height/2, size.width, size.height};
    return rect.containsPoint(point);
}

void BeatQuestScene::tryHitBeat(cocos2d::Vec2 cursor)
{
    int hit = 0;
    for (auto iter = _runningBeats.begin(); iter != _runningBeats.end();) {
        if (beatCheckSpriteContain(iter->image, cursor) && iter->beatSpeed <= 0)
        {
            hit++;
            if (dealHitBeat(&(*iter))) {
                iter->clear();
                iter = _runningBeats.erase(iter);

                continue;
            }
        }
        iter++;
    }
    CCLOG("%d", hit);
}

bool BeatQuestScene::dealHitBeat(BeatNode* node)
{
    bool shouldDeleta = false;
    switch (node->type) {
        case BeatType::SWORD:
        case BeatType::COMBO:
            // 反转
            node->beatSpeed = -node->beatSpeed*1.5;
            shouldDeleta = false;
            break;

        case BeatType::BLOOD:
            _friendBlood += node->value;
            toast(true, fmt::sprintf("BLOOD +%d", node->value));

            shouldDeleta = true;
            break;

        case BeatType::SHIELD:
            _friendShieldCnt++;
            toast(true, "SHIELD!");
            shouldDeleta = true;
            break;

        default:
            break;
    }

    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/test.wav");

    return shouldDeleta;

}

bool BeatQuestScene::dealFriendHit(BeatNode* node)
{
    bool shouldDelete = false;
    switch (node->type) {
        case BeatType::COMBO:
            //TODO
            break;
        case BeatType::SWORD:
            if (_friendShieldCnt > 0) {
                _friendShieldCnt--;
                node->beatSpeed = -node->beatSpeed*1.5;
                shouldDelete = false;
                toast(true, fmt::sprintf("SHIELD BACK!", _enemyAttack));

            } else {
                _friendBlood -= _enemyAttack;
                toast(true, fmt::sprintf("BLOOD -%d", _enemyAttack));
                shouldDelete = true;
                if (_friendBlood < 0) {
                    toast(true, "DEAD!!!");
                    newFriend();
                }
            }
            break;

        case BeatType::BLOOD:
        case BeatType::SHIELD:
            node->beatSpeed = -node->beatSpeed*1.5;
            shouldDelete = false;
            break;

        default:
            break;
    }
    return shouldDelete;
}

bool BeatQuestScene::dealEnemyHit(BeatNode* node)
{
    bool shouldDelete = false;
    switch (node->type) {
        case BeatType::COMBO:
            //TODO
            break;
        case BeatType::SWORD:
            if (_enemyShieldCnt > 0) {
                _enemyShieldCnt--;
                node->beatSpeed = -node->beatSpeed*1.5;
                shouldDelete = false;
                toast(false, fmt::sprintf("SHIELD BACK!", _enemyAttack));
            } else {
                _enemyBlood -= _friendAttack;
                toast(false, fmt::sprintf("BLOOD -%d", _friendAttack));
                shouldDelete = true;
                if (_enemyBlood < 0) {
                    toast(false, "DEAD!!!");
                    newEnemy();
                }
            }
            break;

        case BeatType::BLOOD:
            shouldDelete = true;
            _enemyBlood += node->value;
            toast(false, fmt::sprintf("BLOOD +%d", node->value));
            break;

        case BeatType::SHIELD:
            shouldDelete = true;
            _enemyShieldCnt++;
            toast(false, "BLOOD ADD!");
            break;

        default:
            break;
    }
    return shouldDelete;
}



void BeatQuestScene::toast(bool isfriend, std::string text)
{
    auto lb = isfriend ? _lbFriendInfo : _lbEnemyInfo;

    lb->setVisible(true);
    lb->setString(text);
    lb->runAction(Sequence::create(ScaleTo::create(0.3, 1.5), ScaleTo::create(0.3, 1.0), Hide::create(), NULL));
}