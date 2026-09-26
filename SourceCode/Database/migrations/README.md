# SQLite schema migration status

These migrations target the single physical file `Database/summit.sqlite3`.
Legacy logical databases are represented by physical table prefixes such as
`AccountDB__`, `QuestDB__`, `CoinShopDB__`, and `ServerDB__`.

## Current schema

- latest schema version: **43**
- SQLite user tables after a clean bootstrap: **371**
- tables recovered from the 21 supplied SQL Server `.bak` catalogs: **326 / 326 present**
- additional source-derived compatibility tables: 6 (`QuestDB__RepeatedQuest`,
  `SecurityDB__AgingRecord`, `SecurityDB__BannedIP`, `SecurityDB__BannedMac`,
  `SecurityDB__ClassCheatList`, `StartUserDB__ExtraItems`)

Migrations `001` through `018` contain the previously reconstructed active
contracts, AccountDB, BillingDB, clan/member/archive contracts, and explicit
source-derived compatibility tables. Those migrations are intentionally kept
stable because active callers already depend on their documented contracts.

Migrations `019` through `029` complete the remaining **176 tables whose schema
is directly proven by the supplied SQL Server backups**:

- `019_billinglogdb_catalog.sql` — BillingLogDB (6 tables)
- `020_gamelogdb_catalog.sql` — GameLogDB (17)
- `021_itemlogdb_catalog.sql` — ItemLogDB (3)
- `022_logdb_catalog.sql` — LogDB (5)
- `023_pcroom_catalog.sql` — PCRoom (3)
- `024_pcroomlog_catalog.sql` — PCRoomLog (19)
- `025_rptdb_catalog.sql` — rPTDB (1)
- `026_sod2db_catalog.sql` — SoD2DB remaining catalog (9)
- `027_soddb_catalog.sql` — SodDB (69)
- `028_clandb_catalog.sql` — ClanDB remaining catalog (24)
- `029_remaining_backup_catalogs.sql` — BattlePass (1), CoinShopDB (2),
  PristonDB (16), SecurityDB (1)


Migrations `030` through `043` are server-local additions rather than reconstructed
SQL Server catalog tables:

- `030_local_gameplay_entitlement.sql` — independent local gameplay entitlement (1 table)
- `031_local_audit_events.sql` — append-only character/item audit payloads used to retire non-authoritative `Database.dll` sinks (3 tables)
- `032_local_auth_policy.sql` — explicit account auth ownership + runtime legacy billing fallback policy (2 tables)
- `033_progression_v2.sql` — Covenant/StoryState V2 authoritative persistence in the `ServerDB__` namespace (6 tables)
- `034_story_node_state.sql` — durable per-node StoryState history used by `StoryNodeState` conditions (1 table)
- `035_progression_content.sql` — data-empty server-authoritative Covenant/Story content catalog (19 tables)
- `036_covenant_rewards.sql` — durable Covenant reward receipts plus join/rank reward catalog (2 tables)
- `037_story_dialogue_choices.sql` — explicit server-authoritative Story dialogue branches and choice conditions (2 tables)
- `038_generic_reputation.sql` — explicit generic Reputation definitions/state/source ledger plus content CHECK upgrades (3 tables)
- `039_covenant_single_active.sql` — enforces one active Covenant membership per player
- `040_covenant_default_content.sql` — seeds the five V0.1 Covenants with cumulative Tier I–III modifier content

For the backup-proven tables, names, column order, SQLite affinity, NULLability
and SQL Server `IDENTITY` status are recovered from `sysschobjs` / `syscolpars`
in the supplied backups. An integer SQL Server identity is represented as
`INTEGER PRIMARY KEY AUTOINCREMENT`, consistent with the earlier migration
strategy. This completion is **schema-only**: it does not infer stored
procedures, billing state machines, gameplay rules, or missing web behavior.

`SQLiteConnection::Init()` applies pending migrations automatically using
`PRAGMA user_version`; each migration runs inside its own `BEGIN IMMEDIATE`
transaction and is rolled back on failure. A database with a schema version
newer than the server understands is rejected.

Validation helpers:

- `validate_sqlite_migrations.py` — clean bootstrap, migration sequencing,
  idempotence and active contracts.
- `validate_backup_catalog_completion.py` — proves that all 326 backup tables
  are physically present and validates exact order/affinity/NULLability/identity
  for the 176 tables added in R15.
- `validate_billingdb_sqlite_contract.py` — retained independent BillingDB R14
  oracle.

R6.1 remains the compile/link/runtime regression baseline until the SQLite WIP
is validated on the Windows server build and runtime.

- `041_covenant_runtime_complete.sql` — Covenant runtime state, challenges, mastery, collection/Sigils and Tier III powers

- `042_covenant_inventory.sql` — Covenant-only persistent inventory and Sigil backfill
- `043_covenant_artisan_mode.sql` — persisted chat-first Artisan crafting mode (`off/reforge/forgemaster/master`)

- `044_gathering_mining_runtime.sql` — data-driven Mining/Gathering resources/nodes and persistent resource inventory

- `045_gathering_serverdb_namespace_fix.sql` — moves Gathering/Mining runtime tables into the `ServerDB__*` namespace required by the SQLite compatibility layer
