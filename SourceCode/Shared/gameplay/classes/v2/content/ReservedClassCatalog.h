#pragma once

#include "gameplay/classes/v2/content/IClassContentSource.h"

namespace gameplay
{
namespace classes
{
class PlayableClassRegistry;

// Identities visible in source/protocol history but intentionally disabled
// until their gameplay contracts and assets are proven canonical.
class ReservedClassesContentSource : public IClassContentSource
{
public:
    virtual bool Load(std::vector<PlayableClassDefinition>& out,
                      std::string* error = 0) const;
};

// Compatibility entry point for the currently known reserved Assassin.
bool RegisterReservedAssassinClass(PlayableClassRegistry& registry);
}
}
