# Priston Tale 3D — Source Única de Referência

> Compilação consolidada dos materiais e discussões desta conversa sobre edição de modelos 3D no Priston Tale, com foco principal em **armaduras, heads, meshes de personagem, ASE, Physique, UV, materiais e integração com o cliente**.
>
> Esta source elimina mensagens de fórum sem utilidade prática, repetições e palpites contraditórios. Onde os materiais não confirmam algo, isso é explicitamente marcado.

---

# 1. Objetivo

O workflow documentado aqui serve para:

- editar armaduras existentes;
- substituir partes de armadura;
- criar novas peças visuais usando uma armadura funcional como base;
- editar heads/capacetes e peças rígidas;
- trabalhar com `.ASE` de personagem;
- preservar rigging/Physique;
- corrigir UV, materiais e texturas;
- integrar a nova geometria no arquivo que o jogo já reconhece.

A estratégia mais segura é:

```text
INX / INI
   ↓
descobrir arquivo ASE + nomes de mesh
   ↓
usar um ASE funcional como base
   ↓
modelar nova peça
   ↓
UV + textura
   ↓
posicionar no personagem
   ↓
Physique / bones
   ↓
Export Selected
   ↓
editar/integrar dados no ASE funcional
   ↓
corrigir materiais e caminhos
   ↓
testar no cliente
```

A filosofia principal é **não recriar toda a estrutura da armadura do zero** quando existe um ASE funcional que já contém bones, hierarquia, nomes e estrutura aceitos pelo jogo.

---

# 2. Arquivos principais

## 2.1 Arquivos de personagem

Os materiais descrevem os arquivos aproximadamente assim:

```text
.INX  → configuração compilada da armadura/personagem
.INI  → versão legível da configuração
.SMD  → modelo 3D usado pelo cliente
.SMB  → dados de movimento/animação associados
.ASE  → representação ASCII/editável do modelo
.BMP  → textura bitmap
.TGA  → textura com suporte a alpha em determinados usos
```

O `.INX/.INI` é especialmente importante porque informa:

- qual arquivo de modelo é carregado;
- quais nomes de mesh o jogo procura;
- ligação com dados de animação/bones.

---

# 3. Diretórios e representações diferentes

## 3.1 Modelo vestido pelo personagem

Os materiais antigos usam como referência:

```text
Char/TMABCD
```

Essa pasta contém os modelos que os personagens realmente vestem.

## 3.2 Item no chão / inventário

Os modelos que aparecem jogados no chão e as imagens de inventário ficam em estruturas relacionadas a:

```text
Image/Sinimage/Items
```

Subdiretórios citados:

```text
DropItem
Defense
Accessory
```

Separação descrita:

```text
DropItem:
- modelo ASE do item no chão
- textura do item no chão

Defense:
- imagem de inventário de armor / boots / gauntlets

Accessory:
- imagem de inventário de bracelets
```

Tamanhos citados nos materiais antigos:

```text
66x88 → inventário de armor / boots / gauntlets
44x44 → inventário de bracelets
```

Não confunda:

```text
armadura vestida
≠
armadura no chão
≠
ícone de inventário
```

São assets diferentes.

---

# 4. Convenções de classe

## 4.1 Classes citadas nos materiais mais antigos

```text
A  = Mechanician
B  = Fighter
C  = Pikeman
D  = Archer

mA = Knight
mB = Atalanta
mC = Priestess
mD = Magician
```

## 4.2 Classes adicionais citadas no material mais recente

```text
E  = Assassin
mE = Shaman
mF = Martial Artist
```

Logo, a convenção histórica `TMABCD` não representa literalmente todas as classes de builds posteriores.

---

# 5. Motion files / SMB por classe

O material mais recente fornecido na conversa lista:

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

Use isso como referência para identificar qual conjunto de movimento/animação está relacionado ao personagem.

Para o workflow básico de substituição de mesh, normalmente a ideia é **reaproveitar o SMB existente**, não recriá-lo.

---

# 6. Exemplos históricos de ASE funcionais

Os guias antigos citam ASEs que eram conhecidos por funcionar:

```text
tmbA14_p.ase → Mechanician
tmbB14_p.ase → Fighter
TmbC14_p.ase → Pikeman
tfbD14_p.ase → Archer

MmbA16.ase   → Knight
MfbB18_p.ase → Atalanta
MfbC11_p.ase → Priestess
MmbD11_p.ase → Magician
```

Esses nomes são exemplos históricos.

Não trate essa lista como garantia de que sua distribuição atual usa exatamente os mesmos arquivos.

A regra correta é:

```text
descobrir qual INX/INI sua build realmente usa
```

---

# 7. Como descobrir qual modelo e quais meshes o jogo espera

Em um dos guias, uma conta administrativa podia usar:

```text
F9
```

para visualizar qual INX o personagem estava utilizando.

Isso depende da build.

Se existir no seu cliente, use como ferramenta de diagnóstico.

Depois abra o `.INI` equivalente ou uma versão legível da configuração.

Um exemplo apresentado nos materiais:

```text
*모양파일----- tmbA14_p.ASE

*정밀모양----- "TAHA19"
*정밀모양----- "TBHA19"
*정밀모양----- "TLHA19"
*정밀모양----- "TSHA19"

*보통모양----- "TAHA19"
*보통모양----- "TBHA19"
*보통모양----- "TLHA19"
*보통모양----- "TSHA19"

*저질모양----- "TAHA19"
*저질모양----- "TBHA19"
*저질모양----- "TLHA19"
*저질모양----- "TSHA19"

*파일연결----- M1Bip.in
```

Interpretação prática:

```text
modelo → arquivo ASE
precisão/alta → nomes de meshes high-res
normal → nomes de meshes medium
baixa → nomes de meshes low
file connection → ligação com bones/animação
```

A tradução exata dos rótulos coreanos não é o ponto mais importante.

O importante é:

```text
o INX/INI informa quais nomes de mesh o jogo procura
```

---

# 8. Convenção dos nomes de mesh

Exemplo documentado:

```text
TAHA19
```

Interpretação usada pelo guia:

```text
T  = Tempskron
A  = Arms
H  = High Resolution
A  = Mechanician
19 = número/conjunto da armadura
```

Partes:

```text
A = Arms
B = Body
S = Shoulder
L = Legs
```

Resolução:

```text
H = High
M = Medium
L = Low
```

Exemplo dado para uma ombreira de Atalanta:

```text
MSHB19
```

Interpretado como:

```text
M = Morion
S = Shoulder
H = High Resolution
B = Atalanta
19 = conjunto
```

## Regra crítica

A mesh final precisa manter o nome esperado pelo `INX/INI`.

Não deixe nomes arbitrários como:

```text
Plane01
Cube003
Object001
MyShoulder
```

quando o jogo espera algo como:

```text
MSHB19
```

Os materiais relacionam nomes errados de mesh com objetos invisíveis no jogo.

---

# 9. Backup obrigatório

Antes de editar:

```text
ASE original
SMD original
INI / INX
SMB
texturas
```

Estrutura recomendada:

```text
ArmorMod/
├── original/
├── working/
├── exported/
└── textures/
```

Nunca trabalhe diretamente sobre a única cópia funcional.

---

# 10. Estratégia recomendada: editar um ASE funcional

O método mais confiável documentado é:

```text
ASE funcional
+
dados da nova mesh
```

em vez de:

```text
ASE novo criado completamente do zero
```

Preserve no ASE funcional tudo o que já está correto:

- hierarquia;
- bones;
- estrutura geral;
- nomes;
- organização dos objetos;
- sintaxe que o jogo já aceita.

Substitua somente os blocos necessários.

---

# 11. Ferramentas citadas

## 11.1 Toolchain legado

Os materiais antigos recomendam:

```text
3ds Max 7
+
Sandurr ASE Exporter
```

Motivo:

```text
o exporter suporta PHYSIQUE
```

Também aparecem referências históricas ao Max 5 e 6, mas o guia preferia o Max 7.

Um relato antigo menciona que o Sandurr exporter só passou a carregar corretamente após instalação do runtime:

```text
Microsoft Visual C++ 2005
```

e reparo da instalação do 3ds Max 7.

Isso é troubleshooting de software legado.

## 11.2 Toolchain moderno citado

O tutorial mais recente enviado pelo usuário cita:

```text
3ds Max 2020
.ASE Export & Import 2025 (By IgorSegalla)
XNA Lara XPS
Microsoft XNA Framework Redistributable 3.0
```

Porém, o trecho fornecido não contém a parte de importação/exportação — ela ainda estava marcada como:

```text
soon...
```

Portanto:

**não está confirmado pelos materiais disponíveis se o exporter 2025 substitui completamente o Sandurr para Physique/bones.**

Até essa capacidade ser verificada, trate o Sandurr workflow como o processo documentado para preservar `PHYSIQUE`.

---

# 12. Modelagem

Modele a peça normalmente:

```text
capacete
head
ombreira
braço
peitoral
perna
bota
acessório rígido
```

Os materiais insistem em:

```text
Think low-poly
```

O PT é um engine antigo.

Não existe nos materiais usados aqui um limite numérico confiável e universal de polígonos.

Portanto:

- mantenha a primeira versão simples;
- reduza geometria desnecessária;
- teste em etapas;
- não comece com um modelo extremamente denso.

Uma ombreira simples é recomendada como primeiro teste.

---

# 13. UV e textura antes do rig

Workflow recomendado:

```text
1. finalizar geometria
2. criar UV
3. aplicar textura
4. conferir visualmente
5. só então riggar
```

Se o modelo for criado em uma versão moderna do Max e depois passado para um Max antigo:

- preserve o UVW;
- se necessário, salve/carregue os dados UVW;
- confira a malha antes da exportação final.

---

# 14. Posicionamento no personagem

Importe/carregue um modelo funcional como referência.

Ajuste:

```text
posição
escala
rotação
orientação
```

A nova peça deve encaixar corretamente no corpo original antes do rig.

Exemplo:

```text
nova ombreira
↓
encaixar no ombro da armadura funcional
```

---

# 15. Physique

No workflow legado:

```text
Modifiers
→ Physique
→ Attach to Node
```

Depois:

```text
Physique
→ Vertex
```

Os materiais orientam:

```text
No Blending
```

e então:

```text
Select
→ Assign to Link
```

A lógica é associar cada região da mesh ao link/bone correto.

---

# 16. Associação dos vértices

Exemplo de peça rígida:

```text
todos os vértices
→ um único bone
```

Exemplo apresentado em um tutorial:

```text
Bip01 Head
```

para uma pequena peça presa à cabeça.

Para uma peça de braço/ombro, use o link apropriado.

Não invente nomes de bone.

Use nomes existentes no arquivo funcional.

---

# 17. Todos os vértices precisam estar ligados

Os materiais alertam que vértices não associados corretamente podem causar:

```text
stretching
mesh deformada
vértices presos
deformações extremas
```

Portanto:

```text
cada vértice relevante
→ precisa estar corretamente associado
```

---

# 18. Teste o rig antes de exportar

No 3ds Max:

1. selecione o bone/link;
2. rotacione;
3. movimente;
4. observe a peça.

A peça não deve:

- ficar parada;
- se separar;
- esticar de forma absurda;
- girar em torno de um ponto incorreto.

Só exporte depois que o rig funcionar no Max.

---

# 19. Exporte uma peça por vez

O workflow mais organizado é:

```text
shoulder.ase
arms.ase
body.ase
legs.ase
```

Use:

```text
Export Selected
```

Vantagens:

- ASE menor;
- apenas um `GEOMOBJECT` relevante;
- integração textual mais fácil;
- menor chance de misturar UV/material de várias peças.

Há relatos nos materiais de que exportar várias peças de uma vez podia prejudicar UVW.

---

# 20. Mapping Coordinates

Ao exportar, preserve:

```text
Mapping Coordinates
```

Isso é essencial para:

```text
*MESH_TVERTLIST
*MESH_TFACELIST
```

Sem isso, a geometria pode aparecer, mas a textura ficará incorreta.

---

# 21. Workaround para UV quebrado no Sandurr

Um workaround avançado relatado:

```text
Export A:
Max 7 + Sandurr
→ usar PHYSIQUE

Export B:
Max moderno
→ usar mapping/UV
```

Depois combinar manualmente:

```text
MESH / TVERT / TFACE do export moderno
+
PHYSIQUE do Sandurr
```

Use somente se o Sandurr preservar Physique mas corromper mapping.

---

# 22. Edição textual do ASE

Abra:

```text
ASE funcional
ASE da nova peça
```

em um editor como:

```text
VS Code
Notepad++
MadEdit
```

O Notepad antigo pode ficar lento em arquivos grandes.

---

# 23. GEOMOBJECT

No ASE, objetos geométricos aparecem como:

```text
*GEOMOBJECT {
```

Dentro dele podem aparecer:

```text
*NODE_NAME
*NODE_TM
*MESH
*MATERIAL_REF
*PHYSIQUE
```

A nova peça normalmente será integrada em um `GEOMOBJECT` correspondente à parte que o jogo espera.

---

# 24. Blocos importantes do MESH

Os materiais destacam:

```text
*MESH_VERTEX_LIST
```

coordenadas dos vértices.

```text
*MESH_FACE_LIST
```

faces, smoothing e IDs de material.

```text
*MESH_NUMTVERTEX
*MESH_TVERTLIST
```

coordenadas UV.

```text
*MESH_NUMTVFACES
*MESH_TFACELIST
```

faces de UV.

```text
*MESH_NORMALS
```

normais.

```text
*PHYSIQUE
```

associação vértice → bone/link.

---

# 25. Preservar sintaxe funcional

Exporters diferentes podem produzir ASEs com pequenas diferenças.

Por isso:

```text
ASE original funcional
→ referência estrutural
```

Evite substituir o arquivo inteiro por um ASE recém-exportado se o objetivo é apenas trocar uma parte.

Prefira:

```text
estrutura funcional original
+
dados da nova mesh
```

---

# 26. MATERIAL_REF x MESH_MTLID

São coisas diferentes.

```text
*MATERIAL_REF 0
```

seleciona o material principal/top-level usado pelo objeto.

Dentro das faces:

```text
*MESH_MTLID 15
```

seleciona o submaterial aplicado àquela face.

---

# 27. MATERIAL_COUNT x NUMSUBMTLS

Também são contadores diferentes.

Exemplo:

```text
*MATERIAL_COUNT 2
```

quantidade de materiais top-level.

Dentro de um `Multi/Sub-Object`:

```text
*NUMSUBMTLS 15
```

quantidade de submateriais.

Se existem IDs:

```text
0 até 14
```

e você adiciona:

```text
*SUBMATERIAL 15
```

então atualize:

```text
*NUMSUBMTLS 16
```

---

# 28. Exemplo de submaterial

```text
*SUBMATERIAL 15 {
    *MATERIAL_NAME "CustomArmor"
    *MATERIAL_CLASS "Standard"
    *MATERIAL_AMBIENT 0.5880 0.5880 0.5880
    *MATERIAL_DIFFUSE 0.5880 0.5880 0.5880
    *MATERIAL_SPECULAR 0.9000 0.9000 0.9000
    *MATERIAL_SHINE 0.1000
    *MATERIAL_SHINESTRENGTH 0.0000
    *MATERIAL_TRANSPARENCY 0.0000
    *MATERIAL_SHADING Blinn

    *MAP_DIFFUSE {
        *MAP_NAME "Map #1"
        *MAP_CLASS "Bitmap"
        *MAP_SUBNO 1
        *MAP_AMOUNT 1.0000
        *BITMAP "CustomArmor\\armor.bmp"
        *MAP_TYPE Screen
    }
}
```

Depois associe as faces:

```text
*MESH_MTLID 15
```

---

# 29. PHYSIQUE manual

O workflow preferível é exportar o `PHYSIQUE`.

Porém, um tutorial anterior mostrou que ele também pode ser construído manualmente.

Exemplo:

```text
*PHYSIQUE {
    *PHYSIQUE_NUMVERTEXASSIGNMENT 4

    *PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE 0 "Bip01 Head"
    *PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE 1 "Bip01 Head"
    *PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE 2 "Bip01 Head"
    *PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE 3 "Bip01 Head"
}
```

Use isso como fallback.

Para uma mesh grande, editar centenas de associações manualmente é muito menos prático que exportar Physique corretamente.

---

# 30. Contagem de vértices e Physique

Se:

```text
*MESH_NUMVERTEX 120
```

o `PHYSIQUE` precisa fazer referência de maneira válida aos vértices dessa mesh.

Inconsistências entre:

```text
índices reais de vértices
PHYSIQUE_NUMVERTEXASSIGNMENT
PHYSIQUE_VERTEXASSIGNMENT...
```

podem gerar deformações ou crash.

---

# 31. NODE_NAME

No tutorial mais antigo havia uma tentativa prática de reutilizar nomes existentes no ASE quando um objeto não aparecia.

Com os materiais posteriores, a regra pode ser formulada de forma melhor:

```text
o nome do objeto/mesh deve respeitar o que o INX/INI espera
```

Ou seja:

não trate `NODE_NAME` como tentativa e erro arbitrária.

Primeiro descubra o nome correto na configuração.

---

# 32. Múltiplas meshes para uma mesma parte

Um dos guias relata o uso de mais de uma geometria para a mesma parte lógica.

Exemplo:

```text
mesh base
+
mesh overlay para reflexo/efeito
```

É possível que ambas sejam usadas sob o mesmo nome lógico esperado para a peça.

Trate isso como recurso avançado.

Primeiro faça uma única mesh funcionar.

---

# 33. Texturas

## 33.1 BMP

Boa escolha para primeiro teste:

```text
BMP 24-bit
```

Mais simples para diagnóstico.

## 33.2 TGA

Os materiais documentam TGA com alpha para transparência seletiva.

Use quando realmente precisar de:

```text
alpha
overlay
efeito específico
```

---

# 34. Caminhos de textura

Evite caminhos absolutos como:

```text
C:\Users\Nome\Desktop\Projeto\armor.bmp
```

Os materiais de troubleshooting dizem que o cliente precisa de caminhos relativos apropriados ao tipo de objeto.

A base desse caminho pode variar conforme a rotina de renderização.

Regra prática:

```text
abra um ASE funcional do mesmo tipo
→ copie a convenção de path
```

Não invente uma estrutura diferente sem necessidade.

---

# 35. Texturas são client-side

Um dos relatos técnicos destaca que o servidor não precisa das texturas para renderizar o objeto.

Se a mesh está:

```text
branca
sem textura
com textura incorreta
```

investigue primeiro o cliente e os paths do ASE.

---

# 36. Tamanho de textura

Um troubleshooting específico relata:

```text
1024x1024 → grande demais naquele ambiente
512x512  → citado como máximo
256x256  → recomendado para peças pequenas
128x128  → usado com sucesso em teste
```

Como isso é relato de uma build antiga, trate como regra prática conservadora, não como especificação universal.

Para começar:

```text
128x128 ou 256x256
```

Depois aumente somente se necessário.

---

# 37. Troubleshooting — textura branca

Confira:

```text
arquivo existe?
nome está correto?
extensão está correta?
ASE referencia BMP mas arquivo é TGA?
ASE referencia TGA mas arquivo é BMP?
path está correto?
path é relativo?
arquivo está no diretório correto?
```

---

# 38. Troubleshooting — textura embaralhada

Confira:

```text
MESH_MTLID
MESH_TVERTLIST
MESH_TFACELIST
Mapping Coordinates
UV original
```

Diferença:

```text
Material ID errado
→ polígono recebe material/textura errada

UV errado
→ textura correta aparece deformada
```

---

# 39. Troubleshooting — UV quebra depois da exportação

Tente nesta ordem:

```text
1. exportar apenas uma peça
2. habilitar Mapping Coordinates
3. preservar UVW ao trocar de versão do Max
4. aplicar/recarregar Unwrap UVW
5. comparar TVERT/TFACE entre exports
6. usar mapping de Max moderno + PHYSIQUE do Sandurr
```

---

# 40. Troubleshooting — mesh invisível

Confira:

```text
INX/INI pede esse nome?
NODE_NAME / mesh name está correto?
ASE correto está sendo carregado?
GEOMOBJECT existe?
estrutura está no nível correto?
MATERIAL_REF existe?
MESH_MTLID é válido?
```

A primeira suspeita deve ser:

```text
nome da mesh ≠ nome esperado pelo INX
```

---

# 41. Troubleshooting — crash / game.exe

Investigue:

```text
bone incorreto
PHYSIQUE inválido
índices de vértice inválidos
chaves { } quebradas
contadores errados
MESH inconsistente
estrutura ASE inválida
```

Os materiais associam bones/Physique incorretos a crashes do cliente.

---

# 42. Troubleshooting — stretching

Confira:

```text
todos os vértices estão ligados?
bone correto?
link correto?
No Blending?
algum vértice ficou sem assignment?
```

Teste novamente movimentando o bone no Max.

---

# 43. Troubleshooting — peça em posição errada

Corrija no software 3D:

```text
posição
escala
rotação
transform
```

Não tente corrigir um grande erro de alinhamento editando dezenas de coordenadas manualmente no ASE se ainda possui o projeto 3D.

---

# 44. Troubleshooting — material errado em parte da mesh

Confira:

```text
*MESH_MTLID
```

face por face.

Uma mesh pode ter:

```text
UV correto
textura correta
geometria correta
```

e ainda parecer errada porque certas faces usam o submaterial incorreto.

---

# 45. Troubleshooting — modelo pesado

Se uma geometria simples funciona e a final não:

```text
reduza polígonos
reduza textura
simplifique materiais
teste por partes
```

Não existe nos materiais compilados aqui um limite numérico universal confiável de triângulos.

---

# 46. Erros do Sandurr exporter

Há relatos de que o Sandurr pode exibir erros durante exportações riggadas e ainda assim gerar um arquivo utilizável.

Isso é uma observação de fórum, não uma garantia.

Regra segura:

```text
não confie somente na mensagem de erro
→ verifique se o ASE foi gerado
→ abra-o
→ confira PHYSIQUE
→ teste em cópia do cliente
```

---

# 47. Heads e capacetes

O material mais recente cita como base:

```text
OG Head Models
61 Helmets
```

mas o trecho enviado não contém o tutorial de importação/exportação desses assets.

A lógica geral documentada para uma peça rígida de head é:

```text
modelar
UV
posicionar
ligar ao bone da cabeça
exportar
integrar no ASE funcional
```

O nome exato do bone deve ser copiado de um modelo funcional.

Um exemplo histórico usa:

```text
Bip01 Head
```

Não assuma que toda build usa exatamente o mesmo nome sem conferir.

---

# 48. Efeitos avançados / overlay

Outro material antigo descreve:

```text
duplicar GEOMOBJECT
→ mudar material
→ usar self illumination
→ usar TGA com alpha
```

para efeitos seletivos.

Isso pode ser usado em:

```text
reflexo
brilho
overlay
partes transparentes
```

É avançado e não faz parte do primeiro teste.

---

# 49. Fluxo mínimo recomendado para aprender

Faça primeiro uma ombreira simples.

```text
1. escolher uma armadura funcional
2. descobrir o INX/INI
3. descobrir o nome exato da ombreira
4. fazer backup
5. modelar uma ombreira low-poly
6. criar UV
7. aplicar BMP pequena
8. posicionar no personagem
9. aplicar Physique
10. usar No Blending
11. ligar todos os vértices ao link correto
12. testar movimentando o bone
13. Export Selected
14. usar exporter que preserve Physique
15. preservar Mapping Coordinates
16. abrir ASE novo
17. abrir ASE funcional
18. localizar o GEOMOBJECT original
19. preservar o nome esperado pelo INX
20. integrar MESH
21. integrar UV
22. integrar PHYSIQUE
23. corrigir MATERIAL_REF
24. corrigir MESH_MTLID
25. adicionar/corrigir submaterial
26. corrigir path de textura
27. salvar
28. testar no cliente
```

Se isso funcionar, você comprovou todo o pipeline:

```text
INX
+
mesh name
+
modelo
+
UV
+
material
+
Physique
+
ASE
+
cliente
```

Depois passe para peças mais complexas.

---

# 50. Testes no jogo

Teste pelo menos:

```text
Idle
Walk
Run
Attack
Skill
Death
```

Observe separadamente:

```text
posição
escala
rotação
textura
UV
smoothing
rigging
```

---

# 51. Workflow consolidado

```text
                 INX / INI
                     │
                     ▼
        descobrir ASE + mesh names
                     │
                     ▼
          ASE funcional original
                     │
          ┌──────────┴──────────┐
          │                     │
          │                modelar peça
          │                     │
          │                     ▼
          │                    UVW
          │                     │
          │                     ▼
          │                  textura
          │                     │
          │                     ▼
          │             posicionar no char
          │                     │
          │                     ▼
          │                  Physique
          │                     │
          │             No Blending / Links
          │                     │
          │                     ▼
          │             testar movimentação
          │                     │
          │                     ▼
          │               Export Selected
          │                     │
          │      exporter com suporte a rig
          │                     │
          └──────────┬──────────┘
                     ▼
                edição textual
                     │
              preservar mesh name
                     │
              integrar MESH
                     │
               integrar UV
                     │
            integrar PHYSIQUE
                     │
         MATERIAL_REF / MTLID
                     │
              path relativo
                     │
                     ▼
                  CLIENT
                     │
                     ▼
                   TESTAR
```

---

# 52. O que está confirmado pelos materiais

Os materiais compilados sustentam diretamente:

- `.ASE` como formato ASCII/editável usado nesse workflow;
- uso de ASE funcional como base;
- relação de `.INX/.INI` com modelo e nomes de mesh;
- necessidade de preservar nomes esperados pelo INX;
- uso de Physique;
- uso de `No Blending`;
- associação dos vértices a links/bones;
- importância de `PHYSIQUE`;
- importância de Mapping Coordinates;
- distinção entre `MATERIAL_REF` e `MESH_MTLID`;
- uso de submateriais;
- importância de UV;
- exportação de peças separadamente;
- caminhos de textura relativos;
- BMP/TGA;
- uso de TGA alpha em efeitos específicos;
- separação entre armadura vestida e drop item;
- classes e motion files listados;
- troubleshooting de UV, textura, rig e nomes de mesh.

---

# 53. O que ainda não está confirmado

Com os materiais enviados nesta conversa, ainda não está confirmado:

## 53.1 ASE Export & Import 2025

O tópico moderno cita:

```text
.ASE Export & Import 2025 (By IgorSegalla)
```

mas não foi fornecida documentação técnica mostrando:

- se importa Physique;
- se exporta Physique;
- se preserva bones;
- se substitui o Sandurr;
- quais opções de exportação usar;
- compatibilidade exata com 3ds Max 2020.

Portanto não é correto afirmar ainda que ele elimina o workflow legado.

## 53.2 XNA Lara XPS

O programa foi citado como ferramenta, mas não foi fornecido um pipeline técnico completo:

```text
XPS → Max → ASE → PT
```

Logo, ele não faz parte do procedimento principal desta source.

## 53.3 Adicionar uma armadura totalmente nova sem substituir outra

Os guias usados focam principalmente em:

```text
substituir / editar assets funcionais
```

A parte de registrar uma armadura totalmente nova no cliente/servidor/executáveis não está documentada de maneira suficiente aqui.

## 53.4 Limites exatos do engine

Não há nos materiais uma especificação técnica confiável e universal para:

```text
máximo de polígonos
máximo de vértices
máximo de materiais
```

Existem apenas relatos práticos recomendando low-poly.

---

# 54. Regra final

O método mais confiável compilado de todos os materiais é:

```text
usar um ASE funcional
+
usar o INX/INI como autoridade para nomes
+
modelar/UV/riggar a nova peça
+
exportar dados compatíveis
+
preservar PHYSIQUE
+
integrar somente o necessário
+
usar materiais e paths compatíveis
+
testar em etapas
```

Isso minimiza:

```text
mesh invisível
textura branca
UV quebrado
stretching
crash
game.exe
```

e mantém o máximo possível da estrutura original que o Priston Tale já reconhece.

---

# 55. Fontes consolidadas usadas nesta conversa

Esta source foi compilada a partir de:

- tutorial antigo de edição manual de ASE e inserção de `GEOMOBJECT`, submaterial e `PHYSIQUE`;
- guia “Notes on Armors / Armors 101” e respostas técnicas relacionadas;
- troubleshooting de UV, textura, paths e complexidade de modelo;
- documentação de diretórios e formatos do Priston Tale;
- tópico moderno “Customizando Modelos 3D” fornecido pelo usuário;
- informações modernas fornecidas pelo usuário sobre classes adicionais e motion files;
- discussões e refinamentos feitos ao longo desta conversa.

Mensagens de fórum sem conteúdo técnico relevante foram omitidas.

---

# 56. Estado atual da source

Esta versão deve ser tratada como:

```text
SOURCE OF TRUTH v1
```

para o workflow de:

```text
Armadura / Head / Mesh de personagem
→ ASE
→ UV
→ Material
→ Physique
→ integração
→ teste
```

Próxima expansão útil, caso apareça documentação confiável:

```text
ASE Export & Import 2025
Sandurr SMD Converter
workflow moderno confirmado
monstros
armas
mapas
animações
efeitos
```
---

# 57. Summit Tale — resultados verificados em 2026-08-23

Esta seção registra descobertas obtidas diretamente no fork **Summit Tale**, separando explicitamente o que foi confirmado por inspeção de arquivo, Blender e teste visual/runtime.

## 57.1 Convenção de evidência

Use estas etiquetas nesta documentação:

```text
PROVADO
    confirmado por teste direto e reproduzível.

CONFIRMADO ESTRUTURALMENTE
    estrutura/arquivo/parser conferido, mas sem fechar todo o comportamento
    dentro do cliente.

HIPÓTESE
    inferência ainda não testada.

PENDENTE
    etapa ainda não executada.
```

Não elevar uma hipótese a `PROVADO` apenas porque o formato "parece correto".

---

# 58. Ambiente atual da pipeline Blender

Blender confirmado:

```text
Blender 5.1.1
instalação Flatpak
application id: org.blender.Blender
```

O executável deve ser chamado como:

```bash
flatpak run --filesystem=home org.blender.Blender
```

O caminho `/usr/bin/blender` não deve ser usado neste ambiente.

O Flatpak emite atualmente um warning de OpenColorIO relacionado à diferença:

```text
config.ocio 2.5
OpenColorIO library 2.4.2
```

e entra em fallback de color management.

Esse warning não impediu:

- importação de geometria;
- UV;
- materiais;
- decode de textura;
- armature;
- Physique;
- deformação da mesh.

Portanto ele não é bloqueador da pipeline 3D atual.

---

# 59. Asset de referência usado na validação

ASE de teste:

```text
char/tmABCD/MfbC13.ASE
```

O teste visual confirmou que o conjunto importado corresponde à:

```text
Sacerdotisa
roupa / armadura de nível 80
```

Importante:

o nome do ASE contém `C13`, enquanto as meshes e materiais internos usam referências como:

```text
MshC19
MbhC19
MlhC20
MahC20

MfbC19_01.TGA
MfbC19_02.BMP
pr100_yun01.tga
pr100_yun02.tga
```

Portanto não inferir o nível ou a identidade visual apenas pelo número embutido no nome de um arquivo interno.

A autoridade prática continua sendo:

```text
configuração
+
referências internas
+
teste visual/runtime
```

---

# 60. Estrutura encontrada no MfbC13.ASE

Auditoria:

```text
GEOMOBJECT blocks: 67
```

Meshes texturizadas importadas:

```text
MshC19
    vertices: 163
    faces:    264
    TVERT:    250
    TFACE:    264
    material IDs: 0

MbhC19
    vertices: 143
    faces:    248
    TVERT:    212
    TFACE:    248
    material IDs: 0, 1

MlhC20
    vertices: 286
    faces:    428
    TVERT:    296
    TFACE:    428
    material IDs: 0, 1

MahC20
    vertices: 80
    faces:    146
    TVERT:    124
    TFACE:    146
    material IDs: 0
```

Totais:

```text
vertices geométricos: 672
faces:                1086
TVERT:                  882
TFACE:                 1086
```

A geometria apareceu reconhecível e em pé no Blender.

As quatro meshes formam o conjunto visual esperado.

A UV também foi inspecionada no Blender e apareceu corretamente.

Estado:

```text
GEOMETRIA: PROVADO
FACES:     PROVADO
UV:        PROVADO
```

---

# 61. MATERIAL_REF e MESH_MTLID — validação prática

A pipeline encontrou dois materiais top-level.

Material principal 0:

```text
SUB 0 -> MfbC19_01.TGA
SUB 1 -> MfbC19_02.BMP
```

Material principal 1:

```text
SUB 0 -> pr100_yun01.tga
SUB 1 -> pr100_yun02.tga
```

Mapeamento observado:

```text
MshC19
    MATERIAL_REF 0
    MTLID 0 -> MfbC19_01.TGA

MbhC19
    MATERIAL_REF 0
    MTLID 0 -> MfbC19_01.TGA
    MTLID 1 -> MfbC19_02.BMP

MlhC20
    MATERIAL_REF 1
    MTLID 0 -> pr100_yun01.tga
    MTLID 1 -> pr100_yun02.tga

MahC20
    MATERIAL_REF 1
    MTLID 0 -> pr100_yun01.tga
```

Todos os nomes foram resolvidos para um único arquivo existente no cliente.

Estado:

```text
ASE MATERIAL_REF -> material top-level: PROVADO
MESH_MTLID -> submaterial por face:     PROVADO
resolução de BITMAP no cliente:         PROVADO
```

---

# 62. BMP proprietário do Summit / Priston

## 62.1 Layout confirmado

Para BMPs testados como:

```text
MfbC01.bmp
TmbC01.bmp
MfbC19_02.bmp
```

foi encontrado um layout em que os primeiros 14 bytes substituem o `BITMAPFILEHEADER` padrão.

Exemplo de header proprietário já observado:

```text
41 38 3a 09 13 19 24 31 40 51 9a 79 90 a9
```

Depois desses 14 bytes começa um `BITMAPINFOHEADER` Windows normal:

```text
size:   40
width:  256
height: 256
planes: 1
bpp:    24
```

Tamanho típico:

```text
256 * 256 * 3 + 54
= 196662 bytes
```

## 62.2 Decode para edição

Para imagens desse layout 256x256 RGB24:

```text
1. salvar bytes 0..13;
2. substituir por BITMAPFILEHEADER normal;
3. manter bytes 14..fim;
4. editar como BMP padrão;
5. manter dimensão / bpp / tamanho;
6. recolocar exatamente os 14 bytes proprietários.
```

Header BMP padrão usado para decode desse caso:

```text
42 4d 36 00 03 00 00 00 00 00 36 00 00 00
```

A pipeline Blender reconstruiu:

```text
MfbC19_02.bmp
-> BMP padrão 256x256x24
-> Blender
```

e o material apareceu corretamente.

Estado:

```text
decode BMP protegido -> Blender: PROVADO
reproteção de BMP editado -> cliente: PROVADO em testes anteriores
```

Não generalizar ainda para todo BMP existente no jogo.

---

# 63. TGA proprietário — formato descoberto

Esta foi uma descoberta nova da pipeline Blender.

Arquivos testados:

```text
MfbC19_01.tga
pr100_yun01.tga
pr100_yun02.tga
```

Todos tinham:

```text
tamanho total = 262188 bytes
```

A estrutura foi comprovada como:

```text
18 bytes proprietários
+
256 * 256 * 4 bytes de pixels BGRA
+
26 bytes de footer TGA 2.0
```

Cálculo:

```text
18
+ 262144
+ 26
= 262188
```

Os 18 bytes proprietários observados:

```text
47 38 06 09 10 19 24 31 40 51 64 79 90 aa c4 e2 20 29
```

Footer observado:

```text
00 00 00 00 00 00 00 00
54 52 55 45 56 49 53 49 4f 4e 2d 58 46 49 4c 45 2e 00
```

Os últimos 18 bytes representam:

```text
TRUEVISION-XFILE.\0
```

## 63.1 Decode comprovado

Reconstrução:

```text
header TGA padrão de 18 bytes
image type 2
256x256
32-bit
8-bit alpha
+
data[18:-26]
```

O arquivo reconstruído foi reconhecido como:

```text
Targa image data - RGBA 256 x 256 x 32 - 8-bit alpha
```

e pelo ImageMagick como:

```text
TGA 256x256
8-bit sRGB
```

A V2.2 carregou os três TGAs no Blender sem falha e o modelo apareceu texturizado.

Estado:

```text
layout TGA deste grupo:          PROVADO
decode TGA protegido -> padrão:  PROVADO
TGA padrão -> Blender:           PROVADO
```

Ainda não foi comprovado que todos os TGAs do jogo usam exatamente este layout.

