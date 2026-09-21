#pragma once

#include <cstdint>
#include <string>

#include "gameplay/classes/v2/services/ClassService.h"

namespace gameplay
{
namespace classes
{
namespace legacy
{
// Compatibility DTO for old consumers of JOB_DATA_CODE.  The class domain
// deliberately does not depend on fileread.h; legacy loaders translate this
// view into their historical struct at the boundary.
struct LegacyJobDataView
{
    std::string name;
    std::string displayName;
    std::uint32_t jobBitCode;
    int jobCode;
    int brood;
    CharacterFormulaProfile formula;

    LegacyJobDataView() : jobBitCode(0u), jobCode(0), brood(0) {}
};

inline bool BuildLegacyJobDataView(const ClassService& service,
                                   int legacyJobCode,
                                   int legacyRankIndex,
                                   LegacyJobDataView& out)
{
    // ChangeJob is a zero-based compatibility index; the valid range belongs
    // to the class rank chain rather than a global CHARACTER_RANK5 constant.
    if (!service.IsValidLegacyChangeJob(legacyJobCode, legacyRankIndex))
        return false;

    const PlayableClassDefinition* definition = service.FindEnabled(legacyJobCode);
    if (!definition)
        return false;

    const ClassRankDefinition* rank = definition->FindRank(legacyRankIndex + 1);
    if (!rank || rank->legacyJobBitCode == 0u)
        return false;

    out.name = !rank->displayKey.empty() ? rank->displayKey :
               (!rank->key.empty() ? rank->key : definition->key);
    out.displayName = !rank->displayKey.empty() ? rank->displayKey :
                      (!definition->displayKey.empty() ? definition->displayKey : out.name);
    out.jobBitCode = rank->legacyJobBitCode;
    out.jobCode = definition->legacyJobCode;
    out.brood = definition->brood;

    // JOB_DATA_CODE historically stores formulas only on rank 1 rows. Higher
    // rank rows are title/bit aliases with zeroed formula fields.
    out.formula = CharacterFormulaProfile();
    if (legacyRankIndex == 0)
        out.formula = definition->formula;

    return true;
}
}
}
}
