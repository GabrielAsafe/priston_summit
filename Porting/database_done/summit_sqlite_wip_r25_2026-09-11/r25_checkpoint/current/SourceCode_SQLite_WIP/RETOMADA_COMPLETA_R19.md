# RETOMADA COMPLETA — Summit / SQLite WIP R19

Data: 2026-09-11

Este é o ficheiro principal para retomar o projeto a partir do R19. Ele substitui o R18 como resumo operacional, mas **R6.1 continua sendo a baseline funcional**.

## 1. Regra zero

- `port_rebuild_r6_1.zip` é a última versão comprovada pelo utilizador em compile/link/runtime.
- R6.1 = oracle de regressão.
- R19 = WIP experimental de persistência/web/session/billing cleanup.
- Não promover R19 por causa de testes estáticos.
- Não corrigir loaders, collision, rendering, ownership, UB ou gameplay não relacionado durante esta migração.

Hash histórico R6.1:

`8d793bb604cfa910cc0677332e22615692cd77eac69c4e87f2c2c27b39eb1c1d`

Fontes históricas:

- `pre_refactor.zip` = verdade funcional antiga;
- `final_07.zip` = verdade estrutural desejada;
- R6.1 = reconstrução confirmada funcional.

## 2. Objetivo arquitetural

```text
cliente -- protocolo jogo --> servidor
                              |
                              v
                     Database/summit.sqlite3

backend futuro -------------> mesmo SQLite
```

- cliente nunca abre SQLite;
- sem PHP/ASP/ASPX como camada obrigatória;
- servidor acessa SQLite diretamente;
- backend futuro pode partilhar a DB respeitando WAL/transações/migrations.

## 3. Decisões de escopo

- não importar dados históricos SQL Server; DB limpa é aceitável;
- depois de runtime estável, entregar queries comprovadas para criar `user`, `GM`, `admin`;
- depois entregar mapa/guia prático da DB;
- alvo futuro pode ser multiplayer simples, não MMO comercial completo;
- exportação de personagens é futura;
- billing/MMO só pode ser removido depois de identificar e preservar/decidir efeitos reais de gameplay.

## 4. Estado SQLite

- ficheiro: `Database/summit.sqlite3`;
- WAL;
- foreign keys ON;
- synchronous NORMAL;
- busy timeout 5000 ms;
- FULLMUTEX;
- API legacy `SQLConnection` + singleton `SQL` preservada sobre backend SQLite;
- transações `BEGIN IMMEDIATE`, commit/rollback e rollback defensivo em Close;
- bootstrap via `PRAGMA user_version`.

Schema atual:

- migrations `001..029`;
- `user_version=29`;
- 332 user tables;
- 326/326 tabelas recuperadas dos 21 `.bak` representadas;
- 5.568 colunas catalogadas dos backups;
- seis tabelas adicionais são source-derived, não backup-proven.

R19 **não altera schema**.

## 5. Web/HTTP

Continua como R18/R17:

- único HTTP funcional conhecido no servidor: `clanChipChange.asp`;
- é transferência de liderança de clan;
- permanece bloqueado por falta do contrato completo de códigos/writes/logs;
- não aproximar;
- WinInet não pode ser removido enquanto esse caminho continuar ativo.

## 6. Login antes do R19

R18 aceitava localmente apenas o subconjunto free comprovado em `AccountDB__<Inicial>GameUser`:

- password exata;
- GPCode zero/empty;
- `UsePeriod=0`;
- `inuse=0`;
- sem block/delete;
- `RNo=0`;
- `BNum=0`;
- Channel zero/empty;
- `DisuseDay` não expirado.

Casos paid/PC-room/ambíguos continuavam fallback para `Database.dll`. Sem DLL, falhavam fechados.

Lacuna R18: o login local lia `inuse=0`, mas não gravava o ownership da sessão na DB.

## 7. Database.dll — evidência acumulada

DLL fornecida pelo utilizador:

SHA-256 `c9fd54668cba6e00fa760bda541587635f51c962ab6ef2b19179d0651e7220a7`

