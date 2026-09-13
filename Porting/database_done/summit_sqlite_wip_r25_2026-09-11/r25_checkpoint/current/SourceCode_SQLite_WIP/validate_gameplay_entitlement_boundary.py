#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / 'SrcServer/src/core/server/legacy_runtime/onserver.h'
PARTY = ROOT / 'SrcServer/src/core/server/legacy_runtime/PartySever.cpp'
SERVER = ROOT / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl'
DLL = ROOT / 'SrcServer/src/core/server/legacy_runtime/DllServer.cpp'

header = HEADER.read_text(errors='ignore')
party = PARTY.read_text(errors='ignore')
server = SERVER.read_text(errors='ignore')
dll = DLL.read_text(errors='ignore')

errors = []

def require(text, needle, label):
    if needle not in text:
        errors.append(f'missing {label}: {needle}')

# R21 adds a local source while preserving the historical Bl_RNo source.
require(header, 'inline bool rsHasPCBangGameplayEntitlement(const rsPLAYINFO *lpPlayInfo)', 'entitlement facade declaration')
require(header, 'bool rsHasLocalPCBangGameplayEntitlement(const rsPLAYINFO *lpPlayInfo);', 'local entitlement accessor')
require(header, 'void rsSetLocalPCBangGameplayEntitlement(rsPLAYINFO *lpPlayInfo, bool enabled);', 'local entitlement setter')
require(header, '(lpPlayInfo->Bl_RNo > 0 || rsHasLocalPCBangGameplayEntitlement(lpPlayInfo));', 'dual-source legacy-compatible entitlement predicate')

# Proven gameplay effects must use the semantic entitlement boundary.
require(server, 'if (rsHasPCBangGameplayEntitlement(lprsPlayInfo))\n\t\t\tFallRandom += 50;', 'drop +50 entitlement')
require(server, 'if (rsHasPCBangGameplayEntitlement(lprsPlayInfo))\n\t\t\t{\n\t\t\t\tExpUp += 20;', 'solo EXP +20 entitlement')
require(party, 'if (rsHasPCBangGameplayEntitlement(lpPartyPlayers[cnt]))\n\t\t\t\t\t\tExpUp += 20;', 'party EXP +20 entitlement')
require(server, 'lpPlayInfo->dwPCBang_Pet_NextSetTime < dwPlayServTime && rsHasPCBangGameplayEntitlement(lpPlayInfo)', 'PCBang Pet entitlement gate')

# R20 must not reinterpret the legacy identifier in provider/accounting paths.
for needle, label in [
    ('if(rsPlayInfo[cnt].Bl_RNo)', 'PCUse billing gate'),
    ('bSql_PCRNoReservProduct(lpPlayInfo);', 'reservation/product path'),
    ('smTransCommand.WParam = (lpPlayInfo->Bl_RNo) ^ dwMaskKey;', 'PCBang packet legacy token'),
    ('lpPlayInfo->Bl_RNo = smTransCommand.WParam;', 'PCBang packet receive token'),
    ('LogSqlData[cnt].PC_RNo = lpPlayInfo->Bl_RNo;', 'billing queue legacy token'),
]:
    require(server if 'smTransCommand' in needle or needle.startswith('if(rsPlayInfo') or needle.startswith('bSql_') else dll, needle, label)

# The active server's raw positive checks should now be billing/session UI only,
# never the four gameplay formulas above.
raw_positive = []
for path, text in [(SERVER, server), (PARTY, party)]:
    for m in re.finditer(r'Bl_RNo\s*>\s*0', text):
        line = text.count('\n', 0, m.start()) + 1
        raw_positive.append((path.relative_to(ROOT).as_posix(), line))

party_raw = [x for x in raw_positive if x[0].endswith('PartySever.cpp')]
if party_raw:
    errors.append(f'raw party Bl_RNo gameplay check remains: {party_raw}')

# OnSever raw checks are expected only in billing/display/reservation logic.
# Pin the count to make accidental gameplay reintroduction visible.
server_raw = [x for x in raw_positive if x[0].endswith('OnSever_legacy_server.inl')]
if len(server_raw) != 6:
    errors.append(f'expected 6 raw OnSever billing/session Bl_RNo>0 checks, found {len(server_raw)}: {server_raw}')

if errors:
    print('Gameplay entitlement boundary FAIL')
    for e in errors:
        print(' -', e)
    raise SystemExit(1)

print('Gameplay entitlement boundary PASS: legacy Bl_RNo source preserved + local entitlement source added; drop/solo EXP/party EXP/PCBang Pet use facade; billing/session Bl_RNo paths retained')
