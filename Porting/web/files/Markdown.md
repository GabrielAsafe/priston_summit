Sim. Eu usaria o painel PHP apenas como **especificação do que o administrador consegue fazer hoje** e reimplementaria tudo em C++ com uma arquitetura nova.

A decisão mais importante é esta:

```text
NÃO:

UI C++
 ├─ escreve SQLite diretamente
 ├─ altera .dat diretamente
 └─ mata/inicia processo diretamente

SIM:

UI C++
   ↓
AdminCore
   ├─ AccountService
   ├─ CharacterService
   ├─ EconomyService
   ├─ ClanService
   ├─ ItemService
   ├─ EventService
   ├─ ConfigService
   └─ ServerRuntimeService
        ↓
Repositories / Runtime Gateway
        ↓
SQLite + DataServer + summit-server
```

Assim você elimina PHP, IIS/Apache, ODBC e SQL Server sem criar outro monólito difícil de manter.

---

# 1. Arquitetura final que eu faria

```text
Summit Tale
│
├── summit-server
│   ├── gameplay
│   ├── network
│   ├── world
│   ├── persistence
│   └── AdminRuntimeGateway
│
├── summit-admin
│   └── aplicação C++ desktop
│
├── admin-core
│   ├── AuthService
│   ├── AccountService
│   ├── CharacterService
│   ├── PunishmentService
│   ├── EconomyService
│   ├── ClanAdminService
│   ├── ItemGrantService
│   ├── ItemAuditService
│   ├── RankingService
│   ├── EventAdminService
│   ├── ConfigService
│   ├── BackupService
│   └── ServerControlService
│
├── persistence
│   ├── sqlite
│   └── legacy
│       ├── CharacterDatCodec
│       ├── UserInfoDatCodec
│       └── PostBoxCodec
│
└── data
    ├── summit.db
    ├── DataServer/
    ├── PostBox/
    ├── ClanContent/
    └── Configs/
```

Eu usaria **Qt 6** para `summit-admin` se você quer uma aplicação administrativa tradicional com tabelas, filtros, formulários, logs e diálogos.

O frontend não precisa saber nada sobre offsets de `.dat`, SQL ou protocolo do jogo.

---

# 2. Duas APIs diferentes

Você precisa separar duas coisas.

### API administrativa de domínio

É o que a UI usa:

```cpp
AccountService
CharacterService
ClanService
EconomyService
...
```

### API de runtime do servidor

É o que permite administrar alguém que está conectado:

```cpp
ServerRuntimeService
```

Hoje o próprio servidor já possui uma versão rudimentar disso em:

```text
SrcServer/src/CLI/CLI.cpp
```

com:

```text
connected;
kick(name);
shutdown;
reloadhotuk;
reloadGMS;
showGMS;
boss;
bosschange;
```

Por exemplo, `CLI.cpp:193-209` percorre `rsPlayInfo[]` para listar os jogadores conectados, e `CLI.cpp:211-244` procura o jogador e chama `DisconnectUser()`.

Essa lógica deve virar API em vez de ficar presa ao parser de console.

---

# 3. AdminRuntimeGateway

Eu criaria no servidor:

```cpp
class IAdminRuntimeGateway
{
public:
    virtual std::vector<OnlinePlayer> listPlayers() = 0;

    virtual Result<PlayerSnapshot>
        getPlayer(std::string_view character) = 0;

    virtual Result<void>
        kick(std::string_view character,
             std::string_view reason) = 0;

    virtual Result<void>
        shutdown(ShutdownMode mode) = 0;

    virtual Result<void>
        reloadConfig() = 0;

    virtual Result<void>
        reloadGameMasters() = 0;

    virtual Result<void>
        broadcast(std::string_view message) = 0;

    virtual bool
        isCharacterOnline(std::string_view character) = 0;
};
```

Mais tarde:

```cpp
setLevel()
teleport()
setGold()
changeClass()
grantItem()
```

também podem ter versões runtime-safe.

---

# 4. Por que isso é necessário

Hoje o painel PHP faz isto:

```text
admin muda Level no Character.dat
             ↓
player ainda está online
             ↓
server continua com Level antigo na memória
             ↓
player desloga
             ↓
server salva estado antigo
             ↓
alteração do admin desaparece
```

Ou pode produzir corrupção lógica.

No sistema novo:

```text
CharacterService::setLevel("Player", 100)
               │
               ├── player ONLINE
               │       ↓
               │   comando para summit-server
               │       ↓
               │   altera memória
               │       ↓
               │   server salva normalmente
               │
               └── player OFFLINE
                       ↓
                  CharacterDatRepository
                       ↓
                  backup + mutation
                       ↓
                  atomic rename
```

Essa é uma diferença enorme de segurança.

