#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/social/clan/v2/ClanChatService.h').read_text()
adapter=(root/'SrcServer/src/systems/social/clan/v2/legacy/LegacyClanChatAdapter.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
# isolate around /CLAN>
pos=onserver.index('rsCompString("/CLAN>"')
block=onserver[pos:pos+1800]
checks=[
 ('green clan chat service', 'class IClanChatService' in contract and 'Broadcast(PlayerId sender' in contract),
 ('adapter checks sender membership', 'senderClan.IsMember()' in adapter),
 ('adapter resolves sender snapshot', 'ServerLegacyPlayerAdapter().TryGet(sender, senderPlayer)' in adapter),
 ('adapter preserves name formatting', 'characterName + "> " + text' in adapter),
 ('adapter preserves clan channel color', 'ChatMessage message(formatted, 5, ChatDeliveryKind::Whisper)' in adapter),
 ('adapter enumerates online handles', 'players_.OnlineHandles()' in adapter),
 ('adapter filters same clan', 'SameClan(senderClan, recipientClan)' in adapter),
 ('adapter uses chat delivery', 'ServerLegacyChatDeliveryAdapter().SendToPlayer' in adapter),
 ('parser delegates clan chat', 'ServerLegacyClanChatAdapter().Broadcast' in block),
 ('parser authenticates sender', 'ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in block),
 ('parser no rsPlayInfo scan', 'rsPlayInfo[cnt]' not in block),
 ('parser no direct clan code read', 'dwClanCode' not in block),
]
failed=[]
for n,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+n)
 if not ok: failed.append(n)
if failed: raise SystemExit(1)
print('clan chat boundary: PASS')
