#include "systems/gameplay/mining/v2/MiningRuntime.h"

#include "systems/gameplay/events/v2/GameplayEventRuntime.h"
#include "systems/gameplay/professions/v2/ProfessionRuntime.h"

namespace gameplay
{
namespace mining
{
MiningService& ServerMiningService()
{
    static MiningService service(professions::ServerProfessionService(),
                                 &events::ServerGameplayEventHub());
    return service;
}
}
}
