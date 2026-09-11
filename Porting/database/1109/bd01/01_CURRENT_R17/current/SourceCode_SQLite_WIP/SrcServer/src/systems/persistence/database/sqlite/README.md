# SQLite migration backend

Current migration architecture (WIP):

- both legacy public database APIs are preserved (`SQLConnection` and `SQL`);
- both APIs use `SQLiteConnection` internally;
- every logical `EDatabaseID` opens the same physical file: `Database/summit.sqlite3`;
- physical tables are namespaced as `<LogicalDB>__<Table>` (for example `AccountDB__GameMasters` and `QuestDB__Progress`);
- explicit SQL Server three-part names and bare legacy table names are translated by `SqliteQueryCompat`;
- `sqlite3.dll` is loaded dynamically and must be an x86 build available beside the server executable or in `PATH`.

The compatibility layer currently handles the SQL Server constructs still found in active callers: qualified database names, `SELECT TOP N`, `GETDATE()` (preserved as server-local `datetime('now','localtime')`), `DATEADD(DAY, literal, GETDATE())`, `DAY/MONTH`, and known collations. SQL Server control-flow (`IF/BEGIN/ELSE`) is intentionally rejected; those queries must be migrated explicitly at the caller so bind numbering and write semantics stay reviewable.

`SqliteQueryCompat` is implemented as an `.inl` included by `SQLiteConnection.cpp`. This is intentional: the validated source graph authorizes only one new server translation unit for the SQLite backend.

Schema migration is versioned under `Database/migrations/`. Do not enable/claim a logical database as migrated until all of its tables, positional `SELECT *` contracts, required keys/constraints, and seeds have been reconstructed and validated. In particular, `INSERT OR IGNORE` / `INSERT OR REPLACE` must not be relied on without a matching UNIQUE/PRIMARY KEY constraint.

## Automatic migration bootstrap

`SQLiteConnection::Init()` applies the ordered files `001` through `008` from `Database/migrations/` against the shared physical database before the connection becomes usable. Migration state is stored in `PRAGMA user_version`; every migration is transactional (`BEGIN IMMEDIATE` / rollback on error). Pending migration files are mandatory, and a database whose `user_version` is newer than the server-supported version is rejected rather than silently downgraded.

This bootstrap only guarantees the currently reconstructed schema subset. It does not claim that all historical logical databases/tables are ready yet.
