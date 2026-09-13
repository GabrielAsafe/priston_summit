# RETOMADA COMPLETA — Summit / porte R6.1 + migração SQLite WIP R17

Data do checkpoint: 2026-09-06

Este é o ficheiro principal para retomar o desenvolvimento numa nova conversa/sessão. Ele consolida o checkpoint original, todas as decisões e descobertas relevantes feitas durante a migração SQLite nesta conversa, o estado exato do R17, o que foi provado, o que continua bloqueado e a sequência segura para continuar.

---

## 1. Regra número zero: baseline funcional

A última versão comprovada pelo utilizador em compile/link/runtime é **R6.1** (`port_rebuild_r6_1.zip`).

Contrato permanente enquanto a migração SQLite não passar validação real no Windows:

- R6.1 = oracle funcional/regressão;
- qualquer R2..R17 = WIP experimental de persistência/web;
- não promover WIP a baseline por causa de validadores estáticos;
- não corrigir gameplay, collision, rendering, loaders, ownership, UB ou refactors não relacionados durante a migração DB;
- quando algo quebrar, comparar primeiro contra R6.1.

Hash histórico da baseline R6.1, conforme checkpoint original:

`8d793bb604cfa910cc0677332e22615692cd77eac69c4e87f2c2c27b39eb1c1d`

As fontes históricas continuam:

- `pre_refactor.zip` = verdade funcional antiga;
- `final_07.zip` = estrutura arquitetural desejada;
- R6.1 = combinação reconstruída e confirmada funcional.

---

## 2. Objetivo arquitetural final

Persistência desejada:

```text
cliente  -- protocolo do jogo --> servidor
                                  |
                                  v
                         Database/summit.sqlite3

backend futuro ------------------> mesmo SQLite
```

Regras arquiteturais:

- servidor fala diretamente com SQLite;
- cliente **nunca** abre SQLite;
- PHP/ASP/ASPX/site não devem ser camada obrigatória de persistência;
- serviços web legacy devem ser substituídos por SQLite, storage local ou protocolo do servidor quando o comportamento estiver provado;
- backend futuro pode abrir a mesma DB, respeitando WAL/transações/migrations;
- alterações de schema não devem ocorrer concorrentemente entre backend e servidor.

---

## 3. Estado atual do R17 em uma linha

**R17 mantém schema SQLite v29/332 tabelas, cobre as 326 tabelas encontradas nos 21 backups SQL Server, mantém o login local somente para o subconjunto free comprovado, migrou praticamente todo o clan/SoD2/notice/marks para local, e reduziu o bridge servidor a uma única URL HTTP legacy ativa: `clanChipChange.asp`; liderança paga/billing histórico, import de dados e validação Windows continuam pendentes.**

---

## 4. SQLite físico e namespaces lógicos

Ficheiro físico alvo:

`Database/summit.sqlite3`

Os antigos `EDatabaseID` continuam como contextos lógicos. Tabelas físicas são namespaced:

- `AccountDB__...`
- `ServerDB__...`
- `QuestDB__...`
- `CoinShopDB__...`
- `ClanDB__...`
- `SecurityDB__...`
- etc.

Isso evita colisões reais como `GameMasters` em mais de um banco lógico.

### Singleton SQL

O singleton legacy `SQL::GetInstance()` não carrega `DATABASEID` no call site. O WIP implementou mapeamento histórico explícito de tabelas bare para domínio lógico.

O fallback perigoso “tabela desconhecida -> ServerDB” foi removido no R5. Uma tabela bare desconhecida passa a falhar explicitamente, evitando writes silenciosos no banco errado.

---

## 5. Backend SQLite implementado

`SQLiteConnection` mantém o contrato da API `SQLConnection` para minimizar reescrita de gameplay.

Características implementadas durante a conversa:

- `sqlite3.dll` carregada dinamicamente com `LoadLibraryA`;
- abertura/criação de `Database/summit.sqlite3`;
- `SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX`;
- `PRAGMA foreign_keys=ON`;
- `PRAGMA journal_mode=WAL`;
- `PRAGMA synchronous=NORMAL`;
- busy timeout 5000 ms;
- compatibilidade `Open/Close` com `CRITICAL_SECTION`;
- bind/leitura de integer, short, string, float, int64 e time;
- column length para compatibilidade do singleton SQL;
- `BeginTransaction`, `CommitTransaction`, `RollbackTransaction`;
- transação SQLite usa `BEGIN IMMEDIATE`;
- `Close()` faz rollback se uma transação ficar aberta.

