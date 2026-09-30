# Deployment Architecture

## Artefactos

### Client

- `Game.exe`;
- DLLs runtime;
- `Content/packs/*`;
- assets visuais/áudio que não fazem parte dos packs de configuração.

### Server

- `Server.exe`;
- DLLs runtime;
- `Database/summit.sqlite3`;
- `Content/packs/*`;
- recursos físicos ainda necessários ao runtime.

## Startup server — sequência esperada

1. abrir persistência;
2. carregar configuração;
3. inicializar materiais/personagens/monstros;
4. carregar definitions packs;
5. inicializar NPCs;
6. carregar maps pack e stages físicos;
7. concluir world init;
8. abrir listener;
9. aceitar sessões.

## Health checks mínimos

- processo ativo;
- TCP 1997 em LISTEN;
- SQLite aberto;
- definitions pack carregado;
- maps pack carregado;
- runtime contract/packs válidos;
- login de smoke test.
