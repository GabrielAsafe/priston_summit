# Summit Tale — Schema das Bases

## Contagens

| Base | Tabelas |
|---|---:|
| AccountDB | 69 |
| BattlePass | 3 |
| BillingDb | 43 |
| BillingLogDb | 6 |
| ClanDB | 28 |
| CoinShopDB | 6 |
| GameLogDB | 17 |
| ItemLogDB | 3 |
| Jobs | 2 |
| LogadoPremiado | 1 |
| LogDB | 5 |
| PCRoom | 3 |
| PCRoomLog | 19 |
| PristonDB | 19 |
| QuestDB | 8 |
| rPTDB | 1 |
| SecurityDB | 6 |
| ServerDB | 6 |
| SoD2DB | 10 |
| SodDb | 69 |
| StartUserDB | 2 |

## AccountDB

Tabelas principais identificadas:

```text
AllGameUser
MK_User_DATA
GameMasters
```

`AllGameUser` contém colunas como:

```text
userid
Passwd
Grade
BlockChk
ECoin
LastDay
MXChar
MXLevel
```

`MK_User_DATA` contém:

```text
UserID
UserPWD
NickName
EmailAddr
Sex
DelChk
```

Estado conhecido: as tabelas de conta existem, mas estavam com 0 registros.

## PristonDB

Tabelas observadas:

```text
AgeFail
Beta
CheatLog
Coroa
Coroas
FunctionChecksumList
LiberarPais
ListaNegra
LoginAutenticado
LoginMacAdresss
LogsIpsOnline
LogsProtecao
NoticeSystem
PremiumData
PVP
Security
Tags
TopLevel
WindowCheatList
```

## ServerDB

Tabelas observadas:

```text
BoxRewards
DropLogAlert
NoticeSystem
PremiumData
PVP
Vip
```
