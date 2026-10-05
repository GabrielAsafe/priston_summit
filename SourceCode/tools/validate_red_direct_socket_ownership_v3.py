#!/usr/bin/env python3
from pathlib import Path
import collections,json,re,sys
ROOT=Path(__file__).resolve().parents[1]
base=json.loads((ROOT/"tools/architecture/direct_socket_send_baseline.json").read_text())['files']
pat=re.compile(rb"\b(smWsock[A-Za-z0-9_]*|lpsmSock)\s*->\s*Send\s*\(")
fail=[]
active_red_sends=[]
for rel,entry in base.items():
    path=ROOT/rel
    if not path.exists() or entry.get('role')!='RED': continue
    hits=collections.Counter(x.decode('ascii') for x in pat.findall(path.read_bytes()))
    if hits: active_red_sends.append((rel,dict(hits)))
if active_red_sends:
    for rel,hits in active_red_sends: fail.append(f"active RED direct send owner remains: {rel} {hits}")
# Inactive templates are allowed to preserve historical direct-send text only while no production source includes them.
inactive=[rel for rel,e in base.items() if e.get('role')=='INACTIVE_TEMPLATE']
roots=[ROOT/'SrcGame/src',ROOT/'SrcServer/src',ROOT/'Shared']
for rel in inactive:
    name=Path(rel).name
    include_re=re.compile(r'^\s*#\s*include\s*[<"]([^>"]*'+re.escape(name)+r')[>"]',re.M)
    for root in roots:
        if not root.exists(): continue
        for path in root.rglob('*'):
            if not path.is_file() or path==(ROOT/rel): continue
            if path.suffix.lower() not in {'.cpp','.cc','.cxx','.h','.hpp','.inl'}: continue
            text=path.read_text(errors='ignore')
            if include_re.search(text): fail.append(f"inactive template gained production include: {rel} <- {path.relative_to(ROOT)}")
# The only kernel direct send must remain inside the packet queue transport implementation.
kernels=[rel for rel,e in base.items() if e.get('role')=='KERNEL']
if kernels != ['Shared/network/transport/legacy/SmWSockPacketSendQueueLegacyCommon.inl']:
    fail.append('unexpected KERNEL direct-send owner set')
else:
    text=(ROOT/kernels[0]).read_text(errors='ignore')
    if text.count('lpsmSock->Send(')!=1 or 'int PopSendPacketQue(smWINSOCK *lpsmSock)' not in text:
        fail.append('legacy socket queue kernel send shape changed')
if fail:
    print('\n'.join('FAIL: '+x for x in fail),file=sys.stderr); raise SystemExit(1)
print(f'red direct socket ownership v3: PASS (active_RED_direct_sends=0, inactive_templates={len(inactive)}, kernel_owners={len(kernels)})')
