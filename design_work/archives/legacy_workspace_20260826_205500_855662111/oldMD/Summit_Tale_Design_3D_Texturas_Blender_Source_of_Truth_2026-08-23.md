# Summit Tale — Design, Texturas, Modelos 3D, ASE/SMD e Pipeline Blender
## Source of Truth Técnica — 2026-08-23

> Documentação consolidada dos testes feitos no Summit Tale, auditorias locais do cliente/servidor, leitura do source C/C++, inspeção de arquivos ASE/SMD/SMB/INX/BMP/TGA, ferramentas legadas encontradas e materiais técnicos reunidos durante a investigação.
>
> Objetivo: servir como referência prática para retexturização, remodelagem, personagens, armaduras, heads, armas, itens, Ricarten/mapas e futura integração com Blender.
>
> Regra central: separar sempre o que foi provado no Summit do que é apenas referência histórica, lead externo ou hipótese.

---

# 1. Convenções de evidência

Use estas etiquetas ao atualizar esta documentação:

- **[PROVADO EM RUNTIME]** — alteração feita e observada no cliente.
- **[CONFIRMADO LOCALMENTE]** — encontrado e inspecionado diretamente nos arquivos Summit.
- **[CONFIRMADO PELO SOURCE]** — explícito no código C/C++ desta build.
- **[REFERÊNCIA HISTÓRICA]** — vem de material antigo de Priston Tale; precisa validação na build atual.
- **[LEAD EXTERNO]** — pista de outro projeto/repositório; útil, mas não é especificação do Summit.
- **[HIPÓTESE]** — interpretação plausível ainda sem prova suficiente.
- **[PENDENTE]** — precisa de teste.

Nunca promover hipótese para especificação sem teste.

---

# 2. Diretórios principais

Source:

```text
/home/gabriel/Área de Trabalho/summit/SourceCode
```

Cliente:

```text
/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente
```

Servidor:

```text
/home/gabriel/Área de Trabalho/summit/Servidor
```

Runtime Windows:

```text
C:\SummitTale\Servidor
```

Build:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
sh cliente.sh
```

Donors usados em outras etapas:

```text
/home/gabriel/armazem/priston/pristontale-1-master/bin/Client
/home/gabriel/armazem/priston/pristontale-1-master/bin/Server/GameServer

/home/gabriel/armazem/priston/ExMachina/client
/home/gabriel/armazem/priston/ExMachina/server/GameServer
```

Para design, não copiar pastas inteiras de donor sem auditoria. O Summit possui variantes próprias.

---

# 3. Três níveis de edição visual

## 3.1 Repaint/retexturização

Mantém:

- mesh;
- UV;
- bones;
- animação;
- material;
- nome/path.

Muda somente a imagem.

```text
textura original
→ decode se necessário
→ Krita/GIMP/Photoshop/Substance
→ export
→ encode/header legado
→ cliente
```

É o caminho de menor risco.

## 3.2 Alteração de UV/material

Mantém skeleton/Physique, mas pode mudar:

- UV;
- material/submaterial;
- partes da mesh;
- textura.

Requer ASE ou parser equivalente.

## 3.3 Remodelagem 3D

Muda:

- geometria;
- topologia;
- UV;
- materiais;
- possivelmente associação de vértices.

Precisa preservar:

```text
nome da mesh
+
hierarquia
+
material
+
UV
+
PHYSIQUE/bones
+
formato aceito pelo engine
```

---

# 4. Formatos principais

## 4.1 `.SMD` — formato proprietário do Priston Tale

**[CONFIRMADO LOCALMENTE]**

O `.smd` desta build não é Valve SMD.

Header real:

```text
SMD Model data Ver 0.62
```

Hex inicial observado em `MfbC01.smd`:

```text
53 4d 44 20 4d 6f 64 65 6c 20 64 61 74 61 20 56
65 72 20 30 2e 36 32 00
```

O binário contém strings de:

- nomes de objetos;
- bones;
- caminhos de textura.

Exemplo `MfbC01.smd`:

```text
char\tmABCD\MfbC01.bmp
char\tmABCD\MfbC03.bmp
char\tmABCD\mo-p-ac.BMP
```

Bones encontrados:

```text
Bip01 Head
Bip01 Pelvis
Bip01 Spine
Bip01 Spine1
Bip01 L UpperArm
Bip01 R UpperArm
Bip01 L Forearm
Bip01 R Forearm
Bip01 L Hand
Bip01 R Hand
Bip01 L Thigh
Bip01 R Thigh
Bip01 L Calf
Bip01 R Calf
Bip01 L Foot
Bip01 R Foot
```

`strings` já ajuda bastante mesmo sem parser binário completo.

## 4.2 `.SMB`

**[CONFIRMADO LOCALMENTE]**

Arquivos de movimento/animação associados existem, por exemplo:

```text
MfbC01.smb
tmbC01.smb
```

**[REFERÊNCIA HISTÓRICA]**

Material fornecido indica:

```text
m1.smb → Knight / Mechanician / Fighter
m2.smb → Atalanta / Archer
m3.smb → Magician
m4.smb → Pikeman
m5.smb → Priestess
m6.smb → Assassin
m7.smb → Shaman
m8.smb → Martial Artist
```

Tratar como referência até cruzar com a build.

## 4.3 `.INX` / `.INI`

**[REFERÊNCIA HISTÓRICA + EVIDÊNCIA LOCAL]**

Participam da seleção/configuração de modelo/motion.

Regra segura:

```text
INX/INI = autoridade para descobrir nomes de mesh esperados
```

Não renomear objetos arbitrariamente.

## 4.4 `.ASE`

**[CONFIRMADO LOCALMENTE]**

Formato ASCII real do 3ds Max presente no cliente.

Header típico:

```text
*3DSMAX_ASCIIEXPORT 200
```

Pode conter:

```text
*GEOMOBJECT
*NODE_NAME
*NODE_PARENT
*NODE_TM

*MESH_NUMVERTEX
*MESH_VERTEX_LIST
*MESH_NUMFACES
*MESH_FACE_LIST

*MESH_NUMTVERTEX
*MESH_TVERTLIST
*MESH_NUMTVFACES
*MESH_TFACELIST

*MESH_NORMALS

*MATERIAL_REF
*MESH_MTLID

