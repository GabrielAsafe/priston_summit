# Summit Tale — Blender Tools / 3D / Texturas
## Source of Truth Técnica v11.6 — Remodelagem SMD Runtime, Free Topology e Roadmap de Ferramenta
### Estado consolidado em 2026-08-26

> Continuação operacional da Source of Truth v11.5.
>
> Este documento consolida os testes realizados após a v11.5, principalmente a frente de:
>
> - edição direta de `SMD Model data Ver 0.62`;
> - nova topologia em runtime;
> - suavização e remodelagem do HIGH LOD da Priestess C01;
> - serialização segura com backup/restore;
> - conclusões da auditoria de source recebida em `Texto.zip`;
> - mudança de arquitetura: de patcher específico para serializer genérico.
>
> Regra de evidência:
>
> - **PROVADO EM RUNTIME** — testado dentro do cliente.
> - **PROVADO VISUALMENTE** — inspecionado no Blender ou cliente.
> - **PROVADO ESTRUTURALMENTE** — parser/serializer/reimport fechou semanticamente.
> - **AUDITORIA DA SOURCE** — afirmado pela auditoria fornecida pelo usuário com referência a `.cpp/.h`, mas ainda não revalidado independentemente nesta sessão contra a árvore-fonte.
> - **PENDENTE** — ainda não testado/implementado.
>
> Quando uma conclusão da auditoria ainda não foi validada localmente, ela não deve ser promovida automaticamente a `PROVADO EM RUNTIME`.

---

# 1. Objetivo final do projeto

Construir uma ferramenta Blender capaz de trabalhar diretamente com assets do Summit Tale / Priston Tale:

```text
asset original do jogo
→ importar
→ editar no Blender
→ remodelar livremente
→ UV / materiais / texturas
→ rigging
→ animação
→ LOD
→ validar
→ serializar
→ instalar
→ restaurar
```

Escopo desejado:

```text
Character Editor
Monster Editor
Weapon / Item Editor
Map / Stage Editor
Animation Editor
Material / Texture Editor
```

A direção final não é mais:

```text
patch específico da Priestess C01
```

A direção agora é:

```text
Blender Scene
→ modelo intermediário canônico Summit
→ serializers independentes
    ├── SMD Model 0.62
    ├── SMB
    ├── INX
    └── SMD Stage 0.72
```

---

# 2. Baseline Priestess C01 usada nos testes

Arquivo original:

```text
MfbC01.smd
```

Tamanho:

```text
290229 bytes
```

SHA256 pristine:

```text
9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b
```

Cliente alvo:

```text
Summit Tale Client Full/cliente/char/tmABCD/MfbC01.smd
```

Estrutura global pristine:

```text
objects           = 15
materials         = 6
vertices          = 1444
faces             = 2537
UV/TexLinks       = 2537
bind_vertices     = 1444
bind_bones_used   = 44
armature_bones    = 95
armature_root     = Bip01
```

LOD lógico:

```text
HIGH:
    MbhC01
    MshC01
    MlhC01

DEFAULT:
    MbmC01
    MsmC01
    MlmC01

LOW:
    MblC01
    MslC01
    MllC01
```

Torso:

```text
HIGH    = MshC01
DEFAULT = MsmC01
LOW     = MslC01
```

Contagem pristine de `MshC01`:

```text
vertices = 173
faces    = 331
TexLinks = 331
material = 1
```

O HIGH foi adotado como alvo principal para testes visuais porque as diferenças de modelagem são muito mais fáceis de avaliar de perto.

---

# 3. Caminho direto SMD em runtime — PROVADO

Foi provado que, embora o INX referencie logicamente um `.ASE`, a engine deriva o `.smd`, verifica o SMD primeiro e retorna sem fazer parse do ASE quando o cache/runtime SMD existe.

Consequência:

```text
Blender
→ SMD direto
→ client
```

é uma rota válida para a Priestess C01.

A frente de trabalho deixou de depender do caminho ASE para os testes de remodelagem runtime.

---

# 4. Edição de topologia existente — PROVADA EM RUNTIME

Pipeline seguro inicial:

```text
Blender
→ mover vértices existentes
→ preservar índices/topologia
→ reconstruir SMD
→ backup automático
→ instalar
→ testar
```

Foi provado que alterações XYZ em vértices existentes funcionam no cliente sem quebrar:

```text
textura
animação
Physique
LOD
runtime
```

Ferramenta SAFE existente:

```text
pt_c01_tool_v2.sh
```

Comandos:

```bash
./pt_c01_tool_v2.sh doctor
./pt_c01_tool_v2.sh status
./pt_c01_tool_v2.sh export [--force]
./pt_c01_tool_v2.sh push [--force]
./pt_c01_tool_v2.sh undo
./pt_c01_tool_v2.sh redo
./pt_c01_tool_v2.sh restore-original
./pt_c01_tool_v2.sh history
```

Regra mantida:

```text
não alterar o fluxo SAFE enquanto experimentos estruturais estiverem em andamento
```

---

# 5. Nova topologia DEFAULT — PROVADA EM RUNTIME

## 5.1 Primeiro split controlado

Objeto:

```text
MsmC01
```

Alteração:

```text
125 → 126 vertices
238 → 240 faces
238 → 240 TexLinks
```

Foi criado:

```text
1 novo vértice
2 faces adicionais líquidas
novo UV/TexLink
novo Physique
```

O novo vértice foi rigidamente ligado a:

```text
Bip01 Spine1
```

Resultado runtime:

```text
client estável
textura correta
animação correta
alteração visível no DEFAULT
alteração desaparece ao trocar para HIGH/LOW
```

Conclusão:

**PROVADO EM RUNTIME**

```text
NEW vertex
+ NEW faces
+ NEW UV/TexLinks
+ NEW Physique
+ SMD rebuild
→ runtime
```

## 5.2 Patch DEFAULT multi-face

Alteração:

```text
MsmC01
125 / 238
→
129 / 246
```

Build:

```text
size   = 290997
sha256 = 30b8352d18074fe82982b67b7e670934505d182751ec7529284e614ae5b132d9
```

Runtime estável.

Visualmente a alteração foi pequena; por isso os testes seguintes passaram para o HIGH.

---

# 6. Nova topologia HIGH — PROVADA EM RUNTIME

Helper inicial:

```text
pt_c01_high_topology_make_patch_v1.py
```

Objeto:

```text
MshC01
```

Faces selecionadas:

```text
[154,152,328,165,325,329,162,166]
```

Alteração:

```text
173 → 181 vertices
331 → 347 faces
+8 vertices
+16 faces
```

Exporter:

```text
pt_blender_to_smd_high_topology_v1_append_safe.py
```

Build:

```text
size   = 291765
sha256 = 474d9e22f36af0732f4e20d1c72f01b64960658b92d8cac2899c6fa5b54b17b8
```

Reimport independente:

```text
MshC01: V=181 F=347 UV=347
static_armature_import_check=PASS
```

O usuário confirmou visualmente que a alteração aparecia claramente no HIGH e o cliente permaneceu estável.

Conclusão:

**PROVADO EM RUNTIME**

```text
HIGH
+ new vertices
+ new triangles
+ new TexLinks
+ new Physique
+ normals
→ client
```

---

# 7. Suavização real HIGH — PROVADA VISUALMENTE E EM RUNTIME

O patch artificial provava formato, mas não qualidade artística.

Foi criado:

```text
pt_c01_high_smooth_patch_v1.py
```

Método:

```text
seleção de 12 faces Spine1
1 triangle → 4 triangles
shared edge vertices
PN-style midpoint nas arestas internas
linear midpoint nas bordas
sem offset artificial constante
```

Faces:

```text
[154,152,328,165,325,329,162,166,183,324,323,330]
```

Resultado Blender:

```text
MshC01
173 → 197 vertices
331 → 367 faces
+24 vertices
+36 faces
```

Deslocamento interno PN:

```text
min  ≈ 0.00813
mean ≈ 0.08974
max  ≈ 0.17917
```

Build SMD:

```text
size   = 294021
sha256 = b1678aced23e84f560e65be56a2c11b5f35980228ee9afe32548c1424197ac23
```

Reimport independente:

```text
MshC01: V=197 F=367 UV=367
vertices global = 1468
faces global    = 2573
static_armature_import_check=PASS
```

Runtime:

```text
client estável
```

Avaliação visual do usuário:

```text
"ficou visivelmente mais suave. ficou bom."
```

Conclusão:

**PROVADO VISUALMENTE + PROVADO EM RUNTIME**

---

# 8. Exporter HIGH remodel V2 — compatibilidade byte-exact PROVADA

Foi criado:

```text
pt_blender_to_smd_high_remodel_v2.py
```

Objetivo:

```text
combinar:
    movimento de vértices antigos
    +
    topologia nova
```

Política conservadora V2:

```text
old vertex não movido:
    registro smVERTEX byte-exact

old vertex movido:
    reencodar apenas XYZ
    preservar triplet de normal legado
    preservar rigid Physique bone

new vertex:
    geometry-derived normal
    rigid Physique
```

Teste de compatibilidade sem mover vértices antigos:

```text
input blend:
MfbC01_high_smooth_patch_v1.blend
```

Resultado:

```text
moved_old_vertices = 0
new_vertices       = 24
faces              = 367
size               = 294021
semantic_check     = PASS
high_remodel_export_check = PASS
```

SHA produzido:

```text
b1678aced23e84f560e65be56a2c11b5f35980228ee9afe32548c1424197ac23
```

Esse SHA é **exatamente igual** ao exporter V1 já aprovado em runtime.

Conclusão:

**NO-REGRESSION BYTE-EXACT PASS**

---

# 9. Remodelagem combinada: old vertices + new topology — PROVADA EM RUNTIME

