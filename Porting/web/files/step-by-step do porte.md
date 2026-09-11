Pelo que aparece no `wwwroot` e no plano arquitetural que você já levantou, esses “dois sites” não são propriamente sites necessários ao jogo. Eles formam uma camada de serviço + administração em cima do SQL Server. A parte que o cliente realmente precisa pode ser inteiramente absorvida pelo servidor C++.

Seu documento já aponta a arquitetura correta: manter a UI de clã no cliente C++, usar o socket normal do jogo, colocar um `ClanService` no servidor e persistir em `data/summit.db`, removendo IIS/PHP/ASP/ASPX/WinInet. 

## O que o `wwwroot` realmente implementa

Eu encontrei quatro grupos diferentes dentro do pacote.

**1. Backend operacional de clã usado pelo jogo.** É a parte que precisa obrigatoriamente ser reimplementada se você quiser eliminar HTTP. Seu levantamento anterior identificou chamadas para criação, consulta, entrada, saída, kick, sub-chefe, permissões e clan mark. O mapeamento dos endpoints legados para funções nativas já está parcialmente documentado. 

**2. Painel web de clã do jogador.** O diretório `painel/clan/` duplica várias dessas operações: criação de clã, alteração de frase, escolha de vice-líder, transferência de liderança, exclusão do clã e visualização/kick de membros. Isso não precisa continuar sendo um site se todas essas operações forem colocadas no cliente C++.

**3. Painel administrativo `padm/`.** Ele é bem mais amplo que clãs. Tem administração de contas, bans, créditos, ouro, logs, alteração de personagem, teleport, classe, nickname, SOD, clan money, SiegeMoney etc. Isso não é necessário para o funcionamento normal do cliente, mas você precisa decidir se quer também substituí-lo por comandos/admin tools em C++.

**4. Portal público.** Rankings, status do servidor, Bless Castle, Bellatra/SOD, screenshots, downloads etc. Isso é puramente apresentação. Se o objetivo for “zero webserver”, você pode simplesmente abandonar essas páginas ou criar futuramente outra ferramenta independente. Elas não devem bloquear a migração do game server.

Além disso, `ClanContent/` é basicamente storage de clan marks em BMP. Seu próprio plano já chegou à solução que considero apropriada: `data/clan_marks/<clan_id>.bmp`, mantendo no SQLite apenas metadados como filename/hash/data de atualização. 

---

# Checklist de reimplementação

Eu organizaria o trabalho assim.

### Fase 1 — Congelar o comportamento legado

* [ ] Não começar apagando PHP/ASP.
* [ ] Catalogar todos os métodos públicos de `ISAOCHECK`.
* [ ] Para cada método, registrar parâmetros de entrada.
* [ ] Registrar formato exato da resposta HTTP atualmente interpretada pelo cliente.
* [ ] Registrar códigos de erro (`Code=1`, `Code=2`, `Code=3`, `Code=100` etc.).
* [ ] Registrar efeitos colaterais no banco.
* [ ] Registrar quais métodos alteram mais de uma tabela.
* [ ] Registrar quais operações exigem transação.
* [ ] Criar testes de comportamento antes da substituição.

Isso é especialmente importante porque o cliente atual recebe e utiliza os dois hosts de clã via configuração do servidor, e o código legado está espalhado entre `isaocheck`, configuração e protocolo. 

---

### Fase 2 — Criar o núcleo SQLite

* [ ] Adicionar `sqlite3` ao build do servidor.
* [ ] Criar `SQLiteConnection`.
* [ ] Criar `SQLiteStatement`.
* [ ] Implementar RAII para `sqlite3*`.
* [ ] Implementar RAII para `sqlite3_stmt*`.
* [ ] Implementar `prepare`.
* [ ] Implementar bind de inteiro.
* [ ] Bind de `int64`.
* [ ] Bind de string.
* [ ] Bind de blob.
* [ ] Bind de NULL.
* [ ] `step()`.
* [ ] `reset()`.
* [ ] leitura de colunas.
* [ ] transações.
* [ ] tratamento uniforme de erros.
* [ ] logging do erro SQLite + query.
* [ ] aplicar `PRAGMA foreign_keys=ON`.
* [ ] aplicar WAL.
* [ ] aplicar `busy_timeout`.
* [ ] criar controle de versão do schema.
* [ ] criar migrations automáticas.

