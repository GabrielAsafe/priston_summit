#pragma once

#include <memory>
#include <vector>

#include "Shared/community/v2/NewsFeedService.h"
#include "systems/persistence/database/core/Database.h"

namespace gameplay::community::legacy
{
class LegacyNewsNoticeRepository final : public INewsNoticeRepository
{
public:
    bool LoadActive(std::vector<NewsNoticeRecord>& notices) override
    {
        notices.clear();
        std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ServerDB);
        if(!db || !db->Open())
            return false;

        bool ok = false;
        if(db->Prepare("SELECT * FROM NoticeSystem WHERE Activated=1") && db->Execute())
        {
            do
            {
                char message[256] = {0};
                int messageType = 0;
                db->GetData(message, sizeof(message), 2, PARAMTYPE_String);
                db->GetData(&messageType, 0, 3, PARAMTYPE_Integer);
                notices.push_back(NewsNoticeRecord{message, messageType});
            }
            while(db->NextRow());
            ok = true;
        }

        db->Close();
        return ok;
    }
};

inline LegacyNewsNoticeRepository& ServerLegacyNewsNoticeRepository()
{
    static LegacyNewsNoticeRepository repository;
    return repository;
}
}
