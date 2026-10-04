#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
service=(ROOT/'Shared/gameplay/commerce/v2/MixingService.h').read_text(errors='replace')
adapter=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyMixingAdapter.cpp').read_text(errors='replace')
fail=[]
for needle,label in [('RollbackFailed','MixingCode rollback result'),('InventorySnapshotRollback rollback','snapshot rollback guard'),('sMixtureReset_Recv.Result == TRUE','logical reset result'),('rollback.Restore().Restored()','failure restores inventory')]:
    if needle not in service+adapter: fail.append(label)
if fail:[print('FAIL:',x,file=sys.stderr) for x in fail];sys.exit(1)
print('mixing transaction rollback v2: PASS')
