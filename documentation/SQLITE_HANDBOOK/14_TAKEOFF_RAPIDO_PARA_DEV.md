# 14. Takeoff rápido para um novo developer

Objetivo: conseguir trabalhar com a persistência Summit sem passar horas descobrindo a arquitetura.

## Minuto 0–5: entender as duas persistências

Memorizar:

```text
SQLite = contas/auth/GM/quests/coins/catálogos/logs
DataServer files = personagem principal/inventário/stats/warehouse
```

Nunca assumir que “a DB” contém tudo.

## Minuto 5–10: localizar runtime e source

Source:

```text
/home/gabriel/Área de Trabalho/summit/SourceCode
```

Runtime:

```text
/home/gabriel/Área de Trabalho/summit/Servidor_R25_Wine
```

DB:

```text
Servidor_R25_Wine/Database/summit.sqlite3
```

Build server:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
CLEAN=1 sh server.sh
```

## Minuto 10–15: aprender o namespace

SQL Server antigo:

```text
QuestDB.dbo.Progress
```

SQLite:

```text
QuestDB__Progress
```

C++ ainda pode usar:

```sql
SELECT * FROM Progress
```

quando a conexão é `DATABASEID_QuestDB`.

## Minuto 15–20: aprender o admin helper

Status:

```bash
python3 tools/sqlite_local_admin.py \
  --db "../Servidor_R25_Wine/Database/summit.sqlite3" \
  status
```

Conta:

```bash
... create-account minhaConta --auth local
... show minhaConta
```

GM:

```bash
... set-gm minhaConta --security 4
```

Backup:

```bash
... backup
```

## Minuto 20–25: aprender as regras que quebram tudo

Não fazer:

```text
editar migration velha
reordenar coluna de tabela legada
usar IF/BEGIN/ELSE T-SQL
usar ISNULL() esperando função SQL Server
copiar summit.sqlite3 ignorando WAL
criar personagem só com SQL
usar INSERT OR REPLACE sem UNIQUE
inventar valor para NULL histórico
```

## Minuto 25–30: primeiro workflow seguro

Para mudar uma feature persistente:

1. procurar os callers C++;
2. descobrir logical database;
3. `PRAGMA table_info`;
4. descobrir `SELECT *`/inserts posicionais;
5. escrever migration nova se schema mudar;
6. backup;
7. testar numa cópia;
8. clean build;
9. deploy pelo wrapper;
10. smoke no Wine;
11. `quick_check`.

## Primeiro ficheiro a ler por tipo de tarefa

Conta/auth:

```text
tools/sqlite_local_admin.py
Shared/server/legacy_runtime/OnSever_legacy_server.inl
```

Quest:

```text
cQuestHandler.cpp
Database/migrations/003,005,009
```

Shop:

```text
CoinShop.cpp
CoinShop_T.cpp
Database/migrations/002,014,029
```

Schema:

```text
SQLiteConnection.cpp
Database/migrations/README.md
```

Personagem:

```text
record_legacy_server.inl
record_legacy_client.inl
netplay.cpp
```

## Definition of done para persistência

Uma mudança só está concluída quando:

```text
source contém a regra
migration reproduz o schema
banco real foi validado
bootstrap vazio foi validado
upgrade foi validado
runtime Wine passou
dados sobrevivem logout/relogin/restart
quick_check=ok
```
