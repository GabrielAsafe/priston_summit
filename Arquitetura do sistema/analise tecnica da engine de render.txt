Sim. Com a source antiga e atual que você enviou, dá para responder uma parte muito grande dessas 51 perguntas diretamente pelo código, sem depender de tentativa visual. Vou usar os quatro estados que você pediu: **PROVADO PELA SOURCE**, **PROVADO POR ASSET**, **HIPÓTESE** e **NÃO ENCONTRADO**. A numeração abaixo acompanha exatamente o roteiro que você mandou. 

As linhas abaixo se referem à árvore atual de `atual.tar.xz`; se você editar a source depois, naturalmente elas podem se deslocar.

# Conclusão principal antes dos detalhes

A arquitetura permite chegar bem mais longe do que apenas um “importador que altera alguns vértices”.

**É tecnicamente viável construir um editor Blender de produção que:**

* importe e recrie completamente meshes;
* altere livremente quantidade e ordem de vértices/faces;
* refaça UVs e materiais;
* refaça todo o Physique;
* mantenha ou modifique skeletons;
* crie animações novas;
* escreva `SMD Model 0.62`, `SMB` e `INX`;
* edite monstros e armas sobre o mesmo núcleo;
* leia/escreva `SMD Stage 0.72`;
* edite colisão e materiais do mapa.

As grandes restrições que já estão **provadas** são:

1. **O skinning real do engine é rígido: 1 bone por vértice.**
2. **`smPAT3D` suporta no máximo 128 nodes/objects.**
3. **Um `smOBJ3D` não pode ser renderizado com mais de 30.000 vértices.**
4. **30.000 faces é também o tamanho do buffer de faces, mas não há proteção contra overflow.**
5. **`smFACE` usa `WORD` para índices.**
6. **Há no máximo 8 texture stages/TexLinks úteis por face no renderer.**
7. **O formato binário grava structs C++ inteiras, inclusive ponteiros; portanto a serialização é ABI-sensitive.**
8. **Stage e Model são formatos diferentes, apesar da extensão `.smd`.**
9. **Warps, música, céu, ambient sound e vários metadados do mapa NÃO ficam no Stage SMD.**
10. **Weighted skinning do Blender precisará ser convertido para rigid skinning**, a menos que modifiquemos o engine.

---

# 1. Limites reais de geometria

**Status: PROVADO PELA SOURCE.**

### Vértices por `smOBJ3D`

O limite efetivo do renderer é:

```cpp
#define RENDER_VERTEX_MAX 30000
#define RENDER_FACE_MAX   30000
```

em:

`SrcGame/src/smLib3d/smrend3d.h:46-50`

E `smOBJ3D::BeginRender()` faz:

```cpp
if(nVertex > RENDER_VERTEX_MAX)
    return FALSE;
```

em:

`smObj3d.cpp:898-913`

Portanto:

**máximo renderizável por `smOBJ3D`: 30.000 vértices.**

Esse limite é menor que o limite teórico dos índices `WORD`.

### Faces

`smFACE`:

```cpp
struct smFACE
{
    WORD v[4];
    smFTPOINT t[3];
    smTEXLINK *lpTexLink;
};
```

`smType.h:203-210`

`v[0..2]` são os três vértices; `v[3]` é o material.

Logo, índice de vértice é 16-bit:

```text
0 ... 65535
```

Mas no modelo isso não é o limite útil porque o renderer para em 30.000 vértices.

Para faces existe:

```cpp
#define RENDER_FACE_MAX 30000
```

O problema é pior: `AddObjectFace()` não testa o limite:

```cpp
RendFace[nRendFace]...
nRendFace++;
```

`smRend3d.cpp:1533-1552`

Então:

* `<= 30.000`: dentro do buffer;
* `> 30.000`: **risco real de corrupção de memória/crash**.

### `AddVertex` e `AddFace`

Não há proteção de capacidade:

`smObj3d.cpp:293-340`

```cpp
Vertex[nVertex]...
nVertex++;
```

e:

```cpp
Face[nFace]...
nFace++;
```

Por isso o exporter deve validar tudo antes de escrever.

### Outro limite escondido: buffer final de triângulos

Existe:

```cpp
float RendBuff[64 * 16384];
```

`smRend3d.cpp:234`

Isso dá 4 MiB.

O tamanho de cada vértice enviado ao D3D depende do número de texture stages:

```cpp
D3DFVF_SIZE[9] =
{
    0x18, 0x20, 0x28, 0x30, 0x38,
    0x40, 0x48, 0x50, 0x58
};
```

`smRend3d.cpp:248-258`

Consequentemente, se todas as faces caírem em um único material, aproximadamente:

| Texture stages | Triângulos que cabem no `RendBuff` |
| -------------: | ---------------------------------: |
|              0 |                            ~58.254 |
|              1 |                            ~43.690 |
|              2 |                            ~34.952 |
|              3 |                            ~29.127 |
|              4 |                            ~24.966 |
|              5 |                            ~21.845 |
|              6 |                            ~19.418 |
|              7 |                            ~17.476 |
|              8 |                            ~15.887 |

Então um objeto com 30 mil faces e material com muitas camadas pode estourar o buffer **antes** do `RENDER_FACE_MAX`.

### Objetos por `smPAT3D`

```cpp
smOBJ3D *obj3d[128];
BYTE TmSort[128];
```

`smObj3d.h:256-283`

`AddObject()` também rejeita acima disso.

**Limite: 128 objects/nodes.**

### Materiais

```cpp
#define MATRIAL_MAX 5000
```

`smTexture.h:167`

E o renderer possui arrays do mesmo tamanho.

**Limite runtime: 5.000 materiais.**

Isso é absurdamente maior que o necessário para um personagem, mas é um limite verdadeiro.

### TexLinks

`AddTexLink()` percorre:

```cpp
for(cnt = 0; cnt < 8; cnt++)
```

`smObj3d.cpp:352-397`

E o renderer possui FVF até `TEX8`.

**Máximo funcional: 8 camadas por face.**

O construtor normal do modelo inicialmente reserva:

```text
nFace × 2 TexLinks
```

enquanto o Stage reserva:

```text
nFace × 4
```

Isso é limite do caminho de construção/conversão ASE, não necessariamente da representação binária final, porque no load o engine aloca exatamente `nTexLink`.

### Low-poly é obrigatório?

**Parcialmente.**

Não existe requisito de que o modelo tenha, por exemplo, “menos de 2.000 polígonos”.

Mas a arquitetura é pesada:

```text
CPU animation
→ CPU rigid skinning
→ CPU transform
→ CPU lighting
→ CPU clipping
→ montagem de triângulos
→ DrawPrimitiveUP
```

Então aumentar geometria custa muito mais que em uma engine moderna.

**Low-poly é simultaneamente uma questão de performance e de buffers fixos.**

---

# 2. Topologia completamente livre

**Status: PROVADO PELA SOURCE.**

Você **não precisa preservar**:

* quantidade original de vértices;
* índices originais;
* ordem dos vértices;
* quantidade original de faces;
* ordem das faces.

O arquivo salva:

```cpp
nVertex
nFace
nTexLink
```

e depois grava os arrays correspondentes.

`smOBJ3D::SaveFile()`:

`smObj3d.cpp:1922-1956`

```cpp
WriteFile(... sizeof(smOBJ3D));
WriteFile(... Vertex, sizeof(smVERTEX) * nVertex);
WriteFile(... Face, sizeof(smFACE) * nFace);
WriteFile(... TexLink, sizeof(smTEXLINK) * nTexLink);
```

Logo:

```text
173 vertices / 331 faces
```

pode virar:

```text
500 vertices / 900 faces
```

sem que o engine espere os antigos.

### Algo externo referencia vertex index?

**NÃO ENCONTRADO.**

O INX trabalha com:

* modelo;
* skeleton/motion;
* node names;
* motion ranges;
* item codes;
* job codes;
* skill codes.

Não encontrei uma estrutura externa de gameplay que diga:

```text
vertex #123
face #456
```

O principal dado semanticamente associado a cada vértice é o **Physique**.

### Atenção ao conversor ASE

O caminho ASE chama `SortVertex()`, que:

* remove vértices não usados;
* agrupa alguns duplicados;
* atualiza faces;
* atualiza Physique.

Portanto até o próprio conversor oficial **não considera a ordem original dos vértices sagrada**.

Isso é uma forte evidência de free topology.

---

# 3. Physique / skinning

**Status: PROVADO PELA SOURCE.**

Aqui temos uma conclusão definitiva:

## O runtime suporta 1 bone por vertex.

A estrutura é:

```cpp
smOBJ3D **Physique;
```

`smObj3d.h:88-93`

