# Summit Documentation Package

Copiar para a raiz do projeto `summit/`:

```bash
cp -r docs config tools "/home/gabriel/Área de Trabalho/summit/"
cd "/home/gabriel/Área de Trabalho/summit"
cp config/summit.local.env.example config/summit.local.env
bash tools/generate-sql-reg.sh config/summit.local.env Servidor/sql.reg
```

<!-- BEGIN AUTO:current-status-connection -->
## Estado atual

O servidor SummitTale já:

- compila em MinGW i686;
- conecta ao SQL Server;
- carrega os sistemas principais;
- abre a porta TCP;
- permanece em execução;
- executa como aplicação de consola.

O trabalho atual está concentrado na conexão entre o cliente e o servidor.

Documentos principais:

- `MINGW_PORT_STATUS_2026_07.md`
- `docs/milestone_servidorcompilado_funcional.md`
- `docs/09_CLIENT_SERVER_CONNECTION.md`
- `investigando cliente e server não conectarem.md`
- `docs/04_SERVER.md`
- `docs/05_CLIENT.md`
- `opcodes.md`
<!-- END AUTO:current-status-connection -->
