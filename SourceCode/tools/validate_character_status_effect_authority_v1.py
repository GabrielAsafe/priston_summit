#!/usr/bin/env python3
from pathlib import Path
from collections import Counter
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CHARACTER = ROOT / "SrcServer/src/systems/gameplay/character/core/character.cpp"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/StatusEffectService.h"


def fail(message: str) -> None:
    print("character status effect authority v1: FAIL: " + message, file=sys.stderr)
    raise SystemExit(1)


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def function_body(text: str, marker: str) -> str:
    start = text.find(marker)
    if start < 0:
        fail(f"missing function {marker}")
    match = re.search(r"\n(?:int|void|DWORD|smCHAR\s*\*)\s+smCHAR::", text[start + len(marker):])
    end = start + len(marker) + match.start() if match else len(text)
    return strip_comments(text[start:end])


text = CHARACTER.read_text(encoding="utf-8", errors="replace")
main = function_body(text, "int smCHAR::Main()")
play_attack = function_body(text, "int smCHAR::PlayAttack(int EventFlag)")
event_skill_monster = function_body(text, "int smCHAR::EventSkill_Monster()")

write = re.compile(
    r"(?:PlayStunCount|PlaySlowCount|PlaySlowSpeed|PlayDistortion|"
    r"PlayPoison\s*\[[01]\]|PlayHolyMind\s*\[[01]\]|"
    r"PlayHeavenlyLight\s*\[[01]\])\s*(?:\+=|-=|=(?!=)|\+\+|--)"
)

for label, body in (("PlayAttack", play_attack), ("EventSkill_Monster", event_skill_monster)):
    hits = [(i, line.strip()) for i, line in enumerate(body.splitlines(), 1) if write.search(line)]
    if hits:
        for line_no, line in hits:
            print(f"FAIL: direct status mutation remains in {label}:{line_no}: {line}", file=sys.stderr)
        raise SystemExit(1)

# Main still owns the legacy timer tick/reset shell. Exact counts make that shell
# explicit and reject new gameplay application decisions from being added there.
expected_main_shell = Counter({
    "PlaySlowCount--;": 2,
    "if(PlayDistortion > 0) PlayDistortion--;": 1,
    "PlayStunCount--;": 2,
    "PlayPoison[1]--;": 2,
    "PlaySlowCount++;": 1,
    "PlayHolyMind[1]--;": 1,
    "PlayHeavenlyLight[1]--;": 1,
    "PlayPoison[1] = 0;": 1,
    "PlayDistortion--;": 1,
})
actual_main_shell = Counter(
    line.strip() for line in main.splitlines() if write.search(line)
)
if actual_main_shell != expected_main_shell:
    fail(f"Main status shell baseline changed: expected {dict(expected_main_shell)}, got {dict(actual_main_shell)}")

for token, minimum in (
    ("ServerLegacyStatusEffectAdapter().ApplySlow(", 12),
    ("ServerLegacyStatusEffectAdapter().ApplyStun(", 4),
    ("ServerLegacyStatusEffectAdapter().ApplyPoison(", 2),
    ("ServerLegacyStatusEffectAdapter().ClearStatus(", 1),
):
    count = text.count(token)
    if count < minimum:
        fail(f"expected >= {minimum} {token}, found {count}")

contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
if "enum class SlowTimerMode" not in contract or "CountUpFromNegative" not in contract:
    fail("SlowTimerMode contract for reverse countdown is missing")
if "effect.timerMode == SlowTimerMode::CountUpFromNegative ? -effect.durationTicks" not in adapter:
    fail("legacy adapter does not preserve reverse slow countdown representation")
if "SlowTimerMode::CountUpFromNegative" not in main:
    fail("Main WEB slow does not use reverse countdown through status authority")

print(
    "character status effect authority v1: PASS "
    "(PlayAttack/EventSkill_Monster direct status writes=0; Main timer shell exact)"
)
