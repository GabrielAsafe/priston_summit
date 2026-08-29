# Summit Tale — Visão Geral do Projeto

Data: 2026-07-01

## Objetivo

Documentar o projeto Summit Tale: cliente C/C++, servidor Windows, websites PHP, SQL Server em Docker, CloudBeaver, bases restauradas e plano de configuração única.

## Estrutura principal

```text
summit/
├── cliente/
├── docker/
├── docs/
├── Servidor/
├── PROJECT_INVENTORY.md
├── Documentation.md
├── folders.md
├── client-imports.png
├── server-imports.png
├── import-diagram-client.dot
└── import-diagram-server.dot
```

## Estado conhecido

- Docker MSSQL funcionando.
- CloudBeaver funcionando.
- Sites respondendo HTTP 200.
- Todas as bases principais restauradas.
- Servidor Windows ainda precisa ser testado após ajuste de config SQL/ODBC.
- Cliente ainda não foi testado.
- Foram encontrados IPs e credenciais hardcoded em `Servidor/sql.reg`.
