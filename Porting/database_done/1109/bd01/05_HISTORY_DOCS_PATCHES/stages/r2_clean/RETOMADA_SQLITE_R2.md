# RETOMADA — Summit / reconstrução do porte + migração SQLite

Data do checkpoint: 2026-09-06

Este ficheiro deve ser lido primeiro ao retomar o trabalho. Ele resume as decisões, os erros encontrados, o estado funcional confirmado e o estado exato da migração de persistência para SQLite.

---

## 1. Objetivo atual do projeto

O projeto teve duas fases distintas nesta conversa:

1. Recuperar o porte/refactor sem quebrar o comportamento do jogo.
2. Depois de obter uma baseline comprovadamente funcional, iniciar a substituição completa da persistência SQL Server/ODBC + interfaces web legacy por SQLite local persistido junto com o servidor.

A fase 1 foi concluída com sucesso na baseline **R6.1**.

O utilizador confirmou explicitamente que, com R6.1:

- o cliente compila;
- o cliente linka;
- o servidor compila;
- o servidor linka;
- cliente e servidor funcionam como a source antiga (`pre_refactor`).

Portanto **R6.1 é a baseline funcional de referência a partir deste ponto**.

Não voltar a fazer alterações de gameplay/refactor na R6.1 para “melhorar” o código enquanto a migração DB estiver em andamento.

---

## 2. Fontes de verdade

### Comportamento funcional

`pre_refactor.zip`

SHA-256:

`dd91cb339d8f8f64b09ddc765fdf1d5fabc629e3fc6b476982cb62f7376c98c9`

Este é o código antigo que o utilizador declarou funcional e sem os crashes introduzidos pelo primeiro porte.

### Estrutura arquitetural desejada

`final_07.zip`

SHA-256:

`8b5cb8cc5b8985c523af0dffeec574b30efbd5c6899f76c2b0ca2a7be9cfc75b`

O objetivo do rebuild foi manter as pastas, Shared, abstrações, policies/classes e divisão de monólitos do porte, mas fazer o comportamento executado equivaler ao `pre_refactor`.

### Baseline final conhecida como funcional

`port_rebuild_r6_1.zip`

SHA-256:

`8d793bb604cfa910cc0677332e22615692cd77eac69c4e87f2c2c27b39eb1c1d`

Essa é a versão a usar para regressão durante a migração SQLite.

---

## 3. Regra que guiou a reconstrução do porte

Durante a reconstrução, foi adotado o seguinte contrato:

- `pre_refactor` = verdade funcional;
- `final_07` = verdade estrutural.

A arquitetura do porte foi mantida sempre que foi possível provar equivalência. O código funcional antigo foi extraído mecanicamente para Shared/policies/adapters quando necessário.

Não foram aceites mudanças intencionais de:

- algoritmo;
- gameplay;
- collision;
- rendering;
- network;
- parsing/serialização;
- constantes/limites;
- ownership/memória;
- ordem de inicialização;
- validações;
- comportamento MinGW/MSVC.

Exceção necessária: uma adaptação MinGW em `cE_chip.cpp`, descrita abaixo, porque o código original não compila em MinGW.

Todo o detalhe da reconstrução está em `docs/PORT_REBUILD_BUGFIX_R6_1.md`.

---

## 4. Estado final da reconstrução R6.1

Dos 288 ficheiros cujo conteúdo tinha sido alterado pelo porte:

- 135 `structured-safe`;
- 101 `structured-exact`;
- 10 `macro-equivalent`;
- 40 `policy-equivalent`;
- 1 `compiler-adaptation`;
- 1 `direct-exact-port-native`.

Não existem wrappers oracle ativos.

A validação construída durante a conversa prova, entre outras coisas:

- 1204 ficheiros source do baseline cobertos;
- 420 translation units legacy preservadas;
- nenhuma TU legacy removida;
- SQLite era a única TU nova autorizada no servidor;
- ordem relativa das TUs legacy preservada;
- todos os ficheiros da estrutura `final_07` preservados;
- scripts de build/link preservados;
- includes relocados auditados;
- helpers Shared congelados/provados;
- diretivas de preprocessador comparadas de forma estrita;
- strings/encoding tokenizados sem `errors=ignore`;
- recursos não-C/C++ do baseline também foram verificados.

Detalhes em `docs/REBUILD_VALIDATION_R6_1.md` e nos manifests dentro da source.

---

## 5. Problemas encontrados durante o primeiro porte/refactor

Esses problemas são importantes porque mostram quais tipos de mudança não devem ser repetidos.

### 5.1 Servidor — linker

Foram encontrados problemas de linkage introduzidos pelo refactor:

- `SendWareHouseToClient` e `SendQuickMessage` foram declarados `extern` dentro de anonymous namespace em código extraído, fazendo o linker procurar símbolos `(anonymous namespace)::...` em vez dos providers globais.
- `rsPutItemChecked` tinha callers mas o provider tinha deixado de ser transportado corretamente para `OnSever.cpp`.
- `g_GuideHelp` dependia de `#define GUIDEHELP_CPP` antes de incluir `GuideHelp.h`; o refactor moveu o define para um `.inl` incluído tarde demais, deixando apenas `extern` sem definição real.

### 5.2 Cliente — compilação

Foram encontrados:

- `Tier5SkillSubClient.inl` usando `RetryPlayAttack` antes da declaração;
- problema com `sFIELD`/`field.h` e dupla inclusão de header legacy sem include guard;
- `cE_chip.cpp` chamando `SetFontTextColor(HDC, COLORREF)` embora a função existente tenha apenas um argumento.

A solução final R6.1 para `cE_chip` preserva o source legacy e adiciona apenas no MinGW um overload local que encaminha os dois argumentos para GDI `SetTextColor`.

Isto está classificado como `compiler-adaptation`, não como alteração de gameplay.

### 5.3 Cliente — crash e personagens invisíveis no porte antigo

Antes da reconstrução, o `final_06/final_07` revelou vários exemplos de regressão causada por reescrita dos loaders:

- `AddUserCharacter` dereferenciava `lpDinaPattern->Pat` mesmo quando `SetLoadPattern()` falhava;
- canonicalização de path/checksum `.inx` tinha sido perdida numa abstração nova;
- o novo model cache passou a validar `MotionCount <= NPC_MOTION_INFO_MAX`, onde `NPC_MOTION_INFO_MAX = 30`, embora `MotionInfo` suporte 512;
- personagens reais tinham motion caches com contagens como 63, 64, 78 e 117 e passaram a ser rejeitados.

Essas correções intermediárias serviram para diagnosticar o problema, mas a solução final foi mais segura: reconstruir os loaders a partir do comportamento exato de `pre_refactor` em vez de manter as reescritas “corrigidas”.

### 5.4 Outras regressões detectadas no primeiro porte

Foram encontradas mudanças semânticas em:

- loader `.smd`/`smPAT3D` que passou a tratar offsets/lengths auxiliares legacy como invariantes obrigatórias;
- `smMATERIAL_GROUP` com validações novas de `MaxMaterial` e consumo estrito de blocos;
- `smOBJ3D` exigindo `nVertex <= MaxVertex` / `nFace <= MaxFace` apesar de metadata legacy poder estar stale;
- `GetEmptyHeight`, `CheckVecImpact`, `GetTriangleImact`, `CheckSolid` com comportamento de collision diferente;
- `MaxFrame` e scale tracks em animações;
- rendering que deixava de desenhar um objeto quando `ReformTexture` falhava;
- dezenas de `#pragma once` adicionados em wrappers que mudavam semântica de reinclusão de headers legacy.

Tudo isso foi usado como evidência para abandonar reescritas funcionais e fazer o rebuild mecanicamente.

---

## 6. Bugs reais e más práticas do código legacy