Helper:

```text
pt_c01_high_remodel_helper_v1.py
```

Operação:

```text
HIGH_PATCH_LAPLACIAN_RELAX
```

Input:

```text
MfbC01_high_smooth_patch_v1.blend
```

Vértices originais movidos:

```text
[2,3,16,83,85,87,89,90,104,105,106,169,170]
```

Contagem:

```text
13 old vertices moved
24 appended vertices refitted
```

Deslocamento dos antigos:

```text
min  = 0.0520343045
mean = 0.170682082
max  = 0.316416028
```

Exporter V2:

```text
output_counts = 197 / 367 / 367
moved_old_vertices = 13
new_vertices = 24
face_delta = +36
max_moved_old_vertex_fit_error = 0.00318965433517
max_new_vertex_fit_error       = 0.00329697073114
semantic_check                 = PASS
high_remodel_export_check      = PASS
```

Build:

```text
size   = 294021
sha256 = dc9c95ba6cc6cf78889563bbbcbdb3962f2de24e6d2e401b5121491be6954a0c
```

O usuário confirmou que o personagem ficou mais suave e considerou o teste um sucesso.

Conclusão:

**PROVADO EM RUNTIME**

```text
existing vertex movement
+ new vertices
+ new faces
+ UV
+ Physique
+ animation
+ runtime
```

funcionam juntos.

---

# 10. Anatomia / upper chest — probe e teste regional

A suavização do torso tornou mais evidentes áreas de baixa resolução, especialmente o peitoral superior e a transição peito/ombro.

Foi criado um anatomy probe.

## 10.1 Descoberta de não-manifold

O primeiro probe abortou por assumir manifold estrito.

A malha real contém arestas com 3 faces:

```text
(1,5)  → [7,158,168]
(1,82) → [151,158,168]
(5,82) → [158,168,178]
```

Distribuição de edge degree em `MshC01`:

```text
degree 2 = 484
degree 1 = 16
degree 3 = 3
```

Conclusão:

```text
MshC01 legado não é estritamente manifold.
```

O probe foi corrigido para considerar edge degree 3+ como conectividade válida para análise.

## 10.2 Anatomy probe V1.1

Seed:

```text
12 faces do patch smooth já aprovado
```

Resultados:

### Ring 0

```text
12 faces
13 vertices
todos Bip01 Spine1
0 mixed-bone faces
```

### Ring 1

```text
10 faces
Spine1 + L/R Clavicle
3 mixed-bone faces
```

### Ring 2

```text
12 faces
Spine1 + L/R Clavicle
6 mixed-bone faces
```

### Ring 3

Começa a atingir:

```text
Head
L UpperArm
R UpperArm
```

Definição inicial escolhida:

```text
upper_chest = rings 0..2
```

Total:

```text
34 faces
26 vertices
```

Dessas:

```text
25 faces totalmente Bip01 Spine1
9 faces Spine1 ↔ Clavicle
```

---

# 11. upper_chest_v1 — topologia anatômica controlada

Helper:

```text
pt_c01_high_region_smooth_v1.py
```

Primeira versão deliberadamente conservadora:

```text
subdivide somente as 25 faces 100% Spine1
deixar 9 faces de transição intactas
```

Faces seguras:

```text
[12,13,152,153,154,160,161,162,163,164,165,166,167,
 183,184,188,322,323,324,325,326,327,328,329,330]
```

Faces mistas excluídas:

```text
[3,4,57,59,174,175,190,228,230]
```

Topologia:

```text
unique_region_edges = 43
internal_curved_edges = 32
boundary_linear_edges = 11

MshC01:
173 → 216 vertices
331 → 406 faces

vertex_delta = +43
face_delta   = +75
```

PN displacement:

```text
min  = 0.00813090989
mean = 0.0877605925
max  = 0.179225377
```

Exporter:

```text
output_counts = 216 / 406 / 406
moved_old_vertices = 0
new_vertices = 43
face_delta = +75
max_new_vertex_fit_error = 0.00306129307947
semantic_check = PASS
high_remodel_export_check = PASS
```

Build:

```text
size   = 297737
sha256 = 010f854cd536e799d89aec65b175683da22da8892252031d71ef5630fdf0449c
```

Reimport independente:

```text
MshC01: V=216 F=406 UV=406
global vertices    = 1487
global faces       = 2612
global uv_faces    = 2612
bind_vertices      = 1487
bind_bones_used    = 44
armature_bones     = 95
static_armature_import_check = PASS
```

Runtime install:

```text
INSTALL_UPPER_CHEST_CHECK=PASS
```

Visual:

- diferença claramente perceptível;
- surgiram novos polígonos entre ombro e peito;
- distribuição automática formou uma linha visual pouco anatômica;
- o usuário avaliou que um artista remodelando a região manualmente poderia obter resultado muito melhor.

Conclusão correta:

```text
formato/runtime = sucesso
qualidade anatômica do gerador automático = não é objetivo final
```

