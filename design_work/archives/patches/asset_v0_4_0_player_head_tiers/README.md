# Summit Tale Asset Tool V0.4.0 — Player Head Promotion Tiers

This patch extends V0.3.9 player head assembly with explicit promotion-tier art selection.

## What changed

The canonical client/runtime face tables use separate head art for class progression. Across the eight playable-class tables, the first five normal progression rows follow the same filename sequence:

- base -> `PROMOTION_0`
- suffix `a` -> `PROMOTION_1`
- suffix `b` -> `PROMOTION_2`
- suffix `c` -> `PROMOTION_3`
- suffix `d` -> `PROMOTION_4`

The later `t` row is **not** promoted as a class tier. It is reported as `SPECIAL_T` until its gameplay meaning is independently proven.

The suffix is only an art-group/tier selector. Root/skeleton compatibility is still required to pass the V0.3.8 verified INX `LinkFile/link_code/file_code` binding.

Some canonical `d` heads have an INX+SMD but no physical INF even though the runtime source table names an INF. V0.4.0 permits such a tier only when:

1. its own INX LinkFile binding is verified;
2. it shares an appearance key with a sibling that has a real INF; and
3. it resolves to the same body root/motion context.

This avoids treating arbitrary INX-only character pieces as heads.

## CLI

List compatible head art with promotion metadata:

```bash
python3 pt_asset.py asset heads char/tmABCD/CmfbB02.smd
```

List every tier belonging to one appearance:

```bash
python3 pt_asset.py asset head-tiers \
  char/tmABCD/CmfbB02.smd \
  --head char/tmABCD/MfhB02.smd
```

Validate an exact appearance at a promotion tier:

```bash
python3 pt_asset.py validate animation \
  char/tmABCD/CmfbB02.smd \
  --motion 40 \
  --load-actions selected \
  --head char/tmABCD/MfhB02.smd \
  --head-tier 4
```

Accepted `--head-tier` forms:

- `auto`
- `base` / `0`
- `a` / `1`
- `b` / `2`
- `c` / `3`
- `d` / `4`
- `t` (reported as `SPECIAL_T`, not a promotion tier)

`--head auto --head-tier N` chooses a deterministic technical representative at that tier. Passing an exact head plus `--head-tier N` preserves that appearance identity and moves only within its verified tier family.

## Certification metadata

Animation certifications now preserve:

- `head_appearance_key`
- `head_tier`
- `head_tier_code`
- `head_tier_index`
- `head_tier_kind`
- descriptor kind (`INF` or the guarded `INX_ONLY_PROMOTION_TIER`)

Body and head remain independent rigs as in V0.3.9.
