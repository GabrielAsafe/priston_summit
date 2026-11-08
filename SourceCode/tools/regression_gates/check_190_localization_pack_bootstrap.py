#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[2]
header = (root / 'Shared/content/v1/localization/content/LocalizationRuntime.h').read_text(encoding='utf-8')

required = (
    'ActiveTextResourceSource()',
    '__summit_internal__/localization.sloc',
    'CompiledLocalizationEnabled(){return ActiveLocalizationSource()!=0;}',
)
for token in required:
    if token not in header:
        print('ERROR: merged localization bootstrap missing ' + token, file=sys.stderr)
        sys.exit(1)

pack = root / 'Content/packs/text_resources.sres'
if not pack.is_file() or pack.stat().st_size == 0:
    print('ERROR: consolidated SRES pack missing/empty', file=sys.stderr)
    sys.exit(1)
if (root/'Content/packs/localization.sloc').exists():
    print('ERROR: old SLOC still exists', file=sys.stderr)
    sys.exit(1)

sys.path.insert(0,str(root/'tools/content_port'))
from sres_embedded_localization import extract_sloc_bytes
try:
    data=extract_sloc_bytes(pack)
    if len(data)<80:
        raise ValueError('embedded catalog too small')
except (OSError,ValueError) as error:
    print('ERROR: '+str(error),file=sys.stderr)
    sys.exit(1)

print('PATCH 190 gate: OK (localization embedded in sole SRES pack)')
