# Summit Tale Blender Tools

Ferramentas de importação, edição, normalização, validação, exportação e round-trip de assets do Summit Tale / Priston Tale usando Blender.

## Estado atual

O núcleo de personagem já foi provado em runtime com um asset realmente remodelado no Blender:

```text
MshC01 original
173 vertices / 331 faces
        ↓
Subdivide/modelagem feita no Blender
        ↓
656 vertices / 1129 polígonos artísticos
        ↓
normalização não destrutiva para o jogo
        ↓
656 vertices / 1292 triângulos SMD
        ↓
SMD 0.62 reconstruído
        ↓
reimport independente
        ↓
runtime
        ↓
PASS
```

Também foi provado o caminho completo:

```text
Artist Master
→ Game-Normalized Copy
→ Validate
→ Export SMD
→ Independent Reimport
→ Asset Registry
→ Workspace Publish
→ Dry-run deploy
→ Backup + Verified Apply
→ Client
```

O pipeline atual ainda está especializado em `MshC01` da C01 para a fase de engenharia. A próxima generalização é remover essa especialização e permitir qualquer objeto/LOD/personagem.

---

## 1. Princípios do programa

### O arquivo do artista é a fonte

O `.blend` artístico nunca deve ser sobrescrito para obedecer às limitações da engine.

```text
ARTIST MASTER
    !=
GAME-NORMALIZED COPY
```

O artista pode trabalhar com quads/ngons, Subdivide e ferramentas normais do Blender. As adaptações exigidas pela engine são feitas em uma cópia.

### O jogo usa skinning rígido

Para o caminho estudado:

```text
1 vertex
→ exatamente 1 bone
→ weight efetivo 1.0
```

O Blender pode criar pesos interpolados ao subdividir. A normalização resolve automaticamente apenas os casos defensáveis e marca ambiguidades para revisão.

### Triangulação não deve destruir a topologia artística

O `.blend` pode manter quads/ngons. O SMD recebe uma vista triangular temporária criada em memória por BMesh.

Não use `Ctrl+T` como requisito do workflow.

### Build e deploy são responsabilidades diferentes

```text
Character Pipeline
    cria e valida o build

Asset Registry / Workspace
    sabe onde o asset pertence

Roundtrip Restore
    faz backup, verifica e escreve no cliente
```

O exporter não deve adivinhar o caminho do cliente.

---

## 2. Estrutura recomendada

```text
design_work/
├── README.md
├── MfbC01.smd
├── m5.smb
├── M5Bip.inx
├── sitem_master_initializer_block.bin
│
├── pt_blender_to_smd_free_mesh_v2_3.py
├── pt_free_mesh_validate_v1_4.py
├── pt_rigid_skin_normalize_copy_v1.py
├── pt_rigid_skin_force_resolve_test_v1.py
│
├── summit_prepare_blender_workspace.py
├── summit_asset_manifest.json
├── summit_scan_physical_assets_v2.py
├── summit_workspace_asset_bridge_v1.py
├── summit_roundtrip_restore_v2.py
│
├── blender/
├── blender_workspace_pikeman/
├── client_backups/
├── docs/
├── history_py/
├── logs/
├── pt_build/
├── sh/
└── archives/
```

Convenções:

```text
.blend                  → blender/
Python histórico        → history_py/
shell/wrappers          → sh/
logs                    → logs/
builds                   → pt_build/
backups do cliente      → client_backups/
documentação extensa    → docs/
pacotes antigos         → archives/
```

---

## 3. Dependências

Blender Flatpak:

```bash
flatpak run --filesystem=home org.blender.Blender
```

Python 3 para registry/roundtrip.

A mensagem OpenColorIO abaixo é conhecida e não bloqueia os testes atuais:

```text
The minor version 5 is not supported for major version 2.
Maximum minor version is 4.
Using fallback mode for management
```

---

## 4. Arquivos-base da C01 usada nos gates

```text
MfbC01.smd
size   = 290229
sha256 = 9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b

m5.smb
M5Bip.inx
```

Cliente:

```text
Summit Tale Client Full/cliente/char/tmABCD/MfbC01.smd
```

Estrutura pristine:

```text
objects          = 15
materials        = 6
vertices         = 1444
faces            = 2537
UV/TexLinks      = 2537
bind_vertices    = 1444
bind_bones_used  = 44
armature_bones   = 95
root             = Bip01
```

---

## 5. Verificar o pipeline

