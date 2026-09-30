#pragma once

#include "gameplay/professions/v2/ProfessionService.h"

namespace gameplay
{
namespace professions
{
class SQLiteProfessionRepository;

ProfessionService& ServerProfessionService();
SQLiteProfessionRepository& ServerProfessionRepository();
}
}
