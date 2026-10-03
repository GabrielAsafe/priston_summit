#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[1]
cpp = (root / 'SrcServer/src/systems/commerce/v2/legacy/LegacyTradeAdapter.cpp').read_text(errors='replace')
service = (root / 'Shared/gameplay/commerce/v2/TradeSessionService.h').read_text(errors='replace')
checks = {
    'adapter no dwLastTradeTime authority': 'dwLastTradeTime' not in cpp,
    'adapter no dwLastTradeChksum authority': 'dwLastTradeChksum' not in cpp,
    'generation aware replay key': 'sender.generation' in service,
    'legacy 8s window preserved': 'TradeReplayGuardService replayGuard(8000u)' in cpp,
    'adapter delegates replay check': 'replayGuard_.Check(' in cpp,
    'adapter delegates replay mark': 'replayGuard_.MarkCommitted(' in cpp,
}
failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(('PASS' if ok else 'FAIL') + ': ' + name)
if failed:
    sys.exit(1)
print(f'trade session authority v2: PASS ({len(checks)}/{len(checks)})')
