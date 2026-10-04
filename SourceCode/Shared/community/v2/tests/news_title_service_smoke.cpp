#include <cassert>
#include <string>

#include "Shared/community/v2/NewsTitleService.h"

class FakeTitleRepository final : public gameplay::community::INewsTitleRepository
{
public:
    bool succeed = true;
    std::string stored = "SUMMIT";

    bool LoadTitle(std::string& title) override
    {
        if(!succeed)
            return false;
        title = stored;
        return true;
    }
};

int main()
{
    FakeTitleRepository repository;
    std::string title = "stale";
    assert(gameplay::community::NewsTitleService::Load(repository, title));
    assert(title == "SUMMIT");

    repository.succeed = false;
    title = "stale";
    assert(!gameplay::community::NewsTitleService::Load(repository, title));
    assert(title.empty());
    return 0;
}
