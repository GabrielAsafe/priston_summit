# Bugfix 051 --- DecodeItemInfo legacy CP949 token recovery

## Status

**Estado atual:** em validação por build/runtime.\
**Escopo:** recuperação da pipeline de inicialização de itens do Summit
Tale, com foco no servidor MinGW e no parser legado `DecodeItemInfo()`.

Este documento consolida as descobertas feitas depois da instrumentação
de startup, da auditoria dos literais corrompidos, da recuperação
parcial dos tokens CP949 e da descoberta de que a tabela `sItem` usada
pelo servidor não estava sendo populada pelo catálogo histórico.

## 1. Sintoma original

Cliente e servidor deixaram de crashar, porém os itens não apareciam
corretamente.

A instrumentação de `InitItems()` provou que o servidor enumerava os
arquivos `GameServer//OpenItem//*.txt`, mas os objetos resultantes
permaneciam essencialmente vazios. Exemplos observados:

``` text
[BEFORE] INDEX=0 FILE=GameServer//OpenItem//2_WA108.txt
[AFTER ] INDEX=0 CODE=0x00000000 NAME="" ITEM_INDEX=0 UNIQUE=0x00000000 PRICE=30000 LEVEL=0

[BEFORE] INDEX=10 FILE=GameServer//OpenItem//BC102.txt
[AFTER ] INDEX=10 CODE=0x00000000 NAME="" ITEM_INDEX=0 UNIQUE=0x00000000 PRICE=50000 LEVEL=0

[BEFORE] INDEX=1459 FILE=GameServer//OpenItem//WA101.txt
[AFTER ] INDEX=1459 CODE=0x00000000 NAME="" ITEM_INDEX=0 UNIQUE=0x00000000 PRICE=60 LEVEL=0
```

Ao final:

``` text
DEFAULT_ITEM_COUNT=1826
```

Logo, a enumeração dos arquivos estava funcionando. O problema estava
dentro da interpretação/identificação dos itens.

## 2. Recuperação dos comandos estruturais

A tabela `szIniCommand[]` em:

-   `SourceCode/SrcGame/src/fileread.cpp`
-   `SourceCode/SrcServer/src/fileread.cpp`

continha comandos destruídos por encoding. Quatro posições foram
recuperadas semanticamente como:

``` text
*EQUIPPED_WEAPON
*APPLICABLE_JOB
*LOCATION
*APPLICABLE_SKILL
```

Também foram recuperados tokens auxiliares de modelo/motion:

``` text
ALL
NONE
TOWN
FIELD
```

Essa correção eliminou uma classe de corrupção do DSL, mas não foi
suficiente para restaurar os itens.

## 3. Auditoria de DecodeItemInfo

A auditoria encontrou 84 literais relevantes em `DecodeItemInfo()` tanto
no Game quanto no Server.

Resultado inicial:

``` text
total  : 84
EXACT  : 0
HIGH   : 74
MEDIUM : 0
LOW    : 10
FFFD   : 74
```

Uma análise estrutural posterior produziu 76 candidatos:

``` text
EXACT  : 1
REVIEW : 10
LOW    : 65
```

O manifesto semântico permitiu recuperar muitos tokens pela assinatura
dos campos escritos pelo parser, e não pela aparência do mojibake.

Exemplos comprovados:

``` text
*코드       -> CODE
*내구력     -> DURABILITY
*무게       -> WEIGHT
*가격       -> PRICE
*공격력     -> ATTACK DAMAGE
*사정거리   -> RANGE
*공격속도   -> ATTACK SPEED
*명중력     -> ATTACK RATING
*크리티컬   -> CRITICAL
*연결파일   -> LINK FILE
```

## 4. Estratégia adotada para os tokens CP949

Para evitar depender do encoding do próprio arquivo-fonte, os tokens
recuperados foram representados por byte escapes.

Exemplo:

``` cpp
static const char *ITEM_TOKEN_CODE =
    "\x2A\xC4\xDA\xB5\xE5";
```

Foram introduzidas constantes para os tokens mínimos de prova e as
comparações correspondentes passaram a usar essas constantes.

A associação foi guiada pelo campo escrito por cada ramo de
`DecodeItemInfo()`, evitando substituições globais perigosas em literais
corrompidos que podiam ter colidido para a mesma representação.

## 5. Resultado do CODEGATE

A instrumentação mostrou:

``` text
[CODEGATE] ... next=0 cmp=0
```

Ou seja, o token `*CODE` passou a ser reconhecido.

Em seguida, probes específicos mostraram:

``` text
[SITEM] PARSED file="GameServer//OpenItem//WA101.txt" value="WA101" len=5
[SITEM] SUMMARY ... categories=0 codes=0 max=2000
[SITEM] NO_MATCH ... value="WA101"
```

