#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/DamageContracts.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyDamageAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion damage boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def function_body(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0:
        fail(f"missing function {signature}")
    brace = text.find("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[brace:index + 1]
    fail(f"unterminated function {signature}")
    return ""


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    body = function_body(executor, "static bool ApplySkillDamage")
    direct_body = function_body(executor, "static bool ApplyDirectLifeLoss")

    required = [
        "ServerLegacyDamageAdapter().Apply",
        "DamageRequest",
        "ServerLegacyOnlinePlayerRegistry().IdOf(owner)",
    ]
    for token in required:
        if token not in body:
            fail(f"ApplySkillDamage is not routed through boundary: missing {token}")

    forbidden = [
        "target->smCharInfo.Life[0] -=",
        "target->SetMotionFromCode(CHRMOTION_STATE_DEAD)",
        "target->lpExt2 = owner",
        "target->lpExt3 = caster",
    ]
    for token in forbidden:
        if token in body:
            fail(f"legacy damage authority still present in Companion executor: {token}")

    direct_required = [
        "ServerLegacyDamageAdapter().Apply",
        "DamageMode::DirectLifeLoss",
        "ServerLegacyOnlinePlayerRegistry().IdOf(owner)",
    ]
    for token in direct_required:
        if token not in direct_body:
            fail(f"ApplyDirectLifeLoss is not routed through direct-damage mode: missing {token}")
    for token in forbidden:
        if token in direct_body:
            fail(f"direct life-loss authority still present in Companion executor: {token}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    if "enum class DamageMode" not in contract or "DirectLifeLoss" not in contract:
        fail("GREEN damage contract is missing direct-life-loss policy")
    for forbidden_symbol in ("rsPLAYINFO", "smCHAR", "windows.h", "onserver.h", "character.h"):
        if forbidden_symbol in contract:
            fail(f"GREEN contract contains legacy symbol/include: {forbidden_symbol}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    parity_tokens = [
        "request.mode == DamageMode::DirectLifeLoss",
        "target->smCharInfo.Absorption",
        "!directLifeLoss && target->UseObject_VirtualLife",
        "CHRMOTION_STATE_DEAD",
        "smMonsterInfo.GetExp >>= 1",
        "target->lpExt2 = owner",
        "target->lpExt3 = caster",
    ]
    for token in parity_tokens:
        if token not in adapter:
            fail(f"legacy parity token missing from adapter: {token}")

    print("companion damage boundary v1: PASS")


if __name__ == "__main__":
    main()
