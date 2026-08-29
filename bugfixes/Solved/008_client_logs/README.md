# Fix 008 - Client diagnostic logs directory

## Estado

APROVADO.

## Base

9b2146b - fix(server): restore NPC field path resolution

## Problema

Os logs de diagnóstico do cliente eram escritos diretamente na raiz
do diretório do jogo, apesar de já existir a pasta `LOGS`.

## Causa

Os `fopen`/`freopen` usavam caminhos como:

    mingw_dispatch.txt

em vez de:

    LOGS\mingw_dispatch.txt

## Alteração

Foi adicionado somente o prefixo `LOGS\\` aos caminhos literais de
logs de diagnóstico.

A árvore pré-reset foi usada como whitelist. Uma alteração só foi
permitida quando o mesmo ficheiro/caminho já existia nela como:

    LOGS\\nome_do_log

## Segurança do patch

O patch:

- não cria uma nova pasta LOGS;
- não altera lógica de logging;
- não altera formato dos logs;
- não altera assets;
- não altera gameplay;
- preserva line endings dos ficheiros;
- altera apenas strings passadas a fopen/freopen.

## Fora do escopo

Não foram movidos:

- OpeningField.txt;
- ficheiros do Sky;
- image/system.txt;
- BackupData.log;
- error.log.

## Evidências

- before/
- after/
- hashes_before.sha256
- hashes_after.sha256
- replacements.txt
- change.patch
- audit/

## Teste

Executar:

    cd SourceCode
    sh cliente.sh

Depois confirmar:

1. login funciona;
2. personagem entra no mapa;
3. NPCs continuam presentes;
4. os novos traces aparecem em LOGS/;
5. os mesmos traces deixam de receber novas escritas na raiz.

## Resultado

APROVADO.

Teste funcional realizado em 2026-08-24.

Resultado:

- cliente iniciou normalmente;
- login funcionou;
- seleção de personagem funcionou;
- entrada no mapa funcionou;
- NPCs continuaram presentes;
- não houve regressão funcional observada;
- os writers de diagnóstico do cliente foram redirecionados para a pasta LOGS já existente;
- mingw_winmain_reached.txt também foi corrigido para LOGS\\mingw_winmain_reached.txt.

Conclusão:

Fix 008 aprovado.


## Complemento identificado antes do teste

Após a primeira preparação do patch foi identificado mais um trace
ainda escrito na raiz:

    mingw_winmain_reached.txt

Ele também foi alterado para:

    LOGS\mingw_winmain_reached.txt

A alteração faz parte do mesmo Fix 008 porque o objetivo funcional
continua sendo exclusivamente redirecionar logs de diagnóstico do
cliente para a pasta LOGS já existente.