Na primeira fase, vale preservar a interface pública de `SQLConnection` e trocar apenas sua implementação interna; isso reduz enormemente o tamanho do diff. Essa estratégia também está documentada no plano. 

---

### Fase 3 — Unificar os antigos bancos

Crie inicialmente:

```text
data/
    summit.db
    clan_marks/
```

E mantenha temporariamente:

```cpp
DATABASEID_AccountDB
DATABASEID_ClanDB
DATABASEID_ServerDB
DATABASEID_CoinShopDB
DATABASEID_QuestDB
...
```

mas todos resolvendo para `summit.db`.

* [ ] Criar `DatabaseRouter`.
* [ ] Mapear todos os `DATABASEID_*`.
* [ ] Fazer `SQLConnection::GetConnection()` continuar funcionando.
* [ ] Adaptar também o wrapper antigo `SQL.cpp`.
* [ ] Remover queries com `USE Database`.
* [ ] Remover nomes `database.dbo.Table`.
* [ ] Converter `TOP n` → `LIMIT n`.
* [ ] converter `GETDATE()`.
* [ ] converter `DATEADD`.
* [ ] converter `DAY/MONTH/YEAR`.
* [ ] converter `SET x += 1`.
* [ ] substituir `IF NOT EXISTS...INSERT`.
* [ ] revisar COLLATION/case sensitivity.
* [ ] revisar campos identity/autoincrement.
* [ ] revisar defaults.
* [ ] revisar índices.
* [ ] revisar constraints UNIQUE.

Seu plano já identifica que a source revela bastante do schema, mas não prova índices, triggers, defaults e constraints do SQL Server antigo; portanto, se você ainda tiver o banco original, exportar o schema é muito melhor do que tentar inferi-lo apenas pelas queries. 

---

# Fase 4 — Schema de clã

Pelos scripts que inspecionei, o núcleo antigo gira principalmente em torno de:

```text
CL
UL
CT
LI
```

Eu não manteria esses nomes na arquitetura final, mas manteria na primeira migração para facilitar comparação.

Depois você pode evoluir para algo como:

```text
clans
clan_members
clan_online_members   // talvez nem precise persistir
clan_marks
clan_permissions
```

O registro de clã precisa preservar, pelo menos semanticamente:

```text
ClanName
UserID
ClanZang
MemCnt
Note
MIconCnt
RegiDate
LimitDate
DelActive
PFlag
KFlag
Flag
NoteCnt
Cpoint
CWin
CFail
ClanMoney
CNFlag
SiegeMoney
```

E membros atualmente carregam coisas equivalentes a:

```text
clan id
user id
character name
clan name
class/type
level
permission
join date
flags
mark id
```

Não normalize isso agressivamente no primeiro commit. Primeiro reproduza o comportamento antigo; normalize depois.

---

# Fase 5 — `ClanRepository`

Crie uma camada que **não conheça socket nem UI**:

```cpp
class ClanRepository
{
public:
    std::optional<Clan> FindById(...);
    std::optional<Clan> FindByName(...);
    std::optional<Clan> FindByLeader(...);

    bool ClanNameExists(...);
    bool IsMember(...);

    std::vector<ClanMember> ListMembers(...);

    ClanId Create(...);
    void Disband(...);

    void AddMember(...);
    void RemoveMember(...);

    void UpdateLeader(...);
    void UpdateSubChief(...);

    void UpdateNote(...);
    void UpdateMark(...);

    void UpdateWarehousePermission(...);
    void UpdateFlags(...);
};
```

Checklist:

