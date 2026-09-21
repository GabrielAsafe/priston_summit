#include "gameplay/classes/v2/content/ReservedClassCatalog.h"

#include "gameplay/classes/v2/PlayableClassRegistry.h"

namespace gameplay
{
namespace classes
{
namespace
{
PlayableClassDefinition ReservedAssassin()
{
    PlayableClassDefinition d;
    d.id = 9;
    d.legacyJobCode = 9;
    d.jobBitCode = 0u;
    d.brood = brood::Tempskron;
    d.key = "assassin";
    d.displayKey = "Assassine";
    d.enabled = false;
    d.capabilities = CapabilityBit(ClassCapability::Tempskron);
    // Source contains historical UI/model traces, but canonical creation stats,
    // specialization bits, armor row, and runtime assets remain unproven. Keep
    // creation disabled so adding those fields later is an explicit content act.
    return d;
}
}

bool ReservedClassesContentSource::Load(std::vector<PlayableClassDefinition>& out,
                                        std::string* error) const
{
    out.clear();
    out.push_back(ReservedAssassin());
    if (error) error->clear();
    return true;
}

bool RegisterReservedAssassinClass(PlayableClassRegistry& registry)
{
    return registry.Register(ReservedAssassin());
}
}
}
