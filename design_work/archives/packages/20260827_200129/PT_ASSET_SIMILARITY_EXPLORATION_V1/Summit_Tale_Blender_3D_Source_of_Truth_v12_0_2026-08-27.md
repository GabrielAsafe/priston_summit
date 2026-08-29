# Summit Tale — Blender/3D/Texturas
## Source of Truth Técnica v12.0 — 2026-08-27

## 1. Objetivo desta edição

Esta edição consolida o estado alcançado após aplicar o pipeline completo em
Priestess e, principalmente, Pikeman. Ela registra:

- o que foi provado;
- correções de identidade;
- formatos e ABI já fechados;
- pipeline de textura/material;
- armor/LOD;
- geometria e topologia;
- round-trip e restauração do cliente;
- erros metodológicos encontrados;
- o estado correto do full-character asset;
- por que não devemos repetir essa investigação asset por asset;
- a estratégia de exploração por famílias de similaridade;
- próximos passos.

Regra de evidência:

```text
PROVADO ESTRUTURALMENTE
PROVADO VISUALMENTE
PROVADO EM RUNTIME
PENDENTE
REJEITADO / TESTE INVÁLIDO
```

não devem ser misturados.

---

# 2. Princípios de arquitetura agora consolidados

```text
ARTIST ASSET != GAME ASSET
ARTIST TEXTURE != GAME TEXTURE
```

Blender é ambiente de authoring. O runtime consome estruturas proprietárias
com semântica própria.

Para personagem:

```text
GAME CHARACTER ASSET
    SMD Model0.62
    + SMB skeleton/motion
    + INX configuration/motion metadata
    + texture assets
```

Armor config não é equivalente a um único modelo visual:

```text
Character
→ ArmorConfig Cxxx
→ Model SMD
→ NodeNames selecionados por LOD
→ material slots DO SMD
→ texture assets
```

Material slot não possui semântica global entre SMDs diferentes.

---

# 3. Correção crítica de identidade

Uma parte inicial do trabalho chamou incorretamente:

```text
MfbC01.smd + m5.smb + M5Bip.inx
```

de Pikeman.

Isso é Priestess.

Identidade correta:

```text
Pikeman:
    TmbC01.smd
    m4.smb
    M4Bip.inx

Priestess:
    MfbC01.smd
    m5.smb
    M5Bip.inx
```

Também:

```text
m4.smb → Pikeman
m5.smb → Priestess
TmbC14_p.ase → histórico Pikeman
MfbC11_p.ase → histórico Priestess
```

Consequência:

- provas antigas de `MfbC01` pertencem à Priestess;
- `blender_workspace_pikeman` histórico está semanticamente mal nomeado;
- Pikeman real usa `blender_workspace_pikeman_real`;
- profile correto Pikeman:
  `profiles/pt_character_profile_pikeman_c01_m4_v1.json`.

---

# 4. Pikeman real — identidade e baseline

Cadeia runtime:

```text
M4Bip.inx
  model  = char\tmABCD\TmbC01.ASE
  motion = char\tmABCD\m4.ase

arquivo físico atual:
  char/tmABCD/TmbC01.smd
  char/tmABCD/m4.smb
```

Pristine:

```text
TmbC01.smd
size   187504
SHA256 9c5b137bc637f485419cafae6864c4a37f77835d32210810295c1b58c746443c

m4.smb
size   12414984
SHA256 48af614678593eaa0b659c2be514bd60f86b94f72cfb4305ed8352f48f12cc4a
```

Import V3 do TmbC01:

```text
objects        9
materials      2
skeleton       58
bind vertices  955
bind bones     32
```

Meshes:

```text
TbhC03 92/152
TbmC03 74/122
TblC03 63/100

TblC01 73/112
TbmC01 74/128
TbhC01 85/156

TahC01 222/406
TamC01 157/282
TalC01 115/192
```

Materiais:

```text
material 0 → TmbC03.bmp
material 1 → TmbC01.bmp
```

---

# 5. SMD Model data Ver 0.62 — core fechado

Header:

```text
SMD Model data Ver 0.62
```