* [ ] `FindClanByName`.
* [ ] `FindClanById`.
* [ ] `FindClanByLeader`.
* [ ] `FindMember`.
* [ ] `FindMemberByCharacter`.
* [ ] `ListMembers`.
* [ ] `CountMembers`.
* [ ] `ClanNameExists`.
* [ ] `CharacterHasClan`.
* [ ] `CreateClan`.
* [ ] `DeleteClan`.
* [ ] `InsertMember`.
* [ ] `DeleteMember`.
* [ ] `ChangeLeader`.
* [ ] `SetSubChief`.
* [ ] `RemoveSubChief`.
* [ ] `UpdateNote`.
* [ ] `UpdateMark`.
* [ ] `UpdateMemberPermission`.
* [ ] `UpdatePFlag`.
* [ ] `UpdateKFlag`.
* [ ] `UpdateClanMoney`.
* [ ] `UpdateSiegeMoney`.
* [ ] `UpdateSodPoints`.

---

# Fase 6 — `ClanService`: regras de negócio

Aqui fica aquilo que hoje está espalhado pelos PHP/ASP.

## Criação de clã

No `wwwroot` atual, `ClanInsert.aspx`, `ClanInsert.php`, `verificarcl.asp` e `verificarcl.php` são essencialmente duplicatas.

A operação faz aproximadamente:

```text
validar parâmetros
↓
confirmar que a chamada é válida
↓
verificar se personagem já pertence a clã
↓
verificar se nome já existe
↓
reservar novo número de imagem/clã
↓
INSERT CL
↓
INSERT UL líder
↓
commit
```

Checklist:

* [ ] validar usuário.
* [ ] validar personagem.
* [ ] verificar se personagem pertence ao usuário.
* [ ] verificar se personagem já tem clã.
* [ ] validar nome.
* [ ] verificar unicidade do nome.
* [ ] aplicar comprimento máximo.
* [ ] aplicar charset permitido.
* [ ] impedir nomes reservados se necessário.
* [ ] gerar `clan_id`.
* [ ] não depender de contador `LI` se SQLite puder usar ID próprio.
* [ ] criar clã e líder na mesma transação.
* [ ] rollback se a criação do membro falhar.
* [ ] retornar código equivalente ao cliente.
* [ ] definir corretamente dinheiro inicial.
* [ ] atualizar cache de clã do personagem.

Vale notar que o script atualmente retorna, no sucesso de criação:

```text
Code=1
CMoney=500000
```

mesmo gravando `ClanMoney=0` no `INSERT`. Isso é exatamente o tipo de peculiaridade que você deve decidir conscientemente se preserva ou corrige.

---

## Consulta de clã

* [ ] `GetClanInfo`.
* [ ] lookup por nome.
* [ ] lookup por número/id.
* [ ] retornar líder.
* [ ] retornar quantidade de membros.
* [ ] retornar clan mark.
* [ ] retornar nota/frase.
* [ ] retornar flags.
* [ ] retornar pontos SOD.
* [ ] retornar vitórias/derrotas.
* [ ] retornar clan money.
* [ ] retornar SiegeMoney se utilizado.
* [ ] retornar status do personagem naquele clã.

---

## Membership

* [ ] `MembershipStatus`.
* [ ] `JoinClan`.
* [ ] `LeaveClan`.
* [ ] `KickMember`.
* [ ] impedir join duplicado.
* [ ] impedir personagem em dois clãs.
* [ ] atualizar `MemCnt`.
* [ ] garantir que `MemCnt` não seja a única fonte de verdade.
* [ ] preferencialmente calcular count pela tabela de membros.
* [ ] validar limite máximo de membros.
* [ ] validar autoridade de quem executa kick.
* [ ] impedir kick do líder por caminho comum.
* [ ] notificar personagem afetado se online.

---

# Fase 7 — Hierarquia e permissões

O webroot confirma que existe pelo menos uma convenção:

```text
Permi = 0 -> membro normal
Permi = 2 -> vice/sub-chief
```

Você precisa transformar isso em enum:

```cpp
enum class ClanRole : uint8_t
{
    Member,
    // confirmar demais valores
    SubChief,
    Leader
};
```

Não inventaria os demais valores até verificar a source.

Checklist:

* [ ] `AppointSubChief`.
* [ ] `ReleaseSubChief`.
* [ ] garantir apenas um sub-chief se essa for a regra antiga.
* [ ] limpar o sub-chief anterior ao nomear novo.
* [ ] validar líder.
* [ ] `TransferLeader`.
* [ ] copiar `UserID` corretamente para o novo líder.
* [ ] atualizar `ClanZang`.
* [ ] atualizar roles dos membros.
* [ ] proteger a operação em transação.
* [ ] informar clientes conectados.

