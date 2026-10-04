# Auditoria inicial do código C++

Base analisada: relatórios `includes-*`, `lifecycle-*`, `opcodes.txt`, `databases.txt` e `system-candidates.txt` enviados no pacote.

> Importante: este é um mapa **estático por evidência textual**. A presença de um arquivo nas árvores `SrcGame/src` ou `SrcServer/src` não prova que ele entra no binário. A próxima fase deve auditar os arquivos realmente compilados por cada target.

## 1. Visão geral

- `includes-game.txt`: 2.041 ocorrências de include em 366 arquivos observados.
- `includes-server.txt`: 1.793 ocorrências de include em 301 arquivos observados.
- `lifecycle-game.txt`: 2.821 ocorrências de `Init/Load/Main/Draw/...`.
- `lifecycle-server.txt`: 2.536 ocorrências.
- `opcodes.txt`: 2.316 ocorrências; 288 nomes de opcode distintos detectados.
- 282 dos 288 opcodes aparecem nos dois lados nos relatórios.
- Nos caminhos observados por todos os relatórios: 418 caminhos relativos do Game, 343 do Server e 308 em comum. Isso reforça a existência de uma grande superfície legacy compartilhada/copied.

## 2. Composition root do client

O arquivo mais útil encontrado para mapear sistemas do client é:

`SourceCode/SrcGame/src/sinbaram/sinMain.cpp`

### Inicialização

Linhas 35–80 inicializam diretamente:

- `cItem`
- `cSkill`
- `cInvenTory`
- `cCharStatus`
- `cInterFace`
- `cShop`
- `cTrade`
- `cMessageBox`
- `cSin3D`
- `cSinHelp`
- `cSinWarpGate`
- `cHelpPet`
- `cSinSod2`
- `cCoinShop_T`
- `cCoinShop`
- `cQuestWindow`
- `cRestaure`
- `cKill`
- `cPCBANGPet`
- `cSettings`
- `cTarget`
- `WarMode`
- `Rank`
- `cMixGuide`
- `ClanCustom`
- `NPCinfo`
- `BattlePass`
- `JOBS`

### Loop

Linhas 118–143 chamam `Main()` para vários dos mesmos componentes, incluindo item, interface, skill, inventory, shop, trade, warp, help pet, SoD2, siege, quest, premium item e PCBang pet.

### Render/UI

Linhas 165–321 chamam `Draw()` para status, party, quest, skill, inventory, shop, trade, interface, CoinShop, restore, item, warp, SoD2, siege, quest, settings, kill, rank, BattlePass, NPCinfo, ClanCustom e JOBS.

### Shutdown

Linhas 620–634 fecham inventory, status, interface, item, skill, shop, trade, message box, Sin3D, help, warp, HelpPet, SoD2 e PCBangPet.

**Conclusão:** `sinbaram/sinMain.cpp` é o melhor índice inicial do runtime do client.

## 3. Sistemas identificados

| Sistema | Client | Server | Evidência/entrada principal |
|---|---|---|---|
| Core client/game loop | sim | parcialmente copiado | `sinbaram/sinMain.cpp`, `playmain.cpp`, `playsub.cpp` |
| Items/Inventory | sim | sim/shared | `sinItem`, `sinInvenTory`; chamados em `sinMain` |
| Skills | sim | sim/shared | `cSkill`, `SkillSub`, `sinSkill_*` |
| Shop/Trade | sim | sim/shared | `cShop`, `cTrade`, `sinShop`, `sinTrade` |
| CoinShop | sim | sim | client `Init/Draw`; server `CoinShop.cpp`, `CoinShop_T.cpp`, `CoinShopDB` |
| Quests | sim | sim | client `cQuestWindow/chaQuest`; server `cQuestHandler.cpp`, `QuestDB` |
| Pets | sim | cópia presente | `LowLevelPetSystem`, `PCBangPetSystem` |
| Effects/Particles | sim | cópia presente | `HoBaram`, `NewEffect`, `HoEffectManager` |
| Clan | sim | cópia presente | `TJBOY/clanmenu` + opcodes de clan |
| Party | sim | sim/shared | `HoParty`, `cPartyWindow`, opcodes de party |
| Protocol/Network | sim | sim | `opcodes.h`, `smwsock`, `smPacket`, `netplay` |
| Database | não como núcleo | sim | `Database/Database.*`, `SQLConnection` |
| Game Masters | — | sim | `GameMasters/GameMasters.cpp`, `AccountDB` |
| Restore/Security persistence | UI no client | sim | `cRestaureHandler.cpp`, `SecurityDB` |
| News/Rank/VIP/alerts | partes client | sim | `News`, `Rank`, `cVipHandler`, `Alerta`, `ServerDB` |
| Anti-cheat/security client | sim | integração server | `AntiCheat/`, `Security/`, opcodes/protocolo |
| 3D/rendering | sim | cópia presente | `smLib3d`, `HoBaram` |
| Localization | sim | cópia presente | `Language/` |

