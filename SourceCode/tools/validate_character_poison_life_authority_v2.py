#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "SrcServer/src/systems/gameplay/character/core/character.cpp"
text = SOURCE.read_text(errors="replace")

marker = "if(PlayPoison[1] > 0)"
positions = [m.start() for m in re.finditer(re.escape(marker), text)]
if len(positions) < 2:
    raise SystemExit("FAIL: expected server poison block in smCHAR::Main")

start = positions[-1]
end = text.find("if(PlayHolyIncantation[1] > 0)", start)
if end < 0:
    raise SystemExit("FAIL: could not isolate smCHAR::Main poison block")
block = text[start:end]

checks = {
    "Life adapter included": 'LegacyLifeAdapter.h' in text,
    "non-lethal command used": 'ServerLegacyLifeAdapter().ConsumeNonLethal' in block,
    "entity id used": 'gameplay::EntityId' in block,
    "legacy floor preserved": 'lifeBeforePoison - 1' in block,
    "poison ends at floor": 'poisonAmount >= lifeBeforePoison' in block and 'PlayPoison[1] = 0' in block,
    "no direct life subtraction": 'smCharInfo.Life[0] -=' not in block,
    "no direct life assignment": re.search(r'smCharInfo\.Life\[0\]\s*=', block) is None,
}

failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(("PASS" if ok else "FAIL") + ": " + name)
if failed:
    raise SystemExit("FAIL: " + ", ".join(failed))
print(f"PASS: character poison life authority v2 ({len(checks)}/{len(checks)})")
