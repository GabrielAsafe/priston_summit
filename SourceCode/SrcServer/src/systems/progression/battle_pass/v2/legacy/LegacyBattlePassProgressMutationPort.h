#pragma once

#include <cstdio>
#include <string>

#include "Shared/gameplay/progression/v2/BattlePassProgressApplicationService.h"
#include "systems/persistence/database/odbc/SQL.h"

namespace gameplay::progression::legacy
{
class LegacyBattlePassProgressMutationPort final : public IBattlePassProgressMutationPort
{
public:
    bool AddExperience(const std::string& accountId,
                       int amount,
                       BattlePassProgressResult& result) override
    {
        result = BattlePassProgressResult{};
        SQL* const sql = SQL::GetInstance();
        if(!sql || !sql->IsConnected() || accountId.empty())
            return false;

        sql->EnterSqlSection();
        const bool ok = ApplyLocked(*sql, accountId, amount, result);
        sql->LeaveSqlSection();
        return ok;
    }

private:
    static bool ApplyLocked(SQL& sql,
                            const std::string& accountId,
                            int amount,
                            BattlePassProgressResult& result)
    {
        char query[512] = {0};
        int experience = 0;
        int level = 0;
        int passType = 0;

        std::snprintf(query, sizeof(query),
                      "SELECT * FROM [BattlePass].[dbo].[BP_Info] WHERE [ID]='%s'",
                      accountId.c_str());
        if(!sql.Execute(query))
            return false;

        if(sql.Fetch())
        {
            sql.GetDataValue(2, &experience);
            sql.GetDataValue(3, &level);
            sql.GetDataValue(4, &passType);
        }

        const BattlePassProgressState current{experience, level, passType};
        result = BattlePassProgressService::AddExperience(current, amount);
        if(!result.changed)
            return true;

        std::snprintf(query, sizeof(query),
                      "UPDATE [BattlePass].[dbo].[BP_Info] SET BP_Exp=%d, BP_Level=%d WHERE [ID]='%s'",
                      result.state.experience, result.state.level, accountId.c_str());
        return sql.Execute(query) != FALSE;
    }
};

inline LegacyBattlePassProgressMutationPort& ServerLegacyBattlePassProgressMutationPort()
{
    static LegacyBattlePassProgressMutationPort port;
    return port;
}
}
