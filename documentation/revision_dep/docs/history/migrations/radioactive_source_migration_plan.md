# Plano de migração da source — cliente e servidor

> Baseado no inventário de arquitetura, na árvore real do ZIP e nos quatro scripts de build/link enviados.

## 1. Diagnóstico executivo

- `SrcGame/src` tem **153 arquivos soltos na raiz** e **628 arquivos C/C++** no total.
- `SrcServer/src` tem **101 arquivos soltos na raiz** e **514 arquivos C/C++** no total.
- O inventário contém **5198 registros de função no cliente**, cobrindo **203 arquivos** em **22 sistemas**.
- O inventário contém **5448 registros de função no servidor**, cobrindo **186 arquivos** em **23 sistemas**.
- Uma varredura dos `#include` encontrou **314 includes do cliente** e **271 do servidor** contendo diretamente nomes de diretórios legados como `TJBOY`, `sinbaram`, `HoBaram`, `smLib3d` ou `WinInt`.
- Existem aproximadamente **230 MB de artefatos de build/backups/metadados de IDE** dentro da árvore entregue (`Debug`, `Release`, `.obj`, `.pdb`, `.tlog`, backups, `desktop.ini`, `.vcxproj.user`). Isso não deve ser migrado: deve sair da source.
- Cliente e servidor possuem centenas de caminhos equivalentes, mas quase todos têm conteúdo diferente. Portanto, deduplicar para `Shared` deve ser um projeto posterior e não parte da primeira mudança de layout.

### Decisão principal

A raiz de cada `src` deve conter somente o entry point. Todo o resto deve entrar em uma destas categorias:

1. `systems/` — domínio funcional do jogo;
2. `core/` — bootstrap, runtime e loop principal;
3. `platform/` — integração específica de Windows/OS;
4. `compat/` — pontes de toolchain/MinGW e compatibilidade temporária;
5. `diagnostics/` — dump/crash/debug runtime;
6. `third_party/` — código externo (ex.: Lua), fora dos sistemas do jogo.

## 2. Convenção de nomes

| Inventário atual | Diretório proposto |
|---|---|
| Admin / Operação | `systems/admin` |
| Banco / Persistência | `systems/persistence` |
| Clãs / Party / Social | `systems/social` |
| Combate / Skills | `systems/combat` |
| Compatibilidade MinGW / Bridge | `compat/mingw` |
| Core / Bootstrap / Game Loop | `core/runtime` |
| Diagnóstico / Dump | `diagnostics` |
| Efeitos / Partículas / Animação | `systems/effects` |
| Eventos / PvP / Rank / BattlePass | `systems/events` |
| Itens / Inventário / Economia | `systems/items` |
| Localização / Texto | `systems/localization` |
| Login / Opening | `systems/login` |
| Lua / Scripting | `systems/scripting` |
| Mundo / Character / Field | `systems/world` |
| Pets | `systems/pets` |
| Quests / VIP / Restauração | `systems/quests` |
| Rede / Protocolo | `systems/network` |
| Renderização / 3D / Câmera | `systems/rendering` |
| Segurança / Integridade | `systems/security` |
| Servidor / Orquestração | `systems/server_runtime` |
| Shared / Contratos | `contracts` |
| UI / Input / Settings | `systems/ui` |
| Utilitários / Modules | `utilities` |
| Áudio / Multimídia | `systems/audio` |

### Regras de nomenclatura

- Diretórios em inglês, minúsculos, `snake_case` apenas quando necessário.
- Nome de pasta descreve responsabilidade, nunca autor/apelido/origem histórica.
- `TJBOY`, `Ygy`, `sinbaram`, `HoBaram`, `Park` e `WinInt` não sobrevivem como fronteira arquitetural.
- Nomes de arquivo/símbolos antigos podem ser renomeados em uma segunda passagem depois que a movimentação compilar. Evite mudar caminho + nome de arquivo + símbolo C++ no mesmo commit.

## 3. Árvore-alvo

### Cliente

