#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[5]
path = root / "SrcServer/src/systems/gameplay/character/core/character.cpp"
text = path.read_text(encoding="utf-8", errors="replace")
play_attack_start = text.index("int smCHAR::PlayAttack(int EventFlag)")
death_start = text.index("if(sinGetLife() <= 0)", play_attack_start)
death_end = text.index("ReformCharForm();", death_start)
block = text[death_start:death_end]

for token in (
    "DeathAttributionPolicy::ShouldApplyDeathExperiencePenalty(",
    "DeathAttributionPolicy::ShouldRecordPlayerKill(",
    "DeadPlayerExp(lpCurPlayer);",
    "sinCheckQuest85Die();",
    "SaveGameData();",
    "PlayerKilling(dwObjectSerial);",
):
    assert token in block, f"death attribution guard: missing {token}"

for legacy in (
    "if(smCharInfo.State != smCHAR_STATE_USER && PkMode_CharState != smCHAR_STATE_USER)",
    "if(smCharInfo.State != smCHAR_STATE_USER)",
    "if(smCharInfo.State == smCHAR_STATE_USER || PkMode_CharState == smCHAR_STATE_USER)",
    "if(smCharInfo.State == smCHAR_STATE_USER)",
):
    assert legacy not in block, f"death attribution guard: raw attribution branch remains: {legacy}"

apply_pos = block.index("DeathAttributionPolicy::ShouldApplyDeathExperiencePenalty(")
penalty_pos = block.index("DeadPlayerExp(lpCurPlayer);", apply_pos)
quest_pos = block.index("sinCheckQuest85Die();", penalty_pos)
save_pos = block.index("SaveGameData();", quest_pos)
record_pos = block.index("DeathAttributionPolicy::ShouldRecordPlayerKill(", save_pos)
kill_pos = block.index("PlayerKilling(dwObjectSerial);", record_pos)
assert apply_pos < penalty_pos < quest_pos < save_pos < record_pos < kill_pos, (
    "death attribution guard: legacy side-effect ordering changed"
)

assert block.count("#ifdef LOG_THE_PK") >= 2, "death attribution guard: LOG_THE_PK gates removed"
print("smCHAR death attribution guard: PASS")
