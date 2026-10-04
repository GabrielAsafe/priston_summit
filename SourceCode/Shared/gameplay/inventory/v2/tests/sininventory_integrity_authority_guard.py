#!/usr/bin/env python3
from pathlib import Path
import re

root = Path(__file__).resolve().parents[5]
sources = [
    root / "SrcServer/src/systems/inventory/legacy/sinInvenTory.cpp",
    root / "SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp",
]

for source in sources:
    text = source.read_text(errors="ignore")
    if '#include "gameplay/inventory/v2/InventoryIntegrityService.h"' not in text:
        raise SystemExit(f"missing inventory integrity service include: {source}")
    if "CaptureLegacyInventoryIntegritySnapshot" not in text:
        raise SystemExit(f"missing legacy integrity snapshot adapter: {source}")

    for function in ("CheckInvenItemForm", "ReFormInvenItem"):
        match = re.search(
            r"(?:int|void)\s+cINVENTORY::" + function + r"\s*\([^)]*\)\s*\{(?P<body>.*?)\n\}",
            text,
            re.S,
        )
        if not match:
            raise SystemExit(f"missing {function}: {source}")
        body = match.group("body")
        if "InventoryIntegrityService::ComputeFingerprint" not in body:
            raise SystemExit(f"{function} does not consume InventoryIntegrityService: {source}")
        for legacy_formula in (
            "(i + 1)*InvenItem[i].y",
            "(i + 1)*InvenItemTemp[i].y",
            "ItemHeader.dwChkSum;",
        ):
            if legacy_formula in body:
                raise SystemExit(f"legacy fingerprint formula remains in {function}: {source}: {legacy_formula}")

print("sinInventory integrity authority guard: PASS")