Formato Stage/map é distinto:

```text
SMD Stage data Ver 0.72
```

Nunca misturar os dois.

ABI i686 medido/provado para o modelo:

```text
smDFILE_HEADER 556
smDFILE_OBJINFO 40
smOBJ3D 2236
smVERTEX 24
smFACE 36
smTEXLINK 32
smTM_ROT 20
smTM_POS 16
smTM_SCALE 16
```

Macro-layout:

```text
smDFILE_HEADER
smDFILE_OBJINFO[ObjCounter]
material group
object block 0
...
object block N
```

Object block:

```text
raw smOBJ3D
smVERTEX[nVertex]
smFACE[nFace]
smTEXLINK[nTexLink]
rotation keys
position keys
scale keys
previous-rotation matrices
Physique bone names, se presente
```

Semântica principal:

```text
Face.v[0..2] = vertex indices
Face.v[3]    = material id
Face.lpTexLink = first UV layer
TexLink.NextTex = next UV layer
TexLink.u/v = UV per face corner
```

`smFACE.t[]` não deve ser tratado como UV neste caminho.

Physique do SMD estudado:

```text
um bone/node name rígido por vértice
```

Coordenadas:

```text
SMD = ASE * 256
Blender = SMD / 256
```

Sem troca Y/Z no reader de modelo.

---

# 6. smMATERIAL — ABI e semântica

ABI i686:

```text
sizeof(smFCOLOR)        12
sizeof(smTEXTUREHANDLE) 176
sizeof(smMATERIAL)      320
sizeof(smMATERIAL_GROUP) 88
```

Campos principais provados incluem:

```text
InUse
TextureCounter
BlendType
Shade
TwoSide
Diffuse
Transparency
SelfIllum
TextureClip
UseState
MeshState
WindMeshBottom
AnimTexCounter
```

Pikeman pristine:

```text
material 0:
  texture TmbC03.bmp

material 1:
  texture TmbC01.bmp

BlendType    1
Shade        1
TwoSide      0
Transparency 0
SelfIllum    0
UseState     0
MeshState    1
```

Controles runtime provados:

```text
SelfIllum   RUNTIME PROVEN
TwoSide     RUNTIME PROVEN
BlendType   RUNTIME PROVEN
BlinkColor  RUNTIME PROVEN
```

`Shade`:

```text
serializado PROVADO
uso visual runtime não encontrado
policy = PRESERVE
```

`Transparency`:

```text
serializado PROVADO
ordering/Z-write source-proved
opacidade visual genérica ainda não provada
```

Diffuse não deve ser interpretado automaticamente como cor difusa estática
genérica.

---

# 7. Texturas protegidas

TmbC01.bmp e TmbC03.bmp pertencem à família:

```text
14-byte proprietary prefix
+
BITMAPINFOHEADER padrão começando em offset 14
```

Prefixo observado:

```text
41383a091319243140519a7990a9
```

Baseline estudado:

```text
256x256
24 bpp
BI_RGB
```

Pipeline:

```text
protected BMP
→ substituir prefixo por BITMAPFILEHEADER temporário
→ Blender/editor
→ preservar bytes não-pixel
→ restaurar prefixo proprietário
→ deploy
```

TmbC01 e TmbC03:

```text
decode       PROVEN
edit         PROVEN
reprotect    PROVEN
runtime      PROVEN
restore      PROVEN
```

Princípio de produção:

```text
primeiro preservar tamanho/bpp/alpha/path/header;
mudanças de resolução são outro gate.
```

---

# 8. Armor config e LOD

INX leading layout usado:

```text
0x000 model path
0x040 motion path
0x080 submodel path

0x0C0 High group
0x104 Default group
0x148 Low group
```

Cada `_MODELGROUP`:

```text
count
char name[4][16]
```

C001:

```text
High    TbhC01 + TahC01
Default TbmC01 + TamC01
Low     TblC01 + TalC01
```

C002:

```text
High    TbhC03 + TahC01
Default TbmC03 + TamC01
Low     TblC03 + TalC01
```

Outros configs auditados:

