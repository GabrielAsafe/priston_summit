# Summit Tale Asset Tool V0.5.2 — Zero-Scale Object-Local Spawn + Final Corpus Audit Semantics

V0.5.2 is based on V0.5.1 and resolves the final physical object-local parser failure found by the 432-file corpus audit.

## Final edge case: `Skill5_HyperSonic.smd`

The file is valid Model0.62 static object-local animation with one object (`Sphere01`) and 101 SCALE keys. Its first serialized scale key is exactly `(0,0,0)` and the effect grows continuously to `(5,5,5)`.

V0.5.1 normalized normal object-local scale as `current_scale / first_key_scale`. That is intentionally useful for ordinary non-zero first keys, but it is undefined for a legitimate scale-zero spawn.

The client source (`smOBJ3D::GetScaleFrame` / `TmAnimation`) interpolates the fixed-point scale keys directly and multiplies that scale into the object matrix. It does not divide by the first scale key.

V0.5.2 therefore adds a narrow, source-faithful exception:

- non-zero reference axes: unchanged `FIRST_KEY_RATIO` semantics;
- zero reference axes: `ZERO_SCALE_SPAWN_ABSOLUTE_OVER_STATIC_BIND`;
- the zero-scale object starts collapsed at scale 0 and follows the serialized source scale directly over the imported static bind;
- if both the first source scale and the static bind scale are zero and a later source key becomes non-zero, the parser still fails closed (`ZERO_SCALE_REFERENCE_AND_BIND_ZERO`).

No other object-local numeric path changes.

## Corpus audit completion semantics

Older audit versions continued to mark `object_local_complete_candidate=false` when animated children or unresolved runtime parents existed. Those were originally unsupported discovery tags, but both are now implemented and visually validated by V0.4.9+.

V0.5.2 audit V2 treats the following as supported subtypes:

- animated child local curves + inherited parent motion;
- child with serialized curves;
- deep hierarchy;
- unresolved `NodeParent` treated as runtime root, matching `LinkObject()`;
- legacy affine raw non-unit quaternion matrices.

The corpus completion gate now fails only for:

- physical scan/parser failures;
- `FULL_PARSE_FAILURE`;
- `SEGMENTED_TMFRAME`.

The generated bundle now contains only remaining unsupported representatives, rather than recopied already-supported hierarchy examples.

## Scope and safety

- Source SMDs are never modified.
- Client files are never modified.
- Runtime speed remains `PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED`.
- This patch does not enable editing/exporting object-local animation curves; it is parser/evaluator/preview/corpus-certification support.
