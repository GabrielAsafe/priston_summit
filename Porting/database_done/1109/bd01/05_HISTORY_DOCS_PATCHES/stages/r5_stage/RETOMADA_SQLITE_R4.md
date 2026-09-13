# RETOMADA — Summit SQLite WIP R3

Data: 2026-09-06

## Regra principal

**R6.1 continua sendo a última baseline funcional comprovada.** O diretório `current/SourceCode_SQLite_WIP` deste pacote é R3 experimental e ainda precisa de compile/link/runtime pelo utilizador antes de qualquer promoção.

Não alterar gameplay/refactor enquanto a migração de persistência estiver em curso. `pre_refactor` continua sendo a verdade funcional e R6.1 a regressão prática.

## Arquitetura mantida

- um único ficheiro físico: `Database/summit.sqlite3`;
- DBs legacy continuam como contextos lógicos e viram prefixos físicos `<DB>__<Table>`;
- servidor usa SQLite diretamente;
- backend futuro pode usar o mesmo SQLite com coordenação apropriada;
- cliente nunca abre SQLite;
- interfaces web legacy não devem continuar como camada de persistência;
- as APIs públicas `SQLConnection` e singleton `SQL` continuam preservadas durante a migração.

## Estado R3

R3 parte do WIP R2 e adiciona/corrige:

- `GETDATE()` preservado como hora local via `datetime('now','localtime')`;
- placeholders inválidos de `recordAttack` corrigidos;
- roteamento bare explícito de tabelas ServerDB conhecidas;
- migrations `005` Progress, `006` ServerDB ativo, `007` SecurityDB ativo e `008` PremiumData;
- `BoxRewards` e `PremiumData` deixam de depender de `SELECT *` ambíguo;
- `AgingFailed` deixa de depender de insert posicional;
- bootstrap automático transacional com `PRAGMA user_version`;
- validador reproduzível `validate_sqlite_migrations.py`.

Migrations atuais: **001..008**. DB limpa validada: **schema version 8 / 88 tabelas de utilizador**.

## Cuidados críticos

1. Não assumir que schema version 8 significa migração completa.
2. Não inventar colunas para fazer query preparar.
3. SQL Server `IDENTITY` pode ser omitido por `INSERT ... VALUES (...)`; isso torna vários layouts aparentemente óbvios ambíguos.
4. Para `SELECT *`, a ordem de colunas continua sendo contrato até o caller ser convertido de forma comprovadamente equivalente para projeção explícita.
5. Não criar `INSERT OR IGNORE/REPLACE` sem UNIQUE/PK equivalente ao comportamento antigo.
6. Não remover ODBC/WinInet dos scripts congelados antes da prova runtime correspondente.
7. `Database/migrations/` deve estar no mesmo working-root usado para `Database/summit.sqlite3`; migrations pendentes ausentes fazem o startup falhar de forma explícita.

## Bloqueadores principais

Ainda não materializar sem evidência adicional:

- QuestDB: `Rewards`, `ItemRewards`;
- ServerDB: `NoticeSystem` e demais contratos não provados;
- SecurityDB: `BannedIP`, `BannedMac`, `CheatLog`, `FunctionChecksumList`, `WindowCheatList`, `AgingRecord`;
- StartUserDB: `Level`, `Items`, `ExtraItems` até fechar o contrato/seed;
- ClanDB/BattlePass/JOBS/SoD/logs/billing/PCRoom/rPTDB restantes.

## Validação R3 já feita

- source graph: PASS;
- build scripts frozen: PASS;
- port structure: PASS;
- compiler adaptation ledger: PASS;
- migrations 001..008: PASS;
- schema v8 / 88 tabelas: PASS;
- idempotência/constraints representativas: PASS;
- migration rollback: PASS;
- query rewriter: 14 casos válidos + 1 rejeição esperada, 0 falhas;
- scan T-SQL conhecido/quoted placeholders: limpo.

Não houve build/link Windows neste ambiente por ausência de `i686-w64-mingw32-g++` e de `modules/Dependency`.

Detalhes completos: `docs/SQLITE_WIP_R3_VALIDATION.md`.

---

# Addendum — SQLite WIP R4 (2026-09-06)

R4 extends the migration through `PRAGMA user_version = 13` and 106 user tables. The supplied SQL Server backups were parsed as direct schema evidence (326 tables / 5,568 columns). Migrations 009–013 add the previously blocked Quest rewards, NoticeSystem, selected SecurityDB/StartUserDB tables, and active tables from ClanDB/JOBS/BattlePass/PristonDB/SoD2DB/LogadoPremiado.

Caller fixes remove disproved positional assumptions in VIP, StartUser items and CheatLog. R4 passes the source-graph, frozen-build-script, structure, compiler-adaptation and SQLite migration validators. It remains experimental pending Windows compile/link/runtime testing; R6.1 remains the functional oracle.

Do not invent `BannedIP`, `BannedMac`, `AgingRecord`, `ExtraItems`, `CoinShopDB.Usuario`, or `PAINEL_NET.Usuario` without additional evidence.