O teste cumpriu a função de provar densidade variável e remodelagem regional.

---

# 12. Estado atual do cliente após os testes

Após o teste `upper_chest_v1`, foi executado:

```text
restore-original
```

Resultado:

```text
RESTORE_ORIGINAL_CHECK=PASS
size   = 290229
sha256 = 9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b
```

Portanto o cliente está documentado como:

```text
CURRENT CLIENT BASELINE = PRISTINE MfbC01.smd
```

na última operação registrada.

---

# 13. Segurança operacional consolidada

Todo instalador experimental deve:

```text
1. localizar exatamente um target runtime;
2. validar header;
3. validar size/hash do build aprovado;
4. recusar target desconhecido;
5. criar backup automático;
6. validar SHA/size do backup;
7. instalar via arquivo temporário;
8. verificar arquivo instalado;
9. restaurar automaticamente em caso de falha;
10. oferecer restore-previous / restore-original.
```

Backups:

```text
client_backups/
```

Builds:

```text
pt_build/
```

Logs:

```text
logs/
```

Scripts shell:

```text
sh/
```

Scripts antigos:

```text
history_py/
```

Blends:

```text
blender/
```

Regra permanente:

```text
nenhuma modificação de client sem backup automático verificável
```

---

# 14. Fatos binários consolidados do SMD Model 0.62

ABI i686 medido anteriormente:

```text
sizeof(void*)       = 4
smDFILE_HEADER      = 556
smDFILE_OBJINFO     = 40
smTEXLINK           = 32
smVERTEX            = 24
smFACE              = 36
smTM_ROT            = 20
smTM_POS            = 16
smTM_SCALE          = 16
smOBJ3D             = 2236
```

Offsets relevantes:

```text
smTEXLINK:
    u        = 0
    v        = 12
    hTexture = 24
    NextTex  = 28

smFACE:
    v        = 0
    t        = 8
    lpTexLink= 32

smOBJ3D:
    Vertex   = 4
    Face     = 8
    TexLink  = 12
    Physique = 16

    nVertex  = 84
    nFace    = 88
    nTexLink = 92

    NodeName = 172
    Tm       = 240
```

Empírico em C01:

```text
raw offsets 76 e 80 espelham nVertex/nFace
offset 72 é constante 37 e não é contador
```

Histórico:

```text
face/TexLink pointers
→ base + index * 32
```

Asset C01:

```text
1 TexLink por face
NextTex = 0
```

`smFACE.t` não é UV.

Physique observado no C01:

```text
1 nome de bone de 32 bytes por vertex
```

---

# 15. Normals — estado consolidado

Para a geometria estudada:

```text
BONE_ROW + equal-weight geometric face normals
```

foi a convenção que melhor reproduziu os normals originais.

Erro angular medido anteriormente no HIGH/DEFAULT/LOW:

```text
mean ~1.35°–1.70°
p95  <4.2°
```

Conclusões:

```text
normals são ativas no runtime;
nova topologia precisa de normals coerentes;
novos vertices usam geometric normals transformadas ao espaço local do bone;
```

No remodel V2, vértices antigos movidos preservam o normal legado intencionalmente.

Recomputar normals de old vertices continua sendo uma feature separada, não necessária para provar free topology.

---

# 16. Texturas e materiais — estado anterior que continua válido

A pipeline já possui base prática para:

```text
protected BMP → decode
BMP editado → reprotect → runtime
protected TGA → decode
UV
material assignment
MESH_MTLID / MATERIAL_REF na frente ASE
```

Para a Priestess C01, `MfbC01.bmp` já foi alterado e apareceu em runtime anteriormente.

O importer SMD atual resolve e carrega, entre outros:

```text
MfbC01.bmp
MfbC03.bmp
mo-p-ac.BMP
```

O warning OpenColorIO do Blender Flatpak:

```text
config.ocio 2.5
OpenColorIO 2.4.2
```

continua não bloqueador para:

```text
geometry
UV
materials
texture decode
armature
Physique
deformation
```

---

# 17. Animação — estado anterior que continua válido

Skeleton:

```text
m5.smb
95 nodes
```

Physique C01 usa:

```text
44 bone names
```

Todos existem no skeleton.

Pipeline de animação Blender V2 já validada em:

```text
STAND  MotionInfo[10]
WALK   MotionInfo[16]
RUN    MotionInfo[23]
ATTACK MotionInfo[40]
```

Auxiliares importantes preservados:

```text
Bip weapon01
Bip in01
Bip in-bow
Bip in-cro
```

Conclusão da frente anterior:

```text
animation reverse engineering fundamental
→ suficientemente fechada para automação / Actions / authoring
```

O trabalho futuro principal é writer/export, não redescoberta de STAND/WALK/RUN/ATTACK.

---

# 18. Auditoria de source recebida em Texto.zip

## 18.1 Status desta evidência

Foi recebida uma auditoria textual de aproximadamente 60 KB baseada, segundo o próprio relatório, na árvore atual de source do Summit.

