# Summit Tale — Blender Tools
## Source of Truth Técnica v13.0
### Artist Free Mesh, Game Normalization e Asset Round-Trip
### Estado consolidado em 2026-08-26

Este documento substitui o ponto de retomada da v11.6. Os testes descritos abaixo avançaram além de “free topology teórica”: o fluxo artístico livre foi provado no Blender, reimportado independentemente, executado no cliente e integrado ao asset registry/round-trip.

## Convenção de evidência

- **PROVADO EM RUNTIME** — executado dentro do cliente.
- **PROVADO ESTRUTURALMENTE** — parser/serializer/reimport/manifest fechou.
- **PROVADO POR TESTE DE FERRAMENTA** — comportamento reproduzido por scripts/probes.
- **AUDITORIA DA SOURCE** — afirmação da auditoria fornecida pelo usuário; precisa de revalidação independente contra a árvore-fonte antes de ser elevada.
- **PENDENTE** — não fechado.

---

# 1. Objetivo

Construir uma ferramenta Blender para editar assets existentes do Summit Tale / Priston Tale e produzir arquivos utilizáveis pelo jogo:

```text
Game Asset
→ import
→ artist master
→ editing/remodeling
→ game normalization
→ validation
→ serialization
→ independent roundtrip
→ asset registry
→ verified deploy
→ client
```

Escopo futuro:

```text
Characters
Monsters
Weapons / Items
Animation
Materials / Textures
Maps / Stage
```

Arquitetura de longo prazo:

```text
Blender Scene
→ canonical Summit asset model
→ format serializers
    ├── SMD Model 0.62
    ├── SMB
    ├── INX
    └── SMD Stage 0.72
→ asset registry
→ verified roundtrip deploy
```

---

# 2. Baseline C01

Arquivo:

```text
MfbC01.smd
```

Pristine:

```text
size   = 290229
sha256 = 9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b
```

Cliente:

```text
char/tmABCD/MfbC01.smd
```

Estrutura global:

```text
objects          15
materials        6
vertices         1444
faces            2537
UV/TexLinks      2537
bind_vertices    1444
bind_bones_used  44
armature_bones   95
root             Bip01
```

LOD:

```text
HIGH     MbhC01 MshC01 MlhC01
DEFAULT  MbmC01 MsmC01 MlmC01
LOW      MblC01 MslC01 MllC01
```

MshC01 pristine:

```text
vertices 173
faces    331
TexLinks 331
material 1
```

---

# 3. Direct SMD runtime path — PROVADO EM RUNTIME

Foi provado que a rota direta SMD é utilizada pelo cliente para o asset estudado.

Pipeline válido:

```text
Blender
→ SMD 0.62
→ client
```

A rota ASE não é necessária para os gates atuais de remodelagem direta.

---

# 4. Evolução da prova de topologia

A progressão de risco foi:

```text
mover vertex antigo
→ mover vários
→ novo vertex
→ novas faces
→ HIGH LOD
→ smooth topology
→ remodel antigo + novo
→ região anatômica
→ full reindex
→ full free-mesh writer
→ artist-authored topology
```

## 4.1 Nova topologia controlada

Foram provados em runtime:

```text
mudança de vertex count
mudança de face count
novos TexLinks
novos Physique records
```

## 4.2 Full reindex

V1 reindexou todos os 173 vertices e reordenou todas as 331 faces, mantendo aparência idêntica.

Conclusões:

```text
VERTEX INDEX IDENTITY NOT REQUIRED = PROVEN IN RUNTIME
FACE ORDER IDENTITY NOT REQUIRED   = PROVEN IN RUNTIME
```

## 4.3 Free Mesh Writer

O writer V2 provou:

```text
all_vertices_rebuilt=True
all_faces_rebuilt=True
all_texlinks_rebuilt=True
all_physique_rebuilt=True
```

Build de prova anterior:

```text
MshC01 197 / 367
semantic PASS
independent reimport PASS
runtime PASS
```

Conclusão:

```text
FREE MESH SERIALIZATION = PROVADA EM RUNTIME
```

---

# 5. Primeiro asset de topologia realmente feita pelo artista

O artista partiu do MshC01 original e usou Subdivide/modelagem normal no Blender.

Resultado artístico:

```text
vertices          = 656
artist_polygons   = 1129
loops             = 3550

polygon sizes:
triangles = 984
quads     = 134
5-gons    = 4
6-gons    = 7
```

Isso é importante porque não foi uma topologia produzida por helper algorítmico específico da engine.

---

# 6. Descoberta: Edit Mode / BMesh / UV — PROVADO POR TESTE

Ao abrir o `.blend` salvo em Edit Mode:

```text
Mesh RNA SMD_UV data_len=0
```

Isso inicialmente pareceu perda de UV.

Probe posterior provou:

```text
BMesh:
vertices = 173
faces    = 331
loops    = 993
UV       = 993 corners válidos
```

Após `mode_set(OBJECT)`:

```text
SMD_UV data_len = 993
```

Conclusão:

```text
UV WAS NOT LOST
```

Regra permanente:

```python
if target.mode == "EDIT":
    set active object
    bpy.ops.object.mode_set(mode="OBJECT")
    mesh.update()
```

Só depois ler geometria, UV e atributos.

Essa sincronização não salva o arquivo.

---

# 7. Descoberta: triangulação canônica — PROVADO POR TESTE

No asset artístico 656/1129:

## Mesh.calc_loop_triangles()

```text
triangles         1292
degenerate_count  69
min_area          0
```

## Temporary BMesh BEAUTY triangulation