*PHYSIQUE
```

## 4.5 Stage

Header encontrado/registrado durante a investigação:

```text
SMD Stage data Ver 0.72
```

Não confundir com:

```text
SMD Model data Ver 0.62
```

São estruturas diferentes.

---

# 5. `MATERIAL_REF`, `MESH_MTLID` e submateriais

`MATERIAL_REF`:

```text
*MATERIAL_REF 0
```

seleciona material top-level do objeto.

`MESH_MTLID`:

```text
*MESH_MTLID 15
```

seleciona o submaterial aplicado à face.

Não confundir.

`MATERIAL_COUNT`:

```text
*MATERIAL_COUNT 2
```

é número de materiais top-level.

`NUMSUBMTLS`:

```text
*NUMSUBMTLS 15
```

é número de submateriais dentro de um Multi/Sub-Object.

Se adicionar `SUBMATERIAL 15` a uma lista `0..14`, atualizar:

```text
*NUMSUBMTLS 16
```

---

# 6. Descoberta: `MfbC13.ASE`

**[CONFIRMADO LOCALMENTE]**

Arquivo:

```text
char/tmABCD/MfbC13.ASE
```

Resultados:

```text
GEOMOBJECTS = 67
```

UV vertices:

```text
63 objetos → MESH_NUMTVERTEX 0

4 objetos:
124
212
250
296
```

Totais:

```text
MESH_TVERT = 882
MESH_TFACE = 1086
```

Nomes não `Bip`/`Dummy`:

```text
MahC20
MbhC19
MlhC20
MshC19
```

Há `PHYSIQUE`, por exemplo:

```text
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE 0 "Bip01 Spine1"
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE 5 "Bip01 R Clavicle"
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE 11 "Bip01 R UpperArm"
```

Conclusão:

```text
MfbC13.ASE
=
bones
+
meshes reais
+
UV
+
materiais
+
Physique
```

Portanto não é apenas skeleton.

Atenção: apesar do nome `MfbC13.ASE`, contém famílias C19/C20 e texturas como:

```text
MfbC19_01.TGA
MfbC19_02.BMP
pr100_yun01.tga
pr100_yun02.tga
```

Logo é excelente como referência estrutural, mas não deve ser tratado como correspondência fiel da armadura C13.

---

# 7. Physique e rigging

O ASE auditado usa:

```text
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
```

Isso é favorável para reverse engineering porque cada vértice pode estar rigidamente associado a um bone/link.

Conceitualmente:

```text
vertex 11 → Bip01 R UpperArm
vertex 69 → Bip01 Head
```

## 7.1 Workflow histórico

**[REFERÊNCIA HISTÓRICA]**

No 3ds Max:

```text
Modifiers
→ Physique
→ Attach to Node
```

Depois:

```text
Physique
→ Vertex
→ No Blending
→ Assign to Link
```

Para peça rígida:

```text
todos os vértices
→ um único bone
```

Todos os vértices relevantes devem ter assignment válido. Erros podem causar:

- stretching;
- peça presa;
- deformações extremas;
- crash.

---

# 8. Estratégia 3D mais segura

Usar:

```text
ASE funcional conhecido
+
trocar somente os blocos necessários
```

em vez de substituir tudo por um export novo.

Preservar:

- bones;
- hierarchy;
- nomes;
- sintaxe;
- material não alterado;
- organização de objetos.

Trocar somente:

- mesh necessária;
- UV necessário;
- material necessário;
- Physique necessário.

---

# 9. Blender — viabilidade e arquitetura

## 9.1 Blender pode ser usado

Sim.

O desafio não é o Blender. O desafio é traduzir:

```text
Priston SMD/SMB/INX/ASE
↔
Mesh/Armature/Materials do Blender
```

ASE é a melhor ponte descoberta até agora.

## 9.2 Pipeline proposto

```text
ASE do Priston
→ importer Python para Blender
→ Mesh
→ UV
→ Materials
→ Armature
→ Vertex Groups
→ edição
→ export ASE compatível
→ integração no ASE funcional
→ pipeline do PT
```

## 9.3 Mapeamento para Blender

```text
*NODE_NAME
→ objeto/bone

*NODE_PARENT
→ hierarchy

*MESH_VERTEX_LIST
→ vertices

*MESH_FACE_LIST
→ faces

*MESH_TVERTLIST
→ UV coords

*MESH_TFACELIST
→ UV por face

*MATERIAL_REF / MESH_MTLID
→ material slots