É literalmente:

```text
vertex 0 → ponteiro para bone
vertex 1 → ponteiro para bone
vertex 2 → ponteiro para bone
...
```

Não existe:

```cpp
weight[4]
boneIndex[4]
```

ou equivalente.

No runtime:

`smObj3d.cpp:1650-1709`

```cpp
pV = &Vertex[cnt];
mMat = &Physique[cnt]->mWorld;

rx = pV->x * mMat->_11 + ...
```

Cada vértice usa **uma única matriz**.

## E o Physique blended do ASE?

O parser reconhece:

```text
PHYSIQUE_VERTEXASSIGNMENT_BLENDED_RIGIDTYPE
```

mas não implementa o blending.

`smRead3d.cpp:1573-1590`

Ele lê:

```cpp
*PHYSIQUE_VERTEXASSIGNMENT_NODE
```

e só faz assignment quando:

```cpp
if(x == 0)
{
    obj->Physique[pNum] =
        smPhysique->GetObjectFromName(strBuff);
}
```

Ou seja:

**se um ASE contém múltiplos bones/pesos, só o primeiro node é preservado.**

Os pesos são descartados.

### Portanto:

```text
bone A = 50%
bone B = 50%
```

**não existe no runtime atual.**

Para suportar isso de verdade seria necessário alterar:

* `smOBJ3D`;
* formato SMD;
* loader;
* WorldForm;
* provavelmente renderer/converter.

### Como cotovelo/ombro/joelho funcionam?

Cada vértice pertence rigidamente a uma peça do skeleton.

Um triângulo pode ter:

```text
v0 → UpperArm
v1 → UpperArm
v2 → Forearm
```

e cada vértice é transformado individualmente.

A GPU depois interpola a superfície do triângulo.

Isso produz deformação dura/low-poly, mas **não necessariamente rachadura**.

### Existem vértices duplicados nas juntas?

**NÃO PROVADO PELA SOURCE.**

A engine permite tanto compartilhados quanto duplicados.

Para responder especificamente se os modelos originais duplicam vértices em:

```text
Spine/Clavicle
UpperArm/Forearm
Pelvis/Thigh
```

precisamos medir um asset concreto e procurar posições coincidentes com bone assignments diferentes.

---

# 4. Criação de novos vértices e rigging

**Status: PROVADO PELA SOURCE.**

Sim: um novo vértice pode receber qualquer `NodeName` existente no skeleton.

No load:

`smObj3d.cpp:2022-2031`

```cpp
Physique[cnt] =
    PatPhysique->GetObjectFromName(
        szBuff + cnt * 32
    );
```

O arquivo grava **32 bytes de nome de bone por vértice**.

Não existe bone ID rígido.

### Restrição real

O nome precisa existir.

Se:

```cpp
GetObjectFromName(...)
```

retornar `NULL`, o runtime posteriormente faz:

```cpp
Physique[cnt]->mWorld
```

e pode crashar.

O exporter deve obrigatoriamente validar:

```text
cada vertex group
→ bone existe no skeleton
```

### Só dar o nome basta?

Não completamente.

O engine usa `ReformPhysique()` para converter a posição do vértice para o espaço local do bone/bind pose.

Então um exporter precisa fazer o equivalente:

```text
vertex Blender/world/object
→ inverse bind matrix do bone
→ coordenada local para SMD
```

Não podemos apenas mudar a string do bone e manter as coordenadas antigas.

### Pode refazer todo o rig?

**Sim.**

Desde que:

* todos os bones existam;
* cada vértice receba exatamente um bone;
* as coordenadas sejam recalculadas para o bind correspondente.

Não encontrei outro cache externo de Physique.

---

# 5. Skeleton

**Status: PROVADO PELA SOURCE.**

`SMB` não é uma estrutura completamente diferente do SMD model.

`smASE_ReadBone()`:

`smRead3d.cpp:2072-2151`

lê o ASE e só mantém:

```cpp
if(NodeName começa com "Bip")
    pat->AddObject(obj);
```

Depois salva com:

```cpp
pat->SaveFile(...)
```

E `SaveFile()` usa:

```text
SMD Model data Ver 0.62
```

Portanto:

> **SMB é essencialmente um `smPAT3D` Model 0.62 usado como skeleton/animation bank.**

Ele contém nodes `smOBJ3D` com:

* `NodeName`;
* `NodeParent`;
* bind transforms;
* rotation;
* position;
* scale;
* animation tracks.

### Relação

```text
INX
 ├─ szModelFile     → mesh SMD/ASE
 ├─ szMotionFile    → SMB/ASE skeleton+motions
 ├─ szSubModelFile
 └─ MotionInfo[]
```

`character.cpp:544-641` mostra exatamente esse fluxo.

### Pode adicionar bone?

**Sim, estruturalmente.**

### Remover bone?

Sim, desde que nenhum:

* vertex Physique;
* attachment;
* efeito;
* animation track necessário

continue referenciando-o.

### Renomear bone?

O formato aceita, mas vários nomes são hardcoded em C++.

Exemplos:

`character.cpp:10444-10609`

```text
Bip01 L Finger1
Bip01 R Finger1
Bip01 L UpperArm
Bip01 R UpperArm
bip01 weapon
Bip01 staff02
Bip01 Effect
Bip01 R Hand
```

`sinAssaSkillEffect.cpp:58`:

```cpp
GetObjectFromName("Bip weapon01");
```

E há muitos outros em `HoBaram/AssaParticle.cpp`.

Portanto:

**bones puramente deformadores podem ser renomeados se todas as referências forem atualizadas; socket bones hardcoded não devem ser renomeados sem mudar o cliente.**

### Maximum bones/nodes

```cpp
smOBJ3D *obj3d[128];
```

**128 nodes por `smPAT3D`.**

### Os 95 nodes de `m5.smb`

**PROVADO POR ASSET**, usando a contagem que você já levantou: 95 < 128.

Mas não podemos dizer que todos os 95 possuem animation keys.

Qualquer `smOBJ3D` pode ter tracks; isso não significa que todo node do asset tenha keys não vazias.

### Deform/helper/socket/effect

Não existe uma enum formal distinguindo-os.

A semântica é:

* **deform bone:** referenciado por `Physique`;
* **attachment/socket:** procurado por `GetObjectFromName`;
* **effect bone:** mesma coisa, mas usado por efeito;
* **helper:** node que existe na hierarquia mas não é usado como Physique;
* **weapon bone:** attachment reconhecido por nome/código.

---

# 6. Animação

**Status: PROVADO PELA SOURCE.**

O pipeline real é aproximadamente:

```text
INI
 ↓
smMODELINFO / MotionInfo
 ↓
INX
 ↓
MotionFrame
 ↓
SMB / smPAT3D
 ↓
TmFrame[]
 ↓
TmRot / TmPos / TmScale
 ↓
TmPrevRot
 ↓
TmAnimation
 ↓
mWorld por bone
 ↓
Physique
 ↓
render
```

### 160

A origem é explícita:

`smRead3d.cpp:1110`

```cpp
#define SCENE_TICKSPERFRAME 160
```

ASE:

```cpp
AseSceneLastFrame =
    atoi(frame) * 160;
```

E `character.cpp` faz:

```cpp
frame = MotionInfo->StartFrame * 160;
```

`character.cpp:2342`

Portanto:

**1 frame de animação lógico = 160 ticks internos.**

Não significa 160 FPS. É uma escala temporal interna.

### Rotation

```cpp
struct smTM_ROT
{
    int frame;
    float x,y,z,w;
};
```

O ASE inicialmente fornece axis-angle.

O converter transforma em quaternion.

Também constrói:

```cpp
TmPrevRot[]
```

como acumulado das rotações anteriores.

### Interpolação

`smObj3d.cpp:919-977`

usa:

```cpp
D3DMath_QuaternionSlerp(...)
```

para rotation.

Position:

`GetPosFrame():981-1014`

usa interpolação linear.

Scale:

`GetScaleFrame():1017-1040`

usa interpolação linear fixed-point de 8 bits.

### É possível criar animação nova no Blender?

**Sim.**

O exporter precisará produzir, por node:

```text
TmRot[]
TmPos[]
TmScale[]
TmPrevRot[]
TmRotCnt
TmPosCnt
TmScaleCnt
TmFrame[]
TmFrameCnt
```

e no INX:

```text
StartFrame
EndFrame
MotionFrame
State
EventFrame
Repeat
ItemCodeList
JobBits
SkillCodeList
```

### Limite de keys

O formato em si usa contadores `int`, mas o **conversor ASE** usa temporários fixos:

