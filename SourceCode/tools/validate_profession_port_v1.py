#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
port=(root/'Shared/gameplay/professions/v2/IProfessionService.h').read_text()
service=(root/'Shared/gameplay/professions/v2/ProfessionService.h').read_text()
types=(root/'Shared/gameplay/gathering/v2/GatheringTypes.h').read_text()
mining_h=(root/'SrcServer/src/systems/gameplay/mining/v2/MiningService.h').read_text()
mining_cpp=(root/'SrcServer/src/systems/gameplay/mining/v2/MiningService.cpp').read_text()
checks={
 'profession port exists':'class IProfessionService' in port,
 'port owns no persistence':'IProfessionRepository' not in port and 'SQL' not in port,
 'service implements port':'class ProfessionService : public IProfessionService' in service,
 'gathering types use types only':'ProfessionTypes.h' in types and 'ProfessionService.h' not in types,
 'mining depends on port':'IProfessionService& professions' in mining_h,
 'mining no concrete include':'#include \"gameplay/professions/v2/ProfessionService.h\"' not in mining_h,
 'mining constructor port':'IProfessionService& professions' in mining_cpp,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('profession service port v1: PASS (harvest domain no longer depends on concrete ProfessionService)')
