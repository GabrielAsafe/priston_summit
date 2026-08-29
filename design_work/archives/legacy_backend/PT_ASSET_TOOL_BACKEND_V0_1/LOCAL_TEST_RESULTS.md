# Local test results — PT Asset Tool Backend V0.1

Tested against the exact active-code snapshot supplied on 2026-08-28.

- ZIP/source snapshot integrity: PASS
- Python compile (`pt_asset_tool/*.py`, wrapper): PASS
- Bash syntax (installer/smoke runner): PASS
- Canonical inventory load: PASS — 21,865 client records
- Registry build: PASS — 18,537 first-class SMD/BMP/TGA assets
- Visual-compatible candidates currently exposed: 1,689
  - 494 self-contained rigid models
  - 1,195 static Model0.62 models
- Multi-node INX LOD CSV split: PASS (`ArmBeetle,BeetleJaw` -> 2 nodes)
- Case-only client file handling: PASS; exact-case path wins, ambiguous fallback blocks
- Physical-chain resolution on supplied representative client subset: PASS
- Monbagon rigid structural validation: PASS
  - SMD + SMB structural gate
  - INX dual-layout motion core
- itPR101 static structural/capability validation: PASS
- Certification JSONL append/latest query: PASS
- Visual orchestration with simulated Blender: PASS (rigid + static)
  - prepare import
  - create witness
  - launch/close GUI simulation
  - y/n/skip capture
  - persistent certification

Not locally executable in this environment:

- actual Blender/Flatpak rendering and interactive inspection;
- final game runtime visual validation.

The V0.1 backend has no publish command and does not modify client files.