```cpp
smTM_ROT   TmRot[4096];
smDTM_ROT  TmDRot[4096];
smTM_POS   TmPos[4096];
smTM_SCALE TmScale[4096];
```

`smRead3d.cpp:1149-1152`

Sem bounds check.

Portanto:

**máximo seguro pelo conversor ASE: <4096 keys de cada tipo por node.**

Um serializer SMD direto não é tecnicamente preso a esse array, embora runtime e memória continuem impondo limites práticos.

### Motions

```cpp
#define MOTION_INFO_MAX 512
```

**512 MotionInfo por INX.**

Motion files/segments:

```cpp
#define MOTION_LIST_MAX 32
#define OBJ_FRAME_SEARCH_MAX 32
```

O merge usa:

```cpp
smFRAME_POS TmFrame[32];
```

Então **32 bancos/segments de motion é o hard limit seguro**.

### `M5-motion1..7.smb`

**NÃO ENCONTRADO pelo nome na source.**

O código não atribui significado especial a suffix `motion1`, `motion2`, etc.

Ele possui um mecanismo genérico de merge de múltiplos motion files.

Para dizer exatamente:

```text
M5-motion1 = stand/walk?
M5-motion2 = combat?
...
```

precisamos abrir o INX correspondente e mapear `MotionFrame` + ranges.

---

# 7. Eventos de animação

**Status: PROVADO PELA SOURCE.**

Não encontrei um campo chamado literalmente `EventOffset`.

O que existe é:

```cpp
DWORD EventFrame[4];
```

`smType.h:488-513`

O INI converter faz:

`fileread.cpp:210-269`

```cpp
EventFrame[n] =
    (eventFrame - StartFrame) * 160;
```

Então eventos são **offsets relativos ao início da motion, em ticks internos**.

O runtime compara o frame atual contra esses valores em vários pontos de `character.cpp`.

Eles acionam coisas como:

* ataque;
* skill;
* dano;
* efeitos;
* som.

### Evento fica onde?

**No INX/`smMOTIONINFO`, não no SMB.**

SMB possui o movimento.

INX diz:

```text
qual intervalo
qual estado
em que ponto do intervalo acontece o evento
```

### Pode mudar duração?

Sim, mas é necessário atualizar:

* `StartFrame`;
* `EndFrame`;
* `EventFrame[]`;
* possíveis offsets especiais hardcoded para determinadas skills.

São **até 4 eventos genéricos por motion**.

---

# 8. Seleção de animação

**Status: PROVADO PELA SOURCE.**

`smMOTIONINFO` contém:

```cpp
State
StartFrame
EndFrame
EventFrame[4]

ItemCodeCount
ItemCodeList[104]

dwJobCodeBit
SkillCodeList[8]

MapPosition
Repeat
KeyCode
MotionFrame
```

`smType.h:488-513`

A seleção principal está em:

`character.cpp:2414+`

```cpp
smCHAR::SetMotionFromCode()
```

Ele procura MotionInfo cujo:

```text
State == MotionCode
```

e depois filtra por:

* village/map position;
* job bit;
* skill;
* item/weapon type.

Portanto duas animações `ATTACK` podem ser diferenciadas por arma, job ou skill.

### Blender pode listar automaticamente?

**Sim.**

O addon pode ler o INX e apresentar:

```text
STAND
WALK
RUN
ATTACK
SKILL
DAMAGE
DEAD
...
```

com cada variante e seus filtros.

Isso é uma parte relativamente bem definida do projeto.

---

# 9. LOD

**Status: PROVADO PELA SOURCE.**

`smMODELINFO`:

```cpp
_MODELGROUP HighModel;
_MODELGROUP DefaultModel;
_MODELGROUP LowModel;
```

Cada grupo:

```cpp
char szModelName[4][16];
```

máximo **4 node names**.

O client escolhe em:

`character.cpp:8141-8147`

```cpp
DefaultModel;

if(dDist < 36864)
    HighModel;

if(dDist > 102400)
    LowModel;
```

Esses valores são unidades internas da métrica `dDist`; eu não os converteria para metros sem analisar a função que gera `dDist`.

### LOD é outro SMD?

Não necessariamente.

Ele usa:

```cpp
ChoiseRenderD3D(...)
```

para renderizar **determinados objects/nodes dentro do Pattern**.

Portanto a relação principal é por **nome de objeto**, não por índice de vértice.

### Topologia dos LODs

Pode ser completamente diferente.

Não existe relação:

```text
high.vertex[42] ↔ low.vertex[42]
```

### Mesmos bones?

Se a mesh for skinned, os Physique node names precisam existir no skeleton usado.

Mas as geometrias não precisam corresponder.

### Precisa dos três?

Não.

Se High ou Low não têm nomes configurados, o default permanece.

---

# 10. Normals e smoothing

**Status: PROVADO PELA SOURCE.**

`smVERTEX`:

```cpp
int x,y,z;
int nx,ny,nz;
```

`smType.h:197-201`

O engine possui:

```cpp
smOBJ3D::SetGNormals()
```

`smObj3d.cpp:511-579`

Ele calcula cross product de cada face, soma aos vértices e faz média.

Depois grava normal como um **endpoint relativo à posição**:

```cpp
Vertex[cnt].nx += Vertex[cnt].x;
Vertex[cnt].ny += Vertex[cnt].y;
Vertex[cnt].nz += Vertex[cnt].z;
```

### O ASE importa normals?

**NÃO.**

Não encontrei parser para:

```text
*MESH_VERTEXNORMAL
*MESH_FACENORMAL
*MESH_SMOOTHING
```

Então o caminho oficial ASE → SMD recalcula os normals.

### Smoothing groups?

**NÃO ENCONTRADO no SMD.**

Não há campo de smoothing group em `smVERTEX` ou `smFACE`.

Portanto o efeito de smooth/hard edges precisa ser representado pela topologia:

* compartilhar vertex → normal média;
* duplicar vertex → normals independentes.

### Normals durante skinning

Sim, elas são transformadas pela mesma matriz do bone.

`smObj3d.cpp:1691-1705`

### Afetam o quê?

Provadamente:

* iluminação;
* reflection mapping;
* vertex shading.

Backface culling é controlado por winding/cull mode, não pelo normal armazenado.

### Exporter precisa gravar normals?

Se escrever SMD diretamente, sim, ou executar algoritmo equivalente ao `SetGNormals()`.

---

# 11. UV / `smTEXLINK`

**Status: PROVADO PELA SOURCE.**

```cpp
struct smTEXLINK
{
    float u[3], v[3];
    DWORD *hTexture;
    smTEXLINK *NextTex;
};
```

`smType.h:190-195`

Cada TexLink contém UV para os três corners da face.

### Um por face?

Não.

`NextTex` forma uma linked list:

```text
Face
 ↓
TexLink 0
 ↓
TexLink 1
 ↓
TexLink 2
 ...
```

O renderer percorre até 8.

Logo uma face pode ter múltiplos UV sets/camadas.

### Uso

Isso permite coisas como:

* diffuse;
* opacity;
* reflection;
* scrolling;
* overlay;
* lightmap;
* multitexturing.

### Quantidade

**Até 8 texture stages funcionais.**

---

# 12. Materiais

**Status: PROVADO PELA SOURCE.**

A estrutura real está em:

`smType.h:669-712`

```cpp
smMATERIAL
```

Tem:

* 8 textures;
* 8 stage operations;
* 8 texture-coordinate/form states;
* blend;
* shade;
* two-side;
* diffuse;
* transparency;
* self illumination;
* mesh state;
* animation texture;
* até 32 animated texture handles.

### O que o ASE realmente importa?

`smRead3d.cpp:274+`

Importa:

* diffuse color;
* transparency;
* self illumination;
* two-sided;
* diffuse textures;
* opacity map;
* UV offsets;
* tiling;
* UV angle;
* material scripts.

Não encontrei importação funcional de:

* specular color;
* ambient color;
* shininess/gloss;
* smoothing properties.

Portanto não vale expor esses campos como propriedades nativas importantes do formato se o objetivo é round-trip fiel.

### Blend modes

Source define:

```text
NONE
ALPHA
COLOR
SHADOW
LAMP
ADDCOLOR
INVSHADOW
```

E scripts ASE:

```text
BLEND_ALPHA:
BLEND_COLOR:
BLEND_ADDCOLOR:
BLEND_SHADOW:
BLEND_LAMP:
```

### Material scripts

Também existem:

```text
wind:
anim2:
anim4:
anim8:
anim16:
wind_z1:
wind_z2:
wind_x1:
wind_x2:
water:
wall:
pass:
notpass:
render_latter:
BLINK_COLOR:
ice:
orgwater:
```

`smRead3d.cpp:216-250`

