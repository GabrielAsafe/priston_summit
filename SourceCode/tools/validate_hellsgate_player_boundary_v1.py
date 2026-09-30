#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
header = (root / 'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.h').read_text(errors='replace')
source = (root / 'SrcServer/src/systems/gameplay/events/hells_gate/cHellsGate.cpp').read_text(errors='replace')
combat = (root / 'Shared/gameplay/combat/Svr_Damge_legacy_server.inl').read_text(errors='replace')
interaction = (root / 'SrcServer/src/systems/gameplay/world/v2/legacy/LegacyInteractionAdapter.cpp').read_text(errors='replace')

start = source.find('void cHELLSGATE::SendOpenNpc(')
end = source.find('void cHELLSGATE::RequestJoin', start)
body = source[start:end]
checks = {
    'public open uses player id': 'SendOpenNpc(gameplay::PlayerId player)' in header and 'SendOpenNpc(gameplay::PlayerId player)' in source,
    'open uses chat delivery': 'ServerLegacyChatDeliveryAdapter().SendToPlayer' in body and 'ChatDeliveryKind::Whisper' in body,
    'open uses packet sender': 'ServerLegacyPacketSender().Send' in body and 'OPCODE_OPEN_HG' in body,
    'open has no legacy player/socket': 'rsPLAYINFO' not in body and 'lpsmSock' not in body,
    'combat derives authenticated player for interaction': 'ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo)' in combat and 'ServerLegacyInteractionAdapter().Interact' in combat,
    'interaction boundary owns hells gate dispatch': 'npc.eventCode == 988u' in interaction and 'cHg.SendOpenNpc(request.player)' in interaction,
}
failed = [k for k, v in checks.items() if not v]
if failed:
    print('hellsgate player boundary v1: FAIL: ' + ', '.join(failed), file=sys.stderr)
    sys.exit(1)
print('hellsgate player boundary v1: PASS')
