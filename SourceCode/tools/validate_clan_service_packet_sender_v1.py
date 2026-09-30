#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
s=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
pos=s.index('case OpCode::OPCODE_CLAN_SERVICE:')
end=s.index('case OpCode::OPCODE_CLAN_UPDATE:', pos)
block=s[pos:end]
checks=[
 ('clan service uses packet sender', 'ServerLegacyPacketSender().Send' in block),
 ('target serial becomes PlayerId', 'PlayerId(static_cast<std::uint32_t>(lpTransCharCommand2->LParam))' in block),
 ('packet size preserved', 'lpTransCharCommand2->size' in block),
 ('packet opcode preserved', 'lpTransCharCommand2->code' in block),
 ('no direct user lookup', 'srFindUserFromSerial' not in block),
 ('no direct socket send', 'lpsmSock->Send' not in block),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: raise SystemExit(1)
print('clan service packet sender boundary: PASS')
