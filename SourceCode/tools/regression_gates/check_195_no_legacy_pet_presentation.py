#!/usr/bin/env python3
from pathlib import Path
import sys

r = Path(__file__).resolve().parents[2]
def read(path):
    return path.read_bytes().decode("utf-8", errors="replace")

sinmain = read(r / "SrcGame/src/core/client/legacy_sin_runtime/sinMain.cpp")
pet = read(r / "SrcGame/src/systems/ui/help/legacy_sin/sinHelpPet.cpp")
hud = read(r / "SrcGame/src/systems/ui/v2/legacy/LegacyNotificationHud.cpp")
client_3d = read(r / "SrcGame/src/systems/rendering/effects/legacy_sin/sin3D.cpp")
server_3d = read(r / "SrcServer/src/systems/rendering/effects/sin_legacy/sin3D.cpp")

forbidden = [
    "cHelpPet.Draw();",
    "cHelpPet.DrawText();",
    "cHelpPet.LButtonDown(",
    "cHelpPet.LButtonUp(",
    "cHelpPet.RButtonDown(",
    "cHelpPet.RButtonUp(",
    "cHelpPet.KeyDown();",
]
leaked = [token for token in forbidden if token in sinmain]
if leaked:
    print("PATCH 195 legacy HelpPet calls still reachable from sinMain: " + ", ".join(leaked), file=sys.stderr)
    sys.exit(1)
if "cHelpPet.Draw(&Posi, &Angle);" in client_3d or "cHelpPet.Draw(&Posi, &Angle);" in server_3d:
    print("PATCH 195 legacy HelpPet 3D draw hook still reachable", file=sys.stderr)
    sys.exit(1)

required = [
    ("legacy producer main remains", "cHelpPet.Main();", sinmain),
    ("V2 short pet notice", "cHelpPet.szPetBuff", hud),
    ("V2 tutorial pet notice", "cHelpPet.HelpMsgId", hud),
    ("hidden old hitbox suppressed", "!notificationUiV2OwnsPresentation", pet),
]
missing = [name for name, token, text in required if token not in text]
if missing:
    print("PATCH 195 missing: " + ", ".join(missing), file=sys.stderr)
    sys.exit(1)

print("PATCH 195 gate: OK")
