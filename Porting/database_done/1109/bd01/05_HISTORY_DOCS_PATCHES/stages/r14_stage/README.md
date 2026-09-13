# Summit SQLite WIP R14

Experimental checkpoint for the SQL Server/web -> direct SQLite migration.

Read `RETOMADA_SQLITE_R14.md` first.

- source: `current/SourceCode_SQLite_WIP/`
- delta: `migration/sqlite_wip_r13_to_r14.patch`
- changed-file hashes: `migration/SQLITE_WIP_R14_CHANGED_FILES.tsv`
- recovered BillingDB audit catalog: `migration/BILLINGDB_BACKUP_CATALOG_R14.tsv`
- validation: `docs/SQLITE_WIP_R14_VALIDATION.md`

R6.1 remains the functional regression baseline; R14 is not yet compile/link/runtime-confirmed by the user.
