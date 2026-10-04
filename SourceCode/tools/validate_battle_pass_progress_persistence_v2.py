#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/progression/battle_pass/BattlePass.cpp').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/progression/battle_pass/v2/legacy/LegacyBattlePassProgressMutationPort.h').read_text(errors='replace')
service=(root/'Shared/gameplay/progression/v2/BattlePassProgressApplicationService.h').read_text(errors='replace')
start=cpp.index('void BattlePass::AddExpBattlePass')
body=cpp[start:]
checks={
 'handler delegates application service': 'BattlePassProgressApplicationService::AddExperience' in body,
 'handler has no BP_Info SQL': 'BP_Info' not in body and 'SQL::GetInstance' not in body,
 'amber adapter owns BP_Info load': 'SELECT * FROM [BattlePass].[dbo].[BP_Info]' in adapter,
 'amber adapter owns BP_Info save': 'UPDATE [BattlePass].[dbo].[BP_Info] SET BP_Exp=%d, BP_Level=%d' in adapter,
 'adapter applies green progression rule': 'BattlePassProgressService::AddExperience' in adapter,
 'green application boundary is legacy-free': 'SQL::' not in service and 'rsPLAYINFO' not in service,
}
failed=[name for name,value in checks.items() if not value]
if failed:
    for name in failed: print('FAIL:',name,file=sys.stderr)
    raise SystemExit(1)
print('battle pass progress persistence v2: PASS')
