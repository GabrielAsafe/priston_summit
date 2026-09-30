#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/social/clan/v2/ClanQuery.h').read_text()
adapter=(root/'SrcServer/src/systems/social/clan/v2/legacy/LegacyClanQueryAdapter.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
start=onserver.index('int rsRecvUseLinkCore(')
end=onserver.index('\nint ', start+10)
link=onserver[start:end]
checks=[
 ('green clan id', 'struct ClanId' in contract and 'std::uint32_t value' in contract),
 ('green membership snapshot', 'struct ClanMembershipSnapshot' in contract and 'IsMember()' in contract),
 ('green clan query port', 'class IClanQuery' in contract and 'TryGetMembership' in contract),
 ('same clan helper requires membership', 'lhs.IsMember() && rhs.IsMember()' in contract),
 ('adapter maps runtime clan code', 'player->dwClanCode' in adapter and 'snapshot.clan = ClanId' in adapter),
 ('adapter maps clan mark', 'player->smCharInfo.ClassClan' in adapter),
 ('link core uses clan query', 'ServerLegacyClanQueryAdapter()' in link),
 ('link core authenticates source and target ids', link.count('ServerLegacyOnlinePlayerRegistry().IdOf') >= 2),
 ('link core preserves membership gate', 'sourceClan.IsMember()' in link),
 ('link core preserves same-clan gate', 'SameClan(sourceClan, targetClan)' in link),
 ('link core no direct clan-code comparison', 'lpPlayInfo2->dwClanCode != lpPlayInfo->dwClanCode' not in link),
]
failed=[]
for name,ok in checks:
 print(('PASS' if ok else 'FAIL')+': '+name)
 if not ok: failed.append(name)
if failed: raise SystemExit(1)
print('clan query boundary: PASS')
