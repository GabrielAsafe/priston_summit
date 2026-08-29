# Investigação — Pipeline de Items / Merchant / Cliente

## Data
25 de agosto de 2026

## Estado
**Investigação ativa.**

O sistema de items ainda não está totalmente recuperado. O trabalho recente restaurou parte importante da identidade dos items no servidor e permitiu que alguns merchants voltassem a abrir a janela de loja, porém as lojas continuam sem items visíveis.

A investigação desta etapa conseguiu localizar com precisão a quebra atual da cadeia: **o cliente recebe corretamente os items enviados pelo servidor, mas o catálogo global `sItem[]` do cliente está vazio (`CODE == 0` em todas as 2000 entradas), fazendo `LoadItemImage()` falhar para todos os items recebidos.**

## 1. Sintoma atual

Após os fixes anteriores:
- cliente inicia sem crash;
- servidor inicia sem crash;
- não foram observadas regressões;
- alguns merchants que antes não abriam passaram a abrir a loja;
- a janela da loja abre;
- os slots continuam vazios.

```text
merchant interaction
        ↓
shop window opens
        ↓
shop inventory empty
```

## 2. Cadeia investigada

```text
NPC configuration
        ↓
SellAttackItem[] / SellDefenceItem[] / SellEtcItem[]
        ↓
srLinkNpcItem()
        ↓
DefaultItems[]
        ↓
SendShopItemList()
        ↓
OPCODE_SHOP_ITEMLIST / 0x700000D5
        ↓
socket
        ↓
client netplay.cpp
        ↓
sinRecvMessage()
        ↓
DecodeCompress()
        ↓
sITEMINFO[]
        ↓
sinSetShopItem()
        ↓
LoadItemImage()
        ↓
sItem[]
        ↓
cShop.ShopItem[]
        ↓
shop UI
```

## 3. Servidor — linking dos items do NPC confirmado

Foi instrumentado `srLinkNpcItem()`.

O log demonstrou que os merchants possuem listas válidas e que os códigos são resolvidos contra `DefaultItems`.

Exemplo observado:

```text
Ferreiro Gus
attack = 14
defence = 12
```

Controles conhecidos, como `WA101`, foram resolvidos com CODE válido e índice real de `DefaultItems`.

Conclusão: a hipótese de que a loja ficava vazia porque `srLinkNpcItem()` não encontrava os `DefaultItems` foi descartada.

## 4. Servidor — envio dos packets confirmado

Foi instrumentada `SendShopItemList()`.

Resultado runtime:

```text
[SHOPSEND] NPC="Ferreiro Gus" attack=14 defence=12 etc=0 opcode=0x700000D5 header=32 sockbuf=8192
[SHOPSEND] ATTACK items=14 size=869 kind=1 opcode=0x700000D5
[SHOPSEND] ATTACK Send() => 1
[SHOPSEND] DEFENCE items=12 size=581 kind=2 opcode=0x700000D5
[SHOPSEND] DEFENCE Send() => 1
```

Outro merchant:

```text
[SHOPSEND] NPC="Loja da Allya" attack=0 defence=0 etc=16 opcode=0x700000D5
[SHOPSEND] ETC items=16 size=697 kind=3 opcode=0x700000D5
[SHOPSEND] ETC Send() => 1
```

Conclusão: `ItemCounter`, tamanho, `ItemKind`, opcode e envio pelo socket estão corretos.

## 5. Opcode confirmado

No cliente:

```cpp
OPCODE_SHOP_ITEMLIST = 0x70000000 + 213;
```

Portanto:

```text
0x700000D5
```

A cadeia de dispatch encontrada é:

```text
netplay.cpp
    case OPCODE_SHOP_ITEMLIST
        ↓
    sinRecvMessage(...)
```

## 6. Cliente — packet recebido corretamente

Foi adicionada instrumentação `SHOPRECV` e `SHOPDECODE`.

Exemplo real:

```text
[SHOPRECV] size=869 code=0x700000D5 kind=1 count=14 header=32
```

Os 14 items foram decodificados com CODEs coerentes:

```text
0x01010100 price=60
0x01010200 price=420
0x01010300 price=800
...
```