```text
before_vertices   656
after_vertices    656
before_faces      1129
after_faces       1292
degenerate_count  0
min_area          0.0206603531999
max_area          5.00493296175
UV nonfinite      0
```

Diagnóstico:

```text
BMESH_TRIANGULATION_AVOIDS_LOOPTRI_DEGENERATES
```

Conclusão:

O artista não precisava corrigir a mesh. O problema era o algoritmo de triangulação escolhido pelo exporter.

Regra atual:

```text
artist quads/ngons
→ TEMP BMesh copy
→ bmesh.ops.triangulate BEAUTY
→ triangle export view
```

O `.blend` artístico não é triangulado/salvo destrutivamente.

---

# 8. Physique rígido vs Blender weighted groups

A auditoria da source fornecida indica que o runtime estudado usa um bone por vertex e não preserva weighted skinning real.

O teste artístico confirmou a consequência prática do Blender Subdivide:

```text
vertices                 656
strict_rigid_vertices    399
mixed_vertices           257
empty_group_vertices     0

group count:
1 group   399
2 groups  241
3 groups  16
```

Exemplo:

```text
vertex 173:
Bip01 Spine1      0.5
Bip01 L Clavicle  0.5
```

O validator corretamente rejeitou isso.

---

# 9. Game-Normalized Copy — arquitetura aprovada

Em vez de forçar o artista a modelar sob as restrições da engine:

```text
ARTIST MASTER
    ↓
GAME-NORMALIZED COPY
```

O master é preservado.

Ferramenta:

```text
pt_rigid_skin_normalize_copy_v1.py
```

No teste:

```text
strict_rigid_before  399
mixed_before         257
preserved_rigid      399
auto_resolved        190
unresolved           67
```

Políticas conservadoras:

```text
UNIQUE_MAX_WEIGHT
TIE_ADJACENT_RIGID_MAJORITY
TIE_ADJACENT_RIGID_DISTANCE
```

Ambiguidades são marcadas:

```text
PT_REVIEW_REQUIRED
```

O pipeline de produção para antes do export enquanto houver revisão pendente.

---

# 10. Force Resolve TEST-ONLY

Para fechar o gate de engenharia sem exigir trabalho artístico manual:

```text
pt_rigid_skin_force_resolve_test_v1.py
```

Os 67 ambíguos foram resolvidos considerando apenas os bones candidatos já presentes no vertex e escolhendo o seed rígido geometricamente mais próximo.

Resultado:

```text
review_vertices_before = 67
forced_resolved        = 67
invalid_after          = 0
review_group_removed   = True
final_rigid_bone_count = 22
```

Política:

```text
test_only=True
production_policy=False
```

Não deve ser transformada silenciosamente em política artística final.

---

# 11. Validator V1.4

Ferramenta:

```text
pt_free_mesh_validate_v1_4.py
```

No asset artístico normalizado:

```text
vertices                656
source_polygons         1129
export_triangles        1292
triangulation_delta     +163
triangulation_method    TEMP_BMESH_BEAUTY
artist_mesh_modified    False
active_uv_data_count    3550
used_materials          {1:1292}
rigid_bone_count        22
max_vertex_fit_error    0.00436732026855
mean_vertex_fit_error   0.00163040528089
free_mesh_validation_check=PASS
```

Gates continuam rígidos.

---

# 12. Free Mesh Writer V2.3

Ferramenta:

```text
pt_blender_to_smd_free_mesh_v2_3.py
```

Resultado:

```text
source_counts             173/331/331
artist_vertices           656
artist_source_polygons    1129
export_triangles          1292
output_counts             656/1292/1292
vertex_delta              +483
face_delta                +961

all_vertices_rebuilt      True
all_faces_rebuilt         True
all_texlinks_rebuilt      True
all_physique_rebuilt      True

unrelated_object_blocks_byte_identical 14
semantic_check            PASS
free_mesh_writer_check    PASS
```

Build:

```text
size   = 382625
sha256 = 1695df612bf3b13f5a9a0a27b14c41dfbbb55a39704d9fa3739c6be2013ea030
```

---

# 13. Independent V3 reimport — PROVADO ESTRUTURALMENTE

Importer:

```text
history_py/pt_smd_blender_import_v3_static_armature.py
```

Reimport:

```text
MshC01 V=656 F=1292 UV=1292
```

Resumo:

```text
objects                 15
materials               6
vertices                1927
faces                   3498
uv_faces                3498
bind_vertices           1927
bind_bones_used         44
armature_bones          95
armature_roots          Bip01
mesh_armature_modifiers 15
visible HIGH            MshC01 MlhC01 MbhC01

static_armature_import_check=PASS
```

---

# 14. Artist-authored free topology — PROVADO EM RUNTIME

O build acima foi instalado e executado no jogo.

Relato do teste:

```text
funcionou perfeitamente
```

Conclusão formal:

```text
ARTIST-AUTHORED FREE TOPOLOGY
+ NON-DESTRUCTIVE GAME NORMALIZATION
+ DIRECT SMD EXPORT
= PROVADO EM RUNTIME
```

Foi provado que o artista pode partir do modelo existente, alterar substancialmente a topologia e retornar ao runtime sem depender da identidade de vertices/faces do original.

---

# 15. Character Artist Pipeline V1 / V1.1

Wrapper:

```text
sh/pt_character_artist_pipeline_v1_1.sh
```

Fluxo:

```text
STAGE 1 Normalize Copy
STAGE 2 optional Force Ambiguous Skin TEST ONLY
STAGE 3 Validate
STAGE 4 Export SMD
STAGE 5 Independent V3 Reimport
STAGE 6 optional Publish to Roundtrip Workspace
```

