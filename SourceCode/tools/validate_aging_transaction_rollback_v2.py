#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
service=(ROOT/'Shared/gameplay/commerce/v2/AgingService.h').read_text(errors='replace')
adapter=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyAgingAdapter.cpp').read_text(errors='replace')
fail=[]
for needle,label in [('RollbackFailed','AgingCode rollback result'),('InventorySnapshotRollback rollback','snapshot rollback guard'),('sAging_Recv.Result != FALSE','Apply logical result'),('RestoreAgingInventoryIfCaptured(rollback)','failure restores inventory')]:
    if needle not in (service+adapter): fail.append(label)
if adapter.count('InventorySnapshotRollback rollback') != 2: fail.append('expected rollback on Apply and Upgrade')
if fail:[print('FAIL:',x,file=sys.stderr) for x in fail];sys.exit(1)
print('aging transaction rollback v2: PASS')
