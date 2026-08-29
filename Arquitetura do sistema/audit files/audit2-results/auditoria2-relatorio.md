# Auditoria C++ — segunda coleta (`audit2`)

## Resumo executivo

- Inventário: **617 arquivos** em `SrcGame/src` e **503 arquivos** em `SrcServer/src`.
- Há **464 caminhos relativos presentes nas duas árvores**, **153 exclusivos do client** e **39 exclusivos do server**.
- Considerando unidades de implementação (`.cpp/.c/.cc/.cxx`): **159** têm caminho correspondente nos dois lados, **78** aparecem só no client e **15** só no server.
- Grafo de includes: **3190 arestas**, sendo **3011 resolvidas** para caminhos canônicos e **179 não resolvidas**.
- Protocolo: `opcode-names.txt` contém **290 nomes OPCODE_ distintos coletados**.
- Banco: existem **18 IDs de banco reais + `DATABASEID_NULL`**. **17** aparecem em `CreateSQLConnection(...)`; `DATABASEID_BattlePass` é declarado/mapeado, mas não aparece criado nem obtido nesta coleta.

> Importante: `files-game.txt`/`files-server.txt` são inventários físicos. Esta coleta **não contém um manifesto de build**, então “presente na árvore” ainda não equivale a “compilado no executável”.

## Composition roots confirmados

### Client
`SrcGame/src/Winmain.cpp` contém `WinMain(...)`. O principal agregador de subsistemas encontrado é `SrcGame/src/sinbaram/sinMain.cpp`.

Em `sinMain.cpp`, a fase de inicialização chama diretamente: `cItem`, `cSkill`, `cInvenTory`, `cCharStatus`, `cInterFace`, `cShop`, `cTrade`, `cMessageBox`, `cSin3D`, `cSinHelp`, `cSinWarpGate`, `cHelpPet`, `cSinSod2`, `cCoinShop_T`, `cCoinShop`, `cQuestWindow`, `cKill`, `cPCBANGPet`, `cSettings`, `cTarget`, `WarMode`, `Rank`, `cMixGuide`, `ClanCustom`, `NPCinfo`, `BattlePass` e `JOBS`. O mesmo arquivo concentra grande parte de `Main()`, `Draw()` e `Close()`.

### Server
`SrcServer/src/Winmain.cpp` contém `main(...)` e chama `openDatabase()`. `SrcServer/src/Server/OnSever.cpp` concentra ciclo de vida de `STG_AREA`, personagens, `ROLLHANDLER`, `cHg`, handlers de pacote e grande parte do acesso a bancos.

## Mapa inicial de sistemas

| Sistema | Client | Server | Evidência principal |
|---|---|---|---|
| Core / game loop | `Winmain.cpp`, `playmain`, `playsub`, `sinMain.cpp` | `Winmain.cpp`, `Server/OnSever.cpp` | entrypoints + `Main/Draw/Close` |
| Rede / protocolo | `smwsock`, `netplay`, `opcodes.h`, `Shared/smPacket.h` | `smwsock`, `netplay`, `Server/OnSever.cpp`, `opcodes.h` | hubs de include + 290 nomes de opcode |
| UI / inventário / skills | `sinbaram/sinMain.cpp`, `sinbaram/*`, `UI*`, `Settings/*` | muita estrutura espelhada em `sinbaram/*` | ciclo `Init/Main/Draw/Close` no client |
| Effects / particles | `HoBaram/*`, `HoBaram/NewEffect/*` | estrutura correspondente em `HoBaram/*` | `HoEffectManager`, `HoNewParticleMgr`, `smRender.Init()` |
| Pets | `LowLevelPetSystem`, `PCBangPetSystem` | mesmos caminhos também existem | `Create/Main/Draw/Destroy` |
| CoinShop | `CoinShop`, `CoinShop_T`, UI em `sinMain/netplay` | `CoinShop`, `CoinShop_T`, `OnSever` | `DATABASEID_CoinShopDB` + handlers/opcodes |
| Quests | `cQuestWindow`, `HaQuest` | `cQuestHandler`, `HaQuest` | `DATABASEID_QuestDB` no server |
| Rank | `Rank`, `Ranking` | `Rank.cpp` | server usa `DATABASEID_ServerDB` |
| BattlePass | `BattlePass.cpp`, inicializado em `sinMain` | `BattlePass.cpp` existe | `DATABASEID_BattlePass` está órfão no layer SQL |
| Clans | `TJBOY/clanmenu`, `ClanCustom` | `TJBOY/clanmenu` + DB em `OnSever` | `DATABASEID_ClanDB` |
| Database | — | `Database/Database.*` | `openDatabase`, `SQLConnection::GetConnection` |
| Admin / operação | — | `CLI`, `GameMasters`, `News`, `Alerta` | arquivos server-only |
| Firewall / packet filtering | — | `Firewall`, `PacketFilter` | arquivos server-only |
| Anti-cheat / security | `AntiCheat`, `Security`, XTrap/nProtect refs | `SecurityDB` + compat/security code | include/protocolo + DB |
| Rendering / 3D | `smLib3d`, Direct3D, `HoBaram` | grande espelho legado em `smLib3d/HoBaram` | hubs `smd3d.h`, render init |
| Localization | `Language/*` | `Language/*` | `language.h` é 2º maior hub do client |

