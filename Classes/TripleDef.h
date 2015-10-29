//
//  TripleDef.hpp
//  triplequest
//
//  Created by chenbingfeng on 15/10/29.
//
//

#ifndef TripleDef_hpp
#define TripleDef_hpp

#include <stdio.h>
#include "cocos2d.h"

constexpr static const int NUM_TRIPLE_WIDTH = 7;
constexpr static const int NUM_TRIPLE_HEIGHT = 7;

struct TriplePos
{
    int x;
    int y;

    int distance(const TriplePos& r)
    {
        return std::abs(r.x-x) + std::abs(r.y-y);
    }
};

bool operator==(const TriplePos& r, const TriplePos& l);

namespace std{
    template<>
    class hash<TriplePos> {
        public :
        size_t operator()(const TriplePos &p) const {
            return p.y * 1000 + p.x;
        }
    };
}


enum class TripleType:int
{

    PA = 0, // physic attack

    PD = 1, // physic defence

    BL = 2, // add blood

    MA = 3, // magic attack
    MD = 4, // magic defence
    MAX = 5
};


constexpr const float TRIPLE_CUBE_WIDTH = 750.f/(NUM_TRIPLE_WIDTH+1);

constexpr const float TRIPLE_AREA_CENTER_X = 750.f/2;
constexpr const float TRIPLE_AREA_CENTER_Y = 750.f/2;


inline TripleType genRandomTripleType()
{
    return static_cast<TripleType>(cocos2d::random(0, 2));
}

inline std::string tripleType2string(TripleType trp)
{
    std::string r;
    switch (trp) {
        case TripleType::MA:
            r = "MA";
            break;
        case TripleType::MD:
            r= "MD";
            break;
        case TripleType::PA:
            r = "PA";
            break;
        case TripleType::PD:
            r = "PD";
            break;
        case TripleType::BL:
            r = "BL";
            break;

        default:
            assert(false);
            r= "";
            break;
    }
    return r;
}

#endif /* TripleDef_hpp */
