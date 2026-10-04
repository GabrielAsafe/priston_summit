#!/usr/bin/env python3
import argparse, json, pathlib, sys

ap = argparse.ArgumentParser(description="Valida CoinShop JSON contra perfis de itens client/server.")
ap.add_argument("--catalog", required=True)
ap.add_argument("--items", required=True, help="Diretório canonical/item_profiles")
args = ap.parse_args()

catalog = json.load(open(args.catalog, encoding="utf-8"))
profiles = {}
for name in ("client", "server"):
    p = pathlib.Path(args.items) / (name + ".items.json")
    d = json.load(open(p, encoding="utf-8"))
    profiles[name] = {str(code).upper() for code in d.get("codes", [])}

issues = []
seen = set()
by_currency = {}
for index, item in enumerate(catalog.get("items", [])):
    code = str(item.get("code", "")).strip()
    currency = str(item.get("currency", "")).strip()
    section = item.get("section")
    price = item.get("price")
    key = (currency, code.upper())

    if not code:
        issues.append({"index": index, "issue": "empty_code"})
    if currency not in ("coin", "time"):
        issues.append({"index": index, "code": code, "issue": "invalid_currency", "value": currency})
    if not isinstance(section, int) or not (1 <= section <= 7):
        issues.append({"index": index, "code": code, "issue": "invalid_section", "value": section})
    if not isinstance(price, int) or price <= 0:
        issues.append({"index": index, "code": code, "issue": "invalid_price", "value": price})
    if key in seen:
        issues.append({"index": index, "code": code, "issue": "duplicate_in_currency", "currency": currency})
    seen.add(key)

    upper = code.upper()
    if upper not in profiles["client"]:
        issues.append({"index": index, "code": code, "issue": "missing_client_profile"})
    if upper not in profiles["server"]:
        issues.append({"index": index, "code": code, "issue": "missing_server_profile"})

    by_currency[currency] = by_currency.get(currency, 0) + 1

result = {
    "schema": catalog.get("schema"),
    "items": len(catalog.get("items", [])),
    "by_currency": by_currency,
    "issues": issues,
    "issue_count": len(issues),
}
print(json.dumps(result, ensure_ascii=False, indent=2))
sys.exit(1 if issues else 0)
