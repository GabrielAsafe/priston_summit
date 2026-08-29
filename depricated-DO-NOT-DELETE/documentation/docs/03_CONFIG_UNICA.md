# Summit Tale — Configuração Única

## Problema

O ficheiro `Servidor/sql.reg` contém IP, porta, utilizador, senha e nomes de bases repetidos e hardcoded.

Exemplos:

```reg
"Server1"="127.0.0.1,1433"
"AccountDbIP"="127.0.0.1,1433"
"AccountDbID"="sa"
"AccountDbPwd"="SummitTale@12345"
"AccountDbName"="AccountDB"
```

## Estratégia

Como o servidor atual provavelmente lê configurações do Registry, o caminho seguro é manter um ficheiro canónico e gerar o `.reg` a partir dele.

Fonte única:

```text
config/summit.local.env
```

Gerar Registry:

```bash
bash tools/generate-sql-reg.sh config/summit.local.env Servidor/sql.reg
```

## Quando usar cada host

Servidor Windows na mesma máquina do Docker:

```text
127.0.0.1,1433
```

Servidor Windows noutra VM/máquina:

```text
IP_DO_HOST_DOCKER,1433
```

Containers Docker acessando SQL Server:

```text
summittale-mssql:1433
```

## Futuro ideal

Alterar o código C++ para ler `.ini`, `.json` ou `.env` diretamente, evitando Registry. Até lá, o `.reg` deve ser gerado automaticamente.
