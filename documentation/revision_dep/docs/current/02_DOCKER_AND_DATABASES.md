# Summit Tale — Docker e Bases de Dados

## Serviços

- `summittale-mssql`: SQL Server 2022.
- `summittale-dbadmin`: CloudBeaver.
- `summit-web-bpt`: website BPT, porta 8088.
- `summit-web-epic`: website/painel EPIC, porta 8089.

## URLs

```text
CloudBeaver: http://localhost:8978
BPT:         http://localhost:8088/
EPIC admin:  http://localhost:8089/padm/
EPIC painel: http://localhost:8089/painel/
```

## SQL Server

```text
Host Docker: summittale-mssql
Host externo: localhost
Porta: 1433
User: sa
Password: SummitTale@12345
```

## CloudBeaver

```text
URL: http://localhost:8978
User: summittale
Password: Admin123
```

Dentro do CloudBeaver, usar `summittale-mssql`, não `localhost`, como host do SQL Server.

## Bases restauradas

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

## Organização recomendada

```text
docker/db/backups/*.bak
docker/db/sql/*.sql
docker/db/scripts/*.sh
```

## Gitignore recomendado

```gitignore
docker/db/backups/*.bak
docker/db/data/
*.mdf
*.ldf
```