A R6.1 preserva deliberadamente vários bugs porque o objetivo da reconstrução era equivalência funcional. Não confundir “preservado” com “correto”.

O ledger detalhado está em `docs/PORT_REBUILD_BUGFIX_R6_1.md`.

Principais classes encontradas:

### Memória / UB

- arrays alocados com `new[]` destruídos com `delete` escalar;
- `smPAT3D::Close()` potencialmente não idempotente/double-delete;
- `memset` com `sizeof` incorreto em vários subsistemas;
- `sizeof(pointer)` usado para limpar buffers grandes;
- arrays de tamanho 30/64/100/176 usados com contadores maiores;
- loops que podem indexar além das estruturas;
- variáveis potencialmente não inicializadas;
- type-punning sobre internals de STL/iterators;
- funções `bool` sem retorno em todos os caminhos.

### Persistência / rede

- estruturas persistidas muito próximas do layout de memória C/C++;
- confiança em campos `size` vindos de disco/rede;
- `fread/fwrite` permissivos;
- warehouse e packets com validação insuficiente;
- tabelas/formatos onde ordem e tamanho de campos fazem parte implicitamente do contrato.

### Arquitetura

- globals extensivos;
- `extern/static` usados como arquitetura;
- macros controlando se um header define ou declara um global;
- includes e ordem de includes fazendo parte da semântica;
- APIs Win32 espalhadas por gameplay/persistência;
- duplicação quase igual entre cliente/servidor;
- parsing, I/O e mutação de globals misturados na mesma rotina;
- dependência forte de ABI x86/Win32.

### Importante

Não corrigir esses bugs em massa enquanto a migração DB estiver sendo feita. Fazer uma mudança grande por vez e manter R6.1 como regressão.

---

## 7. Ambiente/runtime aprendido

O `Game.exe` produzido pelo build MinGW declarou imports reais para DLLs como:

- `ADVAPI32.dll`
- `AVIFIL32.DLL`
- `d3d9.dll`
- `d3dx9_43.dll`
- `dsound.dll`
- `GDI32.dll`
- `IMM32.DLL`
- `IPHLPAPI.DLL`
- `KERNEL32.dll`
- `libgcc_s_dw2-1.dll`
- `libstdc++-6.dll`
- `msvcrt.dll`
- `MSVFW32.DLL`
- `ODBC32.dll` (na baseline antiga; alvo de remoção no servidor durante migração)
- `ole32.dll`
- `SHELL32.dll`
- `SHLWAPI.dll`
- `USER32.dll`
- `WININET.DLL`
- `WINMM.DLL`
- `WS2_32.dll`

Foi observado que `libgcc_s_seh-1.dll` não substitui `libgcc_s_dw2-1.dll` para o cliente i686 atual.

`D3DCompiler_43.dll`, `dxbcSigner.dll` e `Ionic.Zip.dll` não apareceram como imports diretos do Game.exe na investigação feita.

---

## 8. Nova fase: SQLite

### Decisão arquitetural final do utilizador

Não haverá mais interface web como camada de persistência do jogo.

Arquitetura desejada:

```
cliente  --protocolo do jogo--> servidor
                                  |
                                  v
                           SQLite direto
                         Database/summit.sqlite3

backend futuro ------------------> SQLite direto
```

Regras:

- servidor fala diretamente com a DB;
- backend futuro também fala diretamente com a DB;
- cliente nunca fala diretamente com a DB;
- PHP/ASP/ASPX/site não serão dependências do servidor;
- notice/clan/etc. que antes dependiam do site devem migrar para DB/local storage/protocolo do servidor;
- downloads HTTP que não são persistência devem ser classificados separadamente antes de remover; o utilizador declarou que não quer interfaces web no jogo, portanto a direção final é eliminar também dependências web de gameplay/serviço.

---

## 9. Materiais de base de dados recebidos

Foram recebidos:

- `database.zip`
- `files.zip`

SHA-256:

- `database.zip`: `fb62496d91f9ffe0ac86808a11bd3f6a8f986c3f3fd0eb66c659f768afa110c3`
- `files.zip`: `cfb1a124b3822fc4fc5ede4dec8e4a136f1a509116ea779bce6251fd702a499f`

`database.zip` contém:

- dumps/scripts SQL Server;
- `summit_schema_sqlite.sql`;
- `create_tables.txt`;
- scripts de criação de várias bases SQL Server;
- seed de `QuestDB_QuestMonsters`;
- esqueleto de persistence/repositories, em grande parte com ficheiros vazios;
- diretórios de clan marks.

Foi validado que `summit_schema_sqlite.sql` possui **69 CREATE TABLE** e consegue criar uma base SQLite sem erro; com `sqlite_sequence` a base de teste tem 70 tabelas.

O seed `QuestDB_QuestMonsters_seed.sql` recebido ainda usa T-SQL (`USE`, `GO`, `IF NOT EXISTS`, `dbo.`) e precisa ser convertido para SQLite idempotente.

Os scripts `server/Persistence/Migrations/*.sql` dentro de `database.zip` são principalmente scripts SQL Server de `CREATE DATABASE`/ALTER DATABASE; não são migrations SQLite prontas.

---

## 10. Inventário SQL do código

Foi feita uma varredura das chamadas SQL no source R6.1.

Artefactos:

- `migration/sql_inventory.json`
- `migration/db_migration_inventory.tsv`

Contagens do inventário bruto:

- 147 ocorrências classificadas como queries via `Prepare`;
- 488 ocorrências/raw SQL detectadas no inventário amplo.

Existem duas APIs de DB no servidor legacy:

1. `SQLConnection`
   - `Open()`
   - `Prepare()`
   - `BindInputParameter()`
   - `Execute()`
   - `GetData()`
   - `NextRow()`
   - `Close()`

2. singleton `SQL::GetInstance()`
   - `Connect()`
   - `Execute()`
   - `Fetch()`
   - `GetDataValue()`

A estratégia escolhida foi **preservar as duas interfaces públicas por enquanto e trocar apenas o backend**. Assim não é necessário reescrever gameplay inteiro na primeira fase da migração.

---

## 11. Uma base física, vários bancos lógicos

Decisão atual:

- um único ficheiro físico: `Database/summit.sqlite3`;
- os antigos `DATABASEID_AccountDB`, `DATABASEID_ServerDB`, `DATABASEID_QuestDB`, etc. continuam existindo como contextos lógicos;
- tabelas físicas no SQLite devem ser prefixadas pelo domínio para evitar colisão:

```
AccountDB__GameMasters
ServerDB__GameMasters
QuestDB__Progress
CoinShopDB__UserInfo
...
```

Isso é necessário porque já foi observada colisão real de nome, por exemplo `GameMasters` usado por mais de um DB lógico.

A camada `SQLConnection` sabe o `EDatabaseID` e pode traduzir automaticamente tabela nua para o namespace físico correto.

O singleton `SQL`, que historicamente não carrega um `DATABASEID`, precisa de mapeamento explícito de tabelas legacy para DB lógico.

---

## 12. Estado EXATO da implementação SQLite WIP

ATENÇÃO: **esta árvore ainda NÃO foi confirmada por compile/link/runtime pelo utilizador.**

A baseline funcional continua sendo R6.1.

A árvore WIP está neste checkpoint em:

`current/SourceCode_SQLite_WIP/`

Ela é R6.1 mais 14 ficheiros adicionados/modificados.

### Ficheiros novos

- `Shared/persistence/database/Database_sqlite_server.h`
- `Shared/persistence/database/Database_sqlite_server.inl`
- `SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.cpp`
- `SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.h`

### Ficheiros modificados

- `Shared/server/legacy_runtime/OnSever_legacy_server.inl`
- `SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp`
- `SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp`
- `SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp`
- `SrcServer/src/systems/persistence/database/core/Database.cpp`
- `SrcServer/src/systems/persistence/database/core/Database.h`
- `SrcServer/src/systems/persistence/database/odbc/SQL.cpp`
- `SrcServer/src/systems/persistence/database/odbc/SQL.h`
- `SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp`
- `SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.h`

