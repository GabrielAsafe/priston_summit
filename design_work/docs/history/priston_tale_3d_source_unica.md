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
