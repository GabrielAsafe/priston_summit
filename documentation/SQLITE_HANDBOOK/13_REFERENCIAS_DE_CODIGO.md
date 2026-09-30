# 13. Referências de código — onde mexer

Mapa rápido entre assunto e source.

## Backend SQLite

```text
SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp
SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.h
SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.inl
SrcServer/src/systems/persistence/database/sqlite/SqliteQueryCompat.h
```

Responsabilidades:

- abrir `summit.sqlite3`;
- carregar `sqlite3.dll`;
- WAL/pragmas;
- aplicar migrations;
- preparar/bind/execute;
- traduzir SQL legado.

## Migrations

```text
Database/migrations/
```

Índice:

```text
Database/migrations/README.md
```

## Admin Python

```text
tools/sqlite_local_admin.py
```

Usos:

- criar conta;
- senha;
- auth ownership;
- GM;
- entitlement;
- status;
- audit local-only;
- backup;
- prepare local-only.

## Auth / session / logout

Partes relevantes:

```text
Shared/server/legacy_runtime/OnSever_legacy_server.inl
```

Também procurar por:

```text
TrySQLiteLocalAuth
TrySQLiteLocalLogout
bSql_Logout
BILLING_SQL_LOGOUT
```

## GameMasters

```text
SrcServer/src/systems/admin/game_masters/GameMasters.cpp
SrcServer/src/systems/admin/game_masters/GameMasters.h
```

## QuestDB

```text
SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp
SrcServer/src/systems/gameplay/quests/server/cQuestHandler.h
```

SQL importante:

```text
Progress
MonsterKilled
Complete
SingleQuest
DailyQuest
RepeatedQuest
QuestMonsters
Rewards
ItemRewards
```

## CoinShop

```text
SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp
SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp
```

## Character persistence

Server:

```text
Shared/persistence/character_record/record_legacy_server.inl
SrcServer/src/systems/persistence/character_record/record.cpp
```

Client:

```text
Shared/persistence/character_record/record_legacy_client.inl
SrcGame/src/systems/persistence/character_record/record.cpp
```

Networking/save trigger:

```text
SrcGame/src/systems/network/client/netplay.cpp
```

Pontos importantes:

```text
InsertCharData
SendUserDataToClient
RecvRecordDataFromClient
MakeRecordData
SendRecordDataToServer
SaveGameData
OPCODE_RECORDDATA
```

## Build/deploy

```text
server.sh
cliente.sh
send_to_s.sh
build_server_mingw_i686_j8.sh
link_server_mingw_i686.sh
build_client_mingw_i686_multicore.sh
link_client_mingw_i686.sh
```

Fluxo normal:

```text
server.sh
cliente.sh
```

## Runtime

```text
../Servidor_R25_Wine/
```

DB:

```text
../Servidor_R25_Wine/Database/summit.sqlite3
```

Personagens:

```text
../Servidor_R25_Wine/DataServer/
```

Logs:

```text
../Servidor_R25_Wine/LOGS/
```

## Validações R25 úteis

No source existem validators como:

```text
validate_sqlite_migrations.py
validate_sqlite_local_admin.py
validate_sqlite_local_session.py
validate_sqlite_local_login.py
validate_sqlite_backup_cutover.py
validate_sqlite_query_coverage.py
validate_game_masters_sqlite.py
validate_backup_catalog_completion.py
```

Quando uma migration nova for criada, atualizar os validators relevantes em vez de confiar apenas no runtime manual.
