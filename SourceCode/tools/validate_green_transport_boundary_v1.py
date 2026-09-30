#!/usr/bin/env python3
from pathlib import Path
import importlib.util
import re
import sys

root = Path(__file__).resolve().parents[1]
architecture_path = root / 'tools/architecture/validate_legacy_boundary.py'
spec = importlib.util.spec_from_file_location('legacy_boundary', architecture_path)
architecture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(architecture)

direct_send = re.compile(r'\b(?:lpsmSock|smWsock[A-Za-z0-9_]*)\s*->\s*Send\s*\(')
source_suffixes = {'.h', '.hpp', '.cpp', '.cc', '.cxx'}
violations = []
green_files = 0
for path in root.rglob('*'):
    if not path.is_file() or path.suffix.lower() not in source_suffixes:
        continue
    rel = path.relative_to(root).as_posix()
    if not architecture.is_green(rel):
        continue
    green_files += 1
    text = path.read_text(errors='replace')
    if direct_send.search(text):
        violations.append(rel)

client_contract = (root / 'Shared/network/v2/ClientCommandSender.h').read_text(errors='replace')
server_contract = (root / 'Shared/network/v2/PacketSender.h').read_text(errors='replace')
client_adapter = (root / 'SrcGame/src/systems/network/v2/legacy/LegacyClientCommandSender.cpp').read_text(errors='replace')
server_adapter = (root / 'SrcServer/src/systems/network/v2/legacy/LegacyPacketSender.cpp').read_text(errors='replace')

checks = [
    ('GREEN direct legacy socket sends are zero', not violations),
    ('client GREEN transport contract exists', 'class IClientCommandSender' in client_contract and 'PacketView' in client_contract),
    ('server GREEN transport contract exists', 'class IPacketSender' in server_contract and 'PacketView' in server_contract),
    ('client AMBER adapter owns gameplay socket send', 'smWsockServer->Send' in client_adapter),
    ('server AMBER adapter owns player socket send', 'lpsmSock->Send' in server_adapter),
]
for label, ok in checks:
    print(('PASS: ' if ok else 'FAIL: ') + label)
if violations:
    for rel in violations:
        print(f'FAIL: direct legacy socket send leaked into GREEN: {rel}', file=sys.stderr)
if not all(ok for _, ok in checks):
    sys.exit(1)
print(f'green transport boundary v1: PASS (green_files={green_files}, direct_socket_send_files=0)')
