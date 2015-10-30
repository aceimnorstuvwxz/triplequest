// (C) 2015 Turnro.com

#include "TestGuideScene.h"
#include "TripleQuestScene.h"
#include "BeatQuestScene.h"

USING_NS_CC;

bool TestGuideScene::init()
{
    assert(TRBaseScene::init());


    addCommonBtn({0.5,0.9}, "Triple Quest", [](){
           Director::getInstance()->pushScene(TripleQuestScene::create());
    });


    addCommonBtn({0.5,0.8}, "Beat Quest", [](){
        Director::getInstance()->pushScene(BeatQuestScene::create());
    });

    return true;
}