*PHYSIQUE...
→ vertex groups / bone assignment
```

Para assignment rigid:

```text
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE 12 "Bip01 R UpperArm"
```

pode virar no Blender:

```text
Vertex Group: Bip01 R UpperArm
Vertex 12: weight 1.0
```

## 9.4 O que falta confirmar

- coordinate system;
- escala;
- handedness;
- rotação dos bones;
- bind pose;
- outros tipos de Physique;
- animação;
- material modes;
- normals/smoothing.

Portanto Blender é tecnicamente viável, mas o pipeline completo ainda não está provado.

---

# 10. Pipeline alternativo: SMD → Blender

Também é possível:

```text
SMD 0.62
→ parser baseado no source
→ estrutura intermediária
→ Blender
```

ou:

```text
SMD
→ parser
→ glTF
→ Blender
```

OBJ não é ideal para personagem porque perde rig.

O source local deve ser tratado como especificação principal.

Funções observadas/registradas:

```cpp
smPAT3D *smASE_ReadBone(char *file);
smPAT3D *smASE_Read(char *file, char *szModelName = 0);
```

---

# 11. Lead externo: JPsTale

**[LEAD EXTERNO]**

Durante a investigação foi identificado um projeto chamado `JPsTale` que declara loaders para:

```text
ASE
INX
SMD
SMB
```

Pode ajudar a acelerar:

- parser SMD;
- parser SMB;
- parser INX;
- importer Blender.

Comparar sempre com o source do Summit antes de considerar compatível.

---

# 12. `SMDReader.exe`

Local:

```text
Servidor/char/tmABCD/SMDReader.exe
```

**[CONFIRMADO LOCALMENTE]**

PE:

```text
PE32 console
Intel i386
```

Strings:

```text
numberOfBytesRead
smdHeader
SMD Model data Ver 0.62
Paranaue dos smd - by Razor
.smd
```

PDB:

```text
C:\Users\thiag\documents\visual studio 2017\Projects\SMDReader\Debug\SMDReader.pdb
```

Imports:

```text
CreateFileA
ReadFile
SetFilePointer
WriteFile
FindFirstFileA
FindNextFileA
```

Dependências Debug:

```text
MSVCP140D.dll
VCRUNTIME140D.dll
ucrtbased.dll
```

## 12.1 Teste no Wine

Falhou antes de entrar no programa:

```text
MSVCP140D.dll not found
VCRUNTIME140D.dll not found
ucrtbased.dll not found
status c0000135
```

Os hashes não mudaram, mas isso não prova o comportamento do reader porque ele não executou.

Não baixar DLL Debug aleatória da internet.

Prioridade:

```text
source do loader + ASE
>
ressuscitar SMDReader
```

---

# 13. Outras ferramentas legadas

## `Editor Japaokiko.exe`

Local:

```text
Servidor/Management/Editor Japaokiko.exe
```

Strings importantes:

```text
EConvertError
EPropertyConvertError
Model|B
OEMConvert
TMenuAnimations
ImportedConstraint
Model Name 1
Model Name 2
```

Referências:

```text
GameServer\Field\village-2.ase.spc
GameServer\Field\pilai.ASE.spc
GameServer\Field\ruin-2.ase.spc
GameServer\Field\village-1.ase.spc
GameServer\Field\ice_ura.ASE.spc
GameServer\Field\town1.ASE.spc
GameServer\Field\castle.ase.spc
```

**[HIPÓTESE]** Pode fazer parte do pipeline de edição/conversão de mapas, mas não está provado o que gera ou edita.

Outras ferramentas localizadas:

```text
@Level Editor.exe
@ItemEditor.exe
@Editor de Monstros.exe
GameServer/Field/NPC Editor.exe
GameServer/Field/NpcEditor.exe
GameServer/Field/NPCManager.exe
Management/Editor De Equipamentos.exe
Management/Editor de Itens.exe
Management/Editor de Monstros.exe
Todas ferramentas em 1 Tools.exe
RazorGodTool.exe
```

Algumas contêm strings:

```text
ModelName1
ModelName2
SetModelPos
basConvert
Convert
```

Usar somente em sandbox/cópia.

---

# 14. Toolchain ASE histórico e moderno

**[REFERÊNCIA HISTÓRICA]**

Workflow antigo:

```text
3ds Max 7
+
Sandurr ASE Exporter
```

Motivo:

```text
suporte a PHYSIQUE
```

Material histórico também menciona troubleshooting com VC++ 2005 runtime.

Material mais recente cita:

```text
3ds Max 2020
.ASE Export & Import 2025 (By IgorSegalla)
XNA Lara XPS
Microsoft XNA Framework Redistributable 3.0
```

Mas ainda não foi demonstrado que o exporter moderno:

- importa Physique;
- exporta Physique;
- preserva bones;
- substitui totalmente o Sandurr.

Continua pendente.

---

# 15. Export Selected e integração por peça

Estratégia histórica segura:

```text
shoulder.ase
arms.ase
body.ase
legs.ase
```

Usar:

```text
Export Selected
```

Preservar:

```text
Mapping Coordinates
```

para manter:

```text
*MESH_TVERTLIST
*MESH_TFACELIST
```

Workaround avançado citado:

```text
Export A:
Max antigo + exporter com Physique
→ usar PHYSIQUE

Export B:
Max moderno
→ usar MESH/TVERT/TFACE
```

Depois combinar os blocos em um ASE funcional.

---

# 16. Texturas: header proprietário de BMP

Uma das descobertas centrais.

## 16.1 `MfbC01.bmp`

**[CONFIRMADO LOCALMENTE]**

```text
char/tmABCD/MfbC01.bmp
```

Tamanho:

```text
196662 bytes
```

Primeiros 14 bytes:

```text
41 38 3a 09 13 19 24 31 40 51 9a 79 90 a9
```

`file` retorna originalmente:

```text
data
```

A partir de offset `0x0E` há um DIB válido:

```text
DIB size = 40
width = 256
height = 256
planes = 1
bpp = 24
```

Tamanho de BMP 256×256 RGB24:

```text
256 × 256 × 3 + 54
= 196662
```

Conclusão:

```text
bytes 0..13
→ header proprietário

bytes 14...
→ DIB/pixels BMP normais
```

---

# 17. Decode de BMP proprietário

```python
from pathlib import Path
import struct

src = Path("MfbC01.bmp")
out = Path("MfbC01.decoded.bmp")

b = bytearray(src.read_bytes())

b[:14] = struct.pack(
    "<2sIHHI",
    b"BM",
    len(b),
    0,
    0,
    54
)

out.write_bytes(b)
```

Validar:

```bash
file MfbC01.decoded.bmp
identify MfbC01.decoded.bmp
```

Resultado real obtido:

```text
PC bitmap
Windows 3.x format
256 x 256 x 24
image size 196608
cbSize 196662
bits offset 54
```

---

# 18. Encode para formato usado pelo cliente

Para edição mantendo exatamente a mesma estrutura:

```python
from pathlib import Path

original = Path("MfbC01.original.bmp").read_bytes()
edit = bytearray(Path("MfbC01.editado.bmp").read_bytes())

assert len(edit) == len(original)
assert edit[:2] == b"BM"

edit[:14] = original[:14]

Path("MfbC01.game.bmp").write_bytes(edit)
```

**Importante:** isso está provado somente mantendo:

```text
mesma largura
mesma altura
mesmo bpp
mesmo tamanho total
```

Ainda não está provado para 512×512.

---

# 19. Teste runtime de `MfbC01.bmp`

**[PROVADO EM RUNTIME]**

Fluxo executado:

```text
MfbC01.bmp
→ decode
→ edição magenta
→ reapply do header
→ instalação
→ cliente
```

Resultado:

```text
a Sacerdotisa ficou com o vestido completamente magenta
```

Portanto:

```text
MfbC01.bmp
→ participa do visual da Sacerdotisa testada
```

Restore também funcionou.

O arquivo decodificado:

```text
MfbC01.decoded.bmp
```

foi identificado pelo usuário como atlas da Sacerdotisa.

Não usar esse atlas como mapa UV do Pikeman.

---

# 20. Família `MfbC`

Arquivos observados:

```text
MfbC01.bmp
MfbC01.smd
MfbC01.smb
MfbC01b.smd
MfbC01h-02.TGA

MfbC03.bmp
MfbC03.smd
MfbC03b.smd

MfbC05.bmp
MfbC05.smd
MfbC05b.smd

