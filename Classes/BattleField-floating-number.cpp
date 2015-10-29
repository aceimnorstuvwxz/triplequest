// (C) 2015 Turnro.com

#include "BattleField.h"
#include "format.h"
#include "DDMapData.h"
#include "TRLocale.h"

USING_NS_CC;

// Floating Number



void FloatingNumberManager::init(cocos2d::Layer *layer)
{
    _layer = layer;

    for (int i = 0; i < NUM_LABELS; i++) {
        auto lb = Label::createWithTTF("", "fonts/fz.ttf", 30);
        lb->setZOrder(BattleField::Z_FLOATING_NUMBER);
        lb->setVisible(false);
        lb->enableOutline(Color4B::BLACK);
        _layer->addChild(lb);
        _labels.push_back(lb);
    }
}


inline Color4B floatingType2color(int type, int state)
{
    // state 0 较少 1 较中 2 较大，不同色彩浓度来反应// TODO
    Color4B r;
    switch (type) {
        case FloatingNumberManager::ATTACK:
            r = Color4B::RED;
            break;
        case FloatingNumberManager::CURE:
            r = Color4B::GREEN;
            break;
        case FloatingNumberManager::MINE:
            r = Color4B::BLUE;
        default:
            break;
    }
    return r;
}

void FloatingNumberManager::toast(int type, int num, const AgentPos &pos, int state) // state 0 较小 1中 2大
{
    // borrow lb
    auto lb = _labels.front();
    _labels.pop_front();
    _labels.push_back(lb);
    lb->stopAllActions();

    // config setting
    lb->setString(fmt::sprintf("%d", num));
    lb->setTextColor(floatingType2color(type, state));

    lb->setOpacity(0);
    lb->runAction(Spawn::create( Sequence::create(Show::create(), FadeIn::create(0.1), DelayTime::create(0.5), FadeOut::create(0.1), Hide::create(), NULL), MoveBy::create(0.7, {0, 20}), NULL));
    lb->setPosition(BattleField::help_agentPos2agentFloatPos(pos));
}