```text
C001 PASS
C002 RUNTIME PROVEN
C003 RUNTIME PROVEN
C004 RUNTIME VALID / PARTIAL_NODE_MATCH
C005 RUNTIME PROVEN
```

C004 revelou uma regra importante da engine:

```text
config pode pedir NodeNames inexistentes;
renderer itera objetos existentes;
pedido inexistente apenas não casa/não renderiza;
não implica crash.
```

Arquitetura de validação:

```text
AUTHORING STRICT
    referências devem resolver

RUNTIME PARITY / LEGACY INSPECTION
    unresolved legacy NodeNames podem ser registrados/reproduzidos
```

Armor visual não deve ser inferido por suffix de filename. A engine usa os
grupos do config.

---

# 9. Armor index

Pikeman normal:

```text
index 0 → C001.ini
index 1 → C001.ini
index 2 → C002.ini
index 3 → C003.ini
index 4 → C004.ini
index 5 → C005.ini
...
```

`SetPlayerArmor` seleciona config por índice/item. Não foi encontrado check de
RequiredLevel na seleção visual estudada.

---

# 10. Geometria — Priestess

A Priestess foi a primeira vertical slice de geometria livre.

Pristine:

```text
MfbC01.smd
15 objects
6 materials
1444 vertices
2537 faces
95 bones
```

Provado:

```text
controlled topology increase
smoothing
free topology/reindex
rebuild vertices/faces/TexLinks/Physique
direct SMD
multi-object
runtime
```

Essas provas pertencem à Priestess, não ao Pikeman.

---

# 11. Geometria — Pikeman milestones

## G1 — vertex position

`TbmC03` scale XY 1.20 com topologia/UV/material preservados.

```text
Blender edit                    PROVEN
rigid groups                    PROVEN
world→SMD local                 PROVEN
binary isolation                PROVEN
independent reimport            PROVEN
runtime                         PROVEN
```

## G2 — LOD position

High/Default position geometry:

```text
CLOSED / RUNTIME PROVEN
```

Não significa topologia nova em todos os LODs.

## G3 — free topology

TbmC03:

```text
74V/122F → 75V/124F
new vertex
new faces
new TexLinks
new Physique
direct SMD rebuild
```

Runtime aprovou. A saliência estava nas costas; isso corrigiu a inferência
errada de que sinal do eixo Y indicava automaticamente frente/trás.

```text
G3 CLOSED / RUNTIME PROVEN
```

## G4 — connected artist remodel

Multi-face connected patch:

```text
+8 vertices
+16 faces
existing vertices moved
new UV
new Physique
```

Primeiro witness foi visualmente sutil. Witness decisivo ampliou a diferença.

```text
G4 CLOSED / RUNTIME PROVEN
```

## G5 — manual artist workflow

Artista editou manualmente `TbmC03`.

Resultado do build aprovado:

```text
source 74/122
artist 83/132 polygons
export 83/140/140
```

Pesos:

```text
80 rigid
3 mixed
3 auto-resolved com regra segura
0 unresolved
```

Runtime:

```text
manual Blender edit
→ safe skin normalization
→ strict validation
→ direct SMD
→ C002
= RUNTIME PROVEN
```

```text
G5 CLOSED
```

## G6 — manual all-LOD

Manual:

```text
TbhC03 High
TbmC03 Default
TblC03 Low
```

Um único SMD, C002 High/Default/Low.

```text
multi-object rebuild
LOD switching
runtime
= PROVEN
```

```text
G6 CLOSED
```

---

# 12. G7 — full character asset e lição de costuras

Objetivo:

```text
9 meshes
→ one SMD
→ C001 + C002
→ High/Default/Low
```

Primeiro witness automático de geometria escalou:

```text
body C01 XY ×1.20 em seu próprio centróide
arms C01 XY ×1.12 em seu próprio centróide
```

Estruturalmente passou, mas runtime revelou:

```text
ligação braço ↔ tronco abriu
```

Esse teste geométrico deve ser classificado:

```text
REJEITADO COMO WITNESS DE FULL-ASSET GEOMETRY
```

