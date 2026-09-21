#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
files = {
    "protocol": ROOT / "Shared/gameplay/companions/CompanionProtocol.h",
    "server": ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp",
    "profile_h": ROOT / "SrcServer/src/systems/gameplay/companions/CompanionProfileStore.h",
    "profile_cpp": ROOT / "SrcServer/src/systems/gameplay/companions/CompanionProfileStore.cpp",
    "character": ROOT / "SrcServer/src/systems/gameplay/character/core/character.cpp",
    "models": ROOT / "SrcGame/src/systems/ui/v2/domain/CompanionUIModels.h",
    "gateway": ROOT / "SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp",
    "page": ROOT / "SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp",
}
text = {name: path.read_text(errors="ignore") for name, path in files.items()}
checks = [
    ("protocol v3", "protocol", "COMPANION_UI_PROTOCOL_VERSION = 3"),
    ("tactic action", "protocol", "COMPANION_UI_SET_TACTIC"),
    ("retreat action", "protocol", "COMPANION_UI_SET_RETREAT_HP"),
    ("range action", "protocol", "COMPANION_UI_SET_PREFERRED_RANGE"),
    ("per-entry mode", "protocol", "std::int32_t mode;"),
    ("profile tactic", "profile_h", "CompanionTacticPreset tactic"),
    ("profile retreat", "profile_h", "retreatHpPercent"),
    ("profile range", "profile_h", "preferredRange"),
    ("profile formation", "profile_h", "formationSlot"),
    ("balanced evaluator", "server", "COMPANION_TACTIC_BALANCED"),
    ("owner focus evaluator", "server", "COMPANION_TACTIC_OWNER_FOCUS"),
    ("defensive evaluator", "server", "COMPANION_TACTIC_DEFENSIVE"),
    ("hold evaluator", "server", "COMPANION_TACTIC_HOLD_POSITION"),
    ("retreat evaluator", "server", "lifePercent <= retreatHpPercent"),
    ("preferred range combat", "server", "EngageTarget(character, target, preferredRange)"),
    ("formation API", "server", "bool GetFollowPosition"),
    ("legacy follow bridge", "character", "gameplay::companions::GetFollowPosition(this, &followX, &followZ)"),
    ("client tactic model", "models", "enum class CompanionUiTactic"),
    ("gateway tactic mapping", "gateway", "CompanionUiAction::SetTactic"),
    ("tactic UI", "page", "Tatica:"),
    ("retreat UI", "page", "Recuar HP:"),
    ("range UI", "page", "Range:"),
]
errors=[]
for label,name,needle in checks:
    if needle not in text[name]:
        errors.append(f"{label}: missing {needle!r} in {files[name].relative_to(ROOT)}")

# Persisting equipment reset must not erase behavior fields.
server=text["server"]
if "LoadoutOverride cleared" in server:
    errors.append("equipment reset still reconstructs a default profile and may erase tactics")

if errors:
    print("Companion tactics/profile V2: FAIL")
    for e in errors:
        print("  -", e)
    raise SystemExit(1)
print("Companion tactics/profile V2: PASS")
