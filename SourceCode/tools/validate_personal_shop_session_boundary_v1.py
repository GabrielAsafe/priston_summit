#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
contract = (root / "Shared/gameplay/commerce/v2/PersonalShopService.h").read_text(encoding="utf-8", errors="replace")
adapter = (root / "SrcServer/src/systems/commerce/v2/legacy/LegacyPersonalShopAdapter.cpp").read_text(encoding="utf-8", errors="replace")
server = (root / "Shared/server/legacy_runtime/OnSever_legacy_server.inl").read_text(encoding="utf-8", errors="replace")

anchor = "int\trsTrade_PersonalShop(rsPLAYINFO* lpPlayInfo, rsPLAYINFO* lpPlayInfo2, smTRANS_COMMAND_BUFF* lpTransCommandBuff)"
if anchor not in server:
    raise SystemExit("personal shop session boundary v1: FAIL: trade function missing")
trade = server.split(anchor, 1)[1].split("//Vl", 1)[0]
case1 = trade.split("case 1:", 1)[1].split("case 2:", 1)[0]
message = server.split("case OpCode::OPCODE_MYSHOP_MESSAGE:", 1)[1].split("#if 0", 1)[0]

checks = {
    "prepare contract": "struct PersonalShopPrepareRequest" in contract and "PrepareTrade" in contract,
    "message contract": "struct PersonalShopMessageRequest" in contract and "ReadListingMessage" in contract,
    "green contract clean": all(x not in contract for x in ("rsPLAYINFO", "sMYSHOP_ITEM_SERVER", "dwMyShopTradeTime", "sMyShopList")),
    "case1 delegates": "ServerLegacyPersonalShopAdapter().PrepareTrade(request)" in case1,
    "case1 no list authority": "MyShopListCount" not in case1 and "sMyShopList" not in case1 and "rsFindInvenItem" not in case1,
    "adapter cooldown": "buyer->dwMyShopTradeTime > dwPlayServTime" in adapter and "buyer->dwMyShopTradeTime = dwPlayServTime + 500" in adapter,
    "adapter item availability": "inventoryQuery_.Contains(request.seller, item).Present()" in adapter and (("potionStackQuery_.TryGetStack(" in adapter) or ("rsGetServerPotion(seller" in adapter)),
    "adapter list tracking": "buyer->sMyShopList[slot]" in adapter and "buyer->MyShopListCount++" in adapter,
    "message delegates": "ServerLegacyPersonalShopAdapter().ReadListingMessage" in message,
    "message handler no lookup": "srFindUserFromSerial" not in message and "dwTradeMsgCode" not in message and "szTradeMessage" not in message,
    "adapter message guard": "target->dwTradeMsgCode" in adapter and "result.message = target->szTradeMessage" in adapter,
    "wire reply preserved": "TransChatMessage.code = OpCode::OPCODE_MYSHOP_MESSAGE" in message and "TransChatMessage.dwObjectSerial = message.target.value" in message,
}

for name, ok in checks.items():
    print(("PASS" if ok else "FAIL") + ": " + name)
if not all(checks.values()):
    sys.exit(1)
print("personal shop session boundary v1: PASS")
