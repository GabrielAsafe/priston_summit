#!/usr/bin/env python3
from pathlib import Path
import csv, hashlib, json, re, sys
root=Path(__file__).resolve().parent
manifest=root/'ORACLE_PARITY_MANIFEST.tsv'
promotions_file=root/'STRUCTURAL_PROMOTIONS.tsv'
policy_eq_file=root/'STRUCTURAL_POLICY_EQUIVALENCE.tsv'
policy_eq={}
if policy_eq_file.exists():
    with policy_eq_file.open(encoding='utf-8') as f:
        policy_eq={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
macro_eq_file=root/'STRUCTURAL_MACRO_EQUIVALENCE.tsv'
macro_eq={}
if macro_eq_file.exists():
    with macro_eq_file.open(encoding='utf-8') as f:
        macro_eq={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
compiler_adapt_file=root/'COMPILER_ADAPTATIONS.tsv'
compiler_adapt={}
if compiler_adapt_file.exists():
    with compiler_adapt_file.open(encoding='utf-8') as f:
        compiler_adapt={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
errors=[]
rows=[]
promotions={}
exact_moves={}
exact_moves_file=root/'STRUCTURAL_EXACT_MOVES.tsv'
if exact_moves_file.exists():
    with exact_moves_file.open(encoding='utf-8') as f:
        exact_moves={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
if promotions_file.exists():
    with promotions_file.open(encoding='utf-8') as f:
        promotions={r['path']:r for r in csv.DictReader(f,delimiter='\t')}
with manifest.open(encoding='utf-8') as f:
    for r in csv.DictReader(f,delimiter='\t'):
        rows.append(r)
        path=root/r['path']
        mode=r['mode']
        expected=r['sha256']
        if mode in ('oracle-wrapper','structured-safe'):
            oracle=root/r['oracle']
            if not oracle.is_file():
                errors.append(f'missing oracle: {r["oracle"]}')
                continue
            got=hashlib.sha256(oracle.read_bytes()).hexdigest()
            if got!=expected:
                errors.append(f'oracle hash mismatch: {r["oracle"]}')
            if mode=='oracle-wrapper':
                if not path.is_file():
                    errors.append(f'missing wrapper: {r["path"]}')
                    continue
                inc=str(oracle.relative_to(root/'Shared')).replace('\\','/')
                expected_wrapper='// Structural wrapper only; behavior is the pre_refactor oracle.\n#include "'+inc+'"\n'
                try: actual=path.read_text(encoding='utf-8')
                except Exception as e:
                    errors.append(f'wrapper unreadable: {r["path"]}: {e}')
                    continue
                if actual!=expected_wrapper:
                    errors.append(f'wrapper changed: {r["path"]}')
                text=oracle.read_text(errors='ignore')
                if any(tok in text for tok in ('__FILE__','__BASE_FILE__','__INCLUDE_LEVEL__')):
                    errors.append(f'location-sensitive macro left inside moved oracle: {r["oracle"]}')
                for line in text.splitlines():
                    m=re.match(r'\s*#\s*include\s*"([^"]+)"',line)
                    if m and m.group(1).startswith('.'):
                        errors.append(f'relative include left inside moved oracle: {r["oracle"]}: {m.group(1)}')
            else:
                promo=promotions.get(r['path'])
                if not promo:
                    errors.append(f'missing structural promotion record: {r["path"]}')
                    continue
                if not path.is_file():
                    errors.append(f'missing structured file: {r["path"]}')
                    continue
                current=hashlib.sha256(path.read_bytes()).hexdigest()
                if current!=promo['structured_sha256']:
                    errors.append(f'structured-safe file changed: {r["path"]}')
                if promo['oracle_sha256']!=expected:
                    errors.append(f'promotion/oracle hash mismatch: {r["path"]}')
        elif mode=='structured-policy-equivalent':
            eq=policy_eq.get(r['path'])
            if not eq:
                errors.append(f'missing policy-equivalence record: {r["path"]}')
                continue
            oracle=root/r['oracle']
            if not path.is_file() or not oracle.is_file():
                errors.append(f'missing policy-equivalent file: {r["path"]}')
                continue
            if hashlib.sha256(path.read_bytes()).hexdigest()!=eq['structured_sha256']:
                errors.append(f'policy-equivalent target changed: {r["path"]}')
            if hashlib.sha256(oracle.read_bytes()).hexdigest()!=expected or eq['oracle_sha256']!=expected:
                errors.append(f'policy-equivalent oracle mismatch: {r["path"]}')
        elif mode=='structured-macro-equivalent':
            eq=macro_eq.get(r['path'])
            if not eq:
                errors.append(f'missing macro-equivalence record: {r["path"]}')
                continue
            oracle=root/r['oracle']
            if not path.is_file() or not oracle.is_file():
                errors.append(f'missing macro-equivalent file: {r["path"]}')
                continue
            if hashlib.sha256(path.read_bytes()).hexdigest()!=eq['structured_sha256']:
                errors.append(f'macro-equivalent target changed: {r["path"]}')
            if hashlib.sha256(oracle.read_bytes()).hexdigest()!=expected or eq['oracle_sha256']!=expected:
                errors.append(f'macro-equivalent oracle mismatch: {r["path"]}')
        elif mode=='structured-compiler-adaptation':
            eq=compiler_adapt.get(r['path'])
            if not eq:
                errors.append(f'missing compiler-adaptation record: {r["path"]}')
                continue
            oracle=root/r['oracle']
            if not path.is_file() or not oracle.is_file():
                errors.append(f'missing compiler-adaptation file: {r["path"]}')
                continue
            if hashlib.sha256(path.read_bytes()).hexdigest()!=eq['adapted_structured_sha256']:
                errors.append(f'compiler-adaptation target changed: {r["path"]}')
            if hashlib.sha256(oracle.read_bytes()).hexdigest()!=expected or eq['oracle_sha256']!=expected:
                errors.append(f'compiler-adaptation oracle mismatch: {r["path"]}')
        elif mode=='structured-exact':
            move=exact_moves.get(r['path'])
            if not move:
                errors.append(f'missing structured-exact record: {r["path"]}')
                continue
            oracle=root/move['oracle']
            implementation=root/move['implementation']
            if not oracle.is_file() or not implementation.is_file() or not path.is_file():
                errors.append(f'missing structured-exact file: {r["path"]}')
                continue
            oracle_hash=hashlib.sha256(oracle.read_bytes()).hexdigest()
            impl_hash=hashlib.sha256(implementation.read_bytes()).hexdigest()
            wrapper_hash=hashlib.sha256(path.read_bytes()).hexdigest()
            if oracle_hash!=expected or impl_hash!=expected or move['implementation_sha256']!=expected:
                errors.append(f'structured-exact implementation mismatch: {r["path"]}')
            if wrapper_hash!=move['wrapper_sha256']:
                errors.append(f'structured-exact wrapper changed: {r["path"]}')
            inc=str(implementation.relative_to(root/'Shared')).replace('\\','/')
            expected_wrapper='// Structural target wrapper; implementation is byte-exact pre_refactor behavior.\n#include "'+inc+'"\n'
            try: actual=path.read_text(encoding='utf-8')
            except Exception as e:
                errors.append(f'structured-exact wrapper unreadable: {r["path"]}: {e}')
                continue
            if actual!=expected_wrapper:
                errors.append(f'structured-exact wrapper content mismatch: {r["path"]}')
        elif mode.startswith('direct-exact'):
            if not path.is_file():
                errors.append(f'missing direct file: {r["path"]}')
                continue
            got=hashlib.sha256(path.read_bytes()).hexdigest()
            if got!=expected:
                errors.append(f'direct hash mismatch: {r["path"]}')
        else:
            errors.append(f'unknown mode {mode}: {r["path"]}')

# SQLite is intentionally retained as dormant future infrastructure.
sqlite=root/'SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp'
if not sqlite.is_file():
    errors.append('future SQLiteConnection.cpp is missing from the port structure')

summary=json.loads((root/'ORACLE_PARITY_SUMMARY.json').read_text(encoding='utf-8'))
if len(rows)!=summary['changed_same_path_effective_source_files']:
    errors.append('manifest/summary row count mismatch')

print(f'Oracle entries : {len(rows)}')
print(f'Wrappers       : {sum(r["mode"]=="oracle-wrapper" for r in rows)}')
print(f'Structured safe: {sum(r["mode"]=="structured-safe" for r in rows)}')
print(f'Structured exact: {sum(r["mode"]=="structured-exact" for r in rows)}')
print(f'Macro equivalent: {sum(r["mode"]=="structured-macro-equivalent" for r in rows)}')
print(f'Policy equivalent: {sum(r["mode"]=="structured-policy-equivalent" for r in rows)}')
print(f'Compiler adaptation: {sum(r["mode"]=="structured-compiler-adaptation" for r in rows)}')
print(f'Direct exact   : {sum(r["mode"].startswith("direct-exact") for r in rows)}')
if errors:
    print(f'FAIL           : {len(errors)}')
    for e in errors[:100]: print(' -',e)
    sys.exit(1)
print('RESULT         : PASS')
