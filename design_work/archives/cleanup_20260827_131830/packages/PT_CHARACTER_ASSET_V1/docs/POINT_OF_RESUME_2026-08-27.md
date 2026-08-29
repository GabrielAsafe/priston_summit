# 38. PONTO DE RETOMADA — use se a conversa for interrompida

## Objetivo global

Criar ferramenta Blender de produção para editar assets existentes do Summit Tale / Priston Tale, incluindo personagens, texturas, animações e mapas, mantendo round-trip seguro para o cliente.

## Workspace esperado

```text
~/Área de Trabalho/summit/design_work
```

Cliente:

```text
/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente
```

Blender Flatpak:

```bash
flatpak run --filesystem=home org.blender.Blender
```

## Baseline C01

```text
MfbC01.smd
size   290229
sha256 9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b

m5.smb
M5Bip.inx
```

Runtime target:

```text
char/tmABCD/MfbC01.smd
```

Registry:

```text
AST-791F1CAE2A4C
```

## Estado provado

```text
Direct SMD Model 0.62                         RUNTIME PROVEN
artist free topology                          RUNTIME PROVEN
arbitrary vertex/face count                   RUNTIME PROVEN
full vertex reindex                           RUNTIME PROVEN
full face reorder                             RUNTIME PROVEN
all smVERTEX rebuilt                          RUNTIME PROVEN
all smFACE rebuilt                            RUNTIME PROVEN
all smTEXLINK rebuilt                         RUNTIME PROVEN
all Physique rebuilt                          RUNTIME PROVEN
rigid skin game normalization                 PROVEN
BMesh non-destructive triangulation           PROVEN
generic object writer 15/15 capability        PROVEN
MbhC01 generic serialization                  RUNTIME PROVEN
HIGH 3-object atomic serialization            RUNTIME PROVEN
C01 9-object HIGH/DEFAULT/LOW serialization   RUNTIME PROVEN
independent reimport high/default/low          PROVEN
registry publish                              PROVEN
managed iterative deploy                      PROVEN
backup/drift detection/atomic replace         PROVEN
```

## Último runtime build aprovado

```text
pt_build/characters/pikeman_c01_m5/all_lods/
MfbC01_all_lods_artist_gate_v1.smd

size   382625
sha256 9b4a992945b7d0ce30aaff63b1141433132ecf57e17a4036177c9d94d8fa5b9c
```

`MshC01` nesse build:

```text
656 vertices
1292 faces/TexLinks
```

## Skinning

Runtime atual estudado:

```text
1 bone por vertex
```

Workflow de produção:

```text
Artist Master
→ Game-Normalized Copy
→ safe automatic rigidization
→ PT_REVIEW_REQUIRED se ambíguo
→ artista corrige
→ validate/export
```

`--force-test` é somente engenharia e pode causar deformação visual.

## Ferramentas core atuais

```text
pt_rigid_skin_normalize_copy_v2.py
pt_rigid_skin_force_resolve_test_v2.py
pt_free_mesh_validate_v2.py
pt_free_mesh_validate_multi_v1.py
pt_blender_to_smd_free_mesh_v3.py
pt_blender_to_smd_free_mesh_v4_multi.py
pt_character_multi_object_lod_pipeline_v2.py
pt_smd_object_capability_audit_v1.py

summit_workspace_asset_bridge_v1_1.py
summit_roundtrip_restore_v3.py
summit_scan_physical_assets_v2.py
summit_prepare_blender_workspace.py
```

Independent gate:

```text
history_py/pt_smd_blender_import_v3_static_armature.py
```

## Profile atual

```text
profiles/pt_character_profile_pikeman_c01_m5_v1.json
```

Em migração para:

```text
profiles/pt_character_profile_pikeman_c01_m5_v2.json
```

com Character Asset components.

## Workspace roundtrip

```text
blender_workspace_pikeman/
workspace_manifest.json
```

## Regra de deploy

Sempre:

```text
pipeline PASS
→ publish
→ restore dry-run
→ apply
```

Restore V3 aceita estado anterior somente se:

```text
REGISTERED_SOURCE
ou
PREVIOUSLY_PUBLISHED_BUILD
```

Qualquer outro SHA:

```text
CLIENT_DRIFT → REFUSE
```

## Próximo ponto exato de trabalho

Implementar/validar:

```text
PT Character Asset V1
```

Objetivo:

```text
profile selecionado
→ resolve SMD + SMB + INX + texturas
→ gera Character Asset manifest
→ sincroniza componentes no workspace registry
→ build all-LODs sem nomes internos
→ publish/deploy pelo target do profile
```

Depois avançar para:

```text
Material + Texture workflow integrado
```

Não voltar a provar free topology/geometry C01: essa etapa está encerrada.
