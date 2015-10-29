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
        return tripleAreaRect().containsPoint(touch->getLocation());
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


