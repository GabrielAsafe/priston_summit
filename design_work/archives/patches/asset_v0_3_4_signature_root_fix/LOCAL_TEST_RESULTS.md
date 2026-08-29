# Local test results — V0.3.4

- `python3 -m compileall pt_asset_tool`: PASS
- Fixture: root bone `parent_index=None`: PASS
- Fixture: root bind `tm` nested 4x4: PASS
- V0.3.3 state migration: PASS (4 structural results imported, no structural rerun)
- Re-sign existing structural PASSes: 2/2 PASS
- Different SMB bytes / same skeleton: collapsed to 1 skeleton signature
- Final status: `UNIQUE_SKELETON`
- Registry reload: `ROOT_SKELETON_BOUND`, `rigid_binding_ready=true`
