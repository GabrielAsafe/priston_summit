# Summit SQLite WIP R16

R16 is an experimental SQLite migration checkpoint built on R15.

Key change: active clan creation (`makeClan` / historical `clanInsert.aspx`) is now handled directly through transactional SQLite/ClanDB with local best-effort clan-mark copying.

Schema remains migration version 29 with 332 user tables.

Read `RETOMADA_SQLITE_R16.md` first and `docs/SQLITE_WIP_R16_VALIDATION.md` for validation details.

R6.1 remains the last compile/link/runtime-confirmed functional baseline.
