// (C) 2015 Arisecbf
#ifndef ScatPixelScene_hpp
#define ScatPixelScene_hpp

#include "TRBaseScene.h"
#include "SimplePixelNode.h"

// Scat Pixel Graphic Experiment


class ScatPixelScene:public TRBaseScene
{
public:
    CREATE_FUNC(ScatPixelScene);
    virtual bool init() override;

protected:

    cocos2d::Layer* _mainLayer;
    cocos2d::Camera* _mainCamera;

    SimplePixelNode* _simplePixelNode;
    void initMainLayer();
};

#endif /* ScatPixelScene_hpp */