Ainda falta prova runtime completa de tipos/NULL/time versus ODBC antigo.

---

## 6. Bootstrap/migrations

O bootstrap ficou dentro da TU já autorizada `SQLiteConnection.cpp`; não foi criada uma nova translation unit de runtime.

Comportamento:

- `PRAGMA user_version` controla versão;
- migrations aplicadas em ordem;
- cada migration usa transação;
- rollback em erro;
- DB criada por versão futura do servidor é recusada;
- ausência de migration necessária é erro explícito;
- não arranca silenciosamente com schema parcial.

Estado R17:

- migrations `001` até `029`;
- `PRAGMA user_version = 29`;
- **332 user tables**.

---

## 7. Catálogo recuperado dos backups SQL Server

Foram usados 21 `.bak` reais recebidos em `ServidorDBS.zip`.

Foi recuperado diretamente o catálogo SQL Server, incluindo:

- nome da tabela;
- ordem exata das colunas;
- tipo SQL;
- nullability;
- `IDENTITY`.

Resultado consolidado:

- **326 tabelas user** recuperadas dos backups;
- **5.568 colunas** catalogadas;
- todas as 326 têm representação física SQLite no R15+.

R15 adicionou as 176 tabelas backup-proven que faltavam. Migrations 019–029:

- 019 BillingLogDB: 6
- 020 GameLogDB: 17
- 021 ItemLogDB: 3
- 022 LogDB: 5
- 023 PCRoom: 3
- 024 PCRoomLog: 19
- 025 rPTDB: 1
- 026 SoD2DB: 9
- 027 SodDB: 69
- 028 ClanDB: 24
- 029 restantes catálogos: 20

### Seis tabelas source-derived

Existem seis contratos de compatibilidade que **não** devem ser chamados de backup-proven:

- `QuestDB__RepeatedQuest`
- `SecurityDB__AgingRecord`
- `SecurityDB__BannedIP`
- `SecurityDB__BannedMac`
- `SecurityDB__ClassCheatList`
- `StartUserDB__ExtraItems`

Foram criados porque o source ainda os referencia, mas não aparecem nos backups fornecidos. Os callers foram convertidos para colunas nomeadas para não depender de uma ordem inventada.

---

## 8. Lições críticas de schema/SQL descobertas

### SELECT *

Em código legacy, `SELECT *` + `GetData(coluna N)` transforma ordem de colunas em ABI de persistência. Não reordenar tabelas arbitrariamente.

### SQL Server IDENTITY

Uma query SQL Server `INSERT INTO Table VALUES (...)` pode omitir uma coluna `IDENTITY` de forma que não se traduz diretamente para SQLite. Isso foi detectado durante a tentativa de criar `BannedIP`/security no R3 e impediu uma migration errada.

Regra: preferir colunas nomeadas nos callers quando o schema moderno contém IDs ocultos ao contrato legacy.

### Defaults/constraints

O parser dos `.bak` recuperou nome/tipo/nullability/IDENTITY, mas nem todos os defaults/constraints históricos foram recuperados. R16 encontrou um exemplo concreto: `ClanDB.CL.LIDER_BC` é `NOT NULL` no backup moderno, enquanto o `clanInsert.aspx` histórico é anterior a esse campo. O create local inicializa `LIDER_BC=0` explicitamente e documenta isso como adaptação ao schema atual, não como comportamento histórico do ASP.

### Hora local

SQL Server `GETDATE()` é hora local do servidor; SQLite `CURRENT_TIMESTAMP` é UTC. O rewriter/callers foram ajustados para `datetime('now','localtime')` onde equivalência com `GETDATE()` é esperada.

### T-SQL removido/reescrito

Foram tratados explicitamente, entre outros:

- `TOP N` -> `LIMIT N`;
- `GETDATE()`;
- `DAY/MONTH`;
- `DATEADD(DAY,...)`;
- collations conhecidas -> `NOCASE`;
- `ISNULL` -> `COALESCE`;
- `Killed += 1` -> `Killed = Killed + 1`;
- `IF EXISTS/IF NOT EXISTS` em callers específicos;
- `INSERT OR IGNORE/REPLACE` somente quando existe constraint compatível.

O rewriter continua rejeitando control-flow T-SQL genérico em vez de fingir suporte.

---

## 9. Histórico resumido R2 -> R17

### R2

- `SqliteQueryCompat.cpp` deixou de ser segunda TU nova; passou a `.inl` incluída por `SQLiteConnection.cpp`;
- source graph voltou ao contrato de apenas uma TU SQLite nova;
- T-SQL `ISNULL`/`Killed+=1` removido;
- migrations iniciais AccountDB/CoinShop/Quest/PVP;
- constraints para `Complete` e `PVP`;
- seed QuestMonsters idempotente;
- aproximadamente 80 tabelas.

### R3

- bootstrap versionado `user_version` + rollback;
- GETDATE localtime;
- migrations adicionais Progress/VIP/security source-proven;
- aproximadamente v8/88 tabelas;
- início de validação de transações/migrations reproduzível.

### R4

- parser dos `.bak` passou a ser fonte de schema real;
- Rewards/ItemRewards/NoticeSystem/security/start-user e outros layouts desbloqueados;
- callers corrigidos para colunas nomeadas onde IDs modernos quebravam `SELECT *`/VALUES;
- v13/106 tabelas.

### R5

- `PAINEL_NET.Usuario`/`CoinShopDB.Usuario` retirados do caminho JOBS usando o contrato real `CoinShopDB.UserInfo`;
- schemas mínimos source-derived para nomes ausentes em backups;
- singleton bare-table routing tornou-se estrito;
- v15/110 tabelas.

### R6

- primeiras leituras clan migradas para SQLite;
- `CL`/`UL` usados diretamente;
- migration UL;
- v16/111 tabelas.

### R7

- contrato clan roles provado: 0 none, 1 member, 2 leader, 5 sub-chief;
- `UL.Permi='2'` = sub-chief;
- join/kick/self-leave migrados com transações;
- preservado bug/comportamento legacy de banish consumir apenas `clwon1`.

### R8

- `GetAllMyClanMember` local com protocolo `Code=1\rCMem=...`;
- appoint/release sub-chief local.

### R9

- dissolução de clan local e transacional;
- archive em `ClanMainDeleted`/`ClanSubDeleted` antes de delete;
- migration 017;
- v17/113 tabelas.

### R10

- `isKflag`/`isPFlag` removidos do HTTP como acknowledgements locais, sem inventar writes de flags.

### R11

- clan marks: HTTP `/ClanContent/*.bmp` -> `Database/clan_marks/*.bmp`;
- leitura limitada a 64 KiB, valida BMP `BM`, rejeita path traversal;
- clan tag assets: webroot/IIS copy -> `Database/clan_tag_assets` -> `Database/clan_marks`;
- resultado de cópia continua best-effort para preservar comportamento legacy.

### R12

- login local-first para subconjunto free comprovado em `AccountDB__<Inicial>GameUser`;
- password exata, GPCode/período/billing/PC-room/block/delete/channel ausentes, DisuseDay válido e não expirado;
- GUID de sessão local;
- casos pagos/PC-room/ambíguos continuam fallback `Database.dll`;
- notice ativo passou a `SELECT Message, Type`, removendo `SELECT *`.

### R13

- `sod2info.asp` removido;
- index 1 códigos 0..6 reproduzidos via ClanDB;
- index 3 ranking com protocolo CR e limite legacy de 9;
- schema ainda v17/113.

### R14

- BillingDB completo a partir do backup: 43 tabelas;
- v18/156;
- **não** ampliou semântica paga.

### R15

- catálogo backup completo materializado;
- migrations 019–029;
- v29/332;
- 326/326 tabelas dos backups presentes.

### R16

- criação de clan (`clanInsert.aspx`) migrada localmente;
- códigos 100/2/3/1;
- LI allocator, CL+UL transaction, `New clan`, `CMoney=500000`;
- default mark `1000000000.bmp` é cópia local best-effort e não é sintetizado;
- `LIDER_BC=0` como adaptação explícita ao schema moderno.