Ela fornece:

```text
arquivo .cpp/.h
função/struct
linhas aproximadas
classificação das conclusões
```

Nesta sessão, a árvore-fonte citada (`atual.tar.xz`) não foi reaberta para confirmar cada linha.

Portanto as conclusões abaixo ficam classificadas como:

**AUDITORIA DA SOURCE**

até validação independente quando necessário.

---

# 19. Free topology — principal descoberta arquitetural

A auditoria afirma que `smOBJ3D::SaveFile()` serializa:

```text
nVertex
nFace
nTexLink
Vertex[nVertex]
Face[nFace]
TexLink[nTexLink]
...
```

e que não existe referência externa encontrada a:

```text
vertex #N
face #N
```

no INX/gameplay.

Também aponta que o próprio caminho ASE pode executar `SortVertex()` e reorganizar/remover vértices.

Conclusão da auditoria:

```text
não é necessário preservar:
    quantidade original de vertices
    índices originais
    ordem dos vertices
    quantidade original de faces
    ordem das faces
```

Portanto, conceitualmente:

```text
173 vertices / 331 faces
```

pode virar:

```text
500 vertices / 900 faces
```

desde que:

```text
contadores
indices
TexLinks
materials
Physique
binary layout
```

fiquem consistentes.

Essa é a descoberta que mais muda a arquitetura.

---

# 20. Limites de geometria indicados pela auditoria

Segundo a auditoria:

```text
RENDER_VERTEX_MAX = 30000
RENDER_FACE_MAX   = 30000
```

Ela referencia:

```text
smrend3d.h
smOBJ3D::BeginRender()
```

Conclusões relatadas:

```text
30.000 vertices por smOBJ3D = hard renderer gate
30.000 faces                = tamanho de face buffer
>30.000 faces               = risco de memória/crash
```

`smFACE` usa:

```text
WORD v[4]
```

Logo os índices são 16-bit, mas o limite útil de renderer seria menor:

```text
30000 vertices
```

Outro limite relatado:

```text
smOBJ3D *obj3d[128]
BYTE TmSort[128]
```

Logo:

```text
128 objects/nodes por smPAT3D
```

Material maximum relatado:

```text
MATRIAL_MAX = 5000
```

Texture stages:

```text
até 8 por face
```

A auditoria também encontrou um `RendBuff` fixo cujo limite prático varia com a quantidade de texture stages.

Conclusão de produto:

```text
não usar 30k como budget artístico;
usar 30k como hard safety ceiling;
criar warnings de performance/buffer antes desse limite.
```

---

# 21. Physique / skinning — limitação estrutural indicada pela auditoria

A auditoria afirma que:

```text
smOBJ3D **Physique
```

é essencialmente:

```text
vertex → 1 bone object
```

No runtime cada vértice usa uma única matriz de bone.

Ela também relata que o parser reconhece:

```text
PHYSIQUE_VERTEXASSIGNMENT_BLENDED_RIGIDTYPE
```

mas mantém apenas o primeiro node e descarta os weights.

Conclusão da auditoria:

```text
weighted skinning real NÃO existe no runtime atual estudado.
```

Modelo de produção esperado:

```text
cada vertex
→ exatamente 1 bone
→ weight 1.0
```

Isso não impede uma superfície contínua de conter triângulos com bones diferentes por vertex:

```text
triangle:
    v0 → Spine1
    v1 → Spine1
    v2 → Clavicle
```

O triângulo continua sendo uma face única; os vértices são transformados individualmente.

Consequência:

```text
"cross-bone topology" não precisa ser um formato especial.
```

O exporter genérico precisa apenas garantir que todo vertex tenha um rigid bone válido.

Weighted painting pode ser usado como ferramenta artística no Blender, mas antes do export teria de ser resolvido para:

```text
dominant bone / rigid assignment
```

ou exigir rigging rígido diretamente.

---

# 22. Texturas / materials — descobertas da auditoria

A auditoria relata suporte da build para extensões como:

```text
BMP
JPG
TGA
PNG
STG
STA
```

Ela não encontrou um hard clamp de 512x512.

Conclusão:

```text
512x512 = não deve ser tratado como limite estrutural universal
```

Testes futuros podem isolar:

```text
512
1024
2048
```

sem misturar com outras alterações.

Recursos de material/texture relatados:

```text
alpha
additive
reflection
self illumination
scroll UV
animated textures
two-sided
opacity
multiple texture stages
```

Também foram citados prefixos/semânticas de animação de textura:

```text
anim2:
anim4:
anim8:
anim16:
```

Essas features ainda precisam ser mapeadas para uma UI Blender própria.

---

# 23. Iluminação e renderização — conclusão arquitetural

Personagens não devem ser tratados como se armazenassem uma cena Blender completa de iluminação.

O modelo fornece essencialmente:

```text
geometry
normals
material
texture
pose/animation
```

e recebe iluminação do renderer/world.

