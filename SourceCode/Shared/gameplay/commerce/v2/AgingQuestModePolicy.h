#pragma once

namespace gameplay
{
namespace commerce
{
enum class AgingQuestMode
{
    ModeratorBypass = -1,
    Normal = 0,
    Quest = 1
};

class AgingQuestModePolicy
{
public:
    static AgingQuestMode Resolve(bool questItemCode, bool moderatorOrHigher)
    {
        if(questItemCode)
            return AgingQuestMode::Quest;

        return moderatorOrHigher ? AgingQuestMode::ModeratorBypass : AgingQuestMode::Normal;
    }
};
}
}