Gate fechado:

```text
artist_master_overwritten=False
test_force_ambiguous_used=true
output_counts=656/1292/1292
smd_size=382625
smd_sha256=1695df...
runtime_installed=False
pipeline_check=PASS
```

Sem `--force-test`, o comportamento correto é:

```text
PIPELINE_STATUS=REVIEW_REQUIRED
```

---

# 16. Asset Registry / Round-Trip

Foi integrado um segundo sistema dedicado a descoberta, registry e movimentação de assets.

Princípio:

```text
workspace path NEVER decides return destination

Asset_ID / roundtrip_target
decides return destination
```

Workspace Pikeman:

```text
blender_workspace_pikeman/workspace_manifest.json
```

O preparador original copiou 69 entradas e reportou 23 faltantes. Isso não bloqueou o SMD físico, que foi registrado pelo Bridge.

---

# 17. Gap descoberto no catálogo inicial

O manifesto source-derived inicial não possuía entradas SMD/SMB físicas suficientes para o caso estudado.

`MfbC01.smd` precisou ser registrado como asset físico.

Bridge:

```text
summit_workspace_asset_bridge_v1.py
```

Registro validado:

```text
asset_id         AST-791F1CAE2A4C
roundtrip_target char/tmABCD/MfbC01.smd
workspace_path   Characters/Pikeman/MfbC01.smd
source_size      290229
source_sha256    pristine
register_check   PASS
```

---

# 18. Publish validado para workspace — PROVADO

O build aprovado foi publicado no slot registrado:

```text
published_size   382625
published_sha256 1695df612bf3b13f5a9a0a27b14c41dfbbb55a39704d9fa3739c6be2013ea030
client_modified  False
publish_check    PASS
```

O Bridge valida:

```text
pipeline_check=PASS
manifest SMD SHA/size
workspace slot
workspace drift
```

antes de publicar.

---

# 19. Bugs encontrados no roundtrip original

Foram reproduzidos dois problemas:

## 19.1 Scanner CSV

O scanner original misturava campos `action` e `import_action` entre assets conhecidos e fallback físicos, podendo gerar `ValueError` no `csv.DictWriter`.

Correção:

```text
summit_scan_physical_assets_v2.py
```

## 19.2 Restore sem client drift

O restore original podia sobrescrever um client asset que tivesse mudado depois da criação do workspace.

Isso é inadequado para produção.

Correção:

```text
summit_roundtrip_restore_v2.py
```

---

# 20. Restore V2 — PROVADO

Dry-run validado:

```text
mode=DRY_RUN
candidate_count=1
PLAN AST-791F1CAE2A4C
roundtrip_restore_check=DRY_RUN_PASS
```

Apply validado:

```text
backup criado
transaction.json criado
WROTE AST-791F1CAE2A4C
size 382625
sha256 1695df...
roundtrip_restore_check=PASS
```

SHA final no cliente:

```text
1695df612bf3b13f5a9a0a27b14c41dfbbb55a39704d9fa3739c6be2013ea030
```

Fluxo integrado:

```text
Artist Pipeline
→ validated SMD
→ Workspace Registry
→ Publish
→ Dry-run
→ Backup
→ Verified Apply
→ Client
```

---

# 21. Segurança de deploy

Regra atual:

```text
build validation
!=
client deployment
```

Restore V2:

```text
path containment
published SHA validation
client drift detection
all-target backup before writes
backup hash verification
atomic replacement
post-write hash/size verification
rollback on failure
transaction report
```

O deploy deve sempre iniciar com dry-run.

---

# 22. Formato SMD 0.62 — fatos já estabelecidos

Layout i686 estudado:

```text
sizeof(void*)      4
smDFILE_HEADER     556
smDFILE_OBJINFO    40
smTEXLINK          32
smVERTEX           24
smFACE             36
smTM_ROT           20
smTM_POS           16
smTM_SCALE         16
smOBJ3D            2236
```

Offsets relevantes:

```text
smTEXLINK:
u0=0
v0=12
hTexture=24
NextTex=28

smFACE:
v=0
t=8
lpTexLink=32

smOBJ3D:
Vertex=4
Face=8
TexLink=12
Physique=16
nVertex=84
nFace=88
nTexLink=92
NodeName=172
Tm=240
```

No C01:

```text
1 TexLink per face
NextTex=0
Physique = one 32-byte bone name per vertex
```

---

# 23. Normals

Para o caminho direto, normals são relevantes no runtime.

O writer atual gera normals para a topologia reconstruída pelo método já aceito visualmente/runtime nos gates realizados.

Hard edges devem ser representados por vertex duplication, não por smoothing groups SMD.

Uma futura implementação canônica deve aproximar o comportamento `SetGNormals()` da engine.

---

# 24. Limites informados pela auditoria da source

Ainda classificados como **AUDITORIA DA SOURCE**, embora coerentes com o código descrito:

```text
smOBJ3D render vertex gate 30000
render face buffers classe 30000
smPAT3D max 128 nodes/objects
smFACE vertex indices WORD
até 8 texture stages úteis
MATRIAL_MAX 5000
rigid Physique efetivo
```

Esses limites devem permanecer como gates conservadores até revalidação independente.

---

# 25. Texturas

Já provado anteriormente:

```text
protected BMP decode → Blender
BMP edit/reprotect → client
protected TGA decode
standard TGA reconstruction
ASE material resolution
UV import/patch
```

MfbC01.bmp foi modificado no runtime com sucesso.

Ainda falta material editor universal.

---

# 26. Animação

Trabalho anterior provou leitura/visualização de motions individuais e mapeamento source→Blender em vários motions.