## Hubs de include (fan-in)

| Referências | Header |
|---:|---|
| 151 | `SourceCode/SrcGame/src/sinbaram/sinLinkHeader.h` |
| 100 | `SourceCode/SrcGame/src/Language/language.h` |
| 73 | `SourceCode/SrcServer/src/sinbaram/sinLinkHeader.h` |
| 67 | `SourceCode/SrcGame/src/smLib3d/smd3d.h` |
| 63 | `SourceCode/SrcGame/src/HoBaram/HoLinkHeader.h` |
| 52 | `SourceCode/SrcGame/src/field.h` |
| 47 | `SourceCode/SrcGame/src/TJBOY/clanmenu/tjclan.h` |
| 41 | `SourceCode/SrcServer/src/smLib3d/smd3d.h` |
| 39 | `SourceCode/SrcGame/src/HoBaram/HoTextFile.h` |
| 38 | `SourceCode/SrcGame/src/TJBOY/clanmenu/tjclanDEF.h` |
| 37 | `SourceCode/SrcServer/src/field.h` |
| 35 | `SourceCode/SrcServer/src/opcodes.h` |
| 34 | `SourceCode/SrcGame/src/opcodes.h` |
| 34 | `SourceCode/SrcGame/src/smwsock.h` |
| 29 | `SourceCode/SrcGame/src/character.h` |
| 29 | `SourceCode/SrcGame/src/Library/LuaSrc/Lua/lobject.h` |
| 28 | `SourceCode/SrcGame/src/TJBOY/clanmenu/cE_CViewClanInfo.h` |
| 27 | `SourceCode/SrcGame/src/Library/LuaSrc/Lua/lua.h` |
| 26 | `SourceCode/SrcGame/src/Settings/Settings.h` |
| 26 | `SourceCode/SrcGame/src/playmain.h` |

Os dois agregadores mais críticos são `sinbaram/sinLinkHeader.h` e `HoBaram/HoLinkHeader.h`. Eles funcionam como *umbrella headers* e escondem acoplamento transversal; devem ser tratados como fronteiras arquiteturais durante a auditoria, não como sistemas em si.

## Maiores fan-outs

| Includes diretos | Arquivo |
|---:|---|
| 56 | `SourceCode/SrcGame/src/sinbaram/sinLinkHeader.h` |
| 51 | `SourceCode/SrcServer/src/Server/OnSever.cpp` |
| 50 | `SourceCode/SrcServer/src/sinbaram/sinLinkHeader.h` |
| 45 | `SourceCode/SrcGame/src/Winmain.bak2.cpp` |
| 45 | `SourceCode/SrcGame/src/Winmain.cpp` |
| 45 | `SourceCode/SrcGame/src/netplay.cpp` |
| 41 | `SourceCode/SrcServer/src/TJBOY/clanmenu/tjclan.cpp` |
| 40 | `SourceCode/SrcGame/src/TJBOY/clanmenu/tjclan.cpp` |
| 40 | `SourceCode/SrcServer/src/Winmain.cpp` |
| 36 | `SourceCode/SrcGame/src/playsub.cpp` |
| 33 | `SourceCode/SrcServer/src/playsub.cpp` |
| 32 | `SourceCode/SrcServer/src/netplay.cpp` |
| 31 | `SourceCode/SrcGame/src/Server/OnSever.cpp` |
| 30 | `SourceCode/SrcGame/src/playmain.cpp` |
| 28 | `SourceCode/SrcServer/src/playmain.cpp` |

`SrcServer/src/Server/OnSever.cpp` é o maior fan-out do server na coleta (51 includes diretos), reforçando que é um hotspot de orquestração/acoplamento. `SrcGame/src/Winmain.cpp` e `netplay.cpp` têm 45 cada.

## Bancos

| ID | CreateSQLConnection | GetConnection explícito | Leitura |
|---|---:|---:|---|
| `DATABASEID_AccountDB` | 1 | 5 | ativo |
| `DATABASEID_BattlePass` | 0 | 0 | **anomalia: declarado, não criado/não usado** |
| `DATABASEID_BillingDB` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_BillingLogDB` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_ClanDB` | 1 | 2 | ativo |
| `DATABASEID_CoinShopDB` | 1 | 10 | ativo |
| `DATABASEID_GameLogDB` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_ItemLogDB` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_LogDB` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_NULL` | 0 | 0 | sentinela |
| `DATABASEID_PCRoom` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_PCRoomLog` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_QuestDB` | 1 | 9 | ativo |
| `DATABASEID_SecurityDB` | 1 | 10 | ativo |
| `DATABASEID_ServerDB` | 1 | 20 | ativo |
| `DATABASEID_SoD2DB` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_SodDB` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |
| `DATABASEID_StartUserDB` | 1 | 1 | ativo |
| `DATABASEID_rPTDB` | 1 | 0 | pool criado; nenhum consumidor `GetConnection` encontrado nesta coleta |

## Arquivos server-only mais importantes

