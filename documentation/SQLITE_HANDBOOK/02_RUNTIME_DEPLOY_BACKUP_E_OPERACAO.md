# 2. Runtime, deploy, backup e operação

## 2.1 Runtime oficial

O runtime validado é:

```text
/home/gabriel/Área de Trabalho/summit/Servidor_R25_Wine
```

Iniciar a partir da raiz:

```bash
cd "/home/gabriel/Área de Trabalho/summit/Servidor_R25_Wine"
wine Server.exe
```

Isso é importante porque muitos caminhos são relativos.

## 2.2 Build oficial

Servidor:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
CLEAN=1 sh server.sh
```

Cliente:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
CLEAN=1 sh cliente.sh
```

Não usar scripts de link/build alternativos como fluxo normal quando `server.sh`/`cliente.sh` já representam o pipeline do projeto.

## 2.3 Deploy

O `send_to_s.sh` original fazia `scp` para um ambiente Windows antigo.

Ele foi alterado para deploy local ao runtime Wine:

```text
Servidor_R25_Wine
```

O deploy deve validar que o `Server.exe` de `dist_mingw_server_i686` e o runtime são o mesmo binário.

Nunca trocar `Server.exe` com uma instância antiga ainda carregada e depois assumir que o processo em execução usa o novo binário.

## 2.4 Database.dll

No ambiente local-only testado:

```text
Database.dll
```

deve permanecer desativada/renomeada.

Exemplo histórico:

```text
Database.dll.R25_DISABLED
```

A autenticação local SQLite não deve depender da DLL.

## 2.5 legacy_billing_fallback

A migration 032 cria:

```text
LocalAuth__RuntimePolicy
```

e semeia:

```text
legacy_billing_fallback = 1
```

Esse valor é lido no startup.

Para verificar:

```bash
python3 tools/sqlite_local_admin.py \
  --db "/home/gabriel/Área de Trabalho/summit/Servidor_R25_Wine/Database/summit.sqlite3" \
  status
```

Para local-only estrito:

```bash
python3 tools/sqlite_local_admin.py \
  --db ".../Database/summit.sqlite3" \
  set-billing-fallback off
```

Depois é necessário reiniciar o servidor.

Não assumir o valor atual sem consultar `status`.

## 2.6 Backup correto

Com WAL, **não** fazer:

```bash
cp summit.sqlite3 backup.sqlite3
```

com o servidor ativo.

Usar SQLite backup API.

O utilitário R25 já faz isso:

```bash
python3 tools/sqlite_local_admin.py \
  --db ".../Database/summit.sqlite3" \
  backup
```

Ele valida:

- `PRAGMA quick_check`;
- `PRAGMA foreign_key_check`;
- `user_version`;
- número de tabelas;
- snapshot consistente incluindo WAL commitado.

## 2.7 Health check mínimo

Antes e depois de alteração importante:

```sql
PRAGMA user_version;
PRAGMA quick_check;
PRAGMA foreign_key_check;
PRAGMA journal_mode;
```

Também conferir número de tabelas:

```sql
SELECT COUNT(*)
FROM sqlite_master
WHERE type='table'
  AND name NOT LIKE 'sqlite_%';
```

No runtime atual espera-se aproximadamente:

```text
user_version=32
tables=339
quick_check=ok
```

A contagem 339 é estado runtime pós-importação, não o bootstrap oficial R25.

## 2.8 Backup antes de migration/import

Fluxo recomendado:

1. parar `Server.exe`;
2. `quick_check`;
3. backup via API;
4. executar alteração numa cópia/staging;
5. validar;
6. aplicar;
7. validar runtime;
8. iniciar servidor;
9. executar smoke test dirigido.

## 2.9 Não mover apenas a DB para a pasta antiga

Trocar só `summit.sqlite3` não reproduz o runtime.

Também existem:

- `DataServer/userinfo`;
- `DataServer/userdata`;
- warehouses;
- configs;
- DLLs;
- migrations;
- Server.exe correspondente.

Por isso `Servidor_R25_Wine` é a unidade operacional validada.