Isso é muito importante para o editor de mapas.

---

# 13. Texturas

**Status: PROVADO PELA SOURCE, com uma exceção.**

`GetTextureFileType()`:

`smTexture.cpp:1490-1535`

reconhece:

```text
BMP
JPG
TGA
PNG
STG
STA
```

`STG` é tratado como JPG.

`STA` como TGA.

Há inclusive “decrypt”/de-obfuscation:

`DecryptBMP():1538-1557`

e:

`DecryptTGA():1560-1574`

São basicamente formatos normais cujo header foi alterado matematicamente.

### PNG

Há um bug interessante:

```cpp
if(extension == png)
    return smTEXFILETYPE_TGA;
```

`smTexture.cpp:1523-1526`

Então a intenção de suporte PNG existe e D3DX consegue ler, mas a função de classificação está errada.

### DDS

**NÃO ENCONTRADO como extensão explicitamente suportada.**

D3DX9 por si só suporta DDS, mas o código do jogo não classifica `.dds` em `GetTextureFileType()`.

Portanto eu não prometeria DDS sem um teste.

### 512×512

**NÃO é limite de source.**

A criação usa:

```cpp
D3DXGetImageInfoFromFileInMemory(...)
```

e:

```cpp
D3DXCreateTextureFromFileInMemoryEx(
    ...,
    ImageInfo.Width,
    ImageInfo.Height,
    ...
)
```

Não encontrei clamp 512.

Portanto 1024/2048 podem funcionar se a GPU/D3D9 suportar.

O limite verdadeiro é:

* `D3DCAPS9.MaxTextureWidth/Height`;
* VRAM/memória;
* performance.

### Power-of-two

Não encontrei exigência no loader.

Em hardware D3D9 antigo isso poderia depender do device caps; não do formato SMD.

### Alpha

TGA/PNG e opacity map passam por D3DX e o renderer usa alpha blending/alpha testing.

Logo transparência parcial é suportada.

---

# 14. Texturas animadas

**Status: PROVADO PELA SOURCE.**

Existem:

```text
anim2:
anim4:
anim8:
anim16:
```

O parser gera sequências de nomes de arquivo e chama:

```cpp
ChangeMaterialToAnimation(...)
```

`smTexture.cpp:877-927`

O material possui:

```cpp
smAnimTexture[32];
AnimTexCounter;
FrameMask;
Shift_FrameSpeed;
AnimationFrame;
```

Portanto textura animada é feature formal do **material**.

Além disso UVs podem ser modificados em runtime com:

```text
FS_SCROLL
FS_SCROLL2..10
FS_SCROLLSLOW...
FS_REFLEX
```

Então o Blender Material UI ideal deveria ter propriedades próprias do Summit, não tentar mapear tudo só para Principled BSDF.

---

# 15. Iluminação de personagens

**Status: PROVADO PELA SOURCE.**

O SMD de personagem guarda:

* geometria;
* normals;
* materiais.

Não encontrei lights armazenadas como parte do personagem.

A iluminação vem do renderer/runtime.

`smRENDER3D` possui:

```cpp
#define REND_LIGHT_MAX 64
```

e sistemas de dynamic lights.

Os normals entram no cálculo de iluminação no CPU.

Portanto:

> **o Blender importer não precisa importar lights do personagem.**

A ferramenta deve criar apenas uma iluminação de preview própria.

---

# 16. Pipeline de renderização

**Status: PROVADO PELA SOURCE.**

O fluxo real é:

```text
smPAT3D
→ TmAnimation
→ hierarchy matrices
→ smOBJ3D::WorldForm
→ rigid Physique transform
→ world vertices/normals
→ smRENDER3D
→ lighting
→ clipping
→ material sorting
→ texture stages
→ DrawPrimitiveUP
→ Direct3D 9
```

### CPU vs GPU

CPU:

* animation evaluation;
* skeleton hierarchy;
* skinning;
* world transform;
* normals;
* lighting em grande parte;
* clipping;
* assembly de render vertices;
* sorting.

GPU:

* texture sampling;
* texture-stage operations;
* blending;
* rasterization.

O engine faz:

```cpp
DrawPrimitiveUP(D3DPT_TRIANGLELIST,...)
```

`smRend3d.cpp:4205+`

Não é uma pipeline moderna de static vertex/index buffers.

Isso explica por que aumentar polígonos custa bastante.

---

# 17. Transparência, brilho e efeitos

**Status: PROVADO PELA SOURCE.**

Materiais implementam:

* alpha;
* additive;
* color blending;
* shadow;
* lamp-like additive;
* self illumination;
* reflection UV;
* animated textures;
* scrolling textures;
* two-sided rendering.

`FS_REFLEX` calcula coordenada de reflection usando view vector + normal em:

`smRend3d.cpp:3322-3350`

Partículas, entretanto, são um sistema separado:

```text
HoBaram/
HoBaram/NewEffect/
particle.*
```

Então:

```text
material glow/reflection → material system
particle/fire/smoke/skills → effect system
```

Um exporter pode recriar os materiais, mas não automaticamente toda lógica de particle effects.

---

# 18. Attachments

**Status: PROVADO PELA SOURCE.**

Armas e efeitos usam bones por **nome**.

Exemplos:

`character.cpp:10444+`

```text
Bip01 L Finger1
Bip01 R Finger1
Bip01 L UpperArm
Bip01 R UpperArm
Bip01 R Hand
Bip01 Effect
bip01 weapon
```

`sinAssaSkillEffect.cpp:58`:

```text
Bip weapon01
```

O padrão é:

```cpp
GetObjectFromName(...)
```

e depois:

```cpp
LinkParentObject(...)
```

### Pode criar attachment point novo?

O formato permite criar um node novo.

Mas o cliente **não saberá automaticamente para que ele serve**.

Para um novo socket ser usado por gameplay/effect, algum código precisa procurar o nome dele.

Então:

* adicionar node: possível sem alterar engine;
* fazê-lo ter nova semântica: normalmente exige configurar/modificar código.

---

# 19. Heads, cabelos, armaduras e partes

**Status: PARCIALMENTE PROVADO PELA SOURCE.**

O sistema principal de personagem suporta:

```text
Pattern
Pattern2
```

e `smMODELINFO` possui:

```cpp
szModelFile
szSubModelFile
```

O loader carrega ambos.

`character.cpp:846-859`, `1004-1027`.

Isso é a base para model/submodel composition.

Armas e escudos são patterns separados ligados aos bones.

`ChoiseRenderD3D()` também permite mostrar somente determinados `smOBJ3D` internos.

### Podemos substituir só torso/braço/perna?

Se essas partes já forem nodes/objects independentes, sim.

Se um personagem original for uma única mesh monolítica, não existe um “slot torso” mágico no SMD.

Mas você pode recriar o SMD com vários `smOBJ3D`, respeitando o limite de 128.

---

# 20. Monstros

**Status: PROVADO PELA SOURCE.**

Monstros passam pelo mesmo núcleo:

```text
smCHAR
smMODELINFO
smPAT3D
smOBJ3D
SMD 0.62
SMB
Physique
materials
motions
```

Não encontrei um “monster mesh format” alternativo.

Existem diferenças de gameplay e efeitos, e vários monstros têm socket names particulares.

Mas o **core Blender importer/exporter pode ser o mesmo.**

Monster Editor: **VIÁVEL COM A SOURCE ATUAL.**

---

# 21. Armas e objetos

**Status: PROVADO PELA SOURCE.**

Itens usam `smPAT3D`/ASE/SMD e são carregados como patterns.

`scITEM::SetPattern()` aparece em `character.cpp`.

São normalmente objetos rígidos ligados a mão/socket.

Eles podem possuir hierarquia/animação se o asset quiser; não existe obrigação de Physique.

Materials/UV usam exatamente o mesmo sistema.

Portanto:

> **Weapon/Item Editor pode compartilhar o mesmo mesh/material importer/exporter.**

---

# 22. Mapas / `SMD Stage data Ver 0.72`

**Status: PROVADO PELA SOURCE.**

Importantíssimo:

```text
SMD Model data Ver 0.62
```

e:

```text
SMD Stage data Ver 0.72
```

**não são o mesmo formato.**

O header Stage está em:

`smStage3d.cpp:2196`

```cpp
"SMD Stage data Ver 0.72"
```

A classe:

`smStage3d.h:12-167`

possui:

```text
StageArea[256][256]
Vertex
Face
TexLink
smLight
MaterialGroup
nVertex
nFace
nTexLink
nLight
vertex colors
lighting vector
area buffer
map rectangle
```

O save grava:

```text
header
raw smSTAGE3D
material group
stage vertices
stage faces
TexLinks
lights
StageArea lists
```