Não é falha do SMD writer. É falha de authoring/composição: peças que
compartilham costuras não podem ser transformadas de modo independente em
pivôs diferentes se a continuidade deve ser preservada.

Lição:

```text
FORMAT COMPATIBILITY != ART ASSEMBLY COMPATIBILITY
```

e:

```text
piece boundaries / seam constraints
```

precisam entrar em qualquer futura ferramenta automática de remodelagem
multi-piece.

---

# 13. Reset completo e full-asset UV witness

Após o problema de costura:

1. cliente restaurado para TmbC01 pristine;
2. alterações G5/G6/G7 arquivadas/retiradas do workspace ativo;
3. novo import feito do pristine;
4. witness automático aplicado somente em `SMD_UV`;
5. posições, topologia e weights permaneceram pristine;
6. 9 meshes rebuildados;
7. C001/C002 × High/Default/Low validados;
8. material audit;
9. publish/deploy por bridge + Restore V4;
10. SHA final verificado;
11. runtime funcionou.

Witness:

```text
u' = u * 0.90 + 0.05
v' = v * 0.90 + 0.05
```

Estado correto:

```text
FULL 9-OBJECT REBUILD            RUNTIME SMOKE PROVEN
C001/C002 CONFIG SELECTION       RUNTIME SMOKE PROVEN
HIGH/DEFAULT/LOW                 RUNTIME SMOKE PROVEN
UV CHANGED                       RUNTIME PROVEN
GEOMETRY POSITIONS               PRISTINE
TOPOLOGY                         PRISTINE
SKIN WEIGHTS                     PRISTINE
```

Este teste não substitui G5/G6 para geometria livre. Ele prova que o full-asset
container/config pipeline é estável sem introduzir seam deformation.

---

# 14. Backup e restore — regra de produção

Todo deploy deve possuir:

```text
verified source
known build SHA
dry-run
client drift detection
backup
transaction
post-write SHA
restore route
```

Restore V4 corrigiu o caso:

```text
workspace = source original
client = previous managed build
```

que o V3 tratava incorretamente como unchanged.

Regra V4:

```text
UNCHANGED somente se:
workspace == source AND client == source

client == known previous managed build
→ restoration candidate

unknown client SHA
→ CLIENT_DRIFT / STOP
```

Esse mecanismo é obrigatório para produção.

---

# 15. Animação / skeleton — estado herdado e reutilizável

A investigação anterior fechou:

```text
SMD/SMB bind pose
INX MotionInfo decode
source-equivalent evaluator
TmSort parent-first
tick/frame unit = 160
STAND
WALK
RUN
ATTACK
source skinning
Blender Armature Mapping
```

Exemplo:

```text
FrameInfo.StartFrame = MotionInfo.StartFrame * 160
FrameInfo.EndFrame   = MotionInfo.EndFrame * 160
```

A infraestrutura já saiu da fase de matemática fundamental.

Pendente principal:

```text
empacotar automação de motions/classes;
expandir representative certification para outras skeleton families;
UI de produção.
```

---

# 16. O problema de escala do projeto

Não é viável repetir:

```text
identity
→ parser proof
→ Blender
→ manual edit
→ build
→ deploy
→ runtime
```

para:

```text
cada personagem
cada NPC
cada mob
cada arma
cada armor
cada map
```

A unidade correta de engenharia passa a ser:

```text
TECHNICAL COMPATIBILITY FAMILY
```

e não:

```text
INDIVIDUAL ASSET
```

---

# 17. Estratégia de similaridade

## Família A — SMD Model0.62 + Physique rígido

Se um asset possui:

```text
SMD Model data Ver 0.62
object table válida
smOBJ3D ABI compatível
Physique por vértice
smFACE/smTEXLINK compatíveis
material group compatível
```

ele não deve receber engenharia reversa do zero.

Hipótese de produção:

```text
existing importer/writer core
+ auto-generated profile
+ skeleton/config pairing
+ representative runtime certification
```

Pikeman e Priestess são referências distintas da mesma macro-família.

## Família B — SMD Model0.62 estático

