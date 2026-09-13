# Summit SQLite WIP R12

R6.1 remains the functional baseline. R12 is an experimental SQLite migration checkpoint.

R12 adds a conservative SQLite-first login path for proven simple/free AccountDB accounts and makes the active notice query column-explicit. Paid/billing/PC-room states still fall back to the legacy Database.dll when available.

See `RETOMADA_SQLITE_R12.md`, `docs/SQLITE_WIP_R12_VALIDATION.md`, and `migration/sqlite_wip_r11_to_r12.patch`.