MfbC07.bmp
MfbC07.smd
MfbC07b.smd

MfbC08.bmp
MfbC08.smd
MfbC08b.smd
MfbC08w.bmp

MfbC09.BMP
MfbC09.smd
MfbC09b.smd
MfbC09-w.TGA
```

Não assumir que o número indica nível diretamente.

---

# 21. Convenções históricas de classe

**[REFERÊNCIA HISTÓRICA]**

```text
A  = Mechanician
B  = Fighter
C  = Pikeman
D  = Archer
E  = Assassin

mA = Knight
mB = Atalanta
mC = Priestess
mD = Magician
mE = Shaman
mF = Martial Artist
```

Usar como filtro de candidatos e confirmar via:

```text
INX/INI
→ SMD
→ runtime
```

---

# 22. Família `TmbC`

Auditoria encontrou:

```text
TmbC01.smd
TmbC01b.smd
tmbC01.smb
TmbC01.bmp

TmbC03.smd
TmbC03b.smd
TmbC03.bmp

TmbC06.smd
TmbC06b.smd
TmbC06a.bmp
TmbC06b.bmp

TmbC08.smd
TmbC08b.smd
TmbC08a.bmp
TmbC08b.bmp

TmbC10.smd
TmbC10b.smd
TmbC10a.bmp
TmbC10b.bmp

TmbC11.smd
TmbC11b.smd
TmbC11a.bmp
TmbC11b.bmp

TmbC13.smd
TmbC13b.smd
TmbC13a.bmp
TmbC13b.bmp

TmbC14.smd
TmbC14_p.smd
TmbC14a.bmp
TmbC14b.bmp
TmbC14c.tga

TmbC15.smd
TmbC16.smd
TmbC17.smd
TmbC18.smd
TmbC19.smd

TmbC20.smd / TmbC20_1.TGA / _2 / _3
TmbC21.smd / TmbC21_1.TGA / _2 / _3
TmbC22.smd / TmbC22.tga
TmbC23.smd / TmbC23.tga
TmbC24.smd / TmbC24.tga
```

## 22.1 `TmbC01.smd`

Strings:

```text
char\tmABCD\TmbC01.bmp
char\tmABCD\TmbC03.bmp
```

Objetos iniciais:

```text
TbhC03
TbmC03
TblC03
TblC01
TbmC01
TbhC01
TahC01
TamC01
TalC01
```

Bones incluem:

```text
Bip01 Head
Bip01 Pelvis
Bip01 Spine
Bip01 Spine1
Bip01 Tail
Bip01 Tailba1
...
```

Isso mostra que um SMD pode conter múltiplas partes/LOD/variantes e múltiplas texturas.

Não assumir:

```text
1 SMD = 1 textura = 1 armadura
```

---

# 23. Testes `TmbC01.bmp` e `TmbC03.bmp`

Ambos mostraram:

```text
196662 bytes
256×256
24-bit
header proprietário:
41 38 3a 09 13 19 24 31 40 51 9a 79 90 a9
```

## 23.1 `TmbC03.bmp`

**[PROVADO EM RUNTIME — resultado negativo]**

Foi criada versão magenta válida, instalada e testada.

Resultado:

```text
nenhum personagem observado ficou magenta
```

Isso prova apenas que o asset não estava visivelmente ativo no conjunto de personagens/estado observado.

Não prova que nunca é usado.

## 23.2 `TmbC01.bmp`

Foram feitos testes de metade/quadrantes e observadas mudanças de regiões.

Entretanto a documentação deve permanecer conservadora sobre a associação exata de classe/equipamento até rastrear o carregamento por INX/INI e runtime.

---

# 24. Diagnóstico UV por cores

Método usado:

```text
pintar regiões da textura
→ observar quais polígonos recebem cada cor
```

Também foi usada uma cruz preta nas fronteiras.

Se aparece uma faixa preta entre duas cores, isso pode significar simplesmente:

```text
ilha UV atravessa a fronteira artificial
```

Não significa automaticamente UV quebrado.

Em textura final contínua essa borda não existe.

---

# 25. Repaint profissional

Depois de identificar atlas:

1. trabalhar no BMP decodificado;
2. manter UV existente;
3. usar bleed/padding;
4. manter resolução inicialmente;
5. exportar em BMP RGB24 compatível;
6. restaurar header;
7. testar;
8. restaurar se necessário.

Ferramentas:

```text
Krita
GIMP
Photoshop
Substance Painter
Blender Texture Paint
```

Camadas úteis:

```text
Base original
Material base
Metal
Tecido
Couro
Sombras baked
Highlights
Desgaste
Detalhes
Correção de cor
```

---

# 26. Renderer legado e PBR

O engine não ganha PBR só porque adicionamos:

```text
normal
roughness
metallic
AO separado
```

Para PBR real é necessário alterar renderer/shader.

Ainda assim, é possível melhorar muito o visual usando diffuse/albedo bem pintado:

- iluminação baked moderada;
- highlight de metal;
- textura de tecido/couro;
- edges;
- AO leve;
- variação de cor;
- alpha/overlay suportados.

Não exagerar na luz pintada porque o personagem passa por ambientes com luz diferente.

---

# 27. Resolução de texturas

256→512:

```text
4× mais pixels
```

256→1024:

```text
16× mais pixels
```

**[PENDENTE]**

Ainda não foi provado que as skins protegidas aceitam 512/1024.

Referência histórica relata 512 como limite em uma build antiga e 1024 problemático, mas isso não é especificação do Summit.

Ordem segura:

```text
256 repaint
→ provar
→ 512 isolado
→ medir estabilidade
→ só depois padronizar
```

---

# 28. TGA — problema de auditoria

A auditoria simples de headers retornou dimensões absurdas para alguns TGA, enquanto os tamanhos físicos parecem corresponder a imagens normais.

Exemplos de tamanhos:

```text
262188 ≈ 256×256×4 + overhead
131116 ≈ 128×256×4 + overhead
65580  ≈ 128×128×4 + overhead
32812  ≈ 64×128×4 + overhead
16428  ≈ 64×64×4 + overhead
1048620 ≈ 512×512×4 + overhead
```

Conclusão:

```text
não confiar no parser simplista de TGA
```

Pode haver header modificado/proteção.

Precisa de investigação equivalente ao BMP.

---

# 29. Blend/material modes

O renderer legado possui modos como:

```text
SMMAT_BLEND_ALPHA
SMMAT_BLEND_LAMP
SMMAT_BLEND_SHADOW
```

Ao redesenhar:

- preservar alpha;
- não converter TGA para BMP sem entender o uso;
- cuidado com preto/color-key;
- manter path/extensão inicialmente.

---

# 30. Head e body

Candidatos de head Pikeman encontrados:

```text
hpike_02.inx
hpike_02.smd
hpike_03.inx
hpike_03.smd
```

Família:

```text
Tmh-C01...
Tmh-C02...
Tmh-C03...
...
```

Variantes:

```text
a
b
c
d
t
```

Exemplos:

```text
Tmh-C01c_face.BMP
Tmh-C01c_cap.bmp
Tmh-C01c_tie.bmp
Tmh-C04c_face01.tga
Tmh-C04c_face02.tga
Tmh-C04c_face03.tga
Tmh-C04c_face04.tga
```

Não assumir `C01 = nível 1`.

Rastrear:

```text
equip/item
→ INX/INI
→ SMD
→ mesh
→ textura
```

---

# 31. Representações diferentes de item

Um equipamento pode ter pelo menos:

```text
1. modelo equipado
2. modelo/drop no chão
3. ícone de inventário
```

Histórico:

```text
char/TMABCD
→ personagem equipado

