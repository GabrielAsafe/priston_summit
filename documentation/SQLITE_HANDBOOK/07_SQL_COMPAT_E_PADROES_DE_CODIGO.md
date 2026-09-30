# 7. SQL compatibility e padrões de código

## 7.1 O compat layer

Arquivo central:

```text
SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.inl
```

Ele traduz parte do SQL Server legado para SQLite.

## 7.2 Traduções automáticas suportadas

### Database/schema

```sql
[QuestDB].[dbo].[Progress]
```

vira:

```text
QuestDB__Progress
```

### GETDATE

```sql
GETDATE()
```

vira:

```sql
datetime('now','localtime')
```

### DATEADD

Forma suportada:

```sql
DATEADD(DAY, literal, GETDATE())
```

vira modifier de `datetime`.

Não é um parser SQL geral.

### DAY/MONTH

Queries antigas de quest têm suporte específico.

### Collations conhecidas

Algumas collations SQL Server são normalizadas para:

```sql
COLLATE NOCASE
```

### SELECT TOP N

O `TOP` externo simples é convertido para `LIMIT`.

## 7.3 Construções deliberadamente rejeitadas

O compat layer rejeita control-flow T-SQL:

```sql
IF NOT EXISTS(...)
IF EXISTS(...)
BEGIN
ELSE
```

Essas queries devem ser migradas no caller.

Motivo:

- bind numbering;
- semântica de rowcount;
- atomicidade;
- comportamento de write;
- legibilidade.

## 7.4 Caso real: `/skiptier`

A versão problemática usava algo equivalente a:

```sql
IF NOT EXISTS(...)
BEGIN
    INSERT ...
END
ELSE
BEGIN
    UPDATE ...
END
```

Isso falhava em `Prepare()` no backend SQLite.

Forma correta:

```sql
INSERT OR REPLACE INTO Complete (...)
VALUES (...);
```

desde que exista a UNIQUE correta.

## 7.5 ISNULL vs COALESCE

SQL Server:

```sql
ISNULL(expr, 0)
```

SQLite não usa `ISNULL()` como função equivalente.

Preferir:

```sql
COALESCE(expr, 0)
```

O `/skiptier` também precisou dessa correção.

## 7.6 Código novo deve ser SQLite-native

Não depender do compat layer para código novo quando não é necessário.

Preferir diretamente:

```sql
datetime('now','localtime')
COALESCE(...)
INSERT ... ON CONFLICT ...
INSERT OR IGNORE ...
```

mas somente com constraints adequadas.

## 7.7 Sempre nomear colunas em INSERT novo

Evitar:

```sql
INSERT INTO Table VALUES (...)
```

Preferir:

```sql
INSERT INTO Table (A,B,C)
VALUES (?,?,?);
```

Isso reduz quebra quando o schema evolui.

Há inserts posicionais legados que não podem ser removidos sem revisão, mas não criar novos.

## 7.8 Bind parameters

Usar parâmetros em vez de concatenar input de jogador.

Bom:

```sql
SELECT * FROM UserInfo WHERE UserID=?
```

Ruim:

```cpp
sprintf("... WHERE UserID='%s'", user);
```

Ainda existem callers antigos por string, mas código novo deve evitar.

## 7.9 Transações

Para mudança econômica ou estado composto:

```text
BEGIN IMMEDIATE
write A
write B
write C
COMMIT
```

Se qualquer etapa falhar:

```text
ROLLBACK
```

Não usar sequência de writes independentes quando a regra exige atomicidade.

## 7.10 Erros

Em diagnóstico C++ usar:

```cpp
db->GetLastError()
```

e registrar:

- query stage;
- logical database;
- entity ID;
- operação;
- erro SQLite.

Não logar senha.

## 7.11 Bare table mapping

O singleton `SQL` não carrega sempre `EDatabaseID`, então o compat layer só infere uma lista conhecida de tabelas.

Tabela nova desconhecida usada via singleton pode falhar com:

```text
tabela bare sem mapeamento de database
```

Para feature nova:

- preferir `SQLConnection::GetConnection(DATABASEID_...)`; ou
- usar referência qualificada que o compat layer consiga resolver; ou
- adicionar mapping explícito com teste.
