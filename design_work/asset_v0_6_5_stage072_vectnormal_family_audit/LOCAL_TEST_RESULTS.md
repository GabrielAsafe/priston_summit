# V0.6.5 local test results

Tested against all 63 physical `SMD Stage data Ver 0.72` files retained from the prior physical corpus.

- files audited: 63
- total faces: 3,066,543
- current byte-exact normal family: 46 files / 2,207,322 faces
- legacy family: 17 files / 859,221 faces
- current-formula mismatch faces outside current family: 19,211
- reciprocal-order legacy formula closes 7 legacy files exactly
- remaining legacy files: 10
- reciprocal-order mismatches in legacy family: 5,635
- maximum component difference in unresolved legacy files: 1 short LSB
- degenerate zero-cross faces: 17,716
- degenerate faces with stored `(0,0,0)`: 17,716
- all current-family files with `VectNormal[3] == 0xCDCD` are exact under the current float32 model
- compileall: PASS
- CLI help: PASS
- original client files modified: no

Geometry writing remains disabled in this release because 10 legacy files do not yet have a byte-exact `SetNormal` evaluation model.
