# SQLite WIP R3 — desenvolvimento e validação

Data: 2026-09-06

## Estado de referência

- **R6.1 continua sendo a única baseline funcional confirmada pelo utilizador.**
- Este R3 continua a partir do SQLite WIP R2 e ainda **não** foi confirmado por compile/link/runtime Windows.
- Não foram feitas alterações intencionais de gameplay, rendering, collision, network protocol ou formato de personagem.

## Alterações R2 -> R3

### 1. Contrato temporal SQL Server preservado

O R2 ainda traduzia o `GETDATE()` final para `CURRENT_TIMESTAMP`. Em SQLite, `CURRENT_TIMESTAMP` é UTC, enquanto SQL Server `GETDATE()` representa a hora local do servidor.

R3 normaliza o contrato para:

```sql
datetime('now','localtime')
```

Isto foi aplicado tanto no rewriter quanto nos callers SQLite já convertidos de quest/coin shop, mantendo `DAY/MONTH` e `DATEADD` no mesmo referencial local.

### 2. `recordAttack` — placeholders SQLite válidos

Foi corrigida a query:

```sql
VALUES ('?', '?', GETDATE(), '?')
```

para placeholders reais:

```sql
VALUES (?, ?, GETDATE(), ?)
```

Em SQLite, `'?`' é texto literal e não um parâmetro bindável. Manter a forma anterior poderia gravar `?` em vez dos valores do caller.

### 3. Roteamento de tabelas bare do singleton

`SqliteQueryCompat` ganhou mapeamento explícito para tabelas bare conhecidas de ServerDB (`BoxRewards`, `DropLogAlert`, `NoticeSystem`, `PremiumData`, `PVP`, `Vip`). Tabelas já qualificadas como `[JOBS].[dbo].[...]`, `[BattlePass].[dbo].[...]`, `[ClanDB].[dbo].[...]`, etc. continuam sendo tratadas mecanicamente pelo rewriter de nomes qualificados.

O fallback histórico para ServerDB foi mantido apenas para compatibilidade com queries singleton ainda não auditadas.

### 4. Migration 005 — `QuestDB__Progress`

O layout de cinco colunas ficou demonstrado pelos próprios callers:

1. `QuestID`
2. `QuestKind`
3. `QuestType`
4. `PlayerName`
5. `StartDate`

Os inserts posicionais fornecem exatamente cinco valores, e `SELECT *` lê 1/2/3 como QuestID/QuestKind/QuestType. Foram criados apenas índices não-únicos; nenhuma unicidade foi inventada.

### 5. Migration 006 — ServerDB ativo

Foram materializadas:

- `ServerDB__Vip`
- `ServerDB__BoxRewards`
- `ServerDB__DropLogAlert`

Para `BoxRewards`, R3 removeu a dependência de `SELECT *`: o caller agora pede explicitamente `ItemCode, Count, Class`, eliminando a necessidade de adivinhar uma coluna legacy não usada.

### 6. Migration 007 — SecurityDB ativo

Foram materializadas:

- `SecurityDB__AgingFailed`
- `SecurityDB__Security`
- `SecurityDB__ClassCheatList`

O writer de `AgingFailed` agora lista explicitamente `PlayerID, PlayerName, ItemName, ItemAging, Head, ChkSum, Date`. Isso evita depender da regra SQL Server que permite omitir uma possível coluna `IDENTITY` em `INSERT ... VALUES (...)` sem lista de colunas.

### 7. Migration 008 — `ServerDB__PremiumData`

O loader ativo deixou de usar `SELECT *` e passou a selecionar explicitamente:

```sql
SELECT TimeLeft, ItemType FROM PremiumData WHERE CharacterName=?
```

O schema SQLite pode portanto ser definido somente pelas três colunas realmente referenciadas pelo código:

- `CharacterName`
- `TimeLeft`
- `ItemType`

Sem inventar ID/IDENTITY ou ordem de colunas legacy não consumidas.

### 8. Bootstrap automático de migrations

`SQLiteConnection::Init()` agora aplica as migrations conhecidas `001` até `008` antes de expor a conexão.

Contrato do runner:

- lê `PRAGMA user_version`;
- recusa DB com versão maior que a suportada pelo binário;
- exige cada migration pendente em `Database/migrations/`;
- usa `BEGIN IMMEDIATE` por migration;
- aplica o SQL;
- grava o novo `PRAGMA user_version` dentro da mesma transação;
- `COMMIT` em sucesso;
- `ROLLBACK` em qualquer falha.

Uma DB limpa chega atualmente a **schema version 8**.

Importante: versão 8 significa apenas “migrations 001..008 aplicadas”. Não significa que todos os bancos/tabelas legacy já tenham sido reconstruídos.

### 9. Validador reproduzível

