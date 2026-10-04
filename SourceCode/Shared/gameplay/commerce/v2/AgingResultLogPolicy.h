#pragma once

namespace gameplay
{
namespace commerce
{
enum class AgingAnnouncementKind
{
    None,
    OneLevelDown,
    TwoLevelsDown,
    Success,
    TwoLevelsUp
};

struct AgingResultLogConfig
{
    int oneLevelDownLog;
    int twoLevelsDownLog;
    int successLog;
    int twoLevelsUpLog;
    int announcementLevel;

    AgingResultLogConfig()
        : oneLevelDownLog(0), twoLevelsDownLog(0), successLog(0),
          twoLevelsUpLog(0), announcementLevel(10) {}
};

struct AgingResultLogPlan
{
    int ageNumber;
    int logCode;
    AgingAnnouncementKind announcement;
    bool announce;

    AgingResultLogPlan()
        : ageNumber(0), logCode(0), announcement(AgingAnnouncementKind::None),
          announce(false) {}
};

class AgingResultLogPolicy
{
public:
    static AgingResultLogPlan Plan(int resultCode,
                                   int resultingAgingLevel,
                                   bool directStone,
                                   const AgingResultLogConfig& config)
    {
        AgingResultLogPlan plan;
        plan.ageNumber = directStone ? resultingAgingLevel : resultingAgingLevel + 1;
        plan.logCode = config.successLog;

        switch(resultCode)
        {
        case 1:
            plan.logCode = config.oneLevelDownLog;
            plan.announcement = AgingAnnouncementKind::OneLevelDown;
            break;
        case 2:
            plan.logCode = config.twoLevelsDownLog;
            plan.announcement = AgingAnnouncementKind::TwoLevelsDown;
            break;
        case 3:
            plan.logCode = config.successLog;
            plan.announcement = AgingAnnouncementKind::Success;
            break;
        case 4:
            plan.logCode = config.twoLevelsUpLog;
            plan.announcement = AgingAnnouncementKind::TwoLevelsUp;
            break;
        default:
            break;
        }

        plan.announce = plan.announcement != AgingAnnouncementKind::None &&
                        plan.ageNumber >= config.announcementLevel;
        return plan;
    }
};
}
}
