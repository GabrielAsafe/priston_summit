#!/usr/bin/env python3
from pathlib import Path
import csv, hashlib, sys

ROOT = Path(__file__).resolve().parent
manifest_path = ROOT / 'COMPILER_ADAPTATIONS.tsv'
oracle_rows = {r['path']: r for r in csv.DictReader((ROOT/'ORACLE_PARITY_MANIFEST.tsv').open(encoding='utf-8'), delimiter='\t')}
rows = list(csv.DictReader(manifest_path.open(encoding='utf-8'), delimiter='\t')) if manifest_path.exists() else []

ADAPT_BLOCK = '''#if defined(__MINGW32__) || defined(__MINGW64__)\n// Compiler-only compatibility overload.  The pre_refactor cE_chip.cpp calls\n// SetFontTextColor(HDC, COLORREF), while the available legacy declaration is\n// SetFontTextColor(COLORREF).  Keep the original call site/policy unchanged and\n// map only the MinGW-only overload to the equivalent Win32 GDI operation.\nstatic inline void SetFontTextColor(HDC dc, COLORREF color)\n{\n\tSetTextColor(dc, color);\n}\n#endif\n\n'''

def sha_bytes(b):
    return hashlib.sha256(b).hexdigest()

def sha_file(p):
    return sha_bytes(p.read_bytes())

errors=[]
for r in rows:
    rel=r['path']; p=ROOT/rel
    m=oracle_rows.get(rel)
    if not m or m['mode']!='structured-compiler-adaptation':
        errors.append('manifest mode mismatch: '+rel); continue
    if not p.is_file():
        errors.append('missing adapted source: '+rel); continue
    current=sha_file(p)
    if current!=r['adapted_structured_sha256']:
        errors.append('adapted source hash changed: '+rel); continue
    oracle=ROOT/m['oracle']
    if not oracle.is_file() or sha_file(oracle)!=r['oracle_sha256'] or m['sha256']!=r['oracle_sha256']:
        errors.append('oracle hash mismatch: '+rel); continue
    text=p.read_text(encoding='utf-8')
    if text.count(ADAPT_BLOCK)!=1:
        errors.append('expected isolated MinGW adaptation block not found exactly once: '+rel); continue
    stripped=text.replace(ADAPT_BLOCK,'',1).encode('utf-8')
    if sha_bytes(stripped)!=r['pre_adaptation_structured_sha256']:
        errors.append('removing compiler adaptation does not recover frozen pre-adaptation structural source: '+rel)
    if 'SetFontTextColor(dc, color);' not in text:
        errors.append('legacy call path missing: '+rel)
    if '\tSetTextColor(dc, color);' not in text:
        errors.append('MinGW GDI compatibility mapping missing: '+rel)

print('Compiler adaptations:', len(rows))
if errors:
    print('FAIL:',len(errors))
    for e in errors: print(' -',e)
    sys.exit(1)
print('RESULT              : PASS')