Foi adicionado:

```text
validate_sqlite_migrations.py
```

Ele usa apenas `sqlite3` da stdlib Python e uma DB temporária. Valida:

- sequência 001..008;
- `user_version=8`;
- 88 tabelas de utilizador;
- idempotência do seed Hopi;
- layout `Progress`;
- layout/projeção `BoxRewards`;
- layout/projeção `PremiumData`;
- insert explícito `AgingFailed`;
- UNIQUE de `Complete` para `INSERT OR REPLACE`;
- UNIQUE de `PVP.Name` para `INSERT OR IGNORE`.

## Validações executadas neste ambiente

- `validate_build_source_graph.py`: **PASS** — cliente 201/201; servidor 179/180; apenas `SQLiteConnection.cpp` como TU nova autorizada.
- `validate_build_scripts.py`: **PASS** — quatro scripts de build/link permanecem congelados.
- `validate_port_structure_presence.py`: **PASS** — 1517 ficheiros estruturais esperados, zero ausentes.
- `validate_compiler_adaptations.py`: **PASS** — continua existindo apenas a adaptação MinGW já autorizada.
- `validate_sqlite_migrations.py`: **PASS** — version 8, 88 tabelas, contratos/idempotência PASS.
- harness portátil do `SqliteQueryCompat.inl`: **14 casos de sucesso + 1 caso T-SQL de rejeição, 0 falhas**.
- bootstrap simulado com semântica equivalente ao runner:
  - DB limpa -> v8: PASS;
  - reabertura v8 -> no-op: PASS;
  - DB `user_version=99` -> rejeitada: PASS;
  - migration 008 deliberadamente quebrada -> rollback mantém v7 e não deixa `PremiumData` parcial: PASS.
- scan dos SQL ativos: zero ocorrências de `CURRENT_TIMESTAMP`, `ISNULL`, `IF EXISTS/NOT EXISTS`, `+=`, `DATEDIFF`, `CONVERT` ou placeholders `?` entre aspas em queries ativas.

## Armadilha descoberta e deliberadamente bloqueada

Durante o R3 foi inicialmente considerada uma migration adicional para `BannedIP`, `CheatLog`, `WindowCheatList` e `AgingRecord`. Ela foi **rejeitada antes do checkpoint**.

Motivo: SQL Server permite que `INSERT INTO T VALUES (...)` omita uma coluna `IDENTITY`, enquanto SQLite exige outro tratamento. Em `BannedIP`, por exemplo, o insert fornece IP/account/data, mas um `SELECT *` posterior lê a coluna 1 como inteiro. Isso é evidência forte de uma coluna ID/IDENTITY anterior ao IP e prova que criar uma tabela “óbvia” de três colunas seria perigoso.

Consequentemente continuam bloqueadas:

- `SecurityDB__BannedIP`
- `SecurityDB__BannedMac`
- `SecurityDB__CheatLog`
- `SecurityDB__FunctionChecksumList`
- `SecurityDB__WindowCheatList`
- `SecurityDB__AgingRecord`

até o layout poder ser provado ou os callers serem convertidos para projeções/writes nomeados sem mudar comportamento.

## Tabelas ainda bloqueadas prioritárias

- `QuestDB__Rewards`
- `QuestDB__ItemRewards`
- `ServerDB__NoticeSystem`
- `StartUserDB__Level`
- `StartUserDB__Items`
- `StartUserDB__ExtraItems`
- ClanDB restante
- BattlePass/JOBS/SoD/logging/billing/PCRoom/rPTDB e demais domínios históricos

`Rewards` e `ItemRewards` continuam especialmente sensíveis porque o código usa `SELECT *` + posições, mas os nomes/ordem completos das colunas não aparecem no material recebido.

## Limitação de build

Não foi possível executar o build/link Windows completo neste ambiente:

- `i686-w64-mingw32-g++` não está instalado;
- `modules/` / `modules/Dependency` não estão presentes na árvore recebida.

Portanto **R3 não é nova baseline funcional**. A promoção só deve ocorrer após compile + link + runtime pelo utilizador, seguindo a regressão contra R6.1.

## Ordem recomendada a seguir

1. Compilar/linkar R3 no ambiente Windows/MinGW do projeto.
2. Iniciar com `Database/migrations/` disponível no working directory e DB limpa.
3. Confirmar bootstrap até `PRAGMA user_version=8`.
4. Testar login, create/load/save character, inventory/warehouse, quests, coin shop, VIP/PVP/restoration e restart persistence.
5. Reconstruir os contratos bloqueados por remoção controlada de `SELECT *`/writes posicionais quando os nomes usados pelo código forem demonstráveis.
6. Só remover ODBC do linker quando todos os callers ativos estiverem migrados e o runtime tiver sido validado.
