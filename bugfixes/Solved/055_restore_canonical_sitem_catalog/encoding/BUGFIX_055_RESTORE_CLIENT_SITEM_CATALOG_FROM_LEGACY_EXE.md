# BUGFIX 055 — Restore Client `sItem[]` Catalog from Historical `Game.exe`

## Status

**Resolved for the recovered historical catalog.**

The client item catalog was restored from a historical Summit Tale `Game.exe`.

After applying the recovered catalog:

- the client starts successfully;
- merchants open normally;
- merchant items are visible again;
- the server → network → client → `LoadItemImage()` chain is functional for the tested shop items.

The recovery intentionally preserves the historical catalog order and technical metadata.

---

# 1. Original symptom

The server-side item recovery had already restored enough item identity information for NPC shops to open.

However, the merchant windows were empty.

The observed runtime chain was:

```text
NPC config
    ↓
DefaultItems resolution
    ↓
SendShopItemList()
    ↓
OPCODE_SHOP_ITEMLIST
    ↓
client receives packet
    ↓
DecodeCompress()
    ↓
valid sITEMINFO.CODE
    ↓
sinSetShopItem()
    ↓
LoadItemImage()
    ↓
FAIL
```

The server was sending valid item lists and the client was decoding valid item CODEs, but every call to `LoadItemImage()` returned false.

---

# 2. Runtime proof of the client-side failure

`LoadItemImage()` was instrumented to inspect the global client catalog:

```text
[LOOKUP] wanted=0x01010100 MAX_ITEM=2000
nonzero entries=0
exactMatches=0
[NOT_FOUND] wanted=0x01010100
```

This behavior repeated for all tested shop items.

Therefore the immediate failure was:

```text
sItem[0..1999].CODE == 0
```

at runtime.

This explained why the shop packet could be completely correct and still produce an empty merchant UI.

---

# 3. Root cause

The client defines the global catalog as:

```cpp
sITEM sItem[MAX_ITEM] = {
#include "items.h"
};
```

The source tree no longer contains the original client-side catalog expected by this initializer.

The include resolves to `Shared/items.h`, but that file is only a small compatibility header and does not contain the historical `sITEM` initializer entries.

As a result:

```text
sItem[MAX_ITEM]
```

was zero-initialized.

`cITEM::Init()` did not populate the catalog. It only copied each already-existing static CODE into `sItemInfo.CODE`:

```cpp
for (int cnt = 0; cnt < MAX_ITEM; cnt++)
{
    sItem[cnt].sItemInfo.CODE = sItem[cnt].CODE;
}
```

Therefore the catalog had to exist before `cITEM::Init()` ran.

---

# 4. Why a minimal CODE-only client fix was rejected

The server-side recovery uses a minimal identity map containing mainly:

```text
CODE
LastCategory
```

That is sufficient for server lookup purposes.

The client requires significantly more static metadata.

The current `sITEM` structure includes fields such as:

```text
CODE
ItemNameIndex
LastCategory
w
h
ItemFilePath
Class
DorpItem
SetModelPosi
SoundIndex
WeaponClass
Flag
x
y
SetX
SetY
lpItem
ItemPosition
PotionCount
NotUseFlag
...
```

`LoadItemImage()` depends on at least:

```text
CODE
LastCategory
ItemFilePath
w
h
```

Other inventory/equipment/effect systems depend on additional fields such as:

```text
Class
DorpItem
SoundIndex
WeaponClass
SetModelPosi
```

Therefore populating only `CODE` and `LastCategory` would have converted a clear catalog failure into later metadata failures.

The correct fix required restoration of the real static item records.

---

# 5. Sources investigated

Several possible recovery sources were evaluated.

## 5.1 Historical `pristontale-1-master`

The historical Priston Tale source contains a real legacy `sITEM` initializer and was useful for proving the expected data model.

It contains entries such as:

```text
WA101
WA108
BC102
```

but does not cover a significant portion of the newer Summit catalog.

Controls such as:

```text
BC117
BI156
BI330
```

showed that it was too old to serve as the complete Summit source.

Therefore it was retained only as historical reference.

---

## 5.2 Modern fPT / rPT sources

Modern external item headers were also inspected.

They contain modern item enums/types, but they do not contain the old Summit `sITEM` initializer layout.

More importantly, some item IDs differ from the recovered Summit identities.

Example:

```text
external modern source:
BI330 = 0x080F1E00

Summit recovered identity:
BI330 = 0x080F2E00
```

Therefore these headers were explicitly rejected as canonical Summit item identity sources.

---

## 5.3 Git history

The repository history was searched for older `items.h` versions.

Historical objects of `Shared/items.h` were found, but all were small compatibility headers rather than the lost catalog.

The commit:

```text
b12f6cdbf38850a6cd00618c6704ad8b755e4037
Remove large dependency files
```

was investigated because it initially looked suspicious.

The relevant files were shown as 100% renames:

```text
SourceCode Summit Tale/Shared/items.h
    →
SourceCode/Shared/items.h

SourceCode Summit Tale/SrcGame/src/sinbaram/sinItem.cpp
    →
SourceCode/SrcGame/src/sinbaram/sinItem.cpp

SourceCode Summit Tale/SrcServer/src/sinbaram/sinItem.cpp
    →
SourceCode/SrcServer/src/sinbaram/sinItem.cpp
```

Therefore that commit did **not** remove the item catalog.

Even before the rename, `sinItem.cpp` already depended on an external `items.h` whose real catalog content was absent from Git.

Conclusion:

> The missing catalog was either never committed to this repository or was supplied externally in the original development/build environment.

---

# 6. Historical Summit `Game.exe` discovered

A historical Summit Tale client executable was located on SSD:

```text
/home/gabriel/Área de Trabalho/Summit Tale/Game.exe
```

SHA-256:

```text
44cec8bbd95b2e02f36dd9cb78486945c6809000b2386df83812096a07a9a1b3
```

Initial string inspection found:

```text
Stone Axe
Steel Axe
WA101
WA108
BC102
BI156
BI330
```

`BC117` was not present.

The presence of both old item names and newer Summit categories such as `BI156` and `BI330` made this executable a much stronger recovery source than the older public source trees.

---

# 7. First executable extraction hypothesis

The first hypothesis assumed the executable contained the current `sITEM` structure inline in a data section:

```text
DWORD CODE
char ItemNameIndex[32]
char LastCategory[16]
...
```

A direct search for:

```text
0x01010100
"Stone Axe"
"WA101"
```

at the expected relative offsets found no structured records.

This hypothesis was rejected.

The executable did contain the strings, but not in the current inline struct layout.

---

# 8. String pool discovery

String offsets showed an ordered item string pool:

```text
WA101       0x003AF860
Stone Axe   0x003AF870
WA102       0x003AF87C
Steel Axe   0x003AF884
WA103       0x003AF890
Battle Axe  0x003AF898
WA104       0x003AF8A4
War Axe     0x003AF8AC
WA108       0x003AF8F8

BI156       0x003AF818
BI330       0x003AE5BC
```

The CODE values were not located near these strings.

PE analysis established:

```text
machine    = 0x014C
PE type    = PE32
image base = 0x00870000
pointer    = 4 bytes
```

The strings were referenced from `.text`, not from a static `.data` table.

This suggested that the executable constructs the catalog during static initialization.

---

# 9. Constructor pattern discovered

Disassembly around `WA101` revealed a repeated constructor call pattern.

For `WA101`:

```asm
push 1
push 1
push 4
push 0x00C20860
push 4
push 0x00C20868
push 0x42
push 0x16
push 0x00C20860
push 0x00C20870
push 0x01010100
mov  ecx,0x05492840
call 0x00A5A2E0
```

Resolving the pointers gives:

```text
0x00C20860 -> "WA101"
0x00C20868 -> "Weapon"
0x00C20870 -> "Stone Axe"
```

The current source constructor is:

```cpp
sITEM(
    DWORD CODE,
    const char* ItemNameIndex,
    const char* LastCategory,
    int w,
    int h,
    const char* ItemFilePath,
    int Class,
    const char* DorpItem,
    DWORD SetModelPosi,
    int SoundIndex,
    int WeaponClass = 0
)
```

Because x86 arguments are pushed right-to-left, the recovered `WA101` call maps exactly to:

```text
CODE          = 0x01010100
ItemNameIndex = "Stone Axe"
LastCategory  = "WA101"
w             = 22
h             = 66
ItemFilePath  = "Weapon"
Class         = 4
DorpItem      = "WA101"
SetModelPosi  = 4
SoundIndex    = 1
WeaponClass   = 1
```

This exactly matches the semantics of the current `sITEM` constructor.

