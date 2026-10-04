#!/usr/bin/env python3
from pathlib import Path
import sys

root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/progression/battle_pass/BattlePass.cpp').read_text(errors='replace')
svc=(root/'Shared/gameplay/progression/v2/BattlePassProgressService.h').read_text(errors='replace')
port=(root/'SrcServer/src/systems/progression/battle_pass/v2/legacy/LegacyBattlePassProgressMutationPort.h').read_text(errors='replace')
start=cpp.index('void BattlePass::AddExpBattlePass')
body=cpp[start:]
checks={
 'handler delegates progression application boundary': 'BattlePassProgressApplicationService::AddExperience' in body,
 'legacy one-level threshold removed from handler': 'UserBP_Exp >= 999' not in body,
 'green progression rule owned by service': 'BattlePassProgressService::AddExperience' in port,
 'level and exp persist together in mutation port': 'SET BP_Exp=%d, BP_Level=%d' in port,
 'handler no longer owns BP_Info persistence': 'BP_Info' not in body and 'SQL::GetInstance' not in body,
 'service supports multiple levels': 'while(accumulated >= ExperiencePerLevel' in svc,
 'service has explicit max level': 'MaxLevel = 20' in svc,
 'inactive pass remains unchanged': 'current.passType <= 0' in svc,
}
failed=[n for n,v in checks.items() if not v]
if failed:
 for n in failed: print('FAIL:',n,file=sys.stderr)
 raise SystemExit(1)
print('battle pass progress service v2: PASS')
