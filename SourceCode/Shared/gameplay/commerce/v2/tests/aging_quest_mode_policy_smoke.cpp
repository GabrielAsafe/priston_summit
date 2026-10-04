#include "gameplay/commerce/v2/AgingQuestModePolicy.h"

#include <cassert>

int main()
{
    using gameplay::commerce::AgingQuestMode;
    using gameplay::commerce::AgingQuestModePolicy;

    static_assert(static_cast<int>(AgingQuestMode::ModeratorBypass) == -1, "legacy moderator mode must remain -1");
    static_assert(static_cast<int>(AgingQuestMode::Normal) == 0, "legacy normal mode must remain 0");
    static_assert(static_cast<int>(AgingQuestMode::Quest) == 1, "legacy quest mode must remain 1");

    assert(AgingQuestModePolicy::Resolve(false, false) == AgingQuestMode::Normal);
    assert(AgingQuestModePolicy::Resolve(false, true) == AgingQuestMode::ModeratorBypass);
    assert(AgingQuestModePolicy::Resolve(true, false) == AgingQuestMode::Quest);
    assert(AgingQuestModePolicy::Resolve(true, true) == AgingQuestMode::Quest);

    return 0;
}
