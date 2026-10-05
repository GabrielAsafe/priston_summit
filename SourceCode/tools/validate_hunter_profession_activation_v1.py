#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EVENT = ROOT / "SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.cpp"
TYPES = ROOT / "Shared/gameplay/professions/v2/ProfessionTypes.h"
READER = ROOT / "SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionReaders.cpp"
REPO = ROOT / "SrcServer/src/systems/gameplay/professions/v2/SQLiteProfessionRepository.cpp"

failures = []
event = EVENT.read_text(errors="replace")
types = TYPES.read_text(errors="replace")
reader = READER.read_text(errors="replace")
repo = REPO.read_text(errors="replace")

if "Hunter" not in types:
    failures.append("ProfessionType::Hunter is missing")
if "ServerProfessionService().AddExperience" not in event:
    failures.append("MonsterKilled runtime does not award profession XP")
if "ProfessionType::Hunter" not in event:
    failures.append("MonsterKilled runtime does not target Hunter identity")
if not re.search(r"ProfessionType::Hunter,\s*1u", event, flags=re.S):
    failures.append("Hunter kill XP must be one unit per credited kill")
if re.search(r'EqualsIgnoreCase\(key,\s*"hunter"\)\)\s*return\s+[1-9]', reader):
    failures.append("Hunter must not be mapped to a guessed legacy JOB_CODE")
if "value <= static_cast<int>(ProfessionType::Custom)" not in repo:
    failures.append("profession repository does not persist the V2 Hunter identity range")
if "ProfessionType INTEGER NOT NULL CHECK(ProfessionType BETWEEN 1 AND 8)" not in repo:
    failures.append("SQLite profession schema does not accept Hunter")

if failures:
    for failure in failures:
        print("FAIL: " + failure, file=sys.stderr)
    raise SystemExit(1)
print("hunter profession activation v1: PASS (V2-only identity + persistent kill XP; no legacy JOB_CODE)")
