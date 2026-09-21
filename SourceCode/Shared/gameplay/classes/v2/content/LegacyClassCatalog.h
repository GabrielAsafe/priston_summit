#pragma once

#include "gameplay/classes/v2/PlayableClassRegistry.h"
#include "gameplay/classes/v2/content/IClassContentSource.h"

namespace gameplay
{
namespace classes
{
// Compiled parity source for the eight classes proven by the current legacy
// runtime. Reserved/future identities are intentionally split into a separate
// source so enabling or replacing them never requires editing this catalog.
class LegacyEightClassesContentSource : public IClassContentSource
{
public:
    virtual bool Load(std::vector<PlayableClassDefinition>& out,
                      std::string* error = 0) const;
};

// Compatibility aggregate retained for existing callers. New composition
// roots should prefer explicitly composing sources with
// CompositeClassContentSource.
class LegacyClassContentSource : public IClassContentSource
{
public:
    virtual bool Load(std::vector<PlayableClassDefinition>& out,
                      std::string* error = 0) const;
};

// Compatibility entry point retained for old tests/patches.
bool RegisterLegacyEightClasses(PlayableClassRegistry& registry);

// Compatibility entry point retained while old tests/patches still call it.
bool RegisterLegacyClassCatalog(PlayableClassRegistry& registry);
}
}
