https://priston.pro/old/comunidade/service/customizando-modelos-3d-heads/
https://forum.ragezone.com/threads/notes-on-armors-armors-101.789886/

Sim. E no contexto do teu Priston, o que estás a descrever é exatamente **texturização por UV**.

O modelo 3D contém a geometria — cabeça, braços, roupa, arma etc. — e também coordenadas UV que dizem algo como:

> “esta parte da cabeça usa este pedaço da imagem 2D; esta parte da camisa usa aquele pedaço.”

A imagem 2D é a **texture map**. No teu cliente ela aparece principalmente como `.tga` e `.bmp`, e também há `.dds`/`.png` em sistemas mais novos. Por exemplo, o teu `char/tmABCD` contém muitos `.inx` e texturas de personagem como `Ava_M_01.tga`, `Ava_W_01.tga` etc. 

Então sim: **consegues redesenhar rosto, pele, roupa, armadura, monstros, armas etc. sem necessariamente alterar o modelo 3D.**

## Como funciona

Imagina que tens este personagem:

```text
MODELO 3D
       ↓
   corpo/cabeça
       ↓
 coordenadas UV
       ↓
┌─────────────────────┐
│ rosto │ cabelo      │
│───────┼──────────────│
│ peito │ braços      │
│───────┼──────────────│
│ pernas│ botas       │
└─────────────────────┘
       TGA
```

Essa TGA parece muitas vezes estranha quando aberta normalmente: rosto desmontado, braços separados, roupa achatada etc.

Quando o motor desenha o modelo, ele “embrulha” essa imagem no mesh.

É exatamente o sistema que estás a imaginar.

---

# No teu Priston eu separaria os assets em 3 níveis

### 1. Imagens puramente 2D

São as mais fáceis de modernizar.

Exemplos:

```text
image/
game/
StartImage/
```

e formatos:

```text
.tga
.bmp
.png
.dds
```

Aqui entram:

* interface;
* botões;
* barras;
* inventário;
* chat;
* ícones de skill;
* buffs;
* cursores;
* loading screens;
* imagens de quests;
* background de menus;
* efeitos 2D;
* partículas.

Já vimos diretamente no teu source coisas como:

```cpp
CreateTextureMaterial(
    "game//target//targetoverlay.tga",
    ...
);
```

e vários outros elementos da UI carregados dessa maneira. 

Nesse caso o pipeline é extremamente simples:

```text
TGA original
    ↓
Photoshop / GIMP / Krita
    ↓
redesign
    ↓
exporta novamente
    ↓
mesmo nome/path
    ↓
abre o jogo
```

Nem precisa recompilar o cliente se mantiveres o mesmo arquivo e path.

---

# 2. “Pintar” novamente personagens, monstros e equipamentos

É exatamente a parte que perguntaste.

Os monstros do teu cliente demonstram muito bem a estrutura. Um único monstro frequentemente possui:

```text
Billy.inx
Billy.smd
billy-ani.smb
Billy.tga
weapon1.TGA
```



Outros possuem várias texturas:

```text
Bknight.inx
Bknight.smb
Bknight.smd
Bknight01_yun.tga
Bknight02_yun.tga
Bwea_yun.tga
```



Esse é o tipo de asset no qual podes fazer um **remaster visual gigantesco sem tocar inicialmente no modelo**.

Por exemplo, pega:

```text
Bknight01_yun.tga
```

Abres no editor e redesenhas:

* tecido;
* metal;
* pele;
* detalhes;
* sujeira;
* olhos;
* couro;
* arranhões;
* ornamentos;
* sombras pintadas;
* highlights.

Mantendo o UV original.

O modelo 3D continua exatamente o mesmo.

---

## Podes aumentar a resolução também

Suponhamos que uma textura antiga seja:

```text
256 × 256
```

Podes experimentar:

```text
512 × 512
1024 × 1024
```

sem modificar o UV.

UV normalmente trabalha com coordenadas normalizadas:

```text
0.0 → 1.0
```

e não “pixel 103 da textura”.

Então:

```text
256x256
512x512
1024x1024
```

podem utilizar o mesmo mapeamento.

**Mas no teu cliente eu faria isso progressivamente.**

Já descobrimos que esse motor tem muitos loaders antigos e algumas incompatibilidades. Portanto:

```text
256 → 512
teste

512 → 1024
teste
```

em vez de converter 3.000 texturas de uma vez.

---

# Uma coisa muito importante sobre os teus `.smd`

Não uses um tutorial qualquer de “SMD Blender” achando que é o mesmo formato do Source/Valve.

O teu SMD é **um formato próprio deste engine**.

O próprio código contém o header:

```text
SMD Model data Ver 0.62
```



Portanto:

**Priston `.smd` ≠ Valve/Source `.smd`.**

Isso é importantíssimo para o teu projeto de design.

---

# O formato interessante para criação de modelos é `.ASE`

Esse é provavelmente o caminho que vamos explorar quando quiseres ir além de repaint.

O teu engine já contém:

```cpp
smASE_Read(...)
smASE_ReadBone(...)
```



e o jogo carrega diretamente vários modelos `.ASE`, por exemplo armas:

```cpp
smASE_Read("weapons//Nt3_DivineI.ASE");
smASE_Read("weapons//Godly_Shield.ASE");
smASE_Read("weapons//crown.ASE");
```



Isso é excelente para nós.

`ASE` é um formato de cena/modelo ASCII historicamente ligado ao 3ds Max.

Então temos aproximadamente:

```text
software 3D
   ↓
mesh
UV
materiais
bones/animação (dependendo do pipeline)
   ↓
ASE
   ↓
Priston loader/conversor
   ↓
SMD / SMB / runtime
```

A parte exata de conversão do teu projeto ainda vale mapearmos, mas o source prova que o motor possui infraestrutura própria para ASE e SMD. 

---

# O que cada formato significa para o teu trabalho

Eu trataria assim, por enquanto:

| Formato     | Papel no projeto                            | Designer deve editar?           |
| ----------- | ------------------------------------------- | ------------------------------- |
| `.TGA`      | textura, UI, sprites, transparência         | **Sim, muito**                  |
| `.BMP`      | textura/background antigo                   | **Sim**                         |
| `.PNG`      | UI mais recente                             | **Sim**                         |
| `.DDS`      | efeitos/texturas mais recentes              | **Sim**, com cuidado            |
| `.ASE`      | modelo 3D em formato de autoria/intercâmbio | **Sim**, quando entrarmos em 3D |
| `.SMD`      | modelo compilado/proprietário do Priston    | normalmente **não diretamente** |
| `.SMB`      | dados 3D/bone/animação associados           | normalmente **não diretamente** |
| `.INX`      | metadata/indexação de modelo/animação       | não para simples repaint        |
| `.INI/.TXT` | configuração/referências                    | às vezes                        |

O teu próprio inventário reforça a associação: diversos monstros aparecem com um conjunto `.inx + .smb + .smd + textura`. 

---

# Para começar o teu projeto de design, eu faria assim

Não começaria pelos modelos 3D.

Começaria com um **vertical slice visual**.

Escolhe:

* 1 personagem;
* 1 armadura;
* 1 arma;
* 2 ou 3 monstros;
* alguns elementos de UI;
* um mapa inicial.

E modernizamos tudo apenas via textura.

Por exemplo:

```text
ANTES
256×256 TGA
pixelada
pouca definição
sombras pintadas rudimentares

        ↓

NOVO
512/1024 TGA
mesmo UV
mais detalhe
tecidos melhores
pele melhor
metal mais legível
paleta visual consistente
```

Isso permite avaliar quanto o engine aguenta **antes de entrarmos em modificação estrutural 3D**.

---

# Ferramentas que eu usaria

Para texturas:

**Krita** ou **GIMP** já seriam suficientes e são ótimos no Linux.

Se trabalhares com Adobe:

**Photoshop**.

Para upscaling/base:

podes usar ferramentas de upscale, mas eu não faria simplesmente “AI upscale e pronto”. Para um redesign bom:

```text
original
 ↓
upscale como referência
 ↓
repaint manual
 ↓
correção de materiais
 ↓
detalhes
 ↓
teste in-game
```

É muito melhor.

---

# E para pintar diretamente sobre o personagem 3D?

Aí entramos numa etapa ainda mais interessante.

O workflow moderno seria:

```text
modelo
 ↓
UV
 ↓
Blender / Substance Painter
 ↓
pintas diretamente no boneco 3D
 ↓
software atualiza a textura 2D
 ↓
exporta TGA
 ↓
Priston carrega a TGA
```

No Substance Painter, por exemplo, em vez de olhar aquela TGA toda desmontada, pintarias diretamente:

```text
[personagem 3D]

pincel → braço
pincel → peito
pincel → rosto
```

e o software sabe onde isso fica na UV.

**É exatamente o workflow que estás a imaginar.**

A dificuldade específica do nosso caso será importar o mesh proprietário do Priston para Blender/Substance.

Se tivermos o `.ASE` correspondente, isso fica muito mais fácil.

Se só tivermos `.SMD/.SMB`, teremos que:

```text
entender formato SMD
        ou
usar o loader existente do source
        ↓
criar/exportar uma conversão
        ↓
OBJ/FBX/GLTF
```

Mas como temos **o source do próprio loader**, isso é perfeitamente atacável tecnicamente.

---

# Há uma limitação importante do visual atual

Melhorar uma textura não transforma automaticamente o renderer num renderer moderno PBR.

O Priston atual trabalha muito com materiais/texturas e modos como:

```cpp
SMMAT_BLEND_ALPHA
SMMAT_BLEND_LAMP
SMMAT_BLEND_SHADOW
```

por exemplo. 

Então inicialmente podemos melhorar brutalmente:

* resolução;
* desenho;
* materiais “fake” pintados;
* transparência;
* paleta;
* contraste;
* legibilidade;
* detalhes.

Mas coisas modernas como:

```text
normal map
roughness
metallic
ambient occlusion real
PBR
parallax
```

não podem ser simplesmente adicionadas como imagens e esperar que o motor as entenda.

Para isso precisaríamos fazer uma **segunda fase no renderer/shaders**.

E como tens o source, isso também é possível mais tarde.

---

## Eu dividiria o projeto gráfico em três fases

**Fase 1 — Remaster sem mudar o engine**

```text
UI
ícones
TGA/BMP/PNG
skins dos personagens
armaduras
monstros
armas
céu
partículas
loading screens
```

Risco baixo e retorno visual muito grande.

**Fase 2 — Pipeline 3D**

```text
ASE ↔ Blender/3ds Max
UV
novos meshes
novas armaduras
novos monstros
novas armas
novos NPCs
animações
```

Aqui descobrimos/construímos o conversor adequado para SMD/SMB/INX.

**Fase 3 — Renderer moderno**

```text
normal maps
specular/roughness
iluminação melhor
sombras
fog
sky
shaders
anti-aliasing
post processing
```

Sim. E apareceu um ponto importante para o teu plano: **eu não começaria reverse engineering do `.smd/.smb/.inx` do zero ainda**. Há indícios de que já existem parsers/conversores na comunidade, e o teu próprio source contém parte suficiente do formato para construirmos um pipeline se necessário.

O projeto JPsTale, por exemplo, declara loaders para `.ase`, `.inx`, `.smd` e `.smb`, incluindo mesh, skeleton e animações. Isso pode nos poupar bastante trabalho, embora ainda precisemos verificar compatibilidade com a versão exata dos arquivos do Summit. ([GitHub][1]) A documentação comunitária também bate com o que encontramos no teu source: `.smd` é o modelo compilado, `.smb` guarda animação/bones, `.inx` descreve/indexa modelo/animações e `.ase` é a representação aberta usada no pipeline de criação. ([priston.pro][2])

