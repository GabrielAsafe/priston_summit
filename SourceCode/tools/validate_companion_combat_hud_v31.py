#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]

def text(rel):
    p = ROOT / rel
    return p.read_text(errors="ignore") if p.exists() else ""

protocol = text("Shared/gameplay/companions/CompanionProtocol.h")
server = text("SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp")
executor_h = text("SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.h")
executor_cpp = text("SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp")
gateway = text("SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp")
models = text("SrcGame/src/systems/ui/v2/domain/CompanionUIModels.h")
hud = text("SrcGame/src/systems/ui/v2/hud/CompanionPartyHud.cpp")

a = []
def require(label, haystack, needle):
    if needle not in haystack:
        a.append(f"{label}: missing {needle!r}")

require("runtime protocol v2", protocol, "COMPANION_RUNTIME_PROTOCOL_VERSION = 2")
require("effect capacity", protocol, "COMPANION_RUNTIME_MAX_EFFECTS = 6")
for field in ("castingSkillCode", "castProgressPermille", "cooldownMs", "statusFlags", "effectCount", "castingSkillName", "effects["):
    require("runtime wire field", protocol, field)
for flag in ("COMPANION_RUNTIME_STATUS_SLOW", "COMPANION_RUNTIME_STATUS_STUN", "COMPANION_RUNTIME_STATUS_POISON", "COMPANION_RUNTIME_STATUS_DISTORTION"):
    require("status wire flag", protocol, flag)
require("runtime packet guard", protocol, "sizeof(CompanionRuntimeStatePacket) < 1024")

require("lock start state", executor_h, "skillLockStartedAt")
require("lock start capture", executor_cpp, "runtime.skillLockStartedAt = now")
require("lock start reset", executor_cpp, "skillLockStartedAt = 0")

for marker in (
    "entry.castingSkillCode",
    "entry.castProgressPermille",
    "entry.cooldownMs",
    "COMPANION_RUNTIME_STATUS_SLOW",
    "COMPANION_RUNTIME_STATUS_STUN",
    "COMPANION_RUNTIME_STATUS_POISON",
    "COMPANION_RUNTIME_STATUS_DISTORTION",
    "entry.effects[entry.effectCount++]",
    "SKILL_FORCE_OF_NATURE",
    "SKILL_REGENERATION_FIELD",
    "SKILL_ENCHANT_WEAPON",
):
    require("server runtime snapshot", server, marker)

# Snapshot transport must remain DAT-I/O free.
start = server.find("void SendRuntimeStateLocked")
end = server.find("\nstatic bool IsOneShotVisualMotion", start)
if start < 0 or end < 0:
    a.append("could not isolate SendRuntimeStateLocked")
else:
    body = server[start:end]
    for forbidden in ("LoadCharacterSnapshot", "rsLoadAccountCharacterRecord"):
        if forbidden in body:
            a.append(f"runtime snapshot performs DAT I/O via {forbidden}")

for marker in ("castingSkillName", "castProgressPermille", "cooldownMs", "statusFlags", "effectCount", "effects{}"):
    require("client runtime model", models, marker)
for marker in ("source.castingSkillCode", "source.castingSkillName", "source.castProgressPermille", "source.cooldownMs", "source.statusFlags", "source.effectCount", "source.effects[effect]"):
    require("gateway mapping", gateway, marker)
for marker in ("Cast: ", "castProgressPermille / 10", "Buffs ", "cooldownMs + 999", '"  Slow"', '"  Stun"', '"  Poison"', '"  Dist"'):
    require("HUD presentation", hud, marker)

# Prevent the old ambiguous decisecond suffix from returning.
if ' + "d"' in hud and "cooldownMs" in hud:
    a.append("HUD cooldown still uses ambiguous 'd' suffix")

if a:
    print("Companion Combat HUD V31: FAIL")
    for e in a:
        print("  -", e)
    raise SystemExit(1)
print("Companion Combat HUD V31: PASS")
