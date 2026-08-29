# Bug Report / Handoff — Distribuidor, Drops de Equipamentos e Starter Gear

## Data

26 de agosto de 2026

## Projeto

Summit Tale / Priston Tale-derived C++ client/server.

Source principal:

```text
/home/gabriel/Área de Trabalho/summit/SourceCode
```

Historical executable used for item catalog recovery:

```text
/home/gabriel/Área de Trabalho/Summit Tale/Game.exe
```

---

# 1. Objetivo deste documento

Este documento serve como handoff para uma nova sessão de debugging.

A cadeia principal de merchant items foi recentemente restaurada, mas **três problemas continuam abertos**:

1. **O Distribuidor não abre.**
2. **Mobs ainda não dropam equipamentos.**
3. **O jogador não recebe os equipamentos iniciais que o jogo originalmente entregava ao criar/iniciar personagem.**

Estes bugs podem ter relação com o catálogo de items restaurado, com tabelas auxiliares ainda incompletas ou com pipelines separados de criação/drop/distribuição.

Não assumir que os três possuem a mesma causa sem evidência runtime.

---

# 2. Estado conhecido do sistema de items

## 2.1 Catálogo client-side `sItem[]` foi restaurado

O cliente anteriormente tinha:

```cpp
sITEM sItem[MAX_ITEM] = {
#include "items.h"
};
```

Mas o `items.h` efetivamente resolvido não continha o catálogo histórico.

Resultado runtime antes da recuperação:

```text
[LOOKUP] wanted=0x01010100 MAX_ITEM=2000
nonzero entries=0
exactMatches=0
[NOT_FOUND] wanted=0x01010100
```

Portanto:

```text
sItem[0..1999].CODE == 0
```

no cliente quebrado.

---

## 2.2 Catálogo recuperado de `Game.exe` histórico

Foi localizado um `Game.exe` antigo funcional do próprio Summit.

SHA-256:

```text
44cec8bbd95b2e02f36dd9cb78486945c6809000b2386df83812096a07a9a1b3
```

A engenharia reversa mostrou que o executável constrói os registros `sITEM` em `.text` usando o construtor:

```cpp
sITEM(
    DWORD CODE,
    const char* ItemNameIndex,
    const char* LastCategory,
    int w,
    int h,
    const char* ItemFilePath,
    int Class,
    const char* DorpItem,
    DWORD SetModelPosi,
    int SoundIndex,
    int WeaponClass = 0
)
```

O construtor identificado no EXE estava em:

```text
0x00A5A2E0
```

Primeiro objeto:

```text
0x05492840
```

Stride:

```text
0x320
```

Foram recuperados:

```text
1819 construtores explícitos
indices 0..1818
0 chamadas inválidas
0 objetos desalinhados
```

Depois desses 1819 registros, o executável inicializava mais:

```text
181 objetos default
```

Total:

```text
1819 + 181 = 2000 = MAX_ITEM
```

---

## 2.3 Validação contra mapa Summit atual

O mapa server-side atual possui:

```text
1723 entries
```

Comparação entre o EXE histórico e o mapa Summit:

```text
exact LastCategory + CODE : 1535
same category / CODE diff : 0
missing category in EXE   : 188
EXE records outside map   : 281
```

Resultado crítico:

```text
same category / CODE diff = 0
```

Ou seja: quando o EXE histórico e o mapa atual conhecem a mesma categoria, os CODEs coincidem.

Controles importantes:

```text
WA101 -> 0x01010100
WA108 -> 0x01010800
BC102 -> 0x080A0200
BI156 -> 0x080B4800
BI330 -> 0x080F2E00
```

`BC117` não existe no EXE histórico.

---

## 2.4 Catálogo gerado no cliente

Foi criado:

```text
SrcGame/src/sinbaram/item_catalog_recovered.inc
```

Quantidade:

```text
1819 registros
```

SHA-256:

```text
7d9ddbe6379fa4650cc7d23f0b060f99fd34cf2025dcc520f02bc34e0cdaa7ba
```

O initializer do cliente passou de:

```cpp
sITEM sItem[MAX_ITEM] = {
#include "items.h"
};
```

para:

```cpp
sITEM sItem[MAX_ITEM] = {
#include "item_catalog_recovered.inc"
};
```

A alteração em `sinItem.cpp` foi reaplicada em bytes para preservar line endings.

Validação:

```text
exact expected patch : True
expected byte delta  : 19
actual byte delta    : 19
```

---

# 3. Merchant normal — estado conhecido

Antes da restauração do catálogo, a pipeline já estava provada até o cliente.

## Server

```text
NPC config                  OK
CODE resolution             OK
DefaultItems                OK
merchant linking            OK
packet building             OK
ItemCounter                 OK
socket Send                 OK
```

## Network / client

