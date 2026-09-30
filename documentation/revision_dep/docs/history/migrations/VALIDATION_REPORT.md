# Validation Report — Batch 124–126

## Estado de entrada reconstruído

O estado pós-Migration 123 foi reconstruído a partir do snapshot fornecido e da mesma cadeia de migrations/hotfixes usada no porte:

- Migration 048 reaplicada;
- 049–055 reproduzidas como moves path-only conforme os pacotes preparados;
- batches 056–070, 071–085 v2, 086–100;
- 101–108, Migration 109 deferida, 110–115;
- 116–123 v3 com a correção da 120.

A reconstrução chegou ao estado esperado: 20 arquivos C/C++ fora das raízes arquiteturais, sendo `SrcGame/src/opcodes.h` + 19 arquivos em `SrcGame/src/Server`.

## Migration 124 — opcodes legado do Server

Comparação entre `SrcGame/src/opcodes.h` e `Shared/opcodes.h`:

- 277 constantes no header legado do SrcGame;
- 281 constantes no Shared;
- 264 símbolos comuns com expressões/valores idênticos;
- 13 símbolos exclusivos do header legado;
- 17 símbolos exclusivos do Shared;
- 0 símbolos comuns com valores divergentes.

Os 13 símbolos exclusivos do header legado não são usados fora do bloco `Server`; os usos encontrados estão confinados ao código server-side legado, especialmente `OnSever.cpp`.

A migration move o header para:

`SrcGame/src/compat/client_server/server_legacy/protocol/opcodes.h`

Somente os seis includes `opcodes.h` dentro do bloco Server são reescritos. Consumidores normais do cliente não são alterados e continuam resolvendo `Shared/opcodes.h` conforme o build atual.

Validação:

- install: OK;
- audit: OK;
- `Shared/opcodes.h` preservado byte-a-byte;
- conteúdo lógico do header movido preservado;
- rollback: restauração byte-a-byte OK.

Contador: 20 → 19.

## Migration 125 — bloco Server legado do cliente

Move 19 arquivos de `SrcGame/src/Server` para:

`SrcGame/src/compat/client_server/server_legacy`

Classificação do build:

- ativo: `DllServer.cpp`;
- explicitamente inativos/server-side: `ClientFuncPos.cpp`, `OnSever.cpp`, `PartySever.cpp`, `Svr_Damge.cpp`, `svrEventSpawn.cpp`.

A migration também:

- atualiza `cpp_files_client_i686.txt` para o novo caminho de `DllServer.cpp`;
- canonicaliza consumidores dos headers Server;
- remove `-ISrcGame/src/Server`;
- altera o regex de exclusão dos cinco `.cpp` inativos para o novo caminho em `compat/client_server/server_legacy`.

Validação do source discovery (apenas `SrcGame/src`, sem os fontes adicionais de `modules/MD5`):

- antes: 201 fontes descobertas;
- depois: 201 fontes descobertas;
- `DllServer.cpp`: continua presente;
- cinco `.cpp` server-side: continuam ausentes da lista descoberta.

Quoted includes:

- antes: 1792;
- depois: 1792;
- não resolvidos: 54 antes / 54 depois.

Quatro entradas não resolvidas mudam apenas o caminho do consumidor porque o próprio arquivo Server foi movido; são os mesmos includes legados já não resolvidos antes (`Database/Database.h`, `serverlog.h`, `B_ServerMsg.h`, `../veitnam/v_event.h`). Não há aumento na quantidade de dependências não resolvidas.

Validação:

- install: OK;
- audit: OK;
- conteúdo lógico dos 19 arquivos preservado fora de linhas `#include`;
- rollback: restauração byte-a-byte OK;
- C/C++ fora das raízes arquiteturais após a migration: 0.

Contador: 19 → 0.

## Migration 126 — limpeza dos include roots legados

A primeira tentativa de validação detectou uma dependência não-C/C++ legítima antes da entrega:

`systems/inventory/legacy/sinItem.cpp` ainda encontrava `item_catalog_recovered.inc` através de `-ISrcGame/src/sinbaram`.

A migration foi corrigida para primeiro mover:

`SrcGame/src/sinbaram/item_catalog_recovered.inc`

para:

`SrcGame/src/systems/inventory/legacy/item_catalog_recovered.inc`

Como `sinItem.cpp` está no mesmo diretório, o include local continua válido sem rewrite do C++.

O preflight confirma que essa é a única dependência restante que usa os include roots legados. Depois remove sete roots:

- `SrcGame/src/sinbaram`
- `SrcGame/src/Skills`
- `SrcGame/src/HoBaram`
- `SrcGame/src/HoBaram/NewEffect`
- `SrcGame/src/HoBaram/NewEffect/math`
- `SrcGame/src/HoBaram/NewEffect/Script`
- `SrcGame/src/HoBaram/NewEffect/Script/lua`

Validação:

- nenhum `.h/.hpp` permanece nessas árvores;
- includes não resolvidos: 54 antes / 54 depois;
- `item_catalog_recovered.inc` preservado byte-a-byte;
- rollback: restauração byte-a-byte OK;
- install/audit final: OK.

## Preservação global

- C/C++ físicos no `SrcGame/src`: 627 antes / 627 depois;
- desconsiderando `Winmain.bak2.cpp`: 626 antes / 626 depois;
- C/C++ fora das raízes arquiteturais: 20 antes / 0 depois.

## Teste do runner

Cenário de sucesso com `cliente.sh` simulado:

- 3 builds chamados;
- 124 → 125 → 126 completaram em ordem;
- estado final com 0 arquivos C/C++ fora das raízes.

Cenário de falha injetada no segundo build:

- build 1 (após 124): sucesso;
- build 2 (após 125): falha injetada;
- runner retornou código 1;
- Migration 126 não foi aplicada;
- comando de rollback da 125 foi exibido.

Durante esse teste foi detectado e corrigido um erro no primeiro runner: `tee` mascarava o status de `cliente.sh`. O runner final usa `pipefail`, portanto a falha real de compilação/link aborta o batch.

## Limitação do ambiente de validação

O ambiente de validação não possui o toolchain completo `i686-w64-mingw32-g++`/dependências do workstation. Assim, o link real do `Game.exe` continua sendo o gate final no ambiente do usuário. O batch executa `cliente.sh` após cada migration e aborta na primeira falha.
