#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[5]
for rel in (
    "SrcServer/src/systems/gameplay/character/core/character.cpp",
    "Shared/gameplay/character/character_legacy_client.inl",
):
    text = (ROOT / rel).read_text(encoding="utf-8", errors="replace")
    start = text.index("int smCHAR::EventSkill()")
    end = text.find("\nint smCHAR::", start + 1)
    block = text[start:end if end >= 0 else len(text)]

    required = (
        "SkillMultiHitPolicy::IsWithinHitWindow(MotionEvent, 3) && chrAttackTarget",
        "SkillMultiHitPolicy::IsWithinHitWindow(MotionEvent, T_Impact_Hit[point - 1])",
        "SkillMultiHitPolicy::IsWithinHitWindow(MotionEvent, Divine_Piercing_AttackNum[point - 1])",
    )
    for token in required:
        assert token in block, f"event skill multi-hit guard: missing {token} in {rel}"

    forbidden = (
        "MotionEvent < 3 && chrAttackTarget",
        "MotionEvent < T_Impact_Hit[point - 1]",
        "MotionEvent < Divine_Piercing_AttackNum[point - 1]",
    )
    for token in forbidden:
        assert token not in block, f"event skill multi-hit guard: inline timing remains in {rel}: {token}"

    assert block.count("dm_SendTransDamage(chrAttackTarget") >= 3, (
        f"event skill multi-hit guard: damage side effects moved unexpectedly in {rel}"
    )

print("event skill multi-hit policy guard: PASS")
