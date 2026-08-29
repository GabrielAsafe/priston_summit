# Novo bug --- Colisão de include guard entre sinItem.h do Game e do Server

## Resumo

Foi descoberto um defeito arquitetural na árvore de headers do Summit
Tale: os headers de item do cliente e do servidor compartilham o mesmo
include guard e podem ser incluídos no mesmo translation unit por
cadeias diferentes.

Arquivos envolvidos:

``` text
SourceCode/SrcGame/src/sinbaram/sinItem.h
SourceCode/SrcServer/src/sinbaram/sinItem.h
```

Ambos usam:

``` cpp
#ifndef _SINITEM_HEADER_
#define _SINITEM_HEADER_
```

Isso permite que a inclusão de um deles silenciosamente suprima o outro.

## Status

**Aberto.**\
**Descoberto durante:** Bugfix 051 --- recuperação da pipeline
`DecodeItemInfo/OpenItem`.\
**Severidade sugerida:** Alta para manutenção/build; impacto runtime
indireto e potencialmente amplo.\
**Prioridade imediata:** não corrigir estruturalmente durante o 051;
isolar APIs novas em headers inequívocos.

## 1. Evidência que revelou o problema

Foi adicionada ao Server `sinItem.h` a declaração:

``` cpp
extern int InitializeServerItemCodeMap();
```

`OnSever.cpp` já continha:

``` cpp
#include "../sinbaram/sinItem.h"
```

Mesmo assim o build reportou:

``` text
SrcServer/src/Server/OnSever.cpp:2224:17:
error: 'InitializeServerItemCodeMap' was not declared in this scope
```

Ao mesmo tempo:

-   a definição da função existia em
    `SrcServer/src/sinbaram/sinItem.cpp`;
-   `sinItem.cpp` compilava;
-   `OnSever.cpp` era o ponto que não enxergava a declaração.

Isso descartou uma simples ausência de definição e apontou para a cadeia
de includes/preprocessamento.

## 2. Include explícito em OnSever.cpp

O servidor possui:

``` cpp
#include "../sinbaram/sinItem.h"
```

Logo, pela leitura superficial do source, a declaração deveria estar
disponível.

O fato de não estar mostrou que o arquivo nominalmente incluído não
estava contribuindo com seu conteúdo naquele ponto.

## 3. Colisão comprovada

Os dois headers possuem o mesmo guard:

``` cpp
#ifndef _SINITEM_HEADER_
#define _SINITEM_HEADER_
```

Portanto, a seguinte ordem é suficiente para produzir o bug:

``` text
incluir SrcGame/.../sinItem.h
        ↓
_SINITEM_HEADER_ definido
        ↓
incluir SrcServer/.../sinItem.h
        ↓
#ifndef falha
        ↓
header do servidor inteiro é ignorado
```

Não há erro do preprocessador. A supressão é silenciosa.

## 4. Como o header Game entra no build do Server

O repositório atual contém:

``` text
Shared/items.h
```

com referência explícita ao header Game:

``` cpp
#include "SrcGame/src/sinbaram/sinItem.h"
```

O build do servidor também possui paths tanto de Server quanto de Game,
incluindo:

``` text
-ISrcServer/src
-ISrcServer/src/sinbaram
-ISrcGame/src
-IShared
```

Assim, o build server não está arquiteturalmente isolado da camada Game.

O preprocessamento investigado mostrou que o header Game pode entrar
antes do Server `sinItem.h`.

## 5. Impacto observado

Impacto já comprovado:

``` text
declaração adicionada apenas ao Server sinItem.h
→ invisível em OnSever.cpp
→ erro de compilação
```

Esse caso foi detectado porque a nova função não existia no header Game.

## 6. Impactos potenciais

A colisão pode mascarar problemas mais antigos:

-   tipos diferentes entre Game e Server com o mesmo nome;
-   macros ou constantes server-only ausentes;
-   membros de estruturas divergentes;
-   prototypes diferentes;
-   comportamento dependente da ordem dos includes;
-   builds que "funcionam" apenas porque um header específico entrou
    primeiro;
-   mudanças legítimas no Server `sinItem.h` sem efeito em determinados
    TUs;
-   mudanças no Game `sinItem.h` afetando acidentalmente o servidor;
-   diferenças entre MSVC e MinGW devido a ordem/resolução de includes;
-   bugs difíceis de reproduzir quando um include novo altera a ordem do
    preprocessamento.

Esses impactos são hipóteses de risco. O único impacto diretamente
provado até agora é a supressão da declaração
`InitializeServerItemCodeMap()`.

## 7. Relação com Shared/items.h

Há ainda uma anomalia relacionada.

Historicamente:

``` cpp
sITEM sItem[MAX_ITEM] = {
#include "items.h"
};
```

esperava um catálogo de inicializadores.

No Summit atual, `Shared/items.h` não é esse catálogo e inclui o
`sinItem.h` do Game.

Isso criou duas classes de problema relacionadas:

``` text
items.h deixou de materializar catálogo histórico
+
items.h introduz dependência explícita Game -> sinItem.h
```

A primeira está sendo contornada no bugfix 051 com
`item_code_map_mingw.inc`.

