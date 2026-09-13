#!/usr/bin/env python3
from pathlib import Path
import csv,hashlib,subprocess,sys
root=Path(__file__).resolve().parent
errors=[]
with (root/'PRE_REFACTOR_SOURCE_MANIFEST.tsv').open(encoding='utf-8') as f:
    rows=list(csv.DictReader(f,delimiter='\t'))
# Files listed in ORACLE_PARITY_MANIFEST.tsv are intentionally allowed to differ
# byte-for-byte when one of the stronger structural/macro/policy validators proves
# behavioral equivalence to the pre_refactor oracle.  Only baseline files outside
# that manifest are required to remain same-path byte exact here.
with (root/'ORACLE_PARITY_MANIFEST.tsv').open(encoding='utf-8') as f:
    oracle_rows=list(csv.DictReader(f,delimiter='\t'))
managed_paths={r['path'] for r in oracle_rows}
for r in rows:
    p=root/r['path']
    if not p.is_file():
        errors.append('missing baseline source: '+r['path']); continue
    if r['path'] not in managed_paths:
        got=hashlib.sha256(p.read_bytes()).hexdigest()
        if got!=r['pre_refactor_sha256']:
            errors.append('baseline hash mismatch: '+r['path'])
# Delegate structured move/promotion checks to the stronger per-file validator.
proc=subprocess.run([sys.executable,str(root/'validate_oracle_parity.py')],capture_output=True,text=True)
if proc.returncode:
    errors.append('oracle/structural validator failed:\n'+proc.stdout+proc.stderr)
# Structural proofs: active extracted helpers must stay token-equivalent, and
# relocated exact implementations must resolve quoted includes identically.
for validator in ('validate_structural_shared_proof.py','validate_structural_shared_include_resolution.py','validate_structured_safe_parity.py','validate_structured_safe_strict_preprocessor.py','validate_structural_macro_equivalence.py','validate_structural_macro_equivalence_strict.py','validate_structural_policy_equivalence.py','validate_structural_policy_equivalence_strict.py','validate_compiler_adaptations.py','validate_active_shared_graph.py','validate_structural_exact_includes.py','validate_build_source_graph.py','validate_active_nonbaseline_graph.py','validate_port_structure_presence.py','validate_build_scripts.py'):
    proc=subprocess.run([sys.executable,str(root/validator)],capture_output=True,text=True)
    if proc.returncode:
        errors.append(validator+' failed:\n'+proc.stdout+proc.stderr)

# No legacy translation unit may disappear; new translation units are explicitly inventoried.
baseline_cpp={r['path'] for r in rows if (r['path'].startswith('SrcGame/src/') or r['path'].startswith('SrcServer/src/')) and Path(r['path']).suffix.lower() in ('.cpp','.c','.cc')}
current_cpp=set()
for base in ('SrcGame/src','SrcServer/src'):
    for p in (root/base).rglob('*'):
        if p.is_file() and p.suffix.lower() in ('.cpp','.c','.cc'):
            current_cpp.add(str(p.relative_to(root)).replace('\\','/'))
removed=sorted(baseline_cpp-current_cpp)
added=sorted(current_cpp-baseline_cpp)
for p in removed: errors.append('compiled source removed: '+p)
allowed_added={'SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp'}
for p in added:
    if p not in allowed_added: errors.append('unapproved compiled source added: '+p)
print(f'Baseline source files : {len(rows)}')
print(f'Baseline TUs          : {len(baseline_cpp)}')
print(f'Added TUs             : {len(added)}')
for p in added: print('  +',p, '(authorized)' if p in allowed_added else '(UNAPPROVED)')
if errors:
    print('FAIL:',len(errors))
    for e in errors[:100]: print(' -',e)
    sys.exit(1)
print('RESULT                : PASS')