### R17

- nenhuma migration nova; v29/332 permanece;
- dead legacy web bridge servidor neutralizado:
  - `login.php/LoginCheck.php/InsertUser.php` via `ISAOCHECK::IsaoCheck`;
  - `Login_End.php`;
  - `clan_readClanName.php`;
  - `clan_WarehouseRight.php`;
  - `updateFlag.php`;
  - NPCSound `.yul` downloader;
  - `clan_insertClanImage.php` uploader;
  - `ktj_login.php` / `WEBdb_Exam`;
- `SrcServer/.../HoOpening.cpp` mantém `szJoinWebUrl` para símbolo/ABI, mas sem homepage URL;
- validador novo exige que a única URL HTTP literal restante no bridge servidor seja `clanChipChange.asp`;
- **não** altera `clanChipChange` por falta de contrato completo.

---

## 10. Clan: estado funcional conhecido

Migrado para SQLite/local no servidor:

- isCheckClanJang;
- isCheckClanName;
- ReadClanInfo/check mark;
- isCheckClanMember;
- isClanMember;
- GetAllMyClanMember/read_ClanData;
- join/invite;
- kick/banish;
- self-leave;
- appoint/release sub-chief;
- breakup/dissolution + archive;
- create clan;
- K/P acknowledgement gates;
- read marks;
- tag copy/storage.

### Roles

- `0` = não membro;
- `1` = membro normal;
- `2` = líder (`CL.ClanZang`);
- `5` = sub-chief (`UL.Permi='2'`).

### Member-list protocol

Formato provado pelo parser:

`Code=1\rCMem=<nome>\rCMem=<nome>...`

### Banish legacy quirk

Embora o cliente possa construir `clwon1`, `clwon2`, etc., o ASP Sandurr lido durante a investigação consumia apenas `clwon1`. O WIP preserva isso deliberadamente.

### Criação

- primeiro mark após default: 1000000001;
- allocator `LI.ID=1`;
- note default `New clan`;
- response `Code=1\rCMoney=500000\r`;
- BMP default opcional/best-effort;
- `LIDER_BC=0` é adaptação ao schema recebido.

### Único HTTP clan funcional ainda aberto

`clanChipChange.asp`, chamado pela função legacy de nome enganoso `ISAOCHECK::LeaveClan`, é na realidade **transferência de liderança** a um membro selecionado.

O cliente:

- chama `WebDB.LeaveClan(cldata.name, membroSelecionado, gserver)`;
- cobra 300000 após código 1;
- aceita códigos 1,2,3,4,5,6,100/1004;
- depois passa pelo acknowledgement K/P.

Os backups provam `CL`, `UL`, `ChipLog`, `ClanMainCharChangeLog` e `ClanSubCharChangeLog`, mas ainda não foi recuperado o corpo histórico de `clanChipChange.asp` que prove a sequência exata de writes/logs e o significado exato de todos os códigos 2..6.

**Não implementar por aproximação.**

---

## 11. HTTP/web: estado exato no R17

### Bridge servidor ativo

O validador R17 confirma exatamente uma string literal HTTP restante em `isaocheck_legacy_server.inl`:

`clanChipChange.asp`

### Neutralizados no servidor R17

As APIs abaixo foram mantidas para linkage/compatibilidade, mas não fazem mais request web:

- `ISAOCHECK::IsaoCheck` — fail-closed;
- `ISAOCHECK::IsaoCheck_GameExit` — no-op local;
- `ISAOCHECK::read_ClanName` — fail-closed;
- `ISAOCHECK::WarhouseRight` — fail-closed;
- `ISAOCHECK::updateFlag` — fail-closed;
- `ISAOCHECK::GetWavFile` — fail-closed;
- `ISAOCHECK::write_ClanImageFile` — fail-closed;
- `ISAOCHECK::WEBdb_Exam` — fail-closed.

Call-graph audit não encontrou produtor/caller de produção para esses caminhos que justificasse manter I/O externo. Warehouse/updateFlag continuam sem semântica persistente comprovada; por isso não foram convertidos para writes inventados.

