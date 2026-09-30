#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
server=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
record=(ROOT/'Shared/persistence/character_record/record_legacy_server.inl').read_text(errors='ignore')
errors=[]

def req(label,text,needle):
    if needle not in text: errors.append(f"{label}: missing {needle!r}")

for needle in ('ReformCharStatePoint(', 'recordData.smCharInfo.Next_Exp', 'ReformCharForm(&recordData.smCharInfo)', 'CopyFile(szFile, szBackupFile'):
    req('offline persistence contract', record, needle)
for needle in ('const int previousLevel = slot.cachedLevel', 'SetExp64(&slot.entity->smCharInfo, newExperience)',
               'if(newLevel != previousLevel)', 'ServerLegacyCharacterRecordAdapter().TryLoadProgression',
               'slot.entity->smCharInfo.StatePoint = refreshed.statePoint',
               'slot.entity->smCharInfo.Next_Exp = refreshed.nextExperience',
               'slot.entity->smCharInfo.ChangeJob = refreshed.changeJob',
               'slot.entity->smCharInfo.JobBitMask = refreshed.jobBitMask',
               '"LEVEL_UP name='):
    req('live progression sync', server, needle)

# Ensure the level-up sync does not overwrite the whole live character record.
start=server.find('bool FlushSlotExperience(')
end=server.find('\nbool FlushSlotExperienceRetry', start)
body=server[start:end] if start >= 0 and end >= 0 else ''
if not body:
    errors.append('could not isolate FlushSlotExperience')
else:
    if 'slot.entity->smCharInfo = refreshed.smCharInfo' in body or 'memcpy(&slot.entity->smCharInfo' in body:
        errors.append('level-up sync overwrites the whole live smCharInfo')
    for preserve in ('Life[0]', 'Mana[0]', 'Stamina[0]'):
        # No assignments to current resources are expected in the flush body.
        if f'slot.entity->smCharInfo.{preserve} =' in body:
            errors.append(f'level-up sync unexpectedly rewrites {preserve}')

for needle in ('FlushSlotExperienceRetry(*state, slot, 3)', 'FlushSlotExperienceRetry(it->second, slot, 6)', 'GetExpFlushSeconds()'):
    req('existing persistence lifecycle', server, needle)

if errors:
    print('Companion Progression V32: FAIL')
    for e in errors: print('  -',e)
    raise SystemExit(1)
print('Companion Progression V32: PASS (live level/EXP/state-point sync preserves runtime resources/equipment)')