[JPsTale no GitHub](https://github.com/smileclound/JPsTale?utm_source=chatgpt.com)

## 1. Antes do redesign: detalhes técnicos que eu preservaria

Ao substituir uma textura, mantém inicialmente:

* **mesmo nome e mesmo path**;
* **mesma extensão**;
* **mesma proporção** da imagem;
* canal alpha quando existir;
* orientação da TGA;
* material ao qual ela está associada;
* quantidade de texturas por material.

No teu engine isso importa bastante porque ele é um renderer legado com vários modos próprios de blend. Já vimos no source `SMMAT_BLEND_ALPHA`, `LAMP`, `SHADOW` etc.  Há inclusive comportamento legado de transparência/color-key e fixed-function que pode fazer uma textura aparentemente normal se comportar diferente de um engine moderno. ([RaGEZONE - MMO Development Forums][3])

Para o primeiro teste eu usaria:

```text
original: 256x256
rework:   512x512
```

e não 2048/4096 de uma vez.

Uma textura 1024×1024 tem **16× a quantidade de pixels** de uma 256×256. Em centenas de assets isso vira pressão de VRAM e tempo de carregamento muito rapidamente.

Também evita alterar o UV inicialmente. O objetivo da primeira fase é:

```text
mesmo mesh
mesmo UV
mesmo material
mesmo arquivo lógico
nova pintura
```

Assim qualquer defeito é fácil de atribuir à textura.

---

# 2. Pikeman é um excelente primeiro personagem

Para armaduras dos personagens, a pasta principal é:

```text
char/tmABCD
```

A convenção conhecida e que coincide com esses assets é:

```text
A  Mechanician
B  Fighter
C  Pikeman
D  Archer
E  Assassin

mA Knight
mB Atalanta
mC Priestess
mD Magician
...
```

Então para o **Pikeman**, o alvo principal é a família **C**. A documentação comunitária inclusive usa `TmbC14_p.ase` como exemplo de armadura de Pikeman. ([priston.pro][2])

O teu próprio inventário já confirmou a estrutura dessa pasta: existem muitos `.inx` e texturas TGA associados às roupas/personagens. 

Eu não assumiria ainda que `C001` é necessariamente “a armadura nível 1”. Primeiro vamos relacionar:

```text
item do inventário
       ↓
armor code
       ↓
Cxxx.inx
       ↓
SMD/ASE
       ↓
TGA/BMP
```

Isso evita redesenhar a textura errada.

---

# 3. Pikeman low-level: assets que precisamos localizar

A relação completa do primeiro vertical slice seria aproximadamente:

| Elemento                 | Onde procurar                                                         |
| ------------------------ | --------------------------------------------------------------------- |
| Corpo/armadura equipada  | `char/tmABCD/`                                                        |
| Configuração da armadura | `char/tmABCD/*.inx`, eventualmente `.ini`                             |
| Mesh da armadura         | `char/tmABCD/*.smd` ou `.ase`                                         |
| Bones/animação           | `char/tmABCD/*.smb`                                                   |
| Skin/textura da armadura | `char/tmABCD/*.tga`, `*.bmp`                                          |
| Ícone da armadura        | `image/Sinimage/Items/Defense/`                                       |
| Armadura no chão         | `image/Sinimage/Items/DropItem/`                                      |
| Mesh do item no chão     | `image/Sinimage/Items/DropItem/*.ase` ou equivalente                  |
| Arma equipada            | provavelmente `weapons/` + referências de item                        |
| Ícone da arma            | `image/Sinimage/Items/`                                               |
| Arma caída no chão       | `image/Sinimage/Items/DropItem/`                                      |
| Efeitos da arma          | `Effect/`, `image/Sinimage/Effect/`, `AssaEffect/` dependendo da arma |

A estrutura `Defense/DropItem` é também documentada no pipeline antigo do PT: armaduras têm modelo usado pelo personagem em `TMABCD`, representação caída e ícone de inventário separados. ([priston.pro][2])

Isso significa uma coisa importante para design:

**redesenhar só a TGA do personagem não altera automaticamente o ícone nem o item no chão.**

Para um rework completo de uma armadura teremos possivelmente **três representações visuais**:

```text
1. equipada no personagem
2. caída no chão
3. ícone no inventário
```

---

# 4. E a arma low-level do Pike?

A arma é um ótimo segundo alvo, mas primeiro temos que descobrir exatamente qual item o jogo entrega/vende como primeiro Pike.

O source já prova que o cliente consegue carregar diretamente modelos `.ASE` de `weapons/`, por exemplo:

```cpp
smASE_Read("weapons//Nt3_DivineI.ASE");
smASE_Read("weapons//Godly_Shield.ASE");
smASE_Read("weapons//crown.ASE");
```



Então `weapons/` merece uma auditoria própria.

---

# 5. Sobre Blender: talvez não precisemos reinventar o formato

Eu vejo quatro caminhos, nesta ordem:

```text
A. Encontrar ferramentas antigas que já existem no teu pacote
       ↓
B. Usar ASE como formato intermediário
       ↓
C. Aproveitar parsers existentes da comunidade
       ↓
D. Reverse-engineer SMD/SMB/INX nós mesmos
```

**ASE é especialmente interessante.**

O teu source possui:

```cpp
smASE_Read(...)
smASE_ReadBone(...)
```



e também possui formato SMD próprio identificado pelo header:

```text
SMD Model data Ver 0.62
```



Então sabemos que não é Valve SMD.

Mas o caminho pode ser:

```text
Priston SMD
     ↓
nosso converter
     ↓
OBJ/glTF/FBX
     ↓
Blender
     ↓
ASE
     ↓
Priston
```

ou melhor ainda, se encontrarmos `.ASE` original:

```text
ASE
 ↓
Blender / 3ds Max
 ↓
ASE
 ↓
loader do próprio Priston
```

Há ferramentas modernas da comunidade voltadas exatamente para import/export ASE de PT. ([priston.pro][2])

---

# 6. Podemos fazer nosso próprio Blender importer/exporter

E isso é perfeitamente viável porque temos uma vantagem incomum:

**temos o código do loader do jogo.**

Não precisaríamos fazer reverse engineering 100% black-box.

O source contém estruturas e funções que:

```text
leem ASE
leem SMD
salvam/carregam SMD
associam materiais
associam bones
associam motion data
```

O header do formato e as rotinas de leitura estão no próprio `smObj3d.cpp/smRead3d.cpp`. 

Uma estratégia futura muito boa seria escrever:

```text
priston_import.py
```

para Blender.

Ele poderia importar:

```text
SMD
SMB
INX
```

e criar no Blender:

```text
mesh
UV
materials
armature
animations
```

Depois um exporter faria o caminho inverso.

Mas antes disso vale muito auditar as ferramentas que já tens.

---

# 7. Ricarten talvez seja um projeto ainda melhor visualmente

Para portfolio/design, eu gosto bastante da ideia de fazer **Ricarten primeiro ou em paralelo ao Pikeman**.

Porque com uma única cidade consegues mostrar:

```text
ANTES → DEPOIS
```

em:

* chão;
* pedras;
* madeira;
* paredes;
* telhados;
* árvores;
* placas;
* céu;
* água;
* props;
* UI;
* iluminação percebida.

O cliente possui uma árvore grande de mapas em:

```text
Field/
```

incluindo diretórios como:

```text
Field/forest
Field/map
Field/castle
Field/cave
Field/desert
Field/dungeon
Field/Ice
Field/Iron
Field/Lost
...
```



Precisamos descobrir qual arquivo corresponde exatamente a Ricarten nessa variante.

---

# 8. Trocar texturas consegue fazer o terreno parecer menos quadrado?

**Sim visualmente; não geometricamente.**

Essa distinção é muito importante.

Imagine o terreno atual:

```text
+----+----+----+
|    |    |    |
| A  | B  | C  |
|    |    |    |
+----+----+----+
|    |    |    |
| D  | E  | F  |
+----+----+----+
```

Se a geometria realmente é uma grade, trocar a textura não muda:

```text
posição dos vértices
polígonos
altura do terreno
silhueta
```

Mas dá para esconder muito do aspecto quadrado através da arte.

Uma textura ruim faz:

```text
grama | terra
------|------
terra | grama
```

e evidencia os tiles.

Uma textura melhor usa:

```text
formas orgânicas
ruído irregular
pedras cruzando visualmente direções
variação de cor
detalhes não alinhados
seams invisíveis
```

e o olho deixa de perceber tanto a grade.

## O que uma nova textura consegue melhorar

Muito:

```text
repetição visual
pixelização
quadrados evidentes
transições bruscas
cor artificial
falta de detalhe
material chapado
```

## O que exige mexer no mapa/modelo

Para realmente mudar:

```text
quinas do terreno
colinas
silhueta
densidade de polígonos
formato das ruas
curvas
bordas
```

precisamos alterar o **stage/mesh**, não só a textura.

O próprio engine distingue SMD de modelo e SMD de mapa; no source existe inclusive:

```text
SMD Stage data Ver 0.72
```

para stages. 

Isso futuramente abre a possibilidade de um **editor/conversor de mapas para Blender** também.

---

# 9. Auditoria Linux — roda este bloco inteiro

Ele não modifica nenhum arquivo.

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
SERVER="/home/gabriel/Área de Trabalho/summit/Servidor"
SRC="/home/gabriel/Área de Trabalho/summit/SourceCode"

OUT="$SRC/design_asset_audit"
mkdir -p "$OUT"

echo "============================================================"
echo "1. PIKEMAN / TMABCD"
echo "============================================================"

find "$CLIENT/char/tmABCD" \
  -maxdepth 1 -type f \
  \( \
      -iname 'C*.inx' -o \
      -iname 'C*.ini' -o \
      -iname '*C*.ase' -o \
      -iname '*C*.smd' -o \
      -iname '*C*.smb' -o \
      -iname '*C*.tga' -o \
      -iname '*C*.bmp' \
  \) \
  -printf '%s\t%f\n' 2>/dev/null |
sort -V |
tee "$OUT/pikeman_tmabcd.tsv"


echo
echo "============================================================"
echo "2. PRIMEIROS Cxxx.inx"
echo "============================================================"

find "$CLIENT/char/tmABCD" \
  -maxdepth 1 -type f \
  -iname 'C*.inx' \
  -printf '%f\n' |
sort -V |
head -40 |
tee "$OUT/pikeman_first_inx.txt"


echo
echo "============================================================"
echo "3. TUDO RELACIONADO A PIKE / PIKEMAN NO SOURCE"
echo "============================================================"

grep -RIna \
  --exclude='*.bak*' \
  --exclude='*.o' \
  -Ei \
  'pikeman|pike|JOBCODE_PIKE|JOB_CODE.*PIKE|C00[0-9]\.inx|tmABCD.*C[0-9]' \
  SrcGame/src SrcServer/src Shared Shared2 \
  2>/dev/null |
tee "$OUT/pikeman_source_refs.txt"


echo
echo "============================================================"
echo "4. INVENTARIO DE ITEMS"
echo "============================================================"

find "$CLIENT/image/Sinimage/Items" \
  -type f \
  \( \
      -iname '*.bmp' -o \
      -iname '*.tga' -o \
      -iname '*.png' -o \
      -iname '*.dds' -o \
      -iname '*.ase' -o \
      -iname '*.smd' \
  \) \
  -printf '%s\t%P\n' 2>/dev/null |
sort -k2 |
tee "$OUT/items_inventory.tsv"


echo
echo "============================================================"
echo "5. SUBPASTAS DE ITEMS"
echo "============================================================"

find "$CLIENT/image/Sinimage/Items" \
  -type d \
  -printf '%P\n' 2>/dev/null |
sort |
tee "$OUT/items_directories.txt"


echo
echo "============================================================"
echo "6. WEAPONS"
echo "============================================================"

find "$CLIENT/weapons" \
  -type f \
  -printf '%s\t%P\n' 2>/dev/null |
sort -k2 |
tee "$OUT/weapons_inventory.tsv"


echo
echo "============================================================"
echo "7. POSSIVEIS ARMAS DE PIKE"
echo "============================================================"

grep -RIna \
  --exclude='*.bak*' \
  -Ei \
  'pike|scythe|spear|javelin|weapon.*pike|pike.*weapon' \
  SrcGame/src SrcServer/src \
  2>/dev/null |
tee "$OUT/pikeman_weapon_refs.txt"


echo
echo "============================================================"
echo "8. RICARTEN / REFERENCIAS"
echo "============================================================"

grep -RIna \
  --exclude='*.bak*' \
  -Ei \
  'ricarten|ricarten|ric[._/\\-]|FIELD.*RIC' \
  SrcGame/src SrcServer/src Shared Shared2 \
  2>/dev/null |
tee "$OUT/ricarten_source_refs.txt"


echo
echo "============================================================"
echo "9. FIELD COMPLETO"
echo "============================================================"

find "$CLIENT/Field" \
  -type f \
  -printf '%s\t%P\n' 2>/dev/null |
sort -k2 |
tee "$OUT/field_inventory.tsv"


echo
echo "============================================================"
echo "10. TEXTURAS DE FIELD"
echo "============================================================"

find "$CLIENT/Field" \
  -type f \
  \( \
      -iname '*.tga' -o \
      -iname '*.bmp' -o \
      -iname '*.dds' -o \
      -iname '*.png' \
  \) \
  -printf '%s\t%P\n' 2>/dev/null |
sort -k2 |
tee "$OUT/field_textures.tsv"


echo
echo "============================================================"
echo "11. MODELOS/STAGES DE FIELD"
echo "============================================================"

find "$CLIENT/Field" \
  -type f \
  \( \
      -iname '*.smd' -o \
      -iname '*.smb' -o \
      -iname '*.ase' -o \
      -iname '*.inx' -o \
      -iname '*.ini' \
  \) \
  -printf '%s\t%P\n' 2>/dev/null |
sort -k2 |
tee "$OUT/field_models.tsv"


echo
echo "============================================================"
echo "12. TODOS ASE DO CLIENTE"
echo "============================================================"

find "$CLIENT" \
  -type f -iname '*.ase' \
  -printf '%s\t%P\n' 2>/dev/null |
sort -k2 |
tee "$OUT/all_ase.tsv"


echo
echo "============================================================"
echo "13. TODOS SMD / SMB / INX"
echo "============================================================"

find "$CLIENT" \
  -type f \
  \( \
      -iname '*.smd' -o \
      -iname '*.smb' -o \
      -iname '*.inx' \
  \) \
  -printf '%s\t%P\n' 2>/dev/null |
sort -k2 |
tee "$OUT/all_priston_models.tsv"


echo
echo "============================================================"
echo "14. FERRAMENTAS NO SERVIDOR/SOURCE"
echo "============================================================"

find "$SERVER" "$SRC" \
  -type f \
  \( \
      -iname '*.exe' -o \
      -iname '*.dll' -o \
      -iname '*.bat' -o \
      -iname '*.cmd' -o \
      -iname '*.ps1' -o \
      -iname '*.py' \
  \) \
  -print 2>/dev/null |
grep -Ei \
  'tool|editor|edit|convert|converter|model|mesh|ase|smd|smb|inx|map|field|stage|viewer|export|import' |
sort |
tee "$OUT/potential_tools.txt"


echo
echo "============================================================"
echo "15. NOMES SUSPEITOS DE FERRAMENTAS EM TODO SUMMIT"
echo "============================================================"

find "/home/gabriel/Área de Trabalho/summit" \
  -type f \
  -printf '%s\t%p\n' 2>/dev/null |
grep -Ei \
  'tool|editor|convert|converter|model|mesh|ase|smd|smb|inx|map|field|stage|viewer|export|import' |
sort -k2 |
tee "$OUT/all_potential_tools.tsv"


echo
echo "============================================================"
echo "16. SOURCE QUE IMPLEMENTA FORMATOS"
echo "============================================================"

grep -RIna \
  --exclude='*.bak*' \
  -E \
  'SMD Model data|SMD Stage data|smASE_Read|smASE_ReadBone|Save.*SMD|Load.*SMD|INX|ChangeFileExt' \
  SrcGame/src \
  2>/dev/null |
tee "$OUT/model_format_source.txt"


echo
echo "============================================================"
echo "17. RESOLUCOES DAS TEXTURAS"
echo "============================================================"

python3 - <<'PY' | tee "$OUT/texture_dimensions.tsv"
from pathlib import Path
import struct

root = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "Summit Tale Client Full/cliente"
)

