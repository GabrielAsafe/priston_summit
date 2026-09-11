# SQLite migration backend

The legacy server keeps ODBC as the default backend. SQLite is opt-in per
`EDatabaseID`, allowing one subsystem to be migrated and validated at a time.

Runtime switches:

- `SUMMIT_SQLITE_DATABASES=QuestDB,CoinShopDB` enables SQLite only for the
  listed database names. Use `*` to opt every database in.
- `SUMMIT_SQLITE_DIR=Database` changes the directory that stores the files.
  The default is `Database`.
- `sqlite3.dll` must be available beside the server executable or in `PATH`.

Each selected database is stored as `<directory>/<DatabaseName>.sqlite3`.

Do not enable a database before its schema and non-portable SQL have been
migrated. The adapter converts `GETDATE()` to `CURRENT_TIMESTAMP` and simple
`SELECT TOP N` queries to `LIMIT N`. It intentionally rejects SQL Server-only
control flow, cross-database names and SQL Server collations so migration gaps
fail visibly instead of changing behaviour silently.
