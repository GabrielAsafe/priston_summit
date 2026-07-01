# Summit Tale — Troubleshooting

## CloudBeaver: connection refused usando localhost

Usar `summittale-mssql`, não `localhost`, quando a conexão parte do CloudBeaver.

## isql não aceita GO

No `isql`, executar comandos sem `GO`.

## `select * from AccountDB` falha

`AccountDB` é database, não tabela. Usar:

```sql
USE AccountDB;
SELECT name FROM sys.tables;
```

## Restore falha por database em uso

Parar websites:

```bash
docker stop summit-web-epic summit-web-bpt
```

## Servidor Windows não conecta

Verificar ODBC, Registry, host SQL, porta 1433 e firewall.