Logo o Blender deve oferecer:

```text
game-like material/lighting preview
```

mas não considerar:

```text
Blender Sun / Point Light / World
```

como dados obrigatórios do arquivo de personagem.

A auditoria descreve uma pipeline antiga CPU-heavy com envio final via Direct3D/`DrawPrimitiveUP`.

Isso reforça:

```text
mais polígonos = custo maior que em uma pipeline GPU moderna
```

mesmo quando abaixo dos hard limits.

---

# 24. Animação — descobertas adicionais da auditoria

A auditoria confirma:

```text
SCENE_TICKSPERFRAME = 160
```

Logo:

```text
1 logical animation frame
=
160 internal ticks
```

Isso confirma o fator já observado empiricamente na nossa pipeline.

Interpolação relatada:

```text
rotation = quaternion SLERP
position = linear
scale    = linear/fixed-point
```

A auditoria conclui que é tecnicamente possível:

```text
Blender Action
→ TmRot
→ TmPos
→ TmScale
→ TmPrevRot
→ TmFrame
→ SMB
```

e produzir motions novas.

Limites relatados:

```text
MotionInfo = até 512
motion segments/frame banks = 32
```

O próximo trabalho é implementar writer/authoring, não provar novamente o playback básico.

---

# 25. Mapas / Stage — nova frente claramente separada

Formato:

```text
SMD Stage data Ver 0.72
```

é distinto de:

```text
SMD Model data Ver 0.62
```

A ferramenta atual **não edita mapas**.

A auditoria relata que Stage 0.72 contém/relaciona:

```text
geometry
faces
UV
materials
TexLinks
normals
vertex colors
lights
StageArea
collision
lightmap / multitexture
```

Terrain seria tratado como mesh no sistema estudado.

Collision estaria ligada à própria geometria/material flags.

Isso torna viável um futuro:

```text
Blender Map Editor
→ stage geometry
→ UV/material
→ collision
→ lightmap
→ lights
→ StageArea
→ SMD Stage 0.72
```

---

# 26. O que NÃO fica apenas no Stage SMD

A auditoria separa vários elementos em outros sistemas, especialmente `sFIELD`:

```text
warps
gates
BGM
ambient sound
sky/day/night
stage object references
field events
spawn-related systems
```

Também relata que parte importante dessa configuração permanece hardcoded em:

```text
field.cpp
```

Consequência:

```text
Map geometry editor
≠
World/field editor completo
```

Uma ferramenta de mapa 3D é viável antes de resolver todo `sFIELD`.

Um world editor completo pode exigir:

```text
editar/refatorar dados hardcoded
```

ou trabalhar com arquivos/configurações adicionais.

---

# 27. Mudança de arquitetura oficial

## Arquitetura antiga de validação

```text
SMD original
→ preservar quase tudo
→ patch mínimo
→ runtime
```

Essa arquitetura foi correta para provar o formato com segurança.

Ela não deve ser a arquitetura final da ferramenta.

## Arquitetura alvo

```text
                    Blender
                       ↓
            Canonical Summit Asset Model
                       ↓
          ┌────────────┼────────────┐
          ↓            ↓            ↓
     SMD Model       SMB          INX
       0.62                         │
          │                         │
          └──────── Character ──────┘

                       +
                       ↓

                 SMD Stage 0.72
                       │
                       ↓
                     Maps
```

Módulos sugeridos:

```text
summit_blender_tools/

io/
    binary
    abi_i686
    smd_model_062
    smb
    inx
    stage_072
    ase

model/
    mesh
    skeleton
    rigid_skin
    animation
    materials
    texlinks

blender/
    importer
    exporter
    armature
    materials
    animation
    validation

character/
    profile discovery
    motion selector
    socket registry
    LOD

map/
    stage
    collision
    lightmap
    placement
    field metadata
```

---

# 28. Estado atual: o que já está realmente resolvido

## Personagem / geometria

```text
SMD parsing                       PROVADO
SMD -> Blender                    PROVADO
bind reconstruction               PROVADO
INX LOD selection                 PROVADO
existing XYZ edit                 PROVADO EM RUNTIME
new vertices                      PROVADO EM RUNTIME
new faces                         PROVADO EM RUNTIME
new TexLinks / UV                 PROVADO EM RUNTIME
new rigid Physique                PROVADO EM RUNTIME
new normals                       PROVADO EM RUNTIME
old+new geometry together         PROVADO EM RUNTIME
variable face counts              PROVADO EM RUNTIME
HIGH topology                     PROVADO EM RUNTIME
safe install/restore              PROVADO
```

## Texturas

```text
protected BMP decode              PROVADO
BMP edit/reprotect/runtime        PROVADO anteriormente
protected TGA decode              PROVADO
material/texture import           PROVADO
UV import                         PROVADO
```

## Skeleton / animação

