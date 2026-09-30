#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/social/clan/v2/ClanUpdateNotifier.h').read_text()
adapter=(root/'SrcServer/src/systems/social/clan/v2/legacy/LegacyClanUpdateNotifier.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
start=onserver.rindex('int rsClanUpdate('); end=onserver.index('\nint rsRecvYahooMotion', start); body=onserver[start:end]
checks=[
 ('green update notifier contract','class IClanUpdateNotifier' in contract and 'ClanUpdateRecipient' in contract),
 ('recipient preserves name and token','characterName' in contract and 'verification' in contract),
 ('adapter preserves opcode','OPCODE_CLAN_UPDATE' in adapter),
 ('adapter preserves source serial','packet.WParam = static_cast<DWORD>(source.value)' in adapter),
 ('adapter preserves verified legacy lookup','FindUserFromCodeName' in adapter and 'verification' in adapter),
 ('adapter sends via packet sender','packets_.Send' in adapter),
 ('legacy parser still RecvClanCommand','RecvClanCommand' in body),
 ('handler converts parsed recipients','ClanUpdateRecipient' in body),
 ('handler delegates notifier','ServerLegacyClanUpdateNotifier().Notify' in body),
 ('handler no direct target lookup','FindUserFromCodeName' not in body),
 ('handler no direct socket send','lpsmSock->Send' not in body),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: raise SystemExit(1)
print('clan update notifier boundary: PASS')