```text
SrcGame/
└── src/
    ├── Main.cpp
    ├── core/
    │   └── runtime/
    ├── systems/
    │   ├── world/
    │   ├── network/
    │   │   ├── protocol/
    │   │   ├── transport/
    │   │   ├── processing/
    │   │   └── memory/
    │   ├── combat/
    │   │   ├── skills/
    │   │   └── damage/
    │   ├── items/
    │   │   ├── inventory/
    │   │   ├── shop/
    │   │   ├── trade/
    │   │   └── crafting/
    │   ├── social/
    │   │   ├── clan/
    │   │   └── party/
    │   ├── events/
    │   │   ├── pvp/
    │   │   ├── ranking/
    │   │   └── battle_pass/
    │   ├── ui/
    │   │   ├── controls/
    │   │   ├── windows/
    │   │   └── settings/
    │   ├── rendering/
    │   │   ├── legacy3d/
    │   │   ├── fonts/
    │   │   └── camera/
    │   ├── effects/
    │   │   ├── particles/
    │   │   ├── animation/
    │   │   └── scripting/
    │   ├── audio/
    │   ├── pets/
    │   ├── quests/
    │   ├── persistence/
    │   ├── localization/
    │   ├── login/
    │   ├── scripting/
    │   ├── security/
    │   └── server_runtime/
    ├── platform/
    │   └── windows/
    ├── compat/
    │   └── mingw/
    └── diagnostics/
```

### Servidor

```text
SrcServer/
└── src/
    ├── Main.cpp
    ├── core/
    │   └── runtime/
    ├── systems/
    │   ├── world/
    │   ├── network/
    │   ├── combat/
    │   ├── items/
    │   ├── social/
    │   ├── events/
    │   ├── ui/
    │   ├── rendering/
    │   ├── effects/
    │   ├── audio/
    │   ├── pets/
    │   ├── quests/
    │   ├── persistence/
    │   ├── localization/
    │   ├── login/
    │   ├── scripting/
    │   ├── security/
    │   ├── server_runtime/
    │   └── admin/
    ├── platform/
    │   └── windows/
    ├── compat/
    │   └── mingw/
    └── diagnostics/
```

### Compartilhado / terceiros

```text
Shared/
└── contracts/
    ├── items/
    ├── network/
    ├── skills/
    └── world/

third_party/
└── lua/
```

> `Shared/Lua` e `SrcGame/src/Library/LuaSrc/Lua` não deveriam coexistir como duas árvores independentes. Consolidar Lua em `third_party/lua` é recomendado, mas **depois** da reorganização dos sistemas.

## 4. Mapa dos diretórios legados

| Origem legada | Destino | Observação |
|---|---|---|
| `TJBOY/clanmenu` | `systems/social/clan` | Clan UI/protocolo social |
| `TJBOY/isaocheck` | `systems/security/integrity` | Autenticação/integridade |
| `TJBOY/park` | `systems/audio/playback` | `MicVolume`/`ParkDXPlayer`; o nome `park` não descreve a função |
| `TJBOY/Ygy/Packet` | `systems/network/protocol` | Pacotes/estruturas |
| `TJBOY/Ygy/Process` | `systems/network/processing` | Processamento de rede |
| `TJBOY/Ygy/memmap` | `systems/network/memory` | Mapeamento/memória auxiliar |
| `TJBOY/Ygy/Util.*` | `systems/network` | Utilitário específico de rede |
| `smLib3d` | `systems/rendering/legacy3d` | Renderizador legado |
| `Language` | `systems/localization` | Manter subpastas de locale inicialmente |
| `Sound` | `systems/audio` | Áudio |
| `HoBaram/NewEffect` | `systems/effects` | Efeitos/partículas + script de efeitos |
| `HoBaram/*` | **dividir por arquivo** | Contém efeitos, login, party e pets; não pode ser movido por prefixo único |
| `sinbaram/*` | **dividir por arquivo** | Contém core, combat, items, effects, quests, pets, UI e world |
| `Server/*` | `systems/server_runtime` + `systems/events` | Há arquivos de orquestração e de evento misturados |
| `WinInt/*` | `platform/windows` ou `systems/audio` | Separar integração Windows de reprodução/stream |
| `Settings/*` | `systems/ui/settings` | UI/configuração |
| `Dump/*` | `diagnostics` | Crash dump |

## 5. Arquivos soltos da raiz: destino recomendado

### CLIENT