### Cliente

As cópias legacy do cliente ainda contêm strings/endpoints históricos. O cliente não foi alterado como parte desta limpeza do servidor. Não usar isso para concluir que o servidor ainda chama tais endpoints.

### WinInet

Ainda **não remover do linker/runtime**: `clanChipChange.asp` continua usando `connect_WEBdb`, e os helpers WinInet genéricos continuam compilados. Só retirar WinInet quando zero caminhos ativos precisarem dele e build/link Windows confirmarem isso.

---

## 12. Login/auth: estado exato

Login realmente usado pelo servidor passa por `bSql_LogOn`/`SQLLogon`, não pelo antigo `ISAOCHECK::IsaoCheck(login.php...)`.

### Subconjunto SQLite local comprovado

Tabela:

`AccountDB__<A..Z>GameUser`

Campos nomeados usados:

`Passwd, GPCode, DisuseDay, UsePeriod, inuse, BlockChk, DelChk, RNo, BNum, Channel`

SQLite aceita diretamente apenas quando:

- password corresponde;
- GPCode vazio/zero;
- sem período pago;
- sem inuse/block/delete;
- sem RNo/BNum/channel de billing/PC-room;
- DisuseDay parseável e não expirado.

Sucesso local gera novo GUID de sessão.

### Fallback

Estados pagos, provider/channel, PC-room, bloqueados, expirados, ambiguidades e falhas de leitura continuam fallback-only para `Database.dll` se disponível. Sem DLL, estados não comprovados falham fechados.

### Billing

BillingDB possui 43 tabelas reconstruídas do backup e PCRoom/PCRoomLog também existem no schema, mas isso **não prova** a máquina de billing da DLL.

Os rowsets fornecidos de `ACHANNEL`, `Excite`, `NetMarble`, `NTTBilling`, `BillingPointCheck` não forneceram exemplos de dados suficientes para reconstruir com segurança `CHARGE_TYPE`, `END_DAY`, `LEFT_TIME`, `UPDATE_CHECK`, charging e PC-room.

Não remover `Database.dll` até decisão explícita:

1. reconstruir billing histórico com evidência adicional; ou
2. abandonar esse billing e definir um novo modelo local aprovado pelo utilizador.

---

## 13. Notice

O notice ativo do servidor já usa SQLite (`ServerDB__NoticeSystem`).

R12 trocou a dependência posicional:

`SELECT * ...`

por:

`SELECT Message, Type FROM NoticeSystem WHERE Activated=1`

`ISAOCHECK::ReadNotice` web não tinha caller ativo encontrado; não é o mecanismo ativo de notice.

---

## 14. SoD2

`sod2info.asp` foi removido no R13.

### index 1

Contrato local reproduz códigos 0..6 e campos legacy do parser:

- CClanMoney
- CTax
- CName
- CNote
- CZang
- CIMG
- TotalEMoney/TotalMoney quando aplicável

### index 3

Ranking usa `ClanDB__CL ORDER BY Cpoint DESC`, formato CR-delimited:

`Code=1\rCIMG=...\rCName=...\rCPoint=...\rCRegistDay=...\r`

Preservado limite histórico de nove linhas examinadas.

---

## 15. Clan marks/assets

Storage local:

- `Database/clan_marks/<ClanMark>.bmp`
- `Database/clan_tag_assets/<CodeIT>.bmp`

Regras do leitor:

- `OPEN_EXISTING`;
- máximo 64 KiB;
- basename legacy sem `\\`, `/`, `:`, `.`;
- assinatura BMP `BM` obrigatória;
- pipeline antigo `clanDATA_save`/`bmpSAVE` preservado;
- missing mark continua missing.

Tag customização copia localmente via `CopyFileA`, mantendo best-effort do legacy.

Não criar automaticamente `1000000000.bmp` se o asset não existir.

---

## 16. Dados históricos: NÃO importados

Muito importante: R15+ fecha schema, não data migration.

Os `.bak` foram usados como fonte estrutural. Isso **não** significa que contas/personagens/clans/logs/billing históricos estejam dentro de `summit.sqlite3`.

Antes de produção decidir:

- DB limpa e nova vida do servidor; ou
- preservação dos dados SQL Server existentes.