```text
opcode                      OK
packet receive              OK
DecodeCompress CODE         OK
```

## Antes do catalog fix

```text
sItem catalog               EMPTY
LoadItemImage lookup        FAIL
cShop population            FAIL
shop render                 EMPTY
```

A cobertura dos items usados nos merchants testados foi:

```text
42 / 42
```

no catálogo recuperado.

Após restaurar `sItem[]`, os lojistas testados passaram a mostrar os items.

Portanto o pipeline genérico:

```text
merchant NPC
    ↓
server item list
    ↓
network packet
    ↓
DecodeCompress
    ↓
LoadItemImage
    ↓
sItem[]
    ↓
shop UI
```

está funcional para os merchants testados.

---

# 4. BUG ABERTO A — Distribuidor não abre

## Sintoma

O NPC/feature chamado **Distribuidor** ainda não abre sua interface.

Isso continua acontecendo mesmo após a restauração do catálogo `sItem[]`, enquanto outros lojistas passaram a abrir e mostrar items corretamente.

---

## O que isso já sugere

Como merchants comuns agora funcionam, o Distribuidor provavelmente:

1. usa um opcode diferente;
2. usa um handler/UI diferente;
3. depende de outra tabela/lista de items;
4. depende de alguma condição/account state;
5. depende de um packet server-side diferente;
6. pode não usar `SendShopItemList()` / `sinSetShopItem()`.

Não assumir que ele é apenas mais um merchant até provar o fluxo.

---

## Primeira investigação recomendada

Localizar todas as referências ao nome/texto e às estruturas relacionadas:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

grep -RIn \
  --include='*.cpp' \
  --include='*.h' \
  -E 'Distribuidor|Distributor|Distribute|Distribution' \
  SrcGame/src SrcServer/src
```

Também procurar NPC/shop modes especiais:

```bash
grep -RIn \
  --include='*.cpp' \
  --include='*.h' \
  -E 'ShopKind|ShopType|NpcKind|NpcType|WareHouse|Warehouse|Distributor' \
  SrcGame/src SrcServer/src |
head -300
```

Objetivo:

```text
interaction
    ↓
server handler?
    ↓
opcode?
    ↓
client dispatch?
    ↓
UI handler?
```

Instrumentar somente depois de localizar a cadeia real.

---

## Critério de sucesso

```text
Distribuidor interaction
    ↓
packet/handler identificado
    ↓
client handler chamado
    ↓
interface abre
```

Se o Distribuidor também entrega items, validar separadamente:

```text
interface opens
items present
item action works
```

---

# 5. BUG ABERTO B — Mobs não dropam equipamentos

## Sintoma

Mobs continuam sem dropar equipamentos.

Este problema permanece mesmo depois da restauração do catálogo client-side.

---

## Separação importante

Não confundir:

```text
drop não é gerado pelo servidor
```

com:

```text
drop é gerado mas cliente não materializa/renderiza
```

A recuperação de `sItem[]` corrigiu um problema client-side central, mas não prova que o servidor esteja gerando equipamento.

A investigação deve começar no servidor.

---

## Hipóteses principais a testar

### B1. Tabela/drop list do mob não é preenchida

Verificar:

```text
monster definition
    ↓
drop configuration parsing
    ↓
item category/CODE resolution
    ↓
drop table in memory
```

### B2. CODE do drop não resolve para `DefaultItems`

Mesmo com o mapa de 1723 identities recuperado, pode existir item moderno entre os 37/188 gaps ou outra tabela não populada.

### B3. RNG/drop branch nunca chega em item equipment

Pode haver:

```text
drop rate
money branch
potion branch
equipment branch
```

com condição quebrada.

### B4. Drop server-side existe, mas packet/world item falha

Precisamos distinguir:

```text
server selected equipment
```

de:

```text
world item object created
```

e:

```text
client received/displayed drop
```

---

## Primeira investigação recomendada

Procurar pontos de geração de drop por nome já conhecido no source:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

grep -RIn \
  --include='*.cpp' \
  --include='*.h' \
  -E 'DropItem|DorpItem|Drop.*Item|Monster.*Item|Create.*Item|Set.*Item' \
  SrcServer/src |
head -400
```

Também procurar consumidores de `DorpItem`:

```bash
grep -RIn \
  --include='*.cpp' \
  --include='*.h' \
  '\bDorpItem\b' \
  SrcServer/src SrcGame/src |
head -200
```

`DorpItem` é especialmente relevante porque foi recuperado do EXE e não é sempre igual a `LastCategory`.

Exemplo real recuperado:

```text
LastCategory = BC102
DorpItem     = BC101
```

Portanto não inferir:

```text
DorpItem == LastCategory
```

---

## Instrumentação recomendada

Depois de encontrar a função de seleção de drop, registrar:

```text
[MOBDROP]
monster=<name/code>
roll=<...>
selectedCategory=<...>
selectedCODE=0x...
DefaultItemsMatch=<index or -1>
worldCreate=<result>
```

Se houver uma transição textual:

```text
DorpItem -> LastCategory/CODE
```

registrar os dois lados.

---

## Critério de sucesso

Para um mob de teste:

```text
equipment branch selected
CODE != 0
DefaultItems match
world item created
client sees equipment on ground
pickup works
```

Não considerar resolvido apenas porque aparece um objeto visual.

---

# 6. BUG ABERTO C — Starter gear não é entregue ao player

## Sintoma

Em versões anteriores, ao criar/iniciar o personagem, o jogo entregava alguns equipamentos iniciais.

Isso não está acontecendo atualmente.

---

## Por que pode ser separado do merchant/drop

Starter gear costuma utilizar um pipeline próprio:

```text
new character
    ↓
class/job selection
    ↓
CreateItem / GiveItem
    ↓
inventory insertion
    ↓
save
```

Pode depender de:

```text
character class
job code
server account/character creation
default inventory template
hard-coded item CODEs
CreateItem()
DefaultItems[]
```

Não assumir que vem de NPC ou drop.

---

## Hipóteses principais

### C1. Lista de starter CODEs está zerada/corrompida

### C2. `CreateItem()` recebe CODE correto, mas lookup falha

### C3. Item é criado mas não inserido no inventory

### C4. Item é inserido e posteriormente apagado por init/load/save

### C5. Starter gear é criado server-side mas packet de inventory inicial não contém os items

### C6. Condição por classe/job nunca é satisfeita

---

## Primeira investigação recomendada

Pesquisar por criação de personagem e starter items:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

grep -RIn \
  --include='*.cpp' \
  --include='*.h' \
  -E 'NewChar|NewCharacter|CreateChar|CreateCharacter|StartItem|Starter|DefaultItem|BeginItem|InitItem' \
  SrcServer/src SrcGame/src |
head -400
```

Pesquisar por CODEs antigos típicos de starter gear:

```bash
grep -RIn \
  --include='*.cpp' \
  --include='*.h' \
  -E '0x01010100|WA101|DB101|DG101|DS101|PM101|PL101|PS101' \
  SrcServer/src SrcGame/src |
head -300
```

Pesquisar funções de criação/inserção:

```bash
grep -RIn \
  --include='*.cpp' \
  --include='*.h' \
  -E 'CreateItem|MakeItem|AddItem|PutItem|InsertItem|SetItem' \
  SrcServer/src |
head -400
```

---

## Instrumentação recomendada

No ponto exato onde starter gear deveria ser criado:

```text
[STARTER_ITEM]
class=<...>
slot=<...>
requestedCODE=0x...
DefaultItemsMatch=<index or -1>
createResult=<...>
inventoryInsert=<...>
```

Depois confirmar:

```text
save contains item?
initial inventory packet contains item?
```

---

## Critério de sucesso

Criar personagem novo e observar:

```text
starter branch reached
starter CODEs valid
items created
items inserted
items persisted
client receives inventory
items visible/equippable
```

---

# 7. Relação possível entre os três bugs

Existem algumas dependências em comum que merecem auditoria:

```text
DefaultItems[]
item CODE lookup
CreateItem()
DorpItem
server item definitions
character/NPC config parsing
```

Mas não há evidência ainda de uma causa única.

A ordem recomendada é:

```text
1. Distribuidor
   porque é um fluxo de interação pequeno e permite mapear outro handler especial.

2. Starter gear
   porque é determinístico e normalmente usa CODEs conhecidos.

3. Mob equipment drops
   porque envolve RNG, monster config e world spawning, sendo o pipeline mais amplo.