- **Main.cpp**: `Winmain.cpp`
- **systems/persistence**: `SQL.cpp`, `SQL.h`, `record.cpp`, `record.h`
- **systems/social**: `ClanCustom.cpp`, `ClanCustom.h`, `cPartyWindow.cpp`, `cPartyWindow.h`
- **systems/combat**: `Damage.cpp`, `DrawDamage.cpp`, `DrawDamage.h`, `ShowDamage.cpp`, `ShowDamage.h`, `SkillSub.cpp`, `SkillSub.h`, `damage.h`
- **compat/mingw**: `client_server_stubs.cpp`, `d3dx_font_alias.cpp`, `mingw_debug_entry.cpp`, `stdafx.cpp`, `stdafx.h`, `targetver.h`
- **core/runtime**: `CAutoCamera.cpp`, `CAutoCamera.h`, `KillHandler.cpp`, `KillHandler.h`, `NPCinfo.cpp`, `NPCinfo.h`, `TCore.cpp`, `TCore.h`, `fileread.cpp`, `fileread.h`, `playmain.cpp`, `playmain.h`, `playmodel.h`, `playsub.cpp`, `playsub.h`, `smReg.cpp`, `smreg.h`, `timer.cpp`, `timer.h`
- **systems/effects**: `BellatraFontEffect.cpp`, `BellatraFontEffect.h`, `Particle.cpp`, `particle.h`
- **systems/events**: `BattlePass.cpp`, `BattlePass.h`, `Rank.cpp`, `Rank.h`, `Ranking.cpp`, `Ranking.h`, `WarMode.cpp`, `WarMode.h`, `cHellsGate.cpp`, `cHellsGate.h`
- **systems/items**: `CLootHandler.cpp`, `CLootHandler.h`, `CoinShop.cpp`, `CoinShop.h`, `CoinShop_T.cpp`, `CoinShop_T.h`, `ItemForm.h`, `ItemForm2.h`, `JOBS.cpp`, `JOBS.h`, `cMixGuide.cpp`, `cMixGuide.h`
- **systems/localization**: `STRCLASS.CPP`, `STRCLASS.H`
- **systems/world**: `ActionGame.cpp`, `ActionGame.h`, `character.cpp`, `character.h`, `field.cpp`, `field.h`
- **systems/quests**: `cQuestWindow.cpp`, `cQuestWindow.h`, `cRestaureWindow.cpp`, `cRestaureWindow.h`, `sinHaQuest.cpp`, `sinHaQuest.h`
- **systems/network**: `NetType.h`, `netplay.cpp`, `netplay.h`, `opcodes.h`, `smwsock.cpp`, `smwsock.h`
- **systems/rendering**: `Drawsub.cpp`, `FontImage.cpp`, `FontImage.h`, `FullZoomMap.cpp`, `FullZoomMap.h`, `drawsub.h`, `makeshadow.cpp`, `makeshadow.h`, `pbackground.cpp`, `pbackground.h`
- **systems/security**: `CurseFilter.cpp`, `CurseFilter.h`, `checkdll.cpp`, `checkname.h`, `cracker.cpp`, `cracker.h`
- **systems/server_runtime**: `AreaServer.cpp`, `AreaServer.h`
- **systems/ui**: `CChatBox.cpp`, `CChatBox.h`, `Ime.cpp`, `Ime.h`, `TextMessage.cpp`, `TextMessage.h`, `UIButton.cpp`, `UIButton.h`, `UICheckbox.cpp`, `UICheckbox.h`, `UICombobox.cpp`, `UICombobox.h`, `UIElement.h`, `UIHorizontalScroll.cpp`, `UIHorizontalScroll.h`, `UIImage.cpp`, `UIImage.h`, `UIInput.cpp`, `UIInput.h`, `UIItemFrame.cpp`, `UIItemFrame.h`, `UILinker.h`, `UIList.cpp`, `UIList.h`, `UIListItem.cpp`, `UIListItem.h`, `UIPlaceholder.h`, `UIText.cpp`, `UIText.h`, `UITitle.cpp`, `UITitle.h`, `UIVerticalScroll.cpp`, `UIVerticalScroll.h`, `UIWindow.cpp`, `UIWindow.h`, `cSettingsWindow.cpp`, `cSettingsWindow.h`, `cTarget.cpp`, `cTarget.h`
- **systems/audio**: `avictrl.cpp`, `avictrl.h`, `effectsnd.cpp`, `effectsnd.h`
- **Revisão manual**: `priston.vcxproj`
- **Remover da source**: `Winmain.bak2.cpp`, `desktop.ini`, `priston.vcxproj.user`