---

# 5. Funcionalidades atuais que precisam ser reimplementadas

Este é o inventário funcional do `padm/` que eu preservei.

| Área          | Funcionalidade atual        | Nova API                               |
| ------------- | --------------------------- | -------------------------------------- |
| Administração | login de GM                 | `AuthService::login()`                 |
| Administração | permissões GM               | `AuthorizationService`                 |
| Contas        | localizar ID pelo nick      | `AccountService::findByCharacter()`    |
| Contas        | listar personagens da conta | `AccountService::listCharacters()`     |
| Contas        | consultar cadastro          | `AccountService::get()`                |
| Contas        | editar cadastro             | `AccountService::updateProfile()`      |
| Contas        | alterar senha               | `AccountService::changePassword()`     |
| Contas        | listar contas               | `AccountService::search()`             |
| Contas        | deletar conta               | `AccountService::deleteAccount()`      |
| Punição       | banir                       | `PunishmentService::ban()`             |
| Punição       | desbanir                    | `PunishmentService::unban()`           |
| Punição       | histórico de bans           | `PunishmentService::history()`         |
| Personagem    | consultar save              | `CharacterService::inspect()`          |
| Personagem    | mudar level                 | `CharacterService::setLevel()`         |
| Personagem    | mudar classe                | `CharacterService::changeClass()`      |
| Personagem    | mudar nick                  | `CharacterService::rename()`           |
| Personagem    | mover para outra conta      | `CharacterService::moveAccount()`      |
| Personagem    | teleportar                  | `CharacterService::teleport()`         |
| Personagem    | restaurar deletado          | `CharacterService::restore()`          |
| Personagem    | restaurar cabelo/model      | `CharacterService::resetAppearance()`  |
| Personagem    | alterar gold                | `CharacterService::setGold()`          |
| Economia      | banco virtual               | `EconomyService::changeBankGold()`     |
| Economia      | coins                       | `CoinService::changeBalance()`         |
| Economia      | consultar coins             | `CoinService::getBalance()`            |
| Clan          | consultar clan              | `ClanAdminService::get()`              |
| Clan          | deletar clan                | `ClanAdminService::deleteClan()`       |
| Clan          | resetar clans               | `ClanAdminService::resetAll()`         |
| Clan          | gold Bless Castle           | `ClanAdminService::setSiegeMoney()`    |
| Clan          | gold SoD                    | `ClanAdminService::setSodMoney()`      |
| Online        | jogadores conectados        | `ServerRuntimeService::players()`      |
| Online        | kick                        | `ServerRuntimeService::kick()`         |
| Online        | zerar CT                    | **não deve existir mais**              |
| SoD           | reset geral                 | `EventAdminService::resetSod()`        |
| SoD           | reset jogador               | `EventAdminService::resetSodPlayer()`  |
| Ranking       | reconstruir ranking         | `RankingService::rebuild()`            |
| Ranking       | resetar ranking             | `RankingService::reset()`              |
| Item          | procurar item no save       | `ItemService::inspectInventory()`      |
| Item          | entregar item               | `ItemGrantService::grant()`            |
| Item          | histórico de entrega        | `ItemGrantService::history()`          |
| Logs          | logs por ID                 | `ItemAuditService::byAccount()`        |
| Logs          | logs por IP                 | `ItemAuditService::byIp()`             |
| Logs          | logs por item               | `ItemAuditService::byItem()`           |
| Config        | editar Hotuk                | `ConfigService`                        |
| Config        | banned.ini                  | `ConfigService`                        |
| Server        | status                      | `ServerControlService::status()`       |
| Server        | iniciar                     | `ServerControlService::start()`        |
| Server        | shutdown                    | `ServerRuntimeService::shutdown()`     |
| Server        | reload Hotuk                | `ServerRuntimeService::reloadConfig()` |

Tem também o sistema `Jeck/premiado`, mas eu classificaria como **feature custom opcional**, não como infraestrutura obrigatória.

---

# 6. AuthService

O painel antigo possui:

```text
ADM.dbo.loginGM

idGM
passGM
nickGM
permissao
```

Não replique isso literalmente.

Crie:

```sql
CREATE TABLE admin_users (
    id              INTEGER PRIMARY KEY,
    username        TEXT NOT NULL UNIQUE,
    password_hash   TEXT NOT NULL,
    display_name    TEXT,
    enabled         INTEGER NOT NULL DEFAULT 1,
    created_at      TEXT NOT NULL
);
```

E:

```sql
CREATE TABLE admin_roles (
    admin_id INTEGER NOT NULL,
    role     TEXT NOT NULL
);
```

Roles que eu sugiro:

```text
Viewer
Support
GM
EconomyAdmin
GameAdmin
SuperAdmin
```