A segunda pertence a este novo bug arquitetural.

## 8. Por que não corrigir os guards imediatamente

Uma correção aparentemente simples seria trocar:

``` cpp
_SINITEM_HEADER_
```

por guards únicos, por exemplo:

``` text
_SUMMIT_GAME_SINITEM_HEADER_
_SUMMIT_SERVER_SINITEM_HEADER_
```

Porém isso altera a semântica histórica do preprocessamento.

Hoje, a colisão impede que os dois headers sejam processados
simultaneamente. Remover a colisão pode fazer ambos entrarem no mesmo TU
e revelar:

-   redefinições;
-   structs duplicadas;
-   classes duplicadas;
-   globals duplicados;
-   macros incompatíveis;
-   overloads/prototypes conflitantes.

Portanto, renomear guards sem uma auditoria prévia tem raio de impacto
desconhecido.

## 9. Mitigação adotada no bugfix 051

A API nova do recovery de itens será isolada em:

``` text
SrcServer/src/sinbaram/server_item_code_map.h
```

com:

``` cpp
#pragma once

int InitializeServerItemCodeMap();
```

`OnSever.cpp` incluirá esse header diretamente.

A declaração será removida do Server `sinItem.h`.

Isso resolve a necessidade imediata sem alterar a arquitetura legada.

## 10. Plano recomendado para corrigir este bug

### Fase A --- auditoria somente leitura

1.  listar guards duplicados entre `SrcGame` e `SrcServer`;
2.  localizar todos os TUs server que incluem headers Game;
3.  executar preprocessamento dos principais TUs;
4.  registrar qual `sinItem.h` entra primeiro;
5.  localizar referências a símbolos exclusivos de cada versão;
6.  comparar estruturalmente os dois `sinItem.h`.

### Fase B --- teste de coexistência

Em branch/bugfix isolado:

1.  atribuir guards temporariamente únicos;
2.  preprocessar sem necessariamente linkar;
3.  coletar todas as redefinições/conflitos;
4.  classificar conflitos por tipo;
5.  decidir se os headers devem:
    -   ser unificados;
    -   compartilhar um header comum;
    -   permanecer separados;
    -   ser encapsulados por adapters server/game.

### Fase C --- saneamento de Shared/items.h

Determinar a responsabilidade correta de:

``` text
Shared/items.h
```

Perguntas a responder:

-   deveria esse arquivo ser realmente compartilhado?
-   por que ele inclui diretamente `SrcGame/src/sinbaram/sinItem.h`?
-   qual arquivo deveria fornecer o catálogo `sItem`?
-   o nome `items.h` está sendo reutilizado para responsabilidades
    incompatíveis?
-   o catálogo histórico deve ser gerado a partir de uma fonte canônica?

### Fase D --- correção

Somente depois das fases anteriores:

1.  tornar guards únicos;
2.  eliminar includes cruzados desnecessários;
3.  mover tipos realmente compartilhados para `Shared`;
4.  separar APIs Game e Server;
5.  compilar cliente e servidor completos;
6.  executar startup instrumentado dos dois;
7.  comparar comportamento antes/depois.

## 11. Critérios de aceite

O bug poderá ser considerado resolvido quando:

``` text
1. Game sinItem.h e Server sinItem.h possuem identidade inequívoca.
2. Nenhum header é suprimido acidentalmente por guard de outro arquivo.
3. O servidor não depende de incluir sinItem.h do Game por acidente.
4. APIs server-only permanecem visíveis independentemente da ordem de includes.
5. Cliente e servidor compilam.
6. Startup de ambos funciona sem regressão.
7. A pipeline de itens mantém os resultados validados pelo bugfix 051.
```

## 12. Probes úteis para a investigação futura

Guards:

``` bash
grep -nE \
  '^#ifndef|^#define|^#pragma once' \
  SrcGame/src/sinbaram/sinItem.h \
  SrcServer/src/sinbaram/sinItem.h
```

Includes cruzados:

``` bash
grep -Rna \
  --include='*.h' \
  --include='*.cpp' \
  'SrcGame/src/sinbaram/sinItem.h' \
  SrcServer Shared
```

Declarações exclusivas no preprocessado:

``` bash
grep -n 'InitializeServerItemCodeMap' /tmp/OnSever.pre.cpp
```

## 13. Relação com o bugfix 051

Este bug foi descoberto graças ao 051, mas não deve ser incorporado como
uma refatoração ampla dentro dele.

Separação recomendada:

``` text
051
    restaurar funcionalidade da pipeline de itens
    popular LastCategory -> CODE
    recuperar tokens CP949
    provar runtime

novo bug
    corrigir arquitetura de headers
    remover colisão de include guards
    reduzir dependências Game/Server cruzadas
```

Essa separação preserva causalidade e torna eventual regressão muito
mais fácil de diagnosticar.

## 14. Estado no momento da abertura

A mitigação `server_item_code_map.h` está sendo submetida a build.

Ainda não há, nesta documentação, resultado desse build. O resultado
deve ser anexado posteriormente como evidência, sem reescrever
retroativamente o estado registrado aqui.
