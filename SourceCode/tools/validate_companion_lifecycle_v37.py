#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
errors=[]
def req(label, needle):
    if needle not in s: errors.append(f"{label}: missing {needle!r}")
for needle in ('nextIdentityCheckTime', 'dwPlayServTime + 2000U',
               'FindUserFromName(managedSlot->characterName)',
               'onlineCharacter && onlineCharacter != owner && onlineCharacter->lpsmSock'):
    req('session identity guard', needle)
for needle in ('const bool flushed = FlushSlotExperience(*state, *managedSlot)',
               'KillEntity(*managedSlot)', 'if(flushed)', 'ClearSlot(*managedSlot)',
               'IDENTITY_CONFLICT_DISMISS', 'SendRuntimeStateLocked(*state, true)',
               'SendVisualStateLocked(*state, true)'):
    req('safe retirement', needle)
req('summon-time online rejection', 'rsPLAYINFO *onlineCharacter = FindUserFromName((char *)characterName);')
req('owner-name rejection', 'lstrcmpiA(owner->getName(), characterName) == 0')
if errors:
    print('Companion Lifecycle V37: FAIL')
    for e in errors: print('  -', e)
    raise SystemExit(1)
print('Companion Lifecycle V37: PASS (late real-player login retires duplicate companion safely)')
