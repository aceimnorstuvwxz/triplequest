// (C) 2015 Turnro.com

#include "Bigmap.h"
#include "DDMapData.h"
#include "format.h"
#include "TRLocale.h"
#include "GameScene.h"

USING_NS_CC;

void Bigmap::initMessageThings()
{
    float fontSize = 25 * DDConfig::fontSizeRadio();
    auto bigmapRect = DDConfig::bigmapAreaRect();
    float px = bigmapRect.origin.x + 0.5f * bigmapRect.size.width;
    float py_now = bigmapRect.origin.y + fontSize;
    _messageYStart = py_now;
    float y_step = (bigmapRect.size.height - fontSize*2)/NUM_MESSAGE_LABEL;
    _messageYStep = y_step;
    auto size = DDConfig::getWinSize();

    cocos2d::Vec2 shadowOffset = Vec2{0,2}*DDConfig::fontSizeRadio();
    for (int i = 0; i < NUM_MESSAGE_LABEL; i++) {
        auto lbAbove = Label::createWithTTF("test message", TRLocale::s().font(), fontSize);
        lbAbove->setPosition({px, py_now});
        lbAbove->setZOrder(GameScene::Z_BIGMAP_MESSAGE_ABOVE);
        lbAbove->setTextColor(Color4B::WHITE);

        _lbMessages[i] = lbAbove; 
        _uiLayer->addChild(lbAbove);

        auto lbShadow = Label::createWithTTF("test message", TRLocale::s().font(), fontSize);
        lbShadow->setPosition(Vec2{px, py_now} + shadowOffset);
        lbShadow->setZOrder(GameScene::Z_BIGMAP_MESSAGE_SHADOW);
        lbShadow->setTextColor(Color4B::GRAY);

        _lbMessagesShadow[i] = lbShadow;
        _uiLayer->addChild(lbShadow);

        // 设置属性
        lbAbove->setWidth(size.width*0.95);
        lbAbove->setAlignment(TextHAlignment::LEFT);
        lbAbove->setVisible(false);

        lbShadow->setWidth(size.width*0.95);
        lbShadow->setAlignment(TextHAlignment::LEFT);
        lbShadow->setVisible(false);

        py_now += y_step;
    }
    _messageHeadIndex = NUM_MESSAGE_LABEL-1;

    _rectEffectNode = DDRectEffectNode::create();
    _rectEffectNode->setPosition({0,0});
    _rectEffectNode->setZOrder(Z_RECT_EFFECT);
    _bigmapLayer->addChild(_rectEffectNode);
}

void Bigmap::message(const std::string& text, const cocos2d::Color4B& color, const cocos2d::Color4B& shadowColor, float scaler)
{
    const float message_fade_out_time = 10.f;
    //所有往上移动一格
    for (int i = 0; i < NUM_MESSAGE_LABEL; i++) {
        _lbMessages[i]->setPositionY(_lbMessages[i]->getPositionY()+_messageYStep);
        _lbMessagesShadow[i]->setPositionY(_lbMessagesShadow[i]->getPositionY()+_messageYStep);
    }
    _lbMessages[_messageHeadIndex]->setString(text);
    _lbMessages[_messageHeadIndex]->setTextColor(color);
    _lbMessages[_messageHeadIndex]->setPositionY(_messageYStart);
    _lbMessages[_messageHeadIndex]->setVisible(true);
    _lbMessages[_messageHeadIndex]->setOpacity(255);
    _lbMessages[_messageHeadIndex]->setScale(scaler);
//    _lbMessages[_messageHeadIndex]->runAction(FadeOut::create(message_fade_out_time));
    _lbMessagesShadow[_messageHeadIndex]->setString(text);
    _lbMessagesShadow[_messageHeadIndex]->setTextColor(shadowColor);
    _lbMessagesShadow[_messageHeadIndex]->setPositionY(_messageYStart);
    _lbMessagesShadow[_messageHeadIndex]->setVisible(true);
    _lbMessagesShadow[_messageHeadIndex]->setScale(scaler);
    _lbMessagesShadow[_messageHeadIndex]->setOpacity(255);
//    _lbMessagesShadow[_messageHeadIndex]->runAction(FadeOut::create(message_fade_out_time));

    // TODO text逐字移出 TODO 我很蠢， label除了可以设置字号之外，还可以用scaler来缩放！！ TODO

    _messageHeadIndex--;
    if (_messageHeadIndex < 0) {
        _messageHeadIndex = NUM_MESSAGE_LABEL - 1;
    }
}

static inline cocos2d::Vec4 color4b2vec4(const cocos2d::Color4B& color) {
    return (1.f/255)* Vec4{static_cast<float>(color.r), static_cast<float>(color.g), static_cast<float>(color.b), static_cast<float>(color.a)};
}

void Bigmap::op_showMessage(BigmapMessageType messageType, const std::string& text, bool direct, bool rectEffect, const MapPos& mappos)
{
    auto colorpair = messageTypeToColor(messageType);
    message(direct ? text : TRLocale::s()[text], colorpair.first, colorpair.second, messageTypeToScaler(messageType));
    if (rectEffect) {
        auto rectConfig = messageTypeToRectConfig(messageType);
        float delay = 0.f;
        Vec2 pos = DDConfig::bigmapCubeWidth() * Vec2{static_cast<float>(mappos.x), static_cast<float>(mappos.y)};
        for (int i = 0; i < rectConfig.number; i++) {
            _rectEffectNode->configRectEffect(color4b2vec4(colorpair.first), pos, rectConfig.expand, rectConfig.interval, delay);
            delay += rectConfig.delayStep;
        }
    }
}