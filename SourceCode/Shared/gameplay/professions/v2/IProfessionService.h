#pragma once

#include <cstdint>
#include <string>

#include "gameplay/professions/v2/ProfessionTypes.h"

namespace gameplay
{
namespace professions
{
class IProfessionService
{
public:
    virtual ~IProfessionService() {}

    virtual bool LoadPlayer(const std::string& playerName) = 0;
    virtual void SetProgress(const std::string& playerName, ProfessionType type,
                             const ProfessionProgress& progress) = 0;
    virtual bool GetProgress(const std::string& playerName, ProfessionType type,
                             ProfessionProgress& out) const = 0;
    virtual int GetLevel(const std::string& playerName, ProfessionType type) const = 0;
    virtual std::uint64_t GetExperience(const std::string& playerName,
                                        ProfessionType type) const = 0;
    virtual bool AddExperience(const std::string& playerName, ProfessionType type,
                               std::uint64_t amount) = 0;
    virtual void ClearPlayer(const std::string& playerName) = 0;
};
}
}