O próprio plano identifica transferência/leave, sub-chief e warehouse permission entre as operações que precisam ser substituídas. 

---

# Fase 8 — Dissolução do clã

Hoje há código web fazendo essencialmente:

```sql
DELETE CL WHERE ClanName=...
DELETE UL WHERE ClanName=...
```

e em uma das variantes grava também um log de dissolução em outro banco.

Faça isso corretamente no service:

* [ ] somente líder pode dissolver.
* [ ] iniciar transação.
* [ ] capturar membros afetados.
* [ ] limpar memberships.
* [ ] limpar permissões.
* [ ] remover registro do clã.
* [ ] lidar com clan mark.
* [ ] lidar com ClanMoney.
* [ ] lidar com SiegeMoney.
* [ ] lidar com Bless Castle ownership.
* [ ] lidar com SOD/ranking.
* [ ] criar audit log.
* [ ] commit.
* [ ] invalidar caches.
* [ ] atualizar membros online.

---

# Fase 9 — Clan mark

Eu faria exatamente o modelo do seu plano:

```text
data/clan_marks/1000001.bmp
```

SQLite:

```sql
CREATE TABLE clan_marks (
    clan_id      INTEGER PRIMARY KEY,
    filename     TEXT NOT NULL,
    hash         TEXT NOT NULL,
    updated_at   INTEGER NOT NULL
);
```

Checklist:

* [ ] `UploadClanMark`.
* [ ] `GetClanMark`.
* [ ] validar existência do clã.
* [ ] validar que quem envia é líder/autorizado.
* [ ] tamanho máximo.
* [ ] validar header BMP.
* [ ] validar dimensões.
* [ ] impedir path traversal.
* [ ] filename gerado pelo servidor, jamais fornecido pelo cliente.
* [ ] gravar arquivo atomicamente.
* [ ] calcular hash.
* [ ] persistir hash.
* [ ] resposta contendo versão/hash.
* [ ] cliente manter cache.
* [ ] cliente não baixar novamente se hash igual.
* [ ] transferência fragmentada se exceder tamanho seguro do packet.

O plano também chama atenção para não assumir que um BMP de ~64 KiB pode simplesmente ser colocado em qualquer pacote existente; o limite do protocolo precisa ser confirmado antes. 

---

# Fase 10 — Protocolo C++ de clã

Você já tem uma vantagem enorme: há `OPCODE_CLAN_SERVICE`, `OPCODE_CLAN_UPDATE` e `OPCODE_CLANMONEY`, e o game client/server já passam tráfego de clã pelo socket normal. 

Eu criaria:

```text
Shared/protocol/clan/
    ClanProtocol.h
    ClanPackets.h
```

E comandos semelhantes a:

```cpp
enum class ClanCommand : uint16_t
{
    GetInfo,
    CheckName,
    Create,
    Disband,

    GetMembership,
    Join,
    Leave,
    Kick,

    ListMembers,

    AppointSubChief,
    ReleaseSubChief,
    TransferLeader,

    SetNote,
    SetWarehousePermission,
    SetFlag,

    GetMarkInfo,
    GetMark,
    UploadMark
};
```

Checklist de wire protocol:

* [ ] `uint8_t/uint16_t/uint32_t/int32_t`.
* [ ] nunca `long`.
* [ ] nunca `size_t`.
* [ ] nunca ponteiro.
* [ ] nunca `std::string` no wire.
* [ ] strings com arrays fixos ou length + bytes.
* [ ] `size`.
* [ ] `opcode`.
* [ ] `protocol_version`.
* [ ] `command`.
* [ ] `request_id`.
* [ ] `status`.
* [ ] limite de payload.
* [ ] static assertions de tamanho.
* [ ] endianess documentada.
* [ ] respostas determinísticas.
* [ ] timeout no cliente.
* [ ] tratamento de resposta fora de ordem.

---

# Fase 11 — `ClanProtocolHandler`

O handler deveria ser fino:

```text
packet
↓
validar tamanho
↓
validar sessão
↓
obter player autenticado
↓
ClanService
↓
montar response
```

Não coloque SQL nele.

* [ ] validar pacote antes de cast.
* [ ] validar versão.
* [ ] validar command.
* [ ] associar request ao jogador autenticado.
* [ ] **não confiar em UserID enviado pelo cliente**.
* [ ] não confiar em nome de personagem arbitrário.
* [ ] extrair identidade da sessão do game server.
* [ ] chamar `ClanService`.
* [ ] mapear erros para `ClanStatus`.
* [ ] enviar response.

Esse ponto é importante porque o PHP antigo aceita vários valores diretamente por request HTTP. Quando você migrar para o socket, aproveite para fazer o servidor ser autoritativo.

---

# Fase 12 — Substituir `ISAOCHECK`

Esse é o corte do HTTP no cliente.

Crie algo como:

```text
SrcGame/src/TJBOY/clanmenu/
    ClanClientGateway.h
    ClanClientGateway.cpp
```

O plano já recomenda exatamente essa separação: o gateway substitui as chamadas HTTP do `ISAOCHECK` por requests do socket. 

Checklist:

* [ ] `isCheckClanJang` → request nativo.
* [ ] `isCheckClanName`.
* [ ] `ReadClanInfo`.
* [ ] `isCheckClanMember`.
* [ ] `isClanMember`.
* [ ] `joinClanMember`.
* [ ] `makeClan`.
* [ ] `read_ClanName`.
* [ ] `read_ClanData`.
* [ ] `BreakUPClan`.
* [ ] `Banishiment`.
* [ ] `AppointSubChip`.
* [ ] `ReleaseSubChip`.
* [ ] `LeaveClan`.
* [ ] `Secession`.
* [ ] `isKPFlag`.
* [ ] `isPFlag`.
* [ ] `WarhouseRight`.
* [ ] `updateFlag`.
* [ ] `write_ClanImageFile`.
* [ ] `read_ClanImage`.
* [ ] `read_ClanImage2`.

Seu levantamento original mostra exatamente esses endpoints e confirma que `hostName` concentra a maioria das operações enquanto `hostName1` era sobretudo criação + download de mark. 

Depois:

* [ ] nenhum `InternetOpen`.
* [ ] nenhum `InternetConnect`.
* [ ] nenhum `HttpOpenRequest`.
* [ ] nenhum parser de resposta ASP/PHP.
* [ ] remover `hostName`.
* [ ] remover `hostName1`.
* [ ] remover configuração IP1/IP2 de clan server.
* [ ] remover WinInet do linker se não tiver outro uso.

---

# Fase 13 — O que fazer com `clancustom.php`

Esse arquivo é um mini painel independente que atualmente:

```text
faz login usando AccountDB
↓
confirma se UserID é líder em ClanDB
↓
permite trocar MIconCnt
↓
permite trocar Note
```

Port nativo:

* [ ] remover login web separado.
* [ ] usar sessão já autenticada do game.
* [ ] criar tela de personalização dentro de `clanmenu`.
* [ ] `SetClanNote`.
* [ ] `SetClanMark`.
* [ ] retornar catálogo de marks predefinidos se ainda quiser isso.
* [ ] ou permitir upload do BMP pelo próprio cliente.
* [ ] validar liderança no servidor, não no cliente.

Assim você elimina inclusive a necessidade de armazenar/validar senha do jogador em uma segunda interface.

---

# Fase 14 — Funcionalidades extras do painel de clã

O `painel/clan/` traz funcionalidades que não aparecem necessariamente como endpoint direto do `ISAOCHECK`, mas que você pode querer preservar:

* [ ] alteração da frase/`Note`.
* [ ] seleção de vice-líder.
* [ ] troca do vice-líder anterior.
* [ ] transferência de liderança.
* [ ] dissolução.
* [ ] listagem ordenada por data de entrada.
* [ ] indicação de membro online.
* [ ] kick em lote ou individual.
* [ ] sincronização de `MemCnt`.
* [ ] exibição da clan mark.
* [ ] data de criação.

Eu colocaria tudo isso no `ClanService`, mesmo que inicialmente a UI C++ não exponha todos os comandos.