```bash
cd ~/Área\ de\ Trabalho/summit/design_work

./sh/pt_character_artist_pipeline_v1_1.sh doctor
```

Esperado:

```text
PRISTINE_CHECK=PASS
pipeline_doctor_check=PASS
```

---

## 6. Preparar um workspace de round-trip

Exemplo Pikeman:

```bash
python3 summit_prepare_blender_workspace.py \
  "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  ./blender_workspace_pikeman \
  --class-code C \
  --category Characters \
  --include-low \
  --hash
```

Isso cria:

```text
blender_workspace_pikeman/workspace_manifest.json
```

O `workspace_manifest.json` é a autoridade que liga:

```text
workspace_path
→ asset_id
→ roundtrip_target
```

---

## 7. Registrar um asset físico ainda ausente do catálogo source-derived

O catálogo inicial não continha o `MfbC01.smd` como entrada SMD física. O Bridge resolve esse caso:

```bash
python3 summit_workspace_asset_bridge_v1.py register \
  ./blender_workspace_pikeman \
  --game-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --target char/tmABCD/MfbC01.smd \
  --workspace-path Characters/Pikeman/MfbC01.smd \
  --category Characters \
  --subcategory "Character Model" \
  --representation "Equipped Body Mesh" \
  --owner Pikeman \
  --class-code C
```

Registro já validado:

```text
asset_id         = AST-791F1CAE2A4C
roundtrip_target = char/tmABCD/MfbC01.smd
workspace_path   = Characters/Pikeman/MfbC01.smd
```

O primeiro argumento é o **workspace**, não o client root.

---

## 8. Rodar o Artist Pipeline — produção

```bash
./sh/pt_character_artist_pipeline_v1_1.sh run \
  --artist blender/SEU_MASTER.blend \
  --name nome_do_build
```

Fluxo:

```text
Normalize Copy
→ Validate
→ Export
→ Independent V3 Reimport
```

Se houver pesos ambíguos:

```text
PIPELINE_STATUS=REVIEW_REQUIRED
```

A cópia normalizada terá o grupo:

```text
PT_REVIEW_REQUIRED
```

O artista deve:

1. selecionar os vértices marcados;
2. escolher exatamente um bone válido para cada vértice;
3. atribuir weight `1.0`;
4. remover o marcador;
5. rodar novamente usando a cópia revisada como `--artist`.

O master original permanece intocado.

---

## 9. Modo de engenharia `--force-test`

Somente para gates técnicos:

```bash
./sh/pt_character_artist_pipeline_v1_1.sh run \
  --artist blender/MfbC01_artist_free_mesh_v1.blend \
  --name MfbC01_gate \
  --force-test
```

Esse modo resolve ambiguidades restantes por heurística de bone rígido-semente mais próximo.

Ele é deliberadamente marcado como:

```text
test_only=True
production_policy=False
```

Não deve virar comportamento silencioso da ferramenta artística.

No gate já validado:

```text
399 rigid vertices preservados
257 mixed detectados
190 resolvidos conservadoramente
67 resolvidos por heurística TEST-ONLY
```

---

## 10. Saída do pipeline

Exemplo validado:

```text
artist vertices          = 656
artist source polygons   = 1129
export triangles         = 1292
output SMD counts        = 656 / 1292 / 1292
```

Build runtime-validado:

```text
size   = 382625
sha256 = 1695df612bf3b13f5a9a0a27b14c41dfbbb55a39704d9fa3739c6be2013ea030
```

O pipeline gera:

```text
blender/<name>_game_normalized.blend
blender/<name>_game_normalized_forced_test.blend     # quando necessário
blender/<name>_reimport.blend

pt_build/C01_topology/<name>.smd
pt_build/C01_topology/<name>_export_report.json

logs/<name>_normalize.json
logs/<name>_validation.json
logs/<name>_pipeline_manifest.json
...
```

O manifest final contém:

```text
pipeline_check=PASS
artist_master_overwritten=False
runtime_installed=False
SMD size/SHA
counts
reimport
```

---

## 11. Publicar o build validado no workspace

Depois de `pipeline_check=PASS`:

```bash
python3 summit_workspace_asset_bridge_v1.py publish \
  ./blender_workspace_pikeman \
  --pipeline-manifest logs/<name>_pipeline_manifest.json \
  --target char/tmABCD/MfbC01.smd
```

O Bridge valida o SMD contra o manifest antes de publicá-lo.

Esperado:

```text
client_modified=False
publish_check=PASS
```

