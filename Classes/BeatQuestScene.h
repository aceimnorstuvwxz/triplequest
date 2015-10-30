// (C) 2015 Arisecbf
#ifndef BeatQuestScene_hpp
#define BeatQuestScene_hpp

#include <stdio.h>

#include "TRBaseScene.h"
#include "format.h"

// Beat Quest Gameplay test

class BeatQuestScene:public TRBaseScene
{
public:
    CREATE_FUNC(BeatQuestScene);
    virtual bool init() override;

protected:
};

#endif /* BeatQuestScene_hpp */
