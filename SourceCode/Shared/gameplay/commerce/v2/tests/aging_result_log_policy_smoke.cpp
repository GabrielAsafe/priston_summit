#include "gameplay/commerce/v2/AgingResultLogPolicy.h"

#include <cassert>

int main()
{
    using namespace gameplay::commerce;

    AgingResultLogConfig config;
    config.oneLevelDownLog = 11;
    config.twoLevelsDownLog = 12;
    config.successLog = 13;
    config.twoLevelsUpLog = 14;
    config.announcementLevel = 10;

    {
        const AgingResultLogPlan plan = AgingResultLogPolicy::Plan(1, 9, false, config);
        assert(plan.ageNumber == 10);
        assert(plan.logCode == 11);
        assert(plan.announcement == AgingAnnouncementKind::OneLevelDown);
        assert(plan.announce);
    }
    {
        const AgingResultLogPlan plan = AgingResultLogPolicy::Plan(4, 12, true, config);
        assert(plan.ageNumber == 12);
        assert(plan.logCode == 14);
        assert(plan.announcement == AgingAnnouncementKind::TwoLevelsUp);
        assert(plan.announce);
    }
    {
        const AgingResultLogPlan plan = AgingResultLogPolicy::Plan(3, 8, false, config);
        assert(plan.ageNumber == 9);
        assert(plan.logCode == 13);
        assert(!plan.announce);
    }
    {
        const AgingResultLogPlan plan = AgingResultLogPolicy::Plan(99, 20, false, config);
        assert(plan.ageNumber == 21);
        assert(plan.logCode == 13);
        assert(plan.announcement == AgingAnnouncementKind::None);
        assert(!plan.announce);
    }

    return 0;
}