```text
SMB structure                     PROVADO nos assets estudados
95-node skeleton                  PROVADO
Physique matching                 PROVADO
Blender armature                  PROVADO
STAND                             PROVADO
WALK                              PROVADO
RUN                               PROVADO
ATTACK                            PROVADO
```

---

# 29. O que ainda NÃO está resolvido

```text
full SMD writer sem template                PENDENTE
free-topology runtime final gate            PENDENTE
generic rigid Physique exporter             PENDENTE
generic materials/TexLinks writer           PENDENTE
multi-stage materials UI                    PENDENTE
full SMB writer                              PENDENTE
new animation export runtime                 PENDENTE
INX writer                                   PENDENTE
generic character discovery                  PENDENTE
multi-class/profile automation               PENDENTE
socket validator geral                       PENDENTE
Stage 0.72 importer/exporter                 PENDENTE
StageArea builder                            PENDENTE
map collision editor                         PENDENTE
field metadata editor                        PENDENTE
world editor completo                        PENDENTE
```

---

# 30. Limitações do engine / formato a tratar como gates

Com base na auditoria recebida:

```text
ENGINE / RUNTIME
    rigid skin only: 1 bone per vertex
    max 128 objects/nodes per smPAT3D
    max 30000 render vertices per smOBJ3D
    ~30000 face buffer
    max 8 texture stages
    max 5000 materials
    CPU-heavy transform/skinning/clipping

FORMAT / ABI
    WORD vertex/material indices
    raw 32-bit C++ ABI
    pointer-relative serialization
    fixed-size NodeName/NodeParent buffers
    motion tables/segment limits
```

Esses valores devem virar validações automáticas antes de qualquer writer genérico ser considerado seguro.

---

# 31. Próximo milestone recomendado

## Não continuar com patches anatômicos automáticos

Os testes do torso/peitoral já provaram o que precisávamos:

```text
densidade variável funciona
nova topologia funciona
runtime aceita
```

A anatomia ruim do último patch é um problema de autoria artística, não de formato.

## Próximo componente

```text
PT SMD MODEL 0.62 — FULL MESH WRITER V1
```

Objetivo:

```text
aceitar MshC01 Blender triangular válido
sem exigir:
    173 vertices
    old indices
    old face order
    append-only topology
```

### Primeiro gate de free topology

Para isolar a premissa sem misturar mudança visual:

```text
1. importar MshC01 pristine;
2. reconstruir/reordenar completamente vertices e faces;
3. manter visual geometricamente equivalente;
4. recalcular/recriar:
       UV
       materials
       normals
       rigid Physique
5. escrever novo MshC01;
6. reimport independente;
7. testar animação;
8. instalar com backup;
9. validar no client.
```

Critério de sucesso:

```text
vertex indices totalmente diferentes
face ordering totalmente diferente
+
visual equivalente
+
textura correta
+
animation correta
+
runtime estável
```

Se passar:

```text
FREE TOPOLOGY
=
PROVADA EM RUNTIME
```

A partir daí o artista pode literalmente:

```text
delete
extrude
merge
dissolve
knife
subdivide
retopology
```

e a ferramenta deixa de depender de patches de validação.

---

# 32. Roadmap após o full mesh writer

Ordem recomendada:

## Milestone 1 — Full SMD Model Writer

```text
canonical mesh
→ smVERTEX
→ smFACE
→ smTEXLINK
→ material refs
→ Physique names
→ smOBJ3D
→ object table
→ complete SMD
```

## Milestone 2 — Generic rigid Physique

```text
Blender vertex groups
→ exactly one valid bone per vertex
→ inverse bind
→ local vertex coordinates
→ Physique serialization
```

Opcional de UX:

```text
weighted artist workflow
→ collapse to dominant bone
→ warnings at boundary
```

## Milestone 3 — Generic materials / textures

```text
UV
materials
1..8 texture stages
alpha
reflection
scroll
animated texture
two-sided
self illumination
```

## Milestone 4 — Character profiles

```text
detect:
    SMD
    SMB
    INX
    LOD groups
    skeleton
    textures
    sockets
    motions
```

sem código especial para Priestess.

## Milestone 5 — Animation writer

```text
Blender Action
→ engine ticks
→ TmRot
→ TmPos
→ TmScale
→ TmPrevRot
→ TmFrame
→ SMB
```

## Milestone 6 — INX writer

```text
MotionInfo
State
MotionFrame
ItemCodeList
JobBits
SkillCodeList
EventOffset
LOD
references
```

## Milestone 7 — Package as artist tool

```text
Import Character
Validate
Export
Install
Restore
Animation Preview
Material Preview
LOD Preview
```

## Milestone 8 — Maps

```text
Stage 0.72 importer
Stage writer
collision
materials
lightmaps
StageArea
lights
```

Depois:

```text
field metadata
warps
objects
audio
sky
world configuration
```

---

# 33. Arquivos e organização recomendada neste ponto

Manter como ferramentas ativas enquanto a transição para serializer genérico não termina:

```text
pt_blender_to_smd_high_remodel_v2.py
```

