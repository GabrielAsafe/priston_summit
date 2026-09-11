Sim. E depois de olhar essa parte da source, eu mudaria a arquitetura de forma mais agressiva do que simplesmente “juntar os dois sites”.

## Onde estão as URLs do clã

O núcleo está duplicado no client e server, principalmente em:

```text
SrcGame/src/TJBOY/isaocheck/
SrcServer/src/TJBOY/isaocheck/
```

O arquivo mais importante é:

```text
SrcGame/src/TJBOY/isaocheck/isaocheck.cpp
```

e existe praticamente a mesma implementação em:

```text
SrcServer/src/TJBOY/isaocheck/isaocheck.cpp
```

Há dois hosts:

```cpp
char hostName[64];
char hostName1[64];
```

em `isaocheck.h:74-77`.

Eles são configurados por:

```cpp
ISAOCHECK::init_HostName_Port(...)
ISAOCHECK::init_HostName_Port1(...)
```

em `isaocheck.cpp:184-204`.

Historicamente aparecem estes dois IPs em `auth.cpp:3555-3556`:

```cpp
char hostN0[] = "211.39.144.157";
char hostN1[] = "211.39.144.158";
```

Mas há uma pegadinha importante: **esses valores hardcoded não são necessariamente os usados atualmente**.

O client recebe os endereços do clan server através do próprio game server em:

```text
SrcGame/src/netplay.cpp:3596-3612
```

especialmente:

```cpp
web_DB_ip_port_init(
    lpTransServerList->ServerInfo[cnt].szServerIp1,
    lpTransServerList->ServerInfo[cnt].dwServerPort1,

    lpTransServerList->ServerInfo[cnt].szServerIp2,
    lpTransServerList->ServerInfo[cnt].dwServerPort2,

    lpTransServerList->ClanTicket
);
```

E o servidor lê isso da configuração em:

```text
SrcServer/src/fileread.cpp:2427-2460
```

através da entrada:

```text
*CLAN_SERVER
```

Essa configuração é carregada por:

```text
SrcServer/src/Server/OnSever.cpp:1707
```

com:

```cpp
smConfigDecode("Configs//hotuk.ini");
```

O `hotuk.ini` não está no pacote de source que você me enviou, então eu não consigo dizer quais IPs você está usando hoje. Mas é ali que está a configuração efetiva.

### O que cada “site” faz

O primeiro host, `hostName`, concentra quase todas as operações de banco/clã:

| Função C++            | Linha aprox. | Endpoint                             |
| --------------------- | -----------: | ------------------------------------ |
| `isCheckClanJang`     |          693 | `/ServerMain/isCheckClanJang.asp`    |
| `isCheckClanName`     |          780 | `/ServerMain/isCheckClanName.asp`    |
| `ReadClanInfo`        |          864 | `/ServerMain/isCheckClanNum.asp`     |
| `isCheckClanMember`   |          951 | `/ServerMain/isCheckClanwon.asp`     |
| `isClanMember`        |         1021 | `/ServerMain/isClanMember.asp`       |
| `joinClanMember`      |         1118 | `/ServerMain/clanInsertClanWon.asp`  |
| `read_ClanName`       |         1311 | `/tjboyh/clan_readClanName.php`      |
| `read_ClanData`       |         1374 | `/ServerMain/GetAllMyClanMember.asp` |
| `BreakUPClan`         |         1455 | `/ServerMain/clanRemove.asp`         |
| `Banishiment`         |         1741 | `/ServerMain/clanWonRelease.asp`     |
| `AppointSubChip`      |         1826 | `/ServerMain/ClanSubChipUpdate.asp`  |
| `ReleaseSubChip`      |         1908 | `/ServerMain/ClanSubChipRelease.asp` |
| `LeaveClan`           |         1986 | `/ServerMain/clanChipChange.asp`     |
| `Secession`           |         2069 | `/ServerMain/clanWonSelfLeave.asp`   |
| `isKPFlag`            |         2152 | `/ServerMain/isKflag.asp`            |
| `isPFlag`             |         2224 | `/ServerMain/isPFlag.asp`            |
| `WarhouseRight`       |         2295 | `/tjboyh/clan_WarehouseRight.php`    |
| `updateFlag`          |         2372 | `/tjboyh/updateFlag.php`             |
| `write_ClanImageFile` |         2875 | `/tjboyh/clan_insertClanImage.php`   |

O segundo, `hostName1`, é usado principalmente para:

```text
/Clan/clanInsert.aspx
/ClanContent/<arquivo>.bmp
```

Por exemplo:

```text
isaocheck.cpp:1207  makeClan()
isaocheck.cpp:2557  read_ClanImage()
isaocheck.cpp:2662  read_ClanImage2()
```

Então sim: **dá para colocar os dois no mesmo host imediatamente**.

No `*CLAN_SERVER`, você poderia fazer IP1 e IP2 apontarem para a mesma máquina/porta, desde que esse único host sirva:

```text
/ServerMain/*
/tjboyh/*
/Clan/*
/ClanContent/*
```

Mas eu não faria isso como solução final.

---

# Eu eliminaria os dois sites

O frontend de clã do jogador **já é C++**.

Ele está principalmente aqui:

```text
SrcGame/src/TJBOY/clanmenu/
```

com classes para:

```text
criação de clan
membros
join
kick/ban
sub-chief
chat
notice
clan mark
informações do clan
```

Ou seja, PHP/ASP/ASPX não são realmente o frontend. Eles estão funcionando como uma espécie de **API REST primitiva de 2004**.

A arquitetura atual é basicamente:

```text
Clan UI C++
    ↓
isaocheck.cpp
    ↓
HTTP
    ↓
ASP / PHP / ASPX
    ↓
SQL Server
```

Eu substituiria por:

```text
Clan UI C++
    ↓
game socket existente
    ↓
Server C++
    ↓
ClanService
    ↓
SQLite
```

Sem:

```text
Apache
IIS
PHP
ASP
ASPX
WinInet
site de clan
servidor web
```

Isso é muito mais coerente com o que você quer fazer.

---

# SQLite é viável?

**Sim. Para o seu cenário local, eu acho uma excelente escolha.**

Mas não é simplesmente trocar uma connection string.

A boa notícia é que grande parte do banco já está abstraída.

Temos três sistemas de acesso a dados diferentes atualmente.

### 1. `SQLConnection`

Está aqui:

```text
SrcServer/src/Database/Database.h
SrcServer/src/Database/Database.cpp
```

Ele expõe exatamente o tipo de interface que queremos manter:

```cpp
Open()
Prepare()
BindInputParameter()
Execute()
GetData()
NextRow()
Close()

SQLConnection::GetConnection(DATABASEID_...)
```

Só 10–11 áreas principais do servidor usam diretamente essa interface.

Isso é ótimo.

Eu manteria a API pública e substituiria o interior ODBC por SQLite.

Hoje:

```text
SQLConnection
    ↓
ODBC
    ↓
SQL Server
```

Depois:

```text
SQLConnection
    ↓
sqlite3
    ↓
summit.db
```

Assim `CoinShop.cpp`, `cQuestHandler.cpp`, `GameMasters.cpp` etc. precisam de muito menos mudanças.

---

## Mas existe um segundo wrapper SQL

Há também:

```text
SrcServer/src/SQL.h
SrcServer/src/SQL.cpp
```

usado principalmente por:

```text
BattlePass.cpp
ClanCustom.cpp
JOBS.cpp
OnSever.cpp
```

Ele também usa ODBC.

Esse precisa ser absorvido pelo mesmo backend.

Eu acabaria com dois wrappers e criaria algo como:

```cpp
Database
DatabaseConnection
DatabaseStatement
```

ou manteria temporariamente as duas APIs chamando o mesmo SQLite.

---

# E há um terceiro sistema muito mais importante

Este foi o detalhe que eu considero fundamental para seu plano.

Existe:

```text
SrcServer/src/Server/DllServer.cpp
```

e ele faz:

```cpp
hClanDLL = LoadLibrary("Clan");
hLogSqlDLL = LoadLibrary("Database");
```

`DllServer.cpp:303-318`.

Depois procura coisas como:

```cpp
SQLLogon
SQLLogout
CharLog
GameLog
SetGameUser
GetGPCode
ClanMoney
EventJoin
PollRecord
ClanUserTicket
ClanPointInsert
...
```

com:

```cpp
GetProcAddress(...)
```

Esses dois DLLs antigos são uma parte importante do motivo de o port estar preso ao ecossistema Windows/SQL Server.

Inclusive seu port MinGW precisou adicionar:

```cpp
CoInitializeEx(...)
```

em `DllServer.cpp:479-507`, porque o antigo `Database.dll` usa **OLE DB/COM**.

Então, para eliminar SQL Server direito, não basta substituir:

```text
Database/Database.cpp
```

Temos que eliminar também:

```text
Database.dll
Clan.dll
```

Essa é, na verdade, uma boa oportunidade de arquitetura.

---

# Como eu faria o SQLite

Eu usaria **um único arquivo**:

```text
data/
└── summit.db
```

e tabelas organizadas logicamente:

```text
accounts
characters
game_masters

clans
clan_members
clan_marks
clan_permissions

coinshop_items
coinshop_users

battlepass_info
battlepass_items

quests
quest_progress
quest_monsters

pvp

security_bans

logs
item_logs
...
```

Hoje o servidor tenta abrir cerca de 17 bancos independentes:

```text
AccountDB
BillingDB
BillingLogDB
ClanDB
GameLogDB
ItemLogDB
LogDB
PCRoom
PCRoomLog
rPTDB
SoD2DB
SodDB
ServerDB
SecurityDB
CoinShopDB
QuestDB
StartUserDB
```

Para servidor local isso é complexidade sem benefício.

Eu colapsaria em:

```text
summit.db
```

mantendo `DATABASEID_*` apenas como aliases durante a migração.

Por exemplo:

```cpp
SQLConnection::GetConnection(DATABASEID_QuestDB)
```

poderia continuar funcionando, mas internamente retornar uma connection para o mesmo:

```text
data/summit.db
```

---

# SQL Server → SQLite exige adaptar queries

Fiz uma varredura específica.

Há aproximadamente **386 linhas da source contendo queries SQL**, distribuídas em cerca de 21 arquivos.

A maioria é SQL bastante simples e migra diretamente:

```sql
SELECT * FROM Progress WHERE PlayerName=?
```

```sql
INSERT INTO MonsterKilled VALUES (...)
```

```sql
UPDATE UserInfo SET UserCoin=? WHERE UserID=?
```

Mas existe SQL Server-specific.

Por exemplo:

```sql
SELECT TOP 10 * FROM PVP
```

vira:

```sql
SELECT * FROM PVP
LIMIT 10
```

Encontrei 11 usos de `TOP`.

Há 19 usos de:

```sql
GETDATE()
```

que podem virar:

```sql
CURRENT_TIMESTAMP
```

Há construções:

```sql
IF NOT EXISTS(...)
BEGIN
    INSERT ...
END
```

que podemos substituir elegantemente por:

```sql
INSERT ... ON CONFLICT ... DO NOTHING
```

ou UPSERT.

Há:

```sql
DATEADD(DAY, ...)
```

e:

```sql
DAY(GETDATE())
MONTH(GETDATE())
```

que precisam ser reescritos.

Existe também:

```sql
UPDATE MonsterKilled SET Killed+=1
```

que vira:

```sql
UPDATE MonsterKilled
SET Killed = Killed + 1
```

E existem queries cross-database como:

```sql
[BattlePass].[dbo].[BP_Info]
```

ou:

```sql
accountdb.dbo.GameMasters
```

Essas desapareceriam porque tudo estaria no mesmo SQLite:

```sql
BP_Info
GameMasters
```

Então é trabalho, mas é um trabalho **finito e bastante mecânico**.

---

# SQLite aguenta o servidor?

No seu cenário: **sim, tranquilamente**.

Eu configuraria:

```sql
PRAGMA journal_mode=WAL;
PRAGMA synchronous=NORMAL;
PRAGMA foreign_keys=ON;
PRAGMA busy_timeout=5000;
```

O servidor já serializa muita operação com critical sections.

Para um jogo local ou pequeno servidor privado, SQLite está muito dentro do que é necessário.

Se no futuro você quiser centenas/milhares de jogadores simultâneos e vários processos de servidor escrevendo no mesmo banco, aí eu mudaria para PostgreSQL.

Mas eu não começaria por isso.

---

# Uma coisa facilita muito: personagens já são arquivos

Nem toda persistência depende do SQL Server.

`record.cpp`, por exemplo, ainda salva dados do personagem diretamente:

```cpp
fopen(...)
fwrite(...)
```

e possui uma fila de gravação.

Então SQLite não precisa substituir imediatamente todo o sistema de character save.

Podemos migrar banco por subsistema.

---

# Arquitetura que eu faria

