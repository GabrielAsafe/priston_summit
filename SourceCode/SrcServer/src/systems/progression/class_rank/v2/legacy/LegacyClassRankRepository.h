#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Shared/gameplay/progression/v2/ClassRankReadModel.h"
#include "systems/persistence/database/core/Database.h"

namespace gameplay::progression::legacy
{
class LegacyClassRankRepository final : public IClassRankRepository
{
public:
    bool Load(const ClassRankQuery& query, std::vector<ClassRankEntry>& entries) override
    {
        entries.clear();
        std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ServerDB);
        if(!db || !db->Open())
            return false;

        const char* sql = nullptr;
        if(query.metric == RankingMetric::Level)
        {
            sql = query.IsGlobal()
                ? "SELECT * FROM [PVP] where id not in (Select accountId COLLATE Latin1_General_CI_AI From accountdb..GameMasters where active =1) and class <> 0 Order By LVL Desc"
                : "SELECT * FROM [PVP] WHERE [CLASS]=? and ID not in (Select accountId COLLATE Latin1_General_CI_AI From accountdb.dbo.GameMasters where active =1) Order By LVL Desc";
        }
        else if(query.metric == RankingMetric::Kills)
        {
            sql = query.IsGlobal()
                ? "SELECT * FROM [PVP] where id not in (Select accountId COLLATE Latin1_General_CI_AI From accountdb.dbo.GameMasters where active =1) AND Kills > 0 Order By Kills Desc"
                : "SELECT * FROM [PVP] WHERE [Class]=? and id not in (Select accountId COLLATE Latin1_General_CI_AI From accountdb.dbo.GameMasters where active =1) AND Kills > 0  Order By Kills Desc";
        }

        if(!sql || !db->Prepare(sql))
        {
            db->Close();
            return false;
        }

        int classCode = query.classCode;
        if(!query.IsGlobal() && !db->BindInputParameter(&classCode, 1, PARAMTYPE_Integer))
        {
            db->Close();
            return false;
        }

        if(!db->Execute())
        {
            db->Close();
            return false;
        }

        do
        {
            char name[32] = {0};
            int classValue = 0;
            ClassRankEntry entry;
            db->GetData(name, sizeof(name), 2, PARAMTYPE_String);
            db->GetData(&classValue, sizeof(classValue), 3, PARAMTYPE_Integer);
            entry.name = name;
            entry.classCode = classValue;
            if(query.metric == RankingMetric::Level)
            {
                std::int64_t level = 0;
                db->GetData(&level, sizeof(level), 4, PARAMTYPE_Int64);
                entry.level = level;
            }
            else
            {
                int kills = 0;
                db->GetData(&kills, sizeof(kills), 5, PARAMTYPE_Integer);
                entry.kills = kills;
            }
            entries.push_back(entry);
        }
        while(db->NextRow());

        db->Close();
        return true;
    }
};

inline LegacyClassRankRepository& ServerLegacyClassRankRepository()
{
    static LegacyClassRankRepository repository;
    return repository;
}
}
