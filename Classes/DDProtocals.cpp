// (C) 2015 Turnro.com

#include "DDProtocals.h"



bool operator==(const MapPos& r, const MapPos& l)
{ return r.x == l.x && r.y == l.y;}

bool DDPropertyFieldProtocal::flagIsTappingExclusive = false;