### SERVER

- **Main.cpp**: `Winmain.cpp`
- **systems/admin**: `Alerta.cpp`, `Alerta.h`
- **systems/persistence**: `SQL.cpp`, `SQL.h`, `record.cpp`, `record.h`
- **systems/social**: `ClanCustom.cpp`, `ClanCustom.h`
- **systems/combat**: `Damage.cpp`, `SkillSub.cpp`, `SkillSub.h`, `damage.h`, `sinSkill_InfoIni.h`
- **compat/mingw**: `resource.h`, `server_build_compat.h`, `server_compat_packets.h`
- **core/runtime**: `CAutoCamera.cpp`, `CAutoCamera.h`, `fileread.cpp`, `fileread.h`, `playmain.cpp`, `playmain.h`, `playmodel.h`, `playsub.cpp`, `playsub.h`, `smReg.cpp`, `smreg.h`, `timer.cpp`, `timer.h`
- **systems/effects**: `BellatraFontEffect.cpp`, `BellatraFontEffect.h`, `Particle.cpp`, `particle.h`
- **systems/events**: `BattlePass.cpp`, `BattlePass.h`, `Rank.cpp`, `Rank.h`, `cHellsGate.cpp`, `cHellsGate.h`
- **systems/items**: `CoinShop.cpp`, `CoinShop.h`, `CoinShop_T.cpp`, `CoinShop_T.h`, `ItemForm.h`, `ItemForm2.h`, `JOBS.cpp`, `JOBS.h`
- **systems/localization**: `STRCLASS.CPP`, `STRCLASS.H`
- **systems/world**: `ActionGame.cpp`, `ActionGame.h`, `character.cpp`, `character.h`, `field.cpp`, `field.h`
- **systems/quests**: `cQuestHandler.cpp`, `cQuestHandler.h`, `cRestaureHandler.cpp`, `cRestaureHandler.h`, `cVipHandler.cpp`, `cVipHandler.h`, `sinHaQuest.cpp`, `sinHaQuest.h`
- **systems/network**: `NetType.h`, `netplay.cpp`, `netplay.h`, `opcodes.h`, `smwsock.cpp`, `smwsock.h`
- **systems/rendering**: `Drawsub.cpp`, `FontImage.cpp`, `FontImage.h`, `FullZoomMap.cpp`, `FullZoomMap.h`, `drawsub.h`, `makeshadow.cpp`, `makeshadow.h`, `pbackground.cpp`, `pbackground.h`
- **systems/security**: `CurseFilter.cpp`, `CurseFilter.h`, `PacketFilter.cpp`, `PacketFilter.h`, `checkdll.cpp`, `checkname.h`, `cracker.cpp`, `cracker.h`
- **systems/server_runtime**: `AreaServer.cpp`, `AreaServer.h`
- **systems/ui**: `Ime.cpp`, `Ime.h`, `TextMessage.cpp`, `TextMessage.h`
- **systems/audio**: `avictrl.cpp`, `avictrl.h`, `effectsnd.cpp`, `effectsnd.h`
- **Revisão manual**: `priston.vcxproj`
- **Remover da source**: `desktop.ini`, `priston.vcxproj.user`

## 6. Estratégia de `#include` para não quebrar tudo

### Estado final desejado

Código interno deve incluir a partir de **uma raiz canônica**, e não depender de dezenas de `-I`:

```cpp
#include "systems/network/protocol/Packet.h"
#include "systems/social/clan/ClanMenu.h"
#include "systems/rendering/legacy3d/smd3d.h"
#include "contracts/network/smPacket.h"
```

Evitar completamente:

```cpp
#include "../../../sinbaram/sinLinkHeader.h"
#include "TJBOY/clanmenu/tjclan.h"
#include "WinIntThread.h"       // só funciona porque existe -I específico
```

### Raízes de include finais

**Cliente:**

```bash
-ISrcGame/src
-IShared
-Ithird_party/lua/include   # quando Lua for consolidado
-Imodules                  # temporário; idealmente também reorganizar depois
```

**Servidor:**

```bash
-ISrcServer/src
-IShared
-Ithird_party/lua/include
-Imodules
```

Diretórios de SDK/Dependency continuam separados e não entram no namespace interno.

### Rewrites principais