---

# 10. `sizeof(sITEM)` / object stride recovered

The destination object addresses are consecutive:

```text
WA101 this = 0x05492840
WA102 this = 0x05492B60
WA103 this = 0x05492E80
WA104 this = 0x054931A0
...
```

Difference:

```text
0x320 bytes
```

for every item.

Therefore the historical executable uses:

```text
sizeof(sITEM) = 0x320
```

for this catalog.

No unaligned object addresses were found.

---

# 11. Full constructor extraction

The executable's static item initialization region was disassembled and parsed automatically.

The extractor identified calls to:

```text
constructor = 0x00A5A2E0
```

and decoded the preceding eleven immediate arguments plus the `ECX` object address.

Recovery result:

```text
constructor calls : 1819
bad calls         : 0
base              : 0x05492840
stride            : 0x320
index range       : 0 .. 1818
unaligned objects : 0
```

The resulting intermediate data was saved as:

```text
/tmp/game_exe_item_catalog.tsv
```

SHA-256:

```text
9bff3a5fc56d9aea7d0ba16c1e3b68dcf54e771dbda2e7689c9837a260ed7ed8
```

---

# 12. Why the catalog has 1819 explicit records instead of 2000

The disassembly after the last explicitly constructed item shows:

```text
ESI = address immediately after item 1818
EDI = 0xB5
loop:
    call default constructor
    ESI += 0x320
    EDI--
```

`0xB5` decimal is:

```text
181
```

Therefore:

```text
1819 explicit item constructors
+
181 default constructors
=
2000 total sITEM slots
```

which exactly matches:

```text
MAX_ITEM = 2000
```

This is strong structural confirmation that the correct global item array was recovered.

---

# 13. Important modern identity controls

The historical Summit executable confirmed modern identities that were already recovered independently on the server.

Examples:

```text
BI156
CODE = 0x080B4800

BI330
CODE = 0x080F2E00
```

This was especially important because external modern sources disagreed on `BI330`.

The executable therefore independently validated the Summit-specific identity map.

---

# 14. Comparison with the current Summit server map

The 1819 executable records were compared against:

```text
SrcServer/src/sinbaram/item_code_map_mingw.inc
```

Current Summit map:

```text
1723 entries
```

Comparison:

```text
exact LastCategory + CODE : 1535
same category / CODE diff : 0
missing category in EXE   : 188
EXE records outside map   : 281
```

The critical result is:

```text
same category / CODE diff = 0
```

Whenever both sources know the same `LastCategory`, they agree on its CODE.

That makes the historical executable a high-confidence source for the static client metadata of those shared items.

---

# 15. Catalog anomalies preserved intentionally

The historical executable contains several duplicates.

Duplicate `LastCategory` examples:

```text
WM132
SK277
SK245
SK252
DS154
BI273
```

Duplicate nonzero CODE examples:

```text
0x01043000
0x01084C00
0x081A0600
0x02044600
0x080C5900
```

These entries were **not deduplicated**.

Reason:

1. they exist in the historical executable;
2. the original order is known;
3. some client lookups return the first sequential match;
4. removing or reordering duplicates would introduce behavior not supported by evidence.

The recovery therefore preserves all 1819 explicit records in their exact original order.

---

# 16. Shop coverage audit

All CODEs observed in the current merchant runtime test are present in the recovered executable catalog.

Coverage:

```text
42 / 42
```

Examples:

```text
0x01010100 -> WA101 [Weapon] 22x66
0x01010200 -> WA102 [Weapon] 44x66
0x01010300 -> WA103 [Weapon] 44x88

0x02020100 -> DB101 [Defense] 44x44
0x02030100 -> DG101 [Defense] 44x44
0x02040100 -> DS101 [Defense] 44x44

0x04020100 -> PL101 [Potion] 22x22
0x04030100 -> PS101 [Potion] 22x22
0x04010100 -> PM101 [Potion] 22x22

0x06010100 -> EC101 [Potion] 44x22

0x03060600 -> FO106 [Accessory] 22x22
0x03062A00 -> FO126 [Accessory] 22x22
```

This provided a deterministic expectation that the existing merchant regression test should recover once `sItem[]` was restored.

---

# 17. `ItemNameIndex` decision

Current source usage of:

```text
ItemNameIndex
```

was audited.

It appears only in:

```text
sITEM definition
default initialization
sITEM constructor
```

