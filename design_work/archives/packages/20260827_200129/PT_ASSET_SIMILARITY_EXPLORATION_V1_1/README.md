# PT Asset Similarity Exploration V1.1

Fixes the V1 final-report failure:

```text
TypeError: Object of type set is not JSON serializable
when serializing known_profiles
```

Cause:
`KNOWN_PROFILES[*]["objects"]` intentionally uses Python sets for fast
similarity/Jaccard operations, but V1 sent those sets directly to json.dumps.

V1.1:
- recursively converts sets/Path/tuples to JSON-safe structures before output;
- adds `scan_checkpoint.ndjson`;
- appends every scanned record immediately;
- on rerun, reuses checkpoint records when path and file size still match;
- ignores a partially-written final checkpoint line safely;
- keeps all scanning read-only with respect to game/design assets.

First V1.1 run after the V1 crash must rescan because V1 had no checkpoint.
Future interrupted runs can resume.

Run:

```bash
bash run_asset_similarity_survey_v1_1.sh
```