- `Alerta.cpp`
- `CLI/CLI.cpp`
- `Database/Database.cpp`
- `Firewall/Firewall.cpp`
- `GameMasters/GameMasters.cpp`
- `Mouse/Mouse.cpp`
- `News/News.cpp`
- `PacketFilter.cpp`
- `Server/CRollHandler.cpp`
- `Server/SvrCaravan.cpp`
- `Skills/Skills.cpp`
- `cQuestHandler.cpp`
- `cRestaureHandler.cpp`
- `cVipHandler.cpp`
- `sinbaram/SkillTables.cpp`

Esses 15 `.cpp` são candidatos fortes a responsabilidades genuinamente server-side. A existência exclusiva na árvore é evidência estrutural, ainda não evidência de linkagem.

## Client-only — grupos relevantes

- **AntiCheat**: `AntiCheat/AntiCheat.cpp`
- **BattleEvents**: `BattleEvents/CRollHandler.cpp`, `BattleEvents/CRollWindow.cpp`
- **ConfirmationBox**: `ConfirmationBox/ConfirmationBox.cpp`
- **Font**: `Font/Font.cpp`, `Font/newFont.cpp`
- **Hook**: `Hook/Hook.cpp`
- **Library/LuaSrc**: `Library/LuaSrc/Lua/lapi.c`, `Library/LuaSrc/Lua/lcode.c`, `Library/LuaSrc/Lua/ldebug.c`, `Library/LuaSrc/Lua/ldo.c`, `Library/LuaSrc/Lua/ldump.c`, `Library/LuaSrc/Lua/lfunc.c`, `Library/LuaSrc/Lua/lgc.c`, `Library/LuaSrc/Lua/lib/lauxlib.c`, `Library/LuaSrc/Lua/lib/lbaselib.c`, `Library/LuaSrc/Lua/lib/ldblib.c`, `Library/LuaSrc/Lua/lib/liolib.c`, `Library/LuaSrc/Lua/lib/lmathlib.c` …
- **Login**: `Login/Login.cpp`
- **PvP**: `PvP/Score.cpp`
- **Security**: `Security/Security.cpp`
- **Settings**: `Settings/Settings.cpp`, `Settings/UI.cpp`
- **Raiz do client**: `CChatBox.cpp`, `CLootHandler.cpp`, `DrawDamage.cpp`, `KillHandler.cpp`, `NPCinfo.cpp`, `Ranking.cpp`, `ShowDamage.cpp`, `TCore.cpp`, `UIButton.cpp`, `UICheckbox.cpp`, `UICombobox.cpp`, `UIHorizontalScroll.cpp`, `UIImage.cpp`, `UIInput.cpp`, `UIItemFrame.cpp`, `UIList.cpp`, `UIListItem.cpp`, `UIText.cpp`, `UITitle.cpp`, `UIVerticalScroll.cpp`, `UIWindow.cpp`, `WarMode.cpp`, `Winmain.bak2.cpp`, `cMixGuide.cpp`, `cPartyWindow.cpp`, `cQuestWindow.cpp`, `cRestaureWindow.cpp`, `cSettingsWindow.cpp`, `cTarget.cpp`, `client_server_stubs.cpp`, `d3dx_font_alias.cpp`, `mingw_debug_entry.cpp`, `stdafx.cpp`

## Includes ainda não resolvidos

Faltam resolver **179 ocorrências** em **58 grafias**. Os maiores grupos:

- 16 × `../HoBaram/HoTextFile.h`
- 12 × `File/File.h`
- 11 × `sinLinkHeader.h`
- 10 × `lua.h`
- 10 × `lauxlib.h`
- 8 × `lualib.h`
- 6 × `Language/Taiwan/T_sinCraftItem.h`
- 4 × `windows.h`
- 4 × `Language/chinese/C_sinCraftItem.h`
- 4 × `Language/English/E_sinCraftItem.h`
- 4 × `Language/ARGENTINA/A_sinCraftItem.h`
- 4 × `../XTrapSrcD5/ClientExt/XTrap4ClientExt1.h`
- 3 × `Helper/common.h`
- 3 × `Helper/Utils/utils_logging.h`
- 2 × `Lua/src/lua.h`

Parte disso é header de sistema/SDK, mas há includes locais (`File/File.h`, `sinLinkHeader.h`, `../HoBaram/HoTextFile.h`) que indicam que o resolvedor ainda precisa conhecer mais include roots (`-I`) e/ou busca case-insensitive compatível com o histórico Windows.

## O que falta para responder “o que realmente compila?”

Precisamos de pelo menos uma destas fontes: `compile_commands.json`, `build.ninja`, `Makefile*`, `CMakeLists.txt`, ou projetos Visual Studio (`*.vcxproj/*.vcproj`). O próximo script (`fedora-audit-build.sh`) coleta esses manifests e extrai unidades de compilação usando apenas `find`, `grep`, `sed`, `awk`, `sort` e utilitários GNU.

Depois disso, a classificação final será: **CLIENT COMPILED**, **SERVER COMPILED**, **SHARED COMPILED**, **PRESENT BUT NOT BUILT** e **UNRESOLVED/GENERATED**.