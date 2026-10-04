#!/usr/bin/env python3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
char=(ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp').read_text(errors='ignore')
sys=(ROOT/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text(errors='ignore')
errors=[]

def req(label,text,needle):
    if needle not in text: errors.append(f"{label}: missing {needle!r}")

for needle in (
    'const bool accountCompanion =',
    'lpMasterPlayInfo->Position.Area < StageAreaCount',
    'STG_AREA *ownerArea = &StageArea[lpMasterPlayInfo->Position.Area]',
    'ownerArea->lpStage != lpStage',
    'lpStage = ownerArea->lpStage',
    'const int floorY = lpStage->GetHeight(followX, followZ)',
    'followY = floorY + (2 * fONE)',
    'gameplay::companions::GetFollowPosition(this, &followX, &followZ)',
    'lpTargetChar = 0',
): req('map transition recovery', char, needle)

# Recovery must happen before ApplyAI / normal hard-leash in this block.
block_start=char.find('// Account companions can survive owner warps/map transitions')
apply=char.find('gameplay::companions::ApplyAI(this);', block_start)
assign=char.find('lpStage = ownerArea->lpStage;', block_start)
if block_start < 0 or apply < 0 or assign < 0 or not (assign < apply):
    errors.append('map transition stage recovery does not precede ApplyAI')

if 'if(owner->smCharInfo.Life[0] <= 0)' not in sys and 'if(!ownerSnapshot.alive)' not in sys:
    errors.append('owner-dead hold policy: missing legacy or snapshot alive gate')

for needle in (
    'managedSlot->skillRuntime.multiHitRemaining = 0',
    'managedSlot->skillRuntime.chargedStrikeTargetSerial = 0',
    'character->lpTargetChar = NULL',
    'character->lpTargetPlayInfo = NULL',
    'character->SetMotionFromCode(CHRMOTION_STATE_STAND)',
): req('owner-dead hold policy', sys, needle)

# Preserve already-closed lifecycle/persistence paths.
for needle in (
    'GetResurrectionCorpseSeconds()',
    'FlushSlotExperienceRetry(*state, slot, 3)',
    'FlushSlotExperienceRetry(it->second, slot, 6)',
    'void OnOwnerDisconnect(rsPLAYINFO *owner)',
): req('existing lifecycle', sys, needle)

if errors:
    print('Companion Lifecycle V33: FAIL')
    for e in errors: print('  -',e)
    raise SystemExit(1)
print('Companion Lifecycle V33: PASS (map-stage recovery + owner-dead hold policy)')
