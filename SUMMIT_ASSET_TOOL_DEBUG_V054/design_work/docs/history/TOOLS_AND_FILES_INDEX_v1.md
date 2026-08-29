# Summit Tale Blender Tools — Índice de Ferramentas

## Ativas

### `pt_rigid_skin_normalize_copy_v1.py`
Cria uma cópia game-normalized sem sobrescrever o master artístico.

Responsabilidades:
- sincronizar Edit→Object;
- detectar rigid/mixed weights;
- preservar rigid vertices;
- resolver somente ambiguidades defensáveis;
- marcar restantes em `PT_REVIEW_REQUIRED`;
- salvar outra `.blend`;
- gerar JSON de relatório.

Estado: **PROVADO NO PIPELINE**.

---

### `pt_rigid_skin_force_resolve_test_v1.py`
Resolver de engenharia para ambiguidades restantes.

Política:
- somente bones já candidatos naquele vertex;
- seed rígido mais próximo;
- exatamente um bone weight 1.0.

Estado: **TEST-ONLY / PROVADO NO GATE**.

Nunca tratar como política de produção silenciosa.

---

### `pt_free_mesh_validate_v1_4.py`
Validator do MshC01 free mesh.

Principais gates:
- Edit Mode sync;
- UV;
- TEMP BMesh BEAUTY triangulation;
- degenerates;
- materials;
- rigid skin;
- bone validity;
- fixed-point fit;
- limites conservadores.

Estado: **PASS no asset 656/1129/1292**.

---

### `pt_blender_to_smd_free_mesh_v2_3.py`
Writer SMD Model 0.62 do MshC01.

Reconstrói:
- smVERTEX;
- smFACE;
- smTEXLINK;
- Physique.

Mantém os outros 14 object blocks byte-identical no caminho atual.

Estado: **RUNTIME PROVEN**.

Build principal:
```text
382625 bytes
1695df612bf3b13f5a9a0a27b14c41dfbbb55a39704d9fa3739c6be2013ea030
```

---

### `history_py/pt_smd_blender_import_v3_static_armature.py`
Importer independente usado como gate de roundtrip.

Valida:
- objects;
- materials;
- UV;
- Physique;
- skeleton;
- armature modifiers;
- INX LOD visibility.

Estado: **PROVADO ESTRUTURALMENTE**.

---

### `sh/pt_character_artist_pipeline_v1_1.sh`
Wrapper operacional.

Stages:
1. Normalize;
2. optional force-test;
3. Validate;
4. Export;
5. independent reimport;
6. optional publish ao workspace.

Padrão de produção:
```text
ambiguous → REVIEW_REQUIRED → STOP
```

Estado: **pipeline_check=PASS**.

---

### `summit_prepare_blender_workspace.py`
Preparador do workspace do asset registry.

Origem: pacote ROUNDTRIPAUTOMATICA.

Cria:
```text
workspace_manifest.json
```

O manifesto é a autoridade de roundtrip.

---

### `summit_asset_manifest.json`
Catálogo machine-readable source-derived usado pelo preparador.

Observação:
o catálogo inicial não mapeava automaticamente todos os assets físicos SMD/SMB. Isso é complementado pelo Bridge.

---

### `summit_workspace_asset_bridge_v1.py`
Integra build pipeline e asset registry.

`register`:
- registra asset físico ausente do catálogo;
- copia pristine para o workspace;
- grava SHA/tamanho/origem.

`publish`:
- exige `pipeline_check=PASS`;
- valida SMD size/SHA;
- publica no slot do workspace;
- não altera cliente.

Estado: **PROVADO**.

Registro C01:
```text
AST-791F1CAE2A4C
→ char/tmABCD/MfbC01.smd
```

---

### `summit_roundtrip_restore_v2.py`
Deploy endurecido.

Características:
- dry-run default;
- filtro por asset/target;
- client drift detection;
- backup antes da primeira escrita;
- verificação dos backups;
- replace atômico;
- verificação pós-write;
- rollback;
- transaction.json;
- ALREADY_APPLIED.

Estado: **PROVADO**.

---

### `summit_scan_physical_assets_v2.py`
Scanner físico corrigido.

Corrige incompatibilidade de colunas entre assets conhecidos e fallback que podia quebrar CSV.

Usar para inventariar a árvore real do cliente.

---

### `sh/pt_c01_artist_free_mesh_runtime_v1.sh`
Instalador/recovery específico do build C01 de engenharia.

Mantido principalmente por:
- restore-original;
- status;
- recuperação independente do registry.

Build aprovado:
```text
382625 / 1695df...
```

---

## Históricas / probes

Mover para `history_py/`:

```text
pt_artist_triangulation_probe_v1.py
pt_blender_uv_storage_probe_v1.py
pt_blender_uv_storage_probe_v2.py
pt_rigid_skin_weight_probe_v1.py
pt_blender_to_smd_high_remodel_v2.py
```

Esses scripts foram essenciais para descobrir:
- UV live no BMesh em Edit Mode;
- degenerates de `calc_loop_triangles`;
- BMesh triangulation válida;
- distribuição dos 257 mixed vertices;
- fases anteriores de remodel.

Não devem ser usados como core atual.

---

## Builds / artefatos históricos

Mover builds de teste antigos da raiz para `pt_build/archive/`:

```text
MfbC01_clean_noop.smd
MfbC01_testv2.smd
MfbC01_testv2_lods.smd
```

Mover `.blend1` da raiz para:

```text
blender/history/
```

---

## Pacotes históricos

Após promover os scripts ativos:

```text
ROUNDTRIPAUTOMATICA/
ROUNDTRIP_INTEGRATION_PATCH_V1/
ROUNDTRIP_INTEGRATION_PATCH_V1.zip
```

podem ser arquivados em:

```text
archives/
```

O cleanup V1 faz isso sem apagar conteúdo.

---

## Arquivos que devem permanecer claramente identificáveis

Base:

```text
MfbC01.smd
m5.smb
M5Bip.inx
sitem_master_initializer_block.bin
```

Ativos:

```text
pt_rigid_skin_normalize_copy_v1.py
pt_rigid_skin_force_resolve_test_v1.py
pt_free_mesh_validate_v1_4.py
pt_blender_to_smd_free_mesh_v2_3.py
summit_prepare_blender_workspace.py
summit_asset_manifest.json
summit_scan_physical_assets_v2.py
summit_workspace_asset_bridge_v1.py
summit_roundtrip_restore_v2.py
```

Diretórios ativos:

```text
blender/
blender_workspace_pikeman/
client_backups/
docs/
history_py/
logs/
pt_build/
sh/
```
