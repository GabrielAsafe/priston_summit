#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
gport=(root/'Shared/gameplay/gathering/v2/IGatheringService.h').read_text()
mport=(root/'Shared/gameplay/mining/v2/IMiningService.h').read_text()
gservice=(root/'SrcServer/src/systems/gameplay/gathering/v2/GatheringService.h').read_text()
mservice=(root/'SrcServer/src/systems/gameplay/mining/v2/MiningService.h').read_text()
interaction=(root/'SrcServer/src/systems/gameplay/mining/v2/MiningInteractionService.h').read_text()
checks={
 'gathering port':'class IGatheringService' in gport and 'Harvest(' in gport,
 'mining port':'class IMiningService' in mport and 'Harvest(' in mport,
 'ports legacy clean':all(x not in gport+mport for x in ('rsPLAYINFO','smCHAR','SQLConnection')),
 'gathering implementation':'class GatheringService : public IGatheringService' in gservice,
 'mining implementation':'class MiningService : public IMiningService' in mservice,
 'interaction depends port':'IMiningService& mining' in interaction and '#include \"systems/gameplay/mining/v2/MiningService.h\"' not in interaction,
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if not all(checks.values()): sys.exit(1)
print('harvest service ports v1: PASS (Gathering/Mining contracts separated from implementations)')