---

# Fase 15 — ClanMoney / SOD / Bless Castle

Não trate isso como “detalhe do site”.

O `wwwroot` consulta e altera:

```text
ClanMoney
SiegeMoney
Cpoint
CWin
```

e existem páginas de Bless Castle/Bellatra e funções administrativas que mexem nisso.

Checklist:

* [ ] descobrir quem é fonte de verdade para `ClanMoney`.
* [ ] localizar usos de `OPCODE_CLANMONEY`.
* [ ] localizar `ClanMoney()` vindo de `Clan.dll`.
* [ ] reimplementar depósito.
* [ ] retirada.
* [ ] validação de saldo.
* [ ] limite máximo.
* [ ] transação.
* [ ] audit log.
* [ ] SiegeMoney.
* [ ] reward de Bless Castle.
* [ ] owner de Bless Castle.
* [ ] Cpoint/SOD.
* [ ] CWin/CFail.
* [ ] reset periódico se houver.
* [ ] ranking pós-restart.

Seu teste final obrigatoriamente deve cobrir Clan Money, Bless Castle e SoD, e isso também está registrado no plano atual. 

---

# Fase 16 — Eliminar `Clan.dll`

Isso é diferente de eliminar os sites.

Faça inventário de cada `GetProcAddress` relacionado a Clan.

Para cada export:

```text
nome
assinatura
call sites
efeito no banco
efeito no personagem
efeito no protocolo
```

Então:

* [ ] reimplementar em C++.
* [ ] substituir call sites.
* [ ] criar teste equivalente.
* [ ] remover `GetProcAddress`.
* [ ] remover `LoadLibrary("Clan")`.
* [ ] remover DLL do deployment.

Seu levantamento confirma que a arquitetura atual ainda carrega `Clan.dll` e `Database.dll` dinamicamente, portanto migrar apenas o site e `SQLConnection` não elimina a dependência antiga. 

---

# Fase 17 — Eliminar `Database.dll`

Mesma metodologia:

* [ ] `SQLLogon`.
* [ ] `SQLLogout`.
* [ ] `CharLog`.
* [ ] `GameLog`.
* [ ] `SetGameUser`.
* [ ] `GetGPCode`.
* [ ] `ClanMoney`.
* [ ] `EventJoin`.
* [ ] `PollRecord`.
* [ ] `ClanUserTicket`.
* [ ] `ClanPointInsert`.
* [ ] todas as demais exports realmente encontradas.
* [ ] mover lógica para services/repositories.
* [ ] substituir log SQL por SQLite/file logger conforme caso.
* [ ] remover COM/OLE DB.
* [ ] remover `CoInitializeEx` caso não exista outro usuário de COM.
* [ ] remover `LoadLibrary("Database")`.

---

# Fase 18 — decidir o destino do painel administrativo

Aqui eu faria uma separação clara.

**Não bloqueia remover os dois sites de clã:**

```text
padm/
painel/
rank100.php
rank100c.php
scripts/servidorup/
```

Você pode primeiro desligar apenas o backend HTTP usado pelo jogo.

Depois, se seu objetivo final for **zero PHP/web**, crie:

```text
AdminService
AdminCLI
```

ou uma ferramenta C++ separada.

Funções do `padm` que identifiquei e que precisam de substituto apenas se você ainda as usa:

* administração de contas;
* ban IP/MAC/conta;
* créditos;
* ouro;
* ClanMoney;
* SiegeMoney;
* reset SOD/rank;
* remoção de clã;
* logs;
* alteração de nick;
* alteração de classe;
* teleport;
* character recovery;
* procura de item;
* manipulação de personagens;
* permissões de GM.

Eu **não colocaria essas funções no protocolo normal do cliente**. Elas deveriam ficar numa CLI local ou canal administrativo autenticado.

---

# Fase 19 — portal público

Você pode simplesmente deixar morrer:

```text
rank100.php
rank100c.php
scripts/servidorup/status.php
bellatra.php
blesscastle.php
Artworks.php
Downloads.php
ScreenShots.php
Videos.php
Wallpapers.php
```

Se futuramente quiser ranking/status, melhor gerar:

