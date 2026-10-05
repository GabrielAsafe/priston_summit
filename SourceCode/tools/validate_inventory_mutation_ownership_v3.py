#!/usr/bin/env python3
from pathlib import Path
import re, sys

ROOT=Path(__file__).resolve().parents[1]
ALLOWED={
 'SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryRuntime.cpp',
 'SrcServer/src/systems/commerce/v2/legacy/LegacyAgingKernel.inl',
 'SrcServer/src/systems/commerce/v2/legacy/LegacyCraftingKernel.inl',
 'SrcServer/src/systems/commerce/v2/legacy/LegacyMixtureResetKernel.inl',
}
MUTATIONS=('rsAddInvenItem','rsDeleteInvenItem')
violations=[]
counts={name:0 for name in MUTATIONS}
paths=list((ROOT/'SrcServer').rglob('*')) + [ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl']
for path in paths:
  if path.suffix not in ('.cpp','.inl'): continue
  if '.before_' in path.name or 'legacy_oracle' in path.parts: continue
  rel=path.relative_to(ROOT).as_posix()
  text=path.read_text(encoding='utf-8',errors='replace')
  for name in MUTATIONS:
   for m in re.finditer(rf'\b{name}\s*\(',text):
    # Function declaration/definition is the private legacy kernel, not a consumer call.
    line=text[text.rfind('\n',0,m.start())+1:text.find('\n',m.start())]
    if re.match(r'\s*(?:extern\s+)?int\s+'+name+r'\s*\(',line):
     continue
    counts[name]+=1
    if rel not in ALLOWED:
     lineno=text.count('\n',0,m.start())+1
     violations.append(f'{rel}:{lineno}: direct {name} consumer')

for rel in sorted(ALLOWED):
 if not (ROOT/rel).is_file(): violations.append(f'missing authorized legacy kernel: {rel}')

if violations:
 for v in violations: print('FAIL: '+v,file=sys.stderr)
 raise SystemExit(1)
print('inventory mutation ownership v3: PASS '
      f'(direct adds={counts["rsAddInvenItem"]}, deletes={counts["rsDeleteInvenItem"]}; all confined to authorized legacy kernels)')
