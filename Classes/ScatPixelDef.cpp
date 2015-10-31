//
//  ScatPixelDef.cpp
//  triplequest
//
//  Created by chenbingfeng on 15/10/31.
//
//

#include "ScatPixelDef.hpp"

#include "../rapidjson/rapidjson.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/filereadstream.h"
#include "../rapidjson/filewritestream.h"

USING_NS_CC;

std::vector<PixelUnit> loadScatPixelFile(std::string fn)
{
    std::vector<PixelUnit> ret;

    auto filedata = FileUtils::getInstance()->getStringFromFile(fn);

    rjson::Document doc;
    doc.Parse(filedata.c_str());

    auto& data = doc["data"];
    for (auto iter = data.Begin(); iter != data.End(); iter++) {
        PixelUnit unit;
        unit.pos.x = (*iter)["x"].GetInt();
        unit.pos.y = (*iter)["y"].GetInt();
        unit.color.r = (*iter)["r"].GetInt();
        unit.color.g = (*iter)["g"].GetInt();
        unit.color.b = (*iter)["b"].GetInt();
        ret.push_back(unit);
    }

    return ret;
}