def tga_size(p):
    b = p.read_bytes()[:18]
    if len(b) < 18:
        return None
    w = int.from_bytes(b[12:14], "little")
    h = int.from_bytes(b[14:16], "little")
    depth = b[16]
    return w, h, depth

def bmp_size(p):
    b = p.read_bytes()[:30]
    if len(b) < 26 or b[:2] != b"BM":
        return None
    w = int.from_bytes(b[18:22], "little", signed=True)
    h = int.from_bytes(b[22:26], "little", signed=True)
    depth = int.from_bytes(b[28:30], "little")
    return abs(w), abs(h), depth

print("width\theight\tdepth\tbytes\tpath")

for p in root.rglob("*"):
    if not p.is_file():
        continue

    ext = p.suffix.lower()

    try:
        if ext == ".tga":
            x = tga_size(p)
        elif ext == ".bmp":
            x = bmp_size(p)
        else:
            continue

        if not x:
            continue

        w,h,d = x

        print(
            f"{w}\t{h}\t{d}\t"
            f"{p.stat().st_size}\t"
            f"{p.relative_to(root)}"
        )

    except Exception:
        pass
PY


echo
echo "============================================================"
echo "AUDITORIA TERMINADA"
echo "============================================================"
echo "$OUT"

ls -lh "$OUT"
```

Isso vai nos dar uma base muito boa.

---

# 10. Um comando especialmente útil para achar a armadura inicial do Pike

Depois da auditoria:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

echo "========== C000..C010 =========="

find "$CLIENT/char/tmABCD" \
  -maxdepth 1 -type f \
  \( \
    -iname 'C00*' -o \
    -iname '*C00*' \
  \) \
  -printf '%s  %f\n' |
sort -V

echo
echo "========== REFERENCIAS DESSES NOMES NO SOURCE =========="

for n in C001 C002 C003 C004 C005 C006 C007 C008 C009 C010
do
    echo "---- $n ----"

    grep -RIna \
      --exclude='*.bak*' \
      "$n" \
      SrcGame/src SrcServer/src Shared Shared2 \
      2>/dev/null |
    head -30
done
```

Se houver `.ini` aberto além do `.inx`, melhor ainda:

```bash
find "$CLIENT/char/tmABCD" \
  -maxdepth 1 \
  \( -iname '*C*.ini' -o -iname '*C*.ase' \) \
  -print |
sort -V
```

---

# 11. Auditoria específica das ferramentas antigas

Como disseste que há ferramentas no pacote, eu rodaria esta também:

```bash
ROOT="/home/gabriel/Área de Trabalho/summit"

find "$ROOT" \
  -type f \
  \( \
    -iname '*.exe' -o \
    -iname '*.dll' \
  \) \
  -print0 |
while IFS= read -r -d '' f
do
    base="$(basename "$f")"

    if echo "$f" |
       grep -qiE \
       'tool|edit|convert|model|mesh|ase|smd|smb|inx|stage|field|map|viewer|export|import'
    then
        echo
        echo "============================================================"
        echo "$f"
        echo "============================================================"

        file "$f"

        strings -a "$f" 2>/dev/null |
        grep -Ei \
        '\.ase|\.smd|\.smb|\.inx|\.tga|\.bmp|model|mesh|bone|animation|export|import|convert' |
        head -80
    fi
done | tee \
"/home/gabriel/Área de Trabalho/summit/SourceCode/design_asset_audit/tool_strings.txt"
```

Isso é muito útil.

Mesmo que tenhamos um executável chamado:

```text
SomeOldTool.exe
```

sem documentação, se dentro dele aparecerem strings como:

```text
*.ASE
*.SMD
Export Model
Import ASE
Bone
Motion
INX
```

já sabemos que merece análise.

---

# 12. No Windows PowerShell

Caso algumas ferramentas só funcionem no Windows, podes fazer o inventário assim:

```powershell
$Root = "C:\SummitTale"

Get-ChildItem $Root -Recurse -File -ErrorAction SilentlyContinue |
Where-Object {
    $_.Extension -match '\.(exe|dll|bat|cmd|ps1)$' -and
    $_.FullName -match '(?i)tool|editor|convert|model|mesh|ase|smd|smb|inx|map|field|stage|viewer|export|import'
} |
Select-Object Length, FullName |
Sort-Object FullName |
Format-Table -AutoSize
```

E para procurar arquivos de modelo:

```powershell
Get-ChildItem "C:\SummitTale" -Recurse -File -ErrorAction SilentlyContinue |
Where-Object {
    $_.Extension -match '(?i)\.(ase|smd|smb|inx|ini|tga|bmp|dds)$'
} |
Select-Object Length, FullName |
Sort-Object FullName |
Out-File C:\SummitTale\model_asset_inventory.txt
```

---

# 13. Para o projeto de design eu escolheria os dois

Em vez de escolher **Pike ou Ricarten**, eu usaria ambos, mas com escopo pequeno:

```text
CHARACTER VERTICAL SLICE
Pikeman
+ rosto/corpo
+ primeira armadura
+ primeira arma
+ ícones correspondentes

ENVIRONMENT VERTICAL SLICE
Ricarten
+ chão
+ pedra
+ madeira
+ parede
+ telhado
+ vegetação
+ céu
```

Isso responde rapidamente duas perguntas diferentes:

**Pikeman:** até onde o renderer deixa um personagem antigo parecer moderno mantendo o mesh?

**Ricarten:** até onde conseguimos modernizar o mundo apenas com materiais/texturas antes de precisar aumentar geometria?

Depois disso teremos dados reais para decidir se vale investir em:

```text
Blender importer
SMD/SMB reverse engineering
editor de mapas
renderer moderno
```

E a descoberta do JPsTale muda bastante o risco dessa fase: já existe um projeto público que afirma interpretar justamente **INX + SMD + SMB**, então nossa primeira ação em 3D deve ser estudar esse loader e comparar as estruturas dele com `smObj3d.cpp` do Summit, não reinventar o formato do zero. ([GitHub][1])

[1]: https://github.com/smileclound/JPsTale?utm_source=chatgpt.com "GitHub - smileclound/JPsTale: Redevelop the game \"Priston Tale\" with jmonkeyengine 3.0 · GitHub"
[2]: https://priston.pro/old/comunidade/service/customizando-modelos-3d-heads/?utm_source=chatgpt.com "Customizando Modelos 3D – Tutoriais – Fórum"
[3]: https://forum.ragezone.com/threads/dev-priston-tale-rewriting-the-client-engine-from-scratch-c-17-dx11.1264977/?utm_source=chatgpt.com "[DEV] Priston Tale rewriting the client engine from scratch (C++17 / DX11) | RaGEZONE - MMO Development Forums"



Sim — e esse `MfbC13.ASE` é um achado muito bom, mas **não vou assumir que ele seja a armadura inicial do Pike só porque contém `C13`**. Precisamos ver o conteúdo do ASE e a cadeia de referências.

Há três conclusões importantes dos dumps.

Primeiro, existe uma ferramenta chamada `SMDReader.exe` dentro de `Servidor/char/tmABCD`, e ela contém literalmente o header `SMD Model data Ver 0.62` e referência a `.smd`. Isso casa exatamente com o formato proprietário do engine.  Então antes de escrever um importer Blender, devemos estudar essa ferramenta.

Segundo, o pacote tem ferramentas potencialmente ligadas a mapas/modelos: `Level Editor.exe`, `Editor Japaokiko.exe` e “Todas ferramentas em 1”. O Japaokiko contém referências a `village-1.ase.spc`, `village-2.ase.spc`, `pilai.ASE.spc`, `town1.ASE.spc`, `castle.ase.spc` e outras.  Isso é evidência forte de um pipeline/editor antigo para stages.

Terceiro, **Ricarten tem sua própria pasta**:

```text
Field/Ricarten/
```

com portas, banners, árvores, janelas, luzes, tecidos, paredes/estruturas etc.  Há inclusive famílias `riy-f...` e `riy-w...`, e a família `riy-f010` possui versões `_mm1` e `_mm2`.  Isso é muito importante para o terreno.

### Um detalhe técnico muito importante: mipmaps separados

Já vimos repetidamente estruturas como:

```text
fo010.bmp
fo010_mm1.bmp
fo010_mm2.bmp
fo010_mm3.bmp
fo010_xm3.bmp
```

com resoluções progressivamente menores. 

E em Ricarten aparece:

```text
riy-f010.tga
riy-f010_mm1.tga
riy-f010_mm2.tga
```



Então quando começarmos a melhorar chão/parede, **não basta sempre substituir somente a textura principal**. O engine aparentemente possui níveis reduzidos físicos em disco em parte dos assets.

Se modernizarmos:

```text
riy-f010.tga
```

mas deixarmos:

