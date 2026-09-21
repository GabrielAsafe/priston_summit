#include "systems/gameplay/professions/v2/ProfessionRuntime.h"

namespace gameplay
{
namespace professions
{
ProfessionService& ServerProfessionService()
{
    static ProfessionService service;
    return service;
}
}
}
