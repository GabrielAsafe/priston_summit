#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[5]
paths = [
    root / "SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp",
    root / "SrcServer/src/systems/inventory/legacy/sinInvenTory.cpp",
]

for path in paths:
    text = path.read_text(errors="ignore")
    marker = "void cINVENTORY::Main(bool processPointerHitTest)" if "void cINVENTORY::Main(bool processPointerHitTest)" in text else "void cINVENTORY::Main()"
    start = text.index(marker)
    end = text.index("void cINVENTORY::Close()", start)
    body = text[start:end]

    required = (
        "InventoryUseDelayPolicy::AdvanceCrystal(dwUseCristalDelayTime)",
        "InventoryUseDelayPolicy::AdvancePotion(dwUsePotionDelayTime)",
        "dwUseCristalDelayTime = delayState.elapsedTicks",
        "dwUsePotionDelayTime = delayState.elapsedTicks",
    )
    for token in required:
        if token not in body:
            raise SystemExit(f"{path}: missing V2 use-delay authority: {token}")

    forbidden = (
        "dwUseCristalDelayTime++",
        "dwUseCristalDelayTime > 70",
        "dwUsePotionDelayTime++",
        "dwUsePotionDelayTime > 50",
    )
    for token in forbidden:
        if token in body:
            raise SystemExit(f"{path}: inline use-delay transition remains: {token}")

print("sinInventory use delay authority guard: PASS")
