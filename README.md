# SummitTale

Projeto de modernização e engenharia reversa do cliente, servidor, banco de dados e ferramentas do SummitTale.

## Estrutura da documentação

### Migração para Linux / MinGW

- `SummitTale_Retomada_Linux_MinGW_Julho_2026.md` — estado atual da migração do cliente para MinGW/Linux, problemas encontrados e próximos passos.
- `RETOMADA_SUMMITTALE_MINGW_LINKAGEM_LUA.md` — histórico da migração da Lua, correções de linkagem e integração com MinGW.

### Banco de dados

- `07_DATABASE_SCHEMA.md` — visão geral do esquema do banco.
- `database/`
  - `SCHEMA_TABLES.md`
  - `SCHEMA_COLUMNS.md`
  - `SCHEMA_RELATIONS.md`
- `db-schema/` — exportação das colunas de todas as bases (AccountDB, ServerDB, PristonDB, SecurityDB, QuestDB, CoinShopDB etc.).
- `db-schema.tar.xz` — pacote contendo o dump do esquema.

### PHP / Website

Documentação do sistema web:

- `php/00_overview.md`
- `php/01_routes.md`
- `php/02_database_map.md`
- `php/03_player_panel.md`
- `php/04_admin_panel.md`
- `php/05_tables.md`
- `php/06_sql_dependencies.md`
- `php/07_legacy_php_issues.md`
- `php/08_server_db_startup_check.md`
- `php/09_server_after_itemlogdb_suspects.md`

### Manutenção Web

- `WEB_MAINTENANCE_SUMMIT.md`

### Auditorias

- `audits/` — auditorias e artefatos gerados durante a análise do projeto.

## Estado atual

### Cliente

- Build Linux utilizando MinGW-w64.
- Migração principal direcionada para **i686 (32-bit)**.
- Linkagem funcional.
- Inicialização Direct3D corrigida.
- Investigação atual concentrada no ciclo de vida da janela principal e no `Winmain.cpp`.

### Servidor

- Migração em andamento.
- Fluxo de autenticação parcialmente documentado.
- Próximo objetivo é restaurar a compilação completa em MinGW.

## Fluxo recomendado para novos colaboradores

1. Ler `SummitTale_Retomada_Linux_MinGW_Julho_2026.md`.
2. Ler `RETOMADA_SUMMITTALE_MINGW_LINKAGEM_LUA.md`.
3. Consultar `07_DATABASE_SCHEMA.md`.
4. Utilizar `database/` e `db-schema/` para entender a estrutura das bases.
5. Para o sistema web, iniciar em `php/00_overview.md` e seguir a sequência numérica.

## Objetivos do projeto

- Compilar cliente e servidor sem Visual Studio.
- Modernizar o processo de build.
- Documentar completamente a arquitetura do jogo.
- Preservar compatibilidade com os recursos e bancos de dados existentes.