---

# 64. Pipeline Blender V1 — geometria e UV

Script:

```text
pt_ase_import_v1.py
```

Funções validadas:

```text
ASE
-> GEOMOBJECT
-> MESH_VERTEX
-> MESH_FACE
-> MESH_TVERT
-> MESH_TFACE
-> MESH_MTLID
-> Blender mesh
```

Resultado no `MfbC13.ASE`:

```text
67 GEOMOBJECT encontrados
4 meshes texturizadas importadas
63 objetos ignorados nesta fase
```

Teste visual:

```text
geometria reconhecível
em pé
quatro partes presentes
UV visível
```

Estado:

```text
V1: PROVADO
```

---

# 65. Pipeline Blender V2 / V2.2 — materiais e texturas

V2 adicionou:

```text
MATERIAL_LIST
SUBMATERIAL
MATERIAL_REF
MESH_MTLID
BITMAP
resolver de path
```

A primeira V2 descobriu um comportamento importante do Blender:

```text
bpy.data.images.load()
```

pode criar um datablock de imagem sem que o conteúdo tenha sido efetivamente decodificado.

Isso produziu inicialmente um falso:

```text
load=LOADED
```

e a falha só apareceu quando o `.blend` foi aberto.

A partir da V2.1/V2.2, o importer passou a:

```text
image.reload()
consultar image.size
tocar o pixel buffer
```

para forçar decode real durante a importação.

V2.2 corrigiu também a detecção do footer TGA.

Resultado final:

```text
Material texture loads:
    LOADED : 6
    MISSING: 0
    FAILED : 0
```

Arquivos decodificados:

```text
MfbC19_01...decoded.tga  -> 262162 bytes
MfbC19_02...decoded.bmp  -> 196662 bytes
pr100_yun01...decoded.tga -> 262162 bytes
pr100_yun02...decoded.tga -> 262162 bytes
```

Teste visual:

```text
personagem apareceu texturizado no Blender
```

Estado:

```text
V2.2: PROVADO
```

---

# 66. Pipeline Blender V3 — Biped / skeleton

Script:

```text
pt_ase_import_v3.py
```

Dados usados:

```text
NODE_NAME
NODE_PARENT
NODE_TM
TM_ROW3
```

Também são preservados como metadata para trabalho futuro:

```text
TM_ROW0
TM_ROW1
TM_ROW2
TM_ROW3
```

O importer cria:

```text
PT_BIPED
```

como Blender Armature.

Objetivo da V3:

```text
reconstruir posições dos joints
+
hierarquia pai/filho
```

A cauda visual dos bones no Blender é heurística.

Portanto:

```text
sticks não encostarem perfeitamente
!=
hierarquia incorreta
```

O teste relevante é se joints e cadeia lógica ficam corretamente posicionados dentro do personagem.

Teste visual confirmou:

```text
bones seguem coerentemente o personagem
hierarquia visual faz sentido
skeleton fica dentro da personagem
```

Estado:

```text
posições / hierarquia V3: PROVADO
bone roll exato:           NÃO PROVADO
orientação local exata:    NÃO PROVADO
```

---

# 67. Pipeline Blender V4 — PHYSIQUE / skinning

Script:

```text
pt_ase_import_v4.py
```

O formato rígido encontrado/esperado:

```text
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
    vertex_index
    "Bip bone name"
```

A V4 mapeia:

```text
ASE vertex index
-> Blender vertex group com o nome do Bip bone
-> weight 1.0
-> Armature Modifier
```

Isso é coerente com o workflow legado `No Blending`.

O importer preserva a ordem de vértices ASE ao criar a mesh Blender, permitindo que os índices do `PHYSIQUE` sejam usados diretamente.

## 67.1 Teste decisivo

No Blender:

```text
PT_BIPED
-> Pose Mode
-> escolher bone
-> G
```

Ao transladar um bone, a parte associada da mesh acompanhou.

Isso prova a cadeia:

```text
PHYSIQUE ASE
-> índice de vértice
-> nome Bip
-> vertex group
-> Armature Modifier
-> deformação da mesh
```

Estado:

```text
V4: PROVADO
```

Ainda pendente:

```text
bone roll exato
rotação local compatível com animação original
controladores de animação
round-trip de skeleton
```

---

# 68. Estado consolidado da pipeline em 2026-08-23

```text
V1
ASE -> geometry/faces/UV
PROVADO

V2.2
ASE -> materials -> protected BMP/TGA -> Blender
PROVADO

V3
ASE -> Biped hierarchy/joint positions
PROVADO

V4
ASE PHYSIQUE -> Blender vertex groups -> deformation
PROVADO
```

Pipeline atual:

```text
Priston ASE
      ↓
GEOMOBJECT
      ↓
Geometry / Faces
      ↓
UV
      ↓
MATERIAL_REF / MESH_MTLID
      ↓
BITMAP
      ↓
BMP/TGA proprietário
      ↓
decode
      ↓
Blender materials
      ↓
NODE hierarchy
      ↓
PT_BIPED
      ↓
PHYSIQUE
      ↓
vertex groups weight 1.0
      ↓
Armature Modifier
      ↓
mesh deformável
```

---

# 69. Estratégia de exportação Blender -> ASE

A exportação deve continuar incremental.

Não é seguro começar recriando um ASE inteiro do zero.

A estratégia recomendada agora é:

```text
ASE funcional original
+
Blender modificado
+
patch somente dos dados que realmente mudaram
```

Isso segue a regra já documentada anteriormente de preservar a estrutura de um ASE aceito pelo engine.

---

# 70. V5a — round-trip conservador de coordenadas

Objetivo:

```text
Blender V4
-> ASE funcional usado como template
-> validar que topologia / UV / Physique continuam iguais
-> substituir somente coordenadas MESH_VERTEX alteradas
```

V5a NÃO deve reescrever ainda:

```text
MESH_FACE_LIST
MESH_TVERTLIST
MESH_TFACELIST
MESH_NORMALS
PHYSIQUE
NODE_TM
materials
Biped
animation
```

## 70.1 Teste zero

Sem modificar o `.blend`:

```text
original ASE
-> V5a
-> exported ASE
```

Resultado obrigatório:

```text
SHA256 original == SHA256 exportado
```

ou seja:

```text
byte-identical round trip
```

Esse teste prova que o exporter não modifica dados que não deveriam mudar.

## 70.2 Teste de um único vértice

Depois:

```text
duplicar / fazer backup do .blend
-> Edit Mode
-> mover exatamente 1 vértice
-> V5a
```

Rodar o exporter com limite:

```text
--max-changed 1
```

Resultado esperado:

```text
1 vértice alterado
topologia idêntica
UV idêntica
Physique idêntico
restante do ASE preservado
```

Depois comparar:

```bash
diff -u original.ASE modificado.ASE
```

O diff deve ficar restrito à linha `MESH_VERTEX` escolhida.

Observação:

como `MESH_NORMALS` ainda é preservado nesta fase, o arquivo de um-vértice é apenas um teste de round-trip/pipeline, não o exporter final para remodelagem completa.

---

# 71. V5b — exporter completo de mesh

Somente após V5a funcionar.

V5b deverá implementar:

```text
MESH_NUMVERTEX
MESH_VERTEX_LIST

MESH_NUMFACES
MESH_FACE_LIST

MESH_NUMTVERTEX
MESH_TVERTLIST

MESH_NUMTVFACES
MESH_TFACELIST

MESH_NORMALS

MATERIAL_REF
MESH_MTLID

PHYSIQUE
```

Requisitos:

- triangulação determinística;
- vertex indices consistentes;
- UV per-loop convertido corretamente para TVERT/TFACE;
- IDs de material preservados;
- normals regeneradas;
- vertex groups convertidos novamente para Physique rígido;
- validação de bones inexistentes;
- nenhuma referência de vértice fora do range.

---

# 72. V5c / V6 — skeleton, animação e formato runtime

Depois da exportação completa de mesh:

```text
V5c
Blender Armature
-> NODE hierarchy / NODE_TM
-> bind transforms
```

Só depois:

```text
animações
```

O ponto crítico ainda não provado é reconstruir exatamente:

```text
bone orientation
bone roll
transform local
controllers/keyframes
```

A etapa runtime será:

```text
Blender
-> ASE compatível
-> conversão/carregamento aceito pelo Priston
-> SMD/SMB/INX quando necessário
-> cliente
```

O conversor/runtime exato para fechar:

```text
ASE editado -> SMD/SMB final
```

ainda precisa ser estabelecido e testado.

---

# 73. Regra de trabalho para o remaster

Para cada alteração:

```text
1. manter backup;
2. registrar SHA256;
3. alterar uma variável por teste;
4. validar estrutura;
5. validar visual;
6. só então marcar PROVADO;
7. nunca substituir famílias inteiras de assets sem necessidade.
```

Para personagem:

```text
mesh
-> UV
-> material
-> textura
-> Physique
-> runtime
```

Para textura:

```text
decode proprietário
-> editar formato padrão
-> preservar dimensão/bpp inicialmente
-> reencodar/reproteger
-> testar
```

Não aumentar resolução ou mudar formato interno até a pipeline original estar estável.

---

# 74. Estado desta documentação

A partir desta seção, o documento passa a ser:

```text
SUMMIT TALE 3D / BLENDER
SOURCE OF TRUTH v2
2026-08-23
```

A parte antiga continua útil como consolidação dos guias e materiais históricos.

As seções 57 em diante têm prioridade quando uma conclusão antiga conflitar com um teste direto feito no Summit Tale.
---

# 75. V5a — no-op round-trip Blender -> ASE PROVADO

Teste executado em 2026-08-23.

Template original:

```text
char/tmABCD/MfbC13.ASE
```

Tamanho:

```text
456373 bytes
```

SHA256 original:

```text
86a45454c98db4ef8239a1264413a7b34ec558f993c3d4148a1ccd0c03935887
```

O `.blend` usado foi o resultado da V4, sem qualquer edição de mesh.

O exporter V5a validou:

```text
MahC20
    V=80
    F=146
    TV=124
    TF=146
    PHYS=80
    CHANGED=0

MbhC19
    V=143
    F=248
    TV=212
    TF=248
    PHYS=143
    CHANGED=0

MlhC20
    V=286
    F=428
    TV=296
    TF=428
    PHYS=286
    CHANGED=0

MshC19
    V=163
    F=264
    TV=250
    TF=264
    PHYS=163
    CHANGED=0
```

Resultado:

```text
Total changed vertices: 0
Output bytes:           456373
Output SHA256:          86a45454c98db4ef8239a1264413a7b34ec558f993c3d4148a1ccd0c03935887
Byte-identical:         True
NO-OP ROUND-TRIP: BYTE-IDENTICAL
```

Validação externa:

```text
sha256sum original == sha256sum exportado
cmp -s original exportado == idêntico
```

Estado:

```text
Blender V4 -> V5a -> ASE sem edição: PROVADO BYTE A BYTE
```

Isto prova que a estratégia de usar o ASE funcional original como template preserva 100% dos bytes quando nenhuma coordenada de vértice foi alterada.

Também prova que, para este arquivo, as validações atuais do V5a aceitam corretamente:

```text
topologia original
UV original
PHYSIQUE rígido original
ordem original dos vértices
```

sem introduzir alterações colaterais.

---

# 76. Próximo teste — exatamente um MESH_VERTEX alterado

O próximo teste da V5a deve modificar apenas:

```text
MahC20
vertex 0
X += 1.0
```

em uma cópia do `.blend` V4.

O `.blend` original deve permanecer intacto.

Depois o V5a deve ser executado com:

```text
--max-changed 1
```

Resultado esperado:

```text
MahC20 CHANGED=1
MbhC19 CHANGED=0
MlhC20 CHANGED=0
MshC19 CHANGED=0

Total changed vertices: 1
Byte-identical: False
```

A comparação de linhas entre o ASE original e o ASE exportado deve apontar exatamente:

```text
1 linha diferente
```

e essa linha deve ser:

```text
*MESH_VERTEX 0 ...
```

dentro do `GEOMOBJECT` de `MahC20`.

Importante:

a V5a ainda NÃO recalcula `MESH_NORMALS`.

Portanto o ASE de um-vértice é um artefato de validação da pipeline:

```text
Blender -> coordenada editada -> ASE
```

e não deve ainda ser tratado como exporter final para remodelagem completa ou teste runtime definitivo.

Se esse teste passar, o próximo estágio será V5b:

```text
Blender mesh
-> vertices
-> faces
-> UV
-> normals
-> material IDs
-> PHYSIQUE
-> ASE consistente
```
---

# 77. V5a — teste de exatamente um vértice PROVADO

Teste executado em 2026-08-23.

Foi criada uma cópia do `.blend` V4:

```text
MfbC13_import_v4_onevertex.blend
```

Alteração aplicada:

```text
objeto: MahC20
vertex: 0
eixo:   X
delta:  +1.0
```

Coordenada antes no Blender:

```text
(-6.557600021362305, 1.332800030708313, 22.271799087524414)
```

Depois:

```text
(-5.557600021362305, 1.332800030708313, 22.271799087524414)
```

O V5a foi executado com:

```text
--max-changed 1
```

Resultado do exporter:

```text
MahC20: V=80  F=146 TV=124 TF=146 PHYS=80  CHANGED=1
MbhC19: V=143 F=248 TV=212 TF=248 PHYS=143 CHANGED=0
MlhC20: V=286 F=428 TV=296 TF=428 PHYS=286 CHANGED=0
MshC19: V=163 F=264 TV=250 TF=264 PHYS=163 CHANGED=0

Total changed vertices: 1
Byte-identical: False
```

Tamanho original:

```text
456373 bytes
```

Tamanho exportado:

```text
456375 bytes
```

A diferença de tamanho ocorre apenas pela representação textual da nova coordenada.

Validação linha a linha:

```text
linhas original:  8678
linhas exportado: 8678
linhas diferentes: 1
```

Única linha alterada:

```text
ORIGINAL:
*MESH_VERTEX 0  -6.5576  1.3328  22.2718

NOVO:
*MESH_VERTEX 0  -5.5576  1.3328  22.271799
```

Resultado:

```text
PROVADO: somente uma linha MESH_VERTEX foi alterada
```

Também:

```text
cmp -s original exportado
-> arquivos diferem, como esperado
```

Estado consolidado da V5a:

```text
no-op Blender -> ASE byte-identical:      PROVADO
1 vertex Blender -> 1 MESH_VERTEX no ASE: PROVADO
topologia preservada:                     PROVADO neste teste
UV preservada:                            PROVADO neste teste
PHYSIQUE preservado:                      PROVADO neste teste
alterações colaterais:                    nenhuma detectada
```

Isto prova a direção:

```text
Blender
-> coordenada de vértice editada
-> template ASE funcional
-> patch controlado
-> somente dado escolhido alterado
```

A V5a continua sendo um exporter de validação, pois ainda preserva os
`MESH_NORMALS` originais após uma edição geométrica.

---

# 78. Gate antes da V5b — provar convenção de normals

Antes de recalcular `MESH_NORMALS`, medir diretamente no ASE funcional se:

```text
MESH_FACENORMAL
<->
Blender polygon.normal
```

e:

```text
MESH_VERTEXNORMAL por corner
<->
Blender corner normal
```

também preservam:

```text
ordem de faces
ordem de corners
índice de vértice
orientação / handedness
smoothing
```

A V5b só deve ser implementada após esse probe.

Motivo:

```text
normals erradas
-> iluminação errada
-> costuras visuais
-> shading invertido
```

Mesmo que a geometria e a UV estejam corretas.
---

# 79. MfbC13.ASE — ausência de MESH_NORMALS PROVADA

Diagnóstico executado em 2026-08-23 no ASE funcional:

```text
char/tmABCD/MfbC13.ASE
```

Contagem no arquivo inteiro:

```text
MESH_NORMALS:      0
MESH_FACENORMAL:   0
MESH_VERTEXNORMAL: 0
```

Também foi isolado o `GEOMOBJECT`:

```text
MahC20
```

Resultado:

```text
MESH_NORMALS:      0
MESH_FACENORMAL:   0
MESH_VERTEXNORMAL: 0
```

As linhas de face contêm, porém:

```text
*MESH_SMOOTHING 1
```

junto com:

```text
*MESH_MTLID
```

Exemplo estrutural:

```text
*MESH_FACE 0:
    A: ...
    B: ...
    C: ...
    AB: ...
    BC: ...
    CA: ...
    *MESH_SMOOTHING 1
    *MESH_MTLID 0
```

Conclusão para este ASE:

```text
normals explícitas no arquivo: NÃO EXISTEM
smoothing por face:            EXISTE
```

Estado:

```text
ausência de MESH_NORMALS no MfbC13.ASE: PROVADO
```

Portanto a pipeline Summit não deve adicionar `MESH_NORMALS` ao exportar
edições desse template apenas porque ASE genérico pode suportar esse bloco.

A regra correta é:

```text
preservar a convenção do ASE funcional
```

Neste caso:

```text
vertices
faces + smoothing
UV
material IDs
Physique
```

sem bloco explícito de normals.

Isso invalida a premissa anterior de que a V5b precisaria obrigatoriamente
recalcular e escrever:

```text
MESH_FACENORMAL
MESH_VERTEXNORMAL
```

para este asset.

---

# 80. V5b — exporter de atributos com topologia preservada

A próxima etapa segura é ampliar a V5a sem ainda reconstruir topologia.

V5b deve aceitar alterações em:

```text
MESH_VERTEX
TVERT já existente
MESH_MTLID
PHYSIQUE rígido / No Blending
```

e preservar:

```text
quantidade de vertices
quantidade de faces
índices A/B/C das faces
AB/BC/CA
MESH_SMOOTHING
quantidade de TVERT
TFACE
estrutura do GEOMOBJECT
NODE_TM
materiais top-level
Biped
demais bytes do template
```

## 80.1 UV na V5b

Como Blender usa UV por loop/corner e o ASE deste arquivo usa:

```text
TVERT
+
TFACE
```

a V5b mantém os índices TVERT existentes.

Se vários corners apontam para o mesmo TVERT:

```text
todos precisam continuar com a mesma coordenada UV
```

Se o usuário separar esses corners no Blender e criar implicitamente uma
nova seam que exigiria dois TVERTs diferentes, a V5b deve parar.

Essa operação será suportada na fase de rebuild de UV/topologia.

## 80.2 Material ID

A V5b converte:

```text
Blender polygon.material_index
-> slot criado pelo importer
-> MESH_MTLID original associado ao slot
```

e modifica somente o número após:

```text
*MESH_MTLID
```

preservando smoothing e flags de borda da linha da face.

## 80.3 Physique

Para este workflow rígido:

```text
cada vertex
-> exatamente um vertex group Bip*
-> weight 1.0
```

A V5b pode converter novamente:

```text
Blender vertex group
-> bone name
-> PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
```

Se um vertex possuir:

```text
zero grupos Bip
mais de um grupo Bip
peso não rígido
```

o exporter deve parar em vez de produzir um ASE ambíguo.

## 80.4 Limite desta fase

A V5b ainda não suporta:

```text
adicionar/remover vertices
adicionar/remover faces
mudar triangulação
nova seam que exija novo TVERT
alterar quantidade de TVERT
```

Essas operações exigem reconstrução de listas/contadores e ficam para a
etapa seguinte.
---

# 81. V5b.1 — no-op round-trip PROVADO

Teste executado em 2026-08-23.

A V5b inicial encontrou uma característica válida do ASE:

```text
MlhC20 possui pelo menos um TVERT não referenciado por TFACE.
```

Exemplo observado:

```text
TVERT 56
```

A primeira implementação tratava isso como erro.

Correção V5b.1:

```text
TVERT sem referência por TFACE
-> não possui loop Blender correspondente
-> preservar bytes originais
-> não considerar erro
```

Após a correção, a V5b.1 foi executada sobre o `.blend` V4 original.

Resultado:

```text
MahC20:
VCHG=0
UVCHG=0
MTLCHG=0
PHYSCHG=0

MbhC19:
VCHG=0
UVCHG=0
MTLCHG=0
PHYSCHG=0

MlhC20:
VCHG=0
UVCHG=0
MTLCHG=0
PHYSCHG=0

MshC19:
VCHG=0
UVCHG=0
MTLCHG=0
PHYSCHG=0
```

Totais:

```text
Total vertex changes: 0
Total TVERT changes:  0
Total MTLID changes:  0
Total PHYS changes:   0
```

ASE original:

```text
456373 bytes
SHA256:
86a45454c98db4ef8239a1264413a7b34ec558f993c3d4148a1ccd0c03935887
```

ASE V5b.1 no-op:

```text
456373 bytes
SHA256:
86a45454c98db4ef8239a1264413a7b34ec558f993c3d4148a1ccd0c03935887
```

Validação externa:

```text
cmp -s
-> byte a byte idêntico
```

Estado:

```text
V5b.1 no-op: PROVADO
```

Também fica confirmado para este asset:

```text
ASE pode conter TVERT órfão/não referenciado
```

e esse dado deve ser preservado pelo exporter template-based.

---

# 82. Próximo teste V5b.1 — exatamente um TVERT

Objetivo:

```text
Blender UV
-> alterar coordenadas de todos os loops que apontam para um mesmo TVERT
-> exporter V5b.1
-> exatamente uma linha MESH_TVERT alterada
```

O helper deve escolher um TVERT realmente referenciado por `TFACE`.

Se o mesmo TVERT for utilizado por múltiplos corners:

```text
todos esses corners devem receber exatamente a mesma nova UV
```

Isso preserva a relação:

```text
1 ASE TVERT
<-
N corners Blender compartilhando a mesma coordenada
```

Teste esperado:

```text
VCHG=0
UVCHG=1
MTLCHG=0
PHYSCHG=0
```

e comparação textual:

```text
1 linha diferente
*MESH_TVERT ...
```

Esse teste provará:

```text
Blender UV
-> índice TVERT existente
-> ASE
```

sem reconstrução de TFACE ou criação de nova seam.
---

# 83. V5b.1 — teste de um TVERT PROVADO semanticamente

Teste executado em 2026-08-23.

Objeto:

```text
MahC20
```

TVERT escolhido:

```text
0
```

UV original:

```text
(0.3804, 0.0128, -0.0390)
```

O mesmo TVERT era referenciado por dois corners:

```text
face 0  / corner 1
face 10 / corner 1
```

Os dois loops Blender foram movidos juntos:

```text
U += 0.01
```

Resultado da V5b.1:

```text
MahC20:
VCHG=0
UVCHG=1
MTLCHG=0
PHYSCHG=0

demais meshes:
todas as mudanças = 0
```

Totais:

```text
Total vertex changes: 0
Total TVERT changes:  1
Total MTLID changes:  0
Total PHYS changes:   0
```

Comparação de arquivo:

```text
8678 linhas original
8678 linhas exportado
1 linha diferente
```

Linha alterada:

```text
ORIGINAL:
*MESH_TVERT 0  0.3804  0.0128  -0.0390

NOVO:
*MESH_TVERT 0  0.3904  0.0128  -0.039
```

Conclusão funcional:

```text
Blender UV
-> TVERT existente
-> exatamente uma linha MESH_TVERT
```

Estado:

```text
mapeamento UV -> TVERT: PROVADO
```

## 83.1 Detalhe de preservação textual encontrado

Embora apenas U tenha sido editado, a V5b.1 reformatou também o token W:

```text
-0.0390
->
-0.039
```

O valor numérico não mudou, mas os bytes mudaram.

O mesmo tipo de normalização textual havia ocorrido no teste V5a de um
vértice, onde um componente não editado foi reformatado por precisão
float32.

Portanto distinguir:

```text
alteração semântica
vs
alteração textual byte-minimal
```

A V5b.1 está provada para o primeiro caso, mas será refinada para o segundo.

---

# 84. V5b.2 — patch por componente

A V5b.2 altera a estratégia de escrita de:

```text
reescrever X/Y/Z inteiro de um MESH_VERTEX alterado
reescrever U/V/W inteiro de um MESH_TVERT alterado
```

para:

```text
comparar cada componente individualmente com epsilon
```

e:

```text
componente realmente alterado
-> formatar novo valor

componente não alterado
-> reutilizar literalmente o token original do ASE
```

Exemplo esperado:

```text
ORIGINAL:
*MESH_TVERT 0  0.3804  0.0128  -0.0390

EDIT:
U += 0.01

V5b.2:
*MESH_TVERT 0  0.3904  0.0128  -0.0390
```

Assim:

```text
V e W permanecem byte-identical
```

O mesmo princípio vale para:

```text
MESH_VERTEX X/Y/Z
```

Objetivo:

```text
alterar somente os bytes correspondentes ao dado efetivamente editado
```

sempre que a estrutura da linha permitir.
---

# 85. V5b.2 — UV byte-minimal PROVADO

Teste executado em 2026-08-23.

Foi reutilizado:

```text
MfbC13_import_v4_one_tvert.blend
```

Objeto:

```text
MahC20
```

TVERT:

```text
0
```

O TVERT é compartilhado por dois corners Blender:

```text
face 0  / corner 1
face 10 / corner 1
```

Os dois loops foram movidos juntos:

```text
U += 0.01
```

Resultado da V5b.2:

```text
MahC20:
VCHG=0
UVCHG=1
MTLCHG=0
PHYSCHG=0

MbhC19:
VCHG=0
UVCHG=0
MTLCHG=0
PHYSCHG=0

MlhC20:
VCHG=0
UVCHG=0
MTLCHG=0
PHYSCHG=0

MshC19:
VCHG=0
UVCHG=0
MTLCHG=0
PHYSCHG=0
```

Totais:

```text
Total vertex changes: 0
Total TVERT changes:  1
Total MTLID changes:  0
Total PHYS changes:   0
```

O arquivo manteve:

```text
456373 bytes
```

A comparação encontrou:

```text
linhas diferentes: 1
```

Única linha alterada:

```text
ORIGINAL:
*MESH_TVERT 0  0.3804  0.0128  -0.0390

NOVO:
*MESH_TVERT 0  0.3904  0.0128  -0.0390
```

Portanto:

```text
U: alterado
V: byte-identical
W: byte-identical
```

Estado:

```text
Blender UV -> MESH_TVERT existente: PROVADO
patch por componente:                PROVADO
preservação textual de V/W:          PROVADO
1 TVERT lógico -> 1 linha ASE:        PROVADO
```

---

# 86. Próximo teste V5b.2 — exatamente um MESH_MTLID

O próximo teste isolado usa:

```text
MbhC19
```

porque essa mesh usa:

```text
MTLID 0
MTLID 1
```

e possui slots Blender correspondentes aos dois submateriais.

O helper deve:

```text
1. localizar os slots ASE_M..._S0 e ASE_M..._S1;
2. escolher uma única face;
3. trocar polygon.material_index para o outro submaterial;
4. salvar em novo .blend;
5. não modificar geometria, UV ou Physique.
```

Resultado esperado da V5b.2:

```text
VCHG=0
UVCHG=0
MTLCHG=1
PHYSCHG=0
```

Comparação textual esperada:

```text
1 linha diferente
```

e dentro dessa linha somente:

```text
*MESH_MTLID 0
->
*MESH_MTLID 1
```

ou o inverso.

Todo o restante da linha de face deve permanecer literalmente igual:

```text
A/B/C
AB/BC/CA
MESH_SMOOTHING
tabs/espaçamento
```

Se passar, o mapping:

```text
Blender polygon.material_index
-> material slot ASE
-> MESH_MTLID
```

estará PROVADO.
---

# 87. V5b.2 — MESH_MTLID byte-minimal PROVADO

Teste executado em 2026-08-23.

Objeto:

```text
MbhC19
```

Face escolhida:

```text
0
```

Estado original:

```text
Blender slot: 0
material:
ASE_M0_S0_Material_434_MfbC19_01.TGA.001

MATERIAL_REF: 0
MESH_MTLID:  0
```

Novo estado:

```text
Blender slot: 1
material:
ASE_M0_S1_Material_435_MfbC19_02.BMP

MATERIAL_REF: 0
MESH_MTLID:  1
```

O MATERIAL_REF permaneceu:

```text
0
```

portanto a operação foi exclusivamente uma troca entre submateriais
válidos do mesmo material top-level.

Resultado V5b.2:

```text
MahC20:
VCHG=0 UVCHG=0 MTLCHG=0 PHYSCHG=0

MbhC19:
VCHG=0 UVCHG=0 MTLCHG=1 PHYSCHG=0

MlhC20:
VCHG=0 UVCHG=0 MTLCHG=0 PHYSCHG=0

MshC19:
VCHG=0 UVCHG=0 MTLCHG=0 PHYSCHG=0
```

Totais:

```text
Total vertex changes: 0
Total TVERT changes:  0
Total MTLID changes:  1
Total PHYS changes:   0
```

Comparação textual:

```text
linhas diferentes: 1
```

Única linha alterada:

```text
ORIGINAL:
*MESH_FACE 0:
A: 4 B: 3 C: 2
AB: 1 BC: 1 CA: 1
*MESH_SMOOTHING 1
*MESH_MTLID 0

NOVO:
*MESH_FACE 0:
A: 4 B: 3 C: 2
AB: 1 BC: 1 CA: 1
*MESH_SMOOTHING 1
*MESH_MTLID 1
```

Portanto permaneceram literalmente preservados:

```text
face index
A/B/C
AB/BC/CA
MESH_SMOOTHING
espaçamento restante da linha
```

Estado:

```text
Blender polygon.material_index -> MESH_MTLID: PROVADO
patch de uma única face:                     PROVADO
preservação do restante da linha:            PROVADO
```

---

# 88. Estado V5b.2 antes do teste PHYSIQUE

Já estão provados individualmente:

```text
Blender vertex coordinate
-> MESH_VERTEX

Blender UV
-> MESH_TVERT

Blender polygon material
-> MESH_MTLID
```

Falta provar isoladamente:

```text
Blender rigid vertex group
-> PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
```

Depois disso, a V5b.2 terá fechado todos os atributos que ela se propõe
a editar mantendo a topologia existente.

---

# 89. Próximo teste — exatamente uma associação PHYSIQUE

O teste deve:

```text
1. usar o .blend V4 original;
2. escolher uma mesh com mais de um grupo Bip*;
3. escolher um único vértice;
4. descobrir seu bone atual;
5. remover esse vértice do grupo atual;
6. adicioná-lo a outro grupo Bip* existente com weight 1.0;
7. salvar em outro .blend;
8. executar V5b.2 com --max-physique-changed 1.
```

Nenhum outro atributo deve mudar.

Resultado esperado:

```text
VCHG=0
UVCHG=0
MTLCHG=0
PHYSCHG=1
```

Comparação textual esperada:

```text
1 linha diferente
```

com somente:

```text
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
<vertex> "Bone Antigo"

->
<vertex> "Bone Novo"
```

O índice do vértice deve permanecer idêntico.

Se passar, estará provado:

```text
Blender vertex group Bip*
-> bone name PHYSIQUE
-> ASE
```
---

# 90. V5b.2 — PHYSIQUE byte-minimal PROVADO

Teste executado em 2026-08-23.

Objeto:

```text
MbhC19
```

Vértice:

```text
0
```

Associação original no Blender:

```text
Bip01 Spine1
weight 1.0
```

Associação nova:

```text
Bip-0007
weight 1.0
```

Após a alteração, o vértice possuía exatamente:

```text
[('Bip-0007', 1.0)]
```

Resultado V5b.2:

```text
MahC20:
VCHG=0 UVCHG=0 MTLCHG=0 PHYSCHG=0

MbhC19:
VCHG=0 UVCHG=0 MTLCHG=0 PHYSCHG=1

MlhC20:
VCHG=0 UVCHG=0 MTLCHG=0 PHYSCHG=0

MshC19:
VCHG=0 UVCHG=0 MTLCHG=0 PHYSCHG=0
```

Totais:

```text
Total vertex changes: 0
Total TVERT changes:  0
Total MTLID changes:  0
Total PHYS changes:   1
```

Comparação textual:

```text
linhas diferentes: 1
```

Única linha alterada:

```text
ORIGINAL:
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
0 "Bip01 Spine1"

NOVO:
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
0 "Bip-0007"
```

O índice do vértice permaneceu:

```text
0
```

Estado:

```text
Blender rigid vertex group -> PHYSIQUE bone name: PROVADO
patch de uma única associação:                       PROVADO
preservação do vertex index:                         PROVADO
```

---

# 91. V5b.2 — estado final

A V5b.2 fecha o round-trip com topologia preservada.

Todos os caminhos abaixo estão PROVADOS:

```text
Blender vertex coordinate
-> MESH_VERTEX

Blender UV
-> MESH_TVERT

Blender polygon material
-> MESH_MTLID

Blender rigid Bip vertex group
-> PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
```

Também foram provados:

```text
no-op byte-identical
TVERT órfão preservado
ausência de MESH_NORMALS no MfbC13.ASE
MESH_SMOOTHING preservado
patch por componente para valores float
```

Portanto:

```text
V5b.2 = EXPORTER DE ATRIBUTOS COM TOPOLOGIA EXISTENTE
PROVADO
```

---

# 92. Gate para V5c — dados que passam a precisar ser criados

A V5c permitirá mudança de topologia:

```text
adicionar/remover vértices
adicionar/remover faces
nova triangulação
novas seams UV
novo número de TVERT
novo TFACE
novo PHYSIQUE para vértices novos
```

Isso exige reconstruir informações que até V5b.2 eram preservadas do
template.

Dois campos precisam ser medidos antes de definir regras de geração:

## 92.1 TVERT W

ASE usa:

```text
*MESH_TVERT index U V W
```

A importação Blender atual usa U/V para o UV Editor.

O terceiro componente W não é editado no Blender.

Exemplo já observado:

```text
*MESH_TVERT 0  0.3804  0.0128  -0.0390
```

Portanto W não deve ser automaticamente definido como zero sem auditoria.

A V5c precisa descobrir por mesh:

```text
mínimo W
máximo W
quantos valores W distintos
se W é constante
se W depende de U/V
```

## 92.2 Face edge flags AB/BC/CA

ASE usa em cada face:

```text
A:
B:
C:
AB:
BC:
CA:
*MESH_SMOOTHING
*MESH_MTLID
```

V5b.2 preservava esses flags literalmente.

Ao criar faces novas, a V5c precisa saber se:

```text
AB/BC/CA são usados pelo loader/runtime
```

ou se são apenas metadata do ASE/3ds Max.

Antes de inventar:

```text
AB=1 BC=1 CA=1
```

é necessário auditar:

```text
distribuição no asset funcional
+
parser do source
```

---

# 93. Estratégia V5c

Somente após o gate acima:

```text
ASE funcional como template
-> manter MATERIAL_LIST / NODE_TM / Biped / animation
-> reconstruir apenas MESH do objeto alterado
-> reconstruir PHYSIQUE desse objeto
```

Primeiro teste V5c recomendado:

```text
uma única face subdividida
```

ou:

```text
um único vértice novo + triangulação controlada
```

e depois:

```text
exportar
-> reimportar com V4
-> comparar geometria/UV/material/Physique
```

O runtime vem apenas depois desse round-trip estrutural.
---

# 94. V5c gate — W de TVERT varia de verdade

Auditoria executada no `MfbC13.ASE`.

Resultados:

```text
MshC19
TVERT: 250
W min: -2.1609
W max:  2.21
W distintos (6 casas): 118

MbhC19
TVERT: 212
W min: -1.2273
W max:  3.74
W distintos (6 casas): 92

MlhC20
TVERT: 296
W min: -0.0055
W max:  0.9945
W distintos (6 casas): 113

MahC20
TVERT: 124
W min: -1.638
W max:  1.719
W distintos (6 casas): 61
```

Conclusão:

```text
W NÃO é constante.
```

Portanto a V5c não pode assumir:

```text
W = 0
```

ao criar novos TVERTs sem antes estabelecer a função/semântica correta.

Estado:

```text
TVERT W variável no asset funcional: PROVADO
significado exato de W:               PENDENTE
```

---

# 95. V5c gate — AB/BC/CA não são constantes

O primeiro probe encontrou múltiplos padrões nas faces.

Exemplos:

```text
MshC19
(1,1,1)
(0,1,1)
(1,1,0)
(1,0,1)

MbhC19
(1,1,1)
(1,1,0)
(0,1,1)
(1,0,1)

MlhC20
(1,1,0)
(1,1,1)

MahC20
(1,1,0)
(1,1,1)
```

Conclusão:

```text
AB/BC/CA NÃO são valores fixos universais.
```

Portanto a V5c não deve preencher toda nova face com:

```text
AB=1 BC=1 CA=1
```

sem entender a regra.

Estado:

```text
flags mistos no ASE funcional: PROVADO
semântica/runtime desses flags: PENDENTE
```

---

# 96. Falha detectada no primeiro face gate

O primeiro probe exigia que uma linha de face tivesse simultaneamente:

```text
A/B/C
AB/BC/CA
MESH_SMOOTHING
MESH_MTLID
```

para entrar na contagem.

Isso produziu:

```text
MbhC19: 246 faces
MlhC20: 420 faces
```

mas a pipeline V1-V5 já havia provado:

```text
MbhC19: 248 faces
MlhC20: 428 faces
```

Portanto:

```text
10 linhas MESH_FACE válidas foram ignoradas pela regex do probe.
```

Isso NÃO invalida o ASE.

Invalida apenas a contagem/distribuição incompleta do primeiro auditor.

Antes de definir regras de geração para V5c, essas linhas precisam ser
identificadas e classificadas.

Novo probe:

```text
pt_v5c_face_gate_probe_v2.py
```

Ele:

```text
conta toda linha MESH_FACE primeiro;
parseia A/B/C e AB/BC/CA separadamente;
trata smoothing como opcional;
trata MTLID como opcional;
mostra índices ausentes/duplicados;
imprime linhas incomuns.
```

Estado:

```text
primeiro face gate: PARCIAL
face gate v2:       PENDENTE TESTE
```
---

# 97. V5c face gate v2 — PROVADO

Auditoria corrigida executada em 2026-08-23.

O probe v2 passou a contar toda linha:

```text
*MESH_FACE
```

antes de exigir `MESH_SMOOTHING` ou `MESH_MTLID`.

Resultado:

```text
MshC19
MESH_FACE physical lines: 264
parsed:                   264
indices:                  0..263
missing:                  0
duplicates:               0
malformed:                0

MbhC19
MESH_FACE physical lines: 248
parsed:                   248
indices:                  0..247
missing:                  0
duplicates:               0
malformed:                0

MlhC20
MESH_FACE physical lines: 428
parsed:                   428
indices:                  0..427
missing:                  0
duplicates:               0
malformed:                0

MahC20
MESH_FACE physical lines: 146
parsed:                   146
indices:                  0..145
missing:                  0
duplicates:               0
malformed:                0
```

Essas contagens agora batem exatamente com V1-V5.

Estado:

```text
face gate v2: PROVADO
```

---

# 98. Causa da contagem incompleta do primeiro probe

A diferença anterior era causada por faces com:

```text
*MESH_SMOOTHING
```

presente, porém sem número/grupo depois da palavra.

Em `MbhC19`:

```text
face 34
face 158
```

Total:

```text
2 faces com smoothing vazio
```

Em `MlhC20`:

```text
face 157
face 158
face 159
face 160
face 371
face 372
face 373
face 374
```

Total:

```text
8 faces com smoothing vazio
```

Somando:

```text
10 faces
```

que eram exatamente as 10 linhas descartadas pela regex antiga.

Importante:

todas essas faces continuam possuindo:

```text
A/B/C
AB/BC/CA
MESH_MTLID
```

e não apresentam índice ausente ou duplicado.

Estado:

```text
smoothing vazio presente em faces funcionais do template: PROVADO
```

Portanto a V5c não deve obrigar toda nova face a ter necessariamente:

```text
*MESH_SMOOTHING 1
```

sem antes entender como o loader interpreta smoothing vazio.

---

# 99. Distribuições corrigidas de AB/BC/CA

Com todas as faces incluídas:

```text
MshC19
(1,1,1) -> 236
(0,1,1) -> 10
(1,1,0) -> 10
(1,0,1) -> 8

MbhC19
(1,1,1) -> 216
(1,1,0) -> 11
(0,1,1) -> 11
(1,0,1) -> 10

MlhC20
(1,1,0) -> 332
(1,1,1) -> 96

MahC20
(1,1,0) -> 128
(1,1,1) -> 18
```

Estado:

```text
AB/BC/CA mistos: PROVADO
```

Ainda não está provado:

```text
se esses flags são usados pelo loader/runtime
```

---

# 100. Distribuições corrigidas de smoothing e material

```text
MshC19
smoothing 1: 264
MTLID 0:    264

MbhC19
smoothing 1:   246
smoothing vazio: 2
MTLID 0:       152
MTLID 1:        96

MlhC20
smoothing 1:   420
smoothing vazio: 8
MTLID 0:       216
MTLID 1:       212

MahC20
smoothing 1: 146
MTLID 0:      146
```

Isso confirma que o `MESH_MTLID` não era a causa do mismatch anterior.

---

# 101. Próximo gate — parser real smRead3d.cpp

O source scan encontrou o parser ASE em:

```text
smLib3d/smRead3d.cpp
```

regiões relevantes:

```text
*MESH_FACE
*MESH_MTLID
*MESH_TVERT
*MESH_TFACE
```

O scan preliminar indica que, em `MESH_TVERT`, o código parece armazenar:

```text
u
v
```

e incrementar o contador sem armazenar explicitamente o terceiro valor W.

Isso é apenas:

```text
INDÍCIO FORTE
```

até a leitura do bloco completo do parser.

Também é necessário verificar no bloco de `MESH_FACE` se:

```text
AB
BC
CA
MESH_SMOOTHING
```

são lidos, ignorados ou apenas saltados pelo tokenizer.

Somente depois dessa inspeção será definida a regra de geração V5c.
---

# 102. Parser Summit — AB/BC/CA e smoothing ignorados: PROVADO

Inspeção executada em 2026-08-23 em:

```text
SrcGame/src/smLib3d/smRead3d.cpp
SrcServer/src/smLib3d/smRead3d.cpp
```

No bloco:

```text
*MESH_FACE
```

o loader:

```text
1. consome o índice da face;
2. lê A;
3. lê B;
4. lê C;
5. chama obj->AddFace(a,b,c);
6. percorre até 16 tokens procurando *MESH_MTLID;
7. lê somente o valor do MESH_MTLID.
```

O código não possui comparação direta com:

```text
AB:
BC:
CA:
*MESH_SMOOTHING
```

na implementação inspecionada.

Portanto esses tokens são apenas atravessados pelo loop que procura:

```text
*MESH_MTLID
```

Estado para o loader Summit inspecionado:

```text
A/B/C:          USADOS
MESH_MTLID:     USADO
AB/BC/CA:       IGNORADOS
MESH_SMOOTHING: IGNORADO
```

Isso também explica por que faces com:

```text
*MESH_SMOOTHING
```

vazio continuam parseáveis: o loader não tenta converter o valor de
smoothing.

Conclusão:

```text
AB/BC/CA ignorados pelo loader Summit:       PROVADO
MESH_SMOOTHING ignorado pelo loader Summit:  PROVADO
```

---

# 103. Parser Summit — terceiro componente W de TVERT ignorado: PROVADO

No bloco:

```text
*MESH_TVERT
```

o código inspecionado faz:

```text
ler índice;
ler U -> lpTexVertex[].u;
ler V -> lpTexVertex[].v;
GetWord(...) para o terceiro token;
não armazena esse terceiro token;
incrementa TVertexCnt.
```

Posteriormente, na criação dos texture links, somente:

```text
tPoint[...].u
tPoint[...].v
```

são utilizados.

Portanto:

```text
MESH_TVERT U: USADO
MESH_TVERT V: USADO
MESH_TVERT W: CONSUMIDO E DESCARTADO
```

Estado:

```text
W ignorado pelo loader Summit inspecionado: PROVADO
```

Isto explica por que o W do ASE original pode variar sem fazer parte do
mapeamento UV runtime dessa implementação.

---

# 104. Regras canônicas para novos dados V5c

Com base no parser real da build atual, a V5c pode usar para dados novos:

```text
*MESH_TVERT index U V 0.0
```

O `0.0` é um valor canônico de serialização.

Ele não participa do resultado runtime do loader inspecionado.

Para faces novas:

```text
AB: 1
BC: 1
CA: 1
*MESH_SMOOTHING 1
```

também podem ser usados como forma canônica.

Esses campos são mantidos porque fazem parte da sintaxe ASE legível e
compatível com ferramentas, mas não são usados para criar a face no loader
Summit inspecionado.

O dado runtime relevante continua sendo:

```text
A
B
C
MESH_MTLID
```

---

# 105. V5c.1 — gate de reconstrução completa sem topologia nova

Antes de inserir um vértice ou face nova:

```text
não mudar a topologia;
reconstruir as listas MESH de uma única peça;
usar serialização canônica;
validar que os dados efetivamente usados pelo loader são os mesmos.
```

Alvo inicial:

```text
MahC20
```

Motivo:

```text
80 vertices
146 faces
124 TVERT
146 TFACE
1 MTLID
```

e portanto é a peça mais simples para esse gate.

A V5c.1 deve reconstruir:

```text
MESH_NUMVERTEX
MESH_VERTEX_LIST
MESH_NUMFACES
MESH_FACE_LIST
MESH_NUMTVERTEX
MESH_TVERTLIST
MESH_NUMTVFACES
MESH_TFACELIST
```

mas preservar:

```text
NODE_TM
MATERIAL_REF
PHYSIQUE
Biped
animação
todo o restante do GEOMOBJECT/template
```

A topologia V5c real só será habilitada após esse rebuild sem alteração
passar.
---

# 106. V5c.1 — rebuild interno passou, validação externa ainda pendente

Teste executado em 2026-08-23 sobre:

```text
MahC20
```

Fonte Blender:

```text
MfbC13_import_v4.blend
```

Template:

```text
MfbC13.ASE
```

Saída:

```text
MfbC13_roundtrip_v5c_1_MahC20.ASE
```

Resultado do exporter:

```text
Vertices: 80
Faces:    146
TVERT:    124
TFACE:    146
Orphan TVERTs: 0
```

A V5c.1 preservou do template:

```text
PHYSIQUE
MATERIAL_REF
NODE_TM
```

e reconstruiu o MESH com a forma canônica definida pelo source audit:

```text
AB/BC/CA       = 1/1/1
MESH_SMOOTHING = 1
W referenciado = 0.0
```

Self-check do arquivo serializado:

```text
serialized vertex count : OK
serialized face topology: OK
serialized MTLID        : OK
serialized TVERT count  : OK
serialized TFACE mapping: OK
```

Tamanhos:

```text
template: 456373 bytes
output:   452853 bytes
```

Hashes:

```text
template:
86a45454c98db4ef8239a1264413a7b34ec558f993c3d4148a1ccd0c03935887

output:
cca7817ba202203def589979349a4caccf74da3588890097b43663cf412aa785
```

A diferença de tamanho é esperável porque as listas foram reserializadas
em formato canônico e não são mais um patch byte-minimal.

Estado correto:

```text
V5c.1 serialização + self-check interno: PASSOU
V5c.1 round-trip independente via importer V4: PENDENTE
V5c.1 runtime cliente: PENDENTE
```

Não marcar ainda como:

```text
V5c.1 PROVADO
```

até o importer V4 reabrir a saída.

---

# 107. Próximo teste — reimport independente com V4

O próximo gate usa:

```text
pt_ase_import_v4.py
```

sobre:

```text
MfbC13_roundtrip_v5c_1_MahC20.ASE
```

O importer V4 é independente da lógica de rebuild V5c.1 e já foi validado
em etapas anteriores.

Critérios mínimos:

```text
MahC20:
V=80
F=146
TV=124
TF=146
UVfaces=146

PHYSIQUE:
declared=80
parsed=80
applied=80
unassigned=0
invalidVertex=0
missingBone=0
```

Os demais três meshes devem continuar com suas contagens originais porque
a V5c.1 substituiu somente o MESH de `MahC20`.

Se esse reimport passar:

```text
ASE template
-> Blender V4
-> V5c.1 rebuild completo de MahC20
-> ASE
-> Blender V4 novamente
```

estará fechado estruturalmente.

Só depois será criada topologia nova.
---

# 108. V5c.1 — round-trip independente via importer V4 PROVADO

Teste executado em 2026-08-23.

ASE reconstruído:

```text
MfbC13_roundtrip_v5c_1_MahC20.ASE
```

foi aberto pelo importer independente:

```text
pt_ase_import_v4.py
```

Resultado do skeleton:

```text
Bip nodes found:   63
Bones created:     63
Parent links:      62
Missing positions: 0
```

Resultado PHYSIQUE:

```text
MshC19:
declared=163 parsed=163 applied=163
unassigned=0 missingBone=0 invalidVertex=0 duplicates=0

MbhC19:
declared=143 parsed=143 applied=143
unassigned=0 missingBone=0 invalidVertex=0 duplicates=0

MlhC20:
declared=286 parsed=286 applied=286
unassigned=0 missingBone=0 invalidVertex=0 duplicates=0

MahC20:
declared=80 parsed=80 applied=80
unassigned=0 missingBone=0 invalidVertex=0 duplicates=0
```

Totais:

```text
Physique meshes:       4
Assignments parsed:  672
Vertices applied:    672
Unassigned vertices:   0
Missing-bone assigns:  0
Invalid-vertex refs:   0
Unsupported lines:     0
```

Resultado das meshes:

```text
MshC19
V=163 F=264 TV=250 TF=264 UVfaces=264 MATREF=0

MbhC19
V=143 F=248 TV=212 TF=248 UVfaces=248 MATREF=0

MlhC20
V=286 F=428 TV=296 TF=428 UVfaces=428 MATREF=1

MahC20
V=80 F=146 TV=124 TF=146 UVfaces=146 MATREF=1
```

Todas as seis resoluções/carregamentos de textura usados pelo importer:

```text
LOADED:  6
MISSING: 0
FAILED:  0
```

O `.blend` de reimport também foi salvo com sucesso.

Pipeline provada:

```text
ASE funcional
-> importer V4
-> Blender
-> V5c.1 full-MESH rebuild
-> ASE reconstruído
-> importer V4 independente
-> Blender novamente
```

Estado:

```text
V5c.1 rebuild estrutural sem topologia nova: PROVADO
runtime cliente desse ASE:                   PENDENTE
```

---

# 109. V5c.2 — primeiro teste real de topologia nova

A próxima alteração controlada será:

```text
1 triângulo
->
3 triângulos
```

com:

```text
+1 vértice geométrico
+2 faces líquidas
+1 TVERT central
+2 TFACE líquidos
+1 associação PHYSIQUE
```

Alvo:

```text
MahC20
```

Estado esperado:

```text
antes:
V=80
F=146
TV=124
TF=146
PHYS=80

depois:
V=81
F=148
TV=125
TF=148
PHYS=81
```

## 109.1 Seleção segura da face

O helper deve escolher apenas uma face cujos três vértices:

```text
tenham exatamente um grupo Bip* ativo;
tenham weight 1.0;
pertençam ao mesmo bone.
```

Assim o vértice central possui uma associação rígida inequívoca:

```text
novo vértice -> mesmo bone
```

sem inventar blend weights.

## 109.2 Geometria

Para face original:

```text
(A,B,C)
```

novo vértice:

```text
D = centroid(A,B,C)
```

novas faces:

```text
(A,B,D)
(B,C,D)
(C,A,D)
```

A face original é substituída pelas três.

## 109.3 UV

Para TFACE original:

```text
(tA,tB,tC)
```

novo TVERT central:

```text
tD.u = média(Ua,Ub,Uc)
tD.v = média(Va,Vb,Vc)
tD.w = 0.0
```

novos TFACE:

```text
(tA,tB,tD)
(tB,tC,tD)
(tC,tA,tD)
```

O mesmo `tD` é compartilhado pelos três corners centrais.

## 109.4 Material

As três faces novas herdam:

```text
MESH_MTLID
```

da face original.

## 109.5 PHYSIQUE

Os vértices antigos devem manter exatamente as associações originais.

Somente o novo índice:

```text
80
```

receberá uma nova linha rígida:

```text
*PHYSIQUE_VERTEXASSIGNMENT_NONBLENDED_RIGIDTYPE
80 "<bone comum>"
```

## 109.6 Critério de prova

O primeiro gate da V5c.2 será novamente independente:

```text
Blender V4 original
-> helper split 1 face
-> V5c.2 export
-> ASE com topologia nova
-> importer V4
```

Esperado no reimport:

```text
MahC20:
V=81
F=148
TV=125
TF=148
UVfaces=148

PHYSIQUE:
declared=81
parsed=81
applied=81
unassigned=0
missingBone=0
invalidVertex=0
```

Somente após esse round-trip estrutural será considerado teste runtime.
---

# 110. V5c.2 helper v1 — falha de vertex-group após mesh swap

Primeira execução do helper de split controlado:

```text
pt_v5c_make_one_face_split_test.py
```

falhou depois da criação/substituição do novo datablock `Mesh`.

Erro:

```text
RuntimeError:
Missing vertex group after mesh swap: 'Bip01 R Forearm'
```

A falha ocorreu na etapa de reaplicar os pesos antigos após:

```text
obj.data = new_mesh
```

Portanto o problema observado foi:

```text
helper Blender / reconstrução da tabela de vertex groups
```

e NÃO:

```text
ASE parser
V5c serializer
regra de TVERT
regra de face
PHYSIQUE do formato
```

O ASE ainda nem havia sido exportado nessa etapa.

Estado:

```text
V5c.2 topologia Blender: ainda não testada até o fim
helper v1: corrigido para v2
```

---

# 111. V5c.2 helper v2 — recriação determinística de vertex groups

A correção captura antes do mesh swap:

```text
lista completa dos vertex groups
ordem dos grupos
todos os memberships/weights antigos por vertex
```

Depois de:

```text
obj.data = new_mesh
```

o helper v2:

```text
1. remove qualquer tabela parcial/restante de vertex groups;
2. recria todos os grupos na ordem original;
3. valida que os nomes/ordem são idênticos;
4. reaplica todos os memberships/weights antigos;
5. adiciona o novo vértice somente ao bone comum da face;
6. valida rigid_bone(new_vertex) == common_bone.
```

Essa estratégia preserva:

```text
grupos Bip*
outros grupos eventualmente existentes
nomes
ordem
pesos antigos
```

e evita depender do comportamento da build Blender ao trocar o datablock
de mesh de um objeto.

Próximo teste:

```text
repetir somente a criação do .blend de split
```

Critérios:

```text
Old vertices: 80
New vertices: 81
Old faces:    146
New faces:    148
Vertex groups: <N> restored
New vertex bone == Common rigid bone
```

Somente depois desse helper passar será criada a V5c.2 ASE exporter.
---

# 112. V5c.2 helper v2 — topologia nova no Blender PROVADA

Teste executado em 2026-08-23.

Alvo:

```text
MahC20
```

Operação:

```text
SPLIT_ONE_FACE_1_TO_3
```

Face selecionada:

```text
0
```

Triângulo original:

```text
(1, 0, 3)
```

Os três vértices originais pertenciam rigidamente ao mesmo bone:

```text
Bip01 R Forearm
```

Novo estado:

```text
Old vertices: 80
New vertices: 81
New vertex index: 80

Old faces: 146
New faces: 148
```

Coordenada do novo vértice:

```text
(-5.91700013478597,
  1.2815333207448323,
 23.50393295288086)
```

Faces substitutas:

```text
(1, 0, 80)
(0, 3, 80)
(3, 1, 80)
```

UV central:

```text
(0.36763333280881244,
 0.05793333395073811)
```

A tabela de deform groups foi restaurada:

```text
18 vertex groups restored
```

e o novo vértice recebeu:

```text
Bip01 R Forearm
weight 1.0
```

Estado:

```text
primeira mudança controlada de topologia dentro do Blender: PROVADO
```

Ainda pendente:

```text
export ASE dessa topologia;
reimport independente;
runtime.
```

---

# 113. V5c.2 exporter — escopo controlado

Foi criada a primeira versão do exporter de topologia:

```text
pt_blender_to_ase_v5c_2.py
```

Ela NÃO é um exporter genérico ainda.

Aceita exclusivamente a operação registrada pelo helper:

```text
SPLIT_ONE_FACE_1_TO_3
```

e valida antes de escrever:

```text
+1 vertex apenas
+2 faces líquidas apenas
novo vertex == índice V antigo
target face == metadata do helper
old vertices sem mudança
topologia das faces antigas preservada
três faces novas exatamente no padrão esperado
material das faces novas herdado da face original
UVs antigos preservados
novo UV == centro da UV da face original
old Physique == template
novo vertex rigidamente no bone comum
```

---

# 114. V5c.2 — alterações ASE esperadas

Para `MahC20`:

```text
MESH_NUMVERTEX
80 -> 81

MESH_NUMFACES
146 -> 148

MESH_NUMTVERTEX
124 -> 125

MESH_NUMTVFACES
146 -> 148

PHYSIQUE_NUMVERTEXASSIGNMENT
80 -> 81
```

Novo vertex:

```text
80
```

Novo TVERT:

```text
124
```

com:

```text
U = centro da face
V = centro da face
W = 0.0
```

Novos TFACE da área dividida:

```text
(tA,tB,124)
(tB,tC,124)
(tC,tA,124)
```

Novo Physique:

```text
vertex 80
-> "Bip01 R Forearm"
```

---

# 115. Safety stop para blocos ainda não tratados

Antes de exportar topologia nova, a V5c.2 recusa o target MESH caso encontre:

```text
MESH_NUMCVERTEX
MESH_CVERTLIST
MESH_NUMCVFACES
MESH_CFACELIST
MESH_NORMALS
MESH_FACENORMAL
MESH_VERTEXNORMAL
```

A razão é evitar manter dados per-vertex/per-face obsoletos depois de uma
mudança de topologia.

Para `MfbC13.ASE`, a ausência global de normals explícitas já foi provada.

Os blocos de color vertex/face ainda serão aceitos somente depois de serem
medidos, se algum asset futuro os utilizar.

---

# 116. Próximo gate V5c.2

Executar:

```text
MfbC13_import_v4_v5c2_one_face_split_v2.blend
->
pt_blender_to_ase_v5c_2.py
->
ASE
```

Critérios do self-check:

```text
V:    80 -> 81
F:   146 -> 148
TV:  124 -> 125
TF:  146 -> 148
PHYS: 80 -> 81

new vertex coordinate: OK
split face topology:   OK
split TFACE topology:  OK
old Physique:          OK
new Physique:          OK
```

Somente depois:

```text
reimport independente com V4
```
---

# 117. V5c.2 — export de topologia nova passou o self-check

Teste executado em 2026-08-23.

Entrada Blender:

```text
MfbC13_import_v4_v5c2_one_face_split_v2.blend
```

Template:

```text
MfbC13.ASE
```

Exporter:

```text
pt_blender_to_ase_v5c_2.py
```

Saída:

```text
MfbC13_roundtrip_v5c_2_one_face_split.ASE
```

Operação:

```text
SPLIT_ONE_FACE_1_TO_3
```

Face original:

```text
0
```

Triângulo original:

```text
(1, 0, 3)
```

Novo vértice:

```text
80
```

Coordenada serializada:

```text
(-5.917000000000001,
  1.2815333333333332,
 23.503933333333332)
```

Bone rígido:

```text
Bip01 R Forearm
```

Novo TVERT:

```text
124
```

UV central:

```text
(0.3676333333333333,
 0.05793333333333334)
```

## 117.1 Contagens

```text
Vertices : 80  -> 81
Faces    : 146 -> 148
TVERT    : 124 -> 125
TFACE    : 146 -> 148
PHYSIQUE : 80  -> 81
```

## 117.2 Faces substitutas

```text
0: (1, 0, 80)
1: (0, 3, 80)
2: (3, 1, 80)
```

## 117.3 TFACE substitutos

```text
0: (1, 0, 124)
1: (0, 3, 124)
2: (3, 1, 124)
```

## 117.4 Self-check

```text
serialized counts:        OK
new vertex coordinate:    OK
split face topology:      OK
split TFACE topology:     OK
old Physique assignments: OK
new Physique assignment:  OK
```

Hashes:

```text
template:
86a45454c98db4ef8239a1264413a7b34ec558f993c3d4148a1ccd0c03935887

output:
b1150ca297cde307338228db22f7cc0992490f0dc38322637945307a348cfb9b
```

Tamanhos:

```text
template: 456373 bytes
output:   453250 bytes
```

Estado correto neste ponto:

```text
V5c.2 Blender topology edit:      PROVADO
V5c.2 ASE serialization/selfcheck: PASSOU
V5c.2 independent V4 reimport:    PENDENTE
V5c.2 runtime client:             PENDENTE
```

Não marcar ainda a V5c.2 completa como PROVADA até o importer V4
reabrir a topologia nova.

---

# 118. Próximo gate — importer V4 sobre topologia nova

Executar:

```text
MfbC13_roundtrip_v5c_2_one_face_split.ASE
->
pt_ase_import_v4.py
->
novo .blend
```

Critérios para `MahC20`:

```text
V=81
F=148
TV=125
TF=148
UVfaces=148

PHYSIQUE:
declared=81
parsed=81
applied=81
unassigned=0
missingBone=0
invalidVertex=0
duplicates=0
```

As outras três meshes devem permanecer exatamente:

```text
MshC19: V=163 F=264 TV=250 TF=264
MbhC19: V=143 F=248 TV=212 TF=248
MlhC20: V=286 F=428 TV=296 TF=428
```

Se esse gate passar, estará provado:

```text
Blender
-> topology edit
-> V5c.2 ASE
-> independent V4 importer
-> Blender novamente
```

A etapa seguinte será escolher entre:

```text
a) runtime controlado no cliente;
b) antes disso, um segundo round-trip V5c.2 -> V4 -> V5c.2
   para testar estabilidade do novo ASE.
```

A abordagem conservadora recomenda primeiro o reimport independente.
---

# 119. V5c.2 — reimport independente com topologia nova PROVADO

Teste executado em 2026-08-23.

ASE com topologia nova:

```text
MfbC13_roundtrip_v5c_2_one_face_split.ASE
```

foi reaberto por:

```text
pt_ase_import_v4.py
```

sem usar a lógica interna de self-check do exporter.

## 119.1 Skeleton

O skeleton permaneceu íntegro:

```text
Bip nodes found:   63
Bones created:     63
Parent links:      62
Missing positions: 0
```

## 119.2 Meshes

As três meshes não editadas permaneceram:

```text
MshC19:
V=163 F=264 TV=250 TF=264 UVfaces=264

MbhC19:
V=143 F=248 TV=212 TF=248 UVfaces=248

MlhC20:
V=286 F=428 TV=296 TF=428 UVfaces=428
```

A mesh alterada voltou como:

```text
MahC20:
V=81
F=148
TV=125
TF=148
UVfaces=148
MATREF=1
```

Isso corresponde exatamente à operação:

```text
+1 vertex
+2 faces líquidas
+1 TVERT
+2 TFACE líquidos
```

## 119.3 PHYSIQUE

Resultado:

```text
MshC19:
declared=163 parsed=163 applied=163
unassigned=0 missingBone=0 invalidVertex=0 duplicates=0

MbhC19:
declared=143 parsed=143 applied=143
unassigned=0 missingBone=0 invalidVertex=0 duplicates=0

MlhC20:
declared=286 parsed=286 applied=286
unassigned=0 missingBone=0 invalidVertex=0 duplicates=0

MahC20:
declared=81 parsed=81 applied=81
unassigned=0 missingBone=0 invalidVertex=0 duplicates=0
```

Totais:

```text
Physique meshes:       4
Assignments parsed:  673
Vertices applied:    673
Unassigned vertices:   0
Missing-bone assigns:  0
Invalid-vertex refs:   0
Unsupported lines:     0
```

## 119.4 Materiais e texturas

O importer também manteve:

```text
LOADED:  6
MISSING: 0
FAILED:  0
```

## 119.5 Resultado

Pipeline agora provada:

```text
ASE funcional
-> importer V4
-> Blender
-> mudança real de topologia
-> V5c.2
-> ASE com topologia nova
-> importer V4 independente
-> Blender novamente
```

Estado:

```text
V5c.2 topologia nova / round-trip estrutural: PROVADO
V5c.2 runtime cliente:                         PENDENTE
```

---

# 120. Primeiro runtime gate para modelo com topologia nova

O próximo teste deve responder somente:

```text
o cliente Summit consegue carregar/renderizar o ASE V5c.2?
```

Não é necessário tentar avaliar visualmente a qualidade da remodelagem:
a alteração atual é minúscula e foi feita apenas para validar o pipeline.

Procedimento:

```text
1. confirmar qual MfbC13.* está presente no cliente;
2. criar backup com timestamp e SHA256;
3. instalar somente o ASE V5c.2;
4. iniciar o cliente;
5. equipar/visualizar a Priestess level-80 correspondente;
6. observar:
   - crash;
   - model missing;
   - deformação extrema;
   - textura/UV;
   - animação;
7. restaurar o original após o teste.
```

Critério mínimo:

```text
cliente inicia;
modelo aparece;
não ocorre crash na carga/render;
animação continua;
não há corrupção evidente da mesh.
```

Somente após isso:

```text
ASE com topologia nova no runtime Summit: PROVADO
```

A partir desse ponto a V5c pode evoluir de um split controlado para um
exporter de remodelagem arbitrária triangular, com reconstrução geral de:

```text
vertices
faces
TVERT
TFACE
MTLID
PHYSIQUE rígido
```

mantendo template para:

```text
materials
NODE_TM
Biped
animation/controllers
demais metadados do asset
```
---

# 121. Runtime preflight MfbC13 — arquivo concorrente ausente

Preflight executado em 2026-08-23 no diretório:

```text
cliente/char/tmABCD
```

Para o basename:

```text
MfbC13
```

foi encontrado apenas:

```text
MfbC13.ASE
456373 bytes
```

Não foram encontrados no mesmo diretório:

```text
MfbC13.SMD
MfbC13.SMB
MfbC13.INX
```

Hash do ASE original instalado:

```text
86a45454c98db4ef8239a1264413a7b34ec558f993c3d4148a1ccd0c03935887
```

Hash do ASE V5c.2:

```text
b1150ca297cde307338228db22f7cc0992490f0dc38322637945307a348cfb9b
```

Ambos foram identificados pelo `file` como:

```text
ASCII text, with CRLF line terminators
```

Conclusão:

```text
não existe outro arquivo MfbC13.* lado a lado que possa mascarar
o teste substituindo o ASE pelo V5c.2.
```

Estado:

```text
runtime asset preflight: PASSOU
runtime load/render:      PENDENTE
```

---

# 122. Runtime gate controlado — instalação com backup e hash

Procedimento definido:

```text
1. copiar MfbC13.ASE original para backup timestamped;
2. registrar SHA256 original;
3. instalar somente o ASE V5c.2;
4. confirmar SHA256 instalado;
5. iniciar o cliente;
6. visualizar a Priestess / outfit correspondente;
7. observar crash, missing model, deformação, UV e animação;
8. restaurar o backup após o teste.
```

Critério de runtime mínimo:

```text
cliente carrega;
modelo aparece;
animação continua;
não ocorre crash/corrupção evidente ao carregar a peça.
```

Observação importante:

o split atual é geometricamente coplanar e usa UV central interpolado.
Portanto ele foi desenhado para ser visualmente quase ou totalmente
indistinguível do triângulo original.

Consequentemente:

```text
ausência de diferença visual é esperada.
```

O objetivo desse primeiro runtime gate é compatibilidade de carga/render,
não demonstrar uma remodelagem visível.
---

# 123. Correção — nível de MfbC13 não está provado

A associação anterior:

```text
MfbC13.ASE = armadura/outfit Priestess level 80
```

não possui, até este ponto, prova suficiente na source nem na documentação
operacional do Summit Tale.

Portanto essa associação deve ser rebaixada para:

```text
MfbC13.ASE = modelo Priestess de progressão/índice C13
nível exato requerido = PENDENTE
```

Não utilizar o número `13` do filename como level.

Também não interpretar automaticamente:

```text
Drop 80a
Drop 80b
Drop 80c
Drop 80d
```

da documentação Summit como uma relação direta com `MfbC13`.

Esses blocos documentam grupos de códigos de item/drop, não o filename do
body mesh.

---

# 124. Evidências atuais sobre progressão de armor

A source já observada contém tabelas sequenciais em `playmodel.h` com nomes:

```text
C001.ini
C002.ini
...
C019.ini

mC001.ini
mC002.ini
...
mC019.ini
```

e variantes `...t.ini`.

Para a família Moryon, `mC` é uma forte pista para a classe Priestess, mas
a relação exata:

```text
armor item code
-> índice de armor
-> mCxxx.ini
-> MfbCxx mesh
-> required level
```

ainda precisa ser lida no código/dados.

A existência de:

```text
mC001.ini
```

na primeira posição torna `C01` a candidata natural para o próximo teste de
armadura inicial, mas isto ainda não deve ser marcado como nível 1 até
inspecionar `SetPlayerArmor` e o conteúdo/configuração correspondente.

---

# 125. Documentação Summit — o que ela realmente informa sobre level/item

A documentação operacional informa:

```text
Level Inicial 1
Level Cap 120
Itens BPT até 118
```

e contém comandos administrativos como:

```text
/setlevelpt
/Levelup
```

Ela também agrupa códigos por blocos:

```text
Drop 80a
Drop 80b
Drop 80c
Drop 80d
Drop 90
Drop 100
...
```

Mas não fornece uma tabela direta:

```text
MfbCxx -> level
```

nem:

```text
mCxxx.ini -> level
```

Portanto ela não confirma `MfbC13 = level 80`.

---

# 126. Novo objetivo de runtime

Em vez de depender de uma Priestess de level alto, o runtime gate deve
migrar para uma armadura/body de início de progressão que possa ser
visualizado em um personagem recém-criado.

Primeiros candidatos a auditar:

```text
mC001.ini
MfbC01.*
```

Antes de portar o teste V5c.2 para esse modelo, é necessário provar a cadeia:

```text
item inicial Priestess
-> SetPlayerArmor index
-> mC001.ini (ou outro)
-> mesh/body efetivamente carregado
```

Assim o teste runtime poderá ser reproduzido desde level 1.
---

# 127. Auditoria de armor Priestess — evidência direta de `playmodel.h`

A auditoria de `SrcGame/src/playmodel.h` confirmou que `szArmorNumFile` possui uma
linha explicitamente marcada:

```cpp
//PRIESTESS
```

e sua sequência inicial é:

```text
mC001.ini
mC001.ini
mC002.ini
mC003.ini
mC004.ini
mC005.ini
mC006.ini
mC007.ini
mC008.ini
mC009.ini
mC010.ini
mC011.ini
mC012.ini
mC013.ini
...
```

Isto PROVA que `mC` é a família normal de armor da Priestess dentro da tabela
`szArmorNumFile`.

Também existe a tabela `szArmorNumFile_Comic`, cuja linha Priestess começa:

```text
mC001t.ini
mC001t.ini
mC002.ini
mC003t.ini
...
```

Portanto as variantes `...t.ini` pertencem ao caminho Comic/Traje e não devem ser
confundidas com a sequência normal.

O nível exato de cada posição ainda NÃO está provado por essa tabela sozinha.

---

# 128. Candidato inicial Priestess ficou muito mais forte

No cliente atual existem:

```text
MfbC01.smd
MfbC01.smb
MfbC01.bmp
MfbC01b.smd
```

e também progressões como:

```text
MfbC03.smd
MfbC03.bmp
MfbC05.smd
MfbC05.bmp
```

Assim:

```text
mC001.ini + MfbC01.*
```

é agora o par candidato principal para o body/armor inicial da Priestess.

Ainda falta ler `SetPlayerArmor()` / `ChangeModelSkin()` para provar a transformação:

```text
índice/código de armor
-> szArmorNumFile[classe][posição]
-> arquivo mCxxx.ini
-> body MfbCxx
```

Não marcar ainda `MfbC01 = level 1` apenas pelo nome.

---

# 129. Limitação importante — só existe um ASE Priestess no cliente auditado

A busca por:

```text
MfbC*.ASE
```

retornou somente:

```text
MfbC13.ASE
```

Enquanto os modelos iniciais `MfbC01`, `MfbC03` e `MfbC05` estão em SMD/SMB.

Consequência prática:

- o pipeline ASE/Blender atual não pode simplesmente trocar de `MfbC13` para
  `MfbC01` usando um ASE original do cliente, porque esse ASE não está presente;
- para um runtime gate em armor inicial será necessário:
  - encontrar uma fonte ASE correspondente em outro conjunto confiável; ou
  - converter/reconstruir a partir do SMD/SMB; ou
  - temporariamente forçar o modelo ASE C13 em runtime, se a source permitir,
    apenas para validar o loader/exporter independentemente do level.

A escolha só deve ser feita depois de fechar `SetPlayerArmor()` e
`ChangeModelSkin()`.

---

# 130. Estado atual de `MfbC13.ASE` no cliente

Na auditoria atual, o arquivo localizado no cliente possui:

```text
MfbC13.ASE = 453250 bytes
```

Isto deve ser tratado como estado atual do arquivo, e não automaticamente como o
template original usado nos primeiros testes.

Antes de novos testes runtime com esse arquivo, comparar hash/tamanho com os
backups/templates conhecidos e evitar sobrescrever a referência boa.

---

# 131. Próxima auditoria exata

Próximo passo:

```text
ler somente:
- LoadOftenArmor()
- SetPlayerArmor()
- ChangeModelSkin()
- região que calcula a variável Armor antes de chamar SetPlayerArmor()
```

Objetivo:

```text
PROVAR o significado do índice da tabela;
PROVAR se a seleção visual possui ou não checagem de level;
PROVAR se podemos forçar um armor visual para teste sem alterar level/item.
```

Somente depois disso escolher o modelo do runtime gate.
---

# 132. Base visual da Priestess após criação — índice 0 PROVADO

A inspeção de `SrcGame/src/playsub.cpp` fechou a seleção visual de armor.

`SetPlayerArmor(int nItem_LowCode, int nItem_HiCode)`:

```text
- converte nItem_LowCode == -1 para 0;
- usa JOB_CODE para selecionar a linha da classe;
- para armor normal usa:
  szArmorNumFile[ArmorJobNum[jobNum]][nItem_LowCode];
- chama AddLoaderPattern() com esse nome;
- não consulta Level / RequiredLevel nessa função.
```

No slot `INVENTORY_POS_ARMOR`:

```text
equipado:
Armor = ((dwCode & sinITEM_MASK3) >> 8)
if Armor > 25: Armor -= 16
SetPlayerArmor(Armor, itemHighCode)

desequipado:
SetPlayerArmor(0, -1)

sem CODE:
SetPlayerArmor(-1, -1)
```

Como `-1` é normalizado para `0`, ambos os caminhos de "sem armor" resultam em
índice visual 0.

Na tabela `szArmorNumFile`, a linha `//PRIESTESS` possui:

```text
index 0 -> mC001.ini
index 1 -> mC001.ini
index 2 -> mC002.ini
...
```

Portanto, para uma Priestess normal, sem comic/costume especial:

```text
base visual / sem armor
-> SetPlayerArmor(0, -1)
-> szArmorNumFile[PRIESTESS][0]
-> mC001.ini
```

Estado:

```text
Priestess base usa índice visual 0: PROVADO
Priestess índice 0 aponta mC001.ini: PROVADO
nível não participa da troca visual em SetPlayerArmor: PROVADO
```

---

# 133. Consequência — personagem base é o melhor runtime gate

Para testes reproduzíveis, o personagem recém-criado é melhor que um armor de
progressão alta.

Vantagens:

```text
- disponível imediatamente;
- independe de level;
- independe de obter/equipar item;
- pode ser reproduzido criando uma nova Priestess;
- base visual é selecionada por índice 0.
```

Isso torna `mC001.ini` / família `MfbC01.*` o alvo preferencial de design e
runtime.

---

# 134. Textura base Priestess — já temos evidência runtime

O arquivo:

```text
MfbC01.bmp
```

já foi alterado em teste visual anterior e a mudança apareceu no corpo/dress da
Priestess.

Portanto, para reskin 2D do personagem base:

```text
MfbC01.bmp
```

é um alvo runtime já confirmado.

A pipeline de BMP protegido também já foi provada:

```text
preservar header proprietário de 14 bytes;
editar payload RGB24 256x256;
restaurar header proprietário;
mesmo tamanho/dimensões/bpp;
cliente renderiza.
```

Assim a parte "design da personagem base por textura" pode começar agora sem
depender do Blender ou de level.

---

# 135. Geometria base — bloqueio atual é formato, não level

Os arquivos encontrados para o começo da progressão Priestess são:

```text
MfbC01.smd
MfbC01.smb
MfbC01b.smd
MfbC01.bmp
```

Não existe `MfbC01.ASE` no cliente auditado.

O único `MfbC*.ASE` encontrado foi `MfbC13.ASE`.

Consequência:

```text
editar geometria base no Blender pelo pipeline ASE atual:
AINDA NÃO DIRETO
```

O bloqueio é:

```text
SMD/SMB -> representação Blender/ASE editável
```

e não requirement de level.

Próximo objetivo técnico para geometria base:

```text
provar como mC001.ini resolve para MfbC01.smd/smb;
depois abrir/converter o SMD/SMB ou localizar ASE fonte equivalente.
```

---

# 136. Seleção visual não possui checagem de level

A inspeção de `SetPlayerArmor()` e do caminho `INVENTORY_POS_ARMOR` não mostrou
qualquer uso de:

```text
Level
RequireLevel
ItemInfo.Level
lpCurPlayer->smCharInfo.Level
```

para escolher o arquivo visual.

Logo:

```text
requirement de item e seleção visual são camadas separadas nesse trecho.
```

Isso significa que, para fins de debug, um índice visual pode em princípio ser
forçado independentemente do level, desde que o caminho de modelo correspondente
seja válido.

Não aplicar patch de força ainda; primeiro preferir o personagem base index 0.
---

# 137. Resolução do personagem base — pontos de entrada localizados

Nova auditoria localizou os pontos exatos da cadeia:

```text
szArmorDirectory
= "char//tmABCD//"
```

em:

```text
SrcGame/src/playsub.cpp:68
```

`SetPlayerArmor()` monta:

```text
char//tmABCD//mC001.ini
```

para a Priestess base e entrega o caminho a:

```text
AddLoaderPattern(lpCurPlayer, szbuff, head)
```

A definição de `AddLoaderPattern()` está em:

```text
SrcGame/src/character.cpp:704
```

A definição de `LoadOftenMeshPattern()` está em:

```text
SrcGame/src/playsub.cpp:282
```

Também foram localizados os pontos de extensão/model resolution:

```text
ChangeFileExt()
SrcGame/src/smLib3d/smRead3d.cpp:110

smFindPatFile()
SrcGame/src/smLib3d/smObj3d.cpp:2875
```

e usos de `ChangeFileExt()` durante leitura de modelo em:

```text
smRead3d.cpp:1849
smRead3d.cpp:2058
smRead3d.cpp:2148
smRead3d.cpp:2237
smRead3d.cpp:2470
smRead3d.cpp:3433
```

Estado:

```text
entrada visual base mC001.ini: PROVADO
resolução mC001.ini -> MfbC01.smd/smb: PENDENTE
```

---

# 138. Próximo gate de resolução de modelo

Ler somente:

```text
character.cpp 680..820
playsub.cpp 260..340
smRead3d.cpp 90..140
smRead3d.cpp 1820..1885
smRead3d.cpp 2025..2085
smRead3d.cpp 2125..2170
```

Objetivo:

```text
descobrir se:
mC001.ini
  -> lookup/config interno
  -> MfbC01
  -> .smd/.smb

ou se:
mC001.ini
  -> ChangeFileExt
  -> outro arquivo intermediário
```

Não alterar runtime antes de fechar essa resolução.
---

# 139. Loader base Priestess — estado após auditoria AddLoaderPattern

A auditoria confirmou:

```text
AddLoaderPattern()
SrcGame/src/character.cpp:704
```

não resolve o nome de modelo imediatamente. Ele apenas copia:

```text
szName  -> smLoader[cnt].szLoadName
szName2 -> smLoader[cnt].szLoadName2
```

marca:

```text
lpChar->PatLoading = TRUE
```

e dispara:

```text
CreateLoaderThread()
```

Portanto:

```text
SetPlayerArmor(0,-1)
-> "char//tmABCD//mC001.ini"
-> AddLoaderPattern()
```

está PROVADO até a fila assíncrona, mas o trecho da branch `lpChar` dentro de
`smLoaderThreadProc()` ainda precisa ser lido além da linha 820.

---

# 140. LoadOftenMeshPattern confirma entrada em smPATTERN::LoadCharactor

`LoadOftenMeshPattern()` é:

```cpp
DynTempPat[DynTempPatCnt] = smPattern.LoadCharactor(szFileName);
```

Portanto o preload normal de armor entrega o nome lógico da tabela de armor para:

```text
smPATTERN::LoadCharactor()
```

A definição dessa função ainda não foi exibida.

Este é agora o ponto central da resolução:

```text
mC001.ini
-> smPATTERN::LoadCharactor()
-> ?
-> MfbC01.smd / MfbC01.smb
```

---

# 141. ChangeFileExt e formatos compilados

A source confirmou:

```text
szFileModelBip = "smb"
szFileModel    = "smd"
```

e `smFindFile()` pode substituir a extensão do nome fornecido usando
`ChangeFileExt()`.

A pipeline ASE interna também mostra:

```text
ASE -> SaveFile(.smd) -> smReadModel(.smd)
ASE bone -> SaveFile(.smb)
```

Logo:

```text
SMD = formato compilado de mesh/model desta engine
SMB = formato compilado de bone/animation desta engine
```

está confirmado pela própria source.

Porém, trocar simplesmente:

```text
mC001.ini -> mC001.smd
```

não explica os assets atuais, porque o cliente possui:

```text
MfbC01.smd
MfbC01.smb
```

e não foi observado `mC001.smd`.

Portanto existe, com alta probabilidade, uma camada de mapping ainda não
inspecionada dentro de `LoadCharactor()` ou das rotinas chamadas por ele.

Isso é uma INFERÊNCIA, não ainda PROVADO.

---

# 142. Direção oficial para o vertical slice

O alvo oficial passa a ser:

```text
Priestess base recém-criada
```

com:

```text
textura:   MfbC01.bmp
geometria: MfbC01.smd
bones:     MfbC01.smb
```

A textura `MfbC01.bmp` já possui prova runtime anterior.

Para geometria, o próximo objetivo é localizar e entender
`smPATTERN::LoadCharactor()` antes de construir qualquer importer/exporter SMD.
---

# 143. Thread de personagem — LoadCharactor é o choke point real

A branch `lpChar` de `smLoaderThreadProc()` foi inspecionada.

Ela executa:

```cpp
smDPat = smPattern.LoadCharactor(smLoader[cnt].szLoadName);
```

e, quando o load passa:

```text
SetDinaPattern(smDPat)
SetMotionFromCode(STAND)
szModelName = smDPat->szPatName
```

Se `smDPat->lpModelInfo->szSubModelFile` existir, ele também chama:

```cpp
smPattern.LoadCharactor(smDPat->lpModelInfo->szSubModelFile)
```

para o segundo pattern.

Portanto o body/base lógico:

```text
char//tmABCD//mC001.ini
```

entra diretamente em:

```text
smPATTERN::LoadCharactor()
```

Estado:

```text
SetPlayerArmor -> AddLoaderPattern -> smLoaderThreadProc -> LoadCharactor:
PROVADO
```

---

# 144. LoadCharactor usa smMODELINFO e modelInfo.szModelFile

A definição de:

```text
smPATTERN::LoadCharactor
```

foi localizada em:

```text
SrcGame/src/character.cpp:544
```

O grep da própria source revelou dentro desse bloco:

```text
smMODELINFO modelInfo
modelInfo.szModelFile
smASE_Read(modelInfo.szModelFile)
```

Também foi localizado:

```text
smModelDecode(char *iniFile, smMODELINFO *Info)
SrcGame/src/fileread.cpp:1001
```

e:

```text
AddModelDecode()
SrcGame/src/fileread.cpp:278
```

`AddModelDecode()` contém atribuições para:

```text
Info->szModelFile
Info->szLinkFile
```

Logo o mapping:

```text
mC001.ini -> smMODELINFO -> szModelFile
```

está claramente centralizado nessa camada.

Ainda não marcar:

```text
szModelFile == MfbC01.smd
```

até abrir o corpo das funções.

---

# 145. Hipótese INX deve ser verificada

Como `mC001.ini` não foi encontrado no diretório `tmABCD`, mas a engine possui
rotinas de model-info compilado e referências comunitárias/anteriores a INX, o
próximo diagnóstico deve procurar:

```text
mC001.*
```

e não apenas:

```text
mC001.ini
```

Isso pode revelar um:

```text
mC001.inx
```

ou outro arquivo intermediário compilado.

Estado:

```text
hipótese .inx: PENDENTE
```

---

# 146. Próximo gate mínimo

Inspecionar apenas:

```text
character.cpp 530..655
fileread.cpp 960..1225
```

e listar:

```text
mC001.*
MfbC01.*
```

no cliente.

Objetivo:

```text
PROVAR:
mC001 logical armor file
-> model-info decode
-> exact szModelFile
-> exact link/bone file
-> MfbC01 SMD/SMB (ou corrigir a hipótese)
```
---

# 147. mC001.inx encontrado — camada compilada PROVADA

A auditoria do cliente encontrou:

```text
char/tmABCD/mC001.inx
95268 bytes
```

Logo o nome lógico da tabela:

```text
mC001.ini
```

não precisa existir como arquivo texto no cliente.

`smModelDecode()` chama:

```text
smFindFile(szFile, szFileExt, &dwFileLen)
```

e aceita o arquivo encontrado quando:

```text
dwFileLen == sizeof(smMODELINFO)
```

Depois executa:

```text
fread(&smModelInfo, sizeof(smMODELINFO), 1, fp)
ModelKeyWordDecode(...)
```

Portanto a cadeia:

```text
mC001.ini
-> mC001.inx
-> smMODELINFO
```

está PROVADA.

---

# 148. LoadCharactor fecha a próxima etapa

`smPATTERN::LoadCharactor()` executa:

```text
smModelDecode(szFileName, &modelInfo)
```

e depois:

```text
DinaPat[Num].Pat = smASE_Read(modelInfo.szModelFile)
```

Também usa:

```text
modelInfo.szMotionFile
modelInfo.szTalkMotionFile
modelInfo.szSubModelFile
```

quando presentes.

Assim:

```text
mC001.inx
-> decoded smMODELINFO
-> exact model file
-> smASE_Read()
```

está PROVADO estruturalmente.

O valor concreto de:

```text
modelInfo.szModelFile
```

para `mC001.inx` ainda precisa ser observado no trace/runtime.

---

# 149. Trace MinGW já existente pode provar o mapping exato

Dentro de `smModelDecode()` existe:

```text
MINGW_MODEL_PATH_CHECKSUM_V2
```

que grava:

```text
LOGS\mingw_model_decode_trace.txt
```

com:

```text
input
canonical
size
code
keyword
model
link
motionCount
```

Portanto o próximo gate não precisa de patch.

Basta iniciar uma Priestess base e inspecionar a entrada correspondente a:

```text
mC001.inx
```

Resultado esperado, ainda como hipótese:

```text
model = char\tmABCD\MfbC01.smd
link  = char\tmABCD\MfbC01.smb
```

ou outra combinação equivalente.

Somente a saída real do trace poderá marcar esse mapping como PROVADO.

---

# 150. Assets físicos candidatos confirmados

No cliente existem:

```text
MfbC01.bmp        196662
MfbC01b.smd       108144
MfbC01h-02.TGA     32812
MfbC01.smb        221216
MfbC01.smd        290229
```

Logo existe uma família física coerente para o personagem base.

Estado atual:

```text
Priestess base index 0 -> mC001.ini: PROVADO
mC001.ini -> mC001.inx:             PROVADO
mC001.inx -> smMODELINFO:            PROVADO
exact szModelFile/szLinkFile:        PENDENTE
```
---

# 151. Mapping exato da Priestess base observado no trace runtime

O trace `MINGW_MODEL_PATH_CHECKSUM_V2` registrou para o decode lógico de
`mC001.inx`:

```text
canonical=[char\tmABCD\mc001.inx]
size=95268
code=0x71153D8C
keyword=1
model=[char\tmABCD\MfbC01.ASE]
link=[char\tmABCD\M5Bip.in]
motionCount=10
```

Portanto está PROVADO no runtime/source:

```text
Priestess base
-> mC001.ini (nome lógico da tabela)
-> mC001.inx
-> smMODELINFO
-> szModelFile = char\tmABCD\MfbC01.ASE
-> szLinkFile  = char\tmABCD\M5Bip.in
```

A linha seguinte decodifica o link:

```text
canonical=[char\tmABCD\M5Bip.inx]
size=95268
code=0xF4154175
keyword=1
model=[char\tmABCD\MfbC01.ASE]
link=[char\tmABCD\M5Bip.in]
motionCount=64
```

Assim `M5Bip` é a camada de motion/link compartilhada e não o body mesh.

---

# 152. Explicação do campo `input` aparentemente estranho no trace

O trace mostra em algumas linhas:

```text
input=[char\tmABCD\M5Bip.inx]
canonical=[char\tmABCD\mc001.inx]
```

Isso não significa que `mC001` foi ignorado.

O patch MinGW primeiro copia o path retornado por `smFindFile()` para o buffer
local `mingwModelCodePath`, que preserva:

```text
char\tmABCD\mc001.inx
```

Depois chama:

```text
ModelKeyWordDecode(dwCode, &smModelInfo)
```

Essa rotina usa `ChangeFileExt()` sobre `szLinkFile`; `ChangeFileExt()` trabalha
com buffer global estático. O ponteiro `lpFile` também aponta para esse buffer
global, portanto seu conteúdo pode passar a mostrar `M5Bip.inx` antes do
`fprintf`.

Logo, para identificar o arquivo que gerou o checksum nessa instrumentação, o
campo confiável é:

```text
canonical=[...]
```

e não o `input=[...]` impresso depois da chamada.

---

# 153. Descoberta crítica: o INX referencia ASE, não SMD diretamente

O mapping real não contém:

```text
model=MfbC01.smd
```

Ele contém:

```text
model=MfbC01.ASE
```

Isso combina com `LoadCharactor()`:

```cpp
DinaPat[Num].Pat = smASE_Read(modelInfo.szModelFile);
```

Porém o cliente atual não contém `MfbC01.ASE`; contém:

```text
MfbC01.smd
MfbC01.smb
```

Logo o passo seguinte precisa confirmar a prioridade interna de `smASE_Read()`:

hipótese forte:

```text
smASE_Read("MfbC01.ASE")
-> procura MfbC01.smd
-> se SMD compilado existe, carrega SMD
-> somente se necessário lê/converte ASE
```

Essa hipótese é consistente com a existência de `smFindFile(file, "smd")`,
`smReadModel()` e com a pipeline ASE->SMD da source, mas ainda deve ser fechada
lendo o início de `smASE_Read()`.

---

# 154. Impacto para o vertical slice base

O alvo correto está agora inequivocamente identificado:

```text
logical armor: mC001
body source-name: MfbC01.ASE
runtime compiled body: MfbC01.smd (hipótese de precedência a confirmar)
motion/link: M5Bip
texture: MfbC01.bmp
```

Isto é uma ótima notícia para o pipeline Blender: a engine conceitualmente
trata o body base como um asset ASE chamado `MfbC01.ASE`, mesmo que a distribuição
runtime mantenha somente sua versão compilada SMD.

Próximo gate:

```text
ler smASE_Read() 1920..2070
```

e provar a política ASE/SMD.
---

# 155. Prioridade SMD no runtime — PROVADO

A implementação completa de:

```text
smASE_Read()
SrcGame/src/smLib3d/smRead3d.cpp:1942
```

foi inspecionada.

O início executa:

```cpp
SetDirectoryFromFile(file);

if(smMeshReload && (szFile = smFindFile(file, szFileModel)))
{
    pat = smReadModel(szFile, szName);

    if(pat)
    {
        return pat;
    }
}
```

e a source define:

```cpp
static int smMeshReload = TRUE;
```

Além disso:

```text
szFileModel = "smd"
```

Portanto, quando recebe:

```text
MfbC01.ASE
```

o runtime primeiro procura:

```text
MfbC01.smd
```

e, se o SMD carregar, retorna sem abrir o ASE.

Estado:

```text
MfbC01.ASE logical name -> MfbC01.smd runtime preference: PROVADO
```

---

# 156. Fallback ASE e recompilação automática

Se o SMD não existir ou falhar, `smASE_Read()` faz:

```cpp
fp = fopen(file, "rt");
```

e lê o ASE.

Após construir o `smPAT3D`, executa:

```cpp
szFile = ChangeFileExt(file, szFileModel);
pat->SaveFile(szFile);

delete pat;
pat = nullptr;

pat = smReadModel(szFile, szName);
```

Logo a engine possui nativamente a pipeline:

```text
ASE
-> parse
-> SaveFile(.smd)
-> smReadModel(.smd)
```

Estado:

```text
ASE -> SMD runtime compiler path: PROVADO
```

---

# 157. Consequência para testes do personagem base

Com `MfbC01.smd` presente:

```text
MfbC01.ASE editado ao lado do SMD
```

NÃO é suficiente para testar o ASE, porque o SMD tem prioridade.

Um runtime gate ASE deve ser:

```text
1. backup MfbC01.smd;
2. instalar MfbC01.ASE novo;
3. remover/renomear temporariamente MfbC01.smd;
4. iniciar cliente;
5. smASE_Read cai no ASE;
6. cliente gera novo MfbC01.smd;
7. testar Priestess recém-criada;
8. restaurar original.
```

Não executar ainda: primeiro precisamos gerar um ASE base confiável a partir do
SMD existente.

---

# 158. Novo gargalo técnico — SMD -> Blender

O problema de level/model selection está encerrado.

O alvo oficial é:

```text
Priestess base
logical: MfbC01.ASE
runtime: MfbC01.smd
motion/config: mC001.inx / M5Bip
texture: MfbC01.bmp
```

Para editar geometria base, o próximo objetivo é:

```text
MfbC01.smd
-> reader Python baseado na própria source
-> Blender
```

A source contém `smPAT3D::LoadFile()` / `SaveFile()` e as estruturas usadas pelo
formato proprietário, portanto essa rota deve ser preferida ao `SMDReader.exe`
externo.

Próximo gate:

```text
localizar e inspecionar:
smPAT3D::LoadFile
smPAT3D::SaveFile
smOBJ3D::LoadFile
smOBJ3D::SaveFile
header SMD Model data Ver 0.62
estruturas serializadas de vertex/face/texture/material
```

Depois disso será possível escrever um importer SMD V1 somente leitura, sem
tocar ainda em export.
---

# 159. SMD de personagem — funções de serialização localizadas

A auditoria localizou as implementações exatas do formato SMD de personagem:

```text
SrcGame/src/smLib3d/smObj3d.cpp:1922
smOBJ3D::SaveFile(HANDLE hFile)

SrcGame/src/smLib3d/smObj3d.cpp:1960
smOBJ3D::LoadFile(HANDLE hFile, smPAT3D *PatPhysique)

SrcGame/src/smLib3d/smObj3d.cpp:2706
smPAT3D::SaveFile(char *szFile)

SrcGame/src/smLib3d/smObj3d.cpp:2790
smPAT3D::LoadFile(char *szFile, char *szNodeName)
```

Header confirmado:

```text
SMD Model data Ver 0.62
```

em:

```text
SrcGame/src/smLib3d/smObj3d.cpp:2702
```

Também existe:

```text
SMD Stage data Ver 0.72
```

em `smStage3d.cpp`, mas esse é um formato distinto de stage/map e não deve ser
misturado com o SMD de personagem.

---

# 160. Estratégia oficial para importer SMD V1

A rota preferida agora é:

```text
MfbC01.smd
-> replicar smPAT3D::LoadFile()
-> replicar smOBJ3D::LoadFile()
-> validar header/offsets/counts
-> extrair meshes
-> importar no Blender
```

Não fazer engenharia reversa heurística enquanto a própria source define a
serialização.

Primeiro importer deve ser somente leitura.

Não implementar ainda:

```text
export SMD
reescrita de bones
animação
topologia nova
```

até provar que a leitura de `MfbC01.smd` reproduz a geometria corretamente.

---

# 161. Próximo gate

Inspecionar:

```text
smObj3d.cpp 1900..2035
smObj3d.cpp 2680..2940
smObj3d.h   70..340
```

Objetivo:

```text
identificar:
- structs gravadas diretamente com WriteFile/ReadFile;
- tamanho/header do arquivo;
- offsets de material/object table;
- contagens de objetos;
- layout de vertex/face/texlink;
- transforms;
- ligação com PatPhysique;
- material group serialization.
```

Somente depois disso escrever o parser Python.
---

# 162. Layout binário SMD de personagem — estrutura macro PROVADA

A inspeção de `smPAT3D::SaveFile/LoadFile` e `smOBJ3D::SaveFile/LoadFile`
confirmou o layout macro do arquivo `SMD Model data Ver 0.62`.

No nível `smPAT3D`:

```text
smDFILE_HEADER
smDFILE_OBJINFO[ObjCounter]
material group serializado (se MatCounter > 0)
objeto 0
objeto 1
...
objeto N-1
```

`smDFILE_OBJINFO` contém pelo menos:

```text
szNodeName
Length
ObjFilePoint
```

e `ObjFilePoint` é usado pelo loader para `SetFilePointer()` quando um node
específico é requisitado.

O header armazena:

```text
MatCounter
ObjCounter
MatFilePoint
First_ObjInfoPoint
TmFrameCounter
TmFrame[OBJ_FRAME_SEARCH_MAX]
```

O header string é:

```text
SMD Model data Ver 0.62
```

---

# 163. Cada bloco smOBJ3D é dump ABI cru + arrays

`smOBJ3D::SaveFile()` faz primeiro:

```cpp
WriteFile(hFile, &Head, sizeof(smOBJ3D), ...)
```

Como `Head` é o primeiro membro do objeto, isso grava a representação binária
inteira da instância `smOBJ3D`, inclusive campos escalares, matrizes e valores
dos ponteiros existentes naquele momento.

Depois grava, nesta ordem:

```text
smVERTEX[nVertex]
smFACE[nFace]
smTEXLINK[nTexLink]
smTM_ROT[TmRotCnt]
smTM_POS[TmPosCnt]
smTM_SCALE[TmScaleCnt]
smMATRIX[TmRotCnt]   // TmPrevRot no SaveFile
```

e, se o campo `Physique` serializado for não nulo:

```text
char boneName[nVertex][32]
```

Portanto o parser Python não pode assumir apenas uma estrutura lógica; ele precisa
reproduzir o ABI de 32 bits usado para compilar o cliente.

---

# 164. Ponteiros serializados são usados como índices relativos

Ao carregar, a engine lê o dump cru de `smOBJ3D`, preserva o valor antigo de:

```text
TexLink
```

em:

```text
lpOldTexLink
```

e depois aloca um novo array.

Os ponteiros serializados:

```text
smTEXLINK.NextTex
smFACE.lpTexLink
```

são reparados através de subtração em relação a `lpOldTexLink`.

Em C/C++ essa subtração de ponteiros já produz um índice em unidades de
`smTEXLINK`.

Logo, para um parser Python:

```text
oldTexBase = valor 32-bit serializado de smOBJ3D::TexLink
index = (serializedPointer - oldTexBase) / sizeof(smTEXLINK)
```

é a interpretação binária equivalente, desde que os ponteiros realmente sejam
do ABI 32-bit esperado.

Esse detalhe torna obrigatório medir:

```text
sizeof(smOBJ3D)
offsetof dos membros
sizeof(smFACE)
sizeof(smTEXLINK)
```

no mesmo ABI i686.

---

# 165. Physique rígido no SMD

Se `smOBJ3D::Physique` estiver não nulo, `SaveFile()` grava exatamente 32 bytes
por vértice contendo:

```text
Physique[cnt]->NodeName
```

O `LoadFile()` lê esses nomes e executa:

```text
PatPhysique->GetObjectFromName(...)
```

para cada vértice.

Isso confirma que esse formato SMD usa, neste caminho:

```text
um bone/node name rígido por vértice
```

o que é consistente com a associação rígida que já foi observada no ASE/PHYSIQUE.

---

# 166. Próximo passo deve medir ABI, não escrever parser ainda

Antes de criar `pt_smd_probe_v1.py`, ainda faltam os tamanhos e offsets exatos
dos tipos serializados.

É necessário obter:

```text
sizeof(smDFILE_HEADER)
sizeof(smDFILE_OBJINFO)
sizeof(smOBJ3D)
sizeof(smVERTEX)
sizeof(smFACE)
sizeof(smTEXLINK)
sizeof(smTM_ROT)
sizeof(smTM_POS)
sizeof(smTM_SCALE)
sizeof(smMATRIX)
sizeof(smFMATRIX)

offsetof relevantes de smOBJ3D
offsetof de lpTexLink em smFACE
offsetof de NextTex em smTEXLINK
```

Como o projeto-alvo é i686/32-bit, essa medição deve ser feita com o mesmo
toolchain MinGW de 32 bits sempre que possível.

Depois dos valores ABI, o primeiro probe Python poderá:

```text
1. validar header;
2. ler ObjCounter/MatCounter;
3. ler a tabela de objetos;
4. pular material via ObjFilePoint;
5. ir diretamente a cada object block;
6. decodificar contagens/nome;
7. validar que Length == GetSaveSize calculado;
8. só então extrair vertices/faces/UV.
```
---

# 167. Toolchain i686 disponível — medição ABI será direta

Foi confirmado no ambiente:

```text
/usr/lib64/ccache/i686-w64-mingw32-g++
i686-w64-mingw32-g++ (GCC) 15.2.1
```

Portanto os tamanhos/offsets do SMD podem ser medidos com um compilador alvo
Windows i686/32-bit compatível com o build atual do projeto.

Não é necessário inferir ponteiros como 64-bit do host Linux.

---

# 168. Header e object table — layout fonte identificado

`smDFILE_HEADER`:

```cpp
struct smDFILE_HEADER
{
    char szHeader[24];
    int ObjCounter;
    int MatCounter;
    int MatFilePoint;
    int First_ObjInfoPoint;
    int TmFrameCounter;
    smFRAME_POS TmFrame[32];
};
```

`smFRAME_POS`:

```cpp
struct smFRAME_POS
{
    int StartFrame;
    int EndFrame;
    int PosNum;
    int PosCnt;
};
```

`smDFILE_OBJINFO`:

```cpp
struct smDFILE_OBJINFO
{
    char szNodeName[32];
    int Length;
    int ObjFilePoint;
};
```

Por aritmética de layout natural de 4 bytes, os tamanhos candidatos são:

```text
smFRAME_POS      = 16
smDFILE_HEADER   = 556
smDFILE_OBJINFO  = 40
```

Estado:

```text
tamanhos por inferência: FORTE
tamanhos por ABI i686:   PENDENTE
```

Não marcar como PROVADO até o probe compilar/rodar.

---

# 169. Próximo gate antes do ABI probe

Ainda faltam as definições completas de:

```text
smMATRIX
smFMATRIX
smTEXLINK
smVERTEX
smFACE
smTM_ROT
smTM_POS
smTM_SCALE
```

e qualquer diretiva de packing:

```text
#pragma pack
__attribute__((packed))
```

O próximo dump deve mostrar somente:

```text
smType.h 70..245
packing dentro de smLib3d
```

Depois disso será criado um ABI probe i686 que imprime `sizeof` e `offsetof`
dos tipos serializados.
---

# 170. Tipos serializados completos — layout lógico confirmado

A auditoria de `smType.h` confirmou:

```text
smMATRIX    = 16 x int
smFMATRIX   = 16 x float

smTEXLINK:
  float u[3]
  float v[3]
  DWORD *hTexture
  smTEXLINK *NextTex

smVERTEX:
  int x,y,z
  int nx,ny,nz

smFACE:
  WORD v[4]
  smFTPOINT t[3]
  smTEXLINK *lpTexLink

smTM_ROT:
  int frame
  float x,y,z,w

smTM_POS:
  int frame
  float x,y,z

smTM_SCALE:
  int frame
  int x,y,z
```

A busca não encontrou `#pragma pack`, `packed` ou alinhamento explícito dentro de
`SrcGame/src/smLib3d`.

Os tamanhos naturais esperados em ABI i686 são:

```text
smMATRIX      64
smFMATRIX     64
smTEXLINK     32
smVERTEX      24
smFACE        36
smTM_ROT      20
smTM_POS      16
smTM_SCALE    16
smFRAME_POS   16
smDFILE_HEADER 556
smDFILE_OBJINFO 40
smOBJ3D       ~2236 (estimativa manual)
```

Esses números continuam como EXPECTED até o probe i686 real confirmar.

---

# 171. ABI probe V1 preparado

Foi preparado:

```text
pt_smd_abi_probe_v1.cpp
```

Objetivo:

```text
- confirmar sizeof(void*) == 4;
- medir sizeof de todos os tipos serializados;
- medir offsetof dos campos necessários;
- medir sizeof(smOBJ3D);
```

O probe inclui diretamente o `smObj3d.h` real do projeto e deve ser compilado
com `i686-w64-mingw32-g++`.

Nenhum asset é modificado por esse teste.
---

# 172. ABI probe V1 — falha de include, não falha de ABI

O primeiro probe não compilou porque incluiu:

```cpp
#include "SrcGame/src/smLib3d/smObj3d.h"
```

sem incluir previamente:

```cpp
#include "SrcGame/src/smLib3d/smType.h"
```

`smObj3d.h` usa tipos definidos em `smType.h`, incluindo:

```text
smVERTEX
smFACE
smTEXLINK
POINT3D
smMATRIX
smFMATRIX
smTM_ROT
smTM_POS
smTM_SCALE
```

A falha ocorreu antes de qualquer medição de `sizeof`/`offsetof`.

Portanto:

```text
ABI i686 incompatível: NÃO demonstrado
probe V1 com include incompleto: PROVADO
```

---

# 173. ABI probe V2

Foi preparado:

```text
pt_smd_abi_probe_v2.cpp
```

Diferença funcional única:

```cpp
#include "SrcGame/src/smLib3d/smType.h"
#include "SrcGame/src/smLib3d/smObj3d.h"
```

O teste seguinte deve apagar qualquer `.exe` antigo antes da compilação e usar
`if ...; then ... else ... fi`, evitando interpretar um status posterior como
resultado da compilação.
---

# 174. ABI probe V2 — bloqueio atual é include path de smPacket.h

O probe V2 avançou além da falha anterior e agora para em:

```text
SrcGame/src/smLib3d/smType.h:3:10:
fatal error: smPacket.h: No such file or directory
```

Isso significa que:

```text
smType.h foi encontrado;
a ordem de include V2 está correta;
o compilador ainda não recebeu o diretório que contém smPacket.h.
```

A compilação terminou explicitamente com:

```text
COMPILE_FAIL
```

Portanto ainda não houve qualquer medição ABI.

Estado:

```text
falha por ABI:            NÃO demonstrada
falha por include path:   PROVADA
```

---

# 175. Próximo gate mínimo

Antes de alterar o comando de compilação, localizar exatamente:

```text
smPacket.h
```

no source tree.

Não adicionar diretórios de include por tentativa.

Comando:

```bash
find . -type f -iname 'smPacket.h' -print
```

Depois usar somente o diretório real retornado no próximo `-I`.
---

# 176. smPacket.h — dois candidatos ativos encontrados

A busca localizou:

```text
./Shared/smPacket.h
./Shared2/smPacket.h
```

além de várias cópias sob `depricated/`, que não devem ser usadas.

Ainda não há prova suficiente para escolher entre:

```text
Shared
Shared2
```

como include path correto do cliente.

Próximo gate:

```text
1. inspecionar cliente.sh por -I...Shared / -I...Shared2;
2. comparar SHA256 dos dois smPacket.h;
3. somente então recompilar o ABI probe.
```

Não adicionar os dois diretórios simultaneamente, pois isso poderia mascarar qual
header o build real resolve primeiro.
---

# 177. Include paths reais do cliente — Shared + Shared2 PROVADOS

O script real:

```text
build_client_mingw_i686_multicore.sh
```

usa:

```text
-I.
-IShared
-IShared2
```

nessa ordem.

Logo o build oficial do cliente disponibiliza os dois trees, com `Shared`
precedendo `Shared2` na busca normal dos `-I`.

O probe ABI deve reproduzir essa mesma ordem, em vez de escolher manualmente um
dos dois.

---

# 178. Probe ABI deve usar as flags reais do cliente

O build do cliente também define:

```text
-m32
-std=gnu++17
-fpermissive

-DWIN32
-D_WINDOWS
-D_M_IX86
-Di386
-D__i386__
-DNDEBUG
-D_CLIENT
```

e todos os include paths usados pelo cliente, incluindo DirectX9.

Para medir `sizeof`/`offsetof` com fidelidade, o próximo compile do probe deve
reutilizar esse ambiente de preprocessador/include.

Isso reduz o risco de um header alterar layout ou typedefs por conditional
compilation diferente do executável real.

Estado:

```text
toolchain i686:                      PROVADO
include path Shared + Shared2:       PROVADO
macros do build cliente:             PROVADO
ABI sizeof/offsetof:                 PENDENTE
```
---

# 179. ABI i686 do SMD — PROVADO pelo probe compilado

