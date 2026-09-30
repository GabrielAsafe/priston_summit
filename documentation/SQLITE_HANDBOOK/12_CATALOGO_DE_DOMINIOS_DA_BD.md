# 12. Catálogo de domínios da base

Este ficheiro é um mapa de alto nível. Não substitui `sqlite_master`, migrations ou `PRAGMA table_info`.

## AccountDB

Responsabilidades:

- contas;
- dados pessoais legados;
- estado de login;
- flags/block;
- GameMasters.

Principais tabelas:

```text
AccountDB__AGameUser ... AccountDB__ZGameUser
AccountDB__AllGameUser
AccountDB__APersonalMember ... ZPersonalMember
AccountDB__ALLPersonalMember
AccountDB__GameMasters
AccountDB__BlockUserList
```

## LocalAuth

Extensão local R23+:

```text
LocalAuth__AccountPolicy
LocalAuth__RuntimePolicy
LocalAuth__GameplayEntitlement
```

Não veio do SQL Server.

## CoinShopDB

```text
CoinShopDB__ItemList
CoinShopDB__ItemList_T
CoinShopDB__UserInfo
CoinShopDB__UserInfo_T
CoinShopDB__PendingPayments
CoinShopDB__Transactions
```

## QuestDB

```text
QuestDB__RepeatedQuest
QuestDB__DailyQuest
QuestDB__SingleQuest
QuestDB__QuestMonsters
QuestDB__MonsterKilled
QuestDB__Progress
QuestDB__Complete
QuestDB__Rewards
QuestDB__ItemRewards
```

## ServerDB

Conteúdo server/gameplay:

```text
ServerDB__PVP
ServerDB__Vip
ServerDB__PremiumData
ServerDB__NoticeSystem
ServerDB__DropLogAlert
ServerDB__BoxRewards
```

## SecurityDB

Contratos de segurança e catálogos legados:

```text
SecurityDB__WindowCheatList
SecurityDB__AgingFailed
SecurityDB__AgingRecord
SecurityDB__BannedIP
SecurityDB__BannedMac
SecurityDB__ClassCheatList
...
```

## ClanDB

Clãs, notices, money e siege.

Exemplos:

```text
ClanDB__ClanMainView
ClanDB__ClanSubView
ClanDB__ClanMoneyLog
ClanDB__SiegeMoneyLog
ClanDB__SiegeMoneyTax
ClanDB__SiegeClanList
ClanDB__SiegeCurrentList
```

## GameLogDB / ItemLogDB / LogDB

Logs e histórico legado.

R22 também adicionou sinks locais append-only, entre eles:

```text
GameLogDB__LocalCharacterEvent
ItemLogDB__LocalItemEvent
ItemLogDB__LocalItemEventEntry
```

## BattlePass

Dados importados:

```text
BattlePass__BP_Info
BattlePass__BP_Item
BattlePass__BP_XP
```

## StartUserDB

Dados de configuração inicial:

```text
StartUserDB__Level
StartUserDB__Items
StartUserDB__ExtraItems
```

## BillingDB / BillingLogDB

Catálogo legado reconstruído.

Grande parte está vazia no dump atual e não deve ser confundida com a autoridade local de auth R23.

## PCRoom / PCRoomLog

Catálogo histórico de PC room.

Gameplay entitlement novo não depende diretamente de `RNo`; usa `LocalAuth__GameplayEntitlement`.

## SodDB / SoD2DB

Tabelas históricas de SOD/records.

Muitas estavam vazias no dump.

## PristonDB

Vários catálogos auxiliares importados.

## ADM

O dump Docker continha:

```text
ADM.dbo.loginGM
```

com dados.

Foi materializado como:

```text
ADM__loginGM
```

no runtime atual.

Ainda não existe logical database `DATABASEID_ADM` no backend.

## DataServer: fora da SQLite

Não esquecer:

```text
DataServer/userinfo
DataServer/userdata
DataServer/warehouse
DataServer/PostBox
```

fazem parte do estado persistente do servidor, embora não estejam no `summit.sqlite3`.