No active client system reads it.

Some historical executable item names contain legacy Korean text represented as mojibake when decoded through Western encodings.

Example from the recovered executable:

```text
BC102 ItemNameIndex = legacy non-ASCII text
```

To avoid importing unnecessary legacy-encoding strings into source, the generated catalog intentionally stores:

```cpp
ItemNameIndex = ""
```

for all recovered entries.

This does not remove technical metadata used by current item loading.

Only user-facing/historical item names are omitted.

Technical fields:

```text
LastCategory
ItemFilePath
DorpItem
```

were audited and all are ASCII.

---

# 18. Generated client catalog

A dedicated generated include was created:

```text
SrcGame/src/sinbaram/item_catalog_recovered.inc
```

Properties:

```text
records : 1819
encoding: ASCII
```

SHA-256:

```text
7d9ddbe6379fa4650cc7d23f0b060f99fd34cf2025dcc520f02bc34e0cdaa7ba
```

Each line uses the existing `sITEM` constructor.

Example:

```cpp
sITEM(
    0x01010100u,
    "",
    "WA101",
    22,
    66,
    "Weapon",
    0x00000004,
    "WA101",
    0x00000004u,
    1,
    1
),
```

`BC102` demonstrates why literal executable metadata was preserved instead of inferred:

```cpp
sITEM(
    0x080A0200u,
    "",
    "BC102",
    22,
    22,
    "Event",
    0x00000001,
    "BC101",
    0x00000000u,
    14,
    0
),
```

Notice:

```text
LastCategory = BC102
DorpItem     = BC101
```

An inference such as `DorpItem == LastCategory` would have been incorrect.

---

# 19. Final source patch

The original initializer was:

```cpp
sITEM sItem[MAX_ITEM] = {
#include "items.h"
};
```

It was replaced with:

```cpp
sITEM sItem[MAX_ITEM] = {
#include "item_catalog_recovered.inc"
};
```

No shop, packet, parser or rendering logic was changed.

Final behavioral fix is therefore limited to restoring the missing static catalog.

---

# 20. Line-ending safety correction

The first automated edit used Python text mode and unintentionally normalized line endings in the large legacy `sinItem.cpp`.

That produced a misleading full-file Git diff.

The file was restored from the exact pre-patch backup.

Original SHA-256:

```text
f08b20afb1d3b0238d25744e4743bddc48ae212b24aaf5071c1055f7ecd9e2a3
```

The include replacement was then reapplied as a byte-level substitution.

Validation:

```text
exact expected patch : True
before bytes         : 234866
after bytes          : 234885
expected byte delta  : 19
actual byte delta    : 19
```

Final textual diff:

```diff
 sITEM sItem[MAX_ITEM] = {
-#include "items.h"
+#include "item_catalog_recovered.inc"
 };
```

This guarantees no unrelated source normalization was introduced by this fix.

---

# 21. Runtime validation after restoration

Before the fix:

```text
[LOOKUP] wanted=0x01010100 MAX_ITEM=2000
nonzero entries=0
exactMatches=0
[NOT_FOUND] wanted=0x01010100
```

Shop decoding was already successful, but:

```text
[SHOPLOAD] ... result=0
```

for every tested item.

After restoring the catalog:

```text
merchant windows contain items
```

The previously empty merchant inventory is now populated.

This closes the tested path:

```text
server NPC config
        ↓
server item identity
        ↓
DefaultItems
        ↓
SendShopItemList
        ↓
network
        ↓
OPCODE_SHOP_ITEMLIST
        ↓
DecodeCompress
        ↓
sITEMINFO.CODE
        ↓
LoadItemImage
        ↓
recovered sItem[]
        ↓
cShop.ShopItem[]
        ↓
merchant UI
```

---

# 22. What was NOT changed

This fix deliberately did not modify:

```text
SendShopItemList()
OPCODE_SHOP_ITEMLIST
sinRecvMessage()
DecodeCompress()
sinSetShopItem()
LoadItemImage()
server merchant parsing
server item linking
```

Those stages had already been proven functional.

This minimizes the risk of introducing unrelated behavior changes.

---

# 23. Remaining catalog gap

The recovered historical executable does not contain every current Summit identity.

Against the 1723-entry current server identity map:

```text
188 current categories are absent from the historical EXE
```

Examples include:

