#pragma once

#include <string>
#include <vector>

#include "Shared/community/v2/NewsContentService.h"

namespace gameplay::community
{
struct NewsNoticeRecord
{
    std::string message;
    int messageType = 0;
};

class INewsNoticeRepository
{
public:
    virtual ~INewsNoticeRepository() = default;
    virtual bool LoadActive(std::vector<NewsNoticeRecord>& notices) = 0;
};

class NewsFeedService final
{
public:
    static bool LoadActiveMessages(INewsNoticeRepository& repository,
                                   int bossMinutes,
                                   std::vector<std::string>& messages)
    {
        std::vector<NewsNoticeRecord> notices;
        messages.clear();
        if(!repository.LoadActive(notices))
            return false;

        messages.reserve(notices.size());
        for(const NewsNoticeRecord& notice : notices)
        {
            std::string formatted;
            if(NewsContentService::FormatNotice(formatted, notice.message,
                                                notice.messageType, bossMinutes))
                messages.push_back(std::move(formatted));
        }
        return true;
    }
};
}