```

Alternativamente, se houver uma função central `CreateItem()` compartilhada entre starter gear e mob drops, inverter 2/3 e auditar primeiro essa função pode resolver ambos.

---

# 8. Pontos já comprovados — não repetir investigação

Não repetir sem nova evidência:

## Shop packet

```text
OPCODE_SHOP_ITEMLIST = 0x700000D5
```

Packets foram recebidos corretamente no cliente.

## DecodeCompress

CODEs dos items de shop foram decodificados corretamente.

## Generic merchant Send()

Servidor enviou listas com:

```text
attack = 14
defence = 12
etc = 16
```

e `Send()` retornou sucesso.

## Generic `LoadItemImage()`

O motivo anterior de falha era catálogo `sItem[]` vazio.

Esse catálogo foi restaurado.

## Historical client catalog

Já existe uma fonte recuperada e auditada:

```text
SrcGame/src/sinbaram/item_catalog_recovered.inc
1819 records
```

Não reconstruir novamente pelos BMPs ou por outra source sem motivo.

---

# 9. Limitações conhecidas do catálogo atual

O catálogo recuperado do EXE histórico é real e funcional, mas não cobre todas as identities modernas do Summit.

Contra o mapa server-side:

```text
188 current categories are absent from the EXE
```

Exemplos:

```text
BC117
BC118
BC133
BI160
BI161
BI185
BI199
BI200
BI238
BI239
BI254
BI299
BI301...
CA201...
DA130...
...
```

Portanto, se um bug envolver diretamente um desses items, a ausência de metadados client-side ainda pode ser relevante.

Não preencher esses 188 por adivinhação.

---

# 10. Include guard collision já descoberto

Existe uma colisão arquitetural separada:

```text
SrcGame/src/sinbaram/sinItem.h
SrcServer/src/sinbaram/sinItem.h
```

compartilham o include guard:

```text
_SINITEM_HEADER_
```

Isso pode fazer uma TU server-side incluir primeiro o header Game e suprimir o header Server.

Uma mitigação já foi usada no server item-code map com header dedicado.

Não fazer rename amplo desse include guard durante estes bugs sem auditoria, porque pode expor redefinições e gerar regressões.

---

# 11. Instrumentações/backups temporários existentes

Podem ainda existir:

```text
SrcGame/src/sinbaram/sinSubMain.cpp.bak_before_loaditem_catalog_probe
SrcGame/src/sinbaram/sinSubMain.cpp.bak_shop_client_probe

SrcServer/src/Server/OnSever.cpp.bak_before_shoplink_probe
SrcServer/src/Server/OnSever.cpp.bak_before_shopsend_probe
```

Não adicionar esses backups em commits.

Também podem permanecer probes como:

```text
[SHOPRECV]
[SHOPDECODE]
[SHOPLOAD]
[LOOKUP]
[MATCH]
[NOT_FOUND]
[SHOPSEND]
[SHOPLINK]
```

Remover somente depois que deixarem de ser necessários e em commit separado ou claramente documentado.

---

# 12. Princípios de trabalho para a próxima sessão

Usar investigação incremental e baseada em evidência.

Antes de alterar:

```text
snapshot
hash
grep/trace
runtime proof
```

Depois:

```text
minimal patch
diff
build
runtime regression
isolated commit
```

Evitar:

```text
global encoding conversion
broad HDD scans
manual guessing of item metadata
mass replacement
unrelated cleanup
```

Technical identifiers devem permanecer ASCII.

Legacy CP949 deve ser tratado apenas onde compatibilidade real exigir.

---

# 13. Ordem sugerida para a próxima sessão

## Passo 1 — mapear o Distribuidor

Descobrir se usa shop genérico ou handler especial.

Produzir:

```text
interaction entry point
server function
opcode
client dispatch
UI function
```

## Passo 2 — localizar função central de item creation

Verificar se starter gear e mob drops convergem para:

```text
CreateItem()
DefaultItems[]
```

ou função equivalente.

## Passo 3 — starter gear

Por ser determinístico, instrumentar a criação de personagem e provar:

```text
branch
CODE
lookup
create
insert
save
send
```

## Passo 4 — mob drop

Instrumentar:

```text
monster
roll
selected item
lookup
world spawn
packet
client render
```

## Passo 5 — somente então corrigir

Não aplicar correções antes de localizar o primeiro ponto onde o estado diverge do esperado.

---

# 14. Resumo executivo para novo chat

Contexto curto:

> O catálogo global `sItem[]` do cliente estava vazio porque o antigo `items.h` com os initializers desapareceu. Um `Game.exe` histórico do Summit foi usado como fonte de dados: foram identificadas 1819 chamadas ao construtor `sITEM`, stride `0x320`, indices 0..1818, e os 181 restantes eram default para completar MAX_ITEM=2000. Os dados técnicos foram extraídos para `item_catalog_recovered.inc`. 1535 entries batem exatamente com o mapa Summit atual e não existe nenhum CODE conflict. Depois disso os merchants normais voltaram a mostrar items.

Bugs ainda abertos:

```text
A. Distribuidor não abre.
B. Mobs não dropam equipamentos.
C. Starter equipment não é entregue a personagens novos.
```

Não repetir debugging do shop genérico.

Investigar primeiro se Distribuidor possui fluxo próprio, depois localizar a função central de criação de item e verificar se starter gear e mob drops convergem nela.

---

# 15. Documentação relacionada

BUGFIX 055 completo:

```text
bugfixes/055_restore_canonical_sitem_catalog.md
```

Título:

```text
BUGFIX 055 — Restore Client sItem[] Catalog from Historical Game.exe
```

Essa documentação contém:

```text
root cause
binary recovery
constructor decoding
hashes
coverage
catalog generation
runtime merchant validation
remaining 188-item gap
```

Use-a como referência antes de alterar novamente o catálogo de items.
