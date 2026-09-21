#include "gameplay/classes/v2/content/DefaultClassContent.h"

#include <cstdio>
#include <cstdlib>

#include "gameplay/classes/v2/content/CompositeClassContentSource.h"
#include "gameplay/classes/v2/content/LegacyClassCatalog.h"
#include "gameplay/classes/v2/content/ReservedClassCatalog.h"

namespace gameplay
{
namespace classes
{
bool BuildDefaultPlayableClassRegistry(PlayableClassRegistry& out,
                                       std::string* error)
{
    LegacyEightClassesContentSource legacy;
    ReservedClassesContentSource reserved;
    CompositeClassContentSource sources;
    sources.Add(legacy).Add(reserved);

    PlayableClassRegistry candidate;
    if (!LoadClassContent(candidate, sources, error))
        return false;

    out = candidate;
    if (error) error->clear();
    return true;
}

const PlayableClassRegistry& DefaultPlayableClassRegistry()
{
    static const PlayableClassRegistry registry = []()
    {
        PlayableClassRegistry value;
        std::string error;
        if(!BuildDefaultPlayableClassRegistry(value, &error))
        {
            std::fprintf(stderr, "PlayableClassRegistry startup validation failed: %s\n", error.c_str());
            std::abort();
        }
        return value;
    }();
    return registry;
}
}
}
