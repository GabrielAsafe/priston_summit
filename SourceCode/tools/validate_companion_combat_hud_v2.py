#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
checks = [
    ("runtime opcode", "Shared/gameplay/companions/CompanionProtocol.h", "OPCODE_COMPANION_RUNTIME_STATE"),
    ("runtime size guard", "Shared/gameplay/companions/CompanionProtocol.h", "sizeof(CompanionRuntimeStatePacket) < 1024"),
    ("server 1Hz throttle", "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "lastRuntimeSendTime) < 1000U"),
    ("server runtime sender", "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "SendRuntimeStateLocked"),
    ("no DAT runtime sender", "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp", "Compact 1 Hz HUD telemetry"),
    ("client packet dispatch", "SrcGame/src/systems/network/v2/legacy/LegacyClientGameplayPacketRouter.h", "ApplyLegacyCompanionRuntimePacket"),
    ("gateway runtime snapshot", "SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp", "ApplyRuntimeState"),
    ("combat HUD", "SrcGame/src/systems/ui/v2/hud/CompanionPartyHud.cpp", "CompanionPartyHud::OnRender"),
    ("quick mode", "SrcGame/src/systems/ui/v2/hud/CompanionPartyHud.cpp", "CompanionUiAction::SetMode"),
    ("quick focus", "SrcGame/src/systems/ui/v2/hud/CompanionPartyHud.cpp", "CompanionUiAction::SetTactic"),
    ("quick dismiss", "SrcGame/src/systems/ui/v2/hud/CompanionPartyHud.cpp", "CompanionUiAction::Dismiss"),
    ("bridge registration", "SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp", "BuildCompanionPartyHud"),
]
errors=[]
for label,rel,needle in checks:
    path=ROOT/rel
    text=path.read_text(errors="ignore") if path.exists() else ""
    if needle not in text:
        errors.append(f"{label}: missing {needle!r} in {rel}")

# Runtime sender must not call the character DAT loader in its own body.
server=(ROOT/"SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp").read_text(errors="ignore")
start=server.find("void SendRuntimeStateLocked")
end=server.find("\nvoid SendFullCompanionInfo", start)
if start < 0 or end < 0:
    errors.append("could not isolate SendRuntimeStateLocked")
elif "LoadCharacterSnapshot" in server[start:end] or "rsLoadAccountCharacterRecord" in server[start:end]:
    errors.append("runtime HUD sender performs character DAT I/O")

if errors:
    print("Companion combat HUD V2: FAIL")
    for error in errors:
        print("  -", error)
    raise SystemExit(1)
print("Companion combat HUD V2: PASS")
