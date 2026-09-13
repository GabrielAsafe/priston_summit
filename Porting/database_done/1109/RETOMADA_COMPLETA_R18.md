# RETOMADA COMPLETA — Summit / SQLite WIP R18

Data: 2026-09-11

Este é o ficheiro principal para retomar o projeto a partir do R18. Ele substitui o R17 como resumo operacional, mas R6.1 continua sendo a baseline funcional.

## 1. Regra zero

- `port_rebuild_r6_1.zip` é a última versão comprovada pelo utilizador em compile/link/runtime.
- R6.1 = oracle de regressão.
- R18 = WIP experimental de persistência/web/billing cleanup.
- Não promover R18 por causa de testes estáticos.
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

## 3. Decisões de escopo atuais

- **não importar dados históricos SQL Server**; DB limpa é aceitável;
- quando runtime estiver estável, entregar queries comprovadas para criar `user`, `GM`, `admin`;
- depois entregar mapa/guia da DB;
- alvo futuro pode ser multiplayer simples, não MMO comercial completo;
- exportação de personagens é futura;
- porém billing/MMO só pode ser removido após identificar e preservar/decidir qualquer efeito real de gameplay.

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

## 5. Web/HTTP

R17 já reduziu o bridge servidor a uma URL HTTP funcional conhecida:

`clanChipChange.asp`

Ela é transferência de liderança de clan e continua bloqueada por falta do contrato completo de códigos/writes/logs. Não aproximar.

Todo o restante migrado/classificado permanece como no R17: clan reads/mutations, marks, tags, SoD2 e notice principal são locais; dead legacy server web foi neutralizado.

WinInet não pode ser removido enquanto `clanChipChange` continuar ativo.

## 6. Login

O login local SQLite aceita somente o subconjunto free comprovado em `AccountDB__<Inicial>GameUser`.

Casos billing/PC-room/paid/ambíguos continuam fallback para `Database.dll` se disponível. Sem DLL, falham fechados.

Isto é intencional: schema de BillingDB está recuperado, mas a máquina histórica de charging/provider não está provada.

## 7. Database.dll — nova evidência R18

Ficheiro fornecido pelo utilizador:

SHA-256 `c9fd54668cba6e00fa760bda541587635f51c962ab6ef2b19179d0651e7220a7`

- 101 exports na DLL;
- 37 símbolos pedidos pelo bridge servidor;
- apenas 21 dos 37 existem na DLL fornecida;
- a DLL mistura auth/session, PC-room/accounting, gameplay persistence e logging.

Consultar:

- `docs/DATABASE_DLL_GAMEPLAY_AUDIT_R18.md`;
- `migration/DATABASE_DLL_DEPENDENCIES_R18.tsv`.

## 8. Billing/PC-room que afeta gameplay

O source prova que `Bl_RNo > 0`:

- adiciona +20 EXP solo;
- adiciona +20 EXP party;
- adiciona +50 ao acumulador de drop;
- ativa PCBang Pet.

Além disso:

- `DisuseDay`, `dwPeriodTime`, `dwPeriodTime2` podem fechar o cliente na expiração;
- `PCUse` e `PCSearchReserv` atualizam/consultam estado durante a sessão;
- `/time` usa `PCRNoReservProduct` quando `Bl_RNo>0`.

Logo, não apagar PC-room/billing zerando estado. A simplificação futura deve representar explicitamente o entitlement/bônus que quisermos preservar e separar isso do charging antigo.

`GpType/GpCode` no source observado alimentam principalmente monitorização PC-room por IP; o bônus direto comprovado usa `Bl_RNo`.

## 9. R18 — ClanMoney/SiegeMoney local

`Database.dll` também armazenava gameplay, não só billing.

R18 adiciona `TrySQLiteLocalClanFund` em `DllServer.cpp` e muda o evento `GAMELOG_SQL_CLANMONEY` para SQLite local-first.

Contrato da DLL preservado:

- lookup inicial `UserID + ClanZang`;
- amount=0 retorna saldo;
- operation 1 retira e loga `01/-amount`;
- operation 2 adiciona e loga `02/+amount`;
- sem sufficient-funds guard nessa camada;
- ClanMoney withdrawal não filtra `DelActive`;
- deposit e SiegeMoney filtram clan ativo;
- mutação por UserID, como no SQL histórico.

Fallback DLL só ocorre se a infraestrutura SQLite falhar; resultados de gameplay válidos são tratados localmente.

## 10. R18 — SiegeMoneyTax local

Reverse engineering do export `SiegeMoneyTax` provou o algoritmo:

1. Total==0 -> failure;
2. mark len <9 -> failure;
3. IDX = `atoi(last 6 digits of mark)`;
4. Tax==0 -> crédito zero;
5. caso contrário:
   `credit = Total - round(Total/(1 + Tax*0.01f))`;
6. `CL.SiegeMoney += credit WHERE IDX=?`;
7. log detalhado em `SiegeMoneyTax`;
8. log financeiro em `SiegeMoneyLog` com `Server`, `GS`, `02`.

