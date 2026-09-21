#!/usr/bin/env python3
"""Guard the read/write Story quest adapter against legacy cQUESTHANDLER drift."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "SrcServer/src/systems/gameplay/progression/v2/LegacyQuestContract.h"
HANDLER = ROOT / "SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp"

EXPECTED = {
    "kRepeatedQuestKind": 21,
    "kDailyQuestKind": 7,
    "kSingleQuestKind": 9,
}


def fail(message: str) -> None:
    print(f"legacy quest contract validation: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not CONTRACT.is_file() or not HANDLER.is_file():
        fail("contract or cQuestHandler source is missing")

    contract = CONTRACT.read_text(encoding="utf-8")
    resolved = {}
    for name in EXPECTED:
        match = re.search(rf"\b{name}\s*=\s*(-?\d+)\s*;", contract)
        if not match:
            fail(f"missing {name} in LegacyQuestContract.h")
        resolved[name] = int(match.group(1))
    if resolved != EXPECTED:
        fail(f"unexpected QuestKind constants: {resolved}")

    # Restrict the proof to StartQuest rather than accepting unrelated switch cases.
    raw = HANDLER.read_bytes()
    text = raw.decode("latin-1")
    start = text.find("void cQUESTHANDLER::StartQuest")
    if start < 0:
        fail("cQUESTHANDLER::StartQuest not found")
    end = text.find("void cQUESTHANDLER::", start + 1)
    body = text[start:] if end < 0 else text[start:end]
    cases = {int(value) for value in re.findall(r"\bcase\s+(\d+)\s*:", body)}
    missing = sorted(set(EXPECTED.values()) - cases)
    if missing:
        fail(f"StartQuest no longer handles QuestKind values {missing}")

    print("legacy quest contract validation: PASS")


if __name__ == "__main__":
    main()
