//
//  GameTestScene.cpp
//  triplequest
//
//  Created by chenbingfeng on 15/10/29.
//
//

#include "GameTestScene.h"

USING_NS_CC;

bool GameTestScene::init()
{
    assert(TRBaseScene::init());


    _layerTriple = Layer::create();
    _layerTriple->setPosition(genPos({0.5,0.5}));
    this->addChild(_layerTriple);


    addCommonBtn({0.5,0.1}, "t", [](){});

    return true;
}