//
//  GameTestScene.cpp
//  triplequest
//
//  Created by chenbingfeng on 15/10/29.
//
//

#include "GameTestScene.h"
#include "format.h"

USING_NS_CC;


bool GameTestScene::init()
{
    assert(TRBaseScene::init());


    _tripleLayer = Layer::create();
//    _tripleLayer->setPosition(genPos({0.5,0.5}));
    this->addChild(_tripleLayer);


    addCommonBtn({0.5,0.9}, "t", [](){});

    initTripleField();

    _lbBloodEnemy = Label::createWithTTF("", "fonts/fz.ttf", 40);
    _lbBloodEnemy->setPosition(genPos({0.75, 0.9}));
    _defaultLayer->addChild(_lbBloodEnemy);

    _lbBloodFriend = Label::createWithTTF("", "fonts/fz.ttf", 40);
    _lbBloodFriend->setPosition(genPos({0.25, 0.9}));
    _defaultLayer->addChild(_lbBloodFriend);

    _lbTurnTime = Label::createWithTTF("", "fonts/fz.ttf", 50);
    _defaultLayer->addChild(_lbTurnTime);
    _lbTurnTime->setPosition(genPos({0.5,0.5}));


    scheduleUpdate();

    newFriend();
    newEnemy();

    _turnTimeLeft = turnTime;


    return true;
}


TripleNode GameTestScene::genRandomNode()
{
    TripleNode node;
    node.type = genRandomTripleType();
    
    node.image = Sprite::create(fmt::sprintf("images/icons_%s.png", tripleType2string(node.type)));
    node.image->setScale(TRIPLE_CUBE_WIDTH/node.image->getContentSize().width);
    _tripleLayer->addChild(node.image);
    return node;
}

void TripleNode::setPosition(cocos2d::Vec2 pos, bool isMoveOrOccure)
{
    image->setPosition(pos);
}

void GameTestScene::initTripleField()
{
    for (int x = 0; x < NUM_TRIPLE_WIDTH; x++) {
        for (int y = 0; y < NUM_TRIPLE_HEIGHT; y++) {
            // 从左下角开始
            TriplePos pos = {x,y};
            _tripleNodeMap[pos] = genRandomNode();
            _tripleNodeMap[pos].setPosition(triplepos2floatpos(pos), false);
        }
    }

    static bool touch_moved = false;
    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event){
        auto rect = tripleAreaRect();
        touch_moved = false;
        return rect.containsPoint(touch->getLocation());
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event){
        touch_moved = true;
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event){
        if (!touch_moved) {
            tryRushCubes(touch->getLocation());
        }

    };

    listener->onTouchCancelled = [this](Touch* touch, Event* event){
    };

    _tripleLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _tripleLayer);
}

void TripleNode::clear()
{
    image->getParent()->removeChild(image);
}

void GameTestScene::tryRushCubes(cocos2d::Vec2 cursor) // 尝试消除
{
    // 选中了哪个
    _tripleRunningFlag.clear();
    _tripleRunningCnt = 0;
    auto pos = floatpos2triplepos(cursor);
    CCLOG("%d %d", pos.x, pos.y);
    auto type = _tripleNodeMap[pos].type;
    tripleSelect(pos, type);

    if (_tripleRunningCnt < 2) return;

    // 消除选中的
    CCLOG("type %s number %d", tripleType2string(type).c_str(), _tripleRunningCnt);
    for (auto p : _tripleRunningFlag) {
        _tripleNodeMap[p.first].clear();
        _tripleNodeMap[p.first] = genRandomNode();
        _tripleNodeMap[p.first].setPosition(triplepos2floatpos(p.first), false);
    }

    // 进入queue
    if (_tripleRunningCnt > 0) {
        if (_friendPowerQueue.size() > 0 && _friendPowerQueue.back().type == type) {
            // 放到之前的
            _friendPowerQueue.back().addCount(_tripleRunningCnt);
        } else {
            // 新
            PowerNode power;
            power.count = _tripleRunningCnt;
            power.type = type;
            power.image = Sprite::create(fmt::sprintf("images/icons_%s.png", tripleType2string(power.type)));
            power.label = Label::createWithTTF(fmt::sprintf("%d", power.count), "fonts/fz.ttf", 40);
            power.image->setScale(3);
            _defaultLayer->addChild(power.image);
            _defaultLayer->addChild(power.label);
            _friendPowerQueue.push_back(power);
        }
    }
}