Image/Sinimage/Items/DropItem
→ item no chão

Image/Sinimage/Items/Defense
Image/Sinimage/Items/Accessory
→ ícones/inventário
```

Não confundir esses assets.

Para remaster completo de equipamento:

```text
equipped
+
drop
+
inventory
```

podem precisar ser refeitos separadamente.

---

# 32. Ricarten

**[CONFIRMADO LOCALMENTE]**

Existe diretório dedicado:

```text
Field/Ricarten/
```

Priorizar esse diretório antes de assumir que Ricarten usa só `Field/forest`.

Famílias encontradas:

```text
banner_*
beeranda*
eye_door*
eye_eaves
eye_flag
eye_light*
eye_railing*
eye_ring
eye_roofline
metal_s
na03_*
nn03_*
pl_02
pl_03
rin002
riy-f*
riy-w*
riy001
riy002
tent_cloth
top_pillar
treeani_*
treeA_*
treeB_*
tree_ob
Ttem
Tten
windows01
wind_*
worldcop_*
```

---

# 33. Ricarten possui ASEs

Exemplo:

```text
Field/Ricarten/world_ani06.ASE
```

Referências diretas encontradas:

```text
riy-w001.bmp
riy-f028.bmp
riy-w003.tga
riy-w004.bmp
...
riy-w054.bmp
riy-w055.bmp
riy-w056.bmp
riy-w057.bmp
riy-w058.bmp
riy-w059.bmp
riy-w060.bmp
riy-f011.tga
riy-w062.bmp
riy-w063.bmp
riy-w064.tga
riy-w065.tga
...
```

Mesmo com paths absolutos antigos no ASE, os basenames permitem mapear:

```text
mesh
→ material
→ textura
```

---

# 34. Terreno: textura vs geometria

Repaint pode esconder o grid visualmente:

- textura seamless;
- ruído orgânico;
- evitar linhas direcionais;
- variação de solo;
- transição suave;
- detalhes não alinhados ao tile.

Mas não muda:

- posição de vértices;
- silhueta;
- colinas;
- estradas;
- grid geométrico.

Para desquadrar de verdade:

```text
editar stage/terrain mesh/UV
```

---

# 35. `_mm1`, `_mm2`, `_mm3`

Foram encontrados grupos como:

```text
fo010.bmp
fo010_mm1.bmp
fo010_mm2.bmp
fo010_mm3.bmp
fo010_xm3.bmp
```

e em Ricarten:

```text
riy-f010.tga
riy-f010_mm1.tga
riy-f010_mm2.tga
```

Isso é forte evidência de níveis físicos reduzidos.

Ao remasterizar uma textura:

```text
não substituir só a base sem verificar _mm*
```

Caso contrário o visual distante pode voltar ao asset antigo.

Sem trace de runtime, ainda não marcar como 100% provado quando cada variante é usada.

---

# 36. Sky/background

O cliente possui subsistema em:

```text
SrcGame/src/pbackground.cpp
```

Trecho confirmado:

```cpp
CreateTextureMaterial(
    "StartImage//sky//RainSky.bmp",
    ...
);

AddTexture(
    BGMatNum,
    "StartImage//sky//fsky00.tga"
);

TextureFormState[1] = smTEXSTATE_FS_SCROLL;
MapOpacity = 0;
```

Isso mostra:

```text
background base
+
segunda camada scrolling
```

Diretório:

```text
StartImage/sky
```

é diferente de:

```text
Effect/Sky
```

## 36.1 Céu preto

Ainda não resolvido.

Teste de aliases/path/case não resolveu.

Cópia ampla de `Effect/Sky` de donor causou crash e foi revertida.

Regra:

```text
não copiar Effect/Sky inteiro novamente
```

---

# 37. Background color e z-clip

Em `pbackground.cpp`:

```cpp
smInitColorZClip(21, 17, 14);
```

Em outro ponto:

```cpp
smInitColorZClip(0, 0, 0);
// to change background of the maps to black again
```

O céu preto não foi provado como simples fog.

Não mudar a cor como “correção” definitiva sem rastrear Init/Main/Draw do background.

---

# 38. FarSight

A lógica existente foi comparada e contém:

```text
m_GameFieldViewStep = 22
SetGameFieldViewStep()
```

e dinâmica de `RendSightPos/RendSightOffset`.

Não mexer em FarSight para redesign sem evidência de bug.

---

# 39. Estratégia de redesign de Ricarten

## Fase 1

Texture-only:

- chão;
- pedra;
- madeira;
- paredes;
- telhados;
- vegetação;
- banners.

## Fase 2

Materiais/alpha:

- folhas;
- TGA;
- banners;
- overlays.

## Fase 3

ASE de props:

```text
world_ani*.ASE
```

para remodelar objetos.

## Fase 4

Stage/terreno:

```text
SMD Stage 0.72
SPC
editors legados
```

---

# 40. Armas e props

O cliente/source usa ASE diretamente em alguns conteúdos/armas.

Isso sugere que uma arma rígida pode ser um primeiro teste 3D mais simples que uma armadura animada.

Pipeline:

```text
modelo ativo
→ SMD/ASE
→ textura
→ remodelagem
→ drop
→ ícone
```

---

# 41. Limites do engine

Não temos especificação confiável de:

```text
máximo de vertices
máximo de faces
máximo de materiais
máximo de textura
```

Regra prática:

```text
think low-poly
```

Testar incrementalmente.

---

# 42. Efeitos/overlay

Referência histórica:

```text
duplicar GEOMOBJECT
→ material diferente
→ self illumination
→ TGA alpha
```

Pode servir para:

- brilho;
- reflexo;
- overlay;
- transparência.

Tratar como avançado.

---

# 43. Armadura totalmente nova

**[PENDENTE]**

É diferente de substituir uma aparência existente.

Pode envolver:

- item tables;
- cliente;
- servidor;
- lookup de modelo;
- INX/INI;
- icons;
- drops;
- shop/drop tables;
- stats.

A documentação atual cobre melhor edição/substituição do que registro de um novo item.

---

# 44. Scripts úteis

## 44.1 Descobrir texturas em SMD

```bash
SMD="arquivo.smd"