```text
                         GAME CLIENT
                             │
                        game protocol
                             │
                             ▼
┌─────────────────────────────────────────────────────┐
│                 SUMMIT SERVER                       │
│                                                     │
│ Network                                             │
│   └── GameProtocol                                  │
│                                                     │
│ Gameplay                                            │
│   ├── Character                                     │
│   ├── Items                                         │
│   ├── Skills                                        │
│   ├── Quests                                        │
│   ├── PvP                                           │
│   └── Events                                        │
│                                                     │
│ Services                                            │
│   ├── AccountService                                │
│   ├── ClanService                                   │
│   ├── ShopService                                   │
│   ├── BattlePassService                             │
│   ├── RankingService                                │
│   └── SecurityService                               │
│                                                     │
│ Persistence                                         │
│   └── SQLite                                        │
│       └── data/summit.db                            │
│                                                     │
│ Local assets                                        │
│   ├── character saves                               │
│   ├── clan marks                                    │
│   ├── configs                                       │
│   └── logs                                          │
└─────────────────────────────────────────────────────┘
```

Sem servidor web.

Sem SQL Server.

Sem `Database.dll`.

Sem `Clan.dll`.

---

# Clan mark

Também não precisa de webserver.

Hoje:

```text
client
→ HTTP
→ /ClanContent/xxxxx.bmp
```

Eu faria:

```text
client
→ REQUEST_CLAN_MARK(clan_id)
→ server
→ lê data/clan_marks/<id>.bmp
→ envia bytes
→ client salva no cache
```

Ou guardar o BMP diretamente como BLOB no SQLite.

Eu prefiro arquivo:

```text
data/clan_marks/
    000001.bmp
    000002.bmp
    ...
```

e no SQLite apenas:

```text
clan_id
filename
hash
updated_at
```

Mais simples para edição e backup.

---

# Plano para Linux nativo

Aqui eu faria uma migração em etapas. A ordem importa bastante:

1. **Primeiro remover dependência externa de dados ainda no build Windows/MinGW.** Implementar SQLite atrás de `SQLConnection`, absorver `SQL`, substituir `Database.dll` e `Clan.dll`, migrar login, GameMasters, CoinShop, Quest, PvP, BattlePass e clans. Neste ponto o `Server.exe` ainda é Windows, mas deve rodar localmente sem SQL Server, ODBC ou COM. Isso também lhe dá uma versão muito mais simples de testar no Wine.

2. **Eliminar o backend web de clãs.** Criar `ClanService` + `ClanRepository`, adicionar requests/responses de clan ao protocolo existente e mudar `TJBOY/clanmenu` para chamar o servidor do jogo em vez de `ISAOCHECK`. Depois remover `WinInet`, ASP/PHP/ASPX, `hostName/hostName1` e os dois clan websites. Faça o mesmo gradualmente para notice/SoD endpoints realmente necessários.

3. **Criar um target `server_headless`.** Seu `build_server_mingw_i686_j8.sh` atualmente compila praticamente tudo em `SrcServer/src`, inclusive uma quantidade enorme de UI, DirectDraw, efeitos e código de client espelhado. Esse é o motivo de `Server.exe` importar hoje `USER32`, `GDI32` e `ddraw.dll`. Para Linux, não porte isso: pare de compilar o que um servidor headless não precisa.

4. **Remover a janela Win32 do servidor.** Hoje `SetupDefWindow()` em `Server/OnSever.cpp:815-872` registra uma `WNDCLASS` e cria uma janela `HWND_MESSAGE`. Mais importante: `WndProc()` recebe `WSA_ACCEPT`, `WSA_READ` e `WM_TIMER`, e `WM_TIMER` chama `srPlayMain()`. Em Linux isso deve virar um loop explícito, por exemplo um `steady_clock` fazendo o tick do servidor e `poll/epoll` cuidando dos sockets.

5. **Portar `smwsock`.** Hoje ele usa `WSAAsyncSelect()` em `smwsock.cpp:1326`, `1421`, `1477` e `2680`, enviando eventos para a window procedure. Essa é provavelmente a maior mudança não relacionada ao banco. Preserve o protocolo e os buffers; troque apenas o transporte por POSIX sockets + `poll()` inicialmente. Depois, se necessário, `epoll`.

6. **Trocar sincronização Win32 por C++ standard.** Há centenas de usos de `CriticalSection`, `CreateThread`, `SuspendThread`, `ResumeThread`, `WaitForSingleObject` etc. O pior são `SuspendThread/ResumeThread`: não tente emular isso em Linux. Transforme essas filas em `std::thread + std::mutex + std::condition_variable`. `record.cpp` e `DllServer.cpp` são alvos importantes.