| Include antigo | Include novo |
|---|---|
| `TJBOY/clanmenu/tjclan.h` | `systems/social/clan/tjclan.h` (depois renomear arquivo para `ClanMenu.h`) |
| `TJBOY/clanmenu/tjclanDEF.h` | `systems/social/clan/tjclanDEF.h` |
| `TJBOY/isaocheck/isaocheck.h` | `systems/security/integrity/isaocheck.h` |
| `TJBOY/isaocheck/auth.h` | `systems/security/integrity/auth.h` |
| `TJBOY/park/MicVolume.h` | `systems/audio/playback/MicVolume.h` |
| `TJBOY/park/ParkDXPlayer.h` | `systems/audio/playback/ParkDXPlayer.h` |
| `TJBOY/Ygy/Packet/PacketStruct.h` | `systems/network/protocol/PacketStruct.h` |
| `smLib3d/smd3d.h` | `systems/rendering/legacy3d/smd3d.h` |
| `smLib3d/smType.h` | `systems/rendering/legacy3d/smType.h` |
| `HoBaram/NewEffect/...` | `systems/effects/...` |
| `Language/<locale>/...` | `systems/localization/<locale>/...` |
| `Sound/...` | `systems/audio/...` |
| `sinbaram/SkillFunction/...` | `systems/combat/skills/...` |

### Caso especial: `sinLinkHeader.h` e outros headers-agregadores

`sinLinkHeader.h` é incluído dezenas de vezes e atravessa muitos sistemas. Não deve simplesmente virar `systems/combat/...` ou `systems/items/...`. Recomendo:

1. mover inicialmente para `core/legacy/LegacyGameplayIncludes.h` mantendo o conteúdo;
2. reescrever todos os includes para esse caminho canônico;
3. depois reduzir o agregador, fazendo cada consumidor incluir os headers reais que usa;
4. apagar o agregador quando deixar de ter consumidores.

Faça o mesmo tratamento para `HoLinkHeader.h`, `HoNewParticleLinkHeader.h` e headers equivalentes.

### Caso especial: `ddraw.h`

O cliente atualmente consegue resolver `ddraw.h` através do include path de `smLib3d`. Isso pode mascarar/confundir o header local com o DirectX SDK. Após a migração, qualquer consumidor do header local deve usar caminho explícito (`systems/rendering/legacy3d/ddraw.h`) ou o header deve ser classificado como terceiro/SDK legado.

## 7. Mudanças nos scripts de build

### Cliente

- O `find SrcGame/src -type f` já é recursivo; portanto `systems/**` continuará sendo compilado automaticamente.
- Remover progressivamente os `-I` específicos de `smLib3d`, `sinbaram`, `HoBaram`, `TJBOY`, `WinInt`, `Security`, `Settings` etc. depois que os includes forem canônicos.
- Atualizar os regex de exclusão que citam caminhos antigos (`/Server/...`, `Library/LuaSrc/Lua`, etc.).

### Servidor

- O `find SrcServer/src modules` já encontra recursivamente os arquivos.
- A lista adicional que injeta explicitamente `TJBOY/Ygy/...`, `sinInvenTory.cpp`, `sinSkill.cpp` etc. deve ser **removida**, não apenas renomeada. Ela é redundante com o `find` e vira uma fonte permanente de paths obsoletos.
- Depois disso, o source discovery fica exclusivamente baseado na árvore real + exclusões explícitas.

### Link

- Os dois linkers já consomem a lista de objetos produzida pelo build, em vez de procurar `.o` soltos. Isso é bom: uma mudança de caminho de source não exige manter caminhos antigos no linker.
- No primeiro build após cada lote de movimentos, usar `CLEAN=1` para eliminar cache/objetos associados ao layout anterior.

## 8. Ordem de migração recomendada

### Fase 0 — Baseline
1. Garantir build cliente e servidor verdes antes de mover qualquer arquivo.
2. Salvar logs e listas de objetos como baseline.
3. Remover artefatos de build/backups do repositório e adicionar regras de ignore.
4. Congelar renome de símbolos/refactors funcionais durante a migração de diretório.

### Fase 1 — Criar o esqueleto
1. Criar `systems/`, `core/`, `platform/`, `compat/`, `diagnostics/`.
2. Renomear somente `Winmain.cpp` para `Main.cpp` em cliente/servidor.
3. Manter temporariamente todos os include paths antigos.