R18 implementa `TrySQLiteLocalSiegeMoneyTax`, retira o hard gate `if(!BillSql.SiegeMoneyTax)` de `bSql_RecordCurrency`, e usa DLL só como fallback de infraestrutura.

As tabelas modernas têm `RegistDay NOT NULL`; como o insert histórico omitia a coluna, o adapter SQLite grava explicitamente `datetime('now','localtime')`.

## 11. Validações R18

PASS após as alterações:

- backup catalog 326/326;
- BillingDB 43 tables;
- SoD2 contract;
- migrations v29 / 332;
- query coverage missing=0;
- clan contract;
- transactions;
- clan marks/tags;
- local free login;
- notice;
- HTTP surface (`clanChipChange.asp` único HTTP ativo conhecido);
- Database.dll gameplay contract ClanMoney/SiegeMoney/SiegeMoneyTax;
- source graph;
- build scripts frozen;
- structure 1517/1517;
- compiler adaptation ledger.

Raw: `docs/R18_VALIDATION_RAW.txt`.

## 12. O que ainda não foi provado

Nenhum destes testes substitui:

- compile target Windows;
- link target Windows;
- startup real;
- login end-to-end;
- character create/load/save;
- inventory/warehouse;
- quests;
- coin shop;
- clan runtime;
- Bless Castle tax runtime;
- SoD2 runtime;
- restart persistence;
- WAL concurrent backend;
- billing/PC-room replacement;
- final removal ODBC/Database.dll/WinInet.

R6.1 continua oracle.

## 13. Próximos blockers por prioridade

### A. Session/billing separation

Não reconstruir charging inteiro por reflexo. Extrair responsabilidades:

- auth local;
- session/inuse cleanup (`SQLLogout`, `ServerDown`, `ServerExit`);
- explicit entitlement replacing the gameplay meaning of `Bl_RNo` if approved;
- remove accounting-only/provider machinery afterward.

### B. `clanChipChange.asp`

Ainda o único HTTP clan funcional conhecido. Precisamos dos códigos 1..6/100 e writes/logs exatos antes de migrar.

### C. Logs opcionais

`CharLog`, `CharRecordLog`, item/trade/post logs e `WithSOD2RecordRanks` não aparecem como gates de gameplay nos fluxos inspecionados. Decidir se são úteis no servidor simplificado; podem ser SQLite ou desabilitados depois de confirmação runtime.

### D. Dependencies

Quando zero responsabilidades necessárias dependerem delas:

1. retirar WinInet;
2. retirar Database.dll;
3. retirar ODBC;
4. link/test target após cada remoção.

## 14. Windows regression obrigatória

Executar pelo menos:

1. compile server;
2. link server;
3. DB limpa;
4. bootstrap migrations;
5. login free;
6. login dos tipos oficialmente retidos;
7. create/load/save char;
8. disconnect/reconnect;
9. inventory/warehouse;
10. quests;
11. coin shop;
12. clan completo;
13. ClanMoney/SiegeMoney;
14. Bless Castle tax;
15. SoD2;
16. restart/persistência;
17. WAL concorrente + SQLITE_BUSY;
18. comparar qualquer regressão com R6.1 antes de alterar gameplay.

## 15. Artefactos R18

- source completa R18;
- `migration/sqlite_wip_r17_to_r18.patch`;
- `migration/SQLITE_WIP_R18_CHANGED_FILES.tsv`;
- `migration/DATABASE_DLL_DEPENDENCIES_R18.tsv`;
- `docs/DATABASE_DLL_GAMEPLAY_AUDIT_R18.md`;
- `docs/SQLITE_WIP_R18_VALIDATION.md`;
- `docs/R18_VALIDATION_RAW.txt`;
- `validate_database_dll_gameplay_contract.py`;
- `CONHECIMENTO_CONSOLIDADO_CHAT_R18.md`;
- este `RETOMADA_COMPLETA_R18.md`.

## 16. Regras para próxima sessão

1. Ler este ficheiro antes de editar.
2. R6.1 continua baseline funcional.
3. R18 deve ser congelado depois de empacotado; próximo trabalho = R19.
4. Não importar dados históricos: fora de escopo atual.
5. Não apagar billing/PC-room antes de preservar/decidir os efeitos de gameplay acima.
6. Não mudar os bônus de `Bl_RNo` sem decisão explícita ou parity test.
7. Não aproximar `clanChipChange`.
8. Não adicionar nova TU servidor além da SQLiteConnection autorizada.
9. Repetir todos os validadores depois de cada delta.
10. Só depois de Windows runtime estável criar o guia final da DB e queries user/GM/admin.

## 17. Estado em uma linha

**R18 mantém o schema v29/332 e toda a cobertura R17, acrescenta uma auditoria concreta da Database.dll e retira dela para SQLite local-first três responsabilidades de gameplay comprovadas — ClanMoney, SiegeMoney e SiegeMoneyTax — enquanto preserva conscientemente os estados PC-room que ainda afetam EXP/drop/pet/expiração até que sejam simplificados de forma explícita.**