## 4. Pet System

### Low-level pet

`SourceCode/SrcGame/src/HoBaram/LowLevelPetSystem.cpp`

- `HoLowLevelPetManager::Create()` — linha 705
- `Destroy()` — 720
- `Draw()` — 884
- `Main()` — 895
- wrappers globais `g_CreatePetSystem`, `g_MainPetSystem`, `g_DrawPetSystem`, `g_DestroyPetSystem` — ~912–932

### PCBang pet

`SourceCode/SrcGame/src/HoBaram/PCBangPetSystem.cpp`

- `HoPCBangPetManager::Create()` — 740
- `Destroy()` — 762
- `Draw()` — 946
- `Main()` — 957
- wrappers globais — ~976–994

É um subsistema com lifecycle explícito e fácil de delimitar.

## 5. Effects / particles

Os arquivos com mais ocorrências no relatório de lifecycle são dominados por efeitos:

### Client

- `HoBaram/HoEffect.cpp`: 455 ocorrências
- `HoBaram/hoAssaParticleEffect.cpp`: 301
- `HoBaram/AssaParticle.cpp`: 277
- `HoBaram/NewEffect/HoEffectManager.cpp`: 53

### Server

- `HoBaram/HoEffect.cpp`: 445
- `HoBaram/hoAssaParticleEffect.cpp`: 297
- `HoBaram/AssaParticle.cpp`: 262
- `HoBaram/NewEffect/HoEffectManager.cpp`: 53

Isso não prova que o server execute renderização; prova que existe forte duplicação/superfície compartilhada desses fontes. Verificar os targets de build é obrigatório.

## 6. Rede e protocolo

Foram encontrados 288 opcodes distintos nos relatórios, dos quais 282 aparecem tanto no Game quanto no Server.

Grupos identificáveis pelo nome:

- CoinShop: 4
- Quest: 7
- Clan: 6
- Party: 10
- Skill: 10
- Trade: 6
- Warehouse: 2
- Item: 47
- Pet: 5
- VIP: 2
- Ranking: 2
- Shop/comércio: 17
- Attack: 8
- Damage: 8

Isso torna o protocolo uma ótima segunda dimensão da auditoria: para cada sistema, mapear os opcodes usados e os arquivos que enviam/recebem cada um.

Quase toda a superfície é compartilhada. As exceções observadas foram:

- apenas Game: `OPCODE_ENCODE_PACKET2`
- apenas Server: `OPCODE_NULL`, `OPCODE_END`, `OPCODE_CONTINUEDATA`, `OPCODE_SET_OBJSERIAL`, `OPCODE_FUNCTON_RESULT`

## 7. Database

O enum contém 18 IDs concretos além de `DATABASEID_NULL`:

- AccountDB
- BillingDB
- BillingLogDB
- BattlePass
- ClanDB
- GameLogDB
- ItemLogDB
- LogDB
- PCRoom
- PCRoomLog
- rPTDB
- SoD2DB
- SodDB
- SecurityDB
- ServerDB
- CoinShopDB
- QuestDB
- StartUserDB

