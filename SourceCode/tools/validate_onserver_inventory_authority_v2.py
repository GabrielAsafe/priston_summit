#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
path=ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
text=path.read_text(encoding='utf-8',errors='replace')
# Definitions of rsAdd/rsDelete/rsFind remain the AMBER legacy kernel; consumers in
# the packet router must not call those mutation functions directly anymore.
def body_between(start_token,end_token):
 s=text.find(start_token); e=text.find(end_token,s+len(start_token))
 return text[s:e if e>=0 else len(text)] if s>=0 else ''
router=body_between('int OnServer(', '\nint ')
if not router:
 # Current branch names the dispatcher differently; use opcode region to EOF-safe boundary.
 s=text.find('case OpCode::OPCODE_SAVETHROWITEM:')
 e=text.find('\nint ',s)
 router=text[s:e if e>=0 else len(text)]
checks=[
 ('inventory runtime included','systems/inventory/v2/legacy/LegacyInventoryRuntime.h' in text),
 ('add helper delegates boundary','SummitOnServerInventoryAdd' in text and 'ServerLegacyInventoryRuntime().Add(' in text),
 ('remove helper delegates boundary','SummitOnServerInventoryRemove' in text and 'ServerLegacyInventoryRuntime().Remove(' in text),
 ('express delivery delegates add','SummitOnServerInventoryAdd(lpPlayInfo, lpItem->CODE' in text),
 ('throw anti-cheat delegates remove','!SummitOnServerInventoryRemove(' in text and ').Applied()' in text),
 ('pickup delegates add','const gameplay::inventory::InventoryMutationResult inventoryAdd' in text),
 ('tier5 completion requires actual add','inventoryAdd.code == gameplay::inventory::InventoryMutationCode::Applied' in text),
 ('use-item delegates remove','(void)SummitOnServerInventoryRemove(' in text),
]
# Direct mutation calls are allowed only in the rs* function definitions themselves.
consumer=text
for name in ('rsFindInvenItem','rsDeleteInvenItem','rsAddInvenItem'):
 # strip declaration and full function definition approximately by brace matching
 consumer=re.sub(rf'int\s+{name}\s*\([^;]*\);', '', consumer)
 start=consumer.find(f'int\t{name}(')
 if start<0: start=consumer.find(f'int {name}(')
 if start>=0:
  brace=consumer.find('{',start); depth=0; end=None
  for i in range(brace,len(consumer)):
   if consumer[i]=='{': depth+=1
   elif consumer[i]=='}':
    depth-=1
    if depth==0: end=i+1; break
  if end: consumer=consumer[:start]+consumer[end:]
for name in ('rsAddInvenItem','rsDeleteInvenItem'):
 checks.append((f'no direct {name} consumer calls', re.search(rf'\b{name}\s*\(', consumer) is None))
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed:
 print('onserver inventory authority v2: FAIL: '+', '.join(failed),file=sys.stderr);raise SystemExit(1)
print('onserver inventory authority v2: PASS (active packet consumers delegate inventory mutation)')