```text
BC117
BC118
BC133
BI160
BI161
BI185
BI199
BI200
BI238
BI239
BI254
BI299
BI301...
CA201...
DA130...
...
```

This is expected because the executable is an older Summit build.

Therefore BUGFIX 055 restores the historical client catalog and fixes the merchant regression, but it does **not** claim full metadata coverage for every newer current item.

The 188 missing current identities should be handled separately using evidence from later clients/resources/backups.

They must not be populated by guessing.

---

# 24. Architectural conclusion

The original problem was not a merchant bug.

It was a missing generated/external static client dependency:

```text
items.h catalog
```

The source still contained the consumer:

```cpp
sITEM sItem[MAX_ITEM] = {
#include "items.h"
};
```

but the real initializer data was absent.

The historical `Game.exe` preserved the missing information because C++ static constructors encoded every item record into executable machine code.

The binary effectively served as a serialized source of the lost static dataset.

The recovery process was:

```text
historical Game.exe
        ↓
PE/string analysis
        ↓
find .text references
        ↓
identify sITEM constructor calls
        ↓
recover 11 constructor arguments
        ↓
identify object stride 0x320
        ↓
recover indices 0..1818
        ↓
validate against current Summit CODE map
        ↓
preserve exact ordering/duplicates
        ↓
generate ASCII client include
        ↓
restore sItem[] static initialization
        ↓
merchant inventory functional
```

This approach was preferred over manual reconstruction because it recovers observed Summit behavior rather than inferred metadata.

---

# 25. Reproducibility / provenance

Historical executable:

```text
/home/gabriel/Área de Trabalho/Summit Tale/Game.exe
SHA256:
44cec8bbd95b2e02f36dd9cb78486945c6809000b2386df83812096a07a9a1b3
```

Intermediate extracted TSV:

```text
/tmp/game_exe_item_catalog.tsv
SHA256:
9bff3a5fc56d9aea7d0ba16c1e3b68dcf54e771dbda2e7689c9837a260ed7ed8
```

Generated catalog:

```text
SrcGame/src/sinbaram/item_catalog_recovered.inc
SHA256:
7d9ddbe6379fa4650cc7d23f0b060f99fd34cf2025dcc520f02bc34e0cdaa7ba
```

Recovered executable facts:

```text
PE32
ImageBase = 0x00870000
sITEM constructor = 0x00A5A2E0
first object = 0x05492840
stride = 0x320
explicit records = 1819
default remaining records = 181
MAX_ITEM = 2000
```

These values make the recovery reproducible and independently auditable.

---

# 26. Recommended commit scope

Commit only the functional catalog restoration and its documentation.

Expected functional files:

```text
SourceCode/SrcGame/src/sinbaram/sinItem.cpp
SourceCode/SrcGame/src/sinbaram/item_catalog_recovered.inc
```

Recommended project documentation path:

```text
SourceCode/bugfixes/055_restore_canonical_sitem_catalog.md
```

Do not include temporary instrumentation backups or generated debug logs in the functional commit unless intentionally desired.

---

# 27. Recommended commit message

```text
fix(items): restore client sItem catalog from legacy executable
```

Suggested extended commit body:

```text
Recover 1819 historical sITEM records from a known Summit Tale Game.exe
by decoding its static constructor calls.

The existing client initializer resolved items.h to a compatibility header
without catalog entries, leaving all 2000 sItem CODE values zero at runtime.

The recovered catalog preserves CODE, LastCategory, dimensions, item path,
class, drop identifier, model position, sound index, weapon class, ordering,
and historical duplicates.

1535 entries match the current Summit server CODE map exactly by category and
CODE with zero CODE disagreements. All 42 merchant items used in the runtime
regression test are covered.

ItemNameIndex is intentionally omitted because it is unused by current client
code and historical values include legacy non-ASCII text.

Merchant item lists are visible again after the restoration.
```

---

# 28. Final state

Before:

```text
server shop pipeline      OK
client packet decode      OK
client sItem[]            EMPTY
LoadItemImage             FAIL
merchant items            EMPTY
```

After:

```text
server shop pipeline      OK
client packet decode      OK
client sItem[]            RESTORED (1819 historical entries)
LoadItemImage             FUNCTIONAL for recovered items
merchant items            VISIBLE
```

The original merchant/item-chain regression is therefore closed for the recovered catalog.

The remaining 188 newer current identities are a separate catalog-extension task and should remain explicitly tracked rather than inferred.