`smStage3d.cpp:2199-2269`.

### Stage SMD contém

**PROVADO:**

* geometry;
* vertex positions;
* faces;
* material ID;
* TexLinks/UV;
* materials;
* face normals;
* vertex color;
* lights;
* spatial cell/face lists;
* collision semantics via material;
* lightmap UV/layers quando convertidos.

### NÃO fica no Stage SMD

* warps;
* gates;
* spawn definitions;
* field name;
* BGM;
* ambient sounds;
* sky/day/night;
* stage object file lists;
* vários field events.

Esses pertencem principalmente a `sFIELD` e outros sistemas.

---

# 23. Limites de mapa

**Status: PROVADO PELA SOURCE.**

### Vertex index

`smSTAGE_FACE`:

```cpp
WORD Vertex[4];
```

Então um Stage individual só pode indexar efetivamente:

**65.536 vertices.**

### Faces

Não encontrei um limite de arquivo fixo equivalente a 30k.

O Stage pode conter muito mais faces porque só adiciona ao renderer as regiões visíveis.

Mas em um frame:

```text
RendFace <= 30000
RendVertex <= 30000
```

continua valendo.

### Spatial grid

```cpp
#define MAP_SIZE 256
#define SizeMAPCELL 64
```

`smType.h:5`, `30`

`SetupPolyAreas()` usa:

```cpp
x >> 6
z >> 6
```

e depois:

```cpp
& 0xFF
```

Ou seja:

**256 × 256 células de 64 unidades.**

O índice da grid se repete a cada:

```text
256 × 64 = 16.384 unidades
```

Isso não significa que coordenadas acima de 16.384 são impossíveis — a source possui mapas com posições muito maiores — mas a grid reutiliza os mesmos buckets através do mask `0xFF`.

### Temp face area

`SetupPolyAreas()`:

```cpp
AreaList = new POINT[4096];
```

Uma face gigantesca que atravessasse mais de 4096 células poderia estourar esse temporário.

---

# 24. Colisão

**Status: PROVADO PELA SOURCE.**

Não existe uma collision mesh separada no `Stage 0.72`.

As próprias faces do Stage são usadas.

O material possui:

```cpp
SMMAT_STAT_CHECK_FACE
```

E funções como:

```text
CheckNextMove
GetAreaHeight
GetFloorHeight
CheckSolid
```

testam somente faces cujo material tenha esse bit.

Scripts de material:

```text
pass:
notpass:
```

controlam isso.

`smTexture.cpp:850-858`

Portanto Blender Map Editor pode oferecer:

```text
Collision = ON/OFF
Passable
Ice
Water
```

como propriedades do material/face.

**É viável editar colisão diretamente.**

---

# 25. Lightmaps

**Status: PROVADO PELA SOURCE.**

O parser detecta:

```text
*MESH_MAPPINGCHANNEL
```

e ativa o caminho lightmap.

Existe:

```cpp
ReadASE_LIGHTMAP_MATERIAL()
```

`smRead3d.cpp:632`

e `smSTAGE3D_ReadASE()` usa esse parser.

O lightmap é integrado ao sistema de:

* texture stages;
* TexLinks;
* UVs adicionais.

Não encontrei um grande bloco binário proprietário independente chamado “lightmap”.

Então conceitualmente:

```text
Stage face
 ├─ diffuse UV/texture
 └─ lightmap UV/texture
```

### Blender precisa gerar lightmap?

Para reproduzir um mapa existente fielmente: sim, deve pelo menos preservar/importar.

Para criar um novo mapa, você pode:

1. gerar/bakear lightmap no Blender;
2. exportar segunda UV layer;
3. criar material/TexLink equivalente.

Há também iluminação dinâmica independente.

---

# 26. Objetos colocados no mapa

**Status: PROVADO PELA SOURCE.**

Existe:

`smSTAGE_OBJECT`

`smStgObj.h`

Limites:

```cpp
#define MAX_PATTERN 256
#define MAX_STAGEOBJ 1024
```

Cada placement contém:

```cpp
smPAT3D *Pattern;
smPAT3D *BipPattern;
POINT3D Posi;
POINT3D Angle;
```

Mas há um detalhe fundamental:

**o `smSTAGE3D::SaveFile()` não serializa esses objects como parte do Stage.**

Ao carregar um Stage ele simplesmente faz:

```cpp
StageObject = new smSTAGE_OBJECT;
```

`smStage3d.cpp:2350`

Os arquivos de objetos são registrados pelo sistema `sFIELD`.

`field.h`:

```cpp
char *lpStageObjectName[50];
DWORD StgObjBip[50];
```

Então um editor de mapa completo precisará editar:

```text
Stage SMD
+
field metadata/placements
```

não apenas o `.smd`.

---

# 27. Terrain

**Status: PROVADO PELA SOURCE.**

Não encontrei:

* heightmap terrain;
* tile height system;
* specialized terrain mesh.

O chão principal é a própria triangle mesh do Stage.

Altura é obtida intersectando/testando essas faces.

Logo:

> **terrain é mesh.**

Textura é material/TexLink normal.

Pode haver múltiplas texture layers por face através do mesmo sistema multitexture.

---

# 28. Água

**Status: PROVADO PELA SOURCE.**

Água é principalmente:

```text
geometry
+
material semantics
```

Material scripts:

```text
water:
orgwater:
```

mudam `MeshState/WindMeshBottom`.

Além disso há scrolling/reflection texture modes.

Então um Blender map exporter consegue recriar a parte asset-side da água.

A lógica extra de efeitos/render deve continuar sendo feita pelo cliente.

---

# 29. Portais, warps e triggers

**Status: PROVADO PELA SOURCE.**

Não ficam no Stage SMD.

`sFIELD` possui:

```cpp
sWARPGATE WarpGate[12];
sFGATE FieldGate[12];
```

`field.h:15-30`, `143-149`

E funções:

```cpp
AddWarpGate()
AddWarpOutGate()
AddGate()
CheckWarpGate()
```

`field.cpp`.

Pior para uma ferramenta puramente data-driven: a definição de muitos mapas está **hardcoded em `InitField()`**.

Exemplo:

`field.cpp:446+`

```cpp
sField[cnt].SetName("forest//fore-3.ase", ...);
sField[cnt].AddStageObject(...);
sField[cnt].AddWarpGate(...);
sField[cnt].AddAmbientPos(...);
```

Então um Map Editor completo tem duas opções:

1. gerar/editar código/config equivalente; ou
2. primeiro refatorar isso para um formato externo.

Esse é um dos poucos pontos onde eu recomendaria alterar a arquitetura do jogo.

---

# 30. Navegação / pathfinding

**Status: NÃO ENCONTRADO para navmesh.**

Não encontrei uma navmesh tradicional.

O movimento usa:

* Stage spatial grid;
* collision faces;
* height queries;
* movement tests.

Portanto alterar geometria **não exige regenerar uma navmesh separada**, mas exige regenerar:

```text
StageArea[256][256]
```

com:

```cpp
SetupPolyAreas()
```

Isso funciona como spatial acceleration para collision/render.

---

# 31. Som

**Status: PROVADO PELA SOURCE.**

O Stage SMD não armazena referências de som.

`sFIELD` possui:

```cpp
BackMusicCode;
AmbientPos[80];
```

Cada ambient:

```cpp
x,y,z
round
AmbentNum
```

Então existe spatial ambient sound, porém é **field metadata**, não propriedade do SMD geometry.

Um Blender Map Editor completo deveria importar isso como empties/sound markers, mas salvar em outro formato/configuração.

---

# 32. Câmeras

**Status: PROVADO PELA SOURCE.**

O parser principal de Model/Stage não usa `*CAMERAOBJECT`.

Mas existe um sistema separado:

`CAutoCamera.cpp`

`ReadASE_AutoCamera():106+`

que procura:

```text
*CAMERAOBJECT
```

e grava um cache:

```text
.cam
```

```cpp
#define AUTO_CAMERA_EXT "cam"
```

Portanto:

* câmera não é parte normal do Model SMD;
* não é parte normal do Stage SMD;
* existe um formato/sistema separado de auto-camera.

O Blender pode suportá-lo como módulo separado.

---

# 33. Dados ASE descartados

**Status: PROVADO PELA SOURCE.**

O parser usa:

* node name;
* node parent;
* node transform;
* geometry;
* face/material IDs;
* texture vertices;
* texture faces;
* mapping channel;
* diffuse/material scripts;
* transparency;
* self illumination;
* two sided;
* rotation keys;
* position keys;
* scale keys;
* Physique;
* Stage light objects.

### Descartados/não processados no parser principal

Não encontrei suporte a:

```text
MESH_VERTEXNORMAL
MESH_FACENORMAL
MESH_SMOOTHING
HELPEROBJECT
generic CAMERAOBJECT
material specular
material shininess
material ambient color
weighted Physique weights
```

Normals são recalculados.

Cameras usam parser separado.

Helpers só sobrevivem se forem exportados de uma forma que o parser interprete como `GEOMOBJECT`, especialmente bones `Bip...`.

Isso define bem o escopo do Blender exporter: **não precisamos reproduzir o ASE inteiro.**

---

# 34. ASE → SMD

**Status: PROVADO PELA SOURCE.**

`smFindFile()`:

`smRead3d.cpp:1838-1893`

compara o ASE com o cache SMD.

Se o SMD existe e é mais novo ou igual, ele é reutilizado.

Se o ASE é mais recente:

```cpp
CompareFileTime(...) < 0
```

retorna `FALSE`, forçando parse/rebuild.

`smASE_Read()`:

```text
tenta .smd cache
→ se não puder usar
→ abre ASE
→ parse
→ SortVertex
→ ReformPhysique/ReformTM
→ salva .smd
→ retorna pattern
```

### Existe conversor oficial?

Sim: **o próprio engine é o conversor.**

Principais APIs:

```cpp
smASE_Read()
smASE_ReadBone()
smASE_MergeBone()
smSTAGE3D_ReadASE()
```

Não encontrei ferramenta CLI limpa pronta.

Mas podemos construir uma pequena CLI reaproveitando essas funções ou reproduzir o serializer em Python.

---

# 35. Cache/invalidação

**Status: PROVADO PELA SOURCE.**

Se:

```text
asset.ase
asset.smd
```

existem, o timestamp é comparado.

Se ASE for mais novo, recompila.

Portanto para forçar:

```text
touch asset.ase
```

ou remover:

```text
asset.smd
```

é suficiente no pipeline clássico.

Isso é excelente para a ferramenta Blender: o primeiro exporter poderia até gerar ASE e deixar o engine converter.

Entretanto, para produção eu prefiro serializer SMD direto depois de validado.

---

# 36. INX / INI

**Status: PROVADO PELA SOURCE.**

`fileread.cpp:278+`

```cpp
AddModelDecode()
```

converte configuração textual para `smMODELINFO`.

Depois:

`fileread.cpp:985`

```cpp
fwrite(Info, sizeof(smMODELINFO), 1, fp);
```

gera INX binário.

Então INX é essencialmente um cache binário da estrutura de model info.

### Pode adicionar novo motion/model/LOD/textura?

No nível de modelo: sim.

### Novo item/personagem inteiro?

Depende.

Se um sistema externo precisa de:

* item code;
* character code;
* database row;
* class ID;
* monster ID;

isso não nasce simplesmente no INX.

Então:

```text
substituir asset existente → fácil
registrar model variant → data-driven em boa parte
criar ID de gameplay completamente novo → pode exigir client + server + DB
```

---

# 37. Novo asset vs substituir asset

**Status: PROVADO PELA SOURCE.**

Substituir:

```text
MfbC01
```

mantém todos os IDs e referências existentes.

Você só troca o conteúdo apontado.

Adicionar:

```text
MfbC99
```

exige que algum sistema passe a referenciar esse novo nome.

Dependendo do tipo:

### Novo skin/model variant

Pode bastar:

```text
INI/INX + arquivos
```

se o jogo já possui tabela dinâmica para selecionar o arquivo.

### Novo item

Provavelmente envolve:

* item definition;
* server tables;
* client tables;
* code/DB dependendo do sistema.

### Nova classe

Exigiria muito mais que asset.

Portanto a ferramenta Blender deve separar claramente:

```text
Asset Editor
```

de:

```text
Gameplay Registry Editor
```

---

# 38. Limites de nomes

**Status: PROVADO PELA SOURCE.**

Principais limites:

```text
NodeName          char[32]
NodeParent        char[32]

ModelFile         char[64]
MotionFile        char[64]
SubModelFile      char[64]
LinkFile          char[64]

Texture Name      char[64]
Texture NameA     char[64]

LOD node name     char[16]
```

O ASE parser inclusive trunca node names para 31 caracteres.

Portanto o addon deve impedir nomes maiores.

### Unicode

A arquitetura é fortemente:

```cpp
char*
lstrcmp
strcpy
fopen
```

Não há UTF-16 no asset core.

**Trate nomes como ANSI/legacy codepage, não Unicode moderno.**

---

# 39. Coordenadas e precisão

**Status: PROVADO PELA SOURCE.**

```cpp
#define fONE 256
#define FLOATNS 8
```

`smType.h:20-21`

Posições são em geral:

```text
real_position × 256
```

### Quantização

Um round-trip ideal para int usa resolução:

```text
1 / 256
= 0,00390625 unidade
```

Com arredondamento ao nearest, erro máximo por eixo:

```text
0,5 / 256
≈ 0,001953125
```

Isso é muito pequeno.

### Range teórico int32

```text
±2.147.483.647 / 256
≈ ±8.388.608 unidades
```

Mas esse não é o limite prático.

`AddVertex()` calcula:

```cpp
(mx*mx) + (my*my) + (mz*mz)
```

em `int`.

Então modelos muito gigantes podem overflowar cálculos de bound muito antes do range máximo de coordenada.

### Stage

Coordenadas grandes são usadas no próprio jogo — encontrei valores acima de 100.000 world units em `field.cpp`.

Portanto o engine não está limitado a ±16k world coordinates.

---

# 40. Orientação e sistema de coordenadas

**Status: PARCIALMENTE PROVADO PELA SOURCE.**

Internamente o jogo considera:

```text
X = horizontal
Y = vertical
Z = horizontal/depth
```

porque height APIs recebem:

```cpp
GetHeight(x,z)
```

e retornam `y`.

Mas há uma troca importante no caminho ASE/matrix.

Exemplo:

`smOBJ3D::WorldForm()`

```cpp
smWorldVertex.x = rx
smWorldVertex.z = ry
smWorldVertex.y = rz
```

Ou seja, os componentes Y/Z de algumas matrizes são trocados na passagem para coordenadas do jogo.

`smSTAGE_OBJECT::AddObject()` também lê:

```cpp
x = Tm._41;
y = Tm._43;
z = Tm._42;
```

Portanto:

**a troca Y↔Z é provada.**

### Handedness final Blender

Ainda classifico como **FALTA PROVAR**.

Para determinar sem ambiguidade se precisamos também inverter sinal/winding, quero um único asset conhecido com:

* posição visual identificável;
* triangle winding;
* bone transform conhecido.

Não vale adivinhar isso.

---

# 41. Transformações de objetos

**Status: PROVADO PELA SOURCE.**

Não é obrigatório aplicar toda transformação aos vértices.

`smOBJ3D` mantém:

```text
Tm
TmInvert
TmResult
TmRotate
mWorld
mLocal
NodeParent
```

A hierarquia é realmente usada.

Então Blender pode exportar transforms de objetos/nodes.

Para Physique, entretanto, vertices precisam estar no espaço correspondente ao bind/bone através de `ReformPhysique()`.

Portanto:

* rigid/static object: transform de node é válido;
* skinned mesh: bind-space precisa ser consistente.

---

# 42. Material por face

**Status: PROVADO PELA SOURCE.**

`smFACE.v[3]` é material ID.

```cpp
Face[FaceNum].v[3] = MatNum;
```

`smObj3d.cpp:344-348`

Então faces podem usar materials diferentes livremente.

O material ID é índice dentro do `smMATERIAL_GROUP` associado ao Pattern/Stage.

Adicionar material exige:

```text
MaterialCount++
novo smMATERIAL
face.v[3] apontando para ele
```

O exporter deve garantir:

```text
0 <= materialId < MaterialCount
```

e `<5000`.

---

# 43. Múltiplos objetos novos

**Status: PROVADO PELA SOURCE.**

Sim.

O file header possui:

```cpp
ObjCounter
```

e uma tabela:

```cpp
smDFILE_OBJINFO
{
    char szNodeName[32];
    int Length;
    int ObjFilePoint;
}
```

`smObj3d.h:56-72`

`SaveFile()` escreve a tabela dinamicamente.

Portanto para acrescentar um `smOBJ3D`:

```text
ObjCounter++
nova OBJINFO
novo object block
```

é suficiente no nível do SMD.

Depois:

```cpp
LinkObject()
```

reconstrói a hierarquia via `NodeParent`.

Logo overlays/acessórios internos são estruturalmente viáveis.

Limite: **128 objects/nodes.**

---

# 44. SMD sem Physique

**Status: PROVADO PELA SOURCE.**