Pontos estabelecidos:

```text
SCENE_TICKSPERFRAME 160         # auditoria/source evidence
rotation Slerp
position linear
scale fixed-point linear
```

A automação de todas as motions/classes e writer de animação ainda não é produto consolidado.

---

# 27. Maps / Stage

Model SMD 0.62 e Stage SMD 0.72 são formatos distintos.

A auditoria fornecida indica que Stage inclui:

```text
geometry
materials
UV
vertex color
lights
spatial cells
collision semantics
lightmap-related layers
```

E que outros metadados de mapa vivem em `sFIELD`/estruturas separadas.

Map editor continua como etapa futura.

---

# 28. Ferramentas ativas

Core Blender:

```text
pt_rigid_skin_normalize_copy_v1.py
pt_rigid_skin_force_resolve_test_v1.py
pt_free_mesh_validate_v1_4.py
pt_blender_to_smd_free_mesh_v2_3.py
history_py/pt_smd_blender_import_v3_static_armature.py
```

Pipeline:

```text
sh/pt_character_artist_pipeline_v1_1.sh
```

Registry / deploy:

```text
summit_prepare_blender_workspace.py
summit_asset_manifest.json
summit_scan_physical_assets_v2.py
summit_workspace_asset_bridge_v1.py
summit_roundtrip_restore_v2.py
```

Recovery específico validado:

```text
sh/pt_c01_artist_free_mesh_runtime_v1.sh
```

---

# 29. Artefatos e hashes-chave

Pristine:

```text
MfbC01.smd
290229
9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b
```

Artist free mesh runtime build:

```text
382625
1695df612bf3b13f5a9a0a27b14c41dfbbb55a39704d9fa3739c6be2013ea030
```

Asset Registry:

```text
AST-791F1CAE2A4C
→ char/tmABCD/MfbC01.smd
```

---

# 30. Estado da arquitetura

A distinção final aprovada é:

```text
ARTIST LAYER
    Blender master

NORMALIZATION LAYER
    game-compatible copy

FORMAT LAYER
    SMD/SMB/INX/Stage serializers

VALIDATION LAYER
    semantic/runtime constraints

REGISTRY LAYER
    Asset_ID/origin/roundtrip_target

DEPLOY LAYER
    dry-run/backup/drift detection/atomic restore
```

Essa separação deve ser preservada ao generalizar.

---

# 31. O que NÃO deve ser feito

Não:

```text
afrouxar validator para aceitar degenerados
forçar weighted skinning no SMD estudado
triangular destrutivamente o master
sobrescrever Artist Master
escrever direto no cliente a partir do exporter
usar workspace path para inferir origem
usar --apply sem dry-run
ignorar CLIENT_DRIFT
assumir que todo limite da auditoria já foi runtime-provado
```

---

# 32. Próximo milestone

A pergunta “topologia livre feita por artista funciona?” está encerrada.

A pergunta “round-trip registry/deploy integra com o build pipeline?” também está encerrada.

Próximo trabalho:

```text
GENERIC CHARACTER OBJECT PIPELINE
```

Remover hardcoding de `MshC01` nos componentes relevantes.

Objetivo incremental:

```text
--object MshC01
--object MbhC01
--object MlhC01
--object MsmC01
--object MbmC01
--object MlmC01
...
```

Depois:

```text
--lod high
--lod default
--lod low
```

Depois:

```text
--character <profile>
```

O registry/deploy já pode permanecer como camada comum.

---

# 33. Status final em 2026-08-26

```text
DIRECT SMD MODEL 0.62 RUNTIME              PROVEN
EXISTING TOPOLOGY EDIT                     PROVEN
NEW VERTEX/FACE/TEXLINK/PHYSIQUE           PROVEN
FULL VERTEX REINDEX                        PROVEN
FACE REORDER                               PROVEN
FREE MESH SERIALIZATION                    PROVEN
ARTIST-AUTHORED FREE TOPOLOGY              PROVEN
NON-DESTRUCTIVE BMESH TRIANGULATION        PROVEN
RIGID-SKIN NORMALIZATION                   PROVEN
INDEPENDENT V3 ROUNDTRIP                   PROVEN
ASSET REGISTRY PHYSICAL BRIDGE             PROVEN
VALIDATED BUILD PUBLISH                    PROVEN
DRY-RUN + VERIFIED ROUNDTRIP DEPLOY        PROVEN
CLIENT EXECUTION OF ARTIST BUILD           PROVEN
```

O projeto deixou de ser uma coleção de patches experimentais do C01. O core atual já é uma prova funcional da arquitetura de uma ferramenta de produção; a prioridade agora é generalização e interface.


---

# 34. Update 2026-08-27 — Geometry Core fechado em runtime

## 34.1 Generic single-object runtime

`MbhC01` foi reconstruído pelo writer genérico sem alteração de counts:

```text
MbhC01 130/217/217
materials [1,2]
rigid bones 21
other object blocks byte-identical 14
```

O build foi instalado pelo registry/Restore V3 e o personagem permaneceu plenamente funcional.

Conclusão:

```text
GENERIC SINGLE-OBJECT CORE = PROVADO EM RUNTIME
```

Isto complementa o `MshC01`, que já tinha sido provado com topologia livre.

## 34.2 HIGH multi-object runtime

Um único SMD foi construído reconstruindo atomicamente:

```text
MbhC01
MshC01
MlhC01
```

O `MshC01` artístico permaneceu em:

```text
656 vertices
1129 artist polygons
1292 export triangles
```

Os outros 12 blocos do SMD permaneceram byte-identical.

O cliente carregou e executou o personagem.

Conclusão:

```text
ARTIST-EDITED HIGH LOD
+ FREE TOPOLOGY
+ MULTI-OBJECT REBUILD
+ ATOMIC SMD SERIALIZATION
= PROVADO EM RUNTIME
```

## 34.3 C01 ALL LODs runtime

O pipeline V2 reconstruiu em um único SMD os nove objetos C01:

```text
HIGH
  MbhC01
  MshC01
  MlhC01

DEFAULT
  MbmC01
  MsmC01
  MlmC01

LOW
  MblC01
  MslC01
  MllC01
```

Os seis blocos C03 permaneceram byte-identical.

Build:

```text
size   = 382625
sha256 = 9b4a992945b7d0ce30aaff63b1141433132ecf57e17a4036177c9d94d8fa5b9c
```

O mesmo SMD passou três reimports independentes:

```text
HIGH    visible MshC01 MlhC01 MbhC01
DEFAULT visible MsmC01 MlmC01 MbmC01
LOW     visible MslC01 MllC01 MblC01
```

Todos com:

```text
objects=15
materials=6
vertices=1927
faces=3498
uv_faces=3498
bind_vertices=1927
bind_bones_used=44
armature_bones=95
root=Bip01
static_armature_import_check=PASS
```

O usuário confirmou o build no runtime e o personagem continuou funcional.

Conclusão:

```text
CHARACTER C01 GEOMETRY CORE = PROVADO EM RUNTIME
```

A geometria C01 não precisa mais ser usada para provar conceitos básicos de serialização.

---

# 35. Causa da deformação visual observada

No asset artístico, `MshC01` tinha:

```text
strict rigid 399
mixed        257
auto safe    190
ambiguous     67
```

Em produção, os 67 ambíguos ficam marcados como:

```text
PT_REVIEW_REQUIRED
```

e o pipeline deve parar para revisão do artista.

Nos gates de engenharia foi usado:

```text
--force-test
```

que rigidizou os 67 restantes por proximidade de seeds rígidos.

O usuário confirmou posteriormente que a deformação observada era causada por essa rigidização forçada e que, corrigidos os vínculos, a deformação desapareceu.

Conclusão de produto:

```text
ENGINE COMPATIBILITY = responsabilidade do pipeline
VISUAL RIG QUALITY   = responsabilidade do artista
```

`--force-test` continua:

```text
test_only=True
production_policy=False
```

Nunca transformar essa heurística em política silenciosa de produção.

---

# 36. Milestone atual — Character Asset como unidade

Com a geometria fechada, a unidade de trabalho deixa de ser:

```text
MshC01
MbhC01
MlhC01
...
```

e passa a ser:

```text
Character Asset
```

Um Character Asset profile deve resolver:

```text
Model SMD
Skeleton SMB
Model info INX
LOD groups
material/texture assets
roundtrip targets
registry Asset_IDs
```

O primeiro profile de segunda geração é:

```text
profiles/pt_character_profile_pikeman_c01_m5_v2.json
```

O primeiro orchestrator de Character Asset é:

```text
pt_character_asset_v1.py
```

Comandos de alto nível:

```text
inspect
sync-workspace
build
deploy-plan
deploy
```

A intenção é esconder do artista detalhes como:

```text
MshC01
MbhC01
roundtrip_target
SMB filename
INX filename
```

Esses dados pertencem ao profile.

---

# 37. Próximas features após Character Asset V1

Ordem atual:

```text
1. Character Asset como unidade       <- EM IMPLEMENTAÇÃO
2. Material + Texture workflow
3. Animation SMB + INX
4. Skeleton / sockets
5. Blender UI / addon
6. outros personagens / monstros
7. Stage 0.72 / mapas
```

---

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
---------------------------------------

## A. Em que parte do problema estamos

Hoje eu colocaria o projeto assim:

```text
1. CHARACTER ASSET como unidade        ███████░░░  ~70%
2. MATERIAL + TEXTURE WORKFLOW         ████████░░  ~85%
3. ANIMATION                           ███░░░░░░░  ~30%
4. SKELETON / SOCKETS                  █████░░░░░  ~50%
5. Blender UI / Add-on                 █░░░░░░░░░  ~10%
6. outros personagens / monstros       ██░░░░░░░░  ~20%
7. Stage 0.72 / mapas                  ░░░░░░░░░░  ~0–5%
```

Essas porcentagens são de **maturidade do pipeline**, não de quantidade de código.

### 1. CHARACTER ASSET — estamos no final do núcleo técnico

Já temos a identidade real do Pikeman, profile correto, workspace correto, cadeia `SMD + SMB + INX + textures`, seleção High/Default/Low e armor-family orientada pelo INX. O runtime realmente seleciona High/Default/Low por distância, em vez de heurística por sufixo. 

Também fechamos agora uma parte fundamental que estava faltando: **geometria editada do Pikeman real**.

```text
TmbC01.smd
→ Blender
→ TbmC03 modificado
→ bone-local reconstruído
→ direct SMD
→ reimport
→ runtime

= PROVEN
```

E o teste High mostrou que conseguimos fazer o mesmo sem perder a seleção de LOD: `TbhC03 + TahC01` continuaram sendo exatamente os objetos ativos no High.  

**O grande buraco restante neste bloco é topologia livre no Pikeman real.**

Mover vértices está provado. Agora precisamos provar:

```text
novo vertex count
novo face count
novos TexLinks
novos índices
Physique reconstruído
→ runtime
```

Isso é o que transforma o sistema de “editor de malha existente” em **remodelador real**.

---

### 2. MATERIAL + TEXTURE — núcleo quase fechado

Aqui estamos mais avançados.

