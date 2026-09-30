#pragma once

#include <windows.h>

class rsPLAYINFO;
class smCHAR;

namespace gameplay
{
namespace companions
{
INT64 AwardExperience(rsPLAYINFO* owner, smCHAR* killedMonster, float partyShareRate);
}
}