```text
riy-f010_mm1.tga
riy-f010_mm2.tga
```

antigos, podemos ter algo como:

```text
perto  → textura nova bonita
longe  → textura antiga borrada
```

ou transições perceptíveis.

Esse é provavelmente um dos detalhes mais importantes para o redesign dos mapas.

---

## Outra descoberta: nossa auditoria das dimensões TGA está errada

Não considere estes números:

```text
43920 x 57668
43664 x 58052
```

como dimensões reais.

O próprio tamanho dos arquivos denuncia isso. Por exemplo:

```text
262188 bytes
```

é praticamente:

```text
256 × 256 × 4 + overhead
```

e:

```text
1048620 bytes
```

é compatível com aproximadamente:

```text
512 × 512 × 4 + overhead
```

Mas nosso script simples interpretou o começo do arquivo como se fosse um TGA convencional e obteve dimensões absurdas. O inventário mostra vários desses casos em Ricarten. 

Isso pode significar:

* header modificado;
* proteção/encoding legado;
* alguma variante do formato;
* ou outro detalhe específico desses assets.

**Antes de editar esses TGAs, precisamos provar como abri-los/salvá-los sem quebrar o loader.**

---

# Agora eu investigaria primeiro o `MfbC13.ASE`

Como ASE é textual, essa é uma oportunidade ótima.

Roda:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
SERVER="/home/gabriel/Área de Trabalho/summit/Servidor"

ASE="$CLIENT/char/tmABCD/MfbC13.ASE"

echo "============================================================"
echo "1. FILE"
echo "============================================================"

file "$ASE"
ls -lh "$ASE"

echo
echo "============================================================"
echo "2. CABECALHO ASE"
echo "============================================================"

sed -n '1,160p' "$ASE"

echo
echo "============================================================"
echo "3. MATERIAIS / TEXTURAS"
echo "============================================================"

grep -aEin \
  '\*MATERIAL_NAME|\*MAP_NAME|\*BITMAP|\*MAP_CLASS|\*MATERIAL_CLASS|\*MATERIAL_DIFFUSE|\*MATERIAL_AMBIENT|\*MATERIAL_SPECULAR' \
  "$ASE" |
head -300

echo
echo "============================================================"
echo "4. OBJETOS / MESH"
echo "============================================================"

grep -aEin \
  '\*NODE_NAME|\*NODE_PARENT|\*MESH_NUMVERTEX|\*MESH_NUMFACES|\*MESH_NUMTVERTEX|\*MESH_NUMTVFACES|\*MATERIAL_REF' \
  "$ASE" |
head -400

echo
echo "============================================================"
echo "5. BONES / ANIMACAO"
echo "============================================================"

grep -aEin \
  'Bip|Bone|Physique|Skin|\*CONTROL_|\*TM_ANIMATION|\*NODE_TM|\*TM_POS|\*TM_ROTAXIS|\*TM_ROTANGLE' \
  "$ASE" |
head -400

echo
echo "============================================================"
echo "6. ARQUIVOS MfbC13 / C13"
echo "============================================================"

find "$CLIENT" "$SERVER" \
  -type f \
  \( \
      -iname '*MfbC13*' -o \
      -iname '*C13*' \
  \) \
  -printf '%s\t%p\n' 2>/dev/null |
sort -k2 |
head -500

echo
echo "============================================================"
echo "7. BITMAPS REFERENCIADOS PELO ASE"
echo "============================================================"

python3 - <<'PY'
from pathlib import Path
import re

ase = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "Summit Tale Client Full/cliente/char/tmABCD/MfbC13.ASE"
)

s = ase.read_text(errors="replace")

for x in re.findall(r'\*BITMAP\s+"([^"]+)"', s, re.I):
    print(x)
PY
```

Esse output pode nos responder diretamente:

```text
qual textura ele usa
quantos meshes possui
se possui UV
se contém bones
se contém animação
```

e, principalmente, se conseguimos abrir esse mesmo ASE no Blender.

---

# Depois vamos localizar a armadura inicial verdadeira do Pike

O dump confirmou que o código possui uma sequência:

```text
C001.ini
C002.ini
C003.ini
...
```

e também famílias `mC...` e versões especiais `C001t` etc. 

Isso reforça que a família **C é realmente uma linha importante para o Pike**, mas precisamos chegar no arquivo que a armadura low-level seleciona.

Roda:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
SERVER="/home/gabriel/Área de Trabalho/summit/Servidor"

echo "============================================================"
echo "C001..C005 EM CLIENT + SERVER"
echo "============================================================"

find "$CLIENT" "$SERVER" \
  -type f \
  \( \
      -iname 'C001.*' -o \
      -iname 'C002.*' -o \
      -iname 'C003.*' -o \
      -iname 'C004.*' -o \
      -iname 'C005.*' -o \
      -iname '*C001*' -o \
      -iname '*C002*' -o \
      -iname '*C003*' -o \
      -iname '*C004*' -o \
      -iname '*C005*' \
  \) \
  -printf '%s\t%p\n' |
sort -k2

echo
echo "============================================================"
echo "HEAD / BODY PIKE"
echo "============================================================"

find "$CLIENT" "$SERVER" \
  -type f \
  \( \
      -iname 'tmh-c0*' -o \
      -iname 'tmh_c0*' -o \
      -iname 'tmb-c0*' -o \
      -iname 'tmb_c0*' -o \
      -iname 'MfbC0*' -o \
      -iname '*pike*' \
  \) \
  -printf '%s\t%p\n' |
sort -k2 |
head -500
```

O teu dump inclusive contém referências explícitas de face de Pike como:

```text
char\tmABCD\tmh-c01.inf
char\tmABCD\tmh-c02.inf
...
```

associadas a `szModel_PikeFaceName`. Isso é uma pista muito melhor para identificar a cabeça inicial do personagem do que adivinhar pelo nome do ASE. 

---

# E investigaria o `SMDReader.exe` agora

Não execute ainda com um arquivo original. Primeiro vamos descobrir o comportamento dele.

No Linux:

```bash
SERVER="/home/gabriel/Área de Trabalho/summit/Servidor"
TOOL="$SERVER/char/tmABCD/SMDReader.exe"

echo "============================================================"
echo "PE"
echo "============================================================"

file "$TOOL"

echo
echo "============================================================"
echo "IMPORTS"
echo "============================================================"

i686-w64-mingw32-objdump -p "$TOOL" |
grep -Ei \
'ImageBase|EntryPoint|DLL Name|CreateFile|ReadFile|WriteFile|fopen|fread|fwrite'

echo
echo "============================================================"
echo "STRINGS INTERESSANTES"
echo "============================================================"

strings -a "$TOOL" |
grep -Ei \
'smd|ase|obj|mesh|vertex|face|material|texture|bitmap|bone|frame|motion|animation|usage|input|output|read|write|export|convert|error' |
head -500

echo
echo "============================================================"
echo "TODAS STRINGS CURTAS"
echo "============================================================"

strings -a -n 4 "$TOOL" |
head -500
```

Se aparecer:

```text
Usage:
SMDReader file.smd
```

ou algum nome de output, já sabemos exatamente como usá-lo.

**Se ele apenas lê SMD e imprime informações**, ainda é extremamente útil: podemos comparar os campos impressos com as structs do source e escrever um converter para Blender muito mais rápido.

---

# O `Editor Japaokiko` também subiu muito de prioridade

As strings desse executável são muito mais interessantes do que eu esperava.

Ele conhece:

```text
village-2.ase.spc
pilai.ASE.spc
ruin-2.ase.spc
village-1.ase.spc
ice_ura.ASE.spc
town1.ASE.spc
castle.ase.spc
```



Isso parece fortemente ligado à configuração de **stage/mapa**, não simplesmente edição de conta/item.

Então temos possivelmente dois caminhos separados já fornecidos pelo pacote:

```text
PERSONAGEM
SMDReader.exe
   ↓
SMD Ver 0.62


MAPA
Editor Japaokiko.exe / Level Editor.exe
   ↓
ASE.SPC / Field
```

Muito melhor do que começar reverse engineering do zero.

---

# Sobre começar por Ricarten

Agora eu acho que faz bastante sentido começar **Ricarten e Pike em paralelo**, porque Ricarten revelou uma estrutura organizada.

A pasta contém elementos obviamente bons para redesign:

```text
Field/Ricarten/banner_*.tga
Field/Ricarten/eye_door01.tga
Field/Ricarten/eye_eaves.TGA
Field/Ricarten/eye_railing*.TGA
Field/Ricarten/eye_roofline.*
Field/Ricarten/metal_s.TGA
Field/Ricarten/tent_cloth.TGA
Field/Ricarten/top_pillar.TGA
Field/Ricarten/tree*.tga
Field/Ricarten/windows01.tga
```

 

Eu classificaria para o redesign assim:

```text
FASE A
chão / rua
paredes
pedra
madeira
telhado

FASE B
porta
janela
corrimão
pilares
banners

FASE C
árvores
tecidos
luzes
props

FASE D
eventos de Natal / Halloween etc.
```

Não começaria pelos assets `christmas` ou `haunted_mansion`.

---

# Para descobrir exatamente quais `riy-f` são chão

Em vez de adivinhar que `f = floor`, vamos provar através do stage/modelo.

Roda:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
SERVER="/home/gabriel/Área de Trabalho/summit/Servidor"

echo "============================================================"
echo "STAGES QUE PODEM SER RICARTEN"
echo "============================================================"

find "$CLIENT" "$SERVER" \
  -type f \
  \( \
      -iname '*village-1*' -o \
      -iname '*village-2*' -o \
      -iname '*town1*' -o \
      -iname '*ricarten*' \
  \) \
  -printf '%s\t%p\n' |
sort -k2

echo
echo "============================================================"
echo "REFERENCIAS riy-f / riy-w"
echo "============================================================"

grep -RIna \
  --exclude='*.bak*' \
  -Ei \
  'riy-f|riy-w|Field[/\\]+Ricarten' \
  "$CLIENT" "$SERVER" \
  2>/dev/null |
head -1000
```

Se encontrarmos um `.ASE`, fazemos:

```bash
grep -aEin \
  '\*BITMAP.*riy-|\*NODE_NAME|\*MATERIAL_NAME' \
  arquivo.ASE
```

Aí teremos literalmente:

```text
objeto do mapa → material → textura
```

e saberemos se `riy-f010` é chão, parede, telhado etc.

---

## Eu mudaria a prioridade do projeto para isto

**1. Não editar nada ainda.**

**2. Abrir/analisar `MfbC13.ASE`.**

**3. Mapear `C001` → modelo → textura da primeira armadura do Pike.**

**4. Entender o `SMDReader.exe`.**

**5. Encontrar `village-1/town1` e mapear os materiais de Ricarten.**

Depois disso teremos duas cadeias completas:

```text
PIKE:
item → Cxxx → modelo → UV → textura
```

e:

```text
RICARTEN:
stage → mesh → material → textura → mipmaps
```

A partir daí já dá para começar o redesign sem trabalhar no escuro. E, pelo que apareceu agora, considero **bem menos provável que precisemos fazer reverse engineering completo do pipeline 3D desde o zero**; o pacote já contém peças importantes desse pipeline.


Esse resultado é ótimo porque agora dá para separar bem as peças do pipeline.

O `SMDReader.exe` **não parece ser um conversor SMD→ASE/OBJ**. Ele foi compilado em Debug no Visual Studio 2017, procura arquivos (`FindFirstFileA/FindNextFileA`), reconhece `.smd`, lê o header `SMD Model data Ver 0.62` e também possui `WriteFile`. Portanto pode ser um leitor, reparador ou modificador em lote de SMDs. Como possui escrita, não devemos executá-lo sobre os originais. 

Já o `MfbC13.ASE` revelou algo muito interessante.

### O que é o `MfbC13.ASE`

Ele é um ASE legítimo do 3ds Max:

```text
*3DSMAX_ASCIIEXPORT 200
*SCENE_FIRSTFRAME 0
*SCENE_LASTFRAME 32
*SCENE_FRAMESPEED 30
```



E contém uma hierarquia Biped completa:

```text
Bip01
Bip01 Pelvis
Bip01 Spine
Bip01 Spine1
Bip01 Neck
Bip01 Head
Bip01 L UpperArm
...
```



Isso é excelente porque é um **rig que podemos levar para um pipeline moderno**.

Mas há uma anomalia importante: nos 63 `MESH_NUMTVERTEX` coletados, todos estão em `0`. Os meshes vistos são minúsculos — 6, 8, 16, 37 vértices etc. — típicos da representação geométrica dos próprios bones do Biped, e não de uma armadura completa. 

Então, neste momento, eu classificaria:

```text
MfbC13.ASE
    ↓
muito provavelmente rig / skeleton de trabalho
e não a malha final completa da armadura
```

Ainda quero provar isso com uma contagem direta antes de chamar de definitivo.

Há outra pista estranha: apesar do arquivo se chamar `MfbC13.ASE`, os materiais apontam para:

```text
MfbC19_01.TGA
MfbC19_02.BMP
pr100_yun01.tga
pr100_yun02.tga
```



Isso indica que o arquivo pode ser uma **cena de trabalho reutilizada** de uma armadura diferente. Portanto não devemos assumir que `MfbC13.ASE` representa visualmente a armadura 13.

## O candidato mais interessante para o Pike inicial é `MfbC01`

A tua auditoria mostrou esta família completa:

```text
MfbC01.bmp      196662 bytes
MfbC01.smd      290229
MfbC01.smb      221216
MfbC01b.smd     108144
MfbC01h-02.TGA   32812
```

tanto no servidor quanto no cliente. 

Isso tem muito mais aparência de uma cadeia completa de personagem:

```text
MfbC01.smd     → mesh
MfbC01.smb     → skeleton/animação associada
MfbC01.bmp     → skin principal
MfbC01h-02.TGA → textura adicional/head/detail
MfbC01b.smd    → modelo complementar/variante
```

Ainda não vou afirmar o significado exato do `b` e `h-02`, mas **para o primeiro repaint, `MfbC01.bmp` é agora o candidato nº 1**.

E os arquivos:

```text
C001.inx
C002.inx
C003.inx
...
```

existem tanto no cliente quanto no servidor.  Eles provavelmente fazem parte da seleção/configuração de movimento/modelo, mas não são a textura que vamos pintar.

Também encontramos cabeças específicas:

```text
hpike_02.smd
hpike_03.smd
```



Então já estamos começando a montar a árvore:

```text
PIKEMAN
│
├── corpo/armadura
│   ├── MfbC01.smd
│   ├── MfbC01.smb
│   └── MfbC01.bmp
│
├── cabeça
│   ├── hpike_02.smd
│   └── hpike_03.smd
│
└── motions/index
    └── C001.inx
```

### Primeiro: confirma se o ASE é só rig

Roda:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
ASE="$CLIENT/char/tmABCD/MfbC13.ASE"

echo "========== GEOMOBJECTS =========="
grep -ac '^[[:space:]]*\*GEOMOBJECT' "$ASE"

echo
echo "========== MAIOR NUMERO DE VERTICES =========="

grep -aE '\*MESH_NUMVERTEX[[:space:]]+[0-9]+' "$ASE" |
awk '{print $2}' |
sort -n |
tail -20

echo
echo "========== UV VERTICES =========="

grep -aE '\*MESH_NUMTVERTEX' "$ASE" |
sort |
uniq -c

echo
echo "========== MESH TVERT =========="

grep -ac '\*MESH_TVERT[[:space:]]' "$ASE"

echo
echo "========== MESH TVFACE =========="

grep -ac '\*MESH_TFACE[[:space:]]' "$ASE"

echo
echo "========== NOMES QUE NAO SAO BIP/DUMMY =========="

grep -aE '^[[:space:]]*\*NODE_NAME' "$ASE" |
sed 's/.*"\(.*\)"/\1/' |
grep -Evi '^Bip|^Dummy' |
sort -u

echo
echo "========== SKIN / PHYSIQUE =========="

grep -aEin \
  'PHYSIQUE|SKIN|SKIN_WEIGHT|BONE_LIST|BONE_WEIGHT|VERTEX_WEIGHT' \
  "$ASE" |
head -100
```

Se sair:

```text
UV VERTICES
     63 0

MESH TVERT
0

MESH TVFACE
0
```

e não houver nenhum objeto fora de `Bip/Dummy`, então fica **provado que esse ASE é essencialmente o skeleton/rig**, não a roupa texturizada.

Isso ainda é valioso: pode ser exatamente o rig base que precisaremos para Blender.

---

## Segundo: testar o `SMDReader` com segurança

Não execute dentro de:

```text
Servidor/char/tmABCD
```

Vamos criar uma gaiola de teste contendo **uma cópia** do reader e **uma cópia** de `MfbC01.smd`.

No Linux:

```bash
SERVER="/home/gabriel/Área de Trabalho/summit/Servidor"

TEST="/home/gabriel/Área de Trabalho/summit/SMDReader_Test"

rm -rf "$TEST"
mkdir -p "$TEST"

cp -a \
  "$SERVER/char/tmABCD/SMDReader.exe" \
  "$TEST/"

cp -a \
  "$SERVER/char/tmABCD/MfbC01.smd" \
  "$TEST/"

cd "$TEST"

echo "========== ANTES =========="

sha256sum *
stat --printf='%n | %s bytes | %y\n' *

ls -la
```

Se tiver Wine configurado, pode executar:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SMDReader_Test"

wine ./SMDReader.exe
```

Ele vai mostrar:

```text
Paranaue dos smd - by Razor
Press any key to start.
```

Pressiona uma tecla e deixa terminar.

**Não coloque `.smb`, `.inx`, ASE nem nada mais nessa pasta ainda.**

Depois:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SMDReader_Test"

echo "========== DEPOIS =========="

ls -lah

echo
echo "========== HASHES =========="

sha256sum *

echo
echo "========== TIPOS =========="

file *

echo
echo "========== DATAS =========="

stat --printf='%n | %s bytes | %y\n' *
```

O que quero descobrir é se ele:

1. apenas imprime informações;
2. altera o `.smd`;
3. cria outro arquivo;
4. cria dump textual;
5. modifica algum header interno.

Como ele importa `WriteFile`, **qualquer alteração no hash do SMD é relevante**.

Se:

```text
SHA antes == SHA depois
nenhum arquivo novo
```

então provavelmente é apenas um validator/reader que lê os SMDs do diretório.

Se aparecer arquivo novo, aí pode ser ouro.

---

## Terceiro: investigar o `MfbC01.smd` sem executar nada

Como o SMD contém strings de texturas embutidas em vários lugares do engine, tente:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

SMD="$CLIENT/char/tmABCD/MfbC01.smd"

echo "========== HEADER =========="

strings -a -n 4 "$SMD" |
head -50

echo
echo "========== TEXTURAS REFERENCIADAS =========="

strings -a "$SMD" |
grep -Ei '\.(bmp|tga|dds|png)$|\.bmp|\.tga|\.dds|\.png' |
sort -fu

echo
echo "========== NOMES / BONES =========="

strings -a "$SMD" |
grep -Ei 'Bip|Bone|Pelvis|Spine|Head|Arm|Leg|Foot|Hand' |
sort -u |
head -200
```

E também:

```bash
xxd -g1 -l 256 \
"/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente/char/tmABCD/MfbC01.smd"
```

Isso pode nos dizer diretamente se `MfbC01.bmp` está gravado dentro do SMD como material.

---

## Quarto: a textura `MfbC01.bmp` é ótima para o primeiro experimento

O tamanho:

```text
196662 bytes
```

é praticamente exatamente um BMP:

```text
256 × 256 × 3 + header
```

Então esse arquivo é muito mais simples e seguro que aqueles TGAs estranhos de Ricarten.

Verifica:

```bash
file \
"/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente/char/tmABCD/MfbC01.bmp"

identify \
"/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente/char/tmABCD/MfbC01.bmp" \
2>/dev/null || true
```

Eu apostaria que teremos algo próximo de:

```text
BMP
256x256
24-bit
```

Se sim, **esse é o asset perfeito para começar o redesign do Pike**.

Você pode fazer uma prova visual extremamente simples antes mesmo de desenhar:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

cp -a \
 "$CLIENT/char/tmABCD/MfbC01.bmp" \
 "$CLIENT/char/tmABCD/MfbC01.bmp.before_design_test"
```

Depois alteramos uma região muito óbvia da cópia — por exemplo uma faixa colorida — e entramos no jogo equipado com aquela armadura.

Se a faixa aparecer no corpo:

**pipeline de skin confirmado.**

A partir daí você pode abrir no Krita, GIMP ou Photoshop e começar o repaint real.

---

### E tem uma boa notícia para Blender

O que encontramos agora torna o reverse engineering bem menos assustador.

Temos:

```text
ASE legítimo do 3ds Max
↓
Biped completo

+

SMD proprietary
↓
loader no source

+

SMDReader.exe
↓
ferramenta feita especificamente para SMD 0.62
```

Isso significa que, mesmo se não existir um conversor pronto, não estaremos tentando descobrir tudo às cegas.

O objetivo futuro pode ser perfeitamente:

```text
Priston SMD
   ↓
nosso parser usando as structs do source
   ↓
Blender Python
   ↓
mesh + UV + materials + armature
```

e depois:

```text
Blender
   ↓
ASE
   ↓
loader/conversor legado
   ↓
Priston
```

O próximo resultado que eu mais quero é o **teste isolado do `SMDReader`** junto com as strings de `MfbC01.smd`. Isso deve dizer se já temos um atalho para abrir a malha ou se começamos a construir o nosso próprio importer.



Agora temos uma visão muito mais sólida do pipeline.

O `MfbC13.ASE` **não é só o rig**. Ele contém 67 objetos, quatro meshes que não são `Bip/Dummy` (`MahC20`, `MbhC19`, `MlhC20`, `MshC19`), centenas de UVs e blocos `PHYSIQUE` ligando os vértices aos bones. Ou seja: é uma cena 3D editável de verdade, com geometria + UV + skeleton + skinning. Os materiais também apontam para texturas externas. 

Isso é uma notícia muito boa para Blender.

### O que o ASE nos mostrou

Você encontrou:

```text
MESH_NUMTVERTEX:
63 objetos → 0
outros → 124, 212, 250, 296

MESH_TVERT  = 882
MESH_TFACE  = 1086
```

Portanto os objetos Biped não têm UV, como esperado, mas os quatro meshes reais têm.

E:

```text
*PHYSIQUE_VERTEXASSIGNMENT...
```

mostra exatamente **qual bone controla cada vertex**.

Isso significa que, em princípio, podemos escrever um importer Blender que leia:

```text
*GEOMOBJECT
*MESH_VERTEX
*MESH_FACE
*MESH_TVERT
*MESH_TFACE
*NODE_NAME
*NODE_PARENT
*PHYSIQUE_VERTEXASSIGNMENT
```

e gere:

```text
Mesh
UV Map
Armature
Vertex Groups
Materials
```

Não precisamos fazer reverse engineering binário do SMD para começar a produzir conteúdo novo, desde que consigamos preservar/produzir ASE nesse formato.

Há uma ressalva: esse `MfbC13.ASE` parece ser uma **cena de trabalho reaproveitada**, porque o arquivo se chama C13 mas os meshes/texturas falam de C19/C20 e `pr100_yun`.  Portanto ele é ótimo como documentação do pipeline, mas não necessariamente representa a armadura C13 exatamente.

---

## E o `MfbC01.smd` confirmou a textura do Pike

Esse é talvez o resultado mais útil para começarmos o redesign agora.

O binário contém explicitamente:

```text
SMD Model data Ver 0.62

MsmC01
MshC01
MslC01
...
MbhC01
...

char\tmABCD\MfbC01.bmp
char\tmABCD\MfbC03.bmp
char\tmABCD\mo-p-ac.BMP
```

E a família física existe no cliente: `MfbC01.smd`, `.smb`, `.bmp`, um `MfbC01b.smd` e `MfbC01h-02.TGA`. 

Então agora está **provado que `MfbC01.bmp` é uma textura realmente referenciada pelo modelo SMD**.

Ainda não está provado que ela representa exclusivamente a “armadura nível 1”, porque o mesmo SMD referencia também `MfbC03.bmp` e `mo-p-ac.BMP`. Isso sugere que o SMD pode ser um container com múltiplos objetos/materiais/partes.

Mas já podemos fazer o primeiro teste visual.

---

## Tem mais uma descoberta importante: esses `.bmp` também são customizados/protegidos

O arquivo:

```text
MfbC01.bmp
196662 bytes
```

tem exatamente:

```text
256 × 256 × 3 + 54 = 196662
```

Ou seja, o tamanho é **exatamente o de um BMP 256×256 RGB24 sem compressão**.

Mas:

```bash
file MfbC01.bmp
```

retorna:

```text
data
```

e o ImageMagick não reconhece.

Então provavelmente o conteúdo pixel é BMP normal, mas o header foi alterado/obfuscado pelo pipeline do Priston.

Isso também explica por que nossa auditoria dos TGA devolveu dimensões absurdas.

Antes de começar a pintar, precisamos resolver esse codec. E isso é melhor fazer agora.

Rode este bloco:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
BMP="$CLIENT/char/tmABCD/MfbC01.bmp"

echo "============================================================"
echo "1. PRIMEIROS 128 BYTES"
echo "============================================================"

xxd -g1 -l 128 "$BMP"

echo
echo "============================================================"
echo "2. ULTIMOS 64 BYTES"
echo "============================================================"

tail -c 64 "$BMP" | xxd -g1

echo
echo "============================================================"
echo "3. COMPARA COM BMP NORMAL DO MESMO TAMANHO"
echo "============================================================"

python3 - <<'PY'
from pathlib import Path
p = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "Summit Tale Client Full/cliente/char/tmABCD/MfbC01.bmp"
)

b = p.read_bytes()

print("size =", len(b))
print("first 64 =", b[:64].hex(" "))

print()
print("Expected normal BMP:")
print("signature      =", b[:2])
print("filesize field =", int.from_bytes(b[2:6], "little"))
print("pixel offset   =", int.from_bytes(b[10:14], "little"))
print("DIB size       =", int.from_bytes(b[14:18], "little"))
print("width          =", int.from_bytes(b[18:22], "little", signed=True))
print("height         =", int.from_bytes(b[22:26], "little", signed=True))
print("planes         =", int.from_bytes(b[26:28], "little"))
print("bpp            =", int.from_bytes(b[28:30], "little"))
PY

echo
echo "============================================================"
echo "4. PROCURA O LOADER / DECODER DE BMP/TGA"
echo "============================================================"

grep -RIn -B12 -A35 \
  --exclude='*.bak*' \
  -Ei \
  'Load.*BMP|Read.*BMP|BMP.*Load|Load.*TGA|Read.*TGA|TGA.*Load|BITMAPFILEHEADER|BITMAPINFOHEADER|D3DXCreateTextureFromFile|CreateTextureFromFile|Decode.*BMP|Decode.*TGA' \
  SrcGame/src \
  --include='*.cpp' --include='*.h' |
head -1800
```

Esse é agora o passo principal.

Se encontrarmos algo no loader como:

```cpp
header[0] ^= ...
```

ou:

```cpp
memcpy(...)
Decode(...)
```

vamos conseguir escrever facilmente:

```text
pt_texture_decode.py
pt_texture_encode.py
```

e então teu workflow vira:

```text
MfbC01.bmp protegido
        ↓
decode
        ↓
MfbC01_editable.bmp/png
        ↓
Krita / Photoshop
        ↓
encode
        ↓
MfbC01.bmp
        ↓
jogo
```

Isso é muito melhor do que editar bytes ou depender de uma ferramenta obscura.

---

## O SMDReader ainda não foi realmente testado

O Wine nem chegou ao `main()`.

Ele falhou porque o programa foi compilado contra os runtimes **Debug** do Visual Studio:

```text
MSVCP140D.dll
VCRUNTIME140D.dll
ucrtbased.dll
```

O sufixo `D` é justamente Debug.

Portanto:

```text
SHA antes  = SHA depois
```

não nos diz nada sobre o comportamento do `SMDReader`, porque ele não executou.

Mas também é bom: nada foi modificado.

Eu **não baixaria DLLs aleatórias da internet**.

Primeiro procura se os próprios arquivos vieram junto no pacote:

```bash
ROOT="/home/gabriel/Área de Trabalho/summit"

find "$ROOT" \
  -type f \
  \( \
    -iname 'MSVCP140D.dll' -o \
    -iname 'VCRUNTIME140D.dll' -o \
    -iname 'ucrtbased.dll' \
  \) \
  -printf '%s  %p\n'
```

E também no Wine:

```bash
find ~/.wine \
  -type f \
  \( \
    -iname 'MSVCP140D.dll' -o \
    -iname 'VCRUNTIME140D.dll' -o \
    -iname 'ucrtbased.dll' \
  \) \
  -print 2>/dev/null
```

Se não houver, eu colocaria o `SMDReader` em segundo plano por enquanto. Temos o **source do loader SMD**, que é muito mais valioso do que gastar tempo tentando ressuscitar um utilitário Debug.

---

## E o ASE muda a estratégia do Blender

Agora eu faria o pipeline em duas etapas.

Para **conteúdo existente em SMD**:

```text
SMD
 ↓
parser nosso
 ↓
Blender
```

Para **conteúdo novo** inicialmente:

```text
Blender
 ↓
ASE compatível com Priston
 ↓
smASE_Read / conversor do próprio engine
 ↓
SMD
```

A parte mais difícil normalmente seria descobrir como o skinning funciona. Mas o ASE que você encontrou praticamente documenta isso:

```text
vertex 0 → Bip01 Spine1
vertex 1 → Bip01 Spine1
...
vertex 11 → Bip01 R UpperArm
```

Ou seja, pelo menos nesse arquivo o Physique está usando muitas atribuições **rigid/non-blended**, o que é muito mais simples de reproduzir que um skin moderno com quatro pesos por vértice.

---

## Para começar o Pike, não precisamos esperar o Blender

Eu faria primeiro o repaint de:

```text
char/tmABCD/MfbC01.bmp
```

Você já fez corretamente o backup:

```text
MfbC01.bmp.before_design_test
```

Agora falta apenas descobrir a codificação desse BMP.

Depois que tivermos o decoder, fazemos um teste propositalmente feio e inequívoco — por exemplo colocar uma região da textura em magenta — e testamos no personagem.

Se aparecer no Pike, fechamos:

```text
MfbC01.smd
      ↓
MfbC01.bmp
      ↓
corpo visível no jogo
```

Aí você pode começar o redesign real em 512×512 **depois** de provarmos que o loader aceita aumento de resolução.

Eu testaria nessa ordem:

```text
1. decode 256×256
2. modificar mantendo 256×256
3. jogo
4. restaurar
5. produzir 512×512
6. re-encode
7. jogo
```

Um fator por vez.

### Depois disso: Ricarten

Também apareceu nos dumps que existem **ASEs dentro de `Field/Ricarten`**, inclusive `world_ani06.ASE`, e eles referenciam diretamente `riy-w001.bmp`, `riy-f028.bmp`, `riy-w003.tga`, `riy-w004.bmp` etc. 

Isso é excelente: para Ricarten podemos obter diretamente:

```text
mesh do mapa
 → material
 → textura
 → UV
```

sem reverse engineering do stage inicialmente.

Então a prioridade técnica que eu seguiria agora é:

1. **decodificar/entender o BMP/TGA protegido**;
2. provar `MfbC01.bmp` no Pike;
3. mapear os ASEs de Ricarten;
4. só depois começar o importer Blender para SMD/ASE completo.

Estamos muito mais perto de um pipeline de design utilizável do que parecia inicialmente.
Esse output foi decisivo. Temos três conclusões técnicas bem fortes.

O `MfbC13.ASE` **é uma cena 3D completa**, não apenas um skeleton: há meshes reais com UV (`124`, `212`, `250`, `296` texture vertices), `882` `MESH_TVERT`, `1086` `MESH_TFACE` e `PHYSIQUE` associando vértices aos bones. Portanto esse tipo de ASE já contém exatamente a informação necessária para um pipeline Blender → Priston.

O `MfbC01.smd` também confirmou diretamente referências a `char\tmABCD\MfbC01.bmp`, `MfbC03.bmp` e `mo-p-ac.BMP`. A família física de `MfbC01` está presente no cliente. 

E a descoberta mais prática para começarmos o redesign: **`MfbC01.bmp` é praticamente um BMP normal de 256×256 RGB24, mas com os primeiros 14 bytes proprietários.** O `BITMAPINFOHEADER` começa corretamente no byte `0x0E`: tamanho 40, largura 256, altura 256, 1 plane e 24 bpp. 

Isso significa que podemos começar o repaint agora.

## Primeiro: decodifica para um BMP normal

Roda:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
WORK="/home/gabriel/Área de Trabalho/summit/design_work/pike_c01"

mkdir -p "$WORK"

python3 - <<'PY'
from pathlib import Path
import struct

src = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "Summit Tale Client Full/cliente/char/tmABCD/MfbC01.bmp"
)

out = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "design_work/pike_c01/MfbC01.decoded.bmp"
)

b = bytearray(src.read_bytes())

dib_size = int.from_bytes(b[14:18], "little")
width    = int.from_bytes(b[18:22], "little", signed=True)
height   = int.from_bytes(b[22:26], "little", signed=True)
planes   = int.from_bytes(b[26:28], "little")
bpp      = int.from_bytes(b[28:30], "little")

print("ORIGINAL")
print("size =", len(b))
print("custom header =", b[:14].hex(" "))
print("DIB =", dib_size)
print("width =", width)
print("height =", height)
print("planes =", planes)
print("bpp =", bpp)

assert dib_size == 40
assert width == 256
assert abs(height) == 256
assert planes == 1
assert bpp == 24
assert len(b) == 196662

# BMP 256x256 RGB24:
# BITMAPFILEHEADER = 14 bytes
# BITMAPINFOHEADER = 40 bytes
# pixels begin at byte 54
normal_header = struct.pack(
    "<2sIHHI",
    b"BM",
    len(b),
    0,
    0,
    54
)

b[:14] = normal_header

out.write_bytes(b)

print()
print("CRIADO:", out)
print("header =", b[:14].hex(" "))
PY

echo
echo "========== VERIFICA =========="

file "$WORK/MfbC01.decoded.bmp"

identify "$WORK/MfbC01.decoded.bmp" 2>/dev/null || true
```

O esperado agora é algo como:

```text
PC bitmap
256 x 256
24-bit
```

Se isso acontecer, **o formato de proteção dessa textura está essencialmente quebrado para fins de edição**: basta retirar/recolocar aqueles 14 bytes.

---

## Podemos fazer o primeiro teste sem nem abrir o Photoshop/Krita

Eu prefiro fazer primeiro uma alteração impossível de não perceber. Vamos pintar **metade da textura de magenta**, apenas numa cópia de teste.

Isso também evita a dúvida de “eu editei uma região que o UV nem usa”.

Roda:

```bash
python3 - <<'PY'
from pathlib import Path
import struct

work = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "design_work/pike_c01"
)

src = work / "MfbC01.decoded.bmp"
out = work / "MfbC01.magenta.bmp"

b = bytearray(src.read_bytes())

assert b[:2] == b"BM"

offset = int.from_bytes(b[10:14], "little")
width  = int.from_bytes(b[18:22], "little", signed=True)
height = int.from_bytes(b[22:26], "little", signed=True)
bpp    = int.from_bytes(b[28:30], "little")

assert width == 256
assert abs(height) == 256
assert bpp == 24

h = abs(height)

# BMP rows are padded to 4-byte boundaries.
stride = ((width * 3 + 3) // 4) * 4

# pinta metade esquerda em MAGENTA.
# BMP usa B,G,R.
for y in range(h):
    row = offset + y * stride

    for x in range(width // 2):
        p = row + x * 3
        b[p + 0] = 255   # B
        b[p + 1] = 0     # G
        b[p + 2] = 255   # R

out.write_bytes(b)

print("CRIADO:", out)
PY

file "$WORK/MfbC01.magenta.bmp"
identify "$WORK/MfbC01.magenta.bmp" 2>/dev/null || true
```

Agora podes abrir:

```bash
xdg-open "$WORK/MfbC01.magenta.bmp"
```

ou, se tens Krita:

```bash
krita "$WORK/MfbC01.magenta.bmp"
```

Deves ver a textura UV e metade dela completamente magenta.

---

## Agora voltamos para o formato que o Priston espera

Isso é o mais interessante.

Como mantivemos:

```text
256×256
24 bpp
196662 bytes
```

