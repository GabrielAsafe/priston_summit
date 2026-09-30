# 6. Migrations, schema e como adicionar extensões

## 6.1 Regra fundamental

Uma alteração estrutural permanente deve ser uma **migration nova**.

Não editar uma migration antiga já aplicada em instalações reais.

## 6.2 Como o servidor aplica migrations

Lista atual em `SQLiteConnection.cpp`:

```text
001_accountdb.sql
...
032_local_auth_policy.sql
```

O último número define a versão suportada:

```text
latestVersion = 32
```

Para adicionar v33:

1. criar `Database/migrations/033_....sql`;
2. adicionar `{33, "033_....sql"}` à lista em `SQLiteConnection.cpp`;
3. atualizar validators;
4. testar bootstrap vazio;
5. testar upgrade de uma DB v32;
6. testar runtime Wine.

## 6.3 Não reordenar colunas em tabelas legadas

Existem muitos callers com:

```sql
SELECT *
```

e leitura posicional:

```cpp
GetData(... column 1 ...)
GetData(... column 2 ...)
```

A ordem física é parte da ABI de persistência.

Para adicionar uma coluna numa tabela usada por `SELECT *`:

- preferir adicionar no final;
- revisar todos os positional readers;
- revisar `INSERT ... VALUES(...)` sem lista de colunas.

## 6.4 Tipos SQLite

Conversão geral usada no projeto:

```text
varchar/nvarchar/text -> TEXT
int/bigint/bit        -> INTEGER
datetime              -> TEXT
money/decimal         -> NUMERIC
IDENTITY int          -> INTEGER PRIMARY KEY AUTOINCREMENT
```

Não assumir que affinity substitui regras de aplicação.

## 6.5 UNIQUE antes de UPSERT/REPLACE

Antes de usar:

```sql
INSERT OR IGNORE
INSERT OR REPLACE
ON CONFLICT(...)
```

deve existir uma chave que represente a identidade lógica.

Exemplo correto:

```text
QuestDB__Complete
UNIQUE(QuestID, QuestKind, QuestType, PlayerName)
```

Outro:

```text
CoinShopDB__UserInfo
UNIQUE(UserID)
```

## 6.6 NULLability: regra após a importação

A política adotada é:

> Se o SQL Server histórico contém NULL, o SQLite deve preservar NULL.

Não converter dados históricos para:

```text
''
0
data atual
```

apenas para satisfazer uma constraint reconstruída de forma demasiado restritiva.

Ao mesmo tempo, **novos inserts devem continuar corretos pela regra de negócio**.

Exemplo:

- `SiegeMoneyTax.RegistDay` histórico pode ser `NULL`;
- insert novo continua passando `datetime('now','localtime')`.

## 6.7 Divergência atual que precisa virar migration v33

O runtime validado relaxou NULLability em:

```text
ClanDB__SiegeMoneyLog.RegistDay
ClanDB__SiegeMoneyTax.RegistDay

CoinShopDB__PendingPayments.CompletedDate

GameLogDB__CharGameRecordLg.RegistDay
GameLogDB__CharGameRecordLg.DisuseDay
GameLogDB__CharGameRecordLg.EditDay
GameLogDB__CharGameRecordLg.Active
```

Também foi criada:

```text
ADM__loginGM
```

Porém:

```text
PRAGMA user_version = 32
```

continua igual.

Isso significa que hoje há uma diferença entre:

- schema que um bootstrap limpo R25 cria;
- schema real validado pós-importação.

## 6.8 Proposta para migration 033

Objetivo:

```text
033_legacy_data_fidelity.sql
```

Ela deve:

1. reconstruir as tabelas acima mantendo ordem de colunas;
2. tornar somente as colunas provadamente nullable em `NULL`;
3. copiar dados sem transformação;
4. recriar índices/PK/UNIQUE;
5. criar `ADM__loginGM` usando **o layout real do `columns.tsv` do export**, não uma definição inventada;
6. elevar `user_version` para 33 via migration engine.

### Importante

Não copiar cegamente o CREATE TABLE que foi usado no script de staging sem primeiro comparar:

```sql
PRAGMA table_info(...)
PRAGMA index_list(...)
```

e os metadados exportados do SQL Server.

## 6.9 Como criar uma tabela nova de feature

Se for uma tabela server-owned nova, usar namespace claro.

Exemplo:

```text
LocalFeature__Something
```

ou um namespace lógico existente se a semântica pertencer realmente a ele.

Migration:

```sql
CREATE TABLE ...
CREATE INDEX ...
```

Depois:

- adicionar testes;
- evitar `SELECT *` em código novo;
- sempre nomear colunas em INSERT;
- definir PK/UNIQUE conscientemente.

## 6.10 Como adicionar coluna nova

Perguntas obrigatórias:

1. há `SELECT *`?
2. há `INSERT INTO Table VALUES(...)`?
3. há positional `GetData()`?
4. a coluna pode ser `NULL` em dados históricos?
5. precisa de default?
6. precisa de index?
7. como fazer downgrade/rollback via backup?

## 6.11 Teste de migration

Mínimo:

### Clean bootstrap

Criar DB vazia e iniciar migration 001..N.

Verificar:

```text
user_version=N
quick_check=ok
table count esperado
```

### Upgrade

Cópia de DB real N-1 -> N.

### Idempotência operacional

Reiniciar servidor já em N e confirmar que nenhuma migration reaplica.

### Runtime

Login, personagem, quests, shop e GM smoke.
