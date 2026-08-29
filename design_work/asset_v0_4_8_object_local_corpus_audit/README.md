# Summit Tale Asset Tool V0.4.8 — Object-Local Physical Corpus Audit

V0.4.8 extends V0.4.7 without weakening any existing gates. The V0.4.7 visual results already prove:

- flat multi-object independent animation (`bong.smd`),
- flat two-object rotation (`3ani-04.smd`),
- animated parent + static inherited child (`At3_SplitJavelin40.smd`).

The remaining uncertainty is corpus-wide: the legacy inventory does not store `NodeParent`, per-object `nRot/nPos/nScale`, or per-object `TmFrameCnt`. Therefore absence of rarer hierarchy forms cannot be certified from the inventory alone.

## New command

```bash
python3 pt_asset.py audit object-local-corpus \
  --output pt_build/object_local_corpus_audit.json \
  --bundle PT_OBJECT_LOCAL_RARE_REPS.zip
```

The audit first uses the existing inventory to identify the known object-local candidates, then physically reads only those SMDs from the client and classifies:

- `SINGLE_OBJECT`
- `FLAT_MULTI_INDEPENDENT`
- `HIERARCHICAL`
- `STATIC_CHILD_INHERIT`
- `CHILD_WITH_SERIALIZED_CURVES`
- `ANIMATED_CHILD_LOCAL_PLUS_PARENT`
- `HOLD_CHILD_LOCAL_PLUS_PARENT`
- `SEGMENTED_TMFRAME`
- `DEEP_HIERARCHY`
- `UNRESOLVED_PARENT`
- `FULL_PARSE_FAILURE`

It validates physical resolution and uses the full V0.4.7 parser whenever possible. The shallow header scan is intentionally independent, so rare `TmFrameCnt` or child-curve cases are still reported even if the full preview parser cannot yet consume them.

## Automatic rare-representative bundle

When `--bundle` is supplied, the tool writes a small ZIP containing only representatives of subtypes that still need engineering/visual proof. The ZIP contains a manifest and preserves each client-relative path. If none of the unproven subtypes exists, the bundle contains only the manifest and the report sets:

```text
object_local_complete_candidate = true
```

That flag is only a corpus-coverage conclusion. It does not change the existing timing status; object-local previews remain `PREVIEW_NOT_RUNTIME_SPEED_CERTIFIED` until runtime timing is handled in the final roadmap stage.

## Safety

- Source SMDs are read-only.
- No client files are modified.
- No historical certifications are deleted.
- Unknown/unresolved hierarchy remains fail-closed.
- V0.4.7 preview support remains unchanged.