- 101 exports;
- 37 símbolos pedidos pelo bridge servidor;
- 21/37 presentes;
- mistura auth/session, PC-room/accounting, gameplay persistence e logging.

R18 já migrou local-first:

- `ClanMoney`;
- `SiegeMoney`;
- `SiegeMoneyTax`.

Consultar:

- `docs/DATABASE_DLL_GAMEPLAY_AUDIT_R18.md`;
- `migration/DATABASE_DLL_DEPENDENCIES_R19.tsv`.

## 8. Billing/PC-room que ainda afeta gameplay

`Bl_RNo > 0` está provado como gameplay state:

- +20 EXP solo;
- +20 EXP party;
- +50 no acumulador de drop;
- ativa PCBang Pet.

Também continuam relevantes:

- `DisuseDay`, `dwPeriodTime`, `dwPeriodTime2` podem expirar/desligar a sessão;
- `PCUse` e `PCSearchReserv` atualizam/consultam estado em runtime;
- `/time` usa `PCRNoReservProduct` quando `Bl_RNo>0`.

Logo, R19 **não** apaga nem zera estes campos.

## 9. R19 — contrato de sessão recuperado

A DLL contém os writes históricos essenciais.

Claim/login free:

```sql
update %s set StartDay = '%s', EditDay ='%s', RNo=NULL,
ServerName ='%s', inuse='1', SNo='%s' where userid = '%s'
```

Release/logout:

```sql
UPDATE %s SET LastDay = '%s', inuse='0', RNo=NULL, SNo=NULL
WHERE userid = '%s' and ServerName='%s'
```

Isto prova que `inuse`, `ServerName`, `SNo` e os timestamps pertencem ao lifecycle da sessão e podem ser migrados sem reconstruir charging.

## 10. R19 — login local passa a reclamar a sessão atomicamente

`TrySQLiteLocalLogOn` agora:

1. abre `AccountDB`;
2. inicia `BEGIN IMMEDIATE`;
3. lê/valida o mesmo subset free do R18;
4. cria GUID;
5. grava `StartDay`, `EditDay`, `ServerName`, `inuse='1'`, `SNo`, `RNo=NULL`;
6. commit;
7. só então devolve sucesso.

Consequência: um segundo processo/servidor partilhando o mesmo SQLite não consegue observar e reclamar simultaneamente o mesmo `inuse=0` enquanto o primeiro commit está pendente.

## 11. R19 — logout local-first

Novo helper `TrySQLiteLocalLogout`.

Ele só considera a sessão local se a linha provar:

- assinatura free/local;
- `inuse='1'`;
- `SNo` igual ao GUID enfileirado no logout;
- `ServerName` igual ao servidor atual.

Nesse caso grava transacionalmente:

- `LastDay=datetime('now','localtime')`;
- `inuse='0'`;
- `RNo=NULL`;
- `SNo=NULL`.

Se a linha não for claramente local/free, o `Database.dll!SQLLogout` continua fallback.

## 12. R19 — stale-session cleanup

Novo `ResetSQLiteLocalSessionsForServer`.

Limpa apenas linhas que correspondam simultaneamente a:

- server atual;
- `inuse='1'`;
- `SNo` não vazio;
- GPCode zero/empty;
- `UsePeriod=0`;
- `BNum=0`;
- Channel zero/empty;
- `RNo` zero/null.

Executa:

- no arranque, após `BillSql.Init()` / optional `SetAllGameUser`;
- no `bSql_ServerExit()` final.

Não executa em `bSql_ServerDown()`: o source chama ServerDown no início do countdown, e a DLL observada só mostra bookkeeping/insert de server-down, não o release por conta.

## 13. Validação R19

Novo:

- `validate_sqlite_local_session.py`.

Ele prova:

- claim da conta free;
- `inuse=1` + server + GUID + timestamps;
- segundo claim recusado;
- GUID errado não liberta sessão;
- logout correto limpa a sessão;
- stale reset limpa local/free;
- paid row preservada;
- sessão de outro servidor preservada.

