# Summit SQLite WIP R17

Experimental checkpoint for the SQL Server/web -> direct SQLite migration.

**Read `RETOMADA_COMPLETA_R17.md` first.** It is the canonical restart document and consolidates the original checkpoint plus everything learned through R17. Then read `CONHECIMENTO_CONSOLIDADO_CHAT_R17.md` for the technical lesson/history ledger.

Key state:

- R6.1 remains the last user-confirmed compile/link/runtime functional baseline.
- R17 schema is unchanged from R15/R16: `PRAGMA user_version = 29`, 332 user tables.
- 326/326 tables recovered from the supplied SQL Server backups have SQLite representations.
- Six additional compatibility tables are explicitly source-derived, not backup-proven.
- Active clan/notice/SoD2/mark flows are overwhelmingly local SQLite/storage.
- Local login accepts only the proven simple/free account subset; paid/PC-room states remain legacy-DLL fallback-only.
- R17 neutralizes dead/obsolete HTTP paths in the active server bridge.
- Exactly one HTTP literal remains in that bridge: `clanChipChange.asp` (leader transfer), deliberately preserved because its exact legacy write/log/code contract is still unproven.
- `SrcServer` itself contains no direct HTTP/HTTPS literal after the server-side homepage symbol is neutralized.

Artifacts:

- source: `current/SourceCode_SQLite_WIP/`
- R16 -> R17 patch: `migration/sqlite_wip_r16_to_r17.patch`
- changed-file hashes: `migration/SQLITE_WIP_R17_CHANGED_FILES.tsv`
- HTTP status: `migration/HTTP_REMAINING_R17.tsv`
- validation: `docs/SQLITE_WIP_R17_VALIDATION.md`
- raw validation: `docs/R17_VALIDATION_RAW.txt`
- patch replay proof: `docs/R17_PATCH_APPLY.txt`
- original restart checkpoint: `docs/RETOMADA_ORIGINAL_2026-09-06.md`
- full current restart: `RETOMADA_COMPLETA_R17.md`
- conversation knowledge ledger: `CONHECIMENTO_CONSOLIDADO_CHAT_R17.md`

R17 is not Windows compile/link/runtime-confirmed. Do not promote it over R6.1 until target regression succeeds.
