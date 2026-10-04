#pragma once

#include <string>

namespace gameplay::community
{
class INewsTitleRepository
{
public:
    virtual ~INewsTitleRepository() = default;
    virtual bool LoadTitle(std::string& title) = 0;
};

class NewsTitleService final
{
public:
    static bool Load(INewsTitleRepository& repository, std::string& title)
    {
        title.clear();
        return repository.LoadTitle(title);
    }
};
}
