# V0.4.0 local test results

## Runtime-source table audit

The supplied client source `playmodel.h` defines eight playable class face tables. Their first five normal rows consistently use:

`base -> a -> b -> c -> d`

for the same face appearance number. `t` occurs later and is deliberately classified as `SPECIAL_T`, not as a promotion tier.

## Atalanta B01/B02/B03 structural tier matrix

All fifteen normal promotion variants were tested against the verified Atalanta root motion `m2.smb` with `pt_character_structural_gate_v2.py`:

| Appearance | Promotion | SMD | Result |
|---|---:|---|---|
| B01 | 0 | MfhB01.smd | PASS |
| B01 | 1 | MfhB01a.smd | PASS |
| B01 | 2 | Mfh_B01b.smd | PASS |
| B01 | 3 | MfhB01c.smd | PASS |
| B01 | 4 | Mfh-B01d.smd | PASS |
| B02 | 0 | MfhB02.smd | PASS |
| B02 | 1 | MfhB02a.smd | PASS |
| B02 | 2 | Mfh_B02b.smd | PASS |
| B02 | 3 | MfhB02c.smd | PASS |
| B02 | 4 | Mfh-B02d.smd | PASS |
| B03 | 0 | MfhB03.smd | PASS |
| B03 | 1 | MfhB03a.smd | PASS |
| B03 | 2 | Mfh_B03b.smd | PASS |
| B03 | 3 | MfhB03c.smd | PASS |
| B03 | 4 | Mfh-B03d.smd | PASS |

Every result had `missing_bind_bones=[]` and `skeleton_bones=67`.

Representative vertex growth demonstrates that promotion art is materially different geometry rather than a metadata alias:

- B01 promotion 0: 335 vertices
- B01 promotion 1: 455 vertices
- B01 promotion 2: 766 vertices
- B01 promotion 3: 964 vertices
- B01 promotion 4: 1609 vertices

## Resolver tests

For appearance `MfhB02`:

- `auto` -> `MfhB02.smd` / `PROMOTION_0`
- tier `1` -> `MfhB02a.smd`
- tier `2` -> `Mfh_B02b.smd`
- tier `3` -> `MfhB02c.smd`
- tier `4` -> `Mfh-B02d.smd`
- tier `t` -> `MfhB02t.smd` / `SPECIAL_T`

All selected normal-tier candidates independently resolve through verified INX LinkFile to `M2Bip.inx -> m2.smb`.

The physical canonical client lacks an INF sibling for the tested `d` variants. They are admitted only via `INX_ONLY_PROMOTION_TIER` after a real-INF sibling establishes the same appearance family and LinkFile independently proves the same root.

## Regression

- `python3 -m compileall` PASS
- CLI `asset heads` PASS
- CLI `asset head-tiers` PASS
- exact appearance + tier selection PASS
- invalid tier input fails closed
- existing V0.3.9 body+independent-head behavior preserved