strings -a "$SMD" |
grep -Ei '\.(bmp|tga|dds|png)' |
sort -fu
```

Bones:

```bash
strings -a "$SMD" |
grep -Ei 'Bip|Pelvis|Spine|Head|UpperArm|Forearm|Hand|Thigh|Calf|Foot' |
sort -u
```

Header:

```bash
xxd -g1 -l 256 "$SMD"
```

## 44.2 Paths com espaços

Não usar:

```bash
for f in $(find ...)
```

Usar:

```bash
find "$DIR" -type f -print0 |
while IFS= read -r -d '' f
do
    ...
done
```

Exemplo:

```bash
find "$CLIENT/char/tmABCD"   -maxdepth 1   -type f   -iname 'TmbC*.smd'   -print0 |
sort -zV |
while IFS= read -r -d '' f
do
    echo "----- $(basename "$f") -----"

    strings -a "$f" |
    grep -Ei '\.(bmp|tga|dds|png)' |
    sort -fu
done
```

---

# 45. Auditoria ASE

```bash
ASE="arquivo.ASE"

grep -ac '^[[:space:]]*\*GEOMOBJECT' "$ASE"

grep -aE '\*MESH_NUMVERTEX' "$ASE"
grep -aE '\*MESH_NUMFACES' "$ASE"
grep -aE '\*MESH_NUMTVERTEX' "$ASE"
grep -aE '\*MESH_NUMTVFACES' "$ASE"

grep -ac '\*MESH_TVERT[[:space:]]' "$ASE"
grep -ac '\*MESH_TFACE[[:space:]]' "$ASE"

grep -aE '^[[:space:]]*\*NODE_NAME' "$ASE"

grep -aEin 'PHYSIQUE|SKIN|BONE_WEIGHT|VERTEX_WEIGHT' "$ASE" |
head -300

grep -aEi '\*BITMAP' "$ASE"
```

---

# 46. Script genérico `pt_bmp_decode.py`

```python
#!/usr/bin/env python3

from pathlib import Path
import struct
import sys

src = Path(sys.argv[1])
out = Path(sys.argv[2]) if len(sys.argv) >= 3 else src.with_suffix(".decoded.bmp")

b = bytearray(src.read_bytes())

if len(b) < 54:
    raise SystemExit("arquivo pequeno demais")

dib = int.from_bytes(b[14:18], "little")
w = int.from_bytes(b[18:22], "little", signed=True)
h = int.from_bytes(b[22:26], "little", signed=True)
planes = int.from_bytes(b[26:28], "little")
bpp = int.from_bytes(b[28:30], "little")

print("custom header:", b[:14].hex(" "))
print("DIB:", dib)
print("width:", w)
print("height:", h)
print("planes:", planes)
print("bpp:", bpp)
print("size:", len(b))

if dib != 40:
    raise SystemExit("DIB diferente de BITMAPINFOHEADER 40")

# Validado no caso RGB24 256x256.
pixel_offset = 54

b[:14] = struct.pack(
    "<2sIHHI",
    b"BM",
    len(b),
    0,
    0,
    pixel_offset
)

out.write_bytes(b)
print("criado:", out)
```

---

# 47. Script genérico `pt_bmp_encode.py`

```python
#!/usr/bin/env python3

from pathlib import Path
import sys

original = Path(sys.argv[1])
edited = Path(sys.argv[2])
output = Path(sys.argv[3])

orig = original.read_bytes()
edit = bytearray(edited.read_bytes())

if len(orig) != len(edit):
    raise SystemExit(
        f"tamanho mudou: original={len(orig)}, edited={len(edit)}"
    )

if edit[:2] != b"BM":
    raise SystemExit("arquivo editado não é BMP normal")

edit[:14] = orig[:14]

output.write_bytes(edit)

print("header:", edit[:14].hex(" "))
print("size:", len(edit))
print("criado:", output)
```

---

# 48. Estrutura de trabalho recomendada

```text
design_work/
└── asset/
    ├── original/
    ├── decoded/
    ├── working/
    ├── export/
    ├── game/
    └── notes.md
```

Exemplo:

```text
design_work/
└── priestess_mfbc01/
    ├── original/MfbC01.bmp
    ├── decoded/MfbC01.decoded.bmp
    ├── working/MfbC01.kra
    ├── export/MfbC01.export.bmp
    ├── game/MfbC01.game.bmp
    └── notes.md
```

---

# 49. Backup e SHA obrigatório

```bash
cp -a "$ASSET" "$ASSET.before_design_test"

