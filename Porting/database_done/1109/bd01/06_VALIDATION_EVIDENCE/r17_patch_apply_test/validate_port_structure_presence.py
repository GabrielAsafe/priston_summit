#!/usr/bin/env python3
from pathlib import Path
import csv,sys
ROOT=Path(__file__).resolve().parent
with (ROOT/'PORT_STRUCTURE_MANIFEST.tsv').open(encoding='utf-8') as f:
 expected={r['path'] for r in csv.DictReader(f,delimiter='\t')}
missing=sorted(x for x in expected if not (ROOT/x).is_file())
print('Port structural files expected:',len(expected))
print('Missing:',len(missing))
if missing:
 for x in missing[:100]:print(' -',x)
 sys.exit(1)
print('RESULT: PASS')