podemos simplesmente pegar os **14 bytes proprietários do original** e colocá-los sobre o BMP modificado.

Roda:

```bash
python3 - <<'PY'
from pathlib import Path

client = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "Summit Tale Client Full/cliente/char/tmABCD"
)

work = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "design_work/pike_c01"
)

original = client / "MfbC01.bmp"
edited   = work / "MfbC01.magenta.bmp"
output   = work / "MfbC01.game_test.bmp"

orig = original.read_bytes()
edit = bytearray(edited.read_bytes())

assert len(orig) == 196662
assert len(edit) == len(orig)

assert edit[:2] == b"BM"
assert int.from_bytes(edit[14:18], "little") == 40
assert int.from_bytes(edit[18:22], "little", signed=True) == 256
assert abs(int.from_bytes(edit[22:26], "little", signed=True)) == 256
assert int.from_bytes(edit[28:30], "little") == 24

print("normal header:")
print(edit[:14].hex(" "))

print("Priston header:")
print(orig[:14].hex(" "))

# devolve exatamente o cabeçalho proprietário original
edit[:14] = orig[:14]

output.write_bytes(edit)

print()
print("OUTPUT =", output)
print("SIZE =", len(edit))
print("HEADER =", edit[:14].hex(" "))
PY

echo
echo "========== HASHES =========="

sha256sum \
 "$CLIENT/char/tmABCD/MfbC01.bmp" \
 "$WORK/MfbC01.game_test.bmp"

echo
echo "========== HEADERS =========="

xxd -g1 -l 32 \
 "$CLIENT/char/tmABCD/MfbC01.bmp"

xxd -g1 -l 32 \
 "$WORK/MfbC01.game_test.bmp"
```

O começo dos dois arquivos deverá ser igual:

```text
41 38 3a 09 13 19 24 31 40 51 9a 79 90 a9
```

mas os SHA256 deverão ser diferentes porque modificamos os pixels.

---

## Instala somente esse teste

Tu já tens:

```text
MfbC01.bmp.before_design_test
```

então não precisamos fazer outro backup.

Instala:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
WORK="/home/gabriel/Área de Trabalho/summit/design_work/pike_c01"

cp -a \
 "$WORK/MfbC01.game_test.bmp" \
 "$CLIENT/char/tmABCD/MfbC01.bmp"

sha256sum \
 "$CLIENT/char/tmABCD/MfbC01.bmp" \
 "$CLIENT/char/tmABCD/MfbC01.bmp.before_design_test"
```

**Não compila.**

Abre o cliente e olha o Pikeman.

Se qualquer parte da armadura/corpo aparecer magenta, temos confirmado em runtime:

```text
MfbC01.smd
      ↓
material
      ↓
MfbC01.bmp
      ↓
UV
      ↓
personagem
```

Aí já temos oficialmente o primeiro asset do projeto de redesign.

Se nada mudar, não significa que o decoder falhou: significará apenas que o teu Pike atual está usando outra variante/modelo, e aí rastreamos qual `Cxxx` está equipado.

Para restaurar:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

cp -a \
 "$CLIENT/char/tmABCD/MfbC01.bmp.before_design_test" \
 "$CLIENT/char/tmABCD/MfbC01.bmp"
```

---

### Isso também resolve como trabalhar no Krita/Photoshop

Depois que o teste magenta funcionar, o pipeline de produção será:

```text
MfbC01.bmp do Priston
        ↓
troca header proprietário → BM
        ↓
BMP normal 256×256
        ↓
Krita / Photoshop / Substance
        ↓
BMP normal 256×256 RGB24
        ↓
recoloca header proprietário
        ↓
MfbC01.bmp
        ↓
cliente
```

Para a primeira versão, eu **não aumentaria para 512×512 ainda**.

Os 14 bytes proprietários provavelmente codificam ou verificam alguma informação do arquivo. Enquanto permanecemos em `256×256 / 196662 bytes`, reutilizar o header original é uma estratégia muito segura. Quando formos para 512×512, primeiro comparamos os headers proprietários de texturas de resoluções diferentes para descobrir como esses 14 bytes são produzidos.

E o `SMDReader` pode ficar de lado por agora: ele não chegou a executar por faltar `MSVCP140D.dll`, `VCRUNTIME140D.dll` e `ucrtbased.dll`, e a busca que fizeste não encontrou essas DLLs nem no pacote nem no Wine. 

Neste momento, **o teste magenta em `MfbC01.bmp` é o próximo experimento que mais informação nos dá com o menor risco**.
Perfeito. Esse teste provou duas coisas importantes — e também corrigiu nossa identificação do personagem.

A textura `MfbC01.bmp` **é da Sacerdotisa**, não do Pikeman. A nomenclatura agora fica bem mais clara:

```text
MfbC01
││││
│││└─ C = classe C de Morion → Priestess
││└── b = body
│└─── f = female
└──── M = Morion
```

Isso explica exatamente por que o vestido inteiro ficou magenta.

Para o Pikeman, o candidato correto passa a ser a família:

```text
TmbC...
││││
│││└─ C = Pikeman
││└── b = body
│└─── m = male
└──── T = Tempskron
```

E isso combina com aquele `TmbC14` que já apareceu antes no material antigo do PT.

Mais importante: o pipeline de textura ficou **PROVADO FUNCIONAL para esse tipo de BMP**:

```text
BMP protegido do Priston
        ↓
substitui apenas bytes 0..13
        ↓
BMP normal 256×256 RGB24
        ↓
edição normal
        ↓
recoloca os 14 bytes originais
        ↓
cliente aceita
        ↓
textura modificada aparece no modelo
```

E o restore também funcionou. O que ainda **não** está provado é gerar um header proprietário novo para outra resolução; por enquanto sabemos que reutilizar o header original funciona mantendo tamanho/formato/resolução.

Agora eu iria diretamente ao Pike.Ótimo. A auditoria confirmou que a família **`TmbC` é muito provavelmente o corpo/armaduras do Pikeman**, e temos exatamente o conjunto inicial que queríamos:

```text
TmbC01.smd    187504
TmbC01.smb    178524
TmbC01.bmp    196662
TmbC01b.smd   105896
```

Além disso aparecem progressões `C03`, `C06`, `C08`, `C10`, `C11`, `C13`, `C14` etc., o que tem bastante cara de sequência de equipamentos/modelos. 

O bloco 4 **não testou os SMDs** porque meu comando anterior tinha um bug: `for f in $(find ...)` quebra caminhos que possuem espaços (`Área de Trabalho`, `Summit Tale Client Full`). Foi por isso que apareceu `strings: "/home/gabriel/Área": ficheiro inexistente`. 

Use esta versão correta:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

echo "============================================================"
echo "TEXTURAS REFERENCIADAS PELOS TmbC INICIAIS"
echo "============================================================"

find "$CLIENT/char/tmABCD" \
  -maxdepth 1 \
  -type f \
  -iname 'TmbC*.smd' \
  -print0 |
sort -zV |
head -z -n 12 |
while IFS= read -r -d '' f
do
    echo
    echo "---------- $(basename "$f") ----------"

    strings -a "$f" |
    grep -Ei \
      'char[\\/]+tmABCD[\\/]+.*\.(bmp|tga|dds|png)' |
    sort -fu
done
```

Mas eu faria também uma inspeção **só do `TmbC01.smd`**, porque esse é o nosso candidato nº 1:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

SMD="$CLIENT/char/tmABCD/TmbC01.smd"

echo "========== HEADER / OBJETOS =========="

strings -a -n 4 "$SMD" |
head -80

echo
echo "========== TEXTURAS =========="

strings -a "$SMD" |
grep -Ei '\.(bmp|tga|dds|png)' |
sort -fu

echo
echo "========== BONES =========="

strings -a "$SMD" |
grep -Ei \
'Bip|Pelvis|Spine|Head|UpperArm|Forearm|Hand|Thigh|Calf|Foot' |
sort -u |
head -150
```

### Outra coisa que o inventário já revelou

Para o Pikeman inicial há:

```text
TmbC01.bmp   196662 bytes
```



Isso é **exatamente o mesmo tamanho** do `MfbC01.bmp` da Priestess que conseguimos decodificar:

```text
256 × 256 × 3 + 54 = 196662
```

Então é muito provável que `TmbC01.bmp` use o mesmo esquema de proteção de 14 bytes.

Podemos verificar sem alterar nada:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

for f in \
  "$CLIENT/char/tmABCD/MfbC01.bmp" \
  "$CLIENT/char/tmABCD/TmbC01.bmp"
do
    echo
    echo "========== $(basename "$f") =========="
    ls -l "$f"
    xxd -g1 -l 54 "$f"
done
```

Se `TmbC01.bmp` mostrar:

```text
offset 0x0e = 28 00 00 00
width       = 00 01 00 00
height      = 00 01 00 00
planes      = 01 00
bpp         = 18 00
```

temos novamente:

```text
DIB = 40
256×256
24-bit
```

e podemos usar **exatamente o mesmo decoder**.

## A cabeça do Pike também ficou bem mapeada

A família é claramente:

```text
Tmh-C01
Tmh-C02
Tmh-C03
...
```

e há variantes:

```text
Tmh-C01a
Tmh-C01b
Tmh-C01c
Tmh-C01d
```

com assets como:

```text
Tmh-C01c_face.BMP
Tmh-C01c_cap.bmp
Tmh-C01c_tie.bmp
```



Então no redesign teremos provavelmente uma separação bastante conveniente:

```text
Pikeman
│
├── BODY / ARMOR
│   └── TmbCxx...
│
└── HEAD / FACE / HAIR
    └── Tmh-Cxx...
```

Isso é ótimo para o projeto.

### Próximo objetivo

Primeiro quero ver o output de:

```text
strings TmbC01.smd
```

Se ele disser explicitamente:

```text
char\tmABCD\TmbC01.bmp
```

aí fazemos no `TmbC01.bmp` exatamente o mesmo teste que fizemos na Priestess: decode → metade magenta → recoloca header → instala → abre o Pikeman.

Se o Pike ficar magenta, aí teremos **PROVADO** o primeiro pipeline gráfico dele e já podemos partir para um repaint real da armadura inicial.


Roda só esta auditoria primeiro, sem alterar nada:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

echo "============================================================"
echo "1. FAMILIA TmbC - PIKEMAN BODY"
echo "============================================================"

find "$CLIENT/char/tmABCD" \
  -maxdepth 1 -type f \
  \( \
    -iname 'TmbC*' -o \
    -iname 'tmbC*' -o \
    -iname 'tmb-c*' -o \
    -iname 'tmb_c*' \
  \) \
  -printf '%s\t%f\n' |
sort -V

echo
echo "============================================================"
echo "2. FAMILIA Tmh-C - PIKEMAN HEAD"
echo "============================================================"

find "$CLIENT/char/tmABCD" \
  -maxdepth 1 -type f \
  \( \
    -iname 'Tmh-C*' -o \
    -iname 'tmh_c*' -o \
    -iname 'tmh-c*' \
  \) \
  -printf '%s\t%f\n' |
sort -V

echo
echo "============================================================"
echo "3. PRIMEIROS BODY SMD"
echo "============================================================"

find "$CLIENT/char/tmABCD" \
  -maxdepth 1 -type f \
  -iname 'TmbC*.smd' \
  -printf '%p\n' |
sort -V |
head -20

echo
echo "============================================================"
echo "4. TEXTURAS REFERENCIADAS PELOS PRIMEIROS TmbC"
echo "============================================================"

for f in $(find "$CLIENT/char/tmABCD" \
    -maxdepth 1 -type f \
    -iname 'TmbC*.smd' \
    -printf '%p\n' |
    sort -V |
    head -10)
do
    echo
    echo "---------- $(basename "$f") ----------"

    strings -a "$f" |
    grep -Ei \
      'char[\\/]+tmABCD[\\/]+.*\.(bmp|tga|dds|png)' |
    sort -fu
done
```

O bloco 4 é o mais importante.

Eu espero algo parecido com:

```text
---------- TmbC01.smd ----------
char\tmABCD\TmbC01.bmp
...
```

ou talvez o modelo tenha nomes diferentes internamente.

Quando tivermos isso, fazemos **exatamente o mesmo teste magenta**, mas na textura realmente referenciada pelo `TmbC01.smd`.