Em vez do misterioso:

```text
permissao = 1
permissao = 2
permissao = 3
```

---

# 7. Não confundir dois sistemas de GM

Hoje existem **dois sistemas diferentes**.

### Painel web

```text
ADM.loginGM
```

### GM dentro do jogo

```text
AccountDB.GameMasters
```

O servidor lê `GameMasters` em:

```text
SrcServer/src/GameMasters/GameMasters.cpp:21-60
```

com:

```text
id
accountId
password
security
active
```

E `reloadGMS;` simplesmente chama novamente:

```cpp
GameMasters::getInstance()->readFromDatabase();
```

Não misture esses conceitos na primeira migração.

Eu faria:

```text
admin_users
    → quem pode usar summit-admin

game_masters
    → quem possui privilégios dentro do jogo
```

Uma pessoa pode ter os dois papéis, mas são permissões diferentes.

---

# 8. AccountService

API:

```cpp
class AccountService
{
public:
    Result<Account> get(AccountId id);

    Result<std::vector<Account>>
        search(AccountSearch query);

    Result<std::vector<CharacterSummary>>
        listCharacters(AccountId id);

    Result<AccountId>
        findByCharacter(CharacterName name);

    Result<void>
        updateProfile(AccountId id,
                      AccountProfile profile);

    Result<void>
        changePassword(AccountId id,
                       SecureString password);

    Result<void>
        deleteAccount(AccountId id);
};
```

---

# 9. Acabe com `AGameUser`, `BGameUser` etc.

O sistema antigo duplica dados:

```text
AllGameUser
AGameUser
BGameUser
CGameUser
...
```

e:

```text
AllPersonalMember
APersonalMember
BPersonalMember
...
```

Isso não deve sobreviver.

SQLite final:

```sql
CREATE TABLE accounts (
    id              INTEGER PRIMARY KEY,
    username        TEXT UNIQUE NOT NULL,
    password_hash   TEXT,
    blocked         INTEGER NOT NULL DEFAULT 0,
    created_at      TEXT,
    updated_at      TEXT
);
```

```sql
CREATE TABLE account_profiles (
    account_id  INTEGER PRIMARY KEY,
    first_name  TEXT,
    last_name   TEXT,
    email       TEXT,
    birth_date  TEXT,
    bank_gold   INTEGER NOT NULL DEFAULT 0
);
```

As tabelas A/B/C... só precisam existir no **importador legado**, não na arquitetura final.

---

# 10. CharacterService

Esse será um dos componentes mais importantes de todo o projeto.

```cpp
class CharacterService
{
public:
    Result<CharacterSnapshot>
        inspect(CharacterName);

    Result<void>
        setLevel(CharacterName, int level);

    Result<void>
        changeClass(CharacterName, CharacterClass);

    Result<void>
        rename(CharacterName oldName,
               CharacterName newName);

    Result<void>
        moveAccount(CharacterName,
                    AccountId target);

    Result<void>
        teleport(CharacterName,
                 FieldId field);

    Result<void>
        setGold(CharacterName, int64_t gold);

    Result<void>
        resetAppearance(CharacterName);

    Result<void>
        restore(CharacterName);
};
```

Mas ele **não conhece offsets**.

---

# 11. CharacterDatCodec

Todo conhecimento do formato legado deve ficar em:

```text
persistence/legacy/
    CharacterDatCodec.h
    CharacterDatCodec.cpp
```

Algo assim:

```cpp
struct CharacterSave
{
    std::string name;
    std::string account;

    CharacterClass characterClass;

    int level;
    int64_t experience;
    int gold;

    FieldId field;

    CharacterStats stats;
    CharacterSkills skills;

    CharacterAppearance appearance;

    std::vector<std::byte> unknownData;
};
```

E:

```cpp
class CharacterDatCodec
{
public:
    Result<CharacterSave>
        decode(std::span<const std::byte>);

    Result<std::vector<std::byte>>
        encode(const CharacterSave&);
};
```

Nunca mais:

```cpp
substr(file, 340, ...)
pack("i", ...)
```

espalhado por 15 funções diferentes.

---

# 12. Offsets legados conhecidos

O painel atual revela pelo menos:

```text
0x010    nome do personagem
0x0C4    classe
0x0C8    level
0x154    gold
0x1E4    field/map
0x2C0    account ID
```

Mas eu **não usaria o PHP como autoridade definitiva**.

Antes de implementar writer, devemos conferir cada campo contra as structs reais do server.

O PHP vira evidência auxiliar.

---

# 13. UserInfoDatCodec

A associação conta → personagens também ainda está em arquivo.

O painel conhece cinco slots:

```text
0x30
0x50
0x70
0x90
0xB0
```

