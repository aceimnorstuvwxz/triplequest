// (C) 2015 Arisecbf
#ifndef StealthTestScene_hpp
#define StealthTestScene_hpp

#include "TRBaseScene.h"
#include "SimplePixelNode.h"

// Stealth Test


class StealthTestScene:public TRBaseScene
{
public:
    CREATE_FUNC(StealthTestScene);
    virtual bool init() override;

protected:

    cocos2d::Layer* _mainLayer;
    cocos2d::Camera* _mainCamera;

    SimplePixelNode* _simplePixelNode;
    SimplePixelNode* _bg;

    void initMainLayer();
};

#endif /* StealthTestScene_hpp */