Ferramentas de teste anatômico/topologia podem ser consideradas histórico depois de documentadas:

```text
pt_c01_high_topology_make_patch_v1.py
pt_c01_high_smooth_patch_v1.py
pt_c01_high_remodel_helper_v1.py
pt_c01_high_anatomy_probe_v1.py
pt_c01_high_anatomy_probe_v1_1.py
pt_c01_high_region_smooth_v1.py
pt_blender_to_smd_high_topology_v1_append_safe.py
```

Não apagar:

```text
history_py/
```

porque esses scripts constituem evidência reproduzível dos gates que provaram a pipeline.

Instaladores runtime devem permanecer em:

```text
sh/
```

e builds/hashes em:

```text
pt_build/
client_backups/
logs/
```

---

# 34. Decisões de projeto consolidadas

1. **HIGH continua sendo o LOD principal para inspeção visual de remodelagem.**

2. **Não voltar para ASE quando a rota SMD direta atende o asset/runtime estudado.**

3. **Manter backup/restore como requisito obrigatório de qualquer operação client-write.**

4. **Não enfraquecer gates para fazer um build passar.**
   Se um gate legítimo falhar, corrigir a hipótese/serializer.

5. **Parar de codificar anatomia no exporter.**
   A anatomia pertence ao artista; o exporter deve aceitar a geometria.

6. **Não tratar índices originais como identidade sem necessidade.**
   A arquitetura final deve ser semanticamente consistente, não byte-minimal.

7. **Rigid Physique é o target atual.**
   Weighted skinning real exigiria mudança de engine se a auditoria estiver correta.

8. **Separar personagem e mapa.**
   `SMD Model 0.62` e `SMD Stage 0.72` precisam de serializers próprios.

9. **Separar Stage e Field metadata.**
   Um map editor 3D pode existir antes de um world editor completo.

10. **Usar modelo intermediário canônico.**
    Blender data não deve ser serializado diretamente como se fosse memória C++.

---

# 35. GAP ANALYSIS resumida

## JÁ PROVADO

```text
direct SMD runtime
SMD import
UV/material import
texture decode
static skeleton
Physique mapping
animation playback/bake
existing vertex edit
new topology
new UV/TexLinks
new Physique entries
normals for new topology
combined remodel
variable topology counts
HIGH runtime
backup/restore
```

## AUDITORIA DA SOURCE INDICA COMO VIÁVEL

```text
free topology total
full SMD writer
generic rigid Physique
new animation authoring
SMB writer
INX writer
monster editor
weapon/item editor
Stage 0.72 editor
collision/lightmap editing
```

## FALTA IMPLEMENTAR

```text
canonical asset model
full independent SMD serializer
generic vertex/face rebuild
generic rigid skin exporter
material/TexLink writer
animation writer
INX writer
profile discovery
stage reader/writer
artist-facing addon/UI
```

## LIMITAÇÕES IMPORTANTES

```text
1 bone per vertex
fixed renderer buffers
30k-class object limits
128 nodes/objects
8 texture stages
legacy CPU-heavy renderer
ABI-sensitive raw binary structs
```

## AINDA PRECISA DE VERIFICAÇÃO INDEPENDENTE

```text
cada limite citado pela auditoria da source
exact blended Physique behavior em todas as rotas
practical texture-size ceiling
complete socket taxonomy
all class/monster format variants
full Stage/Field division
```

---

# 36. Ponto exato de retomada

Baseline:

```text
client MfbC01.smd = PRISTINE
size = 290229
sha256 = 9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b
```

Próximo trabalho:

```text
FULL MESH WRITER V1
```

Primeiro objetivo técnico:

```text
reordenar/reconstruir MshC01 completamente
sem mudar sua aparência
```

Primeiro objetivo runtime:

```text
mesma aparência
+
indices totalmente novos
+
faces reordenadas
+
rig/UV/normals reconstruídos
+
client estável
```

Depois desse PASS:

```text
FREE TOPOLOGY RUNTIME = PROVADA
```

e a próxima etapa passa a ser uso artístico real no Blender.

---

# 37. Conclusão

A sequência de testes cumpriu a função de reduzir risco incrementalmente:

```text
1 vertex antigo
→ vários vertices antigos
→ 1 novo vertex
→ várias faces novas
→ HIGH
→ PN smoothing
→ old + new vertices juntos
→ região anatômica maior
→ runtime estável
```

O gargalo não é mais:

```text
"o client aceita nova topologia?"
```

Isso já foi respondido positivamente.

O gargalo agora é:

```text
"conseguimos substituir os serializers de validação por um writer genérico,
sem depender da identidade dos vértices do asset original?"
```

A auditoria de source afirma que sim.

O próximo milestone existe para provar isso diretamente no runtime.

Quando esse milestone passar, o projeto deixa formalmente de ser uma coleção de patches da Priestess e passa a ser a fundação de uma ferramenta de produção de assets Summit Tale para Blender.