Crie:

```cpp
struct UserInfoSave
{
    std::array<std::string, 5> characterNames;
};
```

com:

```cpp
class UserInfoDatCodec
{
public:
    decode();
    encode();
};
```

---

# 14. Preserve o hash 0..255

Hoje:

```text
nome
 ↓
soma ASCII uppercase
 ↓
mod 256
```

define:

```text
DataServer/userdata/<0..255>/Nick.dat
```

Isso precisa permanecer compatível enquanto o servidor usar essa organização.

Implemente uma única função:

```cpp
uint8_t legacyBucket(std::string_view name);
```

com testes usando nomes existentes.

---

# 15. Escrita segura de saves

Nunca faça:

```text
abre Character.dat
↓
sobrescreve
↓
crash no meio
```

Use:

```text
Character.dat
     ↓
ler
     ↓
validar
     ↓
Character.dat.tmp
     ↓
fsync
     ↓
backup
     ↓
atomic rename
```

Por operação:

```text
DataServer/backups/
    2026-08-30T...
        Character.dat
```

---

# 16. Rename é uma transação

Hoje `mudarnick.inc.php` altera:

```text
userinfo.dat
Character.dat
nome físico do arquivo
diretório hash
```

A nova implementação deve tratar tudo como uma única operação:

```text
BEGIN AdminTransaction

1. verificar nick disponível
2. verificar jogador offline
3. backup character.dat
4. backup userinfo.dat
5. mudar name dentro do save
6. mudar slot da conta
7. calcular bucket novo
8. mover arquivo
9. atualizar character_index
10. gravar audit log

COMMIT
```

Se qualquer etapa falhar:

```text
ROLLBACK
```

---

# 17. Character Lock

Crie um sistema de lock.

Por exemplo:

```text
DataServer/.locks/
    CharacterName.lock
```

ou um lock dentro do próprio processo.

A regra:

```text
servidor ONLINE
→ server possui o lock

admin offline operation
→ precisa adquirir o lock
```

Isso impede edição concorrente.

---

# 18. Character Index

Uma melhoria que o painel antigo não tem e que vai facilitar muito sua vida:

```sql
CREATE TABLE character_index (
    name        TEXT PRIMARY KEY,
    account_id  TEXT NOT NULL,
    class       INTEGER,
    level       INTEGER,
    field       INTEGER,
    gold        INTEGER,
    file_path   TEXT,
    file_mtime  INTEGER,
    file_hash   TEXT
);
```

Isso não é a fonte principal do personagem inicialmente.

É um **índice dos `.dat`**.

Então a UI não precisa escanear:

```text
userdata/0
userdata/1
...
userdata/255
```

toda vez que procurar alguém.

---

# 19. SaveIndexer

Faça um serviço:

```cpp
class SaveIndexer
{
public:
    rebuild();
    rescanChanged();
    onCharacterSaved();
};
```

No Linux você pode futuramente usar `inotify`.

Assim:

```text
Character.dat
     ↓
SaveIndexer
     ↓
character_index
```

E pesquisas ficam instantâneas.

---

# 20. PunishmentService

Não replique `BlockChk + LogsBan` como dois mecanismos sem relação.

Faça:

```sql
CREATE TABLE sanctions (
    id          INTEGER PRIMARY KEY,
    account_id  INTEGER NOT NULL,
    type        TEXT NOT NULL,
    reason      TEXT,
    starts_at   TEXT NOT NULL,
    expires_at  TEXT,
    active      INTEGER NOT NULL,
    actor_id    INTEGER NOT NULL
);
```

Tipos:

```text
ACCOUNT_BAN
CHAT_MUTE
CHARACTER_BAN
```

Se você não quer hardware/MAC ban, não precisa implementar.

---

# 21. EconomyService

Hoje existem pelo menos estes saldos:

```text
Character.dat gold
AllPersonalMember.Gold
CoinShopDB.UserInfo.UserCoin
ClanDB.CL.SiegeMoney
ClanDB.CL.ClanMoney
```

Não tente transformá-los em um único campo.

Crie domínios distintos:

```cpp
CharacterWallet
AccountBank
CoinWallet
ClanSiegeTreasury
ClanSodTreasury
```

Isso evita bugs onde o admin acha que está dando gold do personagem e muda o banco.

---

# 22. CoinService

SQLite:

```sql
CREATE TABLE coin_balances (
    account_id  INTEGER PRIMARY KEY,
    balance     INTEGER NOT NULL DEFAULT 0,
    updated_at  TEXT NOT NULL
);
```

API:

```cpp
getBalance();
credit();
debit();
setBalance();
history();
```

Eu evitaria uma simples função:

```cpp
setCoins()
```

sem motivo.