Grande parte do reader/material/UV é reaproveitável.

Diferença provável:

```text
Physique absent
```

Logo deve existir um branch de writer/static model, não um novo parser inteiro.

Armas/props podem cair aqui.

## Família C — INX fixed model/group layout

Config com:

```text
model
motion
submodel
High/Default/Low groups
```

deve usar o parser genérico que já existe.

Profile por classe/config pode ser gerado, não escrito à mão.

## Família D — SMB/Biped

Se a estrutura e nomes/skeleton seguem a família já conhecida:

```text
bind/motion evaluator core é reutilizável
```

O correto é certificar skeleton families, não motions individualmente.

## Família E — protected BMP prefix14

Prefixo/layout compatível:

```text
decode/edit/reprotect core reutilizável imediatamente
```

## Família F — ASE com PHYSIQUE

Reader/edit core já é forte.

Porém, se runtime atual usa SMD, produção deve preferir:

```text
ASE como source/legacy reference
SMD como runtime artifact
```

e não construir duas pipelines concorrentes sem necessidade.

## Família G — Stage0.72

```text
SMD Stage data Ver 0.72
```

é outra tecnologia.

Deve ter vertical slice própria para maps/stages.

---

# 18. Novo scanner de exploração

Criado:

```text
pt_asset_similarity_survey_v1.py
run_asset_similarity_survey_v1.sh
```

O scanner é read-only.

Ele percorre:

```text
client
design_work
```

e inspeciona profundamente:

```text
.smd
.smb
.inx
.ini
.ase
.bmp
.tga
```

Também conta todas as extensões existentes.

Reconhece:

```text
SMD Model0.62 rigid
SMD Model0.62 static
SMD mixed Physique
Stage0.72
INX model/group layout
SMB Biped signal
protected BMP prefix14
standard BMP
standard/nonstandard TGA
ASE Physique
ASE static
INI references
```

Para SMD Model0.62 extrai:

```text
ObjCounter
MatCounter
object table
NodeNames
nVertex
nFace
nTexLink
Physique presence
texture refs
```

Compara estruturalmente com:

```text
PIKEMAN_C01_M4
PRIESTESS_C01_M5
```

e atribui um score heurístico.

Esse score:

```text
NÃO é runtime proof
```

Serve para clustering/priorização.

---

# 19. Tiers de compatibilidade

```text
A_REUSE_CORE_NOW
```

Mesmo core técnico. Próxima ação:

```text
auto profile
batch structural validation
1–3 representative runtime tests
```

```text
B_REUSE_WITH_ADAPTER
```

Maior parte reutilizável, mas existe delta semântico.

Exemplos possíveis:

```text
static/no Physique
protected TGA variant
config variant
```

```text
C_SEPARATE_FAMILY
```

Formato já reconhecido como tecnologia separada:

```text
Stage0.72
```

```text
D_NEW_REVERSE_ENGINEERING
```

Só deve consumir tempo se a família tiver volume/importância suficiente.

---

# 20. Estratégia para avançar em tempo hábil

## Passo 1 — inventário total

Executar scanner sobre client + design_work.

Resultado:

```text
asset_inventory.csv
compatibility_families.csv
asset_sets_from_inx.csv
reuse_queue.csv
asset_similarity_report.md
```

## Passo 2 — medir cobertura

Perguntas:

```text
quantos SMD são Model0.62?
quantos são rigid Physique?
quantos INX usam layout já conhecido?
quantos BMP usam prefix14 conhecido?
quantos assets caem em Stage0.72?
quais extensões/famílias desconhecidas têm maior volume?
```

## Passo 3 — representative sampling

Não testar 500 assets.

Exemplo:

```text
cluster:
SMD0.62
rigid
9–15 objects
2–6 materials
INX standard
Biped skeleton

→ escolher 3 representantes:
  character
  NPC
  monster
```

Se os invariants fecharem e runtime passar:

```text
certify family
```

Depois todos os membros recebem:

```text
structural regression only
```

até aparecer um outlier.

## Passo 4 — auto profile generator

Construir:

```text
SMD + INX + SMB
→ inferred profile JSON
```

Campos automáticos:

```text
model path
skeleton/motion pair
object list
material count
texture refs
LOD groups
bone names/counts
Physique mode
format family
```

O profile deve possuir:

```text
confidence
family id
representative runtime certification id
```

## Passo 5 — generic asset CLI

Unificar operações:

```text
pt_asset inspect
pt_asset import
pt_asset validate
pt_asset build
pt_asset publish
pt_asset restore
```

O usuário não deve escolher scripts internos individualmente.

## Passo 6 — Blender add-on/UI

Somente depois da família/CLI estar estabilizada.

Objetivo:

```text
Import Game Asset
Show LOD/Armor groups
Validate
Build
Publish
Restore
```

## Passo 7 — outras famílias

Prioridade baseada no scanner:

```text
maior quantidade
+
maior reuse score
+
maior valor de gameplay
```

e não por ordem arbitrária.

---

# 21. Próximos milestones recomendados

## S1 — Asset Similarity Survey

```text
client-wide inventory
family clustering
Pike/Priestess comparison
reuse coverage
unknown family ranking
```

Primeiro próximo passo.

## S2 — Auto Profile Generator

Para todo cluster A:

```text
SMD+INX+SMB → profile
```

## S3 — Batch Structural Certification

Rodar parser/validators em centenas de assets sem Blender/runtime.

Objetivo:

```text
prove that same ABI/layout closes across the family
```

## S4 — Cross-category runtime representatives

Escolher poucos:

```text
1 other playable character
1 NPC
1 mob
1 weapon/static model
```

Não um de cada filename.

## A1 — Animation Family Automation

Depois de conhecer skeleton families:

```text
auto enumerate MotionInfo
auto build Blender Actions
auto map equipment motion families
```

## W1 — Weapon/static asset adapter

Se scanner mostrar SMD0.62 static em armas:

```text
reuse SMD reader/material/UV
add no-Physique writer branch
runtime representative
```

## MAP1 — Stage0.72 vertical slice

Somente após character/item coverage estar mensurada.

---

# 22. Política de certificação futura

Um asset individual pode ter estado:

```text
STRUCTURAL_MATCH
FAMILY_CERTIFIED
RUNTIME_REPRESENTATIVE
RUNTIME_DIRECT
OUTLIER
```

Não exigir `RUNTIME_DIRECT` para cada asset.

A família recebe runtime certification quando:

```text
format signature igual
ABI invariants iguais
semantic branch igual
representatives passam
```

Assets restantes:

```text
STRUCTURAL_MATCH to certified family
```

Isso é o mecanismo necessário para escalar.

---

# 23. Estado atual resumido

```text
TEXTURE CORE                       PROVEN
PROTECTED BMP ROUNDTRIP            RUNTIME PROVEN
SMD Model0.62 READ                 PROVEN
SMD Model0.62 DIRECT WRITE         RUNTIME PROVEN
MATERIAL ABI                       PROVEN
MATERIAL CONTROLS                  RUNTIME PROVEN (subset)
SMB BIND/MOTION CORE               PROVEN
INX MODEL/LOD SELECTION            RUNTIME PROVEN
PIKEMAN ARMOR FAMILY C001..C005    CHARACTERIZED / runtime sampled
PIKEMAN FREE TOPOLOGY              RUNTIME PROVEN
PIKEMAN MANUAL ARTIST WORKFLOW     RUNTIME PROVEN
PIKEMAN MANUAL ALL-LOD             RUNTIME PROVEN
FULL 9-MESH REBUILD                RUNTIME SMOKE PROVEN (UV-only)
FULL independent geometry scaling  REJECTED due seam break
PRIESTESS geometry core            RUNTIME PROVEN
Stage0.72 maps                     SEPARATE / PENDING PIPELINE
CLIENT BACKUP/RESTORE V4           PROVEN
```

A frente agora deve sair de:

```text
per-asset reverse engineering
```

para:

```text
family discovery
→ automatic profiles
→ batch certification
→ representative runtime testing
→ production tooling
```
