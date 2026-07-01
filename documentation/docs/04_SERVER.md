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
