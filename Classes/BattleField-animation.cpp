// (C) 2015 Turnro.com

#include "BattleField.h"
#include "DDConfig.h"
#include "format.h"

#include "../rapidjson/rapidjson.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/filereadstream.h"
#include "../rapidjson/filewritestream.h"

USING_NS_CC;


inline std::string animationType2stringkey(DDAnimationType type)
{
    switch (type) {
        case DDAnimationType::ACTION:
            return "action";
        case DDAnimationType::APPEAR:
            return "appear";
        case DDAnimationType::DIE:
            return "die";
        case DDAnimationType::DISAPPEAR:
            return "disappear";
        case DDAnimationType::IDLE:
            return "idle";
        case DDAnimationType::MOVE:
            return "move";
        default:
            assert(false);
            break;
    }
    return "";
}

// key 是agentType * 1000 + animationType。
std::unordered_map<int, std::pair<int, int>> DDAnimationManager::_animationMap;

// 在初始化游戏时，读取各个类型的动作配置文件。
void DDAnimationManager::init()
{
    for (int i = 0; i < DDAgent::AT_MAX; i++) {
        auto atkey = agentType2agentTypeString(i);

        auto file = fmt::sprintf("agents/%s.action.json", atkey);
        auto filedata = FileUtils::getInstance()->getStringFromFile(file);
        CCLOG("%s", filedata.c_str());
        rjson::Document doc;
        doc.Parse(filedata.c_str());
        auto& actions = doc["actions"];
        for (int j = 0; j < static_cast<int>(DDAnimationType::MAX); j++) {
            auto anikey = animationType2stringkey(static_cast<DDAnimationType>(j));
            int start = 0;
            int end = 0;
            if (actions.HasMember(anikey.c_str())) {
                start = actions[anikey.c_str()]["start"].GetInt();
                end = actions[anikey.c_str()]["end"].GetInt();
            }
            _animationMap[i*1000+j] = {start, end};
        }
    }
}



// 给出agent类型和animationType，返回该动作的start-end。
std::pair<int, int> DDAnimationManager::fetch(int agentType, DDAnimationType animationType)
{
    int key = agentType* 1000 + static_cast<int>(animationType);
    assert(_animationMap.count(key) > 0);
    return _animationMap[key];
}