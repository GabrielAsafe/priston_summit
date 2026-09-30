#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "Shared/gameplay/inventory/v2/Inventory.h"
ADAPTER_H = ROOT / "SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
ADAPTER_CPP = ROOT / "SrcServer/src/systems/inventory/v2/legacy/LegacyInventoryRuntime.cpp"


def fail(message: str) -> None:
    print(f"inventory boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (CONTRACT, ADAPTER_H, ADAPTER_CPP):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    header = ADAPTER_H.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER_CPP.read_text(encoding="utf-8", errors="replace")

    for token in (
        "struct ItemInstanceId",
        "enum class InventoryPresenceCode",
        "enum class InventoryMutationCode",
        "class IInventoryQuery",
        "class IInventoryCommands",
        "Contains(PlayerId player, ItemInstanceId item)",
        "Add(PlayerId player, ItemInstanceId item)",
        "Remove(PlayerId player, ItemInstanceId item)",
        "AlreadySatisfied",
        "CapacityExceeded",
    ):
        if token not in contract:
            fail(f"GREEN inventory contract missing: {token}")

    for forbidden in (
        "rsPLAYINFO",
        "DWORD",
        "sinInvenTory",
        "rsFindInvenItem",
        "rsAddInvenItem",
        "rsDeleteInvenItem",
        "InvenItemInfo",
    ):
        if forbidden in contract:
            fail(f"GREEN inventory contract leaks legacy symbol: {forbidden}")

    for token in (
        "class LegacyInventoryRuntime final : public IInventoryQuery, public IInventoryCommands",
        "LegacyOnlinePlayerRegistry",
        "ServerLegacyInventoryRuntime",
    ):
        if token not in header:
            fail(f"inventory adapter header missing: {token}")

    for token in (
        "registry_.Resolve(player)",
        "rsFindInvenItem(",
        "rsAddInvenItem(",
        "rsDeleteInvenItem(",
        "InventoryMutationCode::AlreadySatisfied",
        "InventoryMutationCode::CapacityExceeded",
        "InventoryMutationCode::ItemNotFound",
    ):
        if token not in adapter:
            fail(f"inventory adapter implementation missing: {token}")

    print("inventory boundary v1: PASS (exact item query/add/remove isolated behind GREEN ports)")


if __name__ == "__main__":
    main()