Já temos runtime real para:

```text
protected BMP decode/edit/reprotect      PROVEN
TmbC01 texture                           PROVEN
TmbC03 texture                           PROVEN

SelfIllum                                PROVEN
BlendType                                PROVEN
BlinkColor RGB + timing                  PROVEN
TwoSide                                  PROVEN
```

E agora o `TwoSide` finalmente deixou de ser apenas “aceito visualmente”.

O A tinha `Material 0 TwoSide=0`; B tinha `Material 0 TwoSide=1`, mantendo o material 1 em zero.  

Sua observação runtime foi determinística:

```text
A / TwoSide=0
→ olhando pelo verso:
  face não renderiza
  aparece o ambiente

B / TwoSide=1
→ olhando pelo verso:
  face renderiza
  aparece a textura
```

Portanto:

```text
PIKEMAN smMATERIAL::TwoSide
= RUNTIME PROVEN
```

A e B também chegaram efetivamente ao cliente através do fluxo gerenciado com backup e transaction.  

Ainda não considero o bloco 2 totalmente encerrado porque faltam principalmente:

```text
Transparency
→ sabemos que afeta ordering/Z-write
→ ainda não é um simples "Opacity"

TextureStageState
TextureFormState
TextureType e combinações relevantes
multi-texture/layers onde aplicável

workflow final:
Blender asset
→ SMD + todas as texturas dependentes
→ build/publish como uma única unidade
```

Ou seja: **semântica principal está provada; UX e alguns estados avançados ainda faltam.**

---

### 3. ANIMATION — investigação avançou, mas pipeline de edição não está fechado

Aqui temos bastante conhecimento de SMB/INX, motions e evaluator, mas ainda não temos o equivalente a:

```text
SMB
→ Blender Action utilizável pelo artista
→ editar
→ exportar SMB
→ runtime fiel
```

Essa é uma frente grande por si só.

Então eu não misturaria agora animation com a geometria. Primeiro fechamos o Character Asset estático/remodelável.

---

### 4. SKELETON / SOCKETS — metade do caminho

Já temos:

```text
m4.smb
→ 58 bones
→ hierarchy
→ bind
→ Physique
→ Blender Armature
```

No Pike isso continua íntegro mesmo após nossas alterações; o último reimport manteve 58 bones, 32 usados e 9 Armature modifiers. 

Mas ainda falta transformar isso em authoring real:

```text
editar skeleton
→ exportar

weapon sockets
attachment nodes
weapon02 / in02 / etc.
→ semântica formal
→ editar
→ exportar
→ runtime
```

---

### 5. Blender UI / Add-on — ainda não é a prioridade

Hoje temos excelentes ferramentas de engenharia, mas são ferramentas de CLI.

O artista ainda vê:

```text
python3 ...
flatpak run ...
manifest
SHA
publish
restore
```

O futuro add-on deverá esconder isso atrás de coisas como:

```text
Import Character
Armor Variant
LOD Preview

Material:
  Self Illum
  Two Sided
  Blend Mode
  Blink Color
  Blink Speed

Build Character
Validate
Deploy Test
Restore Original
```

Só vale consolidar isso quando os formatos fundamentais estiverem estabilizados.

---

### 6. Outros personagens / monstros — arquitetura existe, generalização falta

Priestess foi extremamente importante porque nela provamos inclusive topologia livre.

Mas não queremos novamente cometer o erro de:

```text
funcionou na Priestess
→ assumir que funciona no Pike
```

Agora estamos fazendo o correto:

```text
engine-level fact
+
profile-level validation
+
runtime gate por asset family
```

Depois de fechar Pike, a expansão para outras classes deve ficar muito mais rápida.

---

### 7. Stage 0.72 / mapas — ainda é outra campanha

Temos a distinção fundamental:

```text
SMD Model data Ver 0.62
≠
SMD Stage data Ver 0.72
```

O Stage precisa de pipeline próprio. Não devemos misturar isso com personagem.

---

# B. Milestones oficiais que já podemos registrar

Eu consolidaria o estado de hoje assim:

```text
MILESTONE C1 — CHARACTER IDENTITY
Pikeman real = TmbC01.smd + m4.smb + M4Bip.inx
CLOSED

MILESTONE C2 — CHARACTER IMPORT
SMD + SMB + INX → Blender
geometry / UV / material / Physique / LOD
CLOSED

MILESTONE C3 — ARMOR FAMILY
C001..C005 semantics + runtime selection
CLOSED

MILESTONE T1 — PROTECTED TEXTURE
decode → edit → reprotect → deploy → restore
CLOSED

MILESTONE M1 — SELF ILLUM
Blender → SMD → runtime
CLOSED / RUNTIME PROVEN

MILESTONE M2 — BLEND TYPE
Blender → SMD → runtime
CLOSED / RUNTIME PROVEN

MILESTONE M3 — BLINK COLOR
RGB + UseState + BlinkTime
Blender → SMD → runtime pulse
CLOSED / RUNTIME PROVEN

MILESTONE M4 — TWO SIDE
controlled A/B
same geometry
1-byte difference
backface absent vs rendered
CLOSED / RUNTIME PROVEN

MILESTONE G1 — PIKEMAN VERTEX GEOMETRY
Blender TbmC03 edit
→ bone-local rebuild
→ SMD
→ reimport
→ runtime visible
CLOSED / RUNTIME PROVEN

MILESTONE G2 — HIGH/DEFAULT LOD GEOMETRY
TbmC03 + TbhC03 independently modifiable
runtime LOD selection preserved
CLOSED

MILESTONE G3 — FREE TOPOLOGY PIKEMAN
PENDING
```

