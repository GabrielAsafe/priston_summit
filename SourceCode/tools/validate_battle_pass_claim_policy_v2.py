#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
cpp=(root/'SrcServer/src/systems/progression/battle_pass/BattlePass.cpp').read_text(errors='replace')
if 'BattlePassClaimPolicy::Evaluate' not in cpp:
    raise SystemExit('BattlePass handler is not consuming V2 claim policy')
legacy='((Shop->item.Tag % 2 == 0) && BP_Info > 0)'
if legacy in cpp:
    raise SystemExit('legacy pass entitlement expression remains in handler')
if 'UPDATE [BattlePass].[dbo].[BP_Info] SET BP_Resg' not in cpp:
    raise SystemExit('claim persistence path changed unexpectedly')
print('battle pass claim policy validator: PASS')
