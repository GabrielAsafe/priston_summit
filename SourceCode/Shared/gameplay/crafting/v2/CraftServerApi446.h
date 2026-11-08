#pragma once
// SUMMIT 446 production API. Implementation remains in record translation unit.
#include "gameplay/crafting/v2/CraftCommit443.h"
class rsPLAYINFO;
namespace summit446 {
summit443::CraftResult CraftMakeFromChat446(rsPLAYINFO*,const char*,const char*);
bool CraftRecoverBeforeLogin446(const char* account,const char* name,rsPLAYINFO* loginSession);
}