O delta completo está em:

`migration/r6_1_to_sqlite_wip.patch`

E o inventário com hashes está em:

`migration/SQLITE_WIP_CHANGED_FILES.tsv`.

---

## 13. SQLiteConnection WIP

`SQLiteConnection` foi desenhado como um backend fino que mantém o contrato de `SQLConnection`.

Características já implementadas no WIP:

- carrega `sqlite3.dll` dinamicamente com `LoadLibraryA`, evitando dependência de linker direta por enquanto;
- abre/cria `Database/summit.sqlite3`;
- usa `SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX`;
- configura:
  - `PRAGMA foreign_keys=ON`;
  - `PRAGMA journal_mode=WAL`;
  - `PRAGMA synchronous=NORMAL`;
  - busy timeout de 5000 ms;
- mantém `CRITICAL_SECTION` para o padrão legacy `Open/Close`;
- implementa bind de:
  - integer;
  - short;
  - string;
  - float;
  - int64;
  - time;
- implementa leitura de:
  - integer;
  - short;
  - string;
  - float;
  - int64;
  - time;
- implementa column length para compatibilidade do singleton `SQL`.

Não foi ainda feita validação completa de equivalência de todos os tipos/NULL/time com ODBC legacy.

---

## 14. SqliteQueryCompat WIP

Foi criada uma camada de tradução de SQL Server legacy para SQLite:

`SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.*`

Já implementa no estado atual:

- nomes qualificados `[DB].[dbo].[Table]` -> `[DB__Table]`;
- formas `DB..Table` e `DB.dbo.Table` -> tabela física prefixada;
- tabelas sem qualificação após `FROM`, `JOIN`, `INTO`, `UPDATE` -> namespace do `EDatabaseID` atual;
- mapeamento histórico para o singleton `SQL` quando `DATABASEID_NULL`;
- `SELECT TOP N ...` -> `SELECT ... LIMIT N` para o SELECT externo;
- `GETDATE()` -> `CURRENT_TIMESTAMP`;
- `DAY(GETDATE())` / `MONTH(GETDATE())` -> `strftime` SQLite;
- `DAY(column)` / `MONTH(column)` em queries legacy;
- `DATEADD(DAY, literal, GETDATE())` -> `datetime(...)`;
- collations SQL Server conhecidas -> `COLLATE NOCASE`.

Deliberadamente ainda rejeita T-SQL control-flow genérico:

- `IF EXISTS(...)`;
- `IF NOT EXISTS(...)`;
- `BEGIN`;
- `ELSE`.

Esses casos começaram a ser migrados explicitamente nos callers para preservar bind numbering/semântica.

Ainda faltam outras construções SQL Server identificadas no inventário, por exemplo:

- `ISNULL(...)`;
- `Killed += 1` e outras atribuições T-SQL;
- `IF EXISTS/NOT EXISTS` remanescentes;
- possíveis queries multi-statement;
- sintaxe específica de identity/insert/update;
- qualquer stored-procedure ou função específica que apareça fora dos casos já mapeados.

Não assumir que toda query do servidor já passa pelo rewriter.

---

## 15. Callers T-SQL já alterados no WIP

Algumas queries foram convertidas explicitamente:

### Coin shop

`CoinShop.cpp` e `CoinShop_T.cpp`:

`IF NOT EXISTS (...) BEGIN INSERT ... GETDATE() END`

foi convertido para `INSERT ... SELECT ... WHERE NOT EXISTS(...)` com `CURRENT_TIMESTAMP`.

### Quest system

`cQuestHandler.cpp` possui alguns `IF EXISTS/IF NOT EXISTS` convertidos para formas SQLite de `INSERT ... SELECT ... WHERE EXISTS/NOT EXISTS`.

Alguns upserts de `Complete` foram convertidos para `INSERT OR REPLACE`.

