# Summit SQLite WIP R15

Experimental checkpoint for the SQL Server/web -> direct SQLite migration.

Read `RETOMADA_SQLITE_R15.md` first.

Key state:

- R6.1 remains the functional regression baseline.
- R15 schema: `PRAGMA user_version = 29`, 332 user tables.
- All 326 tables recovered from the supplied SQL Server backups have a physical SQLite representation.
- R15 adds 176 backup-proven tables over R14 through migrations 019–029.
- Paid billing/PC-room semantics remain intentionally unexpanded.

Artifacts:

- source: `current/SourceCode_SQLite_WIP/`
- delta: `migration/sqlite_wip_r14_to_r15.patch`
- changed hashes: `migration/SQLITE_WIP_R15_CHANGED_FILES.tsv`
- full recovered catalog: `migration/BACKUP_CATALOG_R15.tsv`
- completion matrix: `migration/BACKUP_SCHEMA_COMPLETION_R15.tsv`
- validation: `docs/SQLITE_WIP_R15_VALIDATION.md`

R15 is not yet Windows compile/link/runtime-confirmed by the user.
