#!/usr/bin/env python3
from pathlib import Path
import csv,hashlib,sys
ROOT=Path(__file__).resolve().parent
errors=[]
with (ROOT/'BASELINE_BUILD_SCRIPT_HASHES.tsv').open(encoding='utf-8') as f:
 rows=list(csv.DictReader(f,delimiter='\t'))
for r in rows:
 p=ROOT/r['path']
 if not p.is_file():errors.append('missing: '+r['path']);continue
 h=hashlib.sha256(p.read_bytes()).hexdigest()
 if h!=r['sha256']:errors.append('hash changed: '+r['path'])
print('Build/link scripts frozen:',len(rows))
if errors:
 print('FAIL:',len(errors));[print(' -',e) for e in errors];sys.exit(1)
print('RESULT: PASS')
