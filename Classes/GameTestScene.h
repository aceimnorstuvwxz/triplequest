//
//  GameTestScene.hpp
//  triplequest
//
//  Created by chenbingfeng on 15/10/29.
//
//

#ifndef GameTestScene_hpp
#define GameTestScene_hpp

#include <stdio.h>

#include "TRBaseScene.h"

class GameTestScene:public TRBaseScene
{
public:
    CREATE_FUNC(GameTestScene);
    virtual bool init() override;


protected:
    cocos2d::Layer* _layerTriple;
};

#endif /* GameTestScene_hpp */