Use:

```cpp
credit(account, 500, reason);
```

para produzir audit trail.

---

# 23. Ledger de economia

Algo que você não pediu, mas vale muito a pena.

```sql
CREATE TABLE currency_ledger (
    id              INTEGER PRIMARY KEY,
    currency        TEXT NOT NULL,
    owner_type      TEXT NOT NULL,
    owner_id        TEXT NOT NULL,
    delta           INTEGER NOT NULL,
    balance_after   INTEGER,
    reason          TEXT,
    admin_id        INTEGER,
    created_at      TEXT NOT NULL
);
```

Então você consegue saber:

```text
quem deu 50.000 gold
quando
para quem
por quê
saldo antes/depois
```

Isso é muito melhor que simplesmente mudar um inteiro.

---

# 24. ClanAdminService

O sistema novo deveria encapsular:

```text
CL
UL
ClanContent/*.bmp
```

API:

```cpp
getClan();
searchClans();
listMembers();

setSiegeMoney();
setSodMoney();

deleteClan();

changeLeader();
removeMember();

getMark();
setMark();
```

SQLite:

```text
clans
clan_members
clan_marks
```

---

# 25. `ClanDB.CT` deve desaparecer

Hoje `CT` representa jogadores online.

Mas o servidor já sabe perfeitamente quem está online:

```cpp
rsPlayInfo[]
```

A CLI inclusive percorre isso em `CLI.cpp:193-209`.

Logo:

```text
CT
```

é uma duplicação ruim.

Novo sistema:

```text
summit-server memory
        ↓
AdminRuntimeGateway
        ↓
summit-admin
```

Nenhuma tabela SQL de online users.

---

# 26. RankingService

Hoje:

```text
Character DAT
     ↓
multimanager.inc.php
     ↓
rPTDB.LevelList
```

Ou seja, `LevelList` é cache.

Novo sistema pode usar:

```text
character_index
```

e fazer:

```sql
SELECT *
FROM character_index
ORDER BY level DESC
LIMIT 100;
```

Possivelmente `LevelList` deixa de existir.

---

# 27. ItemAuditService

Preserve a semântica dos flags de `ItemLogDB.IL`.

Crie enum:

```cpp
enum class ItemEventType
{
    PickedFromGround = 0,
    TradeReceived    = 2,
    AgingPending     = 4,
    BoughtFromNpc    = 5,
    Dropped          = 6,
    SoldToNpc        = 7,
    TradeSent        = 8,
    Mix              = 9,
    Aging            = 10,
    Matured          = 12,
    PersonalShop     = 22
};
```

Não espalhe:

```cpp
if(flag == 22)
```

pelo projeto.

---

# 28. ItemGrantService

O sistema antigo escreve:

```text
PostBox/<bucket>/<account>.dat
```

com linhas como:

```text
Nick da125 1 "Obrigado por jogar SnowPT!!"
```

Isso pode continuar como backend compatível inicialmente.

Crie:

```cpp
ItemGrantService::grant(
    CharacterName,
    ItemCode,
    ItemClass,
    Message
);
```

Internamente:

```text
LegacyPostBoxRepository
```

Depois você pode substituir pelo sistema nativo do servidor sem mudar a UI.

---

# 29. Não edite inventário diretamente

O distribuidor/PostBox é uma ideia muito melhor que tentar inserir bytes no inventário.

Continue com:

```text
Admin grants item
      ↓
outbox
      ↓
server/player claims
```

Isso reduz corrupção e dupes.

---

# 30. AdminAuditLog

Isso é obrigatório.

Todas as operações de escrita devem gerar:

```sql
CREATE TABLE admin_audit_log (
    id              INTEGER PRIMARY KEY,
    admin_id        INTEGER NOT NULL,
    action          TEXT NOT NULL,
    target_type     TEXT,
    target_id       TEXT,
    before_json     TEXT,
    after_json      TEXT,
    reason          TEXT,
    success         INTEGER NOT NULL,
    created_at      TEXT NOT NULL
);
```

Exemplo:

```text
GM Dimas
CHANGE_CHARACTER_LEVEL
target = Pikeman01
before = 86
after = 100
reason = "teste"
```

Não apenas bans e item grants.

**Tudo.**

---

# 31. ConfigService

Nunca replique o antigo:

```text
textarea
→ escreve hotuk.ini inteiro
```

Faça parser tipado:

```cpp
struct ServerConfig
{
    Rates rates;
    BossConfig bosses;
    CoinConfig coins;
    NetworkConfig network;
    ...
};
```

UI:

```text
EXP Rate       [ 2.0 ]
Drop Rate      [ 1.0 ]
Boss Minute    [ 30 ]
...
```

Fluxo:

```text
load
→ parse
→ validate
→ modify
→ backup
→ write
→ ask server reload
```

Deixe um **Raw Editor** apenas em modo avançado.

---

# 32. ServerControlService

O painel antigo usa:

```text
WScript.Shell
→ Server.exe
```

Não recrie isso.

No Linux:

```text
systemd
```

deve ser responsável pelo processo.

Por exemplo:

```text
summit-server.service
```

O admin consulta:

```text
active
inactive
failed
PID
uptime
```

E solicita:

```text
start
stop
restart
```

Idealmente via D-Bus/systemd, e não:

```cpp
system("systemctl ...");
```

---

# 33. Controle runtime ≠ controle de processo

São dois conceitos.

```text
ServerRuntimeService
```

é para:

```text
kick
broadcast
reload
shutdown gracioso
online players
```

Enquanto:

```text
ServerProcessService
```

é:

```text
start process
hard stop
restart
service status
logs
```

Separe.

---

# 34. IPC entre summit-admin e summit-server

Como você quer evitar webserver, eu usaria:

### Linux

```text
Unix Domain Socket
/run/summit/admin.sock
```

### Windows durante transição

```text
Named Pipe
\\.\pipe\summit-admin
```

Protocolo interno:

```text
length
version
request_id
command
payload
```

Você pode usar JSON inicialmente:

```json
{
  "version": 1,
  "command": "kick_player",
  "request_id": 481,
  "payload": {
    "character": "Player01",
    "reason": "admin"
  }
}
```

Não existe motivo para amarrar esse protocolo ao protocolo binário Game.exe ↔ Server.

---

# 35. Command Queue no servidor

Outro ponto importante.

A thread de IPC administrativa **não deve mexer diretamente em `rsPlayInfo[]`**.

Faça:

```text
Admin IPC thread
      ↓
AdminCommandQueue
      ↓
server/game thread
      ↓
executa alteração
      ↓
AdminResponse
```

Isso evita race condition.

---

# 36. Frontend recomendado

Eu dividiria a aplicação em abas:

```text
Dashboard

Players Online

Accounts
Characters
Punishments

Economy
    Gold
    Bank
    Coins

Items
    Grant
    Audit

Clans

Events
    SoD
    Bless Castle

Ranking

Game Masters

Server
    status
    config
    logs

Backups

Audit Log
```

---

# 37. Dashboard novo

Coisas que o painel atual não mostra direito e seriam muito úteis:

```text
Server uptime
players online
peak online today

SQLite health
last backup
last save error

accounts
characters

banned accounts

coins issued today
gold issued today

items granted today

server tick latency
network connections

last admin actions
```

---

# 38. BackupService

Isso deve ser criado antes de permitir funções destrutivas.

Precisa salvar:

```text
summit.db

DataServer/
PostBox/
ClanContent/
Configs/
```

E um manifest:

```json
{
  "timestamp": "...",
  "server_build": "...",
  "schema_version": 7,
  "git_commit": "...",
  "files": [...]
}
```

---

# 39. SQLite backup

Não faça simplesmente:

```text
cp summit.db summit-backup.db
```

durante atividade.

Use a API de backup do SQLite ou:

```sql
VACUUM INTO
```

em condições apropriadas.

---

# 40. Backup antes de operações destrutivas

Especialmente:

```text
delete account
delete clan
rename character
move character
restore
change class
ranking reset
SoD reset
```

Eu colocaria:

```text
automatic snapshot
```

ou pelo menos backup dos registros/arquivos afetados.

---

# 41. Undo

Outra feature que vale muito.

Algumas operações podem ser reversíveis:

```text
set level
set gold
teleport
coin adjustment
clan treasury adjustment
profile change
```

Se você guarda:

```text
before_json
after_json
```

no audit log, pode implementar:

```text
REVERT
```

mais tarde.

---

# 42. Não implemente "zerar logs"

O painel atual pode:

```text
DELETE FROM ItemLogDB.IL
```

Isso é péssimo para auditoria.

Novo sistema:

```text
archive
retention
vacuum
```

Mas administradores normais **não apagam audit logs**.

---

# 43. Senhas

Duas correções imediatas.

O painel antigo exibe senhas de contas.

**Não implemente isso.**

E `GameMasters::print()` hoje faz em:

```text
SrcServer/src/GameMasters/GameMasters.cpp:104-116
```

algo equivalente a:

```text
AccountId: ...
Password: ...
Security: ...
```

Remova também.

---

# 44. Admin password

Para `summit-admin`, use hash forte, por exemplo:

```text
Argon2id
```

via libsodium.

Nunca:

```text
passGM = plaintext
```

---

# 45. Senha do jogador

Essa pode ser migrada gradualmente.

