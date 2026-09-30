#pragma once

#include "gameplay/professions/v2/IProfessionRepository.h"

namespace gameplay
{
namespace professions
{
class SQLiteProfessionRepository final : public IProfessionRepository
{
public:
    bool Ready();
    bool LoadPlayer(const std::string& playerName, std::vector<ProfessionRecord>& out) override;
    bool SaveProgress(const std::string& playerName,
                      ProfessionType type,
                      const ProfessionProgress& progress) override;

private:
    bool EnsureSchema();
};
}
}
