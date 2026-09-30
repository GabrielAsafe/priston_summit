# 1. Arquitetura e estado atual

## 1.1 Um único ficheiro, vários bancos lógicos

O desenho atual mantém a ideia de vários bancos do SQL Server, mas todos vivem no mesmo ficheiro SQLite:

```text
Database/summit.sqlite3
```

O namespace físico segue:

```text
<LogicalDB>__<Table>
```

Exemplos:

| SQL Server legado | SQLite |
|---|---|
| `AccountDB.dbo.GameMasters` | `AccountDB__GameMasters` |
| `CoinShopDB.dbo.ItemList` | `CoinShopDB__ItemList` |
| `QuestDB.dbo.Complete` | `QuestDB__Complete` |
| `ServerDB.dbo.Vip` | `ServerDB__Vip` |
| `ClanDB.dbo.SiegeMoneyTax` | `ClanDB__SiegeMoneyTax` |

Isso permite manter grande parte do código legado usando `EDatabaseID` e nomes bare como `Progress`, `UserInfo` ou `GameMasters`.

## 1.2 APIs C++ preservadas

As duas interfaces antigas foram preservadas:

- `SQLConnection`
- `SQL`

Ambas acabam no backend `SQLiteConnection`.

A escolha do `EDatabaseID` define como um nome sem database é resolvido. Por exemplo, uma conexão criada com `DATABASEID_QuestDB` transforma:

```sql
SELECT * FROM Progress
```

em algo equivalente a:

```sql
SELECT * FROM QuestDB__Progress
```

Referências explícitas do SQL Server como:

```sql
[CoinShopDB].[dbo].[UserInfo]
```

também são traduzidas para a tabela física correspondente.

## 1.3 Pragmas de runtime

Na inicialização, o backend configura:

```sql
PRAGMA foreign_keys=ON;
PRAGMA journal_mode=WAL;
PRAGMA synchronous=NORMAL;
```

Também existe `busy_timeout` de 5000 ms.

Consequências:

- o ficheiro `summit.sqlite3-wal` pode conter commits ainda não checkpointados;
- copiar apenas `summit.sqlite3` com o servidor ativo não é backup seguro;
- múltiplos readers funcionam bem;
- writes concorrentes continuam serializados pelo SQLite.

## 1.4 sqlite3.dll

O servidor carrega SQLite dinamicamente:

```text
LoadLibraryA("sqlite3.dll")
```

Como o `Server.exe` é PE32/i386, a DLL também deve ser **x86**.

A DLL deve estar:

- ao lado do `Server.exe`; ou
- disponível no `PATH` do processo Wine.

Não adicionar import/static library SQLite ao linker sem uma decisão arquitetural explícita; o desenho R25 é carregamento dinâmico.

## 1.5 Migrations

O R25 original aplica automaticamente:

```text
001 ... 032
```

via `SQLiteConnection::Init()`.

O estado é controlado por:

```sql
PRAGMA user_version;
```

Cada migration roda em:

```sql
BEGIN IMMEDIATE;
...
PRAGMA user_version=N;
COMMIT;
```

Falha implica rollback.

Se o banco tiver `user_version` maior do que o servidor conhece, o servidor recusa abrir em vez de fazer downgrade silencioso.

## 1.6 Estado clean bootstrap vs runtime atual

### Bootstrap R25 original

```text
user_version = 32
338 user tables
```

### Runtime validado depois da importação completa

```text
user_version = 32
339 user tables
quick_check = ok
```

A tabela adicional é:

```text
ADM__loginGM
```

Ela veio do SQL Server/Docker porque continha dados legados e não existia no schema R25.

**Atenção:** não existe `DATABASEID_ADM` no mapping atual do backend. A tabela foi preservada como dado histórico, mas não existe hoje uma rota padrão `SQLConnection` para um database lógico ADM. Se uma feature futura precisar dela, deve-se criar um contrato explícito.

## 1.7 Dados importados do SQL Server

Foi feito um export read-only completo do SQL Server Docker:

```text
22 bancos de utilizador
329 tabelas
1391 registros
39 tabelas não vazias
290 tabelas vazias
```

O merge para o SQLite preservou estruturas locais R25 e estado local.

Algumas contagens críticas pós-importação:

```text
BattlePass__BP_Info              21
BattlePass__BP_Item              40
CoinShopDB__ItemList            110
CoinShopDB__ItemList_T           34
CoinShopDB__UserInfo             34
CoinShopDB__UserInfo_T           34
QuestDB__Complete               195
QuestDB__ItemRewards             20
QuestDB__MonsterKilled           14
QuestDB__Progress                20
QuestDB__QuestMonsters           21
QuestDB__Rewards                 21
QuestDB__SingleQuest              5
SecurityDB__WindowCheatList     108
ServerDB__Vip                    16
```

## 1.8 Estruturas locais que não vêm do SQL Server

O R25 adiciona estruturas server-owned, por exemplo:

```text
LocalAuth__AccountPolicy
LocalAuth__RuntimePolicy
LocalAuth__GameplayEntitlement

GameLogDB__LocalCharacterEvent
ItemLogDB__LocalItemEvent
ItemLogDB__LocalItemEventEntry
```

Há também tabelas source-derived de compatibilidade como:

```text
QuestDB__RepeatedQuest
SecurityDB__AgingRecord
SecurityDB__BannedIP
SecurityDB__BannedMac
SecurityDB__ClassCheatList
StartUserDB__ExtraItems
```

Essas tabelas não devem ser eliminadas durante imports de dados legados.

## 1.9 O que não está na SQLite

O estado principal do personagem continua **file-backed**.

Principais diretórios:

```text
DataServer/userinfo/
DataServer/userdata/
DataServer/userdata_backup/
DataServer/warehouse/
DataServer/deleted/
DataServer/PostBox/
```

Logo:

- conta/auth/GM/coins/quests: SQLite;
- personagem completo/inventário/stats/save principal: ficheiros `DataServer`.

Essa separação é central para entender o servidor.