O `pt_smd_abi_probe_v2.cpp` compilou com o mesmo ambiente do cliente e gerou:

```text
PE32 executable for MS Windows
Intel i386
```

Executado sob Wine:

```text
PT_SMD_ABI_PROBE_V2
sizeof(void*)=4
```

Portanto o probe está medindo efetivamente o ABI Windows i686/32-bit usado pelo
build atual do cliente.

Tamanhos confirmados:

```text
smFRAME_POS       16
smDFILE_HEADER   556
smDFILE_OBJINFO   40
smMATRIX          64
smFMATRIX         64
smTEXLINK         32
smVERTEX          24
smFACE            36
smTM_ROT          20
smTM_POS          16
smTM_SCALE        16
smOBJ3D         2236
```

Estado:

```text
tamanhos ABI atuais do cliente MinGW i686: PROVADO
```

---

# 180. Offsets críticos — PROVADOS

`smTEXLINK`:

```text
u        0
v       12
hTexture 24
NextTex 28
```

`smFACE`:

```text
v         0
t         8
lpTexLink 32
```

`smOBJ3D`:

```text
Vertex      4
Face        8
TexLink    12
Physique   16
ZeroVertex 20

nVertex    84
nFace      88
nTexLink   92

NodeName   172
NodeParent 204
pParent    236

Tm         240
TmInvert   304
TmResult   368
TmRotate   432
mWorld     496
mLocal     560

TmRot      668
TmPos      672
TmScale    676
TmPrevRot  680

TmRotCnt   684
TmPosCnt   688
TmScaleCnt 692

TmRotFrame   696
TmPosFrame  1208
TmScaleFrame 1720
TmFrameCnt   2232
```

Esses offsets permitem decodificar o dump bruto de `smOBJ3D` no SMD sem depender
de `ctypes` nativo 64-bit.

---

# 181. O que “reproduzir a arquitetura” significa neste projeto

O SMD faz:

```cpp
WriteFile(hFile, &Head, sizeof(smOBJ3D), ...)
```

Como `Head` é o primeiro campo da classe, a engine grava os 2236 bytes da
instância `smOBJ3D` segundo o layout de memória do executável.

Em 64-bit, por exemplo:

```text
ponteiro = 8 bytes
```

e campos como:

```text
Vertex
Face
TexLink
Physique
pParent
TmRot
...
```

mudariam de tamanho/posição, deslocando tudo que vem depois.

No ABI i686 confirmado:

```text
ponteiro = 4 bytes
```

logo os offsets encontrados correspondem ao formato que o build atual do cliente
espera.

As macros:

```text
_CLIENT
WIN32
_M_IX86
...
```

e os mesmos include paths também foram reproduzidos para evitar que conditional
compilation alterasse structs/typedefs durante a medição.

Wine não determina esses tamanhos; eles já foram definidos na compilação.
Wine apenas executa o PE32 para imprimir os resultados.

---

# 182. Limite da prova atual

O probe prova o ABI do:

```text
cliente Summit Tale MinGW i686 atual
```

Ele ainda não prova sozinho que todo SMD legado distribuído foi originalmente
gerado com exatamente o mesmo ABI histórico.

Esse próximo elo deve ser validado diretamente contra:

```text
MfbC01.smd
```

checando:

```text
header
ObjCounter
object table
ObjFilePoint
Length
nVertex/nFace/nTexLink
GetSaveSize calculado
limites dentro do arquivo
```

Se todas as estruturas fecharem exatamente com o tamanho real do arquivo, então
o layout ABI também estará validado contra o asset legado concreto.

Próximo gate:

```text
pt_smd_probe_v1.py
```

somente leitura.
---

# 183. Primeiro probe estrutural do SMD real preparado

Após a confirmação do ABI i686, foi preparado:

```text
pt_smd_probe_v1.py
```

Características:

```text
somente leitura;
não usa Blender;
não altera MfbC01.smd;
não depende de ctypes nativo;
usa offsets/tamanhos confirmados pelo ABI probe.
```

O probe valida:

```text
header "SMD Model data Ver 0.62";
smDFILE_HEADER;
ObjCounter/MatCounter;
tabela smDFILE_OBJINFO;
ObjFilePoint;
Length;
smOBJ3D de 2236 bytes;
nVertex/nFace/nTexLink;
TmRotCnt/TmPosCnt/TmScaleCnt;
NodeName/NodeParent;
flag Physique;
GetSaveSize reconstruído;
continuidade entre objetos;
limites do arquivo.
```

Também imprime poucas amostras de vértices/faces apenas para confirmar
plausibilidade dos campos.

---

# 184. Fórmula de validação usada pelo probe

Para cada objeto:

```text
Length =
  2236
+ 24 * nVertex
+ 36 * nFace
+ 32 * nTexLink
+ 20 * TmRotCnt
+ 16 * TmPosCnt
+ 16 * TmScaleCnt
+ 64 * TmRotCnt
+ (32 * nVertex se Physique != NULL)
```

Essa fórmula reproduz `smOBJ3D::GetSaveSize()` / `SaveFile()` no ABI confirmado.

O teste só poderá marcar o layout do asset legado como PROVADO se os `Length`,
`ObjFilePoint` e limites reais de `MfbC01.smd` fecharem.
---

# 185. MfbC01.smd — validação estrutural completa PROVADA

O `pt_smd_probe_v1.py` foi executado diretamente contra:

```text
char/tmABCD/MfbC01.smd
```

Tamanho real:

```text
290229 bytes
```

Header:

```text
SMD Model data Ver 0.62
```

Header validado.

Campos principais:

```text
ObjCounter=15
MatCounter=6
MatFilePoint=1156
First_ObjInfoPoint=3309
TmFrameCounter=0
```

A tabela de objetos inicia em:

```text
556
```

e termina em:

```text
1156
```

Exatamente:

```text
556 + 15 * 40 = 1156
```

portanto `MatFilePoint` fecha com o ABI medido.

Região de material:

```text
3309 - 1156 = 2153 bytes
```

---

# 186. Todos os 15 object blocks fecharam

Objetos encontrados:

```text
MsmC01
MshC01
MslC01
MsmC03
MslC03
MlhC01
MlhC03
MbhC01
MshC03
MlmC01
MlmC03
MbmC01
MllC01
MllC03
MblC01
```

Para todos:

```text
NodeName == nome da object table
calculated Length == smDFILE_OBJINFO.Length
calculated end == próximo ObjFilePoint
bounds == OK
```

No último:

```text
calculated_end=290229
file_size=290229
final_file_size_check=OK
```

Resumo real:

```text
objects=15
vertices totais=1444
faces totais=2537
texlinks totais=2537
structural_check=PASS
probe_exit=0
```

Estado:

```text
ABI i686 atual compatível com MfbC01.smd legado: PROVADO
layout macro SMD 0.62 do asset real:              PROVADO
```

---

# 187. Object Head confirma OBJ_HEAD_TYPE_NEW_NORMAL

Todos os objetos amostrados apresentaram:

```text
Head=0xC1424344
```

A source salva:

```text
Head = 0x41424344
Head |= OBJ_HEAD_TYPE_NEW_NORMAL
```

e:

```text
OBJ_HEAD_TYPE_NEW_NORMAL = 0x80000000
```

Logo:

```text
0x41424344 | 0x80000000 = 0xC1424344
```

O asset real confirma exatamente essa assinatura.

---

# 188. Ponteiros históricos preservados no arquivo

Os blocos `smOBJ3D` contêm valores 32-bit históricos como:

```text
Vertex
Face
TexLink
Physique
```

Exemplo:

```text
Vertex=0x21F17770
Face=0x21F18368
TexLink=0x21F1A520
Physique=0x20EB7AA0
```

Esses valores não são endereços utilizáveis pelo Python.

Eles servem como metadados relativos para a reconstrução executada pelo loader,
especialmente:

```text
Face.lpTexLink
smTEXLINK.NextTex
```

A base serializada `smOBJ3D.TexLink` permite converter esses ponteiros antigos
em índices no array `smTEXLINK`.

---

# 189. smFACE.t[] não contém UV útil neste asset

O probe V1 imprimiu repetidamente em `smFACE.t`:

```text
-4.31602e+08
```

Esse valor IEEE-754 corresponde ao padrão:

```text
0xCDCDCDCD
```

repetido.

Isso é consistente com bytes de memória não inicializada de toolchains/runtime
Windows antigos.

Portanto:

```text
smFACE.t[] NÃO deve ser usado como UV da MfbC01.smd.
```

O UV útil deve ser recuperado de:

```text
smTEXLINK.u[3]
smTEXLINK.v[3]
```

através do índice reconstruído a partir de:

```text
Face.lpTexLink
smOBJ3D.TexLink
```

Estado:

```text
smFACE.t como UV: descartado para este asset
smTEXLINK como fonte de UV: próximo gate
```

---

# 190. Próximo gate antes do reader Blender

Antes de escrever o SMD -> Blender, auditar apenas a semântica de:

```text
smOBJ3D::AddFace
smOBJ3D::SetFaceMaterial
smOBJ3D::AddTexLink
smRENDER3D::AddObjectFace
```

Objetivo:

```text
provar significado de Face.v[3];
provar como lpTexLink seleciona o primeiro smTEXLINK;
provar como NextTex encadeia layers;
provar como hTexture/material é resolvido.
```

Depois disso o `pt_smd_probe_v2.py` poderá extrair:

```text
triângulos reais
material id
UV
bone name rígido por vértice
```

sem inferência.
---

# 191. Semântica de smFACE e smTEXLINK — PROVADA pela source

`smOBJ3D::AddFace(a,b,c)` grava:

```text
Face.v[0] = a
Face.v[1] = b
Face.v[2] = c
Face.v[3] = 0
Face.lpTexLink = NULL
```

`SetFaceMaterial(FaceNum, MatNum)` grava:

```text
Face[FaceNum].v[3] = MatNum
```

`smRENDER3D::AddObjectFace()` confirma:

```text
face.v[0..2] -> RendVertex[]
face.v[3]    -> RendFace.Matrial
face.lpTexLink -> RendFace.lpTexLink
```

Portanto:

```text
v[0..2] = índices de vértices: PROVADO
v[3]    = material ID:          PROVADO
```

---

# 192. UV real e cadeia de layers — PROVADOS

`smOBJ3D::AddTexLink()` grava em `smTEXLINK`:

```text
u[0..2]
v[0..2]
hTexture
NextTex = NULL
```

Se a face ainda não possui textura:

```text
Face.lpTexLink = &TexLink[nTexLink]
```

Se já possui:

```text
percorre NextTex
anexa novo TexLink ao final
```

A busca de anexação percorre no máximo 8 iterações.

Logo:

```text
Face.lpTexLink = primeira layer
TexLink.NextTex = próxima layer
TexLink.u/v = UV dos 3 corners da face
```

Estado:

```text
semântica de UV via smTEXLINK: PROVADA
smFACE.t como fonte de UV:     não necessário neste caminho
```

---

# 193. pt_smd_probe_v2.py preparado

O segundo probe, somente leitura, agora:

```text
reconstrói Face.lpTexLink -> índice de smTEXLINK;
reconstrói NextTex;
extrai UV por layer;
valida índices de vértices;
valida material ID contra MatCounter;
conta layers por face;
detecta ponteiro inválido/loop;
lê o bone name rígido de cada vértice;
gera JSON opcional.
```

Ainda não tenta resolver `hTexture` para filename/material real.

Esse elo depende da serialização de `smMATERIAL_GROUP`, que será auditada
separadamente depois da validação do V2.
---

# 194. MfbC01.smd — geometria/UV/material ID/Physique semântico PROVADO

O `pt_smd_probe_v2.py` foi executado contra o `MfbC01.smd` real.

Resumo:

```text
objects=15
vertices=1444
faces=2537
texlinks=2537
physique_assignments=1444
semantic_check=PASS
probe_exit=0
```

Para todos os objetos:

```text
invalid_vertex_faces=0
invalid_material_faces=0
invalid_face_tex_ptrs=0
invalid_next_ptrs=0
loops=0
null_face_tex=0
unreferenced_texlinks=0
multi_referenced_texlinks=0
object_check=PASS
```

Estado:

```text
triângulos do MfbC01.smd:          PROVADO
material ID por face:              PROVADO
UV real via smTEXLINK:             PROVADO
Face.lpTexLink reconstruído:       PROVADO
NextTex reconstruído:              PROVADO
bone rígido por vértice:           PROVADO
```

---

# 195. Uma TexLink layer por face — específico de MfbC01.smd

Todos os 15 objetos mostraram:

```text
texlink_chain_hist={1: nFace}
max_layers=1
```

e globalmente:

```text
nTexLink total == nFace total == 2537
```

Portanto, para este asset:

```text
cada face possui exatamente 1 smTEXLINK
```

Isso NÃO deve ser generalizado como regra do formato SMD, porque a source suporta
encadeamento via `NextTex`.

---

# 196. Distribuição global de material IDs

Somando os `material_hist` dos 15 objetos:

```text
material 0:    0 faces
material 1: 1344 faces
material 2:   62 faces
material 3:  941 faces
material 4:    0 faces
material 5:  190 faces
```

Total:

```text
1344 + 62 + 941 + 190 = 2537
```

`MatCounter=6`, portanto os IDs válidos são:

```text
0..5
```

No `MfbC01.smd`, apenas:

```text
1, 2, 3, 5
```

são efetivamente referenciados por faces.

---

# 197. UVs plausíveis confirmados

Exemplos reais extraídos do `smTEXLINK`:

```text
(0.44, 0.30)
(0.54, 0.35)
(0.56, 0.30)

(0.70, 0.29)
(0.76, 0.31)
(0.76, 0.23)
```

Esses valores substituem definitivamente os bytes `0xCDCDCDCD` observados em
`smFACE.t`.

---

# 198. Physique rígido real confirmado

Os 1444 vértices possuem um nome de bone válido no bloco Physique.

Exemplos:

```text
Bip01 Spine1
Bip01 L UpperArm
Bip01 R UpperArm
Bip01 L Forearm
Bip01 R Forearm
Bip01 Pelvis
Bip01 L Calf
Bip01 R Calf
Bip01 L Foot
Bip01 R Foot
```

Também existem nomes auxiliares como:

```text
Bip-0037
Bip-0019
```

Portanto o importer deve preservar os nomes literalmente e não limitar a uma
lista pré-definida de bones Biped.

---

# 199. Próximo gate — serialização de smMATERIAL_GROUP

A única peça grande ainda faltando para um primeiro SMD -> Blender visual é:

```text
material ID
    ->
smMATERIAL_GROUP
    ->
texture filename
blend/material flags
```

Próxima auditoria deve localizar e inspecionar somente:

```text
smMATERIAL_GROUP::GetSaveSize
smMATERIAL_GROUP::SaveFile
smMATERIAL_GROUP::LoadFile
definições de smMATERIAL / smTEXTURE
```

Objetivo imediato:

```text
decodificar os 2153 bytes da região de material de MfbC01.smd
```

sem inferir `hTexture` como endereço runtime.
---

# 200. Serialização de smMATERIAL_GROUP — estrutura macro PROVADA

A source confirma que `smMATERIAL_GROUP::SaveFile()` grava primeiro o dump cru
da própria classe:

```cpp
WriteFile(hFile, &Head, sizeof(smMATERIAL_GROUP), ...)
```

e para cada material:

```cpp
WriteFile(hFile, &smMaterial[cnt], sizeof(smMATERIAL), ...)
```

Se `smMaterial[cnt].InUse` for verdadeiro, grava depois:

```text
int len
char string_block[len]
```

O `string_block` concatena, em ordem:

```text
Name\0
NameA\0
```

para cada `TextureCounter`, seguido de:

```text
Name\0
NameA\0
```

para cada `AnimTexCounter`.

O loader lê exatamente a mesma sequência e reconstrói os ponteiros através de
`smTexture.Add()`.

Portanto os ponteiros existentes dentro do dump cru de `smMATERIAL` são valores
históricos; os filenames confiáveis estão no bloco de strings que segue cada
material em uso.

---

# 201. Estrutura smMATERIAL identificada

A definição auditada contém:

```text
InUse
TextureCounter
smTexture[8]
TextureStageState[8]
TextureFormState[8]
ReformTexture
MapOpacity
TextureType
BlendType
Shade
TwoSide
SerialNum
Diffuse
Transparency
SelfIllum
TextureSwap
MatFrame
TextureClip
UseState
MeshState
WindMeshBottom
smAnimTexture[32]
AnimTexCounter
FrameMask
Shift_FrameSpeed
AnimationFrame
```

`smMATERIAL_GROUP` contém:

```text
Head
smMaterial*
MaterialCount
ReformTexture
MaxMaterial
LastSearchMaterial
szLastSearchName[64]
```

Como ambos são gravados por `sizeof(...)`, o próximo gate deve medir seus
tamanhos e offsets no ABI MinGW i686 real antes de decodificar os 2153 bytes da
região de materiais.

---

# 202. Material ABI probe V1 preparado

Foi preparado:

```text
pt_smd_material_abi_probe_v1.cpp
```

Ele mede:

```text
sizeof(smFCOLOR)
sizeof(smTEXTUREHANDLE)
sizeof(smMATERIAL)
sizeof(smMATERIAL_GROUP)
```

e os offsets de todos os campos relevantes de `smMATERIAL` e
`smMATERIAL_GROUP`.

Estado:

```text
serialização macro de materiais: PROVADA
layout ABI material:             PENDENTE
filenames das 6 texturas:        PENDENTE
```

Depois desse probe será possível escrever `pt_smd_probe_v3.py` para consumir
exatamente os 2153 bytes de material de `MfbC01.smd`.
---

# 203. ABI de materiais MinGW i686 — PROVADO

O probe compilado como PE32/i386 confirmou:

```text
sizeof(void*)=4
sizeof(smFCOLOR)=12
sizeof(smTEXTUREHANDLE)=176
sizeof(smMATERIAL)=320
sizeof(smMATERIAL_GROUP)=88
```

Offsets de `smMATERIAL_GROUP`:

```text
smMaterial          4
MaterialCount       8
ReformTexture      12
MaxMaterial        16
LastSearchMaterial 20
szLastSearchName   24
```

Offsets principais de `smMATERIAL`:

```text
InUse               0
TextureCounter      4
smTexture           8
TextureStageState  40
TextureFormState   72
ReformTexture     104
MapOpacity        108
TextureType       112
BlendType         116
Shade             120
TwoSide           124
SerialNum         128
Diffuse           132
Transparency      144
SelfIllum         148
TextureSwap       152
MatFrame          156
TextureClip       160
UseState          164
MeshState         168
WindMeshBottom    172
smAnimTexture     176
AnimTexCounter    304
FrameMask         308
Shift_FrameSpeed  312
AnimationFrame    316
```

Estado:

```text
layout ABI de smMATERIAL_GROUP: PROVADO
layout ABI de smMATERIAL:       PROVADO
```

---

# 204. Região de materiais MfbC01 — conta fixa

Para `MfbC01.smd`:

```text
MatFilePoint       = 1156
First_ObjInfoPoint = 3309
material region    = 2153 bytes
MatCounter         = 6
```

Parte fixa confirmada pelo ABI:

```text
88 + 6 * 320 = 2008 bytes
```

Logo:

```text
2153 - 2008 = 145 bytes
```

são variáveis.

Pela source, esses 145 bytes só podem ser compostos por:

```text
int StrLen
+
Name\0 / NameA\0 strings
```

dos materiais com `InUse != 0`.

O número exato de materiais `InUse` ainda deve ser lido do arquivo; não inferir
apenas pelos material IDs referenciados por faces.

---

# 205. pt_smd_probe_v3.py preparado

O V3 é somente leitura e decodifica:

```text
smMATERIAL_GROUP
cada smMATERIAL
InUse
TextureCounter
AnimTexCounter
TextureType
BlendType
Shade
TwoSide
Transparency
SelfIllum
MapOpacity
TextureStageState
TextureFormState
Name / NameA
```

Também valida:

```text
MaterialCount do grupo == MatCounter do SMD;
TextureCounter <= 8;
AnimTexCounter <= 32;
cada string block consome exatamente StrLen;
cursor final == First_ObjInfoPoint.
```

`smFCOLOR` foi medido com 12 bytes, mas sua definição de membros ainda não foi
incluída nessa prova. O V3 preserva os 12 bytes raw e mostra uma visão de 3
floats apenas como diagnóstico, sem tratá-la ainda como semântica provada.

O V3 não usa os ponteiros históricos `smTexture[]` para descobrir nomes;
filenames são obtidos exclusivamente dos string blocks serializados pela source.
---

# 206. MfbC01.smd — região de materiais decodificada por completo

`pt_smd_probe_v3.py` fechou exatamente:

```text
material_cursor=3309
First_ObjInfoPoint=3309
remaining_bytes=0
material_region_end_check=OK
material_semantic_check=PASS
probe_exit=0
```

Portanto os 2153 bytes entre `MatFilePoint` e `First_ObjInfoPoint` foram
consumidos integralmente pela estrutura prevista na source:

```text
smMATERIAL_GROUP
6 x smMATERIAL
StrLen + string blocks dos materiais InUse
```

Estado:

```text
layout da região de materiais de MfbC01.smd: PROVADO
filenames serializados de textura:           PROVADO
```

---

# 207. Materiais reais de MfbC01.smd

O grupo contém seis materiais.

```text
ID 0
InUse=57
TextureCounter=0
BlendType=1 / SMMAT_BLEND_ALPHA
Shade=1
TwoSide=0
sem filename de textura

ID 1
InUse=1991
TextureCounter=1
BlendType=1 / SMMAT_BLEND_ALPHA
Shade=1
TwoSide=0
texture = char\tmABCD\MfbC01.bmp

ID 2
InUse=119
TextureCounter=1
BlendType=1 / SMMAT_BLEND_ALPHA
Shade=1
TwoSide=1
texture = char\tmABCD\MfbC01.bmp

ID 3
InUse=998
TextureCounter=1
BlendType=1 / SMMAT_BLEND_ALPHA
Shade=1
TwoSide=0
texture = char\tmABCD\MfbC03.bmp

ID 4
InUse=57
TextureCounter=1
BlendType=1 / SMMAT_BLEND_ALPHA
Shade=1
TwoSide=1
texture = char\tmABCD\MfbC03.bmp

ID 5
InUse=247
TextureCounter=1
BlendType=1 / SMMAT_BLEND_ALPHA
Shade=1
TwoSide=1
texture = char\tmABCD\mo-p-ac.BMP
```

Todos:

```text
TextureType=SMTEX_TYPE_MULTIMIX
AnimTexCounter=0
Transparency=0
SelfIllum=0
MapOpacity=0
```

---

# 208. Material IDs usados pelas faces versus materiais serializados

Do probe V2:

```text
faces usam IDs 1, 2, 3 e 5
```

Os IDs:

```text
0 e 4
```

não são referenciados pelas 2537 faces atuais, apesar de `InUse` ser não-zero.

Logo `smMATERIAL.InUse` não deve ser interpretado como:

```text
"este material aparece em alguma face atual"
```

nem como booleano literal `0/1`.

Para serialização, a única semântica necessária e provada é:

```text
InUse != 0 -> SaveFile/LoadFile inclui StrLen/string block
```

Os valores devem ser preservados como dados legados/count-like, sem normalizar.

---

# 209. Bytes CDCD preservados no material group

O dump real mostrou:

```text
smMATERIAL_GROUP.Head = 0xCDCDCDCD
LastSearchMaterial     = 0xCDCDCDCD interpretado como signed
szLastSearchName       = bytes 0xCD repetidos
```

Isso confirma novamente que o formato serializa memória bruta contendo campos
de runtime/estado que podem permanecer não inicializados.

Esses campos não devem participar do importer visual.

---

# 210. Próximo gate: escala/coordenadas ASE -> SMD

Antes do primeiro importer SMD -> Blender, é necessário provar a transformação
de posição usada quando a engine lê ASE e constrói `smVERTEX`.

Precisamos auditar:

```text
smOBJ3D::AddVertex
*MESH_VERTEX dentro do reader de GEOMOBJECT de modelo
fONE / FLOATNS
```

Objetivo:

```text
descobrir exatamente como:
ASE float XYZ -> smVERTEX int XYZ
```

e portanto definir a transformação inversa:

```text
SMD int XYZ -> Blender float XYZ
```

Também auditar o trecho exato que copia `*MESH_TVERT` para `smTEXLINK`, para
confirmar se o UV serializado sofre offset/reform antes do SaveFile.

Nenhum importer visual deve fixar escala/axis conversion no chute.
---

# 211. Coordenadas de modelo ASE -> SMD — PROVADAS

No reader de modelo:

```cpp
x = (int)(atof(strBuff) * fONE);
y = (int)(atof(strBuff) * fONE);
z = (int)(atof(strBuff) * fONE);
obj->AddVertex(x, y, z);
```

e:

```text
fONE=256
FLOATNS=8
```

`smOBJ3D::AddVertex` copia `x/y/z` diretamente para `smVERTEX`.

Logo:

```text
SMD_X = trunc(ASE_X * 256)
SMD_Y = trunc(ASE_Y * 256)
SMD_Z = trunc(ASE_Z * 256)
```

A reconstrução estática natural é:

```text
Blender_X = SMD_X / 256
Blender_Y = SMD_Y / 256
Blender_Z = SMD_Z / 256
```

Não há troca Y/Z no reader de modelo.

A troca observada em outra função pertence ao reader de `smSTAGE3D`, não ao
`smOBJ3D` de modelos.

---

# 212. UV ASE -> smTEXLINK — transformação PROVADA

Antes de `obj->AddTexLink`, o reader de modelo:

```text
centraliza UV em 0.5;
aplica rotação UVW;
subtrai UVW offset;
aplica UVW tiling;
retorna ao centro 0.5;
grava U;
grava V como 1 - fv.
```

Portanto o SMD contém UV de runtime já transformado.

Para o Blender visual:

```text
Blender_U = SMD_U
Blender_V = 1 - SMD_V
```

Essa conversão apenas restaura a convenção vertical usada pelo caminho ASE /
Blender; não tenta recuperar os TVERT pré-transformação quando offset/tiling/
rotação eram diferentes.

---

# 213. Primeiro importer direto SMD -> Blender preparado

Arquivo:

```text
pt_smd_blender_import_v1.py
```

Escopo V1:

```text
SMD 0.62 somente leitura;
15 smOBJ3D;
XYZ / 256;
faces;
material ID;
UV do primeiro smTEXLINK;
V runtime -> Blender;
6 smMATERIAL;
filenames de textura serializados;
decode de BMP Priston com prefixo custom de 14 bytes em cache externo;
vertex groups rígidos por Physique, peso 1.0;
save .blend.
```

Não altera nenhum arquivo do cliente.

Não implementa ainda:

```text
M5Bip / armature runtime;
animação;
avaliação completa de Tm/TmInvert/TmResult;
render multi-layer para SMDs que tenham NextTex > 1.
```

Para `MfbC01.smd`, o limite de primeira layer não perde informação, pois foi
provado que todas as 2537 faces possuem exatamente uma layer.

O primeiro teste visual deve produzir:

```text
15 objects
6 materials
1444 vertices
2537 faces
2537 UV faces
```

e permitir comparação visual com a base Priestess já conhecida.
---

# 214. Importer SMD -> Blender V1 executado com sucesso — PROVADO

O comando Blender 5.1.1 em background executou:

```text
pt_smd_blender_import_v1.py
```

diretamente sobre:

```text
char/tmABCD/MfbC01.smd
```

Resultado:

```text
Objects    : 15
Materials  : 6
vertices   = 1444
faces      = 2537
uv_faces   = 2537
import_check=PASS
blender_exit=0
```

Arquivo produzido:

```text
MfbC01_direct_smd_v1.blend
```

Tamanho observado:

```text
563K
```

Estado:

```text
parse SMD -> Blender datablocks: PROVADO
save .blend:                    PROVADO
correção visual final:          PENDENTE
```

---

# 215. Texturas do SMD carregadas no Blender — PROVADO

O importer resolveu e carregou:

```text
char\tmABCD\MfbC01.bmp
char\tmABCD\MfbC03.bmp
char\tmABCD\mo-p-ac.BMP
```

Os dois BMP Priston protegidos foram convertidos somente no cache externo:

```text
_pt_smd_texture_cache/
```

como:

```text
BMP_PRISTON_PREFIX14_256x256_24bpp
```

e o Blender reportou:

```text
load=LOADED
```

para materiais 1, 2, 3, 4 e 5.

`mo-p-ac.BMP` já era BMP standard e foi carregado diretamente.

Nenhum asset original do cliente foi modificado.

---

# 216. Physique -> vertex groups executado

Cada smOBJ3D criou grupos rígidos de vértice.

Exemplos observados:

```text
MsmC01 groups=22
MshC01 groups=22
MslC01 groups=17
MlhC01 groups=4
MbhC01 groups=21
...
```

Isso prova a criação dos vertex groups a partir dos nomes Physique.

Ainda NÃO prova deformação correta por armature, porque o V1 não cria nem liga
o esqueleto `M5Bip`.

---

# 217. Warnings do Blender não bloquearam o teste

Foram observados:

```text
OpenColorIO config 2.5 vs library 2.4.2
fallback color management
```

e:

```text
DeprecationWarning: Material.use_nodes
```

Nenhum deles abortou a importação.

O Blender continuou até:

```text
Info: Saved as "MfbC01_direct_smd_v1.blend"
import_check=PASS
```

Portanto são warnings não bloqueantes para este teste.

---

# 218. Próximo gate — inspeção visual estática

O V1 ainda não aplica:

```text
Tm
TmInvert
TmResult
TmRotate
mWorld
mLocal
```

nem cria o armature runtime.

Logo o próximo teste não deve alterar código.

Abrir somente:

```text
MfbC01_direct_smd_v1.blend
```

e verificar:

```text
1. silhueta corporal reconhecível;
2. 15 partes alinhadas, não explodidas/separadas;
3. orientação vertical/correta;
4. textura não invertida verticalmente;
5. ausência de grandes regiões magenta/sem textura;
6. ausência de faces evidentemente invertidas/desaparecidas.
```

Se a geometria estiver desmontada, o próximo gate será `smOBJ3D` transforms.

Se a geometria estiver montada mas UV estiver invertido, corrigir apenas a
convenção V.

Se ambas estiverem corretas, o próximo gate será o skeleton/link `M5Bip`.
---

# 219. Primeiro teste visual do importer direto — FALHOU

O `.blend` produzido pelo V1 abriu, porém a Priestess:

```text
não está reconhecível
```

Portanto:

```text
parse estrutural SMD:         PROVADO
geometria/topologia extraída: PROVADO
materiais/texturas extraídos: PROVADO
UV extraído:                  PROVADO
montagem espacial estática:   FALHOU / PENDENTE
```

Não alterar novamente:

```text
escala /256;
ordem XYZ do reader de modelo;
material IDs;
decodificação BMP;
UV
```

até existir evidência de que alguma dessas partes está errada.

---

# 220. Hipótese principal: Vertex[] local versus smWorldVertex

A engine renderiza objetos usando:

```text
smWorldVertex
```

em `BeginRender()`, e não diretamente o array serializado:

```text
Vertex[]
```

O importer V1 atualmente envia `Vertex[] / 256` diretamente ao Blender.

Logo a próxima investigação deve fechar:

```text
Vertex[]
  ->
WorldForm / TmAnimation
  ->
Tm / TmInvert / TmResult / pParent
  ->
smWorldVertex[]
```

A hierarquia de `NodeParent/pParent` também pode participar.

Estado:

```text
causa exata: PENDENTE
transform aplicada pela engine: próximo gate
```

---

# 221. Próximo gate de source

Auditar somente:

```text
smOBJ3D::WorldForm
smOBJ3D::TmAnimation
smOBJ3D::ReformTM
smPAT3D::ReformTM
smPAT3D::LinkObject
```

e ocorrências de:

```text
smWorldVertex
TmResult
TmInvert
pParent
```

dentro de `smObj3d.cpp`.

Nenhum novo importer deve ser gerado antes dessa auditoria.
---

# 222. Causa da geometria irreconhecível — Physique/bone-local PROVADO pela source

A falha visual do importer V1 agora tem explicação direta na source.

Quando um modelo ASE é carregado com `smPhysique` ativo, `smASE_Read()` faz:

```cpp
pat->TmParent = smPhysique;

if(smPhysique)
    pat->ReformPhysique();
else {
    pat->LinkObject();
    pat->ReformTM();
}

pat->SaveFile(...smd);
```

Logo um character mesh com Physique não é salvo mantendo simplesmente as
coordenadas globais ASE.

`smOBJ3D::ReformPhysique()` faz, para cada vértice:

```text
pBone = Physique[cnt]
q = inverse(pBone->Tm)

Vertex = Vertex * q
```

ou seja, converte o vértice para o espaço local do bone antes de salvar o SMD.

Estado:

```text
MfbC01.smd contém Physique em todos os 1444 vértices: PROVADO
caminho ASE com Physique chama ReformPhysique antes de SaveFile: PROVADO
ReformPhysique aplica inverse(bone.Tm) ao vértice: PROVADO
```

Portanto o array `Vertex[]` do SMD de character NÃO representa por si só a
silhueta montada em world/bind pose.

---

# 223. Runtime desfaz o bone-local em WorldForm

Quando `Physique` existe, `smOBJ3D::WorldForm()` não usa `obj->mWorld`.

Para cada vértice:

```cpp
mMat = &Physique[cnt]->mWorld;
```

e calcula:

```text
rx = x*_11 + y*_21 + z*_31
ry = x*_12 + y*_22 + z*_32
rz = x*_13 + y*_23 + z*_33

smWorldVertex.x = (rx >> 8) + _41
smWorldVertex.z = (ry >> 8) + _42
smWorldVertex.y = (rz >> 8) + _43
```

Depois `BeginRender()` envia `smWorldVertex[]` para o renderer.

Portanto o caminho real é:

```text
ASE world/bind vertex
    ↓
inverse(bone.Tm)
    ↓
SMD Vertex[] bone-local
    ↓
bone.mWorld runtime
    ↓
smWorldVertex[]
    ↓
renderer
```

O importer V1 pulou a etapa `bone.mWorld`, explicando a silhueta irreconhecível.

---

# 224. Companion bone format é SMB

A source define:

```text
szFileModelBip = "smb"
szFileModel    = "smd"
```

`smASE_ReadBone(file)` procura primeiro:

```text
ChangeFileExt(file, "smb")
```

e carrega com `smReadModel_Bip()`.

Quando precisa gerar o bone cache a partir de ASE, ele:

```text
LinkObject()
ReformTM()
SaveFile(...smb)
```

usando o mesmo `smPAT3D::SaveFile()` utilizado pelo SMD.

Logo `.smb` é o companion binário que contém os objetos/bones e suas matrizes /
animações necessárias para recolocar os vértices Physique no espaço correto.

Para o base Priestess, o logical link já identificado é:

```text
char\tmABCD\M5Bip.in
```

e a resolução de bone troca a extensão para:

```text
char\tmABCD\M5Bip.smb
```

A existência e estrutura do `M5Bip.smb` real ainda devem ser confirmadas no
client antes de escrever o importer V2.

---

# 225. Correção de direção do importer

Não corrigir visualmente no chute:

```text
XYZ
/256
UV
material
```

A próxima versão precisa combinar:

```text
MfbC01.smd mesh
+
M5Bip.smb bones
+
Physique bone name por vértice
+
bone matrices
=
bind/static mesh reconhecível
```

Próximo gate:

```text
localizar M5Bip.smb real;
validar header/layout com o probe estrutural já existente.
```

Só depois extrair as matrizes do SMB.
---

# 226. M5Bip físico no client — somente INX encontrado

Busca case-insensitive em todo o client encontrou apenas:

```text
char/tmABCD/M5Bip.inx
size = 95268 bytes
```

Não foram encontrados:

```text
M5Bip.smb
M5Bip.smd
M5Bip.ase
M5Bip.in
```

nem variantes de nome contendo `M5` + `Bip`.

Estado:

```text
M5Bip.smb no client: NÃO EXISTE
M5Bip.in físico:     NÃO EXISTE
M5Bip.inx físico:    PROVADO
```

Portanto `M5Bip.in` deve ser tratado, por enquanto, apenas como nome lógico
serializado/decodificado. Não inferir extensão física final sem seguir o decoder.

---

# 227. Próximo gate — relação M5Bip.inx versus mC001.inx

`M5Bip.inx` possui exatamente:

```text
95268 bytes
```

o mesmo tamanho anteriormente observado para:

```text
mC001.inx
```

Isso é apenas uma coincidência de tamanho até prova adicional.

O próximo teste deve ser somente:

```text
hash dos dois arquivos;
cmp byte a byte;
primeiros bytes;
strings legíveis de M5Bip.inx.
```

Objetivo:

```text
determinar se M5Bip.inx é:
1. byte-identical a mC001.inx;
2. estrutura do mesmo tipo mas conteúdo diferente;
3. arquivo que já carrega dentro dele a informação de motion/bone.
```

Não executar SMD/SMB parser sobre INX.
---

# 228. M5Bip.inx e mC001.inx — mesmo tamanho, conteúdo diferente

Arquivos:

```text
M5Bip.inx  95268 bytes
mC001.inx  95268 bytes
```

Hashes:

```text
M5Bip.inx:
3d096b1572386ca856d5e51cbad0b14249320a6778acd22913f0c34653500790

mC001.inx:
7e5fba8f59f3ee3f373673b87ec6a65f8195e46aa16bc6a57be49b6b1dcf0c1f
```

`cmp`:

```text
DIFFERENT
```

Portanto `M5Bip.inx` não é alias/cópia byte-identical de `mC001.inx`.

---

# 229. INX é dump binário de smMODELINFO — PROVADO pela source/runtime

A source de geração do cache executa:

```cpp
fwrite(Info, sizeof(smMODELINFO), 1, fp);
```

para o arquivo com extensão INX.

Na leitura, `smModelDecode()` somente entra no caminho binário direto quando:

```cpp
dwFileLen == sizeof(smMODELINFO)
```

e então executa:

```cpp
fread(&smModelInfo, sizeof(smMODELINFO), 1, fp);
```

Os arquivos observados têm 95268 bytes e foram aceitos por esse caminho no
runtime trace anterior.

Logo:

```text
M5Bip.inx = smMODELINFO serializado
mC001.inx = smMODELINFO serializado
sizeof(smMODELINFO) efetivo do cache = 95268 bytes
```

para os arquivos/runtime auditados.

---

# 230. Primeiro campo de M5Bip.inx

A definição de `smMODELINFO` começa com:

```cpp
char szModelFile[64];
```

Os primeiros 64 bytes de ambos os INX são:

```text
char\tmABCD\MfbC01.ASE
```

Logo:

```text
M5Bip.inx.szModelFile = char\tmABCD\MfbC01.ASE
mC001.inx.szModelFile = char\tmABCD\MfbC01.ASE
```

PROVADO.

---

# 231. Offset 0x40 contém m5.ase — nome do campo ainda PENDENTE

Em `M5Bip.inx`, bytes `0x40..` contêm:

```text
char\tmABCD\m5.ase
```

No `mC001.inx`, a mesma região é zero.

Ainda NÃO nomear esse campo como `szLinkFile`.

A definição conhecida mostra `szLinkFile` somente várias linhas depois de
`szModelFile`, e existe também `szSubModelFile` usado pelo loader.

Portanto o próximo gate deve imprimir a definição completa de `smMODELINFO`
para mapear o offset `0x40` sem inferência.

---

# 232. String M5Bip.in aparece dentro do INX

`strings` encontrou:

```text
char\tmABCD\M5Bip.in
```

dentro de `M5Bip.inx`.

A source de `smModelDecode()` possui o fluxo:

```text
case 0 -> iniFile
case 1 -> Info->szLinkFile
case 2 -> Info->szTalkLinkFile

smFindFile(szFile, "inx")
```

e, quando o INX carregado é um link motion, copia para o `Info` principal:

```text
szMotionFile
MotionInfo[]
MotionCount
NpcMotionRate[]
```

Isso reforça que `M5Bip.inx` participa da cadeia de motion/link do personagem,
mas o offset exato de `M5Bip.in` no `smMODELINFO` ainda precisa ser medido/
identificado antes de chamá-lo definitivamente de `szLinkFile`.

---

# 233. Próximo gate

Imprimir somente a definição completa:

```text
smMODELINFO
```

em `smType.h`, incluindo todos os campos desde `szModelFile` até após
`szLinkFile/szTalkLinkFile`.

Objetivo imediato:

```text
mapear offset 0x00
mapear offset 0x40
mapear localização de szLinkFile
mapear localização de szMotionFile
```

Depois disso será seguro seguir `m5.ase` ou `M5Bip.in` conforme o papel real de
cada campo.
---

# 234. Offset 0x40 de M5Bip.inx identificado — szMotionFile PROVADO

A definição completa de `smMODELINFO` começa:

```cpp
char szModelFile[64];
char szMotionFile[64];
char szSubModelFile[64];
```

Portanto:

```text
offset 0x0000 = szModelFile
offset 0x0040 = szMotionFile
offset 0x0080 = szSubModelFile
```

No `M5Bip.inx` real:

```text
szModelFile  = char\tmABCD\MfbC01.ASE
szMotionFile = char\tmABCD\m5.ase
```

Estado:

```text
M5Bip.inx offset 0x40 = szMotionFile: PROVADO
m5.ase é o logical motion/bip asset:   PROVADO
```

---

# 235. LoadCharactor usa szMotionFile como Bip/Physique

A source de `smPATTERN::LoadCharactor()` executa:

```cpp
if(BipPattern && modelInfo.szMotionFile[0])
{
    smDPAT *dMotionPat =
        BipPattern->LoadBipPattern(modelInfo.szMotionFile);

    if(dMotionPat)
    {
        smASE_SetPhysique(dMotionPat->Pat);
        DinaPat[Num].smDinaLink = dMotionPat;
    }
}
```

Depois carrega:

```cpp
DinaPat[Num].Pat = smASE_Read(modelInfo.szModelFile);
```

Logo a sequência real é:

```text
resolved smMODELINFO
    |
    +-- szMotionFile = char\tmABCD\m5.ase
    |       |
    |       +--> BipPattern->LoadBipPattern(...)
    |                |
    |                +--> smASE_SetPhysique(bip pattern)
    |
    +-- szModelFile = char\tmABCD\MfbC01.ASE
            |
            +--> smASE_Read(...)
```

Isso explica como `ReformPhysique`/`WorldForm` encontra os bones correspondentes
aos nomes Physique gravados no `MfbC01.smd`.

---

# 236. Papel de M5Bip.inx na cadeia

`M5Bip.inx` não deve ser tratado como o próprio skeleton binário.

Ele é um `smMODELINFO` cache que fornece, entre outros campos:

```text
szModelFile  = MfbC01.ASE
szMotionFile = m5.ase
```

A string:

```text
char\tmABCD\M5Bip.in
```

também existe dentro do INX, mas o seu campo exato ainda não foi mapeado.

Isso não bloqueia mais a busca do skeleton, porque o campo efetivamente usado
por `LoadCharactor` para o BipPattern já está identificado: `szMotionFile`.

---

# 237. Próximo gate

Localizar somente os arquivos físicos correspondentes a:

```text
char\tmABCD\m5.ase
```

incluindo variantes:

```text
m5.ase
m5.smb
m5.*
```

Não executar parser antes de saber quais arquivos realmente existem.
---

# 238. m5.smb físico localizado

Busca no client encontrou:

```text
char/tmABCD/m5.smb
size = 13931704 bytes
```

e sete blocos adicionais:

```text
M5-motion1.smb  2654612 bytes
M5-motion2.smb  1885044 bytes
M5-motion3.smb  2805584 bytes
M5-motion4.smb  3095768 bytes
M5-motion5.smb  3567288 bytes
M5-motion6.smb  1664456 bytes
M5-motion7.smb  3511084 bytes
```

Também continua presente:

```text
M5Bip.inx = 95268 bytes
```

---

# 239. Cadeia física agora fortemente sustentada

Já estava provado no `M5Bip.inx`:

```text
szMotionFile = char\tmABCD\m5.ase
```

e a source de bone usa a extensão cache:

```text
smb
```

para `smASE_ReadBone`.

A presença física de:

```text
char/tmABCD/m5.smb
```

é exatamente compatível com:

```text
m5.ase logical
    ->
ChangeFileExt(..., "smb")
    ->
m5.smb physical
```

Ainda falta o probe estrutural do arquivo real antes de marcar o conteúdo do
`m5.smb` como decodificado/provado.

Os arquivos `M5-motion1..7.smb` devem ser tratados como extensões/blocos de
motion até a source/INX demonstrar o papel exato de cada um.

---

# 240. Próximo gate

Validar somente:

```text
char/tmABCD/m5.smb
```

com:

```text
header;
SHA256;
pt_smd_probe_v1.py.
```

Objetivos:

```text
confirmar header SMD Model data Ver 0.62;
confirmar ObjCounter;
confirmar MatCounter;
confirmar offsets;
confirmar cada smOBJ3D e seu tamanho;
confirmar fechamento exato do arquivo.
```

Não analisar ainda `M5-motion1..7.smb`.
---

# 241. m5.smb — container e estrutura global PROVADOS

O `pt_smd_probe_v1.py` executado sobre:

```text
char/tmABCD/m5.smb
```

retornou:

```text
file_size=13931704
header='SMD Model data Ver 0.62'
header_check=OK
ObjCounter=95
MatCounter=0
MatFilePoint=4356
First_ObjInfoPoint=4356
TmFrameCounter=7
material_region_size=0
MatFilePoint_check=OK
```

Todos os objetos auditados fecham com:

```text
NodeName_check=OK
Length_check=OK
bounds_check=OK
```

e o final do arquivo:

```text
objects=95
last_calculated_end=13931704
file_size=13931704
structural_check=PASS
probe_exit=0
```

Estado:

```text
m5.smb usa container SMD 0.62:       PROVADO
95 objetos/bones/auxiliares:          PROVADO
sem material/mesh geometry:           PROVADO para os counts observados
7 blocos TmFrame:                     PROVADO
serialização macro smOBJ3D:           PROVADA
```

---

# 242. Hierarquia de bone presente no m5.smb

Exemplos diretamente do arquivo:

```text
Bip01                    parent=''
Bip01 Footsteps          parent='Bip01'
Bip01 Pelvis             parent='Bip01'
Bip01 Spine              parent='Bip01 Pelvis'
Bip01 Spine1             parent='Bip01 Spine'
Bip01 Neck               parent='Bip01 Spine1'
Bip01 Head               parent='Bip01 Neck'
Bip01 L Clavicle         parent='Bip01 Neck'
Bip01 L UpperArm         parent='Bip01 L Clavicle'
Bip01 L Forearm          parent='Bip01 L UpperArm'
...
```

Também existem bones auxiliares como:

```text
Bip-0037
Bip weapon01
Bip in01
Bip in-bow
```

Não converter ou renomear esses nodes.

---

# 243. Próximo gate — correspondência Physique SMD <-> bones SMB

Foi preparado:

```text
pt_smd_smb_bone_match_v1.py
```

Esse probe é somente leitura e compara:

```text
cada nome Physique dos 1444 vértices de MfbC01.smd
```

contra:

```text
NodeName dos 95 objetos de m5.smb
```

Também valida:

```text
nenhum nome Physique vazio;
nenhum bone referenciado ausente;
nenhum NodeName duplicado no skeleton.
```

Objetivo antes de qualquer matemática de matriz:

```text
provar que m5.smb é o conjunto de bones compatível por nome
com o Physique de MfbC01.smd.
```

Somente depois disso auditar/aplicar as matrizes de bind pose.
---

# 244. Correspondência Physique MfbC01.smd <-> m5.smb — PROVADA

O probe:

```text
pt_smd_smb_bone_match_v1.py
```

retornou:

```text
MESH
objects=15
physique_assignments=1444
unique_physique_names=44
empty_physique_names=0

SKELETON
objects=95
unique_node_names=95
duplicate_node_names=0

MATCH
matched_unique_names=44
missing_unique_names=0
all_physique_names_found=YES

bone_match_check=PASS
probe_exit=0
```

Portanto:

```text
todos os 1444 vértices possuem bone name válido;
existem 44 bones únicos referenciados pelo mesh;
todos os 44 existem como NodeName em m5.smb;
nenhum NodeName do skeleton é duplicado.
```

Estado:

```text
MfbC01.smd Physique -> m5.smb NodeName compatibility: PROVADO
```

---

# 245. Exemplos de correspondência e hierarquia

Exemplos reais:

```text
Bip01 Spine1
    assignments=202
    parent=Bip01 Spine

Bip01 L UpperArm
    assignments=111
    parent=Bip01 L Clavicle

Bip01 R UpperArm
    assignments=111
    parent=Bip01 R Clavicle

Bip01 L Forearm
    assignments=108
    parent=Bip01 L UpperArm

Bip01 R Forearm
    assignments=108
    parent=Bip01 R UpperArm

Bip-0037
    assignments=12
    parent=Bip01 Pelvis
```

Os bones auxiliares `Bip-xxxx` também são referências reais do Physique e
devem ser preservados literalmente.

---

# 246. Próximo gate: matriz exata usada para desfazer ReformPhysique

A escolha do skeleton não é mais pendente.

Agora falta provar somente a matemática necessária para reconstruir a bind pose:

```text
MfbC01.smd Vertex bone-local
    +
bone Tm de m5.smb
    =
posição original/montada
```

Antes de implementar essa transformação, auditar exatamente:

```text
smOBJ3D::ReformPhysique
função de inversão de matriz usada por ReformPhysique
estrutura/operação smFMATRIX
```

Objetivos:

```text
confirmar qual matriz é invertida;
confirmar ordem row/column usada na multiplicação;
confirmar onde entra _41/_42/_43;
confirmar se Tm do SMB serializado pode ser usado diretamente para a operação
inversa.
```

Não alterar XYZ, escala, UV ou material durante esse gate.

---

# 247. Convenção de comandos

A partir deste ponto, blocos de teste fornecidos ao usuário não devem terminar
com comando `exit`, para não encerrar o shell de trabalho.
---

# 248. ReformPhysique — direção e convenção da matriz PROVADAS

A source faz:

```cpp
smMatrixInvert(q, pBone->Tm);

x = Vertex.x*q._11 + Vertex.y*q._21 + Vertex.z*q._31;
y = Vertex.x*q._12 + Vertex.y*q._22 + Vertex.z*q._32;
z = Vertex.x*q._13 + Vertex.y*q._23 + Vertex.z*q._33;

Vertex.x = (x >> 8) + q._41;
Vertex.y = (y >> 8) + q._42;
Vertex.z = (z >> 8) + q._43;
```

Portanto a convenção é equivalente a vetor-linha afim:

```text
p' = [x y z 1] * M
```

com tradução na quarta linha:

```text
_41 _42 _43
```

e `fONE=256`.

Como:

```text
q = inverse(pBone->Tm)
```

o arquivo de mesh após `ReformPhysique` guarda:

```text
SMD_local = ASE_bind_world * inverse(bone.Tm)
```

Logo a operação inversa é diretamente:

```text
ASE_bind_world = SMD_local * bone.Tm
```

sujeita somente ao arredondamento fixed-point de 8 bits.

Isto é mais forte que usar `mWorld` para a reconstrução da bind pose: para
desfazer exatamente `ReformPhysique`, a matriz correta é o próprio `Tm`
serializado do bone.

---

# 249. smMatrixInvert — layout afim confirmado

`smMatrixInvert` valida:

```text
_44 ~= 256
_14 ~= 0
_24 ~= 0
_34 ~= 0
```

e calcula a tradução inversa por:

```text
q._41 = -(a._41*q._11 + a._42*q._21 + a._43*q._31) >> 8
q._42 = -(a._41*q._12 + a._42*q._22 + a._43*q._32) >> 8
q._43 = -(a._41*q._13 + a._42*q._23 + a._43*q._33) >> 8
```

Isso confirma a mesma convenção de vetor-linha usada em `ReformPhysique`.

---

# 250. Próximo probe de bind pose preparado

Arquivo:

```text
pt_smd_smb_bind_pose_probe_v1.py
```

Ele é somente leitura e:

```text
lê MfbC01.smd;
lê m5.smb;
casa Physique -> NodeName;
lê bone.Tm no offset ABI-proved 240;
reconstrói cada vértice por local * Tm;
calcula bounding box;
aplica inverse(Tm) novamente;
mede o erro de round-trip fixed-point.
```

O probe não escreve no cliente, não chama Blender e não usa `exit`.

PASS neste gate significa apenas:

```text
offset/layout/matriz/direção internamente consistentes
```

A prova visual continua pendente até gerar o próximo `.blend`.
---

# 251. Bind-pose probe: resultado e correção do gate

Resultado observado:

```text
vertices=1444
bones_used=44

BBOX_RAW_SMD_LOCAL
size=[13.375,10.770,10.625]

BBOX_RECONSTRUCTED_BY_BONE_TM
x=[-9.043,9.090]
y=[-5.727,5.273]
z=[0.000,37.508]
size=[18.133,11.000,37.508]
```

A distribuição por objeto também ficou corporalmente coerente:

```text
membros inferiores: z aproximadamente 0..10
corpo:              z aproximadamente 10..31
partes superiores:  z aproximadamente 17..37.5
```

O round-trip:

```text
local -> Tm -> inverse(Tm) -> local
```

teve:

```text
exact=189/1444
within 1 fixed unit=404/1444
within 2 fixed units=671/1444
max error=13 fixed units
max error=0.050781 model unit
```

O probe retornou erro somente porque havia um limite experimental/arbitrário:

```text
max_abs_error <= 8
```

Esse limite não veio da source e não deve ser usado como condição de validade.

O fixed-point integer inverse faz divisões, shifts e truncamentos; portanto a
operação não é bijetiva/exatamente reversível.

Estado correto:

```text
fórmula local * bone.Tm: source-proved
bone matching:           proved
bbox reconstruída:       coerente
prova visual Blender:    PENDENTE
```

---

# 252. Importer V2 bind pose preparado

Arquivo:

```text
pt_smd_blender_import_v2_bindpose.py
```

Diferença principal em relação ao V1:

```text
V1:
Blender = SMD Vertex / 256

V2:
bone = m5.smb[PhysiqueName]
bind_fixed = SMD Vertex * bone.Tm
Blender = bind_fixed / 256
```

Mantém:

```text
faces;
material IDs;
UV;
texturas;
BMP protegido em cache;
vertex groups Physique.
```

Ainda não cria armature nem animação.

Objetivo único do V2:

```text
provar visualmente se a bind pose reconstruída é uma Priestess reconhecível.
```

---

# 253. Convenção de shell reafirmada

Comandos fornecidos não devem usar `exit`.
---

# 254. Importer V2 bind pose executado com sucesso — PROVADO

O Blender 5.1.1 executou:

```text
pt_smd_blender_import_v2_bindpose.py
```

com:

```text
mesh     = char/tmABCD/MfbC01.smd
skeleton = char/tmABCD/m5.smb
```

Resultado:

```text
Objects          : 15
Materials        : 6
Skeleton objects : 95
Bind vertices    : 1444
Bind bones used  : 44

Bind bbox:
x=[-9.043,9.090]
y=[-5.727,5.273]
z=[0.000,37.508]
```

Todos os objetos fecharam novamente com:

```text
vertices=1444
faces=2537
uv_faces=2537
bind_vertices=1444
bind_bones_used=44
```

e:

```text
bind_pose_import_check=PASS
blender_exit=0
```

Arquivo salvo:

```text
MfbC01_direct_smd_v2_bindpose.blend
```

Tamanho observado:

```text
563K
```

Estado:

```text
SMD parsing:                    PROVADO
SMB skeleton parsing:           PROVADO
Physique -> bone matching:      PROVADO
local * bone.Tm reconstruction: PROVADO em execução
Blender datablock creation:     PROVADO
save .blend:                    PROVADO
correção visual da Priestess:   PENDENTE
```

---

# 255. Materiais/texturas no V2

O V2 manteve o resultado do V1:

```text
MfbC01.bmp -> protected prefix14 -> decoded cache -> LOADED
MfbC03.bmp -> protected prefix14 -> decoded cache -> LOADED
mo-p-ac.BMP -> standard BMP -> LOADED
```

Nenhum asset original do client foi alterado.

---

# 256. Warnings Blender continuam não bloqueantes

Foram observados novamente:

```text
OpenColorIO 2.5 config / 2.4.2 library mismatch
fallback color management
```

e:

```text
DeprecationWarning: Material.use_nodes
```

O processo continuou até:

```text
Info: Saved as "MfbC01_direct_smd_v2_bindpose.blend"
bind_pose_import_check=PASS
Blender quit
blender_exit=0
```

Logo esses warnings continuam não bloqueantes para este pipeline.

---

# 257. Próximo gate — inspeção visual V2

Não alterar código antes da inspeção.

Abrir somente:

```text
MfbC01_direct_smd_v2_bindpose.blend
```

Verificar:

```text
1. personagem reconhecível como corpo humano/Priestess;
2. braços, pernas e torso montados;
3. peças C01/C03 espacialmente alinhadas;
4. orientação correta;
5. UV/texturas não invertidos;
6. ausência de peças explodidas;
7. ausência de deformação grosseira.
```

Se visualmente correto:

```text
SMD + SMB -> Blender bind pose estático: PROVADO
```

e o próximo gate passa a ser:

```text
armature + hierarquia + animação
```

Se ainda incorreto, comparar a imagem com os bboxes e auditar somente a
transformação necessária, sem voltar a alterar formato, materiais ou UV.
---

# 258. Bind pose visual da Priestess — PROVADA

Inspeção visual do:

```text
MfbC01_direct_smd_v2_bindpose.blend
```

mostrou que, após a reconstrução:

```text
MfbC01.smd Vertex bone-local
+
Physique bone name
+
m5.smb bone.Tm
```

o modelo está espacialmente correto/reconhecível.

Estado atualizado:

```text
SMD structural parsing:                  PROVADO
SMB skeleton parsing:                    PROVADO
Physique -> bone matching:               PROVADO
local * bone.Tm bind reconstruction:     PROVADO
Blender static bind-pose geometry:       PROVADO VISUALMENTE
```

---

# 259. Importer V2 mostra variantes simultaneamente — PENDENTE

O V2 cria todos os 15 `smOBJ3D` do `MfbC01.smd` como objetos visíveis.

Foi observado visualmente que ocultar:

```text
MlhC03
MlmC03
MllC03
```

faz a bota retornar à aparência observada originalmente no jogo.

O SMD contém pares:

```text
MlhC01 / MlhC03
MlmC01 / MlmC03
MllC01 / MllC03
```

e as variantes C01/C03 usam material IDs diferentes.

Portanto o formato contém variantes/partes que não necessariamente devem ser
renderizadas simultaneamente.

Ainda NÃO aplicar no importer a regra:

```text
hide all C03
```

nem assumir que somente esses três objetos são sempre ocultos.

Existem outros objetos C03 no mesmo arquivo:

```text
MsmC03
MslC03
MshC03
```

e seu papel/estado de visibilidade runtime precisa ser determinado pela source.

---

# 260. Novo próximo gate — seleção runtime de objetos

Auditar os usos de:

```text
NodeName
smPAT3D render
Object rendering loops
```

e qualquer lógica que examine partes do nome do objeto.

Objetivo:

```text
descobrir quais dos 15 smOBJ3D são renderizados para a configuração base C01;
replicar essa visibilidade no Blender sem heurística visual.
```

A geometria/matriz não deve mais ser alterada durante esse gate.
---

# 261. smPAT3D possui render completo e render por lista — PROVADO

A auditoria de `smObj3d.cpp` mostrou dois comportamentos distintos.

`smPAT3D::RenderD3D(...)` percorre:

```cpp
for(cnt = 0; cnt < nObj3d; cnt++)
    obj3d[cnt]->RenderD3D(...);
```

Portanto o render normal desenha todos os `smOBJ3D` do pattern.

Já:

```cpp
smPAT3D::ChoiseRenderD3D(...)
```

recebe:

```text
char **szNodeList
int ListCnt
```

e somente renderiza um objeto quando:

```cpp
lstrcmpi(szNodeList[cnt2], obj3d[cnt]->NodeName) == 0
```

Logo existe suporte explícito na engine para renderizar somente uma seleção
nominal dos nodes de um SMD.

Estado:

```text
RenderD3D = todos os nodes:       PROVADO
ChoiseRenderD3D = lista de nodes: PROVADO
```

---

# 262. Explicação provável para C01/C03 simultâneos no Blender

O importer Blender V2 cria os 15 objetos e os deixa todos visíveis.

O jogo observado não apresenta a mesma sobreposição; esconder manualmente:

```text
MlhC03
MlmC03
MllC03
```

restaura a aparência original da bota.

Como `ChoiseRenderD3D()` existe exatamente para filtrar por `NodeName`, ele é
agora o principal candidato para reproduzir essa seleção.

Porém ainda NÃO está provado que o caminho do player chama essa função, nem
qual `szNodeList` é passado.

---

# 263. Próximo gate — caller e origem do NodeList

Auditar:

```text
todas as chamadas de ChoiseRenderD3D fora de smObj3d.cpp;
declarações/preenchimentos das listas passadas como terceiro argumento;
caminho de render do smCHAR/player.
```

Objetivo:

```text
obter a lista exata de NodeName usada pela Priestess base;
replicar essa lista no Blender.
```

Não aplicar regra heurística `hide C03` antes dessa prova.
---

# 264. Player/character usa ChoiseRenderD3D — PROVADO

Callers reais encontrados em:

```text
SrcGame/src/character.cpp
```

linhas:

```text
8176
8178
8228
8230
```

As chamadas são:

```cpp
Pattern->ChoiseRenderD3D(
    &CameraPosi,
    &CameraAngle,
    ModelNameList,
    lpModelGroup->ModelNameCnt,
    ...);

Pattern2->ChoiseRenderD3D(
    &CameraPosi,
    &CameraAngle,
    ModelNameList,
    lpModelGroup->ModelNameCnt,
    ...);
```

Logo o caminho de render do personagem realmente utiliza a seleção nominal
de nodes.

Estado:

```text
ChoiseRenderD3D existe:                  PROVADO
character.cpp chama ChoiseRenderD3D:    PROVADO
lista usada = ModelNameList:            PROVADO
count usado = lpModelGroup->ModelNameCnt: PROVADO
```

---

# 265. Próximo gate

Abrir somente o contexto de `character.cpp` ao redor dessas chamadas e localizar:

```text
declaração de ModelNameList;
preenchimento de ModelNameList;
origem de lpModelGroup;
condição que seleciona HighModel / DefaultModel / LowModel.
```

Objetivo:

```text
obter os NodeName exatos que o client escolhe para a Priestess base
e reproduzir essa visibilidade no Blender.
```

Não aplicar filtro heurístico por sufixo C01/C03 antes dessa prova.
---

# 266. Seleção runtime de partes do modelo — regra PROVADA

Em `character.cpp`, o grupo começa como:

```cpp
lpModelGroup = &smMotionInfo->DefaultModel;
```

Depois:

```cpp
if (dDist < VIEW_HIGH_DIST && smMotionInfo->HighModel.ModelNameCnt)
    lpModelGroup = &smMotionInfo->HighModel;

if (dDist > VIEW_MID_DIST && smMotionInfo->LowModel.ModelNameCnt)
    lpModelGroup = &smMotionInfo->LowModel;
```

Logo:

```text
default distance -> DefaultModel
near             -> HighModel, se ModelNameCnt != 0
far              -> LowModel, se ModelNameCnt != 0
```

Quando o grupo possui nomes:

```cpp
ModelNameList[0] = lpModelGroup->szModelName[0];
ModelNameList[1] = lpModelGroup->szModelName[1];
ModelNameList[2] = lpModelGroup->szModelName[2];
ModelNameList[3] = lpModelGroup->szModelName[3];
```

e chama:

```cpp
Pattern->ChoiseRenderD3D(
    ...,
    ModelNameList,
    lpModelGroup->ModelNameCnt,
    ...);
```

O mesmo mecanismo é usado para `Pattern2`, mas com:

```cpp
lpDinaPattern2->lpModelInfo
```

Estado:

```text
seleção por distância High/Default/Low: PROVADA
lista nominal vem de _MODELGROUP:       PROVADA
máximo de quatro NodeName por grupo:    PROVADO pela struct
render seletivo usa ModelNameCnt:       PROVADO
```

---

# 267. Não há regra C01/C03 no renderer

A source do renderer não decide por sufixo de NodeName.

A decisão é orientada por dados:

```text
smMODELINFO.HighModel
smMODELINFO.DefaultModel
smMODELINFO.LowModel
```

Cada `_MODELGROUP` contém:

```cpp
int ModelNameCnt;
char szModelName[4][16];
```

Portanto a sobreposição observada no Blender V2 ocorre porque o importer
deixa os 15 objetos visíveis, enquanto o client usa uma dessas listas.

---

# 268. Offsets dos _MODELGROUP dentro de smMODELINFO

Pela definição já auditada:

```cpp
char szModelFile[64];      // 0x000
char szMotionFile[64];     // 0x040
char szSubModelFile[64];   // 0x080

_MODELGROUP HighModel;     // 0x0C0
_MODELGROUP DefaultModel;  // 0x104
_MODELGROUP LowModel;      // 0x148
```

Como `_MODELGROUP` tem:

```text
4 + 4*16 = 68 bytes = 0x44
```

os offsets são:

```text
HighModel    0x0C0
DefaultModel 0x104
LowModel     0x148
```

O próximo gate pode decodificar diretamente esses 204 bytes do INX sem
precisar entender o restante de `smMODELINFO`.

---

# 269. Próximo gate

Ler:

```text
HighModel
DefaultModel
LowModel
```

de:

```text
M5Bip.inx
mC001.inx
```

e imprimir:

```text
ModelNameCnt
szModelName[0..3]
```

Objetivo final:

```text
identificar exatamente quais objetos do MfbC01.smd o client renderiza
em cada faixa de distância.
```

Depois disso o importer Blender pode aplicar a mesma seleção de visibilidade,
sem regra heurística por nome.
---

# 270. Model groups reais da Priestess base — PROVADOS

Foram decodificados diretamente os três `_MODELGROUP` de:

```text
M5Bip.inx
mC001.inx
```

Os dois arquivos retornaram exatamente o mesmo conteúdo.

## HighModel

```text
ModelNameCnt=3

MbhC01
MshC01
MlhC01
```

## DefaultModel

```text
ModelNameCnt=3

MbmC01
MsmC01
MlmC01
```

## LowModel

```text
ModelNameCnt=3

MblC01
MslC01
MllC01
```

Estado:

```text
HighModel names:    PROVADO
DefaultModel names: PROVADO
LowModel names:     PROVADO
M5Bip.inx == mC001.inx groups: PROVADO
```

---

# 271. C03 não faz parte de nenhum LOD selecionado por este model-info

Nenhum dos seguintes nodes aparece em `HighModel`, `DefaultModel` ou
`LowModel`:

```text
MsmC03
MslC03
MlhC03
MshC03
MlmC03
MllC03
```

Portanto, para este `smMODELINFO`, esses objetos não são enviados ao
`ChoiseRenderD3D()`.

Isso explica a observação visual da bota: o importer Blender V2 deixava
objetos C03 visíveis que o client não renderiza para esse model-info.

---

# 272. O V2 também exibe os três LODs C01 simultaneamente

Mesmo depois de remover visualmente C03, o V2 ainda mantém:

```text
HighModel C01
DefaultModel C01
LowModel C01
```

todos visíveis simultaneamente.

O client não faz isso.

Em distância intermediária, `character.cpp` usa:

```text
DefaultModel
```

portanto a combinação correta para reproduzir a visualização default é:

```text
MbmC01
MsmC01
MlmC01
```

Em proximidade:

```text
MbhC01
MshC01
MlhC01
```

Em distância longa:

```text
MblC01
MslC01
MllC01
```

---

# 273. Próximo gate — visibilidade DefaultModel no Blender

Antes de adicionar armature/animação, testar a visualização exata do runtime:

```text
VISIBLE:
MbmC01
MsmC01
MlmC01

HIDDEN:
todos os outros 12 objetos
```

Se a aparência coincidir com o personagem normal no client:

```text
INX model-group -> Blender visibility: PROVADO
```

Depois disso o importer deve ganhar suporte explícito a:

```text
--model-info
--lod high|default|low
```

mantendo todos os objetos importados, mas configurando a visibilidade de
acordo com `_MODELGROUP`.
---

# 274. Visual DefaultModel da Priestess — PROVADO

A inspeção manual no Blender deixou visíveis somente:

```text
MbmC01
MsmC01
MlmC01
```

que são exatamente os três nodes de:

```text
smMODELINFO.DefaultModel
```

A aparência resultante foi reconhecida como a Sacerdotisa normal observada
no client.

Estado final do gate estático:

```text
INX -> DefaultModel selection:          PROVADO
SMD -> geometria:                       PROVADO
Physique -> m5.smb bone matching:       PROVADO
bone-local -> bone.Tm bind pose:        PROVADO
UV/texturas:                            PROVADO VISUALMENTE
visual Priestess default no Blender:    PROVADO VISUALMENTE
```

Logo:

```text
INX + SMD + SMB -> Blender static character
```

está provado para a Priestess base/default.

---

# 275. Hipótese sobre nodes C03

Foi levantada a hipótese de que os nodes:

```text
MsmC03
MslC03
MlhC03
MshC03
MlmC03
MllC03
```

possam estar ligados a mudança de classe/job ou outra variante do personagem.

Estado:

```text
existência no mesmo SMD:                         PROVADO
não selecionados pelo model-info atual:          PROVADO
relação específica com mudança de classe/job:    PENDENTE
```

Não transformar essa hipótese em regra até localizar na source/INX outro
model-info que selecione esses nodes.

---

# 276. Importer V2.1 preparado — seleção INX/LOD

Novo arquivo:

```text
pt_smd_blender_import_v2_1_inx_lod.py
```

Adiciona:

```text
--model-info <arquivo .inx>
--lod high|default|low
```

O importer:

```text
1. importa todos os smOBJ3D;
2. reconstrói bind pose usando m5.smb;
3. lê HighModel/DefaultModel/LowModel diretamente do INX;
4. mantém visíveis somente os NodeName do LOD escolhido;
5. mantém os demais objetos no .blend, porém ocultos.
```

Para a Priestess base:

```text
--model-info M5Bip.inx
--lod default
```

deve resultar em:

```text
VISIBLE:
MbmC01
MsmC01
MlmC01

HIDDEN:
os outros 12 nodes
```

Esse é o próximo gate antes de armature/animação.
---

# 277. Importer V2.1 INX/LOD — PROVADO EM EXECUÇÃO

O teste do:

```text
pt_smd_blender_import_v2_1_inx_lod.py
```

com:

```text
MfbC01.smd
m5.smb
M5Bip.inx
--lod default
```

funcionou em execução real.

Estado:

```text
INX parsing:                         PROVADO
DefaultModel selection:              PROVADO
visibilidade automática no Blender:  PROVADO
SMD + SMB bind pose:                 PROVADO
visual Priestess default:            PROVADO
```

O pipeline estático da Priestess base está fechado.

---

# 278. Próxima fase

A partir deste ponto não alterar novamente:

```text
parser SMD;
parser SMB;
reconstrução local * bone.Tm;
UV;
materiais;
seleção INX/LOD.
```

Próxima fase:

```text
armature / hierarchy / animation
```

Primeiro gate dessa fase:

```text
reconstruir a hierarquia estática dos 95 bones de m5.smb no Blender,
sem animação.
```

Somente depois de a armature estática estar visualmente correta iniciar a
leitura/aplicação de TmRot/TmPos/TmScale.
---

# 279. Próximo gate — rest skeleton do m5.smb

Foi preparado:

```text
pt_smb_rest_skeleton_probe_v1.py
```

O probe é somente leitura e interpreta cada bone como:

```text
head.x = Tm._41 / 256
head.y = Tm._42 / 256
head.z = Tm._43 / 256
```

Essa escolha é coerente com a operação já provada:

```text
bind_vertex = local_vertex * bone.Tm
```

que produziu o modelo visualmente correto no Blender.

O probe valida:

```text
95 NodeName únicos;
NodeParent existente;
roots;
layout afim de Tm;
bounding box das posições;
distância pai -> filho;
bones pai/filho com posição coincidente.
```

Os bones de comprimento zero precisam ser conhecidos antes da criação da
armature, porque Blender não aceita edit bones com head == tail.

Ainda não criar armature neste gate.
---

# 280. Rest skeleton m5.smb — estrutura PROVADA

Resultado do `pt_smb_rest_skeleton_probe_v1.py`:

```text
objects=95
materials=0
unique_names=95
roots=1
missing_parents=0
leaf_bones=25
affine_tm_invalid=0

edges=94
zero_length_edges=0
near_zero_edges=0

rest_skeleton_structure_check=PASS
probe_exit=0
```

Root:

```text
Bip01
```

A hierarquia anatômica principal apresenta posições coerentes:

```text
Pelvis -> Spine -> Spine1 -> Neck -> Head
L/R Thigh -> Calf -> Foot
L/R Clavicle -> UpperArm -> Forearm -> Hand
```

Não existem parent-child joints coincidentes.

Estado:

```text
NodeName/NodeParent hierarchy: PROVADO
Tm._41/_42/_43 joint positions: PROVADO estruturalmente
Blender armature: PENDENTE
```

---

# 281. Próximo gate — eixo longitudinal da Tm

Antes de criar EditBones, foi preparado:

```text
pt_smb_axis_alignment_probe_v1.py
```

O probe compara a direção:

```text
bone head -> único child head
```

contra os três eixos da matriz `Tm`, testando:

```text
rows X/Y/Z
columns X/Y/Z
```

Também mede:

```text
comprimento dos basis vectors;
ortogonalidade;
determinante;
sinal do eixo dominante.
```

Objetivo:

```text
descobrir qual eixo local do formato Priston representa longitudinalmente
o bone e preservar orientação/roll na futura armature Blender.
```

Nenhuma armature é criada neste gate.
---

# 282. Tm row basis alinhada à hierarquia — PROVADO para m5.smb

Resultado:

```text
ROW_BASIS_ALIGNMENT
single_child_samples=62
best_axis_counts=X:16 Y:6 Z:40
best_absdot_median=0.999992
best_absdot_mean=0.993927
weak_under_0_90=2

COLUMN_BASIS_ALIGNMENT
best_axis_counts=X:11 Y:12 Z:39
best_absdot_median=0.975157
best_absdot_mean=0.931028
weak_under_0_90=18
```

Braços e pernas principais deram alinhamento praticamente exato usando rows:

```text
L/R Clavicle -> UpperArm: X ~= 0.999999
L/R UpperArm -> Forearm:  X ~= 0.999987
L/R Forearm -> Hand:      X ~= 0.999991
L/R Thigh -> Calf:        X ~= 0.999986
L/R Calf -> Foot:         X ~= 0.999997
```

Não existe eixo longitudinal universal:

```text
X: 16
Y: 6
Z: 40
```

Logo a futura armature não deve assumir `bone axis = X`.

---

# 283. Importer V3 static armature preparado

Arquivo:

```text
pt_smd_blender_import_v3_static_armature.py
```

Mantém o pipeline já provado:

```text
SMD
SMB
Physique
bind pose
INX/LOD
textures
UV
```

e adiciona:

```text
PT_m5_Armature
95 bones
```

Regras:

```text
head = Tm._41/_42/_43 / 256
parent = NodeParent
use_connect = false
```

Para bones com um único child:

```text
tail = child head
```

Para branch/leaf, somente o tail visual é construído por continuação segura;
o joint/head real continua sendo exatamente a tradução da Tm.

O roll usa a row-basis da Tm mais coerente com a direção do EditBone.

A Tm inteira original é preservada em custom properties de cada bone.

Também são adicionados Armature Modifiers aos 15 objetos do SMD.

Ainda não existem keyframes/animação.

---

# 284. Gate visual V3

Verificar:

```text
armature atravessa corretamente a Priestess;
pelvis/spine/head coerentes;
braços e pernas coerentes;
mesh default continua visualmente igual ao V2.1;
nenhuma deformação aparece em rest pose.
```

Somente depois disso iniciar animação.
---

# 285. Importer V3 static armature — PROVADO EM EXECUÇÃO

O Blender executou:

```text
pt_smd_blender_import_v3_static_armature.py
```

com:

```text
MfbC01.smd
m5.smb
M5Bip.inx
--lod default
```

Resultado:

```text
STATIC_ARMATURE
bones=95
roots=['Bip01']
mesh_armature_modifiers=15
```

Histograma do eixo principal escolhido para representação:

```text
X: 33
Y: 6
Z: 56
```

A seleção INX continuou correta:

```text
high:
MbhC01
MshC01
MlhC01

default:
MbmC01
MsmC01
MlmC01

low:
MblC01
MslC01
MllC01
```

LOD usado:

```text
default
```

Visíveis:

```text
MsmC01
MlmC01
MbmC01
```

Ocultos:

```text
12 objetos
```

Resumo:

```text
objects=15
materials=6
vertices=1444
faces=2537
uv_faces=2537
bind_vertices=1444
bind_bones_used=44
armature_bones=95
armature_roots=['Bip01']
mesh_armature_modifiers=15
static_armature_import_check=PASS
blender_exit=0
```

Arquivo:

```text
MfbC01_direct_smd_v3_static_armature.blend
```

foi salvo e depois reaberto no Blender 5.1.1 sem erro fatal.

Estado:

```text
criação Blender Armature:      PROVADO EM EXECUÇÃO
95 bones:                      PROVADO
NodeParent aplicado:           PROVADO pela construção
Armature Modifiers:            PROVADO
LOD default preservado:        PROVADO
save/reopen .blend:            PROVADO
alinhamento visual dos bones:  PENDENTE
```

---

# 286. Próximo gate obrigatório antes de animação

Antes de decodificar/aplicar animação, confirmar visualmente:

```text
Bip01 Pelvis no quadril;
Spine/Spine1 no tronco;
Neck/Head no pescoço/cabeça;
L/R UpperArm e Forearm acompanhando os braços;
L/R Thigh, Calf e Foot acompanhando as pernas/pés.
```

Também verificar:

```text
mesh permanece igual ao V2.1;
nenhum membro é deformado em rest pose;
armature aparece no centro correto do personagem.
```

Somente se esse gate visual passar iniciar:

```text
TmRot
TmPos
TmScale
TmPrevRot
TmFrame
```

e reprodução de animação.
---

# 287. Armature estática — gate visual aceito

A inspeção visual indicou que a armature está coerente com a Priestess e
não apresentou desalinhamento evidente.

Estado:

```text
static armature creation: PROVADO EM EXECUÇÃO
static armature visual:   PROVADO VISUALMENTE
```

O pipeline pode avançar para animação.

---

# 288. Skeleton mais amplo que o mesh atual — PROVADO

O skeleton:

```text
m5.smb
```

contém:

```text
95 NodeName
```

enquanto:

```text
MfbC01.smd
```

usa no Physique:

```text
44 unique bone names
```

e todos esses 44 existem no `m5.smb`.

Portanto o skeleton contém muitos nodes que não são usados diretamente pelo
mesh Priestess base atual.

Estado:

```text
m5.smb é mais amplo que MfbC01.smd: PROVADO
```

Hipótese:

```text
os nodes extras servem a outras versões/job changes/equipamentos/attachments
```

continua:

```text
PENDENTE
```

até existir ligação direta na source ou em outros model-info/assets.

---

# 289. Serialização dos tracks — source/ABI já estabelecidos

A carga serializada do `smOBJ3D` ocorre na ordem:

```text
smOBJ3D fixed struct
smVERTEX[nVertex]
smFACE[nFace]
smTEXLINK[nTexLink]
smTM_ROT[TmRotCnt]
smTM_POS[TmPosCnt]
smTM_SCALE[TmScaleCnt]
smFMATRIX/TmPrevRot[TmRotCnt]
Physique names, quando presentes
```

Tamanhos medidos no ABI i686:

```text
smTM_ROT   = 20
smTM_POS   = 16
smTM_SCALE = 16
matrix     = 64
```

Offsets medidos:

```text
TmRotCnt   684
TmPosCnt   688
TmScaleCnt 692

TmRotFrame   696
TmPosFrame  1208
TmScaleFrame 1720
TmFrameCnt   2232
```

`smFRAME_POS`:

```cpp
int StartFrame;
int EndFrame;
int PosNum;
int PosCnt;
```

---

# 290. Próximo gate — inventário de animação do m5.smb

Preparado:

```text
pt_smb_animation_track_inventory_v1.py
```

O probe NÃO interpreta quaternion, posição ou escala ainda.

Ele valida apenas:

```text
offsets físicos dos arrays;
contagens TmRot/TmPos/TmScale;
frame inicial/final de cada track;
monotonicidade;
TmFrameCnt;
TmRotFrame/TmPosFrame/TmScaleFrame;
StartFrame/EndFrame/PosNum/PosCnt;
fechamento exato de cada objeto serializado.
```

Objetivo:

```text
entender como as 7 faixas de animação estão divididas no m5.smb
antes de reproduzir TmAnimation no Blender.
```
---

# 291. "Bone extra" não significa "bone inútil"

Para o mesh `MfbC01.smd`:

```text
Physique unique bones = 44
```

Para o skeleton `m5.smb`:

```text
NodeName = 95
```

Logo existem 51 nodes do skeleton que não são diretamente referenciados pelo
Physique desse SMD.

Isso NÃO autoriza removê-los.

Um node pode ter pelo menos três papéis distintos:

```text
1. deform bone:
   vertices do mesh apontam diretamente para ele via Physique;

2. hierarchy/helper bone:
   pode não ter vertices próprios, mas pode ser pai/ancestral de outro bone;

3. attachment/socket/auxiliary node:
   pode ser consultado por NodeName para arma, efeito, objeto ligado ou outra
   funcionalidade, sem nunca aparecer no Physique do corpo.
```

Conclusão prática:

```text
não deletar bones extras;
não renomear;
não alterar NodeParent;
preservar Tm e tracks originais.
```

---

# 292. Armature Modifier no Blender e bones sem vertex group

No pipeline atual, os grupos de vértices vêm dos nomes de Physique.

Portanto um bone que não possui vértices atribuídos a um vertex group
correspondente não deforma diretamente o mesh.

Porém ele ainda pode influenciar indiretamente se for ancestral de um deform
bone.

Assim, a classificação correta não é apenas:

```text
bone usado / não usado
```

mas:

```text
direct deform;
ancestor-only;
extra fora da ancestry do mesh;
external/attachment usage ainda desconhecido.
```

---

# 293. Animation inventory V1 — FAIL é falso negativo do validator

O probe retornou:

```text
animation_track_inventory_check=FAIL
```

mas isso NÃO constitui evidência de corrupção do `m5.smb`.

A maioria dos erros é causada pelo segmento 0:

```text
StartFrame=0
EndFrame=0
PosNum=-858993460
PosCnt=-858993460
```

`-858993460` em DWORD é:

```text
0xCCCCCCCC
```

padrão conhecido de memória/debug não inicializada no ecossistema MSVC.

Como o segmento possui intervalo `0..0`, esses campos devem ser tratados como
slot inutilizado/sentinel até a source provar outra semântica.

Outro erro:

```text
Bip weapon01: TmRot frames not monotonic
Bip weapon01: TmPos frames not monotonic
```

também não deve ser validado globalmente, pois os arrays contêm múltiplas
faixas concatenadas e `TmRotFrame/TmPosFrame` fornecem `PosNum/PosCnt` por
segmento.

A validação futura deve checar monotonicidade DENTRO de cada faixa, não no
array completo.

---

# 294. Tracks dos extra nodes provam que vários não são lixo morto

Inventário observado:

```text
objects_with_rotation=83
objects_with_position=23
objects_with_scale=0
```

Exemplos fora do Physique base possuem muitos tracks:

```text
Bip-h11  R=1997
Bip-h12  R=2348
Bip-h13  R=2385

Bip weapon01 R=220 P=209
Bip weapon02 R=8   P=8

Bip in01    R=186 P=186
Bip in02    R=3   P=3
Bip in-cro  R=186 P=186
Bip in03    R=3   P=3
Bip in-bow  R=186 P=186
Bip in04    R=3   P=3
```

Logo vários nodes extras são deliberadamente animados.

O papel exato ainda precisa de auditoria de callers/NodeName.

---

# 295. Próximo gate antes de animação — bone usage classifier

Preparado:

```text
pt_smb_bone_usage_classifier_v1.py
```

Ele cruza:

```text
MfbC01.smd Physique
m5.smb NodeParent
M5Bip.inx DefaultModel
```

e separa:

```text
DIRECT_DEFORM_BONES_VISIBLE_LOD
ANCESTOR_ONLY_BONES_NEEDED_TO_PROPAGATE_VISIBLE_LOD
EXTRA_BONES_NOT_IN_VISIBLE_DEFORM_ANCESTRY
```

Também marca quais extras possuem tracks de animação.

Objetivo:

```text
provar quais bones podem ou não afetar o mesh Priestess default
somente pela cadeia de deformação.
```

Essa classificação NÃO conclui uso externo por arma/efeito/socket.
---

# 296. Bone usage classifier — resultado PROVADO

Para:

```text
MfbC01.smd
m5.smb
M5Bip.inx
LOD=default
```

resultado:

```text
skeleton_bones=95
physique_unique_all_smd=44
physique_unique_visible_lod=43

ancestor_only_of_visible_deform=2
extra_not_ancestor_of_visible_deform=50
animated_extra_not_ancestor_of_visible_deform=38
zero_track_extra_not_ancestor_of_visible_deform=12
extra_descendants_of_visible_deform=49

bone_usage_classifier_check=PASS
```

---

# 297. Grafo mínimo que pode deformar a Priestess default

O LOD default usa diretamente 43 bones via Physique.

Além deles, apenas dois bones sem vertex assignment próprio são ancestrais
necessários:

```text
Bip01
Bip01 Neck
```

Portanto o grafo que pode afetar a Priestess default por propagação
hierárquica contém:

```text
43 direct deform
+ 2 ancestor-only
= 45 nodes
```

Isso não significa que os outros 50 sejam descartáveis.

---

# 298. Cinquenta nodes extras não conseguem deformar o mesh default por ancestry

O classifier provou:

```text
extra_not_ancestor_of_visible_deform=50
```

Esses nodes não estão no caminho:

```text
root -> ... -> deform bone
```

de nenhum dos 43 bones que recebem vertices no LOD default.

Em uma hierarquia normal pai->filho, a transformação de um filho não
retropropaga para o pai.

Portanto esses 50 nodes não conseguem alterar o mesh Priestess default por
propagação de skeleton, desde que:

```text
os vertex groups/Physique permaneçam corretos;
NodeParent dos 45 nodes relevantes permaneça correto;
nenhuma lógica externa seja simulada de forma diferente.
```

Quarenta e nove desses extras são descendentes de deform bones e formam
ramos auxiliares. O outro ramo inclui `Bip01 Footsteps`.

---

# 299. Extra nodes deliberadamente animados continuam necessários para preservação

Dos 50 extras fora da ancestry de deformação default:

```text
38 possuem tracks
12 não possuem tracks
```

Exemplos explícitos:

```text
Bip weapon01
Bip weapon02

Bip in01
Bip in02
Bip in-cro
Bip in03
Bip in-bow
Bip in04

Bip-h01 ... Bip-h24
```

Logo não tratá-los como lixo.

Possíveis papéis:

```text
attachment/socket;
arma;
efeito;
variante visual;
cadeia auxiliar;
terminal/end bone.
```

Papel exato de cada família ainda PENDENTE de source/callers.

---

# 300. Segurança do Blender atual

Para o visual default atual:

```text
manter os 95 bones é a opção correta e mais segura.
```

Um extra bone sem vertices e fora da ancestry dos deform bones não afeta
diretamente a Priestess default.

Mesmo assim NÃO:

```text
deletar;
renomear;
reparentear;
reescrever source SMB;
exportar por cima do original.
```

O importer V3 preserva a Tm original como custom properties, permitindo
corrigir posteriormente representação de tail/roll sem perder os dados de
origem.

Diferença importante:

```text
tail visual do Blender errado em bone extra
!=
dados source SMB alterados
```

Enquanto a conversão é somente import/read-only, o asset original continua
intacto.

---

# 301. Próximo gate opcional antes da animação

Pesquisar na source referências literais a:

```text
Bip weapon01
Bip weapon02
Bip in*
Bip-h*
Bip01 Footsteps
```

Se existirem, isso pode provar uso como socket/attachment.

Ausência de string literal NÃO prova ausência de uso, pois o engine também
pode receber NodeName por dados/configuração e chamar `GetObjectFromName`
dinamicamente.
---

# 302. Bones auxiliares usados como sockets — PROVADO pela source

Busca literal na source encontrou:

```cpp
SrcGame/src/character.cpp:
char *szBipName_RightHand = "Bip weapon01";
char *szBipName_BackSpine = "Bip in01";
char *szBipName_BackSpineCross = "Bip in-bow";
char *szBipName_BackSpineBow = "Bip in-cro";
```

O mesmo conjunto existe na source de servidor.

Isso prova que esses nodes possuem função lógica no jogo e não são simples
bones sem uso.

---

# 303. Bip weapon01 como attachment/effect point — PROVADO

Foi encontrado:

```cpp
pEffect->Obj =
    pEffect->pChar->AnimPattern->GetObjectFromName("Bip weapon01");
```

em:

```text
SrcGame/src/sinbaram/sinAssaSkillEffect.cpp
SrcServer/src/sinbaram/sinAssaSkillEffect.cpp
```

Portanto:

```text
Bip weapon01
```

é consultado explicitamente por NodeName durante efeitos de skill.

Além disso:

```text
szBipName_RightHand = "Bip weapon01"
```

confirma sua função de ponto/socket relacionado à mão direita.

Estado:

```text
Bip weapon01 como node funcional: PROVADO
Bip weapon01 como right-hand socket: PROVADO
Bip weapon01 usado por skill effect: PROVADO
```

---

# 304. Sockets das costas — nomes funcionais PROVADOS

A source associa:

```text
Bip in01   -> BackSpine
Bip in-bow -> BackSpineCross
Bip in-cro -> BackSpineBow
```

Logo esses três bones extras têm função nominal explícita ligada às costas /
equipamentos carregados.

Ainda é preciso inspecionar callers para determinar exatamente quais
equipamentos/estados usam cada socket.

---

# 305. Conclusão sobre preservação dos 95 bones

A dúvida sobre remover bones extras está encerrada.

O skeleton contém:

```text
43 direct deform bones do LOD default
2 ancestor-only bones
50 extras fora da ancestry de deformação
```

e a source provou que pelo menos parte dos 50 extras é usada por outros
sistemas através de `GetObjectFromName()` e variáveis de socket.

Portanto:

```text
NÃO deletar bones extras
NÃO renomear
NÃO alterar NodeParent
NÃO remover tracks
```

Mesmo um bone sem vertex weights pode ser necessário para:

```text
arma;
efeito;
item carregado;
socket;
attachment.
```

A estratégia correta do importer é preservar todos os 95 nodes.

---

# 306. Implicação para Blender

Um bone extra pode ter tail/roll visual aproximado sem alterar diretamente o
mesh default, desde que:

```text
head/source joint permaneça correto;
NodeParent permaneça correto;
vertex weights permaneçam corretos;
source Tm/tracks sejam preservados.
```

Porém, antes de exportação futura, todos os transforms usados como socket
deverão ser reproduzidos corretamente, porque objetos externos podem depender
da matriz final desses nodes.

Para import/read-only e estudo:

```text
preservar 95 bones = correto
```

está agora suportado tanto pelo grafo de Physique quanto pela source.
---

# 307. GetTmFrameRot/Pos/Scale — semântica PROVADA pela source

A implementação exata é:

```cpp
if(TmPosFrame[cnt].PosCnt > 0 &&
   TmPosFrame[cnt].StartFrame <= frame &&
   TmPosFrame[cnt].EndFrame > frame)
{
    return TmPosFrame[cnt].PosNum;
}
```

A mesma lógica é usada para rotation e scale.

Portanto:

```text
PosCnt > 0                 = slot de track ativo
StartFrame <= frame < EndFrame = seleção temporal
PosNum                     = índice inicial do segmento no array
```

`TmAnimation()` usa diretamente:

```cpp
&TmPrevRot[NumTmRot]
&TmRot[NumTmRot]
&TmPos[NumTmPos]
&TmScale[NumTmScale]
```

Logo:

```text
PosNum como offset de array: PROVADO
```

---

# 308. Structs dos tracks — PROVADO pela source

```cpp
struct smTM_ROT
{
    int frame;
    float x, y, z, w;
};

struct smTM_POS
{
    int frame;
    float x, y, z;
};

struct smTM_SCALE
{
    int frame;
    int x, y, z;
};
```

Compatível com ABI já medido:

```text
smTM_ROT   = 20 bytes
smTM_POS   = 16 bytes
smTM_SCALE = 16 bytes
```

---

# 309. Correção: 7 slots, mas 6 ranges globais ativos no m5.smb

Header observado:

```text
slot 0: 0..0
slot 1: 0..76160
slot 2: 76160..156320
slot 3: 156320..234880
slot 4: 234880..324640
slot 5: 324640..422400
slot 6: 422400..500160
```

Portanto:

```text
TmFrameCounter = 7
slots físicos  = 7
ranges ativos  = 6
```

O slot 0 é vazio.

Os campos:

```text
PosNum=0xCCCCCCCC
PosCnt=0xCCCCCCCC
```

do header/slots não utilizados não devem ser tratados como índices válidos.

Para os arrays por objeto, a source primeiro exige:

```text
PosCnt > 0
```

antes de usar `PosNum`.

Assim o FAIL do validator V1 era falso negativo.

---

# 310. Monotonicidade correta é por segmento

Os arrays de keys de um node podem conter vários segmentos concatenados.

`GetTmFrame*()` escolhe `PosNum` do slot atual e passa:

```text
&Track[PosNum]
```

para o interpolador.

Consequentemente:

```text
não validar monotonicidade do array inteiro;
validar somente Track[PosNum : PosNum+PosCnt].
```

Isso explica o falso erro anterior de `Bip weapon01`.

---

# 311. Animation Segment Inventory V2 preparado

Arquivo:

```text
pt_smb_animation_segment_inventory_v2.py
```

Valida:

```text
slot ativo somente quando PosCnt > 0;
PosNum dentro do array;
PosNum+PosCnt dentro do array;
monotonicidade dentro do segmento;
cobertura StartFrame/EndFrame;
ranges do objeto iguais aos ranges globais;
slices sem overlap/gap/orphan keys;
fechamento físico do objeto.
```

Ainda não interpreta os valores quaternion/position.

Objetivo:

```text
fechar estruturalmente a segmentação antes de criar qualquer keyframe Blender.
```
---

# 312. Animation Segment Inventory V2 — FAIL por regra de borda incorreta

O V2 confirmou corretamente:

```text
TmFrameCounter=7
slot 0 vazio
slots 1..6 com ranges globais
objects_with_rotation=83
objects_with_position=23
objects_with_scale=0
```

Mas retornou:

```text
errors=298
animation_segment_inventory_check=FAIL
```

Os erros mostrados são predominantemente:

```text
last_key < EndFrame
first_key > StartFrame
```

Essas duas condições NÃO são erros segundo a implementação do engine.

---

# 313. first_key > StartFrame é permitido pela source

`GetRotFrame()` começa com:

```cpp
cnt = 0;

if(tmRot[cnt].frame > frame)
    return frame;
```

`GetPosFrame()` usa a mesma guarda.

Logo um segmento pode começar em:

```text
StartFrame = 0
```

e ter sua primeira key somente em:

```text
frame = 320
```

sem violar a lógica do engine.

Estado:

```text
validator V2 first_key<=StartFrame requirement: REJEITADO
```

---

# 314. PosCnt não limita o loop do interpolador

`GetTmFrameRot()` usa `PosCnt > 0` apenas para decidir se o slot está ativo e
retorna:

```cpp
TmRotFrame[cnt].PosNum
```

Depois `TmAnimation()` chama:

```cpp
GetRotFrame(
    qmat,
    &TmPrevRot[NumTmRot],
    &TmRot[NumTmRot],
    frame,
    ...
);
```

`GetRotFrame()` NÃO recebe `PosCnt`.

Seu loop usa:

```cpp
s = tmRot[cnt].frame;
e = tmRot[cnt + 1].frame;
```

até encontrar:

```text
s <= frame < e
```

Portanto a key imediatamente após:

```text
PosNum + PosCnt - 1
```

pode servir como próxima key de interpolação.

Isso explica padrões como:

```text
Bip01 slot1:
slice=0:223
última key do slice=48320

slot2:
primeira key no índice 223=76160
```

A key do próximo slice pode fechar a interpolação do slice anterior.

Estado:

```text
validator V2 last_key>=EndFrame requirement: REJEITADO
```

---

# 315. Seis ranges globais não equivalem a seis animações

A source de `LoadCharactor()` faz:

```cpp
int sframe =
    AnimPat->TmFrame[
        modelInfo.MotionInfo[cnt].MotionFrame - 1
    ].StartFrame / 160;

modelInfo.MotionInfo[cnt].StartFrame += sframe;
modelInfo.MotionInfo[cnt].EndFrame += sframe;
```

Isso prova que:

```text
TmFrame slot/range = contêiner de motion file
MotionInfo         = animação lógica real
MotionFrame        = seleciona qual range global contém a animação
```

Portanto NÃO importar:

```text
slot1 inteiro como uma animação
```

sem antes decodificar `smMOTIONINFO`.

---

# 316. Próximo gate

Antes de gerar keyframes Blender:

```text
1. obter definição exata de smMOTIONINFO;
2. obter sizeof/offsets relevantes;
3. decodificar MotionInfo[] do M5Bip.inx;
4. aplicar o ajuste MotionFrame -> TmFrame.StartFrame/160;
5. listar animações lógicas reais com StartFrame/EndFrame.
```

Somente depois escolher uma animação real para o primeiro teste.
---

# 317. smMOTIONINFO — layout lógico PROVADO pela source

Definição:

```cpp
struct smMOTIONINFO
{
    DWORD State;

    DWORD MotionKeyWord_1;
    DWORD StartFrame;
    DWORD MotionKeyWord_2;
    DWORD EndFrame;

    DWORD EventFrame[4];

    int  ItemCodeCount;
    BYTE ItemCodeList[MOTION_TOOL_MAX];

    DWORD dwJobCodeBit;
    BYTE SkillCodeList[MOTION_SKIL_MAX];

    int   MapPosition;

    DWORD Repeat;
    CHAR  KeyCode;
    int   MotionFrame;
};
```

Ainda não marcar `sizeof(smMOTIONINFO)` até obter:

```text
MOTION_TOOL_MAX
MOTION_SKIL_MAX
```

e confirmar padding i686.

---

# 318. MotionFrame de motion normal é 1-based em relação a TmFrame

Durante parsing do INI:

```cpp
Info->MotionInfo[Info->MotionCount].MotionFrame = MotionFileListCnt;
```

No carregamento do personagem:

```cpp
int sframe =
    AnimPat->TmFrame[
        modelInfo.MotionInfo[cnt].MotionFrame - 1
    ].StartFrame / 160;

modelInfo.MotionInfo[cnt].StartFrame += sframe;
modelInfo.MotionInfo[cnt].EndFrame += sframe;
```

Conclusão:

```text
MotionFrame = 1 -> TmFrame[0]
MotionFrame = 2 -> TmFrame[1]
...
```

para `MotionInfo` normal.

Atenção: `TalkMotionInfo` possui regra diferente e não deve ser misturado.

---

# 319. Próximo gate

Antes de parsear `MotionInfo[]` do INX:

```text
1. obter MOTION_TOOL_MAX;
2. obter MOTION_SKIL_MAX;
3. obter MOTION_INFO_MAX;
4. obter sizeof/offsets i686 de smMOTIONINFO;
5. auditar ModelKeyWord/MotionKeyWord decode do cache INX.
```

Não interpretar os DWORD crus do INX antes de confirmar a etapa de decode.
---

# 320. Limites de smMOTIONINFO — PROVADOS pela source

```text
MOTION_INFO_MAX      = 512
MOTION_TOOL_MAX      = 104
MOTION_SKIL_MAX      = 8
TALK_MOTION_INFO_MAX = 30
```

Com a definição já obtida de `smMOTIONINFO`, o layout i686 natural esperado é:

```text
0x00 State                DWORD
0x04 MotionKeyWord_1      DWORD
0x08 StartFrame           DWORD
0x0C MotionKeyWord_2      DWORD
0x10 EndFrame             DWORD
0x14 EventFrame[4]        16 bytes
0x24 ItemCodeCount        int
0x28 ItemCodeList[104]    104 bytes
0x90 dwJobCodeBit         DWORD
0x94 SkillCodeList[8]     8 bytes
0x9C MapPosition          int
0xA0 Repeat               DWORD
0xA4 KeyCode              CHAR
0xA5..0xA7 padding
0xA8 MotionFrame          int
```

Tamanho esperado:

```text
0xAC = 172 bytes
```

Estado:

```text
layout lógico: PROVADO
sizeof=172: DERIVADO / ainda não marcado ABI-PROVADO
```

---

# 321. SetIniMotionInfo — semântica de frames PROVADA

A source lê:

```cpp
motion->StartFrame = atoi(strBuff);
motion->EndFrame   = atoi(strBuff);
```

Para cada event frame:

```cpp
motion->EventFrame[EventCount] =
    (atoi(strBuff) - motion->StartFrame) * 160;
```

Também define:

```text
Repeat
KeyCode
NpcMotionRate opcional
```

Portanto `StartFrame/EndFrame` do model-info são inicialmente frames lógicos
locais; o deslocamento do motion-file é aplicado posteriormente por
`LoadCharactor()` através de `MotionFrame`.

---

# 322. Cache INX possui encode/decode — parser cru bloqueado

Antes de escrever:

```cpp
ModelKeyWordEncode(dwCode, Info);

if(Info->MotionCount > CHRMOTION_EXT &&
   Info->TalkMotionCount <= CHRMOTION_EXT)
{
    MotionKeyWordEncode(dwCode, Info);
}

fwrite(Info, sizeof(smMODELINFO), 1, fp);
```

No carregamento:

```cpp
ModelKeyWordDecode(dwCode, &smModelInfo);
```

e para motion info vindo do link:

```cpp
MotionKeyWordDecode(dwCode, &smModelInfo);

CopyMemory(
    Info->MotionInfo,
    smModelInfo.MotionInfo,
    sizeof(smModelInfo.MotionInfo)
);
```

Conclusão:

```text
não interpretar semanticamente MotionInfo[] bruto do INX
até reproduzir os decoders.
```

---

# 323. Próximo gate

Localizar a implementação real de:

```text
ModelKeyWordEncode
ModelKeyWordDecode
MotionKeyWordEncode
MotionKeyWordDecode
```

em TODO o SourceCode, não apenas `SrcGame/src`.

Também obter:

```text
CHRMOTION_EXT
```

Depois disso será possível preparar o primeiro decoder Python confiável de
`M5Bip.inx`.
---

# 324. CHRMOTION_EXT — PROVADO

```cpp
#define CHRMOTION_EXT 10
```

Consequência:

```text
MotionInfo[0..9] = faixa reservada/base do sistema
MotionInfo[10..MotionCount-1] = motions adicionais processados por
                                MotionKeyWordEncode/Decode
```

---

# 325. Implementação de ModelKeyWordEncode/Decode — encontrada

`ModelKeyWordEncode()` não cifra o model-info inteiro.

Ele constrói:

```text
FileTypeKeyWord
LinkFileKeyWord
```

misturando bytes de:

```text
GetSpeedSum(current INX path)
GetSpeedSum(link INX path)
```

`ModelKeyWordDecode()` desfaz a composição e apenas valida os códigos.

Portanto:

```text
ModelKeyWord* = integridade/validação de filename/link
```

não transformação dos demais campos.

---

# 326. MotionKeyWordEncode — algoritmo PROVADO

Para:

```text
i = CHRMOTION_EXT .. MotionCount-1
```

se `StartFrame != 0`, o encoder mistura bytes de:

```text
dwCode
StartFrame
```

entre:

```text
MotionKeyWord_1
StartFrame
```

Se `EndFrame != 0`, mistura:

```text
dwCode
EndFrame
```

entre:

```text
MotionKeyWord_2
EndFrame
```

---

# 327. MotionKeyWordDecode — algoritmo PROVADO

O decoder reconstrói:

```text
KeyWord
Frame
```

a partir dos dois DWORDs misturados.

Para StartFrame:

```text
MotionKeyWord_1 + StartFrame
```

Para EndFrame:

```text
MotionKeyWord_2 + EndFrame
```

Depois exige:

```cpp
if(dwCode != KeyWord)
    return FALSE;
```

e finalmente restaura:

```text
MotionKeyWord_1 = 0
StartFrame      = Frame

MotionKeyWord_2 = 0
EndFrame        = Frame
```

Portanto o parser Python pode reproduzir exatamente o client quando o
`dwCode` correto estiver disponível.

---

# 328. Próximo bloqueio único — GetSpeedSum

O cache usa:

```cpp
DWORD dwCode = GetSpeedSum(szFile);
```

onde `szFile` é o caminho do INX.

O loader MinGW atual também normaliza o path antes de chamar `GetSpeedSum`.

Logo, antes de decodificar `M5Bip.inx`, obter:

```text
implementação exata de GetSpeedSum
```

e confirmar a normalização/case sensitivity do caminho.

Depois disso o decoder Python do INX pode ser construído sem hipótese.
---

# 329. GetSpeedSum — implementação PROVADA pela source

A função:

```cpp
DWORD GetSpeedSum(const char *szName)
```

itera cada byte do texto.

Para `a..z`, converte para maiúscula antes da soma:

```text
value = ch - 0x20
```

e acumula:

```text
Sum2  += value * (cnt + 1)
dwSum += value * (cnt * cnt)
```

No final:

```cpp
return (dwSum << 24) | (Sum1 << 16) | Sum2;
```

onde:

```text
Sum1 = comprimento em bytes
```

Implicações:

```text
ASCII case-insensitive para letras;
slash e backslash NÃO são equivalentes;
path canônico importa;
overflow é DWORD/32-bit.
```

---

# 330. Decoder INX agora pode reproduzir a source

Combinando:

```text
GetSpeedSum
ModelKeyWordDecode
MotionKeyWordDecode
smMOTIONINFO layout
CHRMOTION_EXT=10
```

é possível decodificar o `M5Bip.inx` sem hipótese criptográfica.

Preparado:

```text
pt_inx_motion_decode_probe_v1.py
```

O probe:

```text
lê M5Bip.inx;
valida size smMODELINFO=95268;
usa smMOTIONINFO=172 como hipótese ABI verificável;
lê MotionCount no offset derivado;
reconstrói dwFileCode dos keywords do próprio INX;
compara GetSpeedSum do logical path;
valida o código do link;
reproduz MotionKeyWordDecode;
mapeia MotionFrame -> m5.smb TmFrame[MotionFrame-1];
imprime frames locais e globais;
não modifica arquivos;
não cria keyframes.
```

Se:

```text
MotionCount for plausível;
logical_path_code_match=YES;
link_path_code_match=YES;
decode_errors=0;
```

o layout 172 bytes ganha validação estrutural muito forte.

---

# 331. Gate antes de Blender animation

Ainda não associar `State` numérico a nomes de movimento.

Primeiro obter a tabela real de motions e garantir:

```text
inx_motion_decode_check=PASS
```

Depois mapear `CHRMOTION_STATE_*` para nomes e escolher uma única animação
lógica para o primeiro teste.
---

# 332. INX motion decode V1 — PROVADO EM EXECUÇÃO

Para:

```text
M5Bip.inx
m5.smb
logical path = char\tmABCD\M5Bip.inx
```

resultado:

```text
file_size=95268

smMODELINFO_size=95268
smMOTIONINFO_size_assumed=172
MotionInfo_offset=0x018C
MotionCount_offset=0x1598C
MotionCount=64
```

Keyword validation:

```text
embedded_file_code=0xF4154175
logical_path_code=0xF4154175
logical_path_code_match=YES

embedded_link_code=0xF4154175
calculated_link_code=0xF4154175
link_path_code_match=YES
```

Validação final:

```text
decode_errors=0
frame_errors=0
abi_layout_consistent=YES
keyword_paths_consistent=YES
motion_decode_consistent=YES
motion_ranges_consistent=YES
inx_motion_decode_check=PASS
probe_exit=0
```

Estado:

```text
smMOTIONINFO sizeof=172: PROVADO ESTRUTURALMENTE para este cache/ABI
MotionCount offset 0x1598C: PROVADO
GetSpeedSum reproduction: PROVADO
Model keyword reconstruction: PROVADO
MotionKeywordDecode reproduction: PROVADO
MotionFrame -> SMB range mapping: PROVADO
```

---

# 333. MotionInfo[0..9] reservado/vazio neste M5Bip.inx

Os índices:

```text
0..9
```

possuem:

```text
State=0
MotionFrame=0
StartFrame=0
EndFrame=0
```

Isso é compatível com:

```cpp
#define CHRMOTION_EXT 10
```

e com o loader inicializando:

```text
MotionCount = CHRMOTION_EXT
```

antes de mesclar motions adicionais.

As motions efetivas deste INX são:

```text
10..63
```

total:

```text
54 motions
```

---

# 334. MotionFrame 2..7 usa exatamente os seis ranges ativos

Distribuição observada:

