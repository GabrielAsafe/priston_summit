#pragma once

#include <string>
#include <vector>

#include "gameplay/professions/v2/ProfessionTypes.h"

namespace gameplay
{
namespace professions
{
struct ProfessionRecord
{
    ProfessionType type;
    ProfessionProgress progress;

    ProfessionRecord() : type(ProfessionType::None), progress() {}
    ProfessionRecord(ProfessionType typeValue, const ProfessionProgress& progressValue)
        : type(typeValue), progress(progressValue) {}
};

class IProfessionRepository
{
public:
    virtual ~IProfessionRepository() {}
    virtual bool LoadPlayer(const std::string& playerName, std::vector<ProfessionRecord>& out) = 0;
    virtual bool SaveProgress(const std::string& playerName,
                              ProfessionType type,
                              const ProfessionProgress& progress) = 0;
};
}
}
