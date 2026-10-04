#include <cassert>
#include <string>
#include <vector>

#include "Shared/community/v2/NewsFeedService.h"

class FakeRepository final : public gameplay::community::INewsNoticeRepository
{
public:
    bool succeed = true;
    std::vector<gameplay::community::NewsNoticeRecord> source;

    bool LoadActive(std::vector<gameplay::community::NewsNoticeRecord>& notices) override
    {
        if(!succeed)
            return false;
        notices = source;
        return true;
    }
};

int main()
{
    FakeRepository repository;
    repository.source.push_back({"Maintenance soon", 0});
    repository.source.push_back({"Boss in %d minutes", 1});

    std::vector<std::string> messages;
    assert(gameplay::community::NewsFeedService::LoadActiveMessages(repository, 15, messages));
    assert(messages.size() == 2u);
    assert(messages[0] == "Maintenance soon");
    assert(messages[1] == "Boss in 15 minutes");

    repository.succeed = false;
    assert(!gameplay::community::NewsFeedService::LoadActiveMessages(repository, 15, messages));
    assert(messages.empty());
    return 0;
}
