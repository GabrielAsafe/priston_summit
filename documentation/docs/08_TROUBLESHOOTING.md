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

<!-- BEGIN AUTO:client-server-troubleshooting -->
## Cliente não conecta ao servidor

```bat
tasklist /FI "IMAGENAME eq Server.exe"
netstat -ano | findstr :9000
netsh advfirewall firewall show rule name=all | findstr /I 9000
```

```powershell
Test-NetConnection 127.0.0.1 -Port 9000
```

Verificar:

- IP e porta usados pelo cliente;
- retorno de `connect()` e `WSAGetLastError()`;
- execução de `accept()` no servidor;
- primeiro pacote enviado e recebido;
- opcode e tamanho;
- diferenças de `#pragma pack` e tamanhos de estruturas;
- encerramento prematuro do socket.
<!-- END AUTO:client-server-troubleshooting -->
