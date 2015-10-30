// (C) 2015 Arisecbf

#ifndef SOCommon_hpp
#define SOCommon_hpp

#include <stdio.h>
#include "cocos2d.h"
// cocos2d-x 相关的加强辅助，项目间通用，常用。



inline cocos2d::Vec2 soGenPos(const cocos2d::Vec2& pos)
{
    auto size = cocos2d::Director::getInstance()->getVisibleSize();
    return {size.width * pos.x, size.height * pos.y};
}


#endif /* SOCommon_hpp */
