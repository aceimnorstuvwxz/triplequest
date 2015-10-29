//
//  TripleDef.cpp
//  triplequest
//
//  Created by chenbingfeng on 15/10/29.
//
//

#include "TripleDef.h"

bool operator==(const TriplePos& r, const TriplePos& l)
{
    return r.x == l.x && r.y == l.y;
}