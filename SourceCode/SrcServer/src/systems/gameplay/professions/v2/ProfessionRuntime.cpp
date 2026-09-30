#include "systems/gameplay/professions/v2/ProfessionRuntime.h"

#include "systems/gameplay/professions/v2/SQLiteProfessionRepository.h"
#include "systems/gameplay/events/v2/GameplayEventRuntime.h"

namespace gameplay
{
namespace professions
{
SQLiteProfessionRepository& ServerProfessionRepository()
{
    static SQLiteProfessionRepository repository;
    return repository;
}

ProfessionService& ServerProfessionService()
{
    static ProfessionService service(&ServerProfessionRepository(), &events::ServerGameplayEventHub());
    return service;
}
}
}