Publicar no workspace **não altera o jogo**.

O Pipeline V1.1 também aceita publicação automática ao fim:

```bash
./sh/pt_character_artist_pipeline_v1_1.sh run \
  --artist blender/SEU_MASTER.blend \
  --name nome_do_build \
  --roundtrip-workspace ./blender_workspace_pikeman \
  --roundtrip-target char/tmABCD/MfbC01.smd
```

Adicione `--force-test` somente em teste de engenharia.

---

## 12. Deploy para o cliente

### Sempre faça dry-run primeiro

```bash
python3 summit_roundtrip_restore_v2.py \
  ./blender_workspace_pikeman \
  --target char/tmABCD/MfbC01.smd
```

Esperado antes de um deploy:

```text
mode=DRY_RUN
candidate_count=1
PLAN ...
roundtrip_restore_check=DRY_RUN_PASS
```

Nenhum arquivo é alterado.

### Aplicar

```bash
python3 summit_roundtrip_restore_v2.py \
  ./blender_workspace_pikeman \
  --target char/tmABCD/MfbC01.smd \
  --apply
```

O Restore V2:

```text
detecta client drift
→ faz backup de todos os alvos
→ verifica SHA/tamanho dos backups
→ escreve atomicamente
→ verifica SHA/tamanho pós-write
→ faz rollback em falha
→ gera transaction.json
```

Se aparecer:

```text
CLIENT_DRIFT
```

não force o deploy. Descubra por que o cliente mudou.

Se já estiver aplicado:

```text
SKIP ... reason=ALREADY_APPLIED
```

---

## 13. Restaurar o C01 pristine

O instalador de engenharia continua útil como recovery específico:

```bash
./sh/pt_c01_artist_free_mesh_runtime_v1.sh restore-original
./sh/pt_c01_artist_free_mesh_runtime_v1.sh status
```

Esperado:

```text
TARGET_STATE=PRISTINE
TARGET_SIZE=290229
TARGET_SHA256=9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b
```

---

## 14. Validações atualmente rígidas

O pipeline não deve enfraquecer estes gates para “fazer passar” um asset:

```text
UV válida
sem NaN/Inf
sem triângulos degenerados no export
<= 30000 vertices por objeto
<= 30000 faces de render por objeto
material permitido
1 bone válido por vertex
fixed-point fit dentro do limite
skeleton/bone names válidos
SMD semantic roundtrip
independent V3 reimport
```

O `.blend` artístico pode conter quads/ngons; o export não.

---

## 15. Descobertas Blender importantes

### Edit Mode e UV

No Blender 5.x, ao salvar em Edit Mode, `Mesh RNA` pode mostrar a UV com `data_len=0` enquanto a informação viva continua no BMesh.

Antes de ler geometria/UV:

```python
if obj.mode == "EDIT":
    bpy.ops.object.mode_set(mode="OBJECT")
mesh.update()
```

Isso apenas sincroniza em memória; não salva o `.blend`.

### Não usar `mesh.calc_loop_triangles()` como caminho canônico

No primeiro asset artístico:

```text
Mesh.calc_loop_triangles()
triangles          = 1292
degenerate_count   = 69

TEMP BMesh BEAUTY
triangles          = 1292
degenerate_count   = 0
min_area           = 0.0206603531999
```

Portanto o exporter usa BMesh temporário.

---

## 16. Limitações atuais

Comprovado para o core atual:

```text
SMD Model 0.62
C01 / MshC01
rigid Physique
UV
material legado usado pelo MshC01
free topology
full vertex/face/TexLink/Physique rebuild
runtime
asset registry/deploy
```

Ainda pendente como ferramenta genérica:

```text
qualquer objeto do personagem
HIGH/DEFAULT/LOW automático
qualquer personagem
weighted skinning real — engine não suporta no caminho estudado
material editor completo
SMB writer genérico
INX writer genérico
animação integrada ao addon
monstros/armas
Stage SMD 0.72
Field metadata
UI/addon Blender final
```

---

## 17. Próxima direção

Não precisamos mais provar se topologia livre funciona.

Próximo milestone:

```text
generic object pipeline
```

Objetivo:

```text
--object MshC01
--object MbhC01
--object MlhC01
--object MsmC01
...
```

Depois:

```text
--lod high
--lod default
--lod low
```

E finalmente:

```text
--character <profile>
```

A partir daí, o mesmo registry/deploy já validado pode ser reutilizado para personagens, armas, monstros e mapas.
