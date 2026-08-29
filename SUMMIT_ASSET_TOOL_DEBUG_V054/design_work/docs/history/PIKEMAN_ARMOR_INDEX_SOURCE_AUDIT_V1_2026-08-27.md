# Pikeman Armor Index Source Audit V1

## Current asset conclusions

```text
C001 coherent
C002 coherent; DEFAULT uses TmbC03.bmp + TmbC01.bmp
C003 coherent; DEFAULT uses TmbC03.bmp
C004 inconsistent/stale in current build
C005 coherent
```

For C004:

```text
TshC03
TsmC03
TslC03
```

do not exist in any of the 3216 SMD files scanned.

Do not weaken the importer and do not patch C004.

## Next question

Prove the Pikeman visual armor table entry that selects C002.

Existing source audit proves the engine rule:

```text
SetPlayerArmor()
→ ArmorJobNum[job]
→ szArmorNumFile[class-row][visual-index]
→ AddLoaderPattern(...)
```

and the equipped armor index calculation:

```text
Armor = ((dwCode & sinITEM_MASK3) >> 8)
if Armor > 25:
    Armor -= 16
```

But previous written evidence explicitly enumerated the Priestess row, not the
Pikeman row.

## Run

Point this scanner at the local Summit source tree, preferably the highest
common root that contains `SrcGame`:

```bash
python3 pt_pikeman_armor_index_source_audit_v1.py \
  --source-root "/home/gabriel/Área de Trabalho/summit" \
  --report logs/pikeman_armor_index_source_audit_v1.json
```

It is read-only and prints:
- `//PIKEMAN` row context;
- literal Cxxx.ini sequence;
- `ArmorJobNum`;
- `SetPlayerArmor`;
- inventory armor-index calculation.

Do not runtime-deploy TmbC03 until the visual index mapping is explicit.
