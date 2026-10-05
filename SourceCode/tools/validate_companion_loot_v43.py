#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
paths = {
    'system': ROOT / 'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp',
    'onserver_h': ROOT / 'SrcServer/src/core/server/legacy_runtime/onserver.h',
    'onserver': ROOT / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl',
    'client_net': ROOT / 'SrcGame/src/systems/network/client/netplay.cpp',
    'client_inven': ROOT / 'SrcGame/src/core/client/legacy_sin_runtime/sinSubMain.cpp',
    'protocol': ROOT / 'Shared/gameplay/companions/CompanionProtocol.h',
}
text = {k: p.read_text(errors='ignore') for k, p in paths.items()}
errors = []

def req(label, key, needle):
    if needle not in text[key]:
        errors.append(f"{label}: missing {needle!r} in {paths[key].relative_to(ROOT)}")

# Runtime state and safe defaults.
for needle in (
    'STG_AREA *lootArea;', 'STG_ITEMS *lootTarget;', 'nextLootScanTime', 'nextLootPickupTime',
    'CompanionLootConfig', '"LootEnabled", 0', '"LootQuestItems", 0',
    '"LootGold", 1', '"LootCrystals", 1', '"LootConsumables", 1', '"LootEquipment", 1',
    '"LootOther", 0', '"LootUnique", 1', '"LootMinimumPrice"', '"LootMinimumDropLevel"',
    'GetCompanionLootCategory', 'ITEM_KIND_QUEST_WEAPON', 'sinEC1',
    'stageItem.dwExclusiveOwnerCode != owner->dwObjectSerial',
    'stageItem.Item.ItemInfo.CODE == sinQT136',
    'IsCompanionLootClaimed', 'FindCompanionLootTarget', 'LootOwnerLeash',
    'candidate->dwCreateTime - dwPlayServTime', 'LootMaxChaseMs',
    'TryHandleCompanionLoot(character, owner, *managedSlot, mode)',
    'mode == COMPANION_MODE_AGGRESSIVE && FindNearestHostile(character)',
    'LOOT_TARGET', 'LOOT_PICKUP',
):
    req('loot runtime', 'system', needle)

# Server pickup bridge must reuse the normal secure item handoff, not invent a
# companion inventory or directly mutate the owner inventory grid.
req('bridge declaration', 'onserver_h', 'int rsCompanionPickupStageItem(')
for needle in (
    'int rsCompanionPickupStageItem(',
    'dwExclusiveOwnerCode != lpPlayInfo->dwObjectSerial',
    'GetPartyMoney(&lpStgItem->Item.ItemInfo, lpPlayInfo)',
    'transItemInfo.code = OpCode::OPCODE_PUTITEM',
    'rsRegist_ItemSecCode(lpPlayInfo, &transItemInfo, lpStgItem->BeginMode)',
    'SendOnServerLegacySocket(lpPlayInfo->lpsmSock, (char *)&transItemInfo',
    'RecordGetHighItem(lpPlayInfo, lpStgItem)',
    'lpStgArea->DeleteItem(lpStgItem)',
):
    req('secure legacy pickup bridge', 'onserver', needle)

# Inventory-full/weight fallback is intentionally delegated to the established
# client pickup contract: if AutoSetInvenItem fails, the client throws it back.
for needle in ('case OpCode::OPCODE_PUTITEM:', 'PushRecvTransItemQue', 'sinSetInvenItem(&lpTransItemInfo->Item)'):
    req('client pickup contract', 'client_net', needle)
for needle in ('AutoSetInvenItem(&sinTempItem)', 'sinThrowItemToFeild(&sinTempItem)'):
    req('inventory full fallback', 'client_inven', needle)

# V43 is server/runtime-only: do not churn the UI protocol ABI.
req('protocol remains v6', 'protocol', 'COMPANION_UI_PROTOCOL_VERSION = 6')
if 'COMPANION_UI_PROTOCOL_VERSION = 7' in text['protocol']:
    errors.append('protocol: V43 loot must not bump UI ABI without a client editor contract')

if errors:
    print('Companion Loot V43: FAIL')
    for e in errors:
        print('  -', e)
    raise SystemExit(1)

print('Companion Loot V43: PASS (ownership + reservation + leash + filters + secure legacy pickup bridge)')