### Fase 2 — Migrar domínios de baixa mistura
Ordem sugerida: `diagnostics` → `persistence` → `localization` → `audio` → `login` → `security` → `admin` (servidor) → `social/TJBOY` → `network/TJBOY`.
Para cada lote: mover `.cpp` + headers associados, reescrever includes, `CLEAN=1`, build cliente, link cliente, build servidor, link servidor.

### Fase 3 — Migrar domínios grandes
`rendering/smLib3d` → `effects/HoBaram` → `combat/items/quests/pets/ui` extraídos de `sinbaram` → `world` → `events`.

### Fase 4 — Core
Mover por último `playmain`, `playsub`, `sinMain`, `sinSubMain`, `fileread`, `timer`, `smReg` e demais runtime. Core tende a ter fan-out alto e é o pior lugar para começar.

### Fase 5 — Matar compatibilidade de paths
1. Remover um `-I` legado de cada vez.
2. Compilar após cada remoção.
3. A meta final é nenhum `-I` apontando para um sistema interno específico.
4. Fazer grep CI por nomes proibidos: `TJBOY|Ygy|sinbaram|HoBaram|smLib3d` em paths de include.

### Fase 6 — Renomear arquivos/símbolos ruins
Só depois do layout estar estável. Exemplos:

| Antigo | Sugestão |
|---|---|
| `tjclan.*` | `ClanMenu.*` ou `ClanUi.*` |
| `tjScroll.*` | `ClanScroll.*` |
| `YgyHeader.h` | `NetworkBridge.h` ou nome baseado no conteúdo real |
| `isaocheck.*` | `IntegrityCheck.*` |
| `WinIntThread.*` | `WindowsMediaThread.*` (se essa for realmente a função) |
| `sinLinkHeader.h` | `LegacyGameplayIncludes.h` temporário; depois eliminar |

## 9. Coisas que eu acrescentaria ao trabalho

### 9.1 Tirar lixo de build da source
Não mover `Debug/`, `Release/`, `.obj`, `.pdb`, `.tlog`, `desktop.ini`, `.vcxproj.user` e backups. Eles devem ser apagados/ignorados.

### 9.2 Unificar Lua, mas depois
Há Lua em mais de um lugar. Escolher uma árvore canônica (`third_party/lua`) e gerar uma biblioteca estática única elimina include paths duplicados e fontes de divergência.

### 9.3 Padronizar C++
O cliente está em C++17 e o servidor em C++14. Não é necessário para a migração de pastas, mas deveria existir uma decisão explícita de padrão de linguagem.

### 9.4 Reduzir permissividade
`-fpermissive` e, no servidor, `-w`, escondem dívida técnica. Não retirar durante a migração de diretórios; criar uma trilha posterior para warnings/portabilidade.

### 9.5 CI de arquitetura
Adicionar verificações automáticas:

- raiz de `src` só pode conter `Main.cpp` (e, se necessário, um PCH temporário);
- nenhum path contém nomes legados proibidos;
- nenhum `#include` interno contém `../`;
- nenhum include interno depende de `-I` de subdiretório de sistema;
- build cliente + servidor + link completos após cada lote.

## 10. Critério de pronto

A reorganização termina quando:

1. `SrcGame/src` contém apenas `Main.cpp` + diretórios arquiteturais;
2. `SrcServer/src` contém apenas `Main.cpp` + diretórios arquiteturais;
3. não existe diretório `TJBOY`, `Ygy`, `sinbaram`, `HoBaram`, `smLib3d` como agrupamento arquitetural;
4. todos os includes internos usam caminho canônico desde a raiz `src`/`Shared`;
5. build scripts não possuem `-I` para subpastas de sistemas;
6. servidor não possui source paths hardcoded redundantes;
7. builds limpos de cliente/servidor e links finalizam com sucesso;
8. artefatos de build não ficam versionados dentro de source.

## 11. Observação sobre o inventário

O inventário é excelente para classificar `.cpp` por responsabilidade, mas não deve ser aplicado cegamente a todos os arquivos. Alguns headers não aparecem diretamente e alguns artefatos de build do Lua aparecem misturados à classificação de `Shared / Contratos`. Neste plano, a classificação do inventário foi tratada como autoridade para código inventariado; associações de headers por arquivo irmão e classificações técnicas adicionais são inferências de migração e devem ser validadas pelo compilador.