Sim.

`Physique` pode ser `NULL`.

`WorldForm()` possui dois caminhos:

```cpp
if(Physique)
{
    bone transform...
}
else
{
    mWorld transform...
}
```

`smObj3d.cpp:1658-1758`

Então objeto rígido funciona normalmente sem skinning.

Ele pode:

* usar sua própria matrix;
* ter parent object;
* ser ligado externamente a um bone via `LinkParentObject`.

Isso é exatamente o necessário para armas/acessórios rígidos.

---

# 45. Segurança da serialização / ponteiros

**Status: PROVADO PELA SOURCE.**

Essa é uma das partes mais críticas para o exporter.

O formato grava:

```cpp
WriteFile(hFile, &Head, sizeof(smOBJ3D), ...)
```

ou seja, grava **a classe inteira**, inclusive ponteiros.

Depois grava arrays.

No load ele lê de volta o raw struct:

```cpp
ReadFile(hFile, &Head, sizeof(smOBJ3D), ...)
```

e reconstrói vários ponteiros.

### Sobrescritos no load

Explicitamente:

```text
Vertex
Face
TexLink
TmRot
TmPos
TmScale
TmPrevRot
```

recebem `new[]`.

`Physique` é reconstruído a partir dos bone names se o ponteiro raw indicar que Physique existe.

`pParent` é restabelecido posteriormente por `LinkObject()`.

### TexLinks

O formato depende dos valores históricos dos ponteiros para calcular **offset relativo**:

```cpp
lpOldTexLink = TexLink;

SubTexLink =
    TexLink[cnt].NextTex - lpOldTexLink;

TexLink[cnt].NextTex =
    TexLink + SubTexLink;
```

Idem para:

```cpp
Face[cnt].lpTexLink
```

Portanto não podemos simplesmente preencher todos os pointers com zero num serializer do zero.

Precisamos representar uma base virtual consistente para:

```text
TexLink base
Face.lpTexLink
TexLink.NextTex
```

### `hTexture`

Não encontrei dereference importante do pointer raw `hTexture` no pipeline normal; ele é carregado/carregado adiante mas os materiais determinam a textura real.

Eu ainda o trataria conservadoramente como zero/canonical até validar assets.

### Stage é pior

`smSTAGE3D` grava:

```cpp
sizeof(smSTAGE3D)
```

e usa os pointers de:

```cpp
StageArea[x][z]
```

como **flags de existência** no loader antes de reconstruí-los.

Então writer Stage sem template é totalmente possível, mas exige engenharia cuidadosa.

---

# 46. ABI

**Status: PROVADO PELA SOURCE; tamanhos derivados para i686, recomendável validar com `static_assert` no próprio build.**

O formato é explicitamente ABI-dependent.

Com ponteiros de 32 bits e alinhamento i686 esperado:

| Estrutura          | tamanho esperado |
| ------------------ | ---------------: |
| `smTEXLINK`        |               32 |
| `smVERTEX`         |               24 |
| `smFACE`           |               36 |
| `smTM_ROT`         |               20 |
| `smTM_POS`         |               16 |
| `smTM_SCALE`       |               16 |
| `smFRAME_POS`      |               16 |
| `smSTAGE_VERTEX`   |               28 |
| `smSTAGE_FACE`     |               28 |
| `smMATERIAL`       |              320 |
| `smMATERIAL_GROUP` |               88 |
| `smOBJ3D`          |         **2236** |
| `smPAT3D`          |         **1252** |
| `smSTAGE3D`        |       **262260** |
| `smMOTIONINFO`     |          **172** |
| `smMODELINFO`      |        **95268** |

O enorme tamanho de `smSTAGE3D` acontece principalmente porque contém:

```cpp
DWORD *StageArea[256][256];
```

dentro da própria classe.

### MSVC vs MinGW

Esse era um risco importante do port.

Mas há uma evidência prática forte: **o cliente MinGW atual consegue carregar os assets binários históricos**.

Então para essas estruturas essenciais o layout atual está suficientemente compatível.

Para o addon Blender, entretanto, não devemos serializar usando `sizeof` nativo do host.

Devemos especificar byte por byte:

```text
little endian
32-bit pointers
32-bit int/DWORD
16-bit WORD
4-byte alignment
```

---

# 47. Compatibilidade entre personagens/classes

**Status: PROVADO PELA SOURCE no pipeline; assets concretos ainda precisam de catalogação.**

Fighter, Mechanician, Archer, Pikeman, Atalanta, Knight, Magician, Priestess etc. passam pelo mesmo:

```text
smMODELINFO
smPAT3D
smOBJ3D
SMD Model 0.62
SMB
INX
Physique
```

Não encontrei formatos 3D diferentes por job.

O que varia:

* skeleton filenames;
* node names;
* motion sets;
* attachment points;
* effect sockets;
* model/submodel composition.

Então não devemos escrever um “Priestess importer”.

Devemos criar:

```text
generic SMD/SMB importer
+
character profile layer
```

onde cada profile descobre automaticamente seus bones/motions/sockets.

---

# 48. Ferramenta Blender ideal

Aqui já consigo classificar.

| Módulo                              | Situação                                      |
| ----------------------------------- | --------------------------------------------- |
| Character import                    | **VIÁVEL COM A SOURCE ATUAL**                 |
| Character free topology             | **VIÁVEL COM A SOURCE ATUAL**                 |
| UV editing                          | **VIÁVEL COM A SOURCE ATUAL**                 |
| Material editing                    | **VIÁVEL COM A SOURCE ATUAL**                 |
| Rigid Physique authoring            | **VIÁVEL COM A SOURCE ATUAL**                 |
| Weighted skinning                   | **PRECISA ALTERAR O CLIENTE**                 |
| Skeleton editing                    | **VIÁVEL, COM RESTRIÇÕES DE NOMES/SOCKETS**   |
| Animation preview                   | **VIÁVEL COM A SOURCE ATUAL**                 |
| New animation authoring             | **VIÁVEL COM A SOURCE ATUAL**                 |
| LOD editing                         | **VIÁVEL COM A SOURCE ATUAL**                 |
| Direct SMD writer                   | **VIÁVEL, precisa concluir ABI writer**       |
| Direct SMB writer                   | **VIÁVEL, precisa concluir animation writer** |
| INX writer                          | **VIÁVEL COM A SOURCE ATUAL**                 |
| Monster editor                      | **VIÁVEL COM A SOURCE ATUAL**                 |
| Weapon/item editor                  | **VIÁVEL COM A SOURCE ATUAL**                 |
| Stage geometry editor               | **VIÁVEL COM A SOURCE ATUAL**                 |
| Stage collision editor              | **VIÁVEL COM A SOURCE ATUAL**                 |
| Stage materials/UV/lightmaps        | **VIÁVEL COM A SOURCE ATUAL**                 |
| Stage placements                    | **VIÁVEL, MAS NÃO ESTÃO NO SMD**              |
| Warp/portal editor                  | **PRECISA EDITAR/REFATORAR `sFIELD`**         |
| Spawn editor                        | **OUTRO SISTEMA, não Stage 0.72**             |
| BGM/ambient/sky/weather             | **OUTRO SISTEMA, não Stage SMD**              |
| Generic weighted Blender rig export | **INCOMPATÍVEL COM ENGINE ATUAL**             |

A maior limitação artística é clara:

> Blender pode usar weights para edição, mas na exportação precisamos resolver cada vertex para **um único bone**.

---

# 49. O que NÃO precisamos preservar

**Status: PROVADO PELA SOURCE.**

Podemos reconstruir livremente:

```text
vertices
face count
vertex order
face order
UVs
TexLinks
normals
Physique assignments
material group
materials
smOBJ3D table
object count
object transforms
animation tracks
motion ranges
LOD geometry
```

desde que cumpramos os limites e referências.

### Não precisamos preservar

```text
vertex #42 continuar sendo vertex #42
face #100 continuar sendo face #100
quantidade original de vertices
quantidade original de faces
topologia original
```

Isso responde uma das perguntas mais importantes do projeto:

## **Free topology está liberada.**

### O que precisa ser preservado semanticamente

Não necessariamente byte por byte, mas precisa continuar consistente:

* bone names usados pelo Physique;
* attachment/socket names esperados pelo código;
* hierarchy necessária;
* motion `State`;
* `ItemCodeList`;
* `JobBits`;
* `SkillCodeList`;
* event timing;
* material collision/pass/water semantics;
* IDs externos de gameplay;
* exact binary ABI;
* pointer-relative encoding dos formatos;
* INX references.

---

# 50. Arquitetura recomendada

Minha recomendação mudou bastante depois desta auditoria.