void GameTestScene::tripleSelect(TriplePos pos, TripleType type)
{
    if (_tripleNodeMap.count(pos) > 0  && _tripleRunningFlag.count(pos) == 0 && _tripleNodeMap[pos].type == type) {
        _tripleRunningFlag[pos] = true;
        _tripleRunningCnt++;

        tripleSelect({pos.x-1, pos.y}, type);
        tripleSelect({pos.x+1, pos.y}, type);
        tripleSelect({pos.x, pos.y-1}, type);
        tripleSelect({pos.x, pos.y+1}, type);
    }
}


void GameTestScene::update(float dt)
{
    _lbBloodEnemy->setString(fmt::sprintf("%d", _enemyBlood));
    _lbBloodFriend->setString(fmt::sprintf("%d", _friendBlood));

    int i = 0;
    for (auto p : _enemyPowerQueue) {
        p.setPosition( genPos({0.1f + i*0.15f, 0.7f}));
        i++;
    }
    i = 0;
    for (auto p : _friendPowerQueue) {
        p.setPosition( genPos({0.1f + i*0.15f, 0.55f}));
        i++;
    }

    _turnTimeLeft -= dt;
    if (_turnTimeLeft < 0) {
        powerTick();
        _turnTimeLeft = turnTime;
    }
    _lbTurnTime->setString(fmt::sprintf("%.1f", _turnTimeLeft));
}

void GameTestScene::newFriend()
{
    _friendBlood = 30;
    for (auto p : _friendPowerQueue) {
        p.clear();
    }
    _friendPowerQueue.clear();
}

void GameTestScene::newEnemy()
{
    _enemyBlood = 30;
    for (auto p : _enemyPowerQueue) {
        p.clear();
    }
    _enemyPowerQueue.clear();

    pushEnemyPower();
    pushEnemyPower();
    pushEnemyPower();
}


void GameTestScene::pushEnemyPower()
{
    PowerNode power;
    power.count = random(1, 5);
    power.type = genRandomTripleType();
    power.image = Sprite::create(fmt::sprintf("images/icons_%s.png", tripleType2string(power.type)));
    power.label = Label::createWithTTF(fmt::sprintf("%d", power.count), "fonts/fz.ttf", 40);
    power.image->setScale(3);
    _defaultLayer->addChild(power.image);
    _defaultLayer->addChild(power.label);
    _enemyPowerQueue.push_back(power);
}

void GameTestScene::powerTick()
{

    auto enemyPower = _enemyPowerQueue.front();
    _enemyPowerQueue.pop_front();
    pushEnemyPower();
    enemyPower.clear();

    bool hasFriend = false;
    PowerNode friendPower;
    if (_friendPowerQueue.size() > 0) {
        friendPower = _friendPowerQueue.front();
        _friendPowerQueue.pop_front();
        friendPower.clear();
        hasFriend = true;
    }


    // 对我方伤害
    if (enemyPower.type == TripleType::MA || enemyPower.type == TripleType::PA) {
        int attack = enemyPower.count;
        //defence
        if (hasFriend && enemyPower.type == TripleType::MA && friendPower.type == TripleType::MD) {
            attack -= friendPower.count;
            if (attack <0) {
                attack = 0;
            }
        }
        if (hasFriend && enemyPower.type == TripleType::PA && friendPower.type == TripleType::PD) {
            attack -= friendPower.count;
            if (attack <0) {
                attack = 0;
            }
        }
        _friendBlood -= attack;
    }

    // 对敌方伤害
    if (hasFriend) {
        if (friendPower.type == TripleType::MA || friendPower.type == TripleType::PA) {
            int attack = friendPower.count;
            //defence
            if (friendPower.type == TripleType::MA && enemyPower.type == TripleType::MD) {
                attack -= enemyPower.count;
                if (attack <0) {
                    attack = 0;
                }
            }
            if (friendPower.type == TripleType::PA && enemyPower.type == TripleType::PD) {
                attack -= enemyPower.count;
                if (attack <0) {
                    attack = 0;
                }
            }
            _enemyBlood -= attack;
        }
    }

    // 我方加血
    if (hasFriend) {
        if (friendPower.type == TripleType::BL)
            _friendBlood += friendPower.count;
    }

    // 地方加血
    if (enemyPower.type == TripleType::BL) {
        _enemyBlood += enemyPower.count;
    }

    if (_friendBlood < 0) {
        newFriend();
    }

    if (_enemyBlood < 0) {
        newEnemy();
    }

}

