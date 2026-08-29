# Summit Tale — Asset Similarity Findings v12.2
## Refinement V2 and S2 transition

## Confirmed by V2

Canonical records:

```text
21865
```

Compatibility:

```text
A_REUSE_CORE_NOW        13891
B_REUSE_WITH_ADAPTER     7893
C_SEPARATE_FAMILY          63
D_NEW_REVERSE_ENGINEERING   18
```

SMD top-level:

```text
3211 / 3216 recognized
```

INX:

```text
2045 / 2046 physical model resolve
549 / 549 motion SMB resolve
```

## Coarse SMD subfamilies

The V2 produced 282 clustering bins. These are NOT 282 engineering formats.
They are combinations of:

```text
Physique mode
category hint
object count
material bucket
geometry bucket
```

Largest bins include:

```text
RIGID CHARACTER 1 object / 1 material / 300-999V       283
STATIC WEAPON 1 object / 1 material / <50V             280
RIGID CHARACTER 1 object / 2-4 materials / 300-999V    210
RIGID CHARACTER 1 object / 2-4 materials / 50-299V     117
STATIC MAP 1 object / 129+ materials / 50-299V         117
STATIC EFFECT 1 object / 5-8 materials / <50V           96
RIGID MOB 2 objects / 2-4 materials / 300-999V          75
```

This demonstrates that most diversity is content complexity, not file-format
diversity.

## Model-centric INX result

The 2045 resolved INX collapse to:

```text
1876 unique physical models
```

Motion binding:

```text
494  models have exactly one physical SMB
1382 models have no motion SMB in their own configs
0    models have multiple ambiguous SMBs
```

Breakdown of the 494 self-contained group:

```text
391 mob models
95  NPC models
7   tmABCD class/root models
1   map-stage hinted rigid model
```

Correction to v12.1:

```text
rigid mobs = 392
self-contained INX+SMB mobs = 391
```

The unpaired rigid mob is:

```text
char/monster/death_night/death_knight.smd
```

## Seven tmABCD root configs discovered automatically

```text
M1Bip.inx → model + m1.smb
M2Bip.inx → model + m2.smb
M3Bip.inx → model + m3.smb
M4Bip.inx → TmbC01.smd + m4.smb
M5Bip.inx → MfbC01.smd + m5.smb
M6Bip.inx → model + m6.smb
M7Bip.inx → model + m7.smb
```

Only semantic identities already proven should be named:

```text
M4 = Pikeman
M5 = Priestess
```

The other root numbers remain semantically unresolved until source/config
evidence identifies them.

This is the correct mechanism for eventually binding the 1382 config-only
tmABCD models to their class skeleton families.

## TGA refinement

Among 2722 TGA nonstandard/protected:

```text
2622 (96.33%)
```

share the normal TGA footer:

```text
TRUEVISION-XFILE.
```

There are 119 distinct first-16-byte signatures, but the largest 10 already
cover 81.59% of all nonstandard TGA files.

This strongly supports:

```text
few protected-TGA header families
+
standard/near-standard TGA payload/footer
```

rather than 2722 distinct formats.

## BMP refinement

The protected-prefix14 group has many prefix values, but all were initially
classified because a valid DIB begins at offset 14.

The largest prefix clusters contain hundreds of assets. The production
decoder should preserve the original 14-byte prefix per file rather than
hardcode the Pikeman prefix.

## Outliers

The five unknown-header SMD files share a conspicuous binary prefix among
`ar0nass` and the `Cartola*` group. They should be treated as one candidate
special-SMD family, not five independent investigations.

`at_dragon.smd` is still Model0.62 and differs only by:

```text
object table MBHB19
object head  MBHB18
```

It is more likely a legacy-name tolerance case than a new format.

## S2 decision

Do not generate production JSON blindly.

First create:

```text
model-centric profile candidates
+
self-contained/config-only classification
+
class-root candidates
+
exact production-profile schema snapshot
```

Then bridge candidates into the real existing profile schema.

This prevents another parallel profile format and preserves strict validators.
