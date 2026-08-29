Concluí o pacote com todos os planos de portabilidade, já cruzados com a source e com as descobertas da auditoria anterior.

[Baixar o pacote completo — Summit_Tale_Port_Plans.zip](sandbox:/mnt/data/Summit_Tale_Port_Plans.zip)

O ZIP contém 9 arquivos:

* `00_INDICE_MESTRE.docx` — visão geral, princípios obrigatórios e ordem recomendada.
* `01_SQLITE_E_CLAN_SERVICE.docx` — plano detalhado para substituir SQL Server/ODBC/COM por **SQLite**, eliminar `Database.dll`/`Clan.dll` e retirar ASP/PHP/ASPX/WinInet do sistema de clãs.
* `02_SERVIDOR_LINUX_NATIVO.docx` — plano completo do `Server.exe` Windows para **servidor Linux headless nativo**.
* `03_SEPARACAO_CLIENT_SERVER_SHARED.docx` — plano de separação **sistema por sistema** entre Client, Server e Shared.
* `04_CONTRATOS_DE_COMPATIBILIDADE.docx` — tudo que deve continuar byte/layout-compatible durante a migração: protocolo, saves, configs, assets e ABI.
* `05_ROADMAP_MESTRE.docx` — milestones M0–M9, dependências, ordem das mudanças, gates e smoke tests.
* `05_SYSTEM_BOUNDARY_MATRIX.csv` — matriz mecânica de todos os sistemas encontrados, com situação atual e destino arquitetural.
* `06_SQLITE_QUERY_MIGRATION_INVENTORY.csv` — inventário das queries SQL Server-specific encontradas e orientação de conversão para SQLite.
* `07_LINUX_PLATFORM_BLOCKERS.csv` — inventário das dependências/APIs Win32 que bloqueiam o servidor Linux.

Os dois documentos centrais também estão disponíveis separadamente:

[Plano SQLite + Clan Service](sandbox:/mnt/data/port_plans/01_SQLITE_E_CLAN_SERVICE.docx)

[Plano do servidor Linux nativo](sandbox:/mnt/data/port_plans/02_SERVIDOR_LINUX_NATIVO.docx)

E o plano de separação:

[Plano Client / Server / Shared](sandbox:/mnt/data/port_plans/03_SEPARACAO_CLIENT_SERVER_SHARED.docx)

### O que ficou definido para SQLite

O plano não propõe espalhar chamadas `sqlite3_*` pelo código. A ideia é preservar inicialmente a API que o jogo já conhece:

```text
SQLConnection
    Open()
    Prepare()
    BindInputParameter()
    Execute()
    GetData()
    NextRow()
    Close()
```

e trocar apenas o backend:

```text
ANTES

game systems
    ↓
SQLConnection / SQL
    ↓
ODBC
    ↓
SQL Server
```

para:

```text
DEPOIS

game systems
    ↓
repository / SQLConnection compatibility
    ↓
SQLiteConnection
    ↓
data/summit.db
```

Os antigos:

```text
DATABASEID_AccountDB
DATABASEID_ClanDB
DATABASEID_CoinShopDB
DATABASEID_QuestDB
DATABASEID_SecurityDB
...
```

podem continuar existindo durante a transição, mas passam a ser **aliases lógicos para o mesmo `summit.db`**.

O documento diz exatamente quais módulos criar, incluindo uma estrutura como:

```text
server/
└── persistence/
    ├── sqlite/
    │   ├── SQLiteConnection.h
    │   ├── SQLiteConnection.cpp
    │   ├── SQLiteStatement.h
    │   └── SQLiteStatement.cpp
    │
    ├── DatabaseRouter.h
    ├── DatabaseRouter.cpp
    │
    └── migrations/
        ├── 001_initial.sql
        ├── 002_accounts.sql
        ├── 003_clans.sql
        ├── 004_coinshop.sql
        ├── 005_quests.sql
        └── ...
```

com:

```text
data/
├── summit.db
└── clan_marks/
```

Também documentei as diferenças SQL que encontrei, como:

```sql
SELECT TOP 10 ...
```

virando:

```sql
SELECT ...
LIMIT 10
```

`GETDATE()`, `DATEADD`, `IF NOT EXISTS`, referências `[Database].[dbo].[Table]`, updates com `+=`, etc. O CSV contém **arquivo e linha** para cada ocorrência encontrada.

### O clan web deixa de existir

O plano transforma:

```text
Clan UI C++
      ↓
ISAOCHECK
      ↓
WinInet
      ↓
ASP/PHP/ASPX
      ↓
SQL Server
```

