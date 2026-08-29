# V0.4.3 Local Test Results

- `python -m compileall pt_asset_tool`: PASS.
- Synthetic Model0.62 with 2 serialized materials, only material 0 referenced by a face:
  - used texture exists; unused historical texture missing -> `FACE_REFERENCED_MATERIALS`, `ready=true`, `required_failures=0`, `historical_unused_failures=1`: PASS.
  - face-used texture missing -> `ready=false`, `required_failures=1`: PASS (fail closed).
  - material decoder unavailable -> `ALL_SERIALIZED_FALLBACK`; historical missing texture becomes required and `ready=false`: PASS (fail closed).
- Required texture names derived from the source material table are added to the overlay even if an older inventory omitted them.
- V0.4.2 strict XYZ-only static witness writer and independent non-XYZ byte comparator are retained unchanged.
- Client/source modification: none.
