#pragma once

#include <string>

#include "gameplay/progression/v2/ProgressionContent.h"

namespace gameplay
{
namespace progression
{
class SQLiteProgressionContentLoader
{
public:
    bool Load(ProgressionContentCatalog& catalog, std::string* error = 0) const;
};
}
}
