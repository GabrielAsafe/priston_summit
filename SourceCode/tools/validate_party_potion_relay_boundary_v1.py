#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
contract = (root / 'Shared/gameplay/social/party/v2/PartyRelayService.h').read_text()
onserver = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()
runtime = (root / 'SrcServer/src/systems/social/party/v2/legacy/LegacyPartyRelayRuntime.cpp').read_text()

checks = [
    ('green relay contract', 'class IPartyRelayService' in contract and 'PartyRelayResult' in contract),
    ('relay composes party query', 'TryGetParty(sender, party)' in contract),
    ('relay excludes sender', 'it->player == sender' in contract),
    ('relay sends through packet sender', 'packets_.Send(it->player, packet)' in contract),
    ('legacy composition uses party query adapter', 'ServerLegacyPartyQueryAdapter()' in runtime),
    ('legacy composition uses packet sender', 'ServerLegacyPacketSender()' in runtime),
    ('onserver delegates potion relay', 'ServerLegacyPartyRelayService().Relay' in onserver),
    ('onserver authenticates sender', 'ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in onserver),
    ('onserver preserves packet size opcode', 'partyPacket->size' in onserver and 'partyPacket->code' in onserver),
    ('onserver no direct party send', 'lpPlayInfo->lpPartyMaster->SendPartyData(lpPlayInfo, SockInfo->Buff)' not in onserver),
]
failed=[]
for name, ok in checks:
    print(('PASS' if ok else 'FAIL') + ': ' + name)
    if not ok: failed.append(name)
if failed:
    raise SystemExit(1)
print('party potion relay boundary: PASS')
