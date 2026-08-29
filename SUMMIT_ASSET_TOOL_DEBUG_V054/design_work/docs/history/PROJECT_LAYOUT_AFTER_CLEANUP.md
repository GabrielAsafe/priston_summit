# Layout esperado após a limpeza

A limpeza padrão é conservadora. Não mexe em `blender_workspace_pikeman`, `client_backups`, `logs`, `pt_build` ativos nem nos arquivos-base.

```text
design_work/
├── README.md
├── M5Bip.inx
├── m5.smb
├── MfbC01.smd
├── sitem_master_initializer_block.bin
│
├── pt_blender_to_smd_free_mesh_v2_3.py
├── pt_free_mesh_validate_v1_4.py
├── pt_rigid_skin_force_resolve_test_v1.py
├── pt_rigid_skin_normalize_copy_v1.py
│
├── summit_asset_manifest.json
├── summit_prepare_blender_workspace.py
├── summit_roundtrip_restore_v2.py
├── summit_scan_physical_assets_v2.py
├── summit_workspace_asset_bridge_v1.py
│
├── archives/
│   ├── cache/
│   ├── roundtrip_original/
│   └── integration_patch_v1/
│
├── blender/
│   └── history/
│
├── blender_workspace_pikeman/
├── client_backups/
├── docs/
│   ├── Summit_Tale_Blender_Tools_Source_of_Truth_v12_0_2026-08-26.md
│   ├── TOOLS_AND_FILES_INDEX_v1.md
│   ├── PROJECT_LAYOUT_AFTER_CLEANUP.md
│   ├── history/
│   └── reference/
│
├── history_py/
├── logs/
├── pt_build/
│   └── archive/
└── sh/
    ├── pt_character_artist_pipeline_v1_1.sh
    ├── pt_c01_artist_free_mesh_runtime_v1.sh
    ├── pt_workspace_cleanup_v1.sh
    └── history/
```

Com `--full-legacy`, diretórios antigos adicionais são movidos para um arquivo timestamped em `archives/legacy_workspace_*`.