Eu **não faria a ferramenta final baseada no “SMD original como template”**.

Template é excelente para a primeira fase de validação, mas a source já prova que podemos chegar a:

```text
Blender Scene
       ↓
canonical Summit asset model
       ↓
serializer
       ├── SMD Model 0.62
       ├── SMB
       ├── INX
       └── SMD Stage 0.72
```

A arquitetura mínima seria:

```text
summit_blender_tools/
│
├── io/
│   ├── binary.py
│   ├── abi_i686.py
│   ├── smd_model_062.py
│   ├── smb.py
│   ├── inx.py
│   ├── ase.py
│   ├── stage_072.py
│   └── camera_cam.py
│
├── model/
│   ├── mesh.py
│   ├── skeleton.py
│   ├── rigid_skin.py
│   ├── animation.py
│   ├── materials.py
│   └── texlinks.py
│
├── blender/
│   ├── importer.py
│   ├── exporter.py
│   ├── armature.py
│   ├── materials.py
│   ├── animation.py
│   └── validation.py
│
├── character/
│   ├── inx_profile.py
│   ├── motion_selector.py
│   └── socket_registry.py
│
└── map/
    ├── stage.py
    ├── collision.py
    ├── lightmap.py
    ├── placement.py
    └── field_metadata.py
```

O ponto mais importante é ter um **modelo intermediário limpo**, não fazer:

```text
Blender objects
→ ponteiros C++ falsos diretamente
```

O serializer deve construir o arquivo no final.

### Primeira milestone

```text
SMD original
→ parse
→ canonical model
→ write novo SMD
→ byte-semantic roundtrip
→ client
```

Depois que esse writer for provado:

```text
Blender novo mesh do zero
→ same canonical model
→ writer
```

Sem template.

---

# 51. Prioridade das próximas descobertas

Esta é a ordem que mais destrava o projeto:

1. **Fechar e testar o writer SMD 0.62 do zero.** É a fundação de personagem, monstro, arma e objeto.
2. **Formalizar a ABI i686 e pointer-offset encoding.** Colocar testes automatizados de `sizeof`, offsets e load pelo client.
3. **Implementar rigid Physique export.** Blender weights → escolha de 1 bone → inverse bind → bone name.
4. **Round-trip completo de material + TexLink.** Inclusive 1–8 texture stages, opacity, reflection, scroll e animated textures.
5. **Criar writer SMB completo.** Gerar hierarchy, `TmRot/TmPos/TmScale/TmPrevRot/TmFrame`.
6. **Criar writer/editor INX.** State, item, job, skill, events e MotionFrame.
7. **Catalogar sockets/hardcoded bone names de todas as classes e monstros.**
8. **Criar character profile discovery automático.** Nada de código especial para Priestess.
9. **Stage 0.72 reader/writer.** Primeiro geometry + materials + UV.
10. **Regenerar `StageArea` e collision.**
11. **Suportar lightmap/multichannel Stage.**
12. **Separar field metadata do Stage.** Warps, BGM, sky, ambient, objects.
13. **Refatorar `field.cpp` hardcoded para dados externos**, se quisermos editor completo de mapas sem recompilar o client.
14. **Só depois fazer testes de limites artísticos** de 10k/20k/30k vertices, 1k/2k textures etc.

---

# Algumas respostas adicionais importantes que emergiram da auditoria

### `smOBJ3D` pode realmente ser criado do zero?

**Sim.**

A source não depende de um ID secreto do objeto original.

O que precisamos construir corretamente é:

```text
raw smOBJ3D header
Vertex[]
Face[]
TexLink[]
TmRot[]
TmPos[]
TmScale[]
TmPrevRot[]
bone names por vertex
```

O maior trabalho não é descobrir a semântica da geometria.

É reproduzir a **serialização raw C++ com os offsets de ponteiro corretos**.

### `SMB` pode ser criado do zero?

**Sim.**

É essencialmente o mesmo container `smPAT3D` com nodes/hierarchy/animation.

### `INX` pode ser criado do zero?

**Sim.**

A própria source produz INX fazendo:

```cpp
fwrite(&smMODELINFO, sizeof(smMODELINFO), ...)
```

### Podemos fazer animações 100% novas?

**Sim.**

Nada liga uma animation key a um vertex original.

Ela opera em nodes/bones.

### Podemos remodelar totalmente a Priestess?

**Sim.**

Você pode:

```text
apagar todos os vertices
modelar corpo novo
retopology
UV nova
material novo
rig novo
```

e continuar usando o skeleton antigo, desde que:

* cada vertex tenha um bone válido;
* seja rigid skin;
* bind-space esteja correto;
* sockets usados pelo gameplay continuem existindo.

---

# BLENDER TOOL — GAP ANALYSIS

## JÁ RESOLVIDO

**PROVADO:**

```text
SMD Model 0.62 basic structure
free topology
vertex/face representation
face materials
UV / TexLink representation
multi-texture chain
normal reconstruction
rigid Physique
bone-name mapping
SMB fundamental structure
skeleton hierarchy
animation tracks
frame scale = 160
rotation interpolation
position interpolation
scale interpolation
INX MotionInfo
animation selection rules
animation events
LOD mechanism
material structure
material scripts
texture loader
animated textures
Direct3D9 render architecture
attachments by node name
monster pipeline
weapon/item pipeline
Stage 0.72 fundamental structure
Stage collision
Stage spatial grid
field warps
field ambient sound
field stage object references
ASE caching/invalidation
```

## FALTA PROVAR

```text
handedness Blender ↔ engine definitivo
seams/bone-boundary topology dos assets originais
exact role de M5-motion1..7.smb
texture dimension practical ceiling na GPU alvo
DDS via loader atual
todos os socket names de todas as classes/monstros
todas as variantes de Model/Pattern2 por classe
```

## FALTA IMPLEMENTAR

```text
SMD 0.62 serializer independente
SMB serializer
INX serializer
rigid-weight converter
material node mapping Blender ↔ Summit
multi-UV/TexLink editor
animation exporter
socket validator
Stage 0.72 serializer
StageArea builder
field metadata editor
```

## LIMITAÇÃO DO ENGINE

```text
1 bone por vertex
128 nodes/objects por smPAT3D
30000 render vertices
30000 render faces buffer
8 texture stages
5000 materials
64 renderer lights
CPU-heavy transform/skinning/clipping
DrawPrimitiveUP pipeline
```

## LIMITAÇÃO DO FORMATO

```text
WORD vertex/material indices
NodeName = 32
NodeParent = 32
LOD names = 16
model/texture paths ≈ 64
raw 32-bit C++ ABI
pointer-relative serialization
32 motion segments
512 MotionInfo
4 generic events por motion
```

## NÃO SABEMOS AINDA

```text
exact mesh seam strategy de todos os personagens
exact semantic grouping dos M5-motion*.smb
complete per-class skeleton taxonomy
all legacy/proprietary texture variants in real assets
exact practical polygon budget acceptable for gameplay
```

---

# Até onde podemos transformar Blender em ferramenta completa sem mudar o engine?

**Muito longe.**

Para **personagens, monstros, armas e objetos**, realisticamente podemos chegar a uma ferramenta praticamente completa sem modificar o engine:

```text
asset original
→ Blender
→ free topology
→ UV/material
→ rigid rigging
→ animação
→ LOD
→ export SMD/SMB/INX
→ client
```

A única limitação artística realmente importante é:

> **sem weighted skinning; cada vertex precisa terminar ligado a um único bone.**

Para **mapas**, também podemos editar a parte 3D completa:

```text
Stage geometry
UV
materials
lightmap
collision
lights
```

Mas “mapa completo do jogo” não cabe apenas no `SMD Stage 0.72`.

Para editar também:

```text
warps
gates
BGM
sky
ambient sounds
object sets
field events
spawns
```

precisaremos incorporar outros sistemas — e uma parte significativa da definição de field ainda está hardcoded em `field.cpp`.

Portanto minha avaliação final é:

**Character Editor: ~90–95% viável sem mudar o engine.**
**Monster Editor: ~90–95%.**
**Weapon/Item Editor: ~95%.**
**Animation Editor: ~90%, após fechar o writer SMB.**
**Map geometry/collision editor: ~85–90%.**
**Map/world editor completo: ~60–70% hoje**, subindo bastante se externalizarmos os dados hardcoded de `sFIELD`.

E a descoberta que mais muda o projeto é esta:

> **Nós não precisamos mais projetar o Blender tool como um “patcher da Priestess”. A source sustenta uma arquitetura de serializer genérico e topologia livre.** O próximo gargalo real é implementar e provar a serialização binária i686 de `SMD 0.62` — não continuar descobrindo offsets específicos de meshes antigas.

