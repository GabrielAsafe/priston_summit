# SQLite WIP R11 — validation report

Date: 2026-09-06

## Scope

R11 is based on R10 and changes only the clan-mark/local-asset storage path. It does not change the SQLite schema and does not promote the WIP to the functional baseline.

## Functional changes in R11

1. `ISAOCHECK::read_ClanImage` and `read_ClanImage2` no longer download `ClanContent/<ClanMark>.bmp` over HTTP.
2. Clan marks are read from `Database/clan_marks/<ClanMark>.bmp` with bounded Win32 file I/O.
3. The local reader rejects path separators/colon/dot in the basename, limits files to 64 KiB, and requires the `BM` signature.
4. Clan tag customization no longer runs a shell `copy` against IIS directories. It uses `CopyFileA` from `Database/clan_tag_assets/<CodeIT>.bmp` to `Database/clan_marks/<ClanMark>.bmp`.
5. The legacy tag-change flow ignored the result of `copy`; R11 deliberately preserves that observable behavior by not turning a missing tag asset into a new gameplay failure.
6. No default clan mark is synthesized. Clan creation remains blocked until the initial/default mark asset contract is proven.

## Validation results

- clan mark local storage: PASS
- clan tag local storage / IIS removal: PASS
- SQLite migrations: PASS — `user_version=17`, 113 user tables
- SQLite query coverage: PASS — missing=0, forbidden external/positional=0
- clan SQLite contract: PASS
- SQLite transaction API: PASS
- source graph: PASS — only authorized new server TU is `SQLiteConnection.cpp`
- build/link scripts frozen: PASS
- port structure: PASS — 1517/1517
- compiler adaptations: PASS — one authorized MinGW adaptation
- R10 -> R11 patch apply/hash verification: PASS

Raw command output is retained in `docs/R11_VALIDATION_RAW.txt`.

## Remaining limitation

This environment still does not provide the full Windows/MinGW dependency setup used by the project, so R11 has not been promoted through the user's compile/link/runtime acceptance gate. R6.1 remains the functional oracle.