7. **Criar uma camada de plataforma.** Centralizar `time`, filesystem, sockets, threads e tipos básicos. `GetCurrentTime/GetTickCount` → `std::chrono::steady_clock`; `CopyFile` → `std::filesystem::copy_file`; `Sleep` → `std::this_thread::sleep_for`; `CRITICAL_SECTION` → mutex. Não espalhe `#ifdef __linux__` pelo projeto inteiro.

8. **Remover coisas exclusivamente Windows.** `HookAPI()`/`SetUnhandledExceptionFilter` de `Winmain.cpp:246-293` não fazem sentido no Linux; WinDump vira signal handler/backtrace; registry (`smReg`) vira arquivo de config; Windows Firewall vira configuração externa ou é removido; WinInet desaparece quando o clan HTTP sair; COM desaparece com `Database.dll`.

9. **Separar a parte 3D de server da renderização.** O servidor realmente reutiliza estruturas como Stage, collision, character e models, então você não pode simplesmente apagar `smLib3d`. Mas ele não precisa de Direct3D. Separe funções de `load/model/stage/collision/math` das de `render/D3D/texture`. O objetivo é o server linkar sem `ddraw`, `GDI32` ou qualquer DirectX.

10. **Primeiro compile Linux em i686, não x86-64.** Eu manteria `-m32` na primeira versão nativa. O cliente é i686 e o protocolo envia muitas structs C legadas diretamente pela rede. Passar ao mesmo tempo para Linux **e** mudar pointer/long/alignment para 64 bits cria duas variáveis perigosas. Depois de estabilizar Linux 32-bit, fazemos uma auditoria de wire structs e colocamos `uint32_t`, `int32_t`, packing explícito e `static_assert(sizeof(...))`; então o server pode virar x86-64 com segurança.

11. **Resolver filesystem case-sensitive.** Essa source nasceu no Windows e já vimos coisas como `Language/language.h` vs `Language/Language.h`, além de `\\` e `//`. Linux não perdoa isso. Crie uma política única de paths e normalize a árvore de assets/configs. Isso é um bloqueador real que não aparece no Wine.

12. **Criar testes de compatibilidade antes de declarar o port concluído.** O mesmo Game.exe Windows deve conseguir conectar tanto no Server.exe atual quanto no novo `summit-server` Linux. Testar login, load/save de personagem, movimento, chat, party, clan, item drop/pickup, shop, quest, PvP, SoD/Bellatra, Bless Castle, CoinShop, BattlePass, logout/relogin e shutdown limpo. Capture pacotes e compare comportamento.

---

# Onde eu começaria concretamente

Eu não começaria tentando compilar `SrcServer` hoje com GCC Linux.

Começaria por:

```text
ETAPA 1
Windows MinGW server
+
SQLite
+
sem Database.dll
+
sem Clan.dll
+
sem clan websites
```

Quando isso estiver funcional:

```text
ETAPA 2
mesmo código
+
server_headless
+
sem USER32/GDI/DDRAW
```

Só então:

```text
ETAPA 3
POSIX sockets
std::thread
std::filesystem
Linux i686
```

Isso reduz muito o espaço de bugs.

A fronteira que eu buscaria é:

```text
          ┌──────────────┐
          │ Game logic   │
          │ portable C++ │
          └──────┬───────┘
                 │
      ┌──────────┴──────────┐
      │                     │
  platform/win32       platform/linux
```

e depois você pode apagar `platform/win32` do servidor quando o Linux estiver maduro.

### Minha avaliação de viabilidade

**SQLite:** alta viabilidade.
**Eliminar os dois sites de clan:** altíssima viabilidade — e eu recomendo.
**Manter toda UI de clan em C++:** ela praticamente já está em C++.
**Server Windows sem SQL Server:** alta viabilidade e deveria ser o primeiro milestone.
**Server Linux nativo:** viável, mas é um port real, não apenas trocar o banco.
**Reaproveitar o Game.exe Windows com server Linux:** sim, e deveria ser o objetivo; o protocolo TCP não precisa mudar.

O ponto mais promissor é que você já fez parte do trabalho sem perceber: o server atual tem `main()` de console e uma CLI em `std::thread`, a camada SQL mais nova já está encapsulada, e boa parte da persistência de personagem continua baseada em arquivos. O principal legado a desmontar agora é **`DllServer.cpp` + ODBC/COM + web clan + WSAAsyncSelect/window messages**.