Defesa chegou com `kind=2 count=12`; ETC chegou com `kind=3 count=16`.

Conclusão: servidor, rede, dispatch e `DecodeCompress()` estão funcionais para o campo CODE.

## 7. Observação sobre sITEMINFO recebido

Os logs mostraram:

```text
CODE correto
Price correto
ItemName=""
Level=0
```

Isso não explica o `return FALSE` atual, porque `LoadItemImage()` primeiro decide pelo `CODE`.

Esses campos continuam relevantes para a recuperação completa de `DecodeItemInfo`, mas não são a causa imediata da loja vazia.

## 8. sinSetShopItem

`sinSetShopItem()` chama `LoadItemImage()` e só copia o item para `cShop.ShopItem[]` quando o retorno é verdadeiro.

A instrumentação mostrou `SHOPLOAD result=0` em massa:
- 14/14 attack;
- 12/12 defence;
- 16/16 etc.

Logo, a falha é global, não específica de um asset isolado.

## 9. LoadItemImage

A implementação real foi localizada em `SrcGame/src/sinbaram/sinSubMain.cpp`.

Lógica central:

```cpp
for(int j = 0; j < MAX_ITEM; j++)
{
    if(sItemInfo->CODE == sItem[j].CODE)
    {
        // monta path
        // copia sITEM
        // copia sITEMINFO
        // carrega imagem
        // retorna TRUE
    }
}

return FALSE;
```

O path é construído com:

```text
Image//sinImage//Items//<ItemFilePath>//it<LastCategory>.bmp
```

## 10. Estrutura sITEM do cliente

Campos relevantes encontrados:

```text
CODE
ItemNameIndex
LastCategory
w
h
ItemFilePath
Class
DorpItem
SetModelPosi
SoundIndex
WeaponClass
Flag
x / y
SetX / SetY
lpItem
ItemPosition
PotionCount
NotUseFlag
...
```

Isso confirma que `sItem[]` é o catálogo visual/estrutural central do cliente.

## 11. Uso global do catálogo

`sItem[]` é usado por:
- inventory;
- shop;
- warehouse;
- trade;
- message boxes;
- character equipment;
- effects/sounds;
- drops;
- record/post;
- event/monster parsing;
- checksums.

Portanto sua recuperação deve ser tratada como fix central de items.

## 12. Initializer atual de sItem[]

Em `SrcGame/src/sinbaram/sinItem.cpp`:

```cpp
sITEM sItem[MAX_ITEM] = {
#include "items.h"
};
```

`MAX_ITEM = 2000`.

Porém não existe `SrcGame/src/sinbaram/items.h`. O include acaba resolvendo para outro `items.h` do include path, especialmente `Shared/items.h`, que não contém o catálogo histórico de inicializadores `sITEM`.

## 13. Prova runtime definitiva

`LoadItemImage()` foi instrumentado para contar entries não-zero de `sItem[]`.

Resultado:

```text
[LOOKUP] wanted=0x01010100 MAX_ITEM=2000
nonzero entries=0
exactMatches=0
[NOT_FOUND] wanted=0x01010100
```

O mesmo se repetiu para todos os CODEs testados.

Conclusão runtime:

```text
sItem[0..1999].CODE == 0
```

na execução atual do cliente.

## 14. Causa atual da loja vazia

```text
Servidor recupera CODE
        ↓
merchant resolve DefaultItems
        ↓
SendShopItemList
        ↓
packet válido
        ↓
cliente recebe
        ↓
DecodeCompress
        ↓
sITEMINFO.CODE correto
        ↓
sinSetShopItem
        ↓
LoadItemImage
        ↓
lookup em sItem[2000]
        ↓
sItem inteiro zerado
        ↓
return FALSE
        ↓
cShop.ShopItem[] não é preenchido
        ↓
janela abre vazia
```

## 15. Hipóteses descartadas

Foram descartadas como causa principal:
- merchant sem item config;
- `srLinkNpcItem()` falhando globalmente;
- `DefaultItems` zerado no momento do merchant;
- `SendShopItemList()` com `ItemCounter=0`;
- falha do socket;
- opcode divergente;
- cliente não recebendo packet;
- `DecodeCompress()` destruindo CODE;
- asset individual faltando como causa primária.

## 16. Catálogo histórico disponível

Foi localizada uma fonte antiga em:

```text
/home/gabriel/armazem/priston/pristontale-1-master/src/sinbaram/sinItem.cpp
```

Ela contém registros completos antigos (`WA101`, `WA108`, `BC102`), mas não cobre controles modernos como `BC117`, `BI156` e `BI330`.

Portanto é uma fonte parcial.

## 17. Fontes modernas investigadas

Foram inspecionados:

```text
fPT-rPT-Project-main/shared/item.h
priston-main/shared/item.h
```

Esses arquivos contêm enums, tipos e IDs modernos, mas não registros `sITEM` completos.

Também existem divergências de ID com o Summit. Exemplo:

```text
projeto moderno:
BI330 = 0x080F1E00

mapa Summit recuperado:
BI330 = 0x080F2E00
```

Logo, esses headers não podem ser usados como fonte canônica direta.

## 18. Mapa server-side existente

O servidor usa atualmente `item_code_map_mingw.inc`, com cerca de 1723 identidades `CODE + LastCategory`.

Esse mapa é suficiente para vários lookups server-side, mas insuficiente para o cliente porque faltam `ItemFilePath`, `w/h`, `Class`, `DorpItem`, `SoundIndex`, `WeaponClass` etc.

## 19. Estado arquitetural

A causa estrutural atual é a perda do catálogo histórico esperado pelo initializer:

```cpp
#include "items.h"
```

O `items.h` efetivamente resolvido não possui os inicializadores originais, deixando `sItem[MAX_ITEM]` zero-inicializado.

## 20. Próximo objetivo técnico

Restaurar ou reconstruir:

```text
CODE → sITEM metadata
```

Primeiro critério:

```text
LoadItemImage()
    ↓
MATCH por CODE
    ↓
path válido
    ↓
w/h válidos
    ↓
cShop.ShopItem preenchido
```

## 21. Estratégia recomendada

Preferência:
1. localizar catálogo Summit completo em backups/commits antigos;
2. se não existir, reconstrução controlada usando:
   - catálogo legado completo (~980);
   - mapa Summit (~1723 CODE/LastCategory);
   - OpenItem atuais (1826);
   - assets atuais;
   - regras por família de item.

Não preencher metadados desconhecidos por adivinhação.

## 22. Campos mínimos prioritários

Para provar merchants/inventory básico:

```text
CODE
LastCategory
ItemFilePath
w
h
Class
DorpItem
```

## 23. Instrumentações atuais

Servidor:
```text
server_startup_items.log
shop_item_pipeline.log
[SHOPSEND]
```

Cliente:
```text
shop_client_pipeline.log
LoadItemImage_debug.txt
```

## 24. Critérios de sucesso

A próxima etapa estará funcional quando:
1. `sItem[]` possuir entries `CODE != 0`;
2. `exactMatches > 0`;
3. `SHOPLOAD result=1`;
4. `cShop.ShopItem[]` receber os items;
5. Ferreiro Gus mostrar attack/defence items;
6. Loja da Allya mostrar etc items;
7. inventory/trade/warehouse não regredirem;
8. cliente permanecer estável.

## 25. Resumo executivo

Estado comprovado:

```text
SERVER
NPC config                  OK
CODE resolution             OK
DefaultItems                OK
merchant linking            OK
packet building             OK
ItemCounter                 OK
socket Send                 OK

NETWORK / CLIENT
opcode                      OK
packet receive              OK
DecodeCompress CODE         OK

CLIENT ITEM MATERIALIZATION
sItem catalog               BROKEN / EMPTY
LoadItemImage lookup        FAIL
cShop population            FAIL
shop render                 EMPTY
```

**Raiz imediata:** `sItem[MAX_ITEM]` do cliente está vazio porque o initializer histórico baseado em `items.h` já não recebe o catálogo `sITEM` que o código espera.

O próximo trabalho deixa de ser debugging de merchant/network e passa a ser recuperação do catálogo central de items do cliente.
