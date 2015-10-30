// (C) 2015 Arisecbf

#ifndef BeatDef_hpp
#define BeatDef_hpp

#include <stdio.h>


enum class BeatType{
    BLOOD, //血
    SWORD, //剑
    SHIELD,//盾
    COMBO, //连击
};

inline std::string beattype2string(BeatType t)
{
    std::string  r;
    switch (t) {
        case BeatType::BLOOD:
            r = "BLOOD";
            break;

        case BeatType::SWORD:
            r = "SWORD";
            break;

        case BeatType::SHIELD:
            r = "SHIELD";
            break;

        case BeatType::COMBO:
            r = "COMBO";
            break;

        default:
            assert(false);
            break;
    }
    return r;
}

#endif /* BeatDef_hpp */
