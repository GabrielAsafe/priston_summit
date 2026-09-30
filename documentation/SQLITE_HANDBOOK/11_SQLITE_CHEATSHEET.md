# 11. SQLite cheatsheet

Use preferencialmente com servidor parado para inspeções que possam evoluir para manutenção.

## Saúde

```sql
PRAGMA user_version;
PRAGMA quick_check;
PRAGMA foreign_key_check;
PRAGMA journal_mode;
```

## Tabelas

```sql
SELECT name
FROM sqlite_master
WHERE type='table'
  AND name NOT LIKE 'sqlite_%'
ORDER BY name;
```

## Schema de uma tabela

```sql
PRAGMA table_info("QuestDB__Progress");
PRAGMA index_list("QuestDB__Progress");
```

## Conta

Para conta que começa por T:

```sql
SELECT *
FROM AccountDB__TGameUser
WHERE lower(userid)=lower('teste');
```

## Auth local

```sql
SELECT *
FROM LocalAuth__AccountPolicy
ORDER BY UserID;

SELECT *
FROM LocalAuth__RuntimePolicy;

SELECT *
FROM LocalAuth__GameplayEntitlement
ORDER BY UserID;
```

## GMs

```sql
SELECT id,accountId,security,active
FROM AccountDB__GameMasters
ORDER BY id;
```

## CoinShop

```sql
SELECT COUNT(*) FROM CoinShopDB__ItemList;
SELECT * FROM CoinShopDB__ItemList ORDER BY ItemSection,szItemCode;

SELECT *
FROM CoinShopDB__UserInfo
WHERE lower(UserID)=lower('teste');
```

## TimeShop

```sql
SELECT COUNT(*) FROM CoinShopDB__ItemList_T;

SELECT *
FROM CoinShopDB__UserInfo_T
WHERE lower(UserID)=lower('teste');
```

## Quest definitions

```sql
SELECT *
FROM QuestDB__SingleQuest
ORDER BY QuestID,QuestType;

SELECT *
FROM QuestDB__DailyQuest
ORDER BY QuestID,QuestType;

SELECT *
FROM QuestDB__RepeatedQuest
ORDER BY QuestID,QuestType;
```

## Uma quest específica

```sql
SELECT * FROM QuestDB__SingleQuest
WHERE QuestID=10010;

SELECT * FROM QuestDB__QuestMonsters
WHERE QuestID=10010;

SELECT * FROM QuestDB__Rewards
WHERE QuestID=10010;

SELECT * FROM QuestDB__ItemRewards
WHERE QuestID=10010;
```

## Estado de quest por personagem

```sql
SELECT * FROM QuestDB__Progress
WHERE lower(PlayerName)=lower('pike');

SELECT * FROM QuestDB__MonsterKilled
WHERE lower(PlayerName)=lower('pike');

SELECT * FROM QuestDB__Complete
WHERE lower(PlayerName)=lower('pike')
ORDER BY EndDate DESC;
```

## Ver Tier test 10010–10013

```sql
SELECT *
FROM QuestDB__Complete
WHERE lower(PlayerName)=lower('pike')
  AND QuestID IN (10010,10011,10012,10013)
ORDER BY QuestID;
```

## Contagens críticas

```sql
SELECT 'CoinShop items', COUNT(*) FROM CoinShopDB__ItemList
UNION ALL
SELECT 'TimeShop items', COUNT(*) FROM CoinShopDB__ItemList_T
UNION ALL
SELECT 'Quest complete', COUNT(*) FROM QuestDB__Complete
UNION ALL
SELECT 'Quest progress', COUNT(*) FROM QuestDB__Progress
UNION ALL
SELECT 'Quest single', COUNT(*) FROM QuestDB__SingleQuest;
```

## Detectar sessões presas

Executar para as tabelas relevantes:

```sql
SELECT userid,inuse,ServerName,SNo,StartDay,LastDay
FROM AccountDB__TGameUser
WHERE COALESCE(CAST(inuse AS TEXT),'0') NOT IN ('','0');
```

## Ver NULLs históricos importantes

```sql
SELECT COUNT(*)
FROM ClanDB__SiegeMoneyLog
WHERE RegistDay IS NULL;

SELECT COUNT(*)
FROM ClanDB__SiegeMoneyTax
WHERE RegistDay IS NULL;

SELECT COUNT(*)
FROM CoinShopDB__PendingPayments
WHERE CompletedDate IS NULL;

SELECT COUNT(*)
FROM GameLogDB__CharGameRecordLg
WHERE RegistDay IS NULL
   OR DisuseDay IS NULL
   OR EditDay IS NULL
   OR Active IS NULL;
```

## Não fazer diretamente sem backup

Evitar manutenção casual com:

```sql
DELETE ...
UPDATE saldo ...
UPDATE inuse ...
DROP TABLE ...
ALTER schema ...
```

Sempre entender a regra de negócio e manter uma migration/recovery path.
