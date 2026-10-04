# Auditoria de compilação — client/server

## Escopo

Classificação derivada dos dois scripts de build fornecidos e cruzada com o inventário `audit2`. Os números abaixo cobrem as árvores `SrcGame/src` e `SrcServer/src`; os scripts também selecionam `modules/MD5` no client e `modules` no server, que não estavam no inventário `audit2` usado para a contagem.

## Resultado

- Client: 196 translation units selecionadas em `SrcGame/src`.
- Server: 174 translation units selecionadas em `SrcServer/src`.
- Mesmo caminho relativo compilado dos dois lados: 153.
- Compiladas apenas no client: 43.
- Compiladas apenas no server: 21.

Os scripts usam `-c`, portanto produzem `.o` e não fazem o link final. “Compilado” aqui significa translation unit transformada em objeto; não prova por si só que o objeto entra no executável final.

## Fronteiras confirmadas

- Client: `-D_CLIENT`, C++17, fontes de `SrcGame/src`, mais `modules/MD5`.
- Server: `-D_SERVER`, C++14, fontes de `SrcServer/src`, mais `modules`; `SrcGame/src` aparece apenas como include path.
- O client exclui explicitamente implementações server-side em `SrcGame/src/Server`: `ClientFuncPos.cpp`, `OnSever.cpp`, `PartySever.cpp`, `Svr_Damge.cpp`, `svrEventSpawn.cpp`.
- O server compila essas implementações em `SrcServer/src/Server`.
- `smLib3d/FilterEffect.cpp` é excluído do client, mas compilado no server.
- `AntiCheat/AntiCheat.cpp` é compilado no client; o filtro do server remove caminhos contendo `AntiCheat`.

## Server-only compilado

- `Alerta.cpp`
- `CLI/CLI.cpp`
- `Database/Database.cpp`
- `Firewall/Firewall.cpp`
- `GameMasters/GameMasters.cpp`
- `Mouse/Mouse.cpp`
- `News/News.cpp`
- `PacketFilter.cpp`
- `Server/CRollHandler.cpp`
- `Server/ClientFuncPos.cpp`
- `Server/OnSever.cpp`
- `Server/PartySever.cpp`
- `Server/SvrCaravan.cpp`
- `Server/Svr_Damge.cpp`
- `Server/svrEventSpawn.cpp`
- `Skills/Skills.cpp`
- `cQuestHandler.cpp`
- `cRestaureHandler.cpp`
- `cVipHandler.cpp`
- `sinbaram/SkillTables.cpp`
- `smLib3d/FilterEffect.cpp`

## Client-only compilado

- `AntiCheat/AntiCheat.cpp`
- `BattleEvents/CRollHandler.cpp`
- `BattleEvents/CRollWindow.cpp`
- `CLootHandler.cpp`
- `ConfirmationBox/ConfirmationBox.cpp`
- `DrawDamage.cpp`
- `Font/Font.cpp`
- `Font/newFont.cpp`
- `Hook/Hook.cpp`
- `KillHandler.cpp`
- `Login/Login.cpp`
- `NPCinfo.cpp`
- `PvP/Score.cpp`
- `Ranking.cpp`
- `Security/Security.cpp`
- `Settings/Settings.cpp`
- `Settings/UI.cpp`
- `ShowDamage.cpp`
- `TCore.cpp`
- `UIButton.cpp`
- `UICheckbox.cpp`
- `UICombobox.cpp`
- `UIHorizontalScroll.cpp`
- `UIImage.cpp`
- `UIInput.cpp`
- `UIItemFrame.cpp`
- `UIList.cpp`
- `UIListItem.cpp`
- `UIText.cpp`
- `UITitle.cpp`
- `UIVerticalScroll.cpp`
- `UIWindow.cpp`
- `WarMode.cpp`
- `cMixGuide.cpp`
- `cPartyWindow.cpp`
- `cQuestWindow.cpp`
- `cRestaureWindow.cpp`
- `cSettingsWindow.cpp`
- `cTarget.cpp`
- `client_server_stubs.cpp`
- `d3dx_font_alias.cpp`
- `mingw_debug_entry.cpp`
- `stdafx.cpp`

## Sistemas — evidência de compilação

### Core/loop

- client: 7 unidades relacionadas
- server: 7 unidades relacionadas
- mesmo caminho relativo dos dois lados (nome completo ainda com prefixo diferente): 7

### Network/protocol

- client: 3 unidades relacionadas
- server: 7 unidades relacionadas
- mesmo caminho relativo dos dois lados (nome completo ainda com prefixo diferente): 3

### Effects/render

- client: 37 unidades relacionadas
- server: 34 unidades relacionadas
- mesmo caminho relativo dos dois lados (nome completo ainda com prefixo diferente): 33

### UI/client

- client: 25 unidades relacionadas
- server: 1 unidades relacionadas
- mesmo caminho relativo dos dois lados (nome completo ainda com prefixo diferente): 1

### Gameplay

- client: 25 unidades relacionadas
- server: 20 unidades relacionadas
- mesmo caminho relativo dos dois lados (nome completo ainda com prefixo diferente): 17

### Commerce

- client: 4 unidades relacionadas
- server: 4 unidades relacionadas
- mesmo caminho relativo dos dois lados (nome completo ainda com prefixo diferente): 4

### Database/server

- client: 0 unidades relacionadas
- server: 6 unidades relacionadas

### Security

- client: 2 unidades relacionadas
- server: 1 unidades relacionadas

### Audio

- client: 3 unidades relacionadas
- server: 3 unidades relacionadas
- mesmo caminho relativo dos dois lados (nome completo ainda com prefixo diferente): 3

### Clan/social

- client: 19 unidades relacionadas
- server: 19 unidades relacionadas
- mesmo caminho relativo dos dois lados (nome completo ainda com prefixo diferente): 19

## Próxima prova necessária

Localizar o comando de link ou script que consome `object_files_client_mingw.txt` e `object_files_server_mingw.txt`. Isso permite classificar definitivamente `LINKED INTO CLIENT EXE`, `LINKED INTO SERVER EXE` e objetos compilados mas não linkados.