Se houver preservação, fazer import determinístico separado, com relatório de contagens/hashes/rejeições. Não misturar conversão de dados no bootstrap de schema.

---

## 17. ODBC

Ainda não remover ODBC do build/link só por haver SQLite.

Regra original continua válida:

- provar zero callers que dependem do backend ODBC;
- validar todas as queries/tipos;
- compilar/linkar no target;
- só então remover imports/libs/configuração ODBC.

A baseline antiga importava `ODBC32.dll`; o objetivo final é remover isso do servidor, não necessariamente do cliente sem análise separada.

---

## 18. Translation units / estrutura

Contrato estrutural da reconstrução continua preservado.

Validação R17:

- client baseline/current: 201/201;
- server baseline/current: 179/180;
- única adição autorizada: `SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp`;
- build/link scripts congelados: 4;
- estrutura do porte: 1517/1517;
- compiler adaptation ledger: 1 (`cE_chip.cpp` MinGW).

`SqliteQueryCompat` é `.inl`, não uma TU adicional.

---

## 19. Bugs legacy que NÃO devem ser corrigidos agora

A baseline preserva vários problemas reais, incluindo:

- `new[]`/`delete` incorreto;
- double-delete/idempotência ruim;
- `memset(sizeof(pointer))`;
- arrays/contadores incompatíveis;
- type-punning/STL internals;
- bool sem retorno;
- dependência de ABI x86/Win32;
- includes/macros com semântica de definição;
- loaders/collision/rendering com regras frágeis.

Não misturar correções desse ledger com migração DB/web sem aprovação específica.

---

## 20. Validações que passam no R17

Disponíveis neste ambiente e PASS:

- `validate_backup_catalog_completion.py`
- `validate_billingdb_sqlite_contract.py`
- `validate_sod2_sqlite_contract.py`
- `validate_sqlite_migrations.py`
- `validate_sqlite_query_coverage.py`
- `validate_clan_sqlite_contract.py`
- `validate_sqlite_transactions.py`
- `validate_clan_mark_storage.py`
- `validate_clan_tag_storage.py`
- `validate_sqlite_local_login.py`
- `validate_notice_sqlite_contract.py`
- `validate_server_http_surface.py` (novo R17)
- `validate_build_source_graph.py`
- `validate_build_scripts.py`
- `validate_port_structure_presence.py`
- `validate_compiler_adaptations.py`

Resultados-chave:

- schema v29;
- 332 user tables;
- 326/326 backup tables;
- query coverage missing=0;
- apenas 1 HTTP literal no bridge servidor: `clanChipChange.asp`;
- nenhuma URL HTTP/HTTPS direta em `SrcServer` após neutralizar homepage server-side;
- uma única TU servidor nova autorizada.

---

## 21. O que NÃO foi provado

Mesmo com todos os PASS acima, ainda não foi provado no R17:

- compile target Windows da árvore atual;
- link target Windows;
- startup real usando `sqlite3.dll` e migrations;
- login real end-to-end;
- create/load/save character;
- inventory/warehouse runtime;
- quests runtime;
- coin shop runtime;
- clan runtime completo;
- SoD2 runtime;
- restart/persistência;
- WAL com backend concorrente;
- equivalência paid/PC-room billing;
- import dos dados históricos;
- remoção segura de ODBC/Database.dll/WinInet.

Por isso R6.1 continua oracle.

---

## 22. Blockers atuais, por prioridade

### A. `clanChipChange.asp` / transferência de liderança

É o único HTTP literal funcional ainda preservado no bridge servidor.

Não implementar até provar:

- requisitos de líder/alvo/level/money;
- sequência exata de `CL`/`UL` updates;
- semântica de `Permi` do antigo/new leader;
- writes em `ChipLog`/`ClanMainCharChangeLog`/`ClanSubCharChangeLog`;
- significado exato dos códigos 1..6/100;
- rollback/atomicidade desejada.

### B. Billing/PC-room

Precisamos de material adicional ou decisão de produto. Schema não basta.

### C. Data import

Decidir se é obrigatório preservar DB histórica.

### D. Dependencies

Depois de A/B (ou decisão de abandonar B), auditar/remover:

- WinInet;
- Database.dll;
- ODBC;
- configs/web hosts não usados.

### E. Windows validation

Obrigatória antes de promoção.

---

## 23. Próxima sequência recomendada

1. **Congelar R17** e nunca editar esse checkpoint em-place.
2. Investigar `clanChipChange` por script histórico/backup/source; se continuar sem prova, não aproximar.
3. Decidir explicitamente billing: reconstrução histórica ou novo contrato local.
4. Se dados históricos forem necessários, criar ferramenta de import separada.
5. Quando nenhum caller ativo depender de web, retirar WinInet do servidor e testar link.
6. Quando nenhum caller precisar de `Database.dll`, retirar fallback e testar login completo.
7. Quando toda query usar SQLite, retirar ODBC do servidor e build/link.
8. Executar regressão Windows completa contra R6.1.
9. Somente após isso promover nova baseline SQLite.

---

## 24. Regressão Windows obrigatória

Executar pelo menos:

1. compilar servidor;
2. linkar servidor;
3. confirmar cliente R6.1 sem alteração funcional inesperada;
4. iniciar com DB limpa;
5. verificar migrations/user_version;
6. login free;
7. login dos tipos que forem oficialmente suportados;
8. criar/carregar personagem;
9. save/disconnect/reconnect;
10. inventory;
11. warehouse;
12. quests;
13. coin shop;
14. clan: create/read/join/kick/leave/subchief/breakup/marks/SoD;
15. liderança quando migrada;
16. restart do servidor;
17. confirmar persistência;
18. abrir DB concorrente em WAL;
19. testar busy/retry;
20. comparar falhas contra R6.1 antes de alterar gameplay.

---

## 25. Artefactos que devem acompanhar R17

O checkpoint R17 deve conter:

- `current/SourceCode_SQLite_WIP/` — source completa;
- `migration/sqlite_wip_r16_to_r17.patch`;
- `migration/SQLITE_WIP_R17_CHANGED_FILES.tsv`;
- `migration/HTTP_REMAINING_R17.tsv`;
- `docs/SQLITE_WIP_R17_VALIDATION.md`;
- `docs/R17_VALIDATION_RAW.txt`;
- `docs/RETOMADA_ORIGINAL_2026-09-06.md` — checkpoint original fornecido pelo utilizador;
- `CONHECIMENTO_CONSOLIDADO_CHAT_R17.md`;
- este `RETOMADA_COMPLETA_R17.md`;
- documentos R13–R16 mantidos como histórico imediato.

---

## 26. Cuidados para qualquer próxima sessão

1. Ler este ficheiro inteiro antes de editar source.
2. Ler também o `CONHECIMENTO_CONSOLIDADO_CHAT_R17.md`.
3. R6.1 é a baseline funcional; R17 é WIP.
4. Não criar API web intermediária.
5. Não ligar cliente ao SQLite.
6. Não inventar schema/coluna/ordem/default quando houver contrato posicional.
7. Não usar `INSERT OR IGNORE/REPLACE` sem UNIQUE/PK adequada.
8. Não inferir billing a partir apenas dos nomes das tabelas.
9. Não remover WinInet enquanto `clanChipChange` estiver ativo.
10. Não remover `Database.dll` enquanto existirem estados billing/PC-room suportados somente pelo fallback.
11. Não remover ODBC antes de build/link/runtime provar zero dependência.
12. Não corrigir bugs de gameplay/refactor junto desta migração.
13. Criar sempre um novo delta/checkpoint, não editar uma versão congelada.
14. Repetir todos os validadores depois de cada mudança.
15. Se runtime falhar, reproduzir na R6.1 antes de “corrigir” comportamento.

---

## 27. Estado resumido final

**R17 é o checkpoint SQLite/web mais avançado desta conversa: schema completo para todos os catálogos dos backups, backend/migrations/transactions estáveis nos testes disponíveis, clan/notice/SoD2/marks praticamente locais, login free local com fallback conservador, e apenas `clanChipChange.asp` ainda mantém HTTP no bridge servidor. O trabalho restante é majoritariamente transferência de liderança, decisão de billing/data import, remoção final das DLLs/libs legacy e validação compile/link/runtime Windows.**