O Game.exe ainda manda a senha ao servidor.

O servidor pode fazer:

```text
senha recebida
    ↓
Argon2 verify
    ↓
accounts.password_hash
```

sem necessariamente mudar o protocolo do client.

Mas eu faria isso em uma etapa separada da migração administrativa.

---

# 46. Online/offline é uma regra de domínio

Toda API de personagem deveria declarar explicitamente:

```cpp
enum class MutationPolicy
{
    OfflineOnly,
    RuntimeIfOnline,
    ForceDisconnect
};
```

Exemplo:

```cpp
changeClass(
    player,
    Knight,
    MutationPolicy::ForceDisconnect
);
```

Isso evita decisões implícitas.

---

# 47. Algumas operações deveriam continuar offline-only

Eu manteria inicialmente:

```text
rename
change class
move account
restore deleted
```

como:

```text
OFFLINE ONLY
```

São operações grandes demais para fazer live no primeiro release.

Já:

```text
kick
teleport
gold
coins
broadcast
```

podem facilmente ser runtime.

---

# 48. Validação forte

Antes de executar:

```text
setLevel(999999)
```

a camada de domínio precisa recusar.

Não dependa da UI.

```cpp
Result<void>
CharacterService::setLevel(...)
{
    if(level < 1 || level > MAX_LEVEL)
        return InvalidArgument;
}
```

O mesmo para:

```text
gold
coins
class
field ID
item code
nick
account ID
clan money
```

---

# 49. Não aceite item code arbitrário

Antes de:

```text
grantItem("da125")
```

valide contra o catálogo real de itens.

Isso é especialmente importante porque o painel antigo só aceita uma string.

Crie:

```cpp
ItemCatalog
```

com:

```text
code
name
category
class
valid
```

---

# 50. Confirmation tokens

Operações perigosas devem exigir confirmação explícita.

Exemplo:

```text
DELETE ACCOUNT
"joao123"

Characters:
 - Fighter01
 - Archer01

Esta operação...
```

e exigir digitar:

```text
joao123
```

ou equivalente.

---

# 51. Migração: a ordem que eu usaria

| Fase    | Objetivo                                      |
| ------- | --------------------------------------------- |
| **M0**  | congelar e documentar comportamento PHP atual |
| **M1**  | criar codecs read-only para `.dat`            |
| **M2**  | criar SQLite novo + migrations                |
| **M3**  | criar AdminCore read-only                     |
| **M4**  | criar UI C++ read-only                        |
| **M5**  | implementar auth/RBAC/audit                   |
| **M6**  | migrar contas e bans                          |
| **M7**  | migrar coins/economia/clans                   |
| **M8**  | criar Runtime Admin Gateway                   |
| **M9**  | implementar character mutations offline       |
| **M10** | implementar item grant/logs                   |
| **M11** | config/server control                         |
| **M12** | backups/restore                               |
| **M13** | retirar `padm/` completamente                 |

---

# 52. M0 — testes primeiro

Antes de reimplementar qualquer coisa, escolha contas/personagens de teste e registre:

```text
account lookup
character list
level
class
gold
map
nickname
coins
clan
rank
item grant
ban
```

Faça snapshots.

Esses viram seus **golden tests**.

---

# 53. M1 — decoder antes de encoder

Primeiro:

```text
Character.dat
→ C++ struct
```

sem modificar nada.

Compare a saída com o painel antigo.

Depois:

```text
C++ struct
→ Character.dat
```

e teste:

```text
original
→ decode
→ encode
→ binary compare
```

Idealmente round-trip byte-exato nas áreas que você não modifica.

---

# 54. Preserve bytes desconhecidos

Muito importante.

No primeiro codec:

```cpp
struct CharacterSave
{
    ...
    std::vector<std::byte> originalBuffer;
};
```

Quando mudar level:

```text
copie arquivo original
↓
altere somente campos conhecidos
↓
preserve todos os bytes desconhecidos
```

Não tente reconstruir o save inteiro do zero agora.

---

# 55. Teste contra o próprio servidor

Além de comparar bytes:

```text
C++ modifica save
↓
summit-server carrega
↓
Game.exe entra
↓
verificar personagem
↓
deslogar
↓
server salvar
↓
ler novamente
```

Esse é o teste definitivo.

---

# 56. M2 — SQLite schema versionado

Crie:

```sql
schema_migrations
```

por exemplo:

```text
001_admin.sql
002_accounts.sql
003_sanctions.sql
004_clans.sql
005_coins.sql
006_audit.sql
007_character_index.sql
...
```

Nunca altere o banco manualmente em produção.

---

# 57. Importador SQL Server → SQLite

Crie uma ferramenta separada:

```text
summit-migrate
```

Fluxo:

```text
SQL Server
↓
read-only export
↓
normalization
↓
SQLite transaction
↓
validation report
```

E compare:

```text
accounts count
clans count
members count
coin totals
bans
game masters
```

---

# 58. Dual-run temporário

Durante migração, você pode manter:

```text
servidor antigo
+
admin novo read-only
```

Depois:

```text
admin novo writes SQLite
+
servidor migrado para SQLite
```

Só então desligar o SQL Server.

Evite dual-write SQL Server + SQLite por muito tempo.

---

# 59. O que eu NÃO portaria do painel antigo

Eu eliminaria:

```text
ALL CONTAS mostrando senhas

DELETE ALL ITEM LOGS

DELETE ALL DISTRIBUTOR LOGS

zerar CT

WScript.Shell

edição raw obrigatória de Hotuk

SQL queries montadas por string

anti_sql()

senhas hardcoded

AGameUser/BGameUser/... final

Jeck aleatório, a menos que você queira a feature

COM

ODBC
```

---

# 60. Coisas que você não pediu, mas considero essenciais

As mais importantes são:

**1. Índice de personagens.** Sem isso, a nova UI vai continuar dependendo de escanear milhares de `.dat`.

**2. Auditoria universal.** Toda alteração de administrador deve ser registrada.

**3. Backup automático.** Antes de qualquer operação destrutiva.

**4. Controle de concorrência.** Não editar personagem online pelo filesystem.

**5. Operações atômicas.** Rename/move/restore tocam múltiplos arquivos.

**6. Versionamento de save.** `CharacterDatCodecV1`, e não um codec “eterno”.

**7. Schema migrations.** SQLite precisa de evolução controlada.

**8. Separar source-of-truth de caches.** Ranking e online list não deveriam ser bancos primários.

**9. Health monitoring.** O admin deve saber se save thread, SQLite e server tick estão saudáveis.

**10. Testes automatizados de administração.** Principalmente rename/move/class change.

---

# 61. Minha proposta de árvore de código

```text
SourceCode/
│
├── Apps/
│   ├── Server/
│   │   └── main.cpp
│   │
│   ├── Admin/
│   │   └── main.cpp
│   │
│   └── Migrate/
│       └── main.cpp
│
├── Admin/
│   ├── Auth/
│   │   ├── AuthService.h
│   │   ├── AuthorizationService.h
│   │   └── Roles.h
│   │
│   ├── Accounts/
│   │   └── AccountService.*
│   │
│   ├── Characters/
│   │   └── CharacterService.*
│   │
│   ├── Clans/
│   │   └── ClanAdminService.*
│   │
│   ├── Economy/
│   │   └── EconomyService.*
│   │
│   ├── Items/
│   │   ├── ItemGrantService.*
│   │   └── ItemAuditService.*
│   │
│   ├── Events/
│   │   └── EventAdminService.*
│   │
│   ├── Config/
│   │   └── ConfigService.*
│   │
│   ├── Runtime/
│   │   ├── AdminCommand.*
│   │   ├── AdminCommandQueue.*
│   │   └── AdminRuntimeGateway.*
│   │
│   └── Audit/
│       └── AuditService.*
│
├── Persistence/
│   ├── SQLite/
│   └── Legacy/
│       ├── CharacterDatCodec.*
│       ├── UserInfoDatCodec.*
│       ├── PostBoxCodec.*
│       └── LegacyBucket.*
│
└── Shared/
    └── AdminProtocol/
```

---

# 62. Critério para desligar o PHP

Eu só apagaria `padm/` quando a seguinte matriz estivesse 100% verde:

```text
Conta
  buscar               OK
  editar               OK
  senha                OK
  ban                   OK
  delete                OK

Character
  inspect               OK
  level                 OK
  class                 OK
  nick                  OK
  move account          OK
  teleport              OK
  gold                  OK
  restore               OK

Economy
  bank                  OK
  coins                 OK
  clan money            OK

Clan
  lookup                OK
  members               OK
  delete                OK

Item
  search                OK
  grant                 OK
  logs                  OK

Events
  SoD reset             OK
  rankings              OK

Server
  online players        OK
  kick                  OK
  reload                OK
  shutdown              OK
  start                 OK

Safety
  RBAC                  OK
  audit                 OK
  backup                OK
  rollback              OK
```

A melhor primeira implementação não é a UI. É **`CharacterDatCodec + UserInfoDatCodec + AdminCore read-only`**. Quando isso estiver confiável, a UI C++ vira relativamente simples; e quando o `AdminRuntimeGateway` estiver dentro do servidor, você finalmente terá um sistema administrativo que conhece a diferença entre **estado persistido no disco e estado vivo na memória**, algo que o painel PHP antigo nunca teve.