```text
MotionFrame=2 -> TmFrame[1]
MotionFrame=3 -> TmFrame[2]
MotionFrame=4 -> TmFrame[3]
MotionFrame=5 -> TmFrame[4]
MotionFrame=6 -> TmFrame[5]
MotionFrame=7 -> TmFrame[6]
```

`TmFrame[0]` é vazio.

Portanto as 54 motions lógicas estão distribuídas nos seis containers ativos
do `m5.smb`.

---

# 335. Próximo gate — mapear State numérico para nome

Valores observados incluem:

```text
0x00000040
0x00000050
0x00000060
0x00000080
0x00000100
0x00000110
0x00000120
0x00000140
0x00000150
0x00000170
0x00000180
0x00000200
0x00000220
```

Não atribuir nomes por inferência.

Auditar definições:

```text
CHRMOTION_STATE_*
```

na source e então escolher uma única motion lógica real para o primeiro teste
de animação Blender.
---

# 336. CHRMOTION_STATE — mapeamento numérico PROVADO

Client e server possuem o mesmo conjunto:

```text
0x040 STAND
0x050 WALK
0x060 RUN
0x080 FALLDOWN

0x100 ATTACK
0x110 DAMAGE
0x120 DEAD
0x130 SOMETIME
0x140 EAT
0x150 SKILL
0x170 FALLSTAND
0x180 FALLDAMAGE

0x200 RESTART
0x210 WARP
0x220 YAHOO

0x300 HAMMER
```

Também existem states `0x400+` para talk/facial motions.

---

# 337. Distribuição das 54 motions reais do M5Bip.inx

Com o decode já validado:

```text
STAND       9
WALK        7
RUN         7
FALLDOWN    1
ATTACK     11
DAMAGE      1
DEAD        1
EAT         1
SKILL      12
FALLSTAND   1
FALLDAMAGE  1
RESTART     1
YAHOO       1
----------------
TOTAL      54
```

Não aparecem no conjunto decodificado atual:

```text
SOMETIME
WARP
HAMMER
```

Isso não significa ausência no engine; apenas ausência entre `MotionInfo[10..63]`
deste `M5Bip.inx`.

---

# 338. Primeiro candidato para teste Blender

Candidato:

```text
MotionInfo[10]
State       = STAND (0x40)
MotionFrame = 2
SMB slot    = 1
local       = 10..50
global      = 10..50
Repeat      = 1
```

Motivo:

```text
motion curta;
looping;
state simples;
range cedo no primeiro container ativo.
```

A função exata de `ItemCodeCount=1` ainda não precisa ser resolvida para testar
a matemática da animação. O teste pretende validar apenas o transform skeleton.

---

# 339. Gate obrigatório antes de gerar keyframes

Auditar integralmente:

```text
smOBJ3D::TmAnimation
smOBJ3D::GetRotFrame
smOBJ3D::GetPosFrame
smOBJ3D::GetScaleFrame
quaternion interpolation usada por GetRotFrame
quaternion -> matrix
matrix multiplication order
parent propagation
```

Não aplicar animação Blender até essa matemática estar fechada pela source.
---

# 340. GetRotFrame — interpolação real PROVADA pela source

A implementação carrega:

```cpp
ax = tmRot[cnt].x;
ay = tmRot[cnt].y;
az = tmRot[cnt].z;
aw = tmRot[cnt].w;

bx = tmRot[cnt + 1].x;
by = tmRot[cnt + 1].y;
bz = tmRot[cnt + 1].z;
bw = tmRot[cnt + 1].w;
```

mas a chamada efetiva é:

```cpp
D3DMath_QuaternionSlerp(
    qx, qy, qz, qw,
    0, 0, 0, 0,
    bx, by, bz, bw,
    alpha
);
```

Portanto o quaternion `tmRot[cnt]` NÃO é passado como origem do slerp.

Em seguida:

```cpp
smFMatrixFromQuaternion(gmat, qx, qy, qz, qw);
smFMatrixMult(gmat, PrevRot[cnt], gmat);
```

Conclusão:

```text
rotation result =
    PrevRot[cnt] *
    quaternion_delta_interpolated_from_zero_to_tmRot[cnt+1]
```

Não substituir isso por `slerp(tmRot[cnt], tmRot[cnt+1])` no Blender.

`TmPrevRot[]` é parte essencial da reprodução fiel.

---

# 341. GetPosFrame — interpolação linear PROVADA

Para:

```text
s <= frame < e
alpha = (frame - s) / (e - s)
```

o engine calcula:

```text
x = x0 + (x1-x0)*alpha
y = y0 + (y1-y0)*alpha
z = z0 + (z1-z0)*alpha
```

Sem easing.

---

# 342. Quaternion -> matrix — fórmula PROVADA

`smFMatrixFromQuaternion()` usa diretamente `x,y,z,w`:

```text
_11 = 1 - 2(yy+zz)
_12 = 2(xy-wz)
_13 = 2(xz+wy)

_21 = 2(xy+wz)
_22 = 1 - 2(xx+zz)
_23 = 2(yz-wx)

_31 = 2(xz-wy)
_32 = 2(yz+wx)
_33 = 1 - 2(xx+yy)
```

Translation é inicialmente zero.

Não converter o quaternion diretamente para `mathutils.Quaternion` antes de
validar convenções de matriz. A estratégia mais segura é reproduzir a matriz
do engine primeiro.

---

# 343. Parent propagation — ordem PROVADA

No overload normal de `TmAnimation`:

```cpp
if(pParent)
    smFMatrixMult(TmResult, qmat, pParent->TmResult);
else
    TmResult = qmat;
```

Logo:

```text
TmResult(child) = qmat_local * TmResult(parent)
```

Isso é consistente com o pipeline row-vector observado anteriormente.

Quando não existe track ativo:

```cpp
smMatrixMult(Mat, Tm, pParent->TmInvert);
smFMatrixFromMatrix(qmat, Mat);
```

Portanto para child sem animação ativa:

```text
qmat_local_bind = child.Tm * inverse(parent.Tm)
```

e depois:

```text
TmResult = qmat_local_bind * parent.TmResult
```

---

# 344. D3DMath_QuaternionSlerp contém peculiaridade/bug legado

A source contém:

```cpp
if(fCosTheta < 0.0f)
{
    Ax = -Ax;
    Ay = -Ay;
    Ax = -Az;
    Aw = -Aw;
    fCosTheta = -fCosTheta;
}
```

O terceiro assignment escreve `Ax = -Az`, não `Az = -Az`.

Isso parece typo legado, mas NÃO corrigir silenciosamente numa reprodução
source-equivalent.

No caminho usado por `GetRotFrame`, porém, A é explicitamente:

```text
(0,0,0,0)
```

portanto:

```text
fCosTheta = 0
```

para qualquer B, e esse branch negativo não é alcançado nesse uso específico.

---

# 345. Gates restantes antes do primeiro evaluator

Ainda obter da source:

```text
1. final completo do overload normal TmAnimation;
2. implementação completa de smFMatrixMult;
3. relação exata FrameCounter / StartFrame / fator 160;
4. chamada smPAT3D -> obj3d TmAnimation e ordem da hierarquia.
```

Especialmente:

```text
MotionInfo local/global 10..50
```

não deve ser passado como `10..50` ao evaluator até provar se o engine chama
`TmAnimation` com:

```text
frame
```

ou:

```text
frame * 160
```

naquele estágio.
---

# 346. Unidade temporal do evaluator — PROVADA

Ao selecionar uma motion:

```cpp
frame = MotionInfo->StartFrame * 160;
```

Durante o update:

```cpp
frame += FrameStep;
```

e o mesmo valor é passado para:

```cpp
smPAT3D::TmAnimation(frame, ...)
```

que por sua vez chama:

```cpp
obj3d[TmSort[cnt]]->TmAnimation(frame, ...)
```

Portanto os `smTM_ROT.frame` / `smTM_POS.frame` e `TmFrame.StartFrame/EndFrame`
trabalham na escala fixa:

```text
1 logical motion frame = 160 animation units
```

Para `MotionInfo[10]`:

```text
State = STAND
logical local/global = 10..50

engine evaluator range:
1600..8000
```

---

# 347. smFMatrixMult — ordem PROVADA

Implementação:

```cpp
pM[4*j+i] += pA[4*j+k] * pB[4*k+i];
```

Logo:

```text
smFMatrixMult(m,a,b) => m = a * b
```

Consequentemente:

```text
TmResult(child) = qmat_local * TmResult(parent)
```

é literal, não ordem invertida escondida pela helper.

---

# 348. mWorld sem rotação externa — PROVADO

No final de `smOBJ3D::TmAnimation`:

```cpp
if(ax == 0 && ay == 0 && az == 0)
{
    smIdentityMatrix(mLocal);
    smMatrixFromFMatrix(mWorld, TmResult);
}
```

Assim, para o probe Blender/source-equivalent com ângulos externos zero:

```text
bone.mWorld = fixed-point(TmResult)
```

Não precisamos reproduzir as rotações externas de personagem/câmera para
validar a animação skeletal local.

---

# 349. Ordem de avaliação do pattern

`smPAT3D::TmAnimation()` executa:

```cpp
for(cnt = 0; cnt < nObj3d; cnt++)
    obj3d[TmSort[cnt]]->TmAnimation(...);
```

Logo a ordem serial do array `obj3d[]` não é usada diretamente.

Ainda falta auditar a construção de:

```text
TmSort[]
```

antes de marcar formalmente que ele é parent-first.

---

# 350. Dois micro-gates restantes antes do evaluator V1

Obter:

```text
FrameInfo.EndFrame assignment
TmSort construction
```

Depois gerar evaluator read-only para `MotionInfo[10]` em unidades
`1600..8000`, sem Blender.
---

# 351. FrameInfo.StartFrame / EndFrame — PROVADOS

Ao selecionar uma motion:

```cpp
frame = MotionInfo->StartFrame * 160;
...
FrameInfo.StartFrame = frame;
FrameInfo.EndFrame = MotionInfo->EndFrame * 160;
```

Para `MotionInfo[10]`:

```text
StartFrame lógico = 10
EndFrame lógico   = 50

FrameInfo.StartFrame = 1600
FrameInfo.EndFrame   = 8000
```

Isso fecha a janela usada por `GetRotFrame()`.

---

# 352. Clamp de rotação no fim da motion

`GetRotFrame()` contém:

```cpp
if(lpFrameInfo && e > (int)lpFrameInfo->EndFrame)
{
    CopyMemory(&gmat, &PrevRot[cnt], sizeof(smFMATRIX));
    return s;
}
```

Portanto, para a reprodução de uma `MotionInfo` real:

```text
uma key de rotação posterior ao EndFrame não deve ser interpolada
através do limite lógico da motion.
```

Isso é diferente do `GetPosFrame()`, que não recebe `lpFrameInfo` e não possui
esse clamp específico.

---

# 353. TmSort parent-first — PROVADO pela construção

`LinkObject()` primeiro resolve `NodeParent -> pParent`.

Depois:

```cpp
for(cnt = 0; cnt < 128; cnt++)
    TmSort[cnt] = 0;

for(cnt = 0; cnt < nObj3d; cnt++)
{
    if(obj3d[cnt]->pParent == 0)
        TmSort[NodeCnt++] = cnt;
}

for(cnt2 = 0; cnt2 < nObj3d; cnt2++)
{
    for(cnt = 0; cnt < nObj3d; cnt++)
    {
        if(obj3d[cnt]->pParent != 0 &&
           obj3d[TmSort[cnt2]] == obj3d[cnt]->pParent)
        {
            TmSort[NodeCnt++] = cnt;
        }
    }
}
```

Conclusão:

```text
roots primeiro;
depois filhos dos nodes já inseridos;
parent sempre é avaliado antes do child em skeleton válido.
```

`smPAT3D::TmAnimation()` percorre exatamente esse `TmSort`.

---

# 354. Primeiro evaluator source-equivalent preparado

Arquivo:

```text
pt_smb_motion_evaluator_v1.py
```

Escopo:

```text
read-only;
sem Blender;
decodifica uma MotionInfo do INX;
default MotionInfo[10];
reproduz GetSpeedSum;
reproduz MotionKeyWordDecode;
mapeia MotionFrame -> TmFrame;
reconstrói TmSort;
avalia 95 bones em ordem parent-first;
reproduz GetTmFrameRot/Pos/Scale;
reproduz GetRotFrame + TmPrevRot;
reproduz GetPosFrame linear;
reproduz GetScaleFrame;
usa FrameInfo.EndFrame no clamp de rotação;
propaga TmResult = qmat_local * parent.TmResult;
mede bbox, finitude, affine form e deslocamentos.
```

Para `motion[10]` o intervalo runtime é tratado como:

```text
[1600, 8000)
```

e o probe avalia cada frame lógico:

```text
10..49
```

em passos de:

```text
160 engine units.
```

O teste não pretende ser bit-idêntico a uma FPU x86 específica; ele reproduz
a matemática/branches da source para validar a estrutura antes do Blender.
---

# 355. Motion evaluator V1 — PASS para STAND motion[10]

Execução:

```text
MotionCount=64
index=10
State=STAND
MotionFrame=2
TmFrame_slot=1
engine_half_open=[1600,8000)
bones=95
TmSort_parent_first=YES
```

Diagnóstico:

```text
logical_frames_scanned=40
engine_frame_first=1600
engine_frame_last=7840
engine_frame_end_exclusive=8000

nonfinite_matrices=0
max_affine_error=0
det3_range=0.988157698..1.01388722

source_evaluator_structural_check=PASS
probe_exit=0
```

Estado:

```text
source-equivalent evaluator estrutural: PROVADO para motion[10]
```

O PASS é estrutural/numerical; a fidelidade visual ainda precisa ser validada
no Blender.

---

# 356. FrameInfo.EndFrame clamp observado em execução

Agregado para os 40 samples:

```text
rot_end_clamps=58
```

Exemplos:

```text
logical=10 -> rot_end_clamps=1
logical=49 -> rot_end_clamps=4
```

Isso confirma que o branch:

```cpp
if(lpFrameInfo && e > lpFrameInfo->EndFrame)
    gmat = PrevRot[cnt];
```

não é teórico: ele participa efetivamente desta STAND.

---

# 357. Pose STAND possui envelope numérico coerente

Samples mostraram bbox de joints estável:

```text
logical 10:
x[-7.7774, 7.3847]
y[-8.7518,11.1064]
z[-0.0168,45.3795]

logical 49:
x[-7.7816, 7.3816]
y[-8.7516,11.0682]
z[-0.0167,45.3792]
```

Vários joints principais retornam próximos da pose inicial no final do ciclo,
incluindo:

```text
Head
L/R Hand
L/R Foot
Bip weapon01
```

Isso é compatível com uma motion STAND looping.

---

# 358. Primeiro bake Blender será incremental sobre V3 estático

Não reimportar SMD/materials.

Entrada:

```text
MfbC01_direct_smd_v3_static_armature.blend
```

Novo script:

```text
pt_blender_apply_motion_v1.py
```

Ele:

```text
abre o .blend estático já validado;
lê m5.smb e M5Bip.inx;
decodifica motion[10];
avalia 40 samples source-equivalent;
aplica os 95 bones;
grava somente a animação em novo .blend.
```

---

# 359. Conversão source deformation -> Blender armature

A malha Blender está em bind/world pose:

```text
p_bind = p_local * source_rest
```

O source animado produz:

```text
p_pose = p_local * source_pose
```

Logo, em row-vector:

```text
D_row = inverse(source_rest) * source_pose
```

Em Blender column-vector:

```text
D_col = source_pose^T * inverse(source_rest^T)
```

O Armature modifier aplica aproximadamente:

```text
pose_bone.matrix * bone.matrix_local^-1
```

Então o bake define:

```text
pose_bone.matrix = D_col * bone.matrix_local
```

Consequência importante:

```text
a deformação não depende do tail/roll de exibição aproximado criado no V3.
```

Ou seja, os 95 bones podem manter a aparência/hierarquia do armature estático
enquanto a deformação usa o transform source real.

---

# 360. Escopo do primeiro Blender animation test

Somente:

```text
MotionInfo[10]
STAND
40 logical frames
Blender frames 1..40
sample step = 160 source units
```

Ainda NÃO importar as outras 53 motions.

Validações do bake:

```text
95 bones presentes;
matrizes source finitas;
erro de assignment pose bone;
bbox das meshes INX-visible em frames 1/mid/final;
Action criada;
novo .blend salvo.
```

Só marcar fidelidade visual da animação após inspeção do arquivo no Blender.
---

# 361. Primeiro bake Blender animado — NÃO aprovado visualmente

Observação do usuário:

```text
personagem completamente deformado nos primeiros frames;
posteriormente a motion parece corresponder a idle.
```

Como `MotionInfo[10].State == CHRMOTION_STATE_STAND`, a interpretação
"parece idle" é compatível com a identificação source-driven da motion.

Porém:

```text
o bake V1 NÃO é considerado visualmente correto.
```

Não marcar a etapa Blender animation como PROVADA.

---

# 362. Separação de responsabilidades após a falha visual

O evaluator read-only anterior permanece estruturalmente consistente:

```text
source_evaluator_structural_check=PASS
nonfinite_matrices=0
max_affine_error=0
```

Portanto a próxima hipótese a testar NÃO é corrupção do SMB.

O suspeito imediato é a ponte:

```text
source TmResult
    ->
Blender pose_bone.matrix
    ->
decomposição location/quaternion/scale
    ->
Armature modifier
```

Não avançar para as outras 53 motions até isolar essa etapa.

---

# 363. Direct Pose Probe V1

Novo teste:

```text
pt_blender_direct_pose_probe_v1.py
```

Ele abre o `.blend` V3 estático já validado e aplica apenas:

```text
MotionInfo[10]
engine frame 1600
logical frame 10
```

sem criar qualquer keyframe.

O probe mede diretamente:

```text
actual Blender bone deformation =
    pose_bone.matrix * inverse(bone.matrix_local)
```

contra a deformação desejada calculada da source:

```text
source_deform =
    source_pose_col * inverse(source_rest_col)
```

Também salva um `.blend` com apenas essa pose estática para inspeção visual.

Interpretação:

```text
se o direct pose já estiver deformado:
    erro está na conversão source->Blender / deform formula / coordinate basis.

se o direct pose estiver correto:
    erro está no bake/decomposição/interpolação das channels Blender.
```

Este teste é deliberadamente anterior a qualquer nova Action.
---

# 364. Direct Pose Probe V1 — FAIL e interpretação

Resultado observado:

```text
max_effective_deform_error=58.3323221
max_effective_deform_error_bone='Bip-h23'
effective_deform_det_range=0.992603183..12.1660004

bind_bbox:
x[-8.1523,8.1445]
y[-5.7266,5.2617]
z[0.0000,37.5078]

direct_pose_bbox:
x[-12.9706,6.3849]
y[-23.9984,3.3129]
z[-1.6275,34.9400]

effective_deform_assignment=FAIL
direct_pose_probe_check=FAIL
```

A inspeção visual confirmou deformação forte.

Conclusão:

```text
não é erro de keyframes;
o probe não cria keys;
a fórmula/representação source TmResult -> Blender pose_bone.matrix
não está reproduzindo a deformação desejada.
```

O evaluator source continua separado desta falha.

---

# 365. Timeline >240 frames no direct pose não significa animação

O direct-pose probe:

```text
no_keyframes=YES
```

e não altera `scene.frame_end`.

Portanto o range grande mostrado no Blender é o range herdado do `.blend`
estático, não centenas de keys criadas pelo probe.

---

# 366. Próximo isolamento — source skinning direto na mesh

Novo probe:

```text
pt_blender_direct_source_skin_v1.py
```

Ele ignora completamente o sistema de pose bones para deformação:

```text
SMD Physique local vertex
    *
source TmResult do bone
    =
vertex world pose
```

Depois escreve diretamente a posição resultante no mesh Blender e desabilita
o Armature modifier no arquivo diagnóstico.

Sem:

```text
pose_bone.matrix
location/quaternion/scale
keyframes
Armature modifier
```

Objetivo:

```text
se a mesh direta ficar correta:
    evaluator + SMD Physique -> TmResult estão corretos;
    problema fica isolado na tradução para armature Blender.

se a mesh direta também deformar:
    revisar evaluator/convenção de matriz antes de qualquer novo bake.
```
---

# 367. Direct source skin — aprovado visualmente

Observação do usuário:

```text
a pose direta apareceu em idle
```

Como esse probe desabilita Armature modifier e calcula diretamente:

```text
SMD Physique local vertex * source TmResult
```

isso valida visualmente a cadeia:

```text
M5Bip.inx
-> motion[10] STAND
-> m5.smb tracks
-> evaluator source-equivalent
-> TmResult por bone
-> SMD Physique local vertex
-> pose world
```

Estado:

```text
source evaluator + source skinning:
PROVADO VISUALMENTE para STAND motion[10] frame 1600.
```

A falha de deformação não está no SMB nem no Physique local.

---

# 368. Defeito agora isolado à tradução para Blender Armature

Falhou:

```text
source TmResult -> pose_bone.matrix -> Armature modifier
```

Passou:

```text
source TmResult -> direct mesh vertex
```

Portanto o próximo teste deve validar a equivalência:

```text
Armature modifier output
vs.
source direct skin output
```

vértice por vértice.

---

# 369. Armature Mapping Probe V2

Novo arquivo:

```text
pt_blender_armature_mapping_probe_v2.py
```

Para um único frame:

```text
motion[10]
engine frame 1600
```

ele:

```text
usa bone.convert_local_to_pose(..., invert=True)
para calcular matrix_basis respeitando parent/rest/inherit-scale;

não cria keyframes;

avalia o Armature modifier;

calcula ground truth dos mesmos vértices pela fórmula source;

compara cada vertex Blender vs source;
mede max e RMS error;
compara bbox;
salva novo .blend diagnóstico.
```

Critério forte:

```text
pose_matrix_mapping=PASS
armature_vs_source_skin=PASS
armature_mapping_probe_check=PASS
```

Somente depois desse PASS refazer o bake da Action.
---

# 370. Armature Mapping Probe V2 — resultado numérico

Para:

```text
motion[10] STAND
engine frame 1600
```

resultado:

```text
POSE_ASSIGNMENT
max_error_during_assignment=0.587631486
max_error_during_assignment_bone='Bip weapon02'
final_pose_max_error=0.587631486
final_pose_max_error_bone='Bip weapon02'
```

Porém o critério geometricamente mais importante ficou muito próximo da source:

```text
VERTEX_COMPARISON
total_vertices=245
max_vertex_error=0.0217075676
rms_vertex_error=0.00728398075
```

Por objeto:

```text
MbmC01 max=0.0164394186 rms=0.00513597241
MlmC01 max=0.00396071551 rms=0.00213614052
MsmC01 max=0.0217075676 rms=0.00919788691
```

---

# 371. BBox Armature V2 praticamente coincide com source

Ground truth source:

```text
x[-8.3705,7.9991]
y[-9.2923,5.4636]
z[0.0091,37.2498]
```

Armature Blender:

```text
x[-8.3788,8.0125]
y[-9.2922,5.4641]
z[0.0091,37.2486]
```

Diferenças máximas de envelope são da ordem de centésimos.

Isso é radicalmente melhor que o Armature V1, cujo frame 1600 produziu bbox
visivelmente deformado.

---

# 372. Interpretação correta do FAIL do probe V2

O probe usava thresholds deliberadamente estritos:

```text
pose_matrix_mapping:
    final_pose_max_error < 1e-4

armature_vs_source_skin:
    max_vertex_error < 1e-3
```

Os dois foram excedidos, então ele imprimiu FAIL.

Isso NÃO equivale à deformação catastrófica do V1.

O critério de skinning real observado é:

```text
max vertex error ~= 0.0217
RMS vertex error ~= 0.00728
bbox quase idêntico
```

Portanto o estado atual é:

```text
Armature Mapping V2:
numericamente MUITO PRÓXIMO da source;
ainda aguardando aprovação visual antes de marcar PROVADO.
```

O erro de pose matrix isolado em `Bip weapon02` não deve ser usado sozinho
para rejeitar o skinning, pois o resultado dos vertices visíveis é muito mais
próximo da ground truth.

---

# 373. Próximo gate

Abrir:

```text
MfbC01_armature_mapping_v2_frame1600.blend
```

e validar visualmente a pose única.

Se a personagem estiver normal em idle:

```text
source -> Blender armature mapping V2
```

fica validado para frame 1600 e o próximo passo será gerar a Action STAND
frames 1..40 usando `matrix_basis` calculado via
`bone.convert_local_to_pose(..., invert=True)`.

Se ainda houver deformação visível, não bakear a Action; investigar apenas os
bones/vertices com maior erro.
---

# 374. Armature Mapping V2 — aprovado visualmente

Observação do usuário:

```text
ela ficou em idle
```

Isso fecha o gate visual do frame 1600 usando:

```text
source TmResult
-> desired Blender pose matrix
-> bone.convert_local_to_pose(..., invert=True)
-> PoseBone.matrix_basis
-> Armature modifier
```

Estado:

```text
source -> Blender Armature Mapping V2:
PROVADO VISUALMENTE para motion[10], engine frame 1600.
```

O pequeno erro vertex-vs-source medido anteriormente permanece documentado,
mas não produz deformação visual significativa no frame validado.

---

# 375. Motion Bake V2 preparado

Novo arquivo:

```text
pt_blender_motion_bake_v2.py
```

Escopo:

```text
MotionInfo[10] STAND
source engine [1600,8000)
40 logical samples
Blender frames 1..40
95 bones
```

Método por frame:

```text
1. evaluate source TmResult;
2. source deformation -> desired Blender pose;
3. Bone.convert_local_to_pose(..., invert=True);
4. set PoseBone.matrix_basis;
5. keyframe location / rotation_quaternion / scale.
```

A timeline é explicitamente limitada a:

```text
1..40
```

evitando herdar o range >240 frames do arquivo estático.

---

# 376. Validação pós-bake contra ground truth source

Após criar a Action, o script percorre novamente todos os 40 frames através
do Blender e compara:

```text
Armature modifier evaluated vertex
vs.
SMD Physique local vertex * source TmResult
```

para todos os vertices das meshes INX-visible.

Métricas:

```text
max_vertex_error
overall_rms_vertex_error
worst_frame
worst_rms_frame
bbox source vs Blender em frames representativos
```

Gate inicial conservador, baseado no frame único V2 já validado:

```text
max_vertex_error <= 0.05
overall_rms_vertex_error <= 0.02
```

Somente após esse PASS e inspeção visual da Action marcar a STAND inteira como
PROVADA no Blender.
---

# 377. STAND motion[10] — bake V2 PASS em todos os 40 samples

Execução:

```text
motion_index=10
state=STAND
repeat=1
motion_frame=2
engine_half_open=[1600,8000)
blender_frames=1..40
bones=95
```

Validação pós-bake:

```text
action=PT_m5_STAND_motion10_V2
keyed_bones=95
keyed_frames=40
total_vertex_samples=9800

max_vertex_error=0.0217075676
overall_rms_vertex_error=0.00727748095

post_bake_source_skin_check=PASS
blender_motion_bake_v2_check=PASS
blender_exit=0
```

O pior erro máximo ocorreu no primeiro sample:

```text
frame=1
engine=1600
max_vertex_error=0.0217075676
```

O pior RMS foi:

```text
frame=13
engine=3520
rms=0.00735928338
```

---

# 378. STAND motion[10] — PROVADO VISUALMENTE como Action completa

Após abrir:

```text
MfbC01_stand10_bake_v2.blend
```

o usuário confirmou:

```text
"a animação está perfeita"
```

Portanto:

```text
STAND motion[10]:
PROVADO NUMERICAMENTE nos 40 samples;
PROVADO VISUALMENTE como animação Blender completa.
```

Este é o primeiro motion clip do formato fechado de ponta a ponta:

```text
INX
-> motion decode
-> SMB segment
-> source evaluator
-> Physique
-> Blender Armature Mapping V2
-> Action
-> post-bake vertex comparison
-> validação visual
```

---

# 379. Observação pendente: interpolation_keypoints_set=0

A execução reportou:

```text
interpolation_keypoints_set=0
```

Isso significa que a helper usada para tentar forçar `LINEAR` não encontrou/
não percorreu os keypoints via API usada nessa versão do Blender.

Como todos os 40 source logical samples foram efetivamente keyed e a Action
foi aprovada visualmente, isso não invalida o PASS atual.

Mas antes de declarar fidelidade de subframes / retiming / export baseado em
interpolação Blender, auditar a Action API do Blender 5.1 e confirmar o modo
de interpolação real das channels.

---

# 380. Próximo alvo recomendado: uma única WALK

Não importar todas as motions ainda.

Próximo passo:

```text
listar os 7 WALK existentes no M5Bip.inx;
escolher uma variante curta/simples;
bakear somente essa WALK com o mesmo pipeline V2;
comparar todos os vertices contra source;
validar visualmente.
```

Depois de uma WALK aprovada, testar RUN e ATTACK antes de automatizar o lote
completo de 54 motions.
---

# 381. WALK inventory do M5Bip.inx — 7 variantes reais

Todas as WALK estão no:

```text
MotionFrame=3
TmFrame slot=2
State=0x50 WALK
Repeat=1
JobBits=0x00000080
EventOffsets=[2,17]
```

Tabela:

```text
[016] local  10..40  global 486..516  ItemCount=26
[017] local  50..80  global 526..556  ItemCount=48
[018] local  90..120 global 566..596  ItemCount=36
[019] local 130..160 global 606..636  ItemCount=33
[020] local 170..200 global 646..676  ItemCount=23
[021] local 210..240 global 686..716  ItemCount=15
[022] local 250..280 global 726..756  ItemCount=6
```

Cada variante possui exatamente:

```text
30 logical frames
```

O `ItemCount` diferente indica que não devemos afirmar ainda qual delas é a
WALK padrão para cada equipamento. Essa seleção ainda precisa ser ligada à
lógica de `ItemCodeList`.

---

# 382. Primeiro WALK test escolhido: MotionInfo[16]

Para validar locomotion sem ampliar o escopo, usar primeiro:

```text
MotionInfo[16]
State       = WALK
MotionFrame = 3
slot        = 2
local       = 10..40
global      = 486..516
Repeat      = 1
EventOffsets= [2,17]
ItemCount   = 26
```

Runtime source:

```text
engine start = 486 * 160 = 77760
engine end   = 516 * 160 = 82560
intervalo    = [77760,82560)
```

Blender bake esperado:

```text
frames 1..30
```

Importante:

```text
MotionInfo[16] é PROVADO como uma WALK real;
ainda NÃO é PROVADO como a variante WALK default de qualquer loadout.
```

O objetivo deste teste é validar o pipeline de locomotion.
---

# 383. WALK motion[16] — aprovada visualmente

Após bake com o pipeline V2, o usuário confirmou:

```text
"funcionou perfeitamente"
```

Portanto:

```text
MotionInfo[16]
State = WALK
Blender Action completa
```

fica:

```text
PROVADA VISUALMENTE.
```

A validação numérica completa do bake deve continuar sendo preservada pelo
script `pt_blender_motion_bake_v2.py` em cada motion.

---

# 384. Próximo candidato: RUN motion[23]

Inventário já decodificado:

```text
[023]
State       = RUN (0x60)
MotionFrame = 3
slot        = 2
local       = 300..320
global      = 776..796
Repeat      = 1
EventOffsets= [2,12]
JobBits     = 0x00000080
ItemCount   = 26
```

Runtime source:

```text
engine start = 776 * 160 = 124160
engine end   = 796 * 160 = 127360
intervalo    = [124160,127360)
```

Blender esperado:

```text
frames 1..20
```

---

# 385. WALK16 / RUN23 — possível par de equipamento

Observação estrutural:

```text
WALK[16] ItemCount = 26
RUN[23]  ItemCount = 26
```

As variantes seguintes também aparecem com contagens semelhantes, sugerindo
pares WALK/RUN por família de `ItemCodeList`.

Estado:

```text
hipótese forte;
ainda NÃO PROVADO semanticamente até auditar ItemCodeList e lógica de seleção.
```

O teste da RUN23 valida primeiro o pipeline de locomotion rápida sem depender
dessa interpretação.
---

# 386. RUN motion[23] — PASS numérico e visual

Execução:

```text
motion_index=23
state=RUN
repeat=1
motion_frame=3
engine_half_open=[124160,127360)
blender_frames=1..20
bones=95
```

Validação pós-bake:

```text
action=PT_m5_RUN_motion23_V2
keyed_bones=95
keyed_frames=20
total_vertex_samples=4900

max_vertex_error=0.0256382421
overall_rms_vertex_error=0.00702122775

post_bake_source_skin_check=PASS
blender_motion_bake_v2_check=PASS
blender_exit=0
```

Pior frame:

```text
frame=1
engine=124160
max_vertex_error=0.0256382421
rms_vertex_error=0.00755354189
```

---

# 387. RUN motion[23] — PROVADA VISUALMENTE

Após abrir:

```text
MfbC01_run23_bake_v2.blend
```

o usuário confirmou:

```text
"funcionou perfeitamente"
```

Portanto:

```text
RUN motion[23]:
PROVADA NUMERICAMENTE;
PROVADA VISUALMENTE.
```

---

# 388. Pipeline de locomotion já validado

Até este ponto:

```text
STAND motion[10] -> PASS numérico + visual
WALK  motion[16] -> PASS visual
RUN   motion[23] -> PASS numérico + visual
```

O pipeline V2 está demonstrado em:

```text
idle;
locomotion lenta;
locomotion rápida.
```

---

# 389. Próximo alvo: ATTACK

Antes de escolher uma ATTACK:

```text
listar todas as MotionInfo com State=0x100;
comparar MotionFrame, ranges, Repeat, EventOffsets,
ItemCount, JobBits e Skills;
preferir inicialmente uma variante compatível com o mesmo grupo
WALK16/RUN23 se a tabela sustentar isso.
```

Não inferir associação por equipamento apenas pelo ItemCount sem auditar
ItemCodeList.
---

# 390. ATTACK inventory do M5Bip.inx — 11 variantes

Todas são:

```text
State       = ATTACK (0x100)
MotionFrame = 5
slot        = 4
Repeat      = 0
JobBits     = 0x00000080
```

Tabela:

```text
[040] local  10..50  global 1478..1518  Event=[20] ItemCount=1
[041] local 160..200 global 1628..1668  Event=[20] ItemCount=48
[042] local 210..260 global 1678..1728  Event=[25] ItemCount=36
[043] local 270..310 global 1738..1778  Event=[22] ItemCount=33
[044] local 370..410 global 1838..1878  Event=[20] ItemCount=3
[045] local 320..360 global 1788..1828  Event=[22] ItemCount=23
[046] local 420..460 global 1888..1928  Event=[20] ItemCount=15
[047] local  60..100 global 1528..1568  Event=[18] ItemCount=20
[048] local 110..150 global 1578..1618  Event=[17] ItemCount=2
[049] local 520..560 global 1988..2028  Event=[18] ItemCount=3
[050] local 470..510 global 1938..1978  Event=[15] ItemCount=6
```

Não há ATTACK com:

```text
ItemCount=26
```

Logo a hipótese simples de parear WALK16/RUN23/ATTACK por igualdade de
`ItemCount` não se sustenta.

A semântica exata depende do conteúdo de `ItemCodeList`, não apenas da
contagem.

---

# 391. Primeiro ATTACK test escolhido: MotionInfo[40]

Escolha:

```text
MotionInfo[40]
State       = ATTACK
MotionFrame = 5
slot        = 4
local       = 10..50
global      = 1478..1518
Repeat      = 0
EventOffset = 20
ItemCount   = 1
```

Runtime source:

```text
engine start = 1478 * 160 = 236480
engine end   = 1518 * 160 = 242880
intervalo    = [236480,242880)
```

Duração Blender:

```text
40 frames
```

Evento:

```text
offset lógico = 20
source event engine offset = 20 * 160 = 3200
source event absolute = 239680
```

Com Blender frame 1 representando offset 0:

```text
evento corresponde ao Blender frame 21
```

O bake V2 ainda não cria marker de EventFrame; essa relação deve ser preservada
para uma etapa posterior de metadata/event markers.

---

# 392. Objetivo do ATTACK[40]

Este teste valida:

```text
motion não-looping;
fim de motion;
FrameInfo.EndFrame clamp;
pose ofensiva;
continuidade até o último sample;
pipeline Armature Mapping V2 em uma Action não-looping.
```

Não usar o teste para afirmar qual arma/equipamento seleciona esta ATTACK.

A associação de equipamento permanece pendente até auditar:

```text
ItemCodeList;
código de seleção de MotionInfo por item equipado.
```
