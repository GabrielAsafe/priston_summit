#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
client = (root / 'SrcGame/src/systems/social/party/ui/cPartyWindow.cpp').read_text(errors='replace')
sender = (root / 'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp').read_text(errors='replace')
onserver = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
contract = (root / 'Shared/gameplay/social/party/v2/PartyCommands.h').read_text(errors='replace')

start = onserver.index('\t\tcase OpCode::OPCODE_PARTY_COMMAND:')
end = onserver.index('\n\n\t\tcase OpCode::OPCODE_PARTY_PLAYUPDATE:', start)
server_body = onserver[start:end]
checks = [
    ('client preserves party command opcode', 'smTransCommand.code = OpCode::OPCODE_PARTY_COMMAND' in client),
    ('client preserves target serial payload', 'smTransCommand.WParam = Members[MenuFlag - 1].dwObjectSerial' in client),
    ('client preserves menu action payload', 'smTransCommand.LParam = opt->ButtonNum' in client and 'smTransCommand.SParam = 0' in client),
    ('client routes party command through sender', 'ClientLegacyCommandSender().Send' in client and 'PacketView(&smTransCommand, smTransCommand.size' in client),
    ('party ui has no direct gameplay socket send', 'smWsockServer->Send' not in client),
    ('amber sender owns socket send', 'LegacyClientCommandSender::SendToLegacySocket' in sender and 'socket->Send' in sender and 'static_cast<int>(packet.size), TRUE' in sender),
    ('server command boundary remains modern', all(token in contract for token in ('TransferLeadership', 'Release(PlayerId requester)', 'Kick(PlayerId requester, PlayerId target)', 'Leave(PlayerId requester)'))),
    ('server preserves action dispatch', all(token in server_body for token in ('partyCommands.TransferLeadership', 'partyCommands.Release', 'partyCommands.Kick', 'partyCommands.Leave'))),
]
failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + name)
if failed:
    sys.exit(1)
print('party client command sender boundary: PASS')
