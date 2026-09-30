#!/usr/bin/env python3
"""Source-level regression guard for the legacy persisted hand-slot contract.

The companion reads offline TRANS_RECORD_DATA, while normal login reconstructs
client inventory state. This validator intentionally ties those two paths
together so ItemPosition 1/2 cannot silently become a companion-only guess.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]

client_inventory_path = ROOT / "SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp"
client_item_header_path = ROOT / "Shared/inventory/legacy/sinItem_legacy_client.h"
server_item_header_path = ROOT / "Shared/inventory/legacy/sinItem_legacy_server.h"
companion_path = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp"
server_bridge_path = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
character_record_adapter_path = ROOT / "SrcServer/src/systems/gameplay/player/v2/legacy/LegacyCharacterRecordAdapter.cpp"

paths = [
    client_inventory_path,
    client_item_header_path,
    server_item_header_path,
    companion_path,
    server_bridge_path,
    character_record_adapter_path,
]
missing = [str(path.relative_to(ROOT)) for path in paths if not path.exists()]
if missing:
    for path in missing:
        print(f"[FAIL] missing {path}")
    sys.exit(1)

client_inventory = client_inventory_path.read_text(encoding="latin1", errors="replace")
client_item_header = client_item_header_path.read_text(encoding="latin1", errors="replace")
server_item_header = server_item_header_path.read_text(encoding="latin1", errors="replace")
companion = companion_path.read_text(encoding="latin1", errors="replace")
server_bridge = server_bridge_path.read_text(encoding="latin1", errors="replace")
character_record_adapter = character_record_adapter_path.read_text(encoding="latin1", errors="replace")

failures = []

def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        failures.append(f"{label}: missing {needle!r}")

# Canonical slot order in the actual client inventory UI/runtime.
slot_array = re.search(
    r"sINVENTORY\s+sInven\[INVENTORY_MAX_POS\]\s*=\s*\{(?P<body>.*?)\n\};",
    client_inventory,
    re.S,
)
if not slot_array:
    failures.append("client inventory: sInven slot table not found")
else:
    body = slot_array.group("body")
    slot_tokens = re.findall(r"\{\s*(INVENTORY_POS_[A-Z0-9_]+)\s*,", body)
    if len(slot_tokens) < 2:
        failures.append("client inventory: fewer than two equipment slots parsed")
    else:
        if slot_tokens[0] != "INVENTORY_POS_RHAND":
            failures.append(f"client inventory: slot 1 is {slot_tokens[0]}, expected RHAND")
        if slot_tokens[1] != "INVENTORY_POS_LHAND":
            failures.append(f"client inventory: slot 2 is {slot_tokens[1]}, expected LHAND")

# The equip path writes ItemPosition from that table index and restores the same
# persisted number back into sInven[ItemPosition - 1].
require(client_inventory, "pItem->ItemPosition = i + 1;", "client equip slot serialization")
require(client_inventory, "if(!(sInven[i].Position & pItem->Class))", "client slot/class compatibility")
require(
    client_inventory,
    "sInven[InvenItem[j].ItemPosition - 1].ItemIndex = j + 1;",
    "client equipped-item restore",
)
require(client_inventory, "TempTwoHandItem.ItemPosition = 2;", "client two-hand mirror")

# Header masks must agree between client/server. Magical stuffs are a genuine
# LHAND class and must not disappear from the companion loadout picker.
for label, text in (("client item header", client_item_header), ("server item header", server_item_header)):
    require(text, "#define INVENTORY_POS_LHAND", label)
    require(text, "#define INVENTORY_POS_RHAND", label)
    require(text, "#define ITEM_CLASS_WEAPON_ONE", label)
    require(text, "#define ITEM_CLASS_WEAPON_TWO", label)
    require(text, "#define ITEM_CLASS_SHIELDS", label)
    require(text, "#define ITEM_CLASS_MAGICIAL_STUFFS", label)

# The persistence adapter now owns the packed-record boundary while Companion
# consumes a record-free DTO. Keep guarding the same ItemSubStart primary cut
# and ItemPosition 1/2 slot semantics across that boundary.
require(character_record_adapter, "int primaryItemCount = record.ItemSubStart;", "character repository primary inventory")
require(character_record_adapter, "translated.position = legacyItem.ItemPosition;", "character repository persisted slot translation")
require(companion, "recordItem.position == 1", "companion snapshot right slot")
require(companion, "recordItem.position == 2", "companion snapshot left slot")
require(companion, "(itemClass & INVENTORY_POS_LHAND) != 0", "companion left-hand class mask")
require(companion, "itemClass != ITEM_CLASS_WEAPON_TWO", "companion two-hand picker exclusion")
require(server_bridge, "int primaryItemCount = recordData->ItemSubStart;", "spawn bridge primary inventory")
require(server_bridge, "recordItem.ItemPosition == 1 || recordItem.ItemPosition == 2", "spawn bridge hand slots")
require(server_bridge, "sItem[handIndex[0]].Class == ITEM_CLASS_WEAPON_TWO", "spawn bridge two-hand collapse")

if failures:
    for failure in failures:
        print(f"[FAIL] {failure}")
    sys.exit(1)

print("Companion equipment/login slot contract: PASS")
