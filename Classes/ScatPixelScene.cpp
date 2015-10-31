// (C) 2015 Arisecbf


#include "ScatPixelScene.h"
#include "format.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

bool ScatPixelScene::init()
{
    assert(TRBaseScene::init());


    addCommonBtn({0.5,0.9}, "t", [](){});

    return true;
}
