#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
handler = (root / "SrcServer/src/systems/commerce/restoration/cRestaureHandler.cpp").read_text(errors="replace")
policy = (root / "Shared/gameplay/commerce/v2/RestorationPricingPolicy.h").read_text(errors="replace")

required = [
    "RestorationPricingPolicy::ChargePrice",
    "RestorationPricingPolicy::QuotedPrice",
]
for token in required:
    if token not in handler:
        raise SystemExit(f"missing handler policy use: {token}")

for literal in ["Price = 100000", "CoinPrice = 6000", "CoinPrice = 18000", "CoinPrice = 1300"]:
    if literal in handler:
        raise SystemExit(f"legacy restoration pricing authority remains: {literal}")

if "QuotedPrice" not in policy or "ChargePrice" not in policy:
    raise SystemExit("pricing policy does not expose both legacy-compatible price views")

print("restoration pricing policy validator: PASS")
