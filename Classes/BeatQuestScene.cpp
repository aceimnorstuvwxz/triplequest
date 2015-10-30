// (C) 2015 Arisecbf


#include "BeatQuestScene.h"
#include "format.h"

USING_NS_CC;


bool BeatQuestScene::init()
{
    assert(TRBaseScene::init());


    addCommonBtn({0.5,0.9}, "t", [](){});

    return true;
}

