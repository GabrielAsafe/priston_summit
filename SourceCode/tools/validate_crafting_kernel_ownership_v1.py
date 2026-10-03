#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
red=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
amber_path=root/'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingKernel.inl'
amber=amber_path.read_text(errors='replace') if amber_path.exists() else ''
wrapper=(root/'SrcServer/src/core/server/legacy_runtime/OnSever.cpp').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingAdapter.cpp').read_text(errors='replace')
names=('rsRecvCraftItem','rsRecvMakeLinkCore','rsRecvUseLinkCore','rsRecvWingItem','rsRecvSkinChangeItem','rsRecvSmeltingItem','rsRecvManufactureItem')
fail=[]
for name in names:
    if name+'(' in red: fail.append(f'{name} still present in RED OnServer')
    if name+'(' not in amber: fail.append(f'{name} missing from AMBER Crafting kernel')
if 'LegacyCraftingKernel.inl' not in wrapper: fail.append('server wrapper does not include Crafting kernel')
for call in ('rsRecvCraftItem(player, &packet)','rsRecvMakeLinkCore(player, &packet)','rsRecvUseLinkCore(player, &packet)','rsRecvWingItem(player, &packet)','rsRecvSkinChangeItem(player, &packet)','rsRecvSmeltingItem(player, &packet)','rsRecvManufactureItem(player, &packet)'):
    if call not in adapter: fail.append('adapter delegation missing: '+call)
if fail:
    [print('FAIL: '+x,file=sys.stderr) for x in fail]; sys.exit(1)
print('crafting kernel ownership v1: PASS (RED definitions=0, AMBER definitions=7)')
