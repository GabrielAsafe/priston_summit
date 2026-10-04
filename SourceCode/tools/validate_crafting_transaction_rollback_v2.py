#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
service=(ROOT/'Shared/gameplay/commerce/v2/CraftingService.h').read_text(errors='replace')
adapter=(ROOT/'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingAdapter.cpp').read_text(errors='replace')
fail=[]
def need(t,n,l):
    if n not in t: fail.append(l)
need(service,'RollbackFailed','CraftingCode rollback failure')
need(adapter,'InventorySnapshotRollback rollback','snapshot transaction guard')
need(adapter,'sCraftItem_Recv.Result == TRUE','Craft uses logical result, not transport return only')
need(adapter,'packet.Result != TRUE','Wing uses logical result')
need(adapter,'sSkinChange_Recv.Result != TRUE','SkinChange uses logical result')
need(adapter,'sSmeltingItem_Recv.Result != TRUE','Smelting uses logical result')
need(adapter,'g_sManufactureItem_Recv.Result != TRUE','Manufacture uses logical result')
need(adapter,'RestoreInventoryIfCaptured(rollback)','failure path restores snapshot')
if adapter.count('InventorySnapshotRollback rollback') < 6: fail.append('expected rollback guards on Craft + 5 mutating auxiliary paths')
if fail:
    [print('FAIL:',x,file=sys.stderr) for x in fail]; sys.exit(1)
print('crafting transaction rollback v2: PASS')
