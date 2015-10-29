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
#include "TripleDef.h"



struct TripleNode
{
    cocos2d::Sprite* image;
    TripleType type;
    void setPosition(cocos2d::Vec2 pos, bool isMoveOrOccure);
    void clear();
};

inline cocos2d::Rect tripleAreaRect()
{
    cocos2d::Vec2 origin = {TRIPLE_AREA_CENTER_X - 0.5f * (TRIPLE_CUBE_WIDTH*NUM_TRIPLE_WIDTH), TRIPLE_AREA_CENTER_Y - 0.5f * (TRIPLE_CUBE_WIDTH*NUM_TRIPLE_HEIGHT)};
    cocos2d::Size size = {TRIPLE_CUBE_WIDTH*NUM_TRIPLE_WIDTH, TRIPLE_CUBE_WIDTH*NUM_TRIPLE_HEIGHT};
    return {origin.x,origin.y,size.width,size.height};
}

inline cocos2d::Vec2 triplepos2floatpos(TriplePos pos)
{
    auto rect = tripleAreaRect();

    float x = rect.origin.x + (pos.x + 0.5f)*TRIPLE_CUBE_WIDTH;
    float y = rect.origin.y + (pos.y + 0.5f)*TRIPLE_CUBE_WIDTH;
    return {x,y};
}

inline TriplePos floatpos2triplepos(cocos2d::Vec2 pos)
{
    auto rect = tripleAreaRect();
    int x = (pos.x - rect.origin.x)/TRIPLE_CUBE_WIDTH;
    int y = (pos.y - rect.origin.y)/TRIPLE_CUBE_WIDTH;
    return {x,y};
}



class GameTestScene:public TRBaseScene
{
public:
    CREATE_FUNC(GameTestScene);
    virtual bool init() override;


protected:
    cocos2d::Layer* _tripleLayer;

    std::unordered_map<TriplePos, TripleNode> _tripleNodeMap;
    std::unordered_map<TriplePos, bool> _tripleRunningFlag;
    int _tripleRunningCnt;
    void initTripleField();
    TripleNode genRandomNode();
    void tryRushCubes(cocos2d::Vec2 cursor); // 尝试消除
    void tripleSelect(TriplePos pos, TripleType type);
};

#endif /* GameTestScene_hpp */
