#!/usr/bin/env python3
from pathlib import Path
import csv,hashlib,sys
root=Path(__file__).resolve().parent
proof=root/'STRUCTURAL_SHARED_PROOF.tsv'
errors=[]; rows=[]
with proof.open(encoding='utf-8') as f:
    rows=list(csv.DictReader(f,delimiter='\t'))
for r in rows:
    p=root/r['path']
    if not p.is_file():
        errors.append('missing: '+r['path']); continue
    got=hashlib.sha256(p.read_bytes()).hexdigest()
    if got!=r['sha256']:
        errors.append(f"hash mismatch: {r['path']} expected={r['sha256']} got={got}")
    if r.get('status')!='token-contiguous':
        errors.append('unapproved status: '+r['path']+' '+r.get('status',''))
print(f'Structural Shared proof files: {len(rows)}')
print(f'Hash/status failures          : {len(errors)}')
if errors:
    for e in errors[:100]: print(' -',e)
    sys.exit(1)
print('RESULT                       : PASS')
