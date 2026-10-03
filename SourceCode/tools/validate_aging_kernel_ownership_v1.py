#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
red=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
amber_path=root/'SrcServer/src/systems/commerce/v2/legacy/LegacyAgingKernel.inl'
amber=amber_path.read_text(errors='replace') if amber_path.exists() else ''
wrapper=(root/'SrcServer/src/core/server/legacy_runtime/OnSever.cpp').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/LegacyAgingAdapter.cpp').read_text(errors='replace')
fail=[]
for name in ('rsRecvAgingItem','rsRecvAgingUpgradeItem'):
    if name+'(' in red: fail.append(f'{name} still defined/referenced in RED OnServer')
    if name+'(' not in amber: fail.append(f'{name} missing from AMBER Aging kernel')
if 'LegacyAgingKernel.inl' not in wrapper: fail.append('server wrapper does not include Aging kernel')
if 'rsRecvAgingItem(player, &packet)' not in adapter: fail.append('Aging apply adapter delegation missing')
if 'rsRecvAgingUpgradeItem(player, &packet)' not in adapter: fail.append('Aging upgrade adapter delegation missing')
if fail:
    [print('FAIL: '+x,file=sys.stderr) for x in fail]; sys.exit(1)
print('aging kernel ownership v1: PASS (RED definitions=0, AMBER definitions=2)')
