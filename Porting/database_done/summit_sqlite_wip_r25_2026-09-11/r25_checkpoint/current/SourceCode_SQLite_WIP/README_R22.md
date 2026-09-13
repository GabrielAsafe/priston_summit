# Summit SQLite WIP R22

R22 moves non-authoritative Database.dll logging sinks to local SQLite while
keeping legacy fallback on local infrastructure failure.

Key additions:

- SoD result statistics -> SQLite-first (`SoD2DB__SOD2RecBySandurr`)
- character new/delete/logout audit -> `GameLogDB__LocalCharacterEvent`
- item/trade/gift audit -> `ItemLogDB__LocalItemEvent` + entries
- migration `031_local_audit_events.sql`
- schema v31 / 336 user tables
- controlled validation 31/31 PASS

See `RETOMADA_COMPLETA_R22.md`, `LOCAL_AUDIT_SINKS_R22.md` and
`SQLITE_WIP_R22_VALIDATION.md`.