O mesmo ocorreu para `WA108` e `BC102`.

Essa foi uma descoberta decisiva: o parser já conseguia ler o valor
textual do CODE, mas não encontrava nenhuma categoria correspondente em
`sItem`.

## 6. Causa intermediária: sItem estava zerado

As definições reais são:

``` cpp
sITEM sItem[MAX_ITEM] = {
#include "items.h"
};
```

em:

-   `SrcGame/src/sinbaram/sinItem.cpp`
-   `SrcServer/src/sinbaram/sinItem.cpp`

Entretanto, no Summit atual existe apenas:

``` text
Shared/items.h
```

e seu conteúdo é essencialmente:

``` cpp
#pragma once
#define ITEM_KIND_SKIN 0x9999
#include "SrcGame/src/sinbaram/sinItem.h"

#ifndef ITEM_KIND_PERFECT
#define ITEM_KIND_PERFECT 9
#endif
```

Portanto, o `#include "items.h"` usado dentro do initializer de `sItem`
não contém mais o catálogo histórico de inicializadores.

A prova binária anterior ao reparo mostrou:

``` text
00002e40 B sItem
```

no objeto do servidor e ausência das strings de controle `WA101`,
`WA108`, `BC102`, etc.

O símbolo estar em `B` é compatível com armazenamento zero-inicializado,
confirmando que o catálogo não estava sendo materializado nesse build.

## 7. Recuperação do mapa LastCategory -\> CODE

Foi localizado um artefato histórico de recuperação:

``` text
item_code_map_mingw.inc
```

com 1723 entradas.

Características:

``` text
rows         : 1723
unique names : 1723
unique codes : 1712
duplicate internal codes: 11
```

A comparação com os OpenItem reais mostrou:

``` text
OpenItem files          : 1826
OpenItem unique CODE    : 1760
Map entries             : 1723
Covered unique CODE     : 1723
Missing unique CODE     : 37
Map entries not in data : 0
Files without CODE      : 0
Coverage                : 97.90%
```

Controles confirmados:

``` text
WA101 OPEN=True MAP=True CODE=0x01010100
WA108 OPEN=True MAP=True CODE=0x01010800
BC102 OPEN=True MAP=True CODE=0x080A0200
BC117 OPEN=True MAP=True CODE=0x080A1100
BI156 OPEN=True MAP=True CODE=0x080B4800
BI330 OPEN=True MAP=True CODE=0x080F2E00
BI901 OPEN=True MAP=False
```

## 8. Implementação do mapa no servidor

Em `SrcServer/src/sinbaram/sinItem.cpp` foi criada uma tabela:

``` cpp
static const MINGW_ITEM_CODE_MAP_ENTRY g_MingwItemCodeMap[] =
{
#include "item_code_map_mingw.inc"
};
```

e uma rotina de população que escreve:

``` text
sItem[i].CODE
sItem[i].LastCategory
sItem[i].sItemInfo.CODE
```

A função interna foi exposta como:

``` cpp
int InitializeServerItemCodeMap()
{
    return MingwPopulateItemCodeMap();
}
```

`cITEM::Init()` também chama a população do mapa, preservando
compatibilidade com o caminho tradicional.

## 9. Inicialização explícita antes de InitItems

Como `InitItems()` depende de `sItem` para resolver o texto lido de
`*CODE`, a inicialização foi movida também para o caminho explícito de
startup:

``` cpp
const int ItemCodeMapCount =
    InitializeServerItemCodeMap();

fprintf(
    stderr,
    "[STARTUP][ITEM_CODE_MAP] populated=%d\n",
    ItemCodeMapCount);
```

A intenção é garantir a ordem:

``` text
popular mapa sItem
    ↓
enumerar OpenItem
    ↓
DecodeItemInfo
    ↓
resolver LastCategory
    ↓
atribuir CODE real ao item
```

## 10. Descoberta durante o build: declaração invisível em OnSever.cpp

A primeira tentativa declarou:

``` cpp
extern int InitializeServerItemCodeMap();
```

em `SrcServer/src/sinbaram/sinItem.h`.

Apesar de `OnSever.cpp` conter:

``` cpp
#include "../sinbaram/sinItem.h"
```

o compilador reportou:

``` text
OnSever.cpp:2224:17:
error: 'InitializeServerItemCodeMap' was not declared in this scope
```

A definição existia e `sinItem.cpp` compilava. Portanto, o defeito era
de visibilidade da declaração no translation unit de `OnSever.cpp`.

## 11. Diagnóstico pelo pré-processador

A investigação dos headers mostrou que os dois arquivos legados:

``` text
SrcGame/src/sinbaram/sinItem.h
SrcServer/src/sinbaram/sinItem.h
```