ATENÇÃO: verificar se o schema final cria constraint/PK/UNIQUE compatível com a semântica esperada do `INSERT OR REPLACE`. Sem essa constraint, a operação não terá o mesmo significado.

### PVP/OnSever

Um `IF NOT EXISTS ... INSERT INTO PVP` foi convertido para `INSERT OR IGNORE`.

ATENÇÃO: `INSERT OR IGNORE` também depende de constraint adequada no schema para implementar “not exists” corretamente.

Essas alterações ainda fazem parte do WIP e precisam ser validadas contra o schema final.

---

## 16. Schema: o que sabemos e o que ainda falta

O `summit_schema_sqlite.sql` recebido cria 69 tabelas e foi testado sintaticamente.

Porém, a source usa vários bancos lógicos e tabelas que não estão cobertos claramente por esse schema consolidado.

O código referencia áreas como:

- AccountDB;
- ServerDB;
- QuestDB;
- CoinShopDB;
- ClanDB;
- SecurityDB;
- BattlePass;
- Billing/BillingLog;
- GameLog/ItemLog/LogDB;
- PCRoom/PCRoomLog;
- SoD/Sod2;
- StartUserDB;
- rPTDB;
- JOBS/Jobs e outras estruturas históricas.

As migrations SQL Server recebidas em `database.zip` criam os DATABASES, mas em grande parte não fornecem o schema real das tabelas.

Portanto o schema final SQLite terá de ser reconstruído de três fontes:

1. `summit_schema_sqlite.sql` e `create_tables.txt` recebidos;
2. queries presentes no código;
3. contratos posicionais de `SELECT *` + `GetData(coluna N, ...)`.

### Regra crítica

Para queries `SELECT *`, **a ordem das colunas faz parte do contrato do código legacy**.

Não criar tabelas apenas com os nomes corretos em ordem arbitrária. É preciso inferir/reconstruir a ordem usada pelos `GetData(position)`.

---

## 17. Interfaces web legacy encontradas

Foi criado:

`migration/HTTP_LEGACY_ENDPOINTS.tsv`

O inventário bruto contém 156 linhas relacionadas a HTTP/WinInet/URLs no código ativo (inclui repetições e chamadas genéricas).

A principal concentração está em `isaocheck`, com endpoints antigos como:

- `login.php`
- `LoginCheck.php`
- `InsertUser.php`
- `Login_End.php`
- `isCheckClanJang.asp`
- `isCheckClanName.asp`
- `isCheckClanNum.asp`
- `isCheckClanwon.asp`
- `isClanMember.asp`
- `clanInsertClanWon.asp`
- `clanInsert.aspx`
- `clan_readClanName.php`
- `GetAllMyClanMember.asp`
- `clanRemove.asp`
- `clanWonRelease.asp`
- `ClanSubChipUpdate.asp`
- `ClanSubChipRelease.asp`
- `clanChipChange.asp`
- `clanWonSelfLeave.asp`
- `isKflag.asp`
- `isPFlag.asp`
- `clan_WarehouseRight.php`
- `updateFlag.php`
- download de notice `.yul`
- clan images `.bmp`
- `clan_insertClanImage.php`
- `ktj_login.php`
- `sod2info.asp`

Também existem URLs legacy como black-warning e homepage/Priston Tale.

### Decisão

Nenhuma dessas rotas deve sobreviver como dependência obrigatória do servidor.

Cada endpoint precisa ser classificado como:

- operação DB -> substituir por repository/SQLite direto;
- asset persistente (clan mark, notice) -> DB/blob ou storage local controlado pelo servidor;
- funcionalidade obsoleta -> remover somente depois de confirmar que não é chamada;
- download externo não-DB -> substituir/retirar em etapa separada, pois o utilizador não quer interfaces web no jogo.

---

## 18. Clan/site legacy

O pacote `database.zip` contém esqueleto de:

- `ClanRepository`
- `ClanProtocolHandler`
- `ClanMarkStorage`
- `DatabaseRouter`
- `ClanClientGateway`

Muitos desses ficheiros estão vazios e **não devem ser tratados como implementação pronta**.

A source real de comportamento continua sendo o código R6.1 e os endpoints/queries legacy.

O caminho recomendado é:

- servidor implementa repository SQLite para clan;
- cliente continua a falar com o servidor pelo protocolo de jogo;
- marca do clan fica armazenada como BLOB ou ficheiro local persistido com metadados em SQLite;
- backend futuro lê/escreve o mesmo SQLite diretamente (com coordenação de locking/transactions adequada).

---

## 19. Concorrência e persistência SQLite

Como servidor e backend futuro podem abrir o mesmo ficheiro, a escolha `journal_mode=WAL` é apropriada para permitir readers concorrentes com writer.

Ainda precisa ser decidido/implementado:

- estratégia de transações por operação;
- índices;
- foreign keys reais;
- migrations versionadas (`PRAGMA user_version` ou tabela de migrations);
- backup consistente;
- compactação/VACUUM política;
- política para backend escrever enquanto servidor está ativo;
- timeout/retry de `SQLITE_BUSY` além do busy timeout atual;
- schema bootstrap automático ao iniciar o servidor.

Não permitir que backend e servidor façam alterações de schema simultaneamente.

---

## 20. Próximas tarefas recomendadas, em ordem

### Etapa A — congelar R6.1

Nunca usar SQLite WIP como novo oracle até ele passar compile/link/runtime.

Sempre poder voltar para `baseline/port_rebuild_r6_1.zip`.

### Etapa B — concluir schema SQLite

1. Criar migrations SQLite versionadas.
2. Converter o schema AccountDB recebido para nomes físicos `AccountDB__*`.
3. Reconstruir schemas ausentes por domínio.
4. Para cada `SELECT *`, documentar ordem das colunas esperada.
5. Criar PK/UNIQUE necessárias para os novos `INSERT OR IGNORE/REPLACE` terem semântica correta.
6. Converter seed QuestMonsters para SQLite idempotente.
7. Criar `Database/summit.sqlite3` automaticamente a partir das migrations.

### Etapa C — completar backend DB local do servidor

1. Fazer `SQLConnection` inteiro funcionar em SQLite.
2. Fazer singleton `SQL` inteiro funcionar em SQLite.
3. Inventariar/rejeitar zero T-SQL não suportado.
4. Remover dependência ODBC do servidor quando todos os caminhos estiverem migrados.
5. Testar primeiro startup, login, character load/save, inventory, warehouse, quests, coin shop, clan.

### Etapa D — remover site/HTTP do servidor

Migrar primeiro endpoints de persistência:

- login/session;
- clan;
- warehouse rights;
- flags;
- clan membership;
- SoD/event info;
- notice;
- clan marks.

Criar repositories explícitos em `SrcServer/src/systems/persistence/...` ou Shared apropriado.

### Etapa E — remover HTTP do cliente quando for serviço web legacy

Não ligar cliente à SQLite.

O cliente deve pedir dados ao servidor usando packet/protocol existente ou um novo opcode interno do jogo.

### Etapa F — backend futuro

O backend futuro pode usar diretamente `Database/summit.sqlite3`, mas deve respeitar schema/migrations/transactions.

---

## 21. Como validar a migração sem confundir regressões

Para cada etapa:

1. compilar servidor;
2. linkar servidor;
3. confirmar que cliente R6.1 continua sem alterações;
4. iniciar servidor com uma DB limpa;
5. verificar criação/migrations;
6. testar login;
7. criar/carregar personagem;
8. salvar/desconectar/reconectar;
9. inventory/warehouse;
10. quests;
11. coin shop;
12. clan;
13. reiniciar servidor e confirmar persistência;
14. testar backend abrindo a mesma DB em WAL sem corromper/lockar o servidor.

Quando um teste falhar, comparar com R6.1 antes de alterar gameplay.

---

## 22. Conteúdo deste checkpoint

### `baseline/`

- `port_rebuild_r6_1.zip`
- hash SHA-256

Última versão confirmada funcional pelo utilizador.

### `current/SourceCode_SQLite_WIP/`

Source completa do estado atual da migração SQLite WIP.

### `docs/`

- contrato/ledger de reconstrução;
- relatório de validação R6.1.

### `migration/`

- SQL inventory;
- DB migration inventory;
- HTTP legacy inventory;
- patch R6.1 -> SQLite WIP;
- lista/hash dos 14 ficheiros alterados;
- schema/database recebidos extraídos;
- `test_summit.sqlite3` criado para validar o schema.

### `references/`

- `pre_refactor.zip`
- `final_07.zip`
- `database.zip`
- `files.zip`
- extrações de database/files para consulta imediata.

### `patches/`

Patches gerados durante diagnóstico/reconstrução, incluindo o patch MinGW R6 -> R6.1.

### `history/`

Checkpoints anteriores gerados durante o trabalho. São históricos/obsoletos; não usar como baseline atual.

### `tools/`

Scripts de auditoria/rebuild usados na reconstrução. Não fazem parte do runtime.

---

## 23. Cuidados ao retomar numa nova conversa/sessão

1. Ler este ficheiro inteiro antes de editar código.
2. Tratar R6.1 como a última versão funcional comprovada.
3. Tratar `current/SourceCode_SQLite_WIP` como código experimental ainda não validado.
4. Não reintroduzir o primeiro `final_07` como base funcional.
5. Não corrigir bugs legacy de gameplay durante a migração DB sem aprovação explícita.
6. Não criar API web intermediária para persistência.
7. Não fazer o cliente abrir SQLite.
8. Não inventar schema quando `SELECT *` depende de ordem de colunas; inferir pelo código.
9. Não usar `INSERT OR IGNORE/REPLACE` sem garantir constraint compatível.
10. Antes de remover ODBC/WinInet do linker, provar que não há caller ativo que dependa deles.

---

## 24. Estado resumido em uma linha

**R6.1 reproduz o jogo antigo e está confirmado funcional; a migração SQLite começou por substituir as duas APIs DB legacy por um backend comum e um rewriter T-SQL, mas schema completo, todas as queries e todas as interfaces HTTP ainda precisam ser concluídos e validados.**

---

## 25. Continuação SQLite WIP R2 — 2026-09-06

A continuação desta sessão produziu um novo estado WIP R2 em `current/SourceCode_SQLite_WIP/`.

Mudanças principais:

- corrigido o source graph: `SqliteQueryCompat` agora é `.inl` incluído por `SQLiteConnection.cpp`; existe novamente apenas uma TU SQLite nova autorizada;
- `ISNULL` e `Killed+=1` removidos dos callers ativos por equivalentes SQLite/SQL standard;
- migrations versionadas `001..004` adicionadas em `Database/migrations/`;
- AccountDB: 69 tabelas prefixadas preservando ordem;
- CoinShopDB: quatro tabelas reconstruídas e testadas;
- QuestDB core: seis tabelas com layout integralmente provado + seed Hopi idempotente;
- `Complete` recebeu a UNIQUE key necessária ao `INSERT OR REPLACE`;
- ServerDB PVP recebeu `UNIQUE(Name)` necessária ao `INSERT OR IGNORE`;
- source graph PASS; scripts de build/link congelados PASS; harness do rewriter 5/5 PASS; migrations e constraints testadas com SQLite local.

A baseline funcional continua R6.1. O WIP R2 ainda NÃO foi compilado/linkado/rodado pelo utilizador.

Detalhes: `docs/SQLITE_WIP_R2_VALIDATION.md`.
Delta sobre o WIP recebido: `migration/sqlite_wip_r2_delta.patch`.
Hashes/ficheiros alterados: `migration/SQLITE_WIP_R2_CHANGED_FILES.tsv`.