Todos os validators R18 continuam PASS neste ambiente. Raw: `docs/R19_VALIDATION_RAW.txt`.

## 14. O que ainda não foi provado

Nenhum validator substitui:

- compile target Windows;
- link target Windows;
- startup real;
- login free end-to-end;
- segundo login simultâneo real;
- disconnect/reconnect;
- crash/restart stale-session recovery;
- character create/load/save;
- inventory/warehouse;
- quests;
- coin shop;
- clan runtime;
- ClanMoney/SiegeMoney;
- Bless Castle tax;
- SoD2;
- restart persistence;
- WAL concurrent backend;
- billing/PC-room replacement;
- final removal ODBC/Database.dll/WinInet.

R6.1 continua oracle.

## 15. Próximos blockers por prioridade

### A. Windows parity para R19 session lifecycle

Testar especificamente:

- login free;
- segundo login da mesma conta;
- disconnect/reconnect;
- logout queue;
- normal shutdown;
- kill/crash + restart;
- account row antes/depois (`inuse`, `SNo`, `ServerName`, timestamps).

### B. Entitlement substituto para `Bl_RNo`

Não reconstruir billing comercial inteiro. Decidir explicitamente se o servidor simplificado:

- mantém um entitlement/perk local equivalente;
- ativa o bônus globalmente;
- remove os bônus como mudança de design aprovada.

Só depois retirar `PCUse`, `PCSearchReserv`, `PCRNoReservProduct` e provider/accounting relacionado.

### C. Logs opcionais

`CharLog`, `CharRecordLog`, item/trade/post logs e `WithSOD2RecordRanks` continuam sem prova de serem gates de gameplay. Decidir SQLite vs disable após runtime.

### D. `clanChipChange.asp`

Ainda precisa de códigos 1..6/100 e writes/logs exatos antes de migração.

### E. Dependencies finais

Quando zero responsabilidades necessárias dependerem delas:

1. retirar WinInet quando `clanChipChange` for resolvido;
2. retirar Database.dll;
3. retirar ODBC;
4. target link/runtime após cada remoção.

## 16. Artefactos R19

- source completa R19;
- `migration/sqlite_wip_r18_to_r19.patch`;
- `migration/SQLITE_WIP_R19_CHANGED_FILES.tsv`;
- `migration/DATABASE_DLL_DEPENDENCIES_R19.tsv`;
- `docs/SQLITE_LOCAL_SESSION_AUDIT_R19.md`;
- `docs/SQLITE_WIP_R19_VALIDATION.md`;
- `docs/R19_VALIDATION_RAW.txt`;
- `validate_sqlite_local_session.py`;
- `CONHECIMENTO_CONSOLIDADO_CHAT_R19.md`;
- `RETOMADA_SQLITE_R19.md`;
- este `RETOMADA_COMPLETA_R19.md`.

## 17. Regras para próxima sessão

1. Ler este ficheiro antes de editar.
2. R6.1 continua baseline funcional.
3. R19 deve ser congelado depois de empacotado; próximo trabalho = R20.
4. Não importar dados históricos.
5. Não apagar billing/PC-room antes de decidir/preservar os efeitos de `Bl_RNo`.
6. Não mudar os bônus de `Bl_RNo` sem decisão explícita/parity test.
7. Não aproximar `clanChipChange`.
8. Não adicionar nova TU servidor além da `SQLiteConnection.cpp` já autorizada.
9. Repetir todos os validators depois de cada delta.
10. Só depois de Windows runtime estável criar o guia final da DB e queries user/GM/admin.

## 18. Estado em uma linha

**R19 mantém schema v29/332 e todo o R18, acrescentando ownership de sessão free/local transacional no login, logout SQLite local-first e cleanup stale seletivo, enquanto deixa paid/PC-room e todos os efeitos de `Bl_RNo` deliberadamente no domínio legacy até uma decisão explícita de produto/runtime.**
