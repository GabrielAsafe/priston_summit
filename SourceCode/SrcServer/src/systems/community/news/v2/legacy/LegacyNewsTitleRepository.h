#pragma once

#include <string>

#include "Shared/community/v2/NewsTitleService.h"
#include "modules/File/File.h"

namespace gameplay::community::legacy
{
class LegacyNewsTitleRepository final : public INewsTitleRepository
{
public:
    bool LoadTitle(std::string& title) override
    {
        title.clear();
        try
        {
            File file("Configs//Sistemas.ini");
            title = file.readString("Title", "message");
            return true;
        }
        catch (FileException)
        {
            return false;
        }
    }
};

inline LegacyNewsTitleRepository& ServerLegacyNewsTitleRepository()
{
    static LegacyNewsTitleRepository repository;
    return repository;
}
}
