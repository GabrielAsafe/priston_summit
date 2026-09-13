# RETOMADA — SQLite WIP R11

Data: 2026-09-06

## Baseline contract

- R6.1 remains the last compile/link/runtime-confirmed functional baseline.
- R11 is experimental SQLite migration work; do not use it as gameplay oracle until user validation.
- Do not repair unrelated legacy gameplay/UB while DB/web migration is in progress.

## R11 delta from R10

R11 closes the active clan-mark read path and the clan-tag webroot filesystem dependency.

### Local clan mark reads

`ISAOCHECK::read_ClanImage` and `read_ClanImage2` now read:

`Database/clan_marks/<ClanMark>.bmp`

instead of HTTP `/ClanContent/<ClanMark>.bmp`.

The reader uses `CreateFileA(... OPEN_EXISTING ...)`, caps the file at 64 KiB, rejects path syntax in the legacy basename, requires a BMP `BM` header, and preserves the existing `clanDATA_save` / `bmpSAVE` pipeline and `1/2` return convention.

Missing marks remain missing. R11 does not create or guess the historical default `1000000000.bmp`.

### Clan tag customization

The server-side customization path no longer runs:

`copy C:\\inetpub\\wwwroot\\brnxcontenttags\\... C:\\inetpub\\wwwroot\\ClanContent\\...`

It now uses `CopyFileA`:

`Database/clan_tag_assets/<CodeIT>.bmp`
-> `Database/clan_marks/<ClanMark>.bmp`

The old code ignored the shell-copy result and still reported the tag update as successful. R11 deliberately preserves that behavior; asset-copy failure is not converted into a new gameplay error.

## Schema

No schema change in R11.

- `PRAGMA user_version = 17`
- 113 user tables

## Validation

All available validators PASS:

- mark storage
- tag storage
- migrations
- SQL coverage
- clan contract
- transactions
- source graph
- frozen build scripts
- port structure
- compiler adaptations

See `docs/SQLITE_WIP_R11_VALIDATION.md`.

## HTTP/web dependencies still intentionally unresolved

See `migration/HTTP_REMAINING_R11.tsv`.

High-priority active blockers are login/session, clan creation/default mark, leader transfer, warehouse/updateFlag, dynamic notice, NPCSound `.yul`, and SoD2 serialization. `read_ClanName` and `write_ClanImageFile` have no active non-definition caller found and are retained for compatibility rather than deleted during this migration phase.

## Next safe work

1. Prove the account/login return-code/session contract from source or supplied server-side materials, then replace login PHP with AccountDB/SQLite.
2. Search supplied packages/backups for the initial clan mark or a provable generation rule; only then migrate `makeClan` fully.
3. Reconstruct `ReadNotice` response semantics against `ServerDB__NoticeSystem` before removing the dynamic notice URL.
4. Reconstruct SoD2 response serialization (`index=1/3`) from parser + historical server material before replacing `sod2info.asp`.
5. Do not remove WinInet/ODBC linker dependencies until all active callers are proven gone.