sha256sum  "$ASSET"  "$ASSET.before_design_test"
```

Restore:

```bash
cp -a  "$ASSET.before_design_test"  "$ASSET"
```

Um asset por teste.

---

# 50. Protocolo de teste de textura

```text
A. decode
B. file/identify
C. alteração óbvia
D. encode
E. instalar um asset
F. abrir cliente
G. observar
H. restore
I. documentar
J. só então repaint real
```

---

# 51. Protocolo de teste 3D

```text
1. localizar INX/INI
2. localizar SMD/ASE
3. descobrir NODE_NAME
4. backup
5. importar ASE funcional
6. modelar peça simples
7. UV
8. material simples
9. posicionar
10. Physique/rig
11. testar pose
12. Export Selected
13. integrar no ASE funcional
14. converter/carregar
15. testar no cliente
```

Primeira peça recomendada:

```text
ombreira simples
```

---

# 52. Teste de animação

Ao alterar mesh riggada, testar:

```text
Idle
Walk
Run
Attack
Skill
Death
```

Observar:

```text
posição
escala
rotação
UV
textura
smoothing
normals
rigging
stretching
seams
z-fighting
```

---

# 53. Troubleshooting

## Textura branca

Verificar:

```text
arquivo existe?
nome?
extensão?
path?
header?
ASE aponta BMP/TGA correto?
MATERIAL_REF?
MESH_MTLID?
```

## Textura embaralhada

Verificar:

```text
MESH_MTLID
MESH_TVERTLIST
MESH_TFACELIST
Mapping Coordinates
UV
```

## Mesh invisível

Verificar:

```text
INX/INI espera esse NODE_NAME?
NODE_NAME correto?
ASE correto?
GEOMOBJECT?
MATERIAL_REF?
MESH_MTLID?
```

## Crash

Verificar:

```text
indices de vertex/face
TVERT/TFACE
PHYSIQUE_NUMVERTEXASSIGNMENT
bone inexistente
contadores de material
SUBMATERIAL
chaves { }
hierarquia
```

## Stretching

Verificar:

```text
todos os vertices têm assignment?
bone correto?
No Blending esperado?
transform?
bind pose?
```

## Seam

Possíveis causas:

```text
UV perto de cores diferentes
falta de bleed
filtering
mipmap
_mm* antigo
UV na borda
```

Usar padding/bleed fora da ilha.

---

# 54. Comparar headers de texturas

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

find "$CLIENT"   -type f   \( -iname '*.bmp' -o -iname '*.tga' \)   -print0 |
while IFS= read -r -d '' f
do
    size=$(stat -c '%s' "$f")
    hdr=$(xxd -p -l 18 "$f" | tr -d '\n')

    printf '%s\t%s\t%s\n'       "$size"       "$hdr"       "$f"
done > "/home/gabriel/Área de Trabalho/summit/texture_header_inventory.tsv"
```

Objetivo:

```text
comparar 64/128/256/512
→ inferir regra do header proprietário
```

---

# 55. Ricarten: auditoria rápida

```bash
ASE="$CLIENT/Field/Ricarten/world_ani06.ASE"

grep -aEin '\*BITMAP|\*NODE_NAME|\*MATERIAL_NAME' "$ASE"
```

Somente `riy-`:

```bash
grep -aEin '\*BITMAP.*riy-' "$ASE"
```

Depois procurar basename em todo `Field`.

---

# 56. Estado atual em matriz

| Área | Estado |
|---|---|
| SMD é proprietário, header 0.62 | CONFIRMADO |
| ASE real existe | CONFIRMADO |
| ASE possui UV + Physique | CONFIRMADO |
| `MfbC13.ASE` tem 67 GEOMOBJECTs | CONFIRMADO |
| `MfbC13.ASE` representa fielmente C13 | NÃO CONFIRMADO |
| BMP 256×256/24 pode ser decodificado trocando 14 bytes | PROVADO |
| Re-encode com header original no mesmo tamanho | PROVADO |
| `MfbC01.bmp` altera Sacerdotisa | PROVADO EM RUNTIME |
| Restore de `MfbC01.bmp` | PROVADO |
| `TmbC03.bmp` alterado não apareceu nos chars testados | PROVADO EM RUNTIME |
| TGA usa mesmo esquema do BMP | NÃO CONFIRMADO |
| 512×512 funciona em skin protegida | NÃO CONFIRMADO |
| Blender importer/exporter completo | NÃO IMPLEMENTADO |
| Physique rigid → vertex groups Blender | VIÁVEL, NÃO PROVADO END-TO-END |
| SMDReader funciona | NÃO TESTADO; runtime Debug ausente |
| Ricarten possui ASEs | CONFIRMADO |
| Ricarten possui texturas próprias | CONFIRMADO |
| `_mm*` são níveis físicos usados | FORTE EVIDÊNCIA; precisa trace |
| Editor Japaokiko converte mapas | HIPÓTESE |
| Céu preto é só asset faltando | NÃO PROVADO |

---

# 57. Próximas prioridades

## 1. Ferramentas permanentes de textura

Criar e versionar:

```text
pt_bmp_decode.py
pt_bmp_encode.py
pt_texture_probe.py
```

## 2. Primeiro repaint real

O caminho de menor risco é:

```text
Sacerdotisa / MfbC01.bmp
```

porque decode, runtime e restore já funcionaram.

## 3. Descobrir header para novas resoluções

Comparar:

```text
64×64
128×128
256×256
512×512
```

## 4. TGA

Mapear:

```text
header
dimensões
alpha
encode/decode
```

## 5. Blender ASE importer

Começar só por geometry/UV.

## 6. Physique

Adicionar hierarchy/vertex groups.

## 7. Ricarten

Gerar tabela:

```text
ASE
→ material
→ bitmap
→ resolução
→ _mm variants
```

## 8. Stage/map

Investigar:

```text
SMD Stage 0.72
SPC
Editor Japaokiko
Level Editor
```

---

# 58. Ordem recomendada para importer Blender

```text
Etapa 1:
MESH_VERTEX + MESH_FACE

Etapa 2:
TVERT + TFACE

Etapa 3:
materials

Etapa 4:
NODE hierarchy

Etapa 5:
Physique rigid → vertex groups

Etapa 6:
export ASE

Etapa 7:
comparar original/reexport

Etapa 8:
testar uma única peça no cliente
```

Validar por contagem, não só pelo viewport:

```text
vertices
faces
UV
NODE_NAME
orientação
bone assignments
```

---

# 59. Estratégia para parser SMD

Antes de escrever exporter:

```text
1. localizar structs do loader no source
2. mapear header
3. mapear contagens
4. mapear offsets
5. parser read-only
6. comparar com strings
7. validar mesh conhecida
8. só então serializer
```

No dump de `MfbC01.smd`, após o header aparecem campos binários como:

```text
0f 00 00 00
06 00 00 00
84 04 00 00
ed 0c 00 00
```

O significado ainda não foi estabelecido. Não chamar esses campos de `numMeshes`, `numMaterials` etc. sem source.

Há também regiões `cd cd cd...`. `0xCD` é associado historicamente a memória Debug do MSVC, mas no arquivo isso continua sendo apenas uma pista; pode ser padding serializado.

---

# 60. Filosofia de trabalho