Esse `G3` é exatamente onde estamos agora.

---

# C. Próxima sequência

Antes de G3 há uma ação obrigatória: **o cliente atualmente está com B**, SHA:

```text
3dada096de4c81496e3fad53a8086839312036628c999de297aa579b2d5149cc
```

O log confirma que B foi realmente instalado e que o Restore V4 criou backup e transaction. 

Portanto o próximo movimento deve ser:

```text
C1. restaurar TmbC01.smd pristine
    SHA 9c5b137b...6443c

C2. registrar milestones acima

C3. construir PIKEMAN FREE TOPOLOGY GATE V1

    TbmC03
    74V / 122F
       ↓
    subdivisão/remodelação controlada
       ↓
    V e F realmente diferentes

C4. reconstruir:
    smVERTEX[]
    smFACE[]
    smTEXLINK[]
    Physique[]
    object offsets/lengths

C5. preservar:
    material group
    outros 8 smOBJ3D
    animation-related object state
    names/Tm
    unrelated bytes sempre que possível

C6. independent V3 reimport

C7. runtime C002

C8. restore pristine
```

Se `G3` funcionar no runtime, muda bastante o estado do projeto.

Nesse ponto poderemos dizer:

```text
CHARACTER MESH AUTHORING CORE

existing game character
→ Blender
→ genuine remodel / topology change
→ game-compatible SMD
→ runtime

PROVEN ON:
Priestess
Pikeman
```

E aí o **item 1 — CHARACTER ASSET como unidade** estará muito próximo de ser considerado tecnicamente fechado; o trabalho restante passará a ser sobretudo automação/profile/generalização, não descoberta fundamental do formato.
## A. Em que parte do problema estamos

Hoje eu colocaria o projeto assim:

```text
1. CHARACTER ASSET como unidade        ███████░░░  ~70%
2. MATERIAL + TEXTURE WORKFLOW         ████████░░  ~85%
3. ANIMATION                           ███░░░░░░░  ~30%
4. SKELETON / SOCKETS                  █████░░░░░  ~50%
5. Blender UI / Add-on                 █░░░░░░░░░  ~10%
6. outros personagens / monstros       ██░░░░░░░░  ~20%
7. Stage 0.72 / mapas                  ░░░░░░░░░░  ~0–5%
```

Essas porcentagens são de **maturidade do pipeline**, não de quantidade de código.

### 1. CHARACTER ASSET — estamos no final do núcleo técnico

Já temos a identidade real do Pikeman, profile correto, workspace correto, cadeia `SMD + SMB + INX + textures`, seleção High/Default/Low e armor-family orientada pelo INX. O runtime realmente seleciona High/Default/Low por distância, em vez de heurística por sufixo. 

Também fechamos agora uma parte fundamental que estava faltando: **geometria editada do Pikeman real**.

```text
TmbC01.smd
→ Blender
→ TbmC03 modificado
→ bone-local reconstruído
→ direct SMD
→ reimport
→ runtime

= PROVEN
```

E o teste High mostrou que conseguimos fazer o mesmo sem perder a seleção de LOD: `TbhC03 + TahC01` continuaram sendo exatamente os objetos ativos no High.  

**O grande buraco restante neste bloco é topologia livre no Pikeman real.**

Mover vértices está provado. Agora precisamos provar:

```text
novo vertex count
novo face count
novos TexLinks
novos índices
Physique reconstruído
→ runtime
```

Isso é o que transforma o sistema de “editor de malha existente” em **remodelador real**.

---

### 2. MATERIAL + TEXTURE — núcleo quase fechado

Aqui estamos mais avançados.

Já temos runtime real para:

```text
protected BMP decode/edit/reprotect      PROVEN
TmbC01 texture                           PROVEN
TmbC03 texture                           PROVEN

SelfIllum                                PROVEN
BlendType                                PROVEN
BlinkColor RGB + timing                  PROVEN
TwoSide                                  PROVEN
```

E agora o `TwoSide` finalmente deixou de ser apenas “aceito visualmente”.

O A tinha `Material 0 TwoSide=0`; B tinha `Material 0 TwoSide=1`, mantendo o material 1 em zero.  

Sua observação runtime foi determinística:

```text
A / TwoSide=0
→ olhando pelo verso:
  face não renderiza
  aparece o ambiente

B / TwoSide=1
→ olhando pelo verso:
  face renderiza
  aparece a textura
```

Portanto:

```text
PIKEMAN smMATERIAL::TwoSide
= RUNTIME PROVEN
```

A e B também chegaram efetivamente ao cliente através do fluxo gerenciado com backup e transaction.  

Ainda não considero o bloco 2 totalmente encerrado porque faltam principalmente:

```text
Transparency
→ sabemos que afeta ordering/Z-write
→ ainda não é um simples "Opacity"

TextureStageState
TextureFormState
TextureType e combinações relevantes
multi-texture/layers onde aplicável

workflow final:
Blender asset
→ SMD + todas as texturas dependentes
→ build/publish como uma única unidade
```

Ou seja: **semântica principal está provada; UX e alguns estados avançados ainda faltam.**

---

### 3. ANIMATION — investigação avançou, mas pipeline de edição não está fechado

Aqui temos bastante conhecimento de SMB/INX, motions e evaluator, mas ainda não temos o equivalente a:

```text
SMB
→ Blender Action utilizável pelo artista
→ editar
→ exportar SMB
→ runtime fiel
```

Essa é uma frente grande por si só.

Então eu não misturaria agora animation com a geometria. Primeiro fechamos o Character Asset estático/remodelável.

---

### 4. SKELETON / SOCKETS — metade do caminho