`Database.cpp` cria conexões para praticamente todos os IDs por `CreateSQLConnection(...)`.

### Anomalia: BattlePass

`DATABASEID_BattlePass` aparece:

- `Database.h:17`
- `Database.cpp:49` no mapa nome -> ID

Mas **não aparece** no relatório em `CreateSQLConnection(DATABASEID_BattlePass)` nem em `SQLConnection::GetConnection(DATABASEID_BattlePass)`.

Classificação: **revisar**. Pode ser enum legado, feature incompleta, acesso por outra camada ou conexão ausente.

### Consumidores claros

- `GameMasters` -> `AccountDB`
- `CoinShop` / `CoinShop_T` -> `CoinShopDB`
- `cQuestHandler` -> `QuestDB`
- `cRestaureHandler` -> `SecurityDB`
- `News`, `Rank`, `cVipHandler`, `Alerta` -> `ServerDB`
- `Server/OnSever.cpp` -> `StartUserDB`, `ServerDB`, `SecurityDB`, `AccountDB`, `ClanDB`

O startup do banco aparece em `SrcServer/src/Winmain.cpp:287` via `openDatabase()`.

## 8. Headers centrais

Top includes do Game (ainda com aliases relativos não completamente resolvidos):

1. `windows.h` — 80
2. `sinbaram/sinLinkHeader.h` — 71
3. `Language/language.h` — 58
4. `opcodes.h` — 34
5. `smLib3d/smd3d.h` — 30
6. `HoBaram/HoLinkHeader.h` — 28
7. `Settings/Settings.h` — 25
8. `field.h` + variantes relativas — alta presença
9. `smwsock.h` — 19
10. `character.h`, `playmain.h`, `netplay.h` — hubs relevantes

Server:

1. `cstring` — 79
2. `windows.h` — 73
3. `sinbaram/sinLinkHeader.h` — 57
4. `Language/language.h` — 56
5. `smLib3d/smd3d.h` — 36
6. `HoBaram/HoLinkHeader.h` — 26
7. `Server/onserver.h` — 21
8. `opcodes.h` — 21
9. `smwsock.h` — 20
10. `Database/Database.h` — 12

### Atenção à normalização

Ainda existem entradas como:

- `field.h`
- `../field.h`
- `sinbaram/sinLinkHeader.h`
- `sinLinkHeader.h`
- `../sinbaram/sinLinkHeader.h`

Logo, a contagem atual **subestima hubs**. A próxima etapa deve resolver o include relativo ao diretório do arquivo chamador e produzir um caminho canônico.

## 9. Separação client/server

Nos caminhos observados pelos relatórios:

### Mais característicos do client

- `AntiCheat/`
- `BattleEvents/`
- `ConfirmationBox/`
- `Hook/`
- `Login/`
- `Security/`
- `Settings/`
- diversos `UI*.cpp/.h`
- janelas (`cQuestWindow`, `cSettingsWindow`, `cPartyWindow`, etc.)

### Mais característicos do server

- `CLI/`
- `Database/`
- `Firewall/`
- `GameMasters/`
- `News/`
- `PacketFilter.*`
- `cQuestHandler.cpp`
- `cRestaureHandler.cpp`
- `cVipHandler.cpp`
- `server_build_compat.h`
- `server_compat_packets.h`

### Grande área duplicada/compartilhada

- `HoBaram/`
- `sinbaram/`
- `TJBOY/`
- `smLib3d/`
- `Language/`
- partes de `Server/`

## 10. Próxima fase recomendada

A maior incerteza agora é **o que realmente é compilado**.

A próxima coleta deve responder:

1. Quais `.cpp` entram no executável Game?
2. Quais `.cpp` entram no executável Server?
3. Quais arquivos são apenas cópias mortas/legadas?
4. Qual é o ponto de entrada real de cada target?
5. Para cada opcode: quem envia e quem recebe?
6. Para cada sistema: qual banco ele acessa?

Só depois disso vale considerar o mapa “arquitetura real” e não apenas “arquivos presentes”.
