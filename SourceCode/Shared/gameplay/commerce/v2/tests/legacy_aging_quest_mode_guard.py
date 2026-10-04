#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[5]
path = root / "SrcServer/src/systems/commerce/v2/legacy/LegacyAgingKernel.inl"
text = path.read_text(encoding="utf-8", errors="replace")
aging = text.split("int rsRecvAgingItem", 1)[1]

if '#include "gameplay/commerce/v2/AgingQuestModePolicy.h"' not in text:
    raise SystemExit("legacy aging quest-mode guard: missing V2 policy include")
if "AgingQuestModePolicy::Resolve" not in aging:
    raise SystemExit("legacy aging quest-mode guard: rsRecvAgingItem does not consume V2 policy")
if "static_cast<int>(agingQuestMode)" not in aging:
    raise SystemExit("legacy aging quest-mode guard: legacy integer mode mapping is not explicit")
if "ccItem.AgingItem(QuestMode);" not in aging:
    raise SystemExit("legacy aging quest-mode guard: AgingItem legacy call changed")

for direct in (
    "if((sItemInfo.CODE&sinITEM_MASK2) == sinMA1)",
    "QuestMode = TRUE;",
    "QuestMode = -1;",
    "QuestMode = FALSE;",
):
    if direct in aging:
        raise SystemExit(f"legacy aging quest-mode guard: direct classification remains: {direct}")

print("legacy aging quest-mode V2 guard: PASS")
