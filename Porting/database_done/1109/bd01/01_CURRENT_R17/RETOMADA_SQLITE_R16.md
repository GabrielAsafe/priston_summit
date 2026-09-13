# RETOMADA — SQLite WIP R16

Data: 2026-09-06

## 1. Baseline contract

- R6.1 remains the last compile/link/runtime-confirmed functional baseline.
- R16 is experimental SQLite migration work and must not be used as the gameplay oracle until target Windows validation.
- R15 is the frozen predecessor for the R15 -> R16 delta.
- Do not mix unrelated gameplay/refactor fixes into this migration.

## 2. Purpose of R16

R16 removes the active `clanInsert.aspx` dependency from clan creation and reproduces the proven legacy database/protocol contract directly through `ClanDB`/SQLite.

No schema migration is added in R16. The physical schema remains:

- `PRAGMA user_version = 29`
- 332 user tables
- 326 backup-proven tables + 6 explicitly source-derived compatibility tables

## 3. Clan creation contract now local

`ISAOCHECK::makeClan(...)` no longer calls HTTP.

The local transaction reproduces the historical `clanInsert.aspx` behavior that is supported by the recovered script and current source contract:

- missing required identity/server/character/clan inputs -> `100`;
- character already associated with a non-empty clan -> `2`;
- stale `UL` row with empty `ClanName` is removed before creation;
- duplicate clan name -> `3`;
- mark allocator uses `LI.ID=1` and starts from legacy `Icon_Default = 1000000000`;
- first allocated mark is `1000000001`;
- `CL` and leader `UL` rows are created in one SQLite transaction;
- default note is `New clan`;
- `LimitDate` is initialized to local current time + 3600 days;
- successful response remains `Code=1\rCMoney=500000\r`;
- the default mark copy is best-effort and does not determine database success.

The default mark, when present, is copied locally from:

`Database/clan_marks/1000000000.bmp`

to the newly allocated mark path under `Database/clan_marks/`.

No default BMP is synthesized when the source asset is absent.

## 4. Current-schema compatibility adaptation: LIDER_BC

The historical clan creation script predates the `LIDER_BC` column present in the supplied modern ClanDB backup.

The recovered current schema has `CL.LIDER_BC` as `NOT NULL`, while `makeClan` has no legacy input that could provide a value for it. R16 therefore initializes `LIDER_BC=0` explicitly when creating a clan.

Treat this as a current-schema compatibility initializer, not as a claim that the old ASP wrote that column.

## 5. Transaction behavior

Clan creation uses the existing `SQLConnection` transaction API:

1. `BEGIN IMMEDIATE`;
2. validate/clean character membership;
3. validate clan-name uniqueness;
4. allocate/update `LI`;
5. insert `CL`;
6. retrieve `CL.IDX`;
7. insert leader `UL`;
8. commit;
9. best-effort local mark copy after commit.

Any database failure before commit rolls back the operation and returns `100`.

## 6. Validation added

`validate_clan_sqlite_contract.py` now covers clan creation in addition to the already migrated clan behaviors.

It verifies:

- first mark allocation `1000000001`;
- `LI` progression;
- expected `CL` initialization;
- expected leader `UL` row;
- response `Code=1\rCMoney=500000\r`;
- duplicate membership result `2`;
- duplicate clan-name result `3`;
- stale empty-`ClanName` `UL` cleanup;
- second allocation `1000000002`;
- static absence of HTTP in the active `makeClan` implementation;
- transaction/local-mark-copy tokens in the C++ path.

## 7. Validation status

PASS in the available environment:

- backup catalog completion — 326/326;
- BillingDB contract — 43/43;
- SoD2 local contract;
- migrations — v29 / 332 / idempotence;
- SQL query coverage;
- clan contract including create/mark allocation;
- SQLite transaction API;
- clan mark/tag local storage;
- local/free SQLite login;
- notice SQLite contract;
- source graph — only authorized new server TU remains `SQLiteConnection.cpp`;
- frozen build/link scripts;
- port structure — 1517/1517;
- compiler adaptation ledger.

See `docs/SQLITE_WIP_R16_VALIDATION.md`.

## 8. Remaining active blockers

The principal unresolved clan/web behaviors after R16 are:

1. leader transfer (`clanChipChange.asp` / legacy `LeaveClan` naming) — exact update/log/code sequence still not fully proven;
2. warehouse rights (`clan_WarehouseRight.php`) — return contract exists but persisted field/semantics are not yet proven;
3. `updateFlag.php` — likewise requires proof before replacing the web call;
4. dead/legacy HTTP helpers should be removed only after caller classification is complete.

Paid/PC-room billing remains fallback-only to the legacy DLL path; schema presence does not prove the historical billing state machine.

## 9. Runtime promotion rule

R16 is not a functional baseline until the target Windows environment passes compile, link and runtime regression. R6.1 remains the functional oracle.
