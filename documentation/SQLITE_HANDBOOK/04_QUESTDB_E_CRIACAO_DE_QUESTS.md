# 4. QuestDB e criação de quests

## 4.1 Tabelas principais

### Definições

```text
QuestDB__RepeatedQuest
QuestDB__DailyQuest
QuestDB__SingleQuest
```

Layout comum:

```text
QuestID
QuestName
QuestTitle
LevelMin
LevelMax
PartyFlag
QuestDesc
QuestType
```

### Objetivos

```text
QuestDB__QuestMonsters
```

Colunas:

```text
QuestID
QuestKind
MonsterName
Quantity
QuestType
QuestMap
```

### Estado do jogador

```text
QuestDB__Progress
QuestDB__MonsterKilled
QuestDB__Complete
```

### Recompensas

```text
QuestDB__Rewards
QuestDB__ItemRewards
```

## 4.2 QuestKind usado no código

O código distingue categorias por `QuestKind`.

Os caminhos importantes observados incluem:

```text
RepeatedQuest -> kind usado pelo fluxo repetível
DailyQuest    -> kind usado pelo fluxo diário
SingleQuest   -> kind 9 no fluxo de single/tier
```

Para novas quests, não escolher `QuestKind` “por intuição”. Copiar o padrão da categoria existente no `cQuestHandler.cpp`.

## 4.3 Progress

Schema R25:

```text
QuestID
QuestKind
QuestType
PlayerName
StartDate
```

O SQL Server exportado tinha a coluna histórica chamada:

```text
Started
```

Durante a importação ela foi mapeada para:

```text
StartDate
```

Esse é um exemplo importante: nomes de coluna do legado e do schema SQLite podem divergir mesmo com a mesma semântica.

## 4.4 Complete

Schema:

```text
QuestID
QuestKind
QuestType
PlayerName
EndDate
```

Existe UNIQUE:

```text
(QuestID, QuestKind, QuestType, PlayerName)
```

Isso permite usar:

```sql
INSERT OR REPLACE
```

de forma determinística.

Sem essa chave, `REPLACE` não teria a semântica esperada.

## 4.5 MonsterKilled

Layout:

```text
MonsterName
Killed
Count
QuestID
QuestKind
QuestType
PlayerName
PartyFlag
QuestMap
```

A ordem é importante porque ainda existem inserts posicionais:

```sql
INSERT INTO MonsterKilled VALUES (...)
```

Não reordenar colunas.

## 4.6 Rewards

`QuestDB__Rewards`:

```text
QuestID
QuestKind
Gold
Exp
QuestType
Coin
TCoin
```

`Coin` e `TCoin` conectam quests à economia CoinShop/TimeShop.

`QuestDB__ItemRewards`:

```text
QuestID
QuestKind
ItemCode
ItemCount
QuestType NULL
```

`QuestType` é deliberadamente nullable porque há dados históricos NULL.

## 4.7 Como adicionar uma SingleQuest nova

Exemplo conceitual — não copiar IDs cegamente.

### 1. Escolher identidade

Definir:

```text
QuestID
QuestType
QuestKind
faixa de level
```

Verificar primeiro:

```sql
SELECT *
FROM QuestDB__SingleQuest
WHERE QuestID = ?;
```

### 2. Inserir definição

```sql
INSERT INTO QuestDB__SingleQuest
(QuestID, QuestName, QuestTitle, LevelMin, LevelMax, PartyFlag, QuestDesc, QuestType)
VALUES (...);
```

### 3. Definir objetivos

Uma row por objetivo/monster:

```sql
INSERT INTO QuestDB__QuestMonsters
(QuestID, QuestKind, MonsterName, Quantity, QuestType, QuestMap)
VALUES (...);
```

### 4. Definir reward base

```sql
INSERT INTO QuestDB__Rewards
(QuestID, QuestKind, Gold, Exp, QuestType, Coin, TCoin)
VALUES (...);
```

### 5. Item rewards opcionais

```sql
INSERT INTO QuestDB__ItemRewards
(QuestID, QuestKind, ItemCode, ItemCount, QuestType)
VALUES (...);
```

### 6. Testar aceite

Ao aceitar a quest, verificar:

```text
QuestDB__Progress
QuestDB__MonsterKilled
```

### 7. Testar conclusão

Após concluir:

```text
Progress: removido
MonsterKilled: removido
Complete: row criada/atualizada
rewards aplicados
```

## 4.8 Não adicionar dados de quest direto no runtime como prática permanente

Para conteúdo novo permanente, prefira:

- migration seed versionada; ou
- loader/data file versionado, se o projeto evoluir para conteúdo data-driven externo.

Evitar “editar DB manualmente em produção” porque um bootstrap novo não reproduzirá a quest.

## 4.9 Caso `/skiptier`

O comando administrativo conclui:

```text
10010
10011
10012
10013
```

A ausência dessas IDs em `SingleQuest` **não deveria impedir o bypass**.

A função foi desenhada para:

```sql
SELECT COALESCE(MAX(QuestType),0), COUNT(*)
FROM SingleQuest
WHERE QuestID=?;
```

Se não houver definição:

```text
QuestType = 0
```

Depois:

```sql
DELETE FROM MonsterKilled WHERE QuestID=? AND PlayerName=?;
DELETE FROM Progress      WHERE QuestID=? AND PlayerName=?;

INSERT OR REPLACE INTO Complete
(QuestID, QuestKind, QuestType, PlayerName, EndDate)
VALUES (..., datetime('now','localtime'));
```

### Bug encontrado

O source live tinha regredido para T-SQL:

```sql
ISNULL(...)
IF NOT EXISTS(...) BEGIN ... END ELSE BEGIN ... END
GETDATE()
```

A camada SQLite rejeita `IF/BEGIN/ELSE`.

A patch de diagnóstico restaurou SQL compatível:

- `ISNULL` -> `COALESCE`;
- control-flow T-SQL -> `INSERT OR REPLACE`;
- data -> `datetime('now','localtime')`;
- adicionou logging por etapa.

Depois disso o `/skiptier` passou.

### Importante para cleanup

Se remover o log `R25_SKIPTIER_DIAG_V1`, **não restaurar a implementação antiga**.

Manter a versão funcional do SQL.

## 4.10 Bugs comuns em quests

### Quest aparece mas não inicia

Verificar:

- `LevelMin/LevelMax`;
- `QuestKind`;
- `QuestType`;
- query `EXISTS` usada no aceite;
- `Progress` já existente.

### Quest inicia mas kills não contam

Verificar:

- `MonsterName` exato;
- `QuestMap`;
- `PartyFlag`;
- QuestID/QuestType;
- filtros especiais no handler.

### Quest completa mas reward não vem

Verificar:

- row em `Rewards`;
- row em `ItemRewards`;
- `QuestKind`/`QuestType` iguais à definição;
- código do item válido;
- Coin/TCoin.

### Quest fica presa

Inspecionar:

```text
Progress
MonsterKilled
Complete
```

Não apagar rows sem entender qual transição falhou.