O método que melhor funcionou no Summit:

```text
auditar
→ mudar uma coisa
→ testar
→ observar
→ restaurar
→ documentar
```

Evitar:

```text
copiar centenas de assets
alterar vários sistemas
testar tudo de uma vez
```

Para design:

```text
uma textura por vez
uma mesh por vez
um material por vez
```

---

# 61. Pipeline consolidado de personagem

```text
ITEM / EQUIP
    ↓
INX / INI
    ↓
modelo + NODE_NAME
    ↓
SMD / ASE
    ↓
Mesh + UV + material + bones
    ↓
Texture
    ↓
decode
    ↓
Krita / Blender / Substance
    ↓
export
    ↓
encode/header
    ↓
CLIENT
    ↓
Idle / Walk / Run / Attack / Skill / Death
```

---

# 62. Pipeline consolidado de remodelagem

```text
INX/INI
→ ASE funcional
→ NODE_NAME + material + bones
→ Blender / 3ds Max
→ modelar low-poly
→ UV
→ textura
→ rig / Physique
→ Export Selected
→ integrar no ASE funcional
→ preservar MATERIAL_REF / MESH_MTLID
→ preservar PHYSIQUE
→ pipeline PT
→ cliente
```

---

# 63. Pipeline consolidado de Ricarten

```text
Field/Ricarten
→ ASE world_ani*
→ NODE_NAME
→ MATERIAL
→ BITMAP riy-*
→ base + _mm*
→ repaint
→ teste
```

Depois:

```text
ASE
→ remodelagem de props
```

Por último:

```text
stage SMD/SPC
→ geometria/terreno
```

---

# 64. O que NÃO fazer

```text
não sobrescrever única cópia
não trabalhar sem SHA
não copiar donor inteiro
não assumir que SMD é Valve SMD
não assumir C01 = level 1
não assumir string no SMD = material ativo
não assumir TGA padrão
não testar 1024 antes de 512
não recriar skeleton do zero sem necessidade
não renomear mesh arbitrariamente
não remover Physique
não perder TVERT/TFACE
não ignorar _mm*
não adicionar PBR esperando suporte automático
```

---

# 65. Primeiro objetivo de produção recomendado

## Textura

```text
Sacerdotisa / MfbC01.bmp
```

Criar:

```text
MfbC01_vanilla.decoded.bmp
MfbC01_remaster.kra
MfbC01_remaster_export.bmp
MfbC01_remaster_game.bmp
```

Manter 256×256 inicialmente.

## 3D

```text
importar ASE funcional
→ uma ombreira simples
→ preservar UV/material
→ vertex group rigid
→ exportar
→ comparar ASE
```

## Ambiente

Selecionar 3–5 texturas realmente referenciadas por:

```text
Field/Ricarten/world_ani06.ASE
```

e fazer repaint com `_mm*` correspondente quando aplicável.

---

# 66. Fonte histórica consolidada fornecida

O documento técnico fornecido durante a investigação, “Priston Tale 3D — Source Única de Referência”, reforça:

- INX/INI como referência de configuração;
- SMD como modelo de runtime;
- SMB associado a movimento;
- ASE como representação ASCII/editável;
- importância de NODE_NAME;
- Physique/No Blending;
- Mapping Coordinates;
- MATERIAL_REF vs MESH_MTLID;
- submateriais;
- exportação por peça;
- paths de textura;
- separação equipped/drop/inventory;
- troubleshooting de UV/rig/material;
- uso de ASE funcional como base.

Esse material é valioso, mas convenções históricas devem ser cruzadas com o Summit.

---

# 67. Correções de entendimento feitas durante a investigação

## SMD

Antes poderia ser confundido com Valve SMD.

Agora:

```text
PROVADO:
SMD Model data Ver 0.62
é proprietário do Priston
```

## `MfbC13.ASE`

Antes parecia possivelmente só rig.

Agora:

```text
PROVADO:
meshes + UV + Physique
```

## BMP

Antes parecia “arquivo quebrado”.

Agora:

```text
PROVADO:
14 bytes proprietários
+
DIB BMP válido
```

## `MfbC01.bmp`

Agora:

```text
PROVADO EM RUNTIME:
Sacerdotisa
```

## TGA

As dimensões absurdas da auditoria não são confiáveis até entender o header.

---

# 68. Checklist antes de editar textura

```text
[ ] asset ativo confirmado?
[ ] backup?
[ ] SHA256?
[ ] extensão real?
[ ] header?
[ ] resolução?
[ ] bpp?
[ ] alpha?
[ ] _mm1/_mm2?
[ ] drop/icon separado?
[ ] uma mudança por teste?
[ ] restore pronto?
```

---

# 69. Checklist antes de editar ASE

```text
[ ] INX/INI identificado?
[ ] NODE_NAME esperado?
[ ] ASE funcional salvo?
[ ] MATERIAL_REF entendido?
[ ] MESH_MTLID entendido?
[ ] UV preservado?
[ ] Physique preservado?
[ ] todos os vertices atribuídos?
[ ] contadores atualizados?
[ ] paths compatíveis?
[ ] teste isolado?
```

---

# 70. Estado desta documentação

Tratar este arquivo como:

```text
SUMMIT TALE DESIGN / 3D SOURCE OF TRUTH v1
2026-08-23
```

Áreas com evidência local forte:

```text
SMD 0.62
ASE
Physique
UV
BMP custom header
repaint da Priestess
Ricarten ASE
inventário de assets
inventário de ferramentas
```

Áreas abertas:

```text
TGA codec
SMD serializer
SMB parser
INX parser completo
Blender importer/exporter
512/1024 texture support
stage SMD 0.72
SPC
Editor Japaokiko
pipeline moderno 3ds Max
céu/background preto
```

Atualizar sempre que um item passar de:

```text
HIPÓTESE
```

para:

```text
CONFIRMADO
```

ou:

```text
PROVADO EM RUNTIME
```

---

# 71. Regra final

Para textura:

```text
asset original
+
decode correto
+
repaint
+
encode/header correto
+
mesma estrutura primeiro
```

Para 3D:

```text
ASE funcional
+
INX/INI
+
NODE_NAME correto
+
mesh/UV
+
Physique
+
material compatível
```

Para mapa:

```text
ASE/material/textura primeiro
+
stage/SPC depois
```

A estratégia é modernizar o visual mantendo o máximo possível da estrutura que o engine já reconhece.
