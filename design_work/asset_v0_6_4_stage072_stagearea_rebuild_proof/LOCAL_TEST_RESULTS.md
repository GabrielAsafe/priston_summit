# V0.6.4 local test results

Corpus físico utilizado: os 63 `SMD Stage data Ver 0.72` do bundle V0.5.8 enviado pelo usuário (quando todos eram representantes por parse failure).

## Reconstrução StageArea

- arquivos: 63/63 PASS
- cell -> ordered face list: 63/63 exact
- StageArea pointer table: 63/63 exact
- wAreaSize physical bug-compatible formula: 63/63 exact
- StageMapRect min/max X/Z: 63/63 exact
- serialized StageArea core bytes: 63/63 exact
- encodings: DWORD32=59, DWORD32_TRAILING_OPAQUE=3, WORD16=1
- capacity model: DWORD pointer-counter step4=62, legacy WORD16 step2=1
- max triangle bounding-box span: 2254 cells
- SourceCode `AreaList[4096]` overflow candidates: 0
- opaque suffixes preserved outside the reconstructed core: 3

## Negative/local safety tests

The reconstructor refuses:

- unresolved WORD16/DWORD32 encoding;
- active StageArea with null serialized area-buffer base;
- invalid face vertex indices;
- WORD16 face indices above 65535;
- any physical mismatch in pointer table / wAreaSize / StageMapRect / core payload marks the file FAIL.

No client file was modified.