```text
status.json
ranking.json
```

ou criar uma pequena API read-only separada. Não deixe o game server depender disso.

---

# Fase 20 — migração dos dados

A ordem que eu usaria:

```text
SQL Server
   ↓
export
   ↓
SQLite staging
   ↓
validation
   ↓
summit.db
```

* [ ] exportar schema original.
* [ ] exportar dados.
* [ ] preservar IDs.
* [ ] preservar datas.
* [ ] preservar nomes exatamente.
* [ ] preservar clan ids/MIconCnt.
* [ ] copiar todos os BMP válidos.
* [ ] detectar marks órfãos.
* [ ] detectar clans sem mark.
* [ ] detectar UL sem CL.
* [ ] detectar CL com `MemCnt` diferente do count real.
* [ ] detectar personagem em múltiplos clans.
* [ ] detectar líderes ausentes de UL.
* [ ] detectar dois sub-chiefs se isso não for permitido.
* [ ] detectar nomes duplicados com diferença apenas de case.
* [ ] validar ClanMoney.
* [ ] validar SiegeMoney.
* [ ] validar Cpoint.
* [ ] comparar contagens SQL Server ↔ SQLite.
* [ ] gerar relatório de inconsistências.
* [ ] só depois desligar writes no SQL Server.

O plano já recomenda migration por etapas, feature flag `web|native` durante a transição e remoção das DLLs apenas depois de equivalência comprovada. 

---

# Ordem que eu realmente implementaria

Se quiser minimizar risco, eu faria nessa sequência:

```text
1. SQLiteConnection
2. migrations + summit.db
3. SQLConnection -> SQLite
4. ClanRepository
5. ClanService read-only
6. GetClanInfo / ListMembers / MembershipStatus
7. novo protocolo client/server
8. CreateClan
9. Join / Leave / Kick
10. sub-chief / transfer leader
11. disband
12. Note / flags / permissions
13. Clan mark
14. ClanMoney / SOD / Bless Castle
15. remover ISAOCHECK HTTP
16. remover hostName/hostName1
17. remover Clan.dll
18. remover Database.dll
19. remover ODBC
20. apagar IIS/PHP/ASP/ASPX
```

A razão para começar por operações **read-only** é simples: você consegue testar protocolo, serialização, lookup e SQLite sem correr risco de corromper o estado do clã.

## Definição de “acabou”

Eu só consideraria a migração concluída quando estas condições forem verdadeiras:

* [ ] `Game.exe` não faz nenhuma requisição HTTP relacionada a clã.
* [ ] não existe `hostName/hostName1` funcionalmente necessário.
* [ ] `*CLAN_SERVER` não é mais necessário.
* [ ] nenhum clan mark é baixado por URL.
* [ ] nenhuma operação de clã passa por PHP/ASP/ASPX.
* [ ] nenhum `odbc_connect` é necessário ao game server.
* [ ] nenhum `LoadLibrary("Clan")`.
* [ ] nenhum `LoadLibrary("Database")`.
* [ ] nenhum OLE DB/COM necessário à persistência.
* [ ] todos os dados necessários estão em `summit.db` + `data/clan_marks`.
* [ ] criar/join/leave/kick/sub-chief/leader/disband funcionam após restart.
* [ ] ClanMoney/SOD/Bless Castle sobrevivem restart corretamente.
* [ ] cliente antigo apresenta o mesmo comportamento visível.
* [ ] você consegue apagar o `wwwroot` inteiro e o jogo continua funcionando.

Isso é praticamente a mesma definição de conclusão já estabelecida no seu plano: zero WinInet/HTTP de clã, zero `Clan.dll`/`Database.dll`, um único `summit.db` versionado e todos os fluxos de clã passando pelo protocolo do jogo. 

O ponto mais importante que a inspeção do `wwwroot` acrescenta é: **não tente “portar o site para C++”.** Extraia dele as regras de negócio e descarte a arquitetura web. `ClanService + ClanRepository + ClanProtocolHandler + ClanClientGateway` cobre a parte necessária. O resto do `wwwroot` deve ser tratado separadamente como ferramentas administrativas ou conteúdo público.