em:

```text
Clan UI C++
      ↓
ClanClientGateway
      ↓
game socket
      ↓
ClanProtocolHandler
      ↓
ClanService
      ↓
ClanRepository
      ↓
SQLite
```

Mantendo o cliente visual em C++.

Os antigos endpoints como:

```text
isCheckClanJang.asp
isCheckClanName.asp
isCheckClanNum.asp
GetAllMyClanMember.asp
clanInsertClanWon.asp
clanRemove.asp
ClanSubChipUpdate.asp
clanInsert.aspx
clan_insertClanImage.php
/ClanContent/*.bmp
```

têm uma contraparte planejada no `ClanService`.

Também deixei uma regra importante: existem opcodes de clan hoje, mas **não vamos simplesmente reutilizar o mesmo opcode com uma struct diferente**, porque isso poderia colidir com o protocolo histórico. O novo protocolo precisa de subcommands/versionamento ou de novos opcodes confirmadamente livres.

---

### Para Linux, o plano não tenta portar código desnecessário

A separação final que proponho é aproximadamente:

```text
project/
│
├── client/
│   ├── ui/
│   ├── render/
│   ├── audio/
│   ├── effects/
│   ├── input/
│   └── presentation/
│
├── server/
│   ├── main/
│   ├── network/
│   ├── gameplay/
│   ├── services/
│   ├── persistence/
│   └── admin/
│
├── shared/
│   ├── protocol/
│   ├── game_data/
│   ├── world_core/
│   ├── math/
│   └── util/
│
├── platform/
│   ├── windows/
│   └── linux/
│
└── third_party/
    ├── lua/
    └── sqlite/
```

Com a regra:

```text
Client ──────┐
             ├──→ Shared
Server ──────┘

Shared -X→ Client
Shared -X→ Server

Client -X→ Server
Server -X→ Client
```

Hoje existe inclusive uma violação concreta:

```cpp
Shared/items.h
    ↓
#include "SrcGame/src/sinbaram/sinItem.h"
```

Isso precisa ser invertido.

O novo `Shared` terá apenas coisas como:

```text
opcodes
wire structs
IDs
enums
item definitions puras
skill definitions puras
math portátil
stage/model parsing realmente comum
collision/world core
```

Não:

```text
windows.h
DirectX
UI
ODBC
SQLite
filesystem específico
socket implementation
render
audio
database
```

### O servidor final não terá render/UI do cliente

O target Linux pretendido deve chegar a:

```text
summit-server
├── protocol
├── networking
├── world
├── collision
├── gameplay
├── persistence
├── services
└── admin
```

e **não linkar**:

```text
Direct3D
DirectDraw
DirectSound
GDI
User32
WinInet
DInput
client UI
particles
client effects
login screen
settings UI
```

O documento também diferencia algo importante: o servidor pode precisar de partes atualmente presas em `smLib3d`, como:

```text
Stage loader
model data
math
collision
height checks
spatial partition
```

mas isso não significa levar `smLib3d` inteiro para Shared. A proposta é extrair somente o núcleo headless:

```text
shared/world_core/
├── StageData
├── StageLoader
├── Collision
├── SpatialGrid
├── ModelData
└── Math
```

e deixar D3D/render no client.

---

### Ordem planejada

O roadmap final ficou:

```text
M0  congelar baseline
 ↓
M1  targets explícitos + contratos Shared
 ↓
M2  backend SQLite compatível
 ↓
M3  remover Database.dll / Clan.dll
 ↓
M4  ClanService C++ sem web
 ↓
M5  Server Windows headless
 ↓
M6  abstração de plataforma
 ↓
M7  Linux i686 nativo
 ↓
M8  deduplicação Client/Server completa
 ↓
M9  Linux x86-64 opcional
```

Eu considero especialmente importante **não começar diretamente pelo Linux**. Primeiro fazer o servidor Windows atual funcionar com:

```text
SQLite
sem SQL Server
sem ODBC
sem Database.dll
sem Clan.dll
sem web clan
```

Isso permite saber que a camada de dados está correta antes de mexermos em sockets, threads e event loop.

Depois removemos a janela Win32/`WSAAsyncSelect`, primeiro ainda no Windows. Só depois implementamos o backend POSIX.

Esse caminho reduz drasticamente a chance de aparecer um bug sem sabermos se veio de SQLite, Linux, protocolo, thread ou gameplay.

Todos os DOCX foram renderizados e revisados visualmente antes de montar o ZIP.

