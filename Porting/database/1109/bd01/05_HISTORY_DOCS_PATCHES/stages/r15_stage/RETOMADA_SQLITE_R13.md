# RETOMADA — SQLite WIP R13

Data: 2026-09-06

## Baseline contract

- R6.1 remains the last compile/link/runtime-confirmed functional baseline.
- R13 is experimental SQLite migration work; do not use it as gameplay oracle until user validation.
- R12 is the frozen predecessor for this delta.

## R13 delta from R12

R13 removes the active `sod2info.asp` dependency from the server.

The exact Sandurr 2006 ASP response contract was recovered and cross-checked with the existing `cSINSOD2` parser. The historical page uses ClanDB `CL`/`UL` for these UI calls; it does not read `SOD2RecBySandurr`.

### index 1 — main Karina page

`ISAOCHECK::read_sod2(..., 1, ...)` now reads SQLite `ClanDB__CL`/`ClanDB__UL` directly and reproduces:

- `Code=0`: character is not in a clan (or no clan ranking exists);
- `Code=1`: leader of the top-ranked clan;
- `Code=2`: sub-chief (`Permi=2`) of the top-ranked clan;
- `Code=3`: ordinary member of the top-ranked clan;
- `Code=4`: leader of a non-winning clan;
- `Code=5`: sub-chief of a non-winning clan;
- `Code=6`: ordinary member of a non-winning clan.

The response remains pipe-separated and preserves the historical fields consumed by `Split_Sod2DaTa`: `CClanMoney`, `CTax`, `CName`, `CNote`, `CZang`, `CIMG`, and, for codes 1/2/3, `TotalEMoney`/`TotalMoney`.

### index 3 — high-score clan list

The response is generated directly from `ClanDB__CL ORDER BY Cpoint DESC`, using the historical CR-delimited protocol:

`Code=1\rCIMG=...\rCName=...\rCPoint=...\rCRegistDay=...\r`

The ASP examined at most its first nine rows; R13 preserves that limit. Non-positive Cpoint rows and the legacy substring-style duplicate-name check are also preserved.

### HTTP

The server implementation of `read_sod2` no longer contains `GetHttpRequest` or a `sod2info.asp` URL. The client-side/oracle copies are not changed by this server migration.

## Schema

No schema change in R13.

- `PRAGMA user_version = 17`
- 113 user tables

The SoD2 web page depends on the already-proven ClanDB `CL`/`UL` schemas, so adding a second SoD-specific representation would be incorrect.

## Validation

New validator: `validate_sod2_sqlite_contract.py`.

It checks source routing/HTTP removal and an executable SQLite contract for index 1 roles 0..6, leader/sub-chief/member money fields, index 3 serialization/order/date trimming, first-nine-row cap, and no-clan behavior.

All existing SQLite/clan/build/structure validators remain PASS. See `docs/SQLITE_WIP_R13_VALIDATION.md`.

## Remaining priorities

1. Continue the `Database.dll` replacement: reconstruct BillingDB `<initial>CHANNEL` and provider tables from supplied `.bak` metadata before widening local login.
2. `makeClan`: still blocked by the missing default initial clan-mark BMP/content.
3. `LeaveClan`/leader transfer: keep blocked until the exact update/log + return-code contract is proven.
4. warehouse/updateFlag: keep blocked until persistence semantics are proven.
5. Remove optional DLL/WinInet deployment dependencies only after the remaining active paths are replaced or deliberately retained.
