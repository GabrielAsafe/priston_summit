#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
red = root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
amber = root / 'SrcServer/src/systems/commerce/v2/legacy/LegacyMixtureResetKernel.inl'
wrapper = root / 'SrcServer/src/core/server/legacy_runtime/OnSever.cpp'
adapter = root / 'SrcServer/src/systems/commerce/v2/legacy/LegacyMixingAdapter.cpp'

signature = 'int rsRecvMixtureResetItem(rsPLAYINFO *lpPlayInfo, sMIXTURE_RESET_ITEM_SERVER *pMixtureResetItem_Server)'
failures = []
if signature in red.read_text(errors='replace'):
    failures.append('Mixture Reset kernel is still defined in RED OnServer')
if not amber.exists() or signature not in amber.read_text(errors='replace'):
    failures.append('Mixture Reset kernel is not owned by the commerce legacy adapter area')
if '#include "systems/commerce/v2/legacy/LegacyMixtureResetKernel.inl"' not in wrapper.read_text(errors='replace'):
    failures.append('server translation unit does not include the extracted Mixture Reset kernel')
adapter_text = adapter.read_text(errors='replace')
if 'rsRecvMixtureResetItem(player, &packet)' not in adapter_text:
    failures.append('IMixingService adapter no longer delegates to the extracted compatibility kernel')
if failures:
    for failure in failures:
        print('FAIL: ' + failure, file=sys.stderr)
    sys.exit(1)
print('mixing kernel ownership v1: PASS (RED definition=0, AMBER definition=1)')