Já temos:

```text
m4.smb
→ 58 bones
→ hierarchy
→ bind
→ Physique
→ Blender Armature
```

No Pike isso continua íntegro mesmo após nossas alterações; o último reimport manteve 58 bones, 32 usados e 9 Armature modifiers. 

Mas ainda falta transformar isso em authoring real:

```text
editar skeleton
→ exportar

weapon sockets
attachment nodes
weapon02 / in02 / etc.
→ semântica formal
→ editar
→ exportar
→ runtime
```

---

### 5. Blender UI / Add-on — ainda não é a prioridade

Hoje temos excelentes ferramentas de engenharia, mas são ferramentas de CLI.

O artista ainda vê:

```text
python3 ...
flatpak run ...
manifest
SHA
publish
restore
```

O futuro add-on deverá esconder isso atrás de coisas como:

```text
Import Character
Armor Variant
LOD Preview

Material:
  Self Illum
  Two Sided
  Blend Mode
  Blink Color
  Blink Speed

Build Character
Validate
Deploy Test
Restore Original
```

Só vale consolidar isso quando os formatos fundamentais estiverem estabilizados.

---

### 6. Outros personagens / monstros — arquitetura existe, generalização falta

Priestess foi extremamente importante porque nela provamos inclusive topologia livre.

Mas não queremos novamente cometer o erro de:

```text
funcionou na Priestess
→ assumir que funciona no Pike
```

Agora estamos fazendo o correto:

```text
engine-level fact
+
profile-level validation
+
runtime gate por asset family
```

Depois de fechar Pike, a expansão para outras classes deve ficar muito mais rápida.

---

### 7. Stage 0.72 / mapas — ainda é outra campanha

Temos a distinção fundamental:

```text
SMD Model data Ver 0.62
≠
SMD Stage data Ver 0.72
```

O Stage precisa de pipeline próprio. Não devemos misturar isso com personagem.

---

# B. Milestones oficiais que já podemos registrar

Eu consolidaria o estado de hoje assim:

```text
MILESTONE C1 — CHARACTER IDENTITY
Pikeman real = TmbC01.smd + m4.smb + M4Bip.inx
CLOSED

MILESTONE C2 — CHARACTER IMPORT
SMD + SMB + INX → Blender
geometry / UV / material / Physique / LOD
CLOSED

MILESTONE C3 — ARMOR FAMILY
C001..C005 semantics + runtime selection
CLOSED

MILESTONE T1 — PROTECTED TEXTURE
decode → edit → reprotect → deploy → restore
CLOSED

MILESTONE M1 — SELF ILLUM
Blender → SMD → runtime
CLOSED / RUNTIME PROVEN

MILESTONE M2 — BLEND TYPE
Blender → SMD → runtime
CLOSED / RUNTIME PROVEN

MILESTONE M3 — BLINK COLOR
RGB + UseState + BlinkTime
Blender → SMD → runtime pulse
CLOSED / RUNTIME PROVEN

MILESTONE M4 — TWO SIDE
controlled A/B
same geometry
1-byte difference
backface absent vs rendered
CLOSED / RUNTIME PROVEN

MILESTONE G1 — PIKEMAN VERTEX GEOMETRY
Blender TbmC03 edit
→ bone-local rebuild
→ SMD
→ reimport
→ runtime visible
CLOSED / RUNTIME PROVEN

MILESTONE G2 — HIGH/DEFAULT LOD GEOMETRY
TbmC03 + TbhC03 independently modifiable
runtime LOD selection preserved
CLOSED

MILESTONE G3 — FREE TOPOLOGY PIKEMAN
PENDING
```

Esse `G3` é exatamente onde estamos agora.

---

# C. Próxima sequência

Antes de G3 há uma ação obrigatória: **o cliente atualmente está com B**, SHA:

```text
3dada096de4c81496e3fad53a8086839312036628c999de297aa579b2d5149cc
```

O log confirma que B foi realmente instalado e que o Restore V4 criou backup e transaction. 

Portanto o próximo movimento deve ser:

```text
C1. restaurar TmbC01.smd pristine
    SHA 9c5b137b...6443c

C2. registrar milestones acima

C3. construir PIKEMAN FREE TOPOLOGY GATE V1

    TbmC03
    74V / 122F
       ↓
    subdivisão/remodelação controlada
       ↓
    V e F realmente diferentes

C4. reconstruir:
    smVERTEX[]
    smFACE[]
    smTEXLINK[]
    Physique[]
    object offsets/lengths

C5. preservar:
    material group
    outros 8 smOBJ3D
    animation-related object state
    names/Tm
    unrelated bytes sempre que possível

C6. independent V3 reimport

C7. runtime C002

C8. restore pristine
```

Se `G3` funcionar no runtime, muda bastante o estado do projeto.

Nesse ponto poderemos dizer:

```text
CHARACTER MESH AUTHORING CORE

existing game character
→ Blender
→ genuine remodel / topology change
→ game-compatible SMD
→ runtime

PROVEN ON:
Priestess
Pikeman
```

E aí o **item 1 — CHARACTER ASSET como unidade** estará muito próximo de ser considerado tecnicamente fechado; o trabalho restante passará a ser sobretudo automação/profile/generalização, não descoberta fundamental do formato.


MILESTONE G5 — MANUAL ARTIST WORKFLOW

GAME CHARACTER
→ Blender
→ edição manual do artista
→ topologia alterada
→ posições alteradas
→ UV alterado
→ pesos alterados
→ normalização segura de skin
→ validação estrita
→ direct SMD
→ C002
→ runtime

= CLOSED / RUNTIME PROVEN