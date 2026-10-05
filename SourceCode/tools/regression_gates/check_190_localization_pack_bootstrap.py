#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[2]
header = (root / 'Shared/content/v1/localization/content/LocalizationRuntime.h').read_text(encoding='utf-8')

required = (
    'SUMMIT_LOCALIZATION_PACK',
    'CompiledLocalizationEnabled(){return ActiveLocalizationSource()!=0;}',
)
for token in required:
    if token not in header:
        print('ERROR: localization bootstrap missing ' + token, file=sys.stderr)
        sys.exit(1)

# Two supported bootstrap contracts:
# 1) canonical candidate search introduced by PATCH 190;
# 2) newer executable-relative RuntimeContentPath authority present in this checkout.
uses_candidates = 'Content/packs/localization.sloc' in header
uses_runtime_resolver = 'ResolveRuntimePackPath("localization.sloc")' in header
if not (uses_candidates or uses_runtime_resolver):
    print('ERROR: localization has no default pack bootstrap without environment override', file=sys.stderr)
    sys.exit(1)

if uses_runtime_resolver:
    runtime_path = (root / 'Shared/content/v1/runtime/RuntimeContentPath.h')
    if not runtime_path.exists():
        print('ERROR: RuntimeContentPath.h missing', file=sys.stderr)
        sys.exit(1)
    runtime_text = runtime_path.read_text(encoding='utf-8')
    for token in ('RuntimeExecutableDirectory()', 'ResolveRuntimePackPath'):
        if token not in runtime_text:
            print('ERROR: runtime content resolver missing ' + token, file=sys.stderr)
            sys.exit(1)
    if 'Content' not in runtime_text or 'packs' not in runtime_text:
        print('ERROR: runtime content resolver does not target Content/packs', file=sys.stderr)
        sys.exit(1)

pack = root / 'Content/packs/localization.sloc'
if not pack.exists() or pack.stat().st_size == 0:
    print('ERROR: compiled localization pack missing/empty', file=sys.stderr)
    sys.exit(1)

print('PATCH 190 gate: OK (default localization pack active without env bootstrap)')