usam o mesmo include guard:

``` cpp
#ifndef _SINITEM_HEADER_
#define _SINITEM_HEADER_
```

Além disso, a cadeia de includes do servidor pode carregar primeiro o
header Game. `Shared/items.h`, por exemplo, referencia explicitamente:

``` cpp
#include "SrcGame/src/sinbaram/sinItem.h"
```

Consequência:

``` text
Game sinItem.h entra primeiro
        ↓
define _SINITEM_HEADER_
        ↓
OnSever.cpp inclui ../sinbaram/sinItem.h
        ↓
Server sinItem.h encontra guard já definido
        ↓
conteúdo do Server sinItem.h é suprimido
        ↓
InitializeServerItemCodeMap não é declarado
```

Essa descoberta é arquitetural e será registrada separadamente como novo
bug.

## 12. Correção de baixo risco para o 051

Não devemos renomear agora os include guards dos headers legados. Isso
poderia fazer os dois headers coexistirem no mesmo TU e revelar
redefinições ou incompatibilidades muito além do escopo do recovery de
itens.

A solução isolada é criar:

``` text
SrcServer/src/sinbaram/server_item_code_map.h
```

com:

``` cpp
#pragma once

int InitializeServerItemCodeMap();
```

e incluí-lo diretamente em `OnSever.cpp`.

A declaração adicionada anteriormente ao Server `sinItem.h` deve ser
removida.

Assim, a API específica do recovery não depende do header legado cuja
seleção está contaminada.

## 13. Higiene do source

Durante a investigação foi encontrado um byte NUL no final de:

``` text
SrcServer/src/sinbaram/sinItem.cpp
```

na região:

``` text
return TRUE;
}
\x00
```

O NUL foi removido e o EOF normalizado.

Após isso:

``` text
NUL count: 0
```

e `git diff --check` deixou de reportar o blank line adicional no EOF.

## 14. Estado de validação

Antes da correção do header dedicado, o build falhava apenas em
`OnSever.cpp` por ausência da declaração.

A correção atual está sendo buildada.

Critérios de sucesso imediatos:

``` text
1. OnSever.cpp compila.
2. Link do servidor completa.
3. Servidor inicia sem crash.
4. Log contém:
   [STARTUP][ITEM_CODE_MAP] populated=1723
5. SITEM deixa de mostrar categories=0/codes=0.
6. WA101, WA108, BC102 etc. encontram LastCategory.
7. [AFTER] passa a apresentar CODE != 0.
```

## 15. Próxima prova runtime

Os controles prioritários são:

``` text
WA101
WA108
BC102
BC117
BI156
BI330
```

O primeiro resultado esperado para `WA101` é aproximadamente:

``` text
[SITEM] PARSED ... value="WA101"
[SITEM] MATCH ...
[AFTER ] ... CODE=0x01010100 ...
```

Os valores exatos dos demais campos devem ser observados, não
presumidos.

## 16. Limitações conhecidas

O mapa cobre 97,90% dos CODE únicos presentes nos OpenItem. Permanecem
37 CODE únicos sem entrada, incluindo:

``` text
BI901
CA115
CA715
CX114
CX115
DA649..DA664
DB226
DG226
JV101..JV108
OM226
TW160
WD109
WD121
WN109
WN121
```

Esses itens não devem ser "inventados". Precisam de recuperação por
fonte histórica, relação semântica ou outro artefato confiável.

Também existem 11 códigos internos duplicados no mapa. Isso deve ser
tratado como dado legado a investigar, não corrigido automaticamente.

## 17. Decisões de engenharia preservadas

-   Não converter massivamente os sources para outro encoding.
-   Não substituir tokens por aparência visual de mojibake.
-   Não reconstruir os 37 CODE ausentes por adivinhação.
-   Não renomear agora os guards de `sinItem.h`.
-   Manter instrumentação até obter prova runtime.
-   Fazer mudanças mínimas, auditáveis e reversíveis.
-   Separar o bug arquitetural de headers do bugfix funcional 051.

## 18. Próximos passos após o build

Se o servidor iniciar e o mapa reportar 1723 entradas:

1.  capturar `server_startup_items.log`;
2.  verificar os seis controles;
3.  contar quantos dos 1826 itens passam a ter `CODE != 0`;
4.  separar falhas por:
    -   CODE ausente do mapa;
    -   token de parser ainda corrompido;
    -   nome/nível ainda não recuperado;
    -   outro problema estrutural;
5.  atualizar `before/after`, hashes e diff do bugfix;
6.  só então expandir a recuperação dos demais tokens `DecodeItemInfo`.

Se o build ainda falhar, a nova falha deve ser tratada como evidência
independente; não alterar o mapa até identificar a causa.
