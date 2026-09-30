# Summit Tale — Servidor Windows

## Diretório

```text
Servidor/
```

## Executável principal

```text
Servidor/Server.exe
```

## Configurações relevantes

```text
Servidor/sql.reg
Servidor/server.txt
Servidor/SQL.txt
Servidor/PriTaTor.Config.txt
Servidor/Configs/
```

## Erro atual observado

`Servidor/server.txt`:

```text
23:24:52 - Falha ao conectar-se ao banco de dados! Log: SQL.txt
```

`Servidor/SQL.txt`:

```text
23:24:52 - SQLAllocHandle failed. Error 4
```

## Possíveis causas

- Driver ODBC SQL Server ausente.
- Driver ODBC 32-bit ausente, se o servidor for 32-bit.
- Registry `.reg` não importado.
- IP/porta SQL incorretos.
- Firewall bloqueando 1433.
- SQL Server em Docker está noutra máquina/VM.

## Chaves Registry usadas

```text
HKEY_LOCAL_MACHINE\SOFTWARE\PristonTale\GameServer
HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\PristonTale\GameServer
```

<!-- BEGIN AUTO:server-network -->
## Rede do servidor

### Porta de escuta

- Protocolo: TCP
- Porta: `9000`
- Bind esperado: `0.0.0.0:9000`

```bat
netstat -ano | findstr :9000
```

Resultado esperado:

```text
0.0.0.0:9000 LISTENING
```

### Erro Winsock 10048

O erro `10048` indica que o endereço ou a porta já está em uso.

```bat
tasklist /FI "IMAGENAME eq Server.exe"
netstat -ano | findstr :9000
taskkill /IM Server.exe /F
```

### Instrumentação recomendada

Adicionar logs em `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, dispatch do opcode, `send()` e `closesocket()`.
<!-- END AUTO:server-network -->