### Agora também entendemos melhor a estrutura das classes

Provavelmente temos algo próximo disto:

```text
Tempskron
TmbA... → Mechanician
TmbB... → Fighter
TmbC... → Pikeman
T?bD... → Archer

Morion
M?bA... → Knight
M?bB... → Atalanta
MfbC... → Priestess
M?bD... → Magician
```

Os detalhes de sexo/prefixo de cada classe podemos confirmar pelos arquivos, mas o caso `MfbC01 → Sacerdotisa` agora foi confirmado visualmente pelo próprio jogo.

E isso revela outra coisa útil: `MfbC01.bmp` parece conter boa parte — talvez praticamente toda — da roupa/base da Sacerdotisa, porque metade da texture map transformou o vestido de forma tão evidente. Isso é exatamente o tipo de UV atlas que você queria redesenhar.

Depois de acharmos o equivalente `TmbC01.bmp`, já podemos criar o primeiro **template de redesign do Pikeman**:

```text
original protegido
→ decoded BMP
→ PSD/KRA de trabalho
→ guia UV
→ repaint
→ export RGB24
→ re-encode header
→ teste no jogo
```

E eu manteria a primeira armadura em **256×256** até o visual estar correto. Só depois testamos 512×512 isoladamente.
Ótimo. A auditoria confirmou que a família **`TmbC` é muito provavelmente o corpo/armaduras do Pikeman**, e temos exatamente o conjunto inicial que queríamos:

```text
TmbC01.smd    187504
TmbC01.smb    178524
TmbC01.bmp    196662
TmbC01b.smd   105896
```

Além disso aparecem progressões `C03`, `C06`, `C08`, `C10`, `C11`, `C13`, `C14` etc., o que tem bastante cara de sequência de equipamentos/modelos. 

O bloco 4 **não testou os SMDs** porque meu comando anterior tinha um bug: `for f in $(find ...)` quebra caminhos que possuem espaços (`Área de Trabalho`, `Summit Tale Client Full`). Foi por isso que apareceu `strings: "/home/gabriel/Área": ficheiro inexistente`. 

Use esta versão correta:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

echo "============================================================"
echo "TEXTURAS REFERENCIADAS PELOS TmbC INICIAIS"
echo "============================================================"

find "$CLIENT/char/tmABCD" \
  -maxdepth 1 \
  -type f \
  -iname 'TmbC*.smd' \
  -print0 |
sort -zV |
head -z -n 12 |
while IFS= read -r -d '' f
do
    echo
    echo "---------- $(basename "$f") ----------"

    strings -a "$f" |
    grep -Ei \
      'char[\\/]+tmABCD[\\/]+.*\.(bmp|tga|dds|png)' |
    sort -fu
done
```

Mas eu faria também uma inspeção **só do `TmbC01.smd`**, porque esse é o nosso candidato nº 1:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

SMD="$CLIENT/char/tmABCD/TmbC01.smd"

echo "========== HEADER / OBJETOS =========="

strings -a -n 4 "$SMD" |
head -80

echo
echo "========== TEXTURAS =========="

strings -a "$SMD" |
grep -Ei '\.(bmp|tga|dds|png)' |
sort -fu

echo
echo "========== BONES =========="

strings -a "$SMD" |
grep -Ei \
'Bip|Pelvis|Spine|Head|UpperArm|Forearm|Hand|Thigh|Calf|Foot' |
sort -u |
head -150
```

### Outra coisa que o inventário já revelou

Para o Pikeman inicial há:

```text
TmbC01.bmp   196662 bytes
```



Isso é **exatamente o mesmo tamanho** do `MfbC01.bmp` da Priestess que conseguimos decodificar:

```text
256 × 256 × 3 + 54 = 196662
```

Então é muito provável que `TmbC01.bmp` use o mesmo esquema de proteção de 14 bytes.

Podemos verificar sem alterar nada:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

for f in \
  "$CLIENT/char/tmABCD/MfbC01.bmp" \
  "$CLIENT/char/tmABCD/TmbC01.bmp"
do
    echo
    echo "========== $(basename "$f") =========="
    ls -l "$f"
    xxd -g1 -l 54 "$f"
done
```

Se `TmbC01.bmp` mostrar:

```text
offset 0x0e = 28 00 00 00
width       = 00 01 00 00
height      = 00 01 00 00
planes      = 01 00
bpp         = 18 00
```

temos novamente:

```text
DIB = 40
256×256
24-bit
```

e podemos usar **exatamente o mesmo decoder**.

## A cabeça do Pike também ficou bem mapeada

A família é claramente:

```text
Tmh-C01
Tmh-C02
Tmh-C03
...
```

e há variantes:

```text
Tmh-C01a
Tmh-C01b
Tmh-C01c
Tmh-C01d
```

com assets como:

```text
Tmh-C01c_face.BMP
Tmh-C01c_cap.bmp
Tmh-C01c_tie.bmp
```



Então no redesign teremos provavelmente uma separação bastante conveniente:

```text
Pikeman
│
├── BODY / ARMOR
│   └── TmbCxx...
│
└── HEAD / FACE / HAIR
    └── Tmh-Cxx...
```

Isso é ótimo para o projeto.

### Próximo objetivo

Primeiro quero ver o output de:

```text
strings TmbC01.smd
```

Se ele disser explicitamente:

```text
char\tmABCD\TmbC01.bmp
```

aí fazemos no `TmbC01.bmp` exatamente o mesmo teste que fizemos na Priestess: decode → metade magenta → recoloca header → instala → abre o Pikeman.

Se o Pike ficar magenta, aí teremos **PROVADO** o primeiro pipeline gráfico dele e já podemos partir para um repaint real da armadura inicial.
Agora ficou bem fechado.

O `TmbC01.smd` contém ao mesmo tempo objetos das famílias `C01` e `C03`:

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

e referencia explicitamente:

```text
char\tmABCD\TmbC01.bmp
char\tmABCD\TmbC03.bmp
```

Então ele parece funcionar como um modelo/container com partes ou variantes de pelo menos essas duas armaduras. Isso explica por que não devemos presumir que cada `.smd` corresponde a uma única textura.

Além disso, `TmbC01.bmp` e o `MfbC01.bmp` da Sacerdotisa têm **exatamente o mesmo formato proprietário**:

```text
tamanho: 196662 bytes
header:  41 38 3a 09 13 19 24 31 40 51 9a 79 90 a9
DIB:     40
dimensão: 256 × 256
bpp:     24
```

Ou seja: podemos aplicar ao Pike exatamente o procedimento que já funcionou na Priestess.

## Próximo teste: somente `TmbC01.bmp`

Vamos fazer tudo numa pasta isolada e modificar **só essa textura**.

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
WORK="/home/gabriel/Área de Trabalho/summit/design_work/pike_tmbc01"

mkdir -p "$WORK"

cp -a \
  "$CLIENT/char/tmABCD/TmbC01.bmp" \
  "$WORK/TmbC01.original.bmp"

sha256sum \
  "$CLIENT/char/tmABCD/TmbC01.bmp" \
  "$WORK/TmbC01.original.bmp"
```

Agora cria de uma vez:

```text
TmbC01.decoded.bmp
TmbC01.magenta.bmp
TmbC01.game_test.bmp
```

com este bloco:

```bash
python3 - <<'PY'
from pathlib import Path
import struct

client = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "Summit Tale Client Full/cliente/char/tmABCD"
)

work = Path(
    "/home/gabriel/Área de Trabalho/summit/"
    "design_work/pike_tmbc01"
)

src = client / "TmbC01.bmp"

original = src.read_bytes()

assert len(original) == 196662

custom_header = original[:14]

print("CUSTOM HEADER =", custom_header.hex(" "))

# ============================================================
# 1. DECODE
# ============================================================

decoded = bytearray(original)

decoded[:14] = struct.pack(
    "<2sIHHI",
    b"BM",
    len(decoded),
    0,
    0,
    54
)

decoded_path = work / "TmbC01.decoded.bmp"
decoded_path.write_bytes(decoded)

# ============================================================
# 2. VALIDA BMP
# ============================================================

assert decoded[:2] == b"BM"
assert int.from_bytes(decoded[14:18], "little") == 40

width = int.from_bytes(
    decoded[18:22],
    "little",
    signed=True
)

height = int.from_bytes(
    decoded[22:26],
    "little",
    signed=True
)

planes = int.from_bytes(
    decoded[26:28],
    "little"
)

bpp = int.from_bytes(
    decoded[28:30],
    "little"
)

offset = int.from_bytes(
    decoded[10:14],
    "little"
)

print("WIDTH  =", width)
print("HEIGHT =", height)
print("PLANES =", planes)
print("BPP    =", bpp)
print("OFFSET =", offset)

assert width == 256
assert abs(height) == 256
assert planes == 1
assert bpp == 24
assert offset == 54

# ============================================================
# 3. METADE MAGENTA
# ============================================================

test = bytearray(decoded)

h = abs(height)

stride = ((width * 3 + 3) // 4) * 4

for y in range(h):

    row = offset + y * stride

    for x in range(width // 2):

        p = row + x * 3

        # BMP = BGR
        test[p + 0] = 255
        test[p + 1] = 0
        test[p + 2] = 255

magenta_path = work / "TmbC01.magenta.bmp"
magenta_path.write_bytes(test)

# ============================================================
# 4. RECOLOCA HEADER DO PRISTON
# ============================================================

game = bytearray(test)

game[:14] = custom_header

assert len(game) == len(original)

game_path = work / "TmbC01.game_test.bmp"
game_path.write_bytes(game)

print()
print("CRIADOS:")
print(decoded_path)
print(magenta_path)
print(game_path)

print()
print("GAME HEADER =", game[:14].hex(" "))
PY
```

Verifica o BMP editável:

```bash
WORK="/home/gabriel/Área de Trabalho/summit/design_work/pike_tmbc01"

file "$WORK/TmbC01.decoded.bmp"
identify "$WORK/TmbC01.decoded.bmp"

file "$WORK/TmbC01.magenta.bmp"
identify "$WORK/TmbC01.magenta.bmp"
```

E pode abrir:

```bash
xdg-open "$WORK/TmbC01.magenta.bmp"
```

Deve aparecer a texture map do Pike com metade magenta.

## Instala para o teste in-game

Primeiro faz o backup independente:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

cp -a \
 "$CLIENT/char/tmABCD/TmbC01.bmp" \
 "$CLIENT/char/tmABCD/TmbC01.bmp.before_design_test"

sha256sum \
 "$CLIENT/char/tmABCD/TmbC01.bmp" \
 "$CLIENT/char/tmABCD/TmbC01.bmp.before_design_test"
```

Depois instala:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
WORK="/home/gabriel/Área de Trabalho/summit/design_work/pike_tmbc01"

cp -a \
 "$WORK/TmbC01.game_test.bmp" \
 "$CLIENT/char/tmABCD/TmbC01.bmp"

echo "========== INSTALADO =========="

sha256sum \
 "$CLIENT/char/tmABCD/TmbC01.bmp" \
 "$CLIENT/char/tmABCD/TmbC01.bmp.before_design_test"

echo
xxd -g1 -l 32 \
 "$CLIENT/char/tmABCD/TmbC01.bmp"
```

**Não precisa recompilar.**

Abre o Pike.

O resultado pode nos dizer três coisas:

* se o corpo/armadura do Pike ficar magenta, `TmbC01.bmp` está **PROVADO** como textura usada pelo visual atual;
* se apenas algumas partes ficarem magenta, já saberemos exatamente quais regiões/material usam C01 e provavelmente as demais vêm de `TmbC03.bmp`;
* se absolutamente nada mudar, então o Pike que estás vendo está usando outro `TmbCxx`.

Para restaurar:

```bash
CLIENT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"

cp -a \
 "$CLIENT/char/tmABCD/TmbC01.bmp.before_design_test" \
 "$CLIENT/char/tmABCD/TmbC01.bmp"
```

Um detalhe interessante: como `TmbC01.smd` traz `C01` **e** `C03`, se o primeiro teste mostrar apenas parte do personagem magenta, não devemos alterar C03 imediatamente. Primeiro observamos **qual parte** mudou. Isso vai começar a nos revelar como o modelo divide peito, pernas, braços etc. entre as texture maps — informação muito útil para criar depois um template de pintura do Pikeman no Krita/Substance/Blender.

