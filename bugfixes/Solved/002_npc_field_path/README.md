# Fix 002 - NPC field path separator

## Estado

APROVADO.

## Base

Commit base:

    32ba9d7 - fix(server): deploy configured game server address

## Problema

Após o Fix 001:

- login funciona;
- seleção de personagem funciona;
- cliente entra no mapa;
- pet aparece;
- NPCs não aparecem.

## Causa encontrada

Em `SetFieldInfoPath()` existia:

    if(szFieldPath[cnt] == '//')

Em C/C++, `'//'` é uma constante multicaractere, e não o caractere `/`.

A função deveria localizar o último separador do caminho para obter o nome
base do ficheiro de mapa.

`STG_AREA::LoadStage()` usa `SetFieldInfoPath()` para gerar os caminhos:

- `.spp`
- `.spc`
- `.spm`

O `.spc` é carregado por `LoadCharInfoFixed()` e posteriormente os NPCs
válidos são criados através de `OpenNpc()`.

Se o separador não é encontrado, o caminho auxiliar pode ser construído
incorretamente e o `.spc` correspondente não é carregado corretamente.

## Alteração

ANTES:

    if(szFieldPath[cnt] == '//')

DEPOIS:

    if(szFieldPath[cnt] == '/' || szFieldPath[cnt] == '\\')

A alteração permite caminhos usando tanto `/` quanto `\`.

## Ficheiro funcional alterado

- SourceCode/SrcServer/src/Server/OnSever.cpp

## Evidências

- `before/` - cópia integral anterior;
- `after/` - cópia integral posterior;
- `hashes_before.sha256` - SHA-256 anterior;
- `hashes_after.sha256` - SHA-256 posterior;
- `before_after.diff` - alteração exata;
- `change.patch` - patch Git.

## Hash BEFORE

    f4d86d8641cbd7086f303d2bd9cd2592b5dcba7822e7c76e0c5e2dff3b4e4c2d

## Teste necessário

1. Fechar `Server.exe` no Windows.
2. Executar:

       cd SourceCode
       sh server.sh

3. Iniciar o servidor.
4. Entrar no mesmo mapa do checkpoint anterior.

## Critérios de aprovação

- login continua funcionando;
- cliente entra normalmente;
- pet continua presente;
- NPCs aparecem;
- crash do Fix 001 não regressa.

## Fora do escopo

Não altera:

- items;
- drops;
- merchants;
- sky;
- chat;
- buffs;
- logs;
- resize;
- IP/portas;
- cliente.

## Resultado

APROVADO.

Teste funcional realizado em 2026-08-24.

Resultado observado:

- servidor compilou e foi implantado;
- cliente entrou normalmente;
- NPCs/personagens voltaram a aparecer;
- o problema de NPCs ausentes foi resolvido.

Conclusão:

A condição incorreta:

    if(szFieldPath[cnt] == '//')

impedia a resolução correta do caminho auxiliar de field.

A correção:

    if(szFieldPath[cnt] == '/' || szFieldPath[cnt] == '\\')

restaurou o carregamento esperado dos NPCs.

