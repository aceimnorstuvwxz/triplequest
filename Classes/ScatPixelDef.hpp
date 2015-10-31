//
//  ScatPixelDef.hpp
//  triplequest
//
//  Created by chenbingfeng on 15/10/31.
//
//

#ifndef ScatPixelDef_hpp
#define ScatPixelDef_hpp

#include <stdio.h>
#include "cocos2d.h"

struct PixelPos
{
    int x, y;
};

struct PixelUnit
{
    PixelPos pos;
    cocos2d::Color3B color;
};

std::vector<PixelUnit> loadScatPixelFile(std::string fn);

#endif /* ScatPixelDef_hpp */
