#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[4]
client = (root / 'SrcGame/src/systems/network/client/netplay.cpp').read_text(encoding='utf-8', errors='replace')
server = (root / 'SrcServer/src/systems/network/server_gameplay/netplay.cpp').read_text(encoding='utf-8', errors='replace')

# Client-only inventory/commerce forwarding sites.
for anchor, end_anchor in (
    ('case OpCode::OPCODE_PUTITEM:', 'case OpCode::OPCODE_GETITEM:'),
    ('case OpCode::OPCODE_OPEN_WAREHOUSE:', 'case PACKET_KILL:'),
):
    start = client.index(anchor)
    end = client.index(end_anchor, start)
    body = client[start:end]
    if 'ClientLegacyBackendChannelSender().Send(' not in body or 'BackendChannel::Data' not in body:
        raise SystemExit(f'client scoped Data route missing in {anchor}')

trade_start = client.index('if(dwLastTransTradeKeyTime &&')
trade_end = client.index('if(ChatDispCnt > 0)', trade_start)
trade = client[trade_start:trade_end]
if 'ClientLegacyBackendChannelSender().Send(' not in trade or 'LastTransTradeItemKey' not in trade:
    raise SystemExit('trade key resend is not routed through typed Data backend')
if 'smWsockDataServer->Send' in trade:
    raise SystemExit('trade key resend still uses direct Data socket')

for text, accessor in ((client, 'ClientLegacyBackendChannelSender'), (server, 'ServerLegacyBackendChannelSender')):
    for start_anchor, end_anchor in (
        ('int\tSendItemExpress(DWORD\tdwItemCode, char *szPassCode)', 'int\tSendItemExpress(DWORD\tdwItemCode)'),
        ('int\tSendStarPointToServer(int Price, int Cash)', 'int\tSendGiveMoneyToServer(int Money)'),
        ('int\tSendGiveMoneyToServer(int Money)', 'int\tSendClanMoneyToServer(int Money, int Flag, int Kind)'),
    ):
        start = text.index(start_anchor)
        end = text.index(end_anchor, start)
        body = text[start:end]
        if f'{accessor}().Send(' not in body or 'BackendChannel::Data' not in body:
            raise SystemExit(f'{accessor}: scoped gameplay Data request not typed: {start_anchor}')
        if 'smWsockDataServer->Send' in body:
            raise SystemExit(f'{accessor}: direct Data socket remains in {start_anchor}')

print('netplay scoped Data tail guard: PASS')
