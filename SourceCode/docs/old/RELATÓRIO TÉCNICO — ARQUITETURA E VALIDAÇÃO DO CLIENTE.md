# RELATÓRIO TÉCNICO — ARQUITETURA E VALIDAÇÃO DO CLIENTE

## 1. Estado atual

O cliente foi reorganizado progressivamente a partir de uma estrutura histórica baseada em pastas como:

- `TJBOY`
- `Ygy`
- `WinInt`
- `HoBaram`
- `sinbaram`
- `smLib3d`
- `Language`
- `Library`
- `Server`
- ficheiros soltos diretamente em `SrcGame/src`

A nova arquitetura passou a organizar o código por **responsabilidade funcional**, principalmente sob:

```text
SrcGame/src/
├── core/
├── systems/
├── platform/
├── compat/
├── diagnostics/
└── third_party/
```

Até a Migration 123, o cliente compila e linka normalmente.

Depois da 123 restavam:

```text
20 arquivos C/C++ fora da arquitetura

19 em SrcGame/src/Server
1  SrcGame/src/opcodes.h
```

O lote final 124–126 foi criado para resolver precisamente esses últimos casos.

Se 124–126 compilarem, teremos:

```text
0 arquivos C/C++ do cliente fora das raízes arquiteturais.
```

Isso encerra a **fase de reorganização física do cliente**.

---

# 2. Princípio arquitetural descoberto

O SourceCode original não estava organizado por arquitetura.

As pastas antigas eram uma mistura de:

- nome de programador/equipa;
- época de desenvolvimento;
- biblioteca;
- subsistema;
- código copiado de servidor;
- código experimental;
- código de compatibilidade;
- sistemas completos;
- sistemas parcialmente duplicados.

Consequentemente, o nome da pasta antiga quase nunca descrevia corretamente a responsabilidade do código.

Exemplo:

```text
HoBaram/
```

não era um único sistema.

Continha:

- login;
- abertura;
- partículas;
- efeitos;
- pets;
- party;
- sky;
- texto;
- scripts;
- Lua;
- física;
- animação.

O mesmo acontecia com:

```text
sinbaram/
```

que continha:

- inventário;
- shop;
- trade;
- quests;
- skills;
- HUD;
- mensagens;
- eventos;
- pets;
- efeitos;
- utilitários;
- runtime principal.

Portanto a regra central adotada foi:

> A arquitetura deve representar responsabilidades do software, não a história dos ficheiros.

---

# 3. Arquitetura funcional descoberta

## 3.1 `systems/authentication`

Responsável por autenticação e entrada do jogador.

Inclui:

- login;
- informações de autenticação;
- character/login UI;
- integração com opening/character selection.

Exemplos migrados:

```text
systems/authentication/Login.*
systems/authentication/character_selection/...
```

---

# 3.2 `systems/security`

Agrupa mecanismos de proteção do cliente.

Subáreas identificadas:

```text
systems/security/client_integrity
systems/security/integrity
systems/security/anticheat
```

Responsabilidades encontradas:

- verificações de integridade;
- validação do cliente;
- detecção de cheat;
- checks de módulos/processos;
- mecanismos anti-tampering;
- autenticação de integridade.

Importante:

Esses sistemas devem ser testados não apenas procurando detecção, mas também garantindo que **jogadores normais não sejam falsamente expulsos ou bloqueados**.

---

# 3.3 `systems/social`

Agrupa sistemas de interação entre jogadores.

## Clan

```text
systems/social/clan
systems/social/clan/customization
```

Inclui:

- criação;
- menus;
- informação de clan;
- personalização;
- crown/tag;
- integração de rede.

## Party

```text
systems/social/party
systems/social/party/ui
```

Inclui:

- lista de membros;
- líder;
- HP/MP dos membros;
- comandos;
- localização/mapa;
- packets de party.

## Chat

```text
systems/social/chat
systems/social/chat/filter
```

Inclui:

- filtros de linguagem;
- validação de IDs;
- conversão de texto;
- filtros específicos de locale.

---

# 3.4 `systems/audio`

Foram encontrados vários subsistemas que originalmente estavam separados em pastas históricas.

## Playback

```text
systems/audio/playback/legacy
```

WAV/streaming e playback DirectSound legado.

## Voice

```text
systems/audio/voice
systems/audio/voice/ipc
systems/audio/voice/legacy_client
```

Inclui:

- microfone;
- volume;
- IPC;
- shared memory;
- packets;
- processamento do voice chat.

## Download

```text
systems/audio/download/legacy_http
```

Responsável pelo caminho legado de download/HTTP/ZIP relacionado a áudio.

## Effects

Efeitos de som associados aos sistemas gráficos/gameplay também foram classificados no domínio de áudio.

---

# 3.5 `systems/media`

```text
systems/media/video
systems/media/video/legacy_avi
```

Responsável por:

- vídeo;
- AVI;
- playback cinematográfico;
- componentes DirectShow/AVI antigos.

---

# 3.6 `systems/rendering`

Foi uma das maiores áreas descobertas.

## Font

```text
systems/rendering/font
systems/rendering/font/legacy_gdi_hook
```

Inclui:

- Font;
- FontImage;
- newFont;
- hooks GDI relacionados.

Nota:

`Font.cpp` original está praticamente vazio/0 bytes. Isso é legado real e não erro da migração.

## 3D legado

```text
systems/rendering/legacy_3d
```

Recebeu o antigo `smLib3d`.

Esse bloco contém uma grande parte da infraestrutura gráfica histórica do cliente.

## Camera

```text
systems/rendering/camera/cinematic
```

`CAutoCamera` foi identificado como câmera cinematográfica/scripted, especialmente ligada à apresentação/opening.

Não é o mesmo conceito de câmera normal de gameplay.

## Effects

Foram identificados vários grupos:

```text
systems/rendering/effects/...
```

incluindo:

- partículas Ho;
- NewEffect;
- AssaParticle;
- efeitos Sin;
- partículas legacy;
- física de efeitos;
- animação;
- scripts;
- efeitos específicos de skills.

## Sky/background/shadows

Também existem responsabilidades separadas para:

- sky;
- backgrounds;
- shadows;
- particles.

---

# 3.7 `systems/ui`

O UI antigo estava espalhado por praticamente todo o projeto.

Foi separado em responsabilidades.

## Core

```text
systems/ui/core
```

Componentes básicos:

- UIButton
- UICheckbox
- UICombobox
- UIElement
- UIImage
- UIInput
- UIList
- UIText
- UITitle
- UIWindow
- scrolls
- placeholders
- frames

Esses componentes formam a infraestrutura genérica da UI moderna do cliente.

## Settings

```text
systems/ui/settings
```

Interface das configurações.

## Messages

```text
systems/ui/messages
systems/ui/messages/legacy_sin
```

Caixas e mensagens de UI.

## Confirmation

```text
systems/ui/confirmation
```

Diálogos de confirmação.

## Targeting

```text
systems/ui/targeting
```

Informações de alvo.

## Maps/HUD/help

Também foram identificadas responsabilidades próprias para:

- mapa;
- full zoom map;
- HUD;
- ajuda;
- world information.

---

# 3.8 `systems/configuration`

```text
systems/configuration/client
```

Responsável pela configuração persistente do cliente.

O antigo `Settings.cpp/.h` pertence aqui.

A interface gráfica de settings pertence a `systems/ui/settings`, não a este domínio.

Essa separação é importante:

```text
configuration = estado/configuração
ui/settings   = interface para editar esse estado
```

---

# 3.9 `systems/combat`

## Damage

```text
systems/combat/damage
```

É o núcleo de damage/attack antigo.

Foi confirmado que `Damage.cpp` não era simples apresentação visual.

Ele contém:

- target selection;
- range;
- accuracy;
- critical;
- construção de packets;
- checksum;
- protections;
- skill damage;
- envio de ataque.

## Damage display

```text
systems/combat/damage_display
```

Contém mecanismos de apresentação como:

- `ShowDamage`
- `DrawDamage`

Não devem ser confundidos com o cálculo/envio do damage.

## PvP

```text
systems/combat/pvp
```

Inclui:

- scores;
- WarMode;
- kill notifications;
- streaks;
- eventos PvP.

## Skills

Grande parte do antigo `sinbaram` corresponde a skills e efeitos relacionados.

---

# 3.10 `systems/gameplay`

Área ampla para sistemas diretamente ligados às regras de jogo.

Foram identificados:

## Loot

```text
systems/gameplay/loot
systems/gameplay/loot/filter
systems/gameplay/loot/roll
```

Inclui:

- loot filter;
- Roll/Skip;
- filtragem de drops.

## Events

Eventos como:

- Hells Gate;
- Bellatra/SoD;
- eventos de mundo.

## Quests

```text
systems/gameplay/quests
systems/gameplay/quests/ui
systems/gameplay/quests/legacy
```

O projeto contém mais de uma geração de sistemas de quests.

Isso deve ser mantido explícito até existir uma futura refatoração funcional.

## Pets

Foram encontrados:

- LowLevelPet;
- PCBangPet;
- pets Sin;
- UI/mensagens associadas.

## NPC

Informação e UI de NPC.

## Character

Status e lógica auxiliar de personagem.

---

# 3.11 `systems/progression`

Inclui:

```text
systems/progression/battle_pass
systems/progression/class_rank
systems/progression/rankings
```

Responsável por:

- Battle Pass;
- ranks;
- ranking;
- progressão fora do combate imediato.

---

# 3.12 `systems/commerce`

## Coin Shop

```text
systems/commerce/coin_shop
```

Existem dois stores relacionados mas distintos:

```text
CoinShop
CoinShop_T
```

O primeiro é associado a SCoin.

O segundo é associado a TCoin/TimeShop.

Eles foram movidos juntos por responsabilidade, mas **não foram fundidos**.

## Restoration

```text
systems/commerce/restoration
```

Restauração de itens usando créditos/moeda.

## Premium

Sistemas de premium items.

## Shop/trade

O legado Sin contém ainda sistemas históricos de:

- shop;
- trade;
- commerce.

---

# 3.13 `systems/crafting`

Foram identificados pelo menos dois sistemas distintos.

## Mix Guide

```text
systems/crafting/mix_guide
```

Interface/guias de mix.

## Professions

```text
systems/crafting/professions
```

O antigo `JOBS` foi inicialmente suspeito de representar classes/jobs do personagem.

A inspeção mostrou que isso estava errado.

Ele representa um sistema de:

- profissões;
- mineração;
- crafting;
- mercado;
- venda;
- blueprints/informações associadas.

Por isso foi classificado em crafting.

---

# 3.14 `systems/network`

Responsabilidades identificadas:

```text
systems/network/client
systems/network/transport
systems/network/protocol
systems/network/region
```

Incluem:

- netplay;
- socket legado;
- tipos de rede;
- packets;
- códigos de operação;
- region/AreaServer.

## Descoberta crítica: `opcodes.h`

Existem pelo menos dois universos distintos:

```text
SrcGame/src/opcodes.h
Shared/opcodes.h
```

Eles não são cópias equivalentes.

A Migration 109 demonstrou isso quando consumidores foram redirecionados ao header errado.

Constantes como:

```text
OPCODE_ID_GETUSERINFO
SEND_ALERT_PVP
DELETE_ITEM_PREMIUM
OPCODE_FIX_ATTACK
FIND_CHEAT
UPDATE_BATTLE_POINTS
```

pertencem ao `Shared/opcodes.h`.

O antigo `SrcGame/src/opcodes.h` ficou identificado como pertencente ao legado Server incorporado ao cliente.

Conclusão:

> Headers homônimos nunca devem ser migrados usando apenas o basename.

É necessário saber **qual ficheiro cada include realmente resolvia durante a compilação**.

---

# 3.15 `systems/world`

```text
systems/world/field
```

`field.cpp/.h` pertence ao domínio de world/field.

É uma parte central do runtime e altamente conectada.

---

# 3.16 `systems/persistence`

```text
systems/persistence/character_record
```

Responsável por estruturas/fluxos ligados a:

- record;
- persistência;
- dados de personagem.

---

# 3.17 `core`

Usado para runtime central que não pertence corretamente a um sistema isolado.

Inclui responsabilidades como:

- gameplay runtime;
- client runtime;
- ActionGame;
- playmain;
- playsub;
- suporte central.

Estes ficheiros têm dependências enormes.

Por isso foram deixados para o final da migração.

---

# 3.18 `platform`

Código dependente de plataforma.

## Windows resources

```text
platform/windows/resources
```

Inclui:

- `.rc`;
- `.ico`;
- resource headers.

## Registry

```text
platform/windows/registry
```

`smReg` implementa lógica relacionada ao Registry/arquivo `ptReg.rgx`.

## Database

```text
platform/database/odbc
```

`SQL.cpp/.h` é um wrapper ODBC.

## Windows/application

O `Winmain.cpp` pertence conceitualmente à entrada/aplicação Windows.

---

# 3.19 `compat`

Código que existe para compatibilidade, e não por ser um domínio do jogo.

## DirectX

```text
compat/directx/d3dx9
```

`d3dx_font_alias.cpp` é um shim ABI/linker.

Mesmo sem consumidores textuais, pode ser necessário para resolver símbolos no link.

## MinGW

```text
compat/mingw/startup
```

`mingw_debug_entry.cpp` existe especificamente para o toolchain MinGW.

## Hooks legado

```text
compat/legacy/hooks
```

Exemplo: `vtablehook`.

## Client/server bridge

```text
compat/client_server
```

Uma das áreas de maior dívida técnica.

---

# 4. `client_server_stubs.cpp` — dívida técnica de alta prioridade

Esse ficheiro merece uma revisão própria.

Ele contém comentários indicando explicitamente:

```text
TEMP_MINGW_CLIENT_STUB
```

E implementa substitutos mínimos para símbolos originalmente server-side ou compartilhados.

Inclui:

- globais de server;
- estado de eventos;
- `ServerMode`;
- informações de Babel;
- BlessCastle;
- hardcore event;
- timing/debug server;
- funções de clan;
- área SoD;
- áreas de evento.

Também possui dezenas de funções que simplesmente retornam:

```cpp
0
```

em substituição a implementações reais.

Exemplos de categorias:

- quest;
- monstros;
- skills;
- conexão;
- itens;
- trade;
- logs;
- dinheiro;
- persistência.

## Revisão futura obrigatória

Antes de considerar essa camada definitiva, deve ser feita uma comparação símbolo a símbolo com o código do **Armazém**.

Verificar:

- assinaturas;
- tipos;
- globais;
- valores default;
- `GetClanCode`;
- regras de áreas;
- quests;
- combate;
- itens;
- conexão;
- persistência.

Um cliente compilar e abrir só prova que os símbolos existem.

Não prova que os stubs tenham comportamento correto.

**Prioridade dessa revisão: ALTA.**

---

# 5. `Shared/` é parte efetiva da arquitetura do cliente

Uma descoberta importante durante os batches foi que o cliente não é definido apenas por:

```text
SrcGame/src
```

O build também depende diretamente de:

```text
Shared/
```

Isso causou a falha da Migration 081.

Quando `sinItem.h` foi movido, três consumidores continuavam em:

```text
Shared/smPacket.h
Shared/roll.h
Shared/items.h
```

O primeiro gerador não os procurava.

Isso levou ao erro:

```text
sinbaram/sinItem.h: No such file or directory
```

Conclusão:

> Qualquer alteração de header deve auditar tanto `SrcGame/src` quanto todos os include roots utilizados pelo build, especialmente `Shared/`.

Ao mesmo tempo:

```text
SrcServer/
```

não deve ser automaticamente reescrito quando tiver headers homônimos.

O servidor possui cópias próprias de alguns desses ficheiros.

---

# 6. Include roots fazem parte do comportamento do código

Outro ponto importante apareceu na Migration 120.

`HoEffectMain.h` continha:

```cpp
#include "Script/lua/Win32/lua.h"
```

Esse caminho não era relativo ao ficheiro real.

Funcionava porque o compilador recebia um include root:

```text
SrcGame/src/HoBaram/NewEffect
```

Depois que os headers Lua foram movidos, o include deixou de resolver.

Conclusão:

> Não basta analisar caminhos relativos no source. É necessário reproduzir a resolução real considerando todos os `-I` de `cliente.sh`.

Essa regra passou a fazer parte do gerador de migrations.

---

# 7. Third-party / Lua

O projeto contém código Lua legado.

Foi deliberadamente tratado como:

```text
third_party/lua
```

e não como sistema próprio do jogo.

Havia também headers homônimos:

```text
lua.h
lauxlib.h
lualib.h
```

em mais de uma localização.

Por isso eles foram adiados inicialmente até ser possível saber qual implementação cada consumidor realmente utilizava.

Outro risco encontrado:

O build excluía os fontes Lua usando o caminho antigo.

Se a pasta fosse simplesmente movida, dezenas de `.c` poderiam entrar acidentalmente no build do Game.exe.

A migração correta precisou mudar ao mesmo tempo:

- include roots;
- localização;
- regex de exclusão do source discovery.

Isso preservou exatamente o conjunto de translation units compilados.

---

# 8. Umbrellas legados

Dois headers tinham papel de grandes agregadores:

```text
HoLinkHeader
sinLinkHeader
```

Durante grande parte da migração eles foram mantidos.

Isso foi proposital.

Removê-los cedo demais exigiria modificar centenas de dependências ao mesmo tempo.

A estratégia utilizada foi:

```text
mover primeiro os sistemas
→ atualizar caminhos dentro/para o umbrella
→ quando os subsistemas estiverem estabilizados
→ mover/canonicalizar o umbrella
→ eventualmente eliminar o umbrella numa futura refatoração
```

Essa abordagem reduz drasticamente o risco.

O antigo:

```text
YgyHeader
```

foi um dos poucos umbrellas realmente eliminados.

Ao removê-lo, descobrimos que `Process.h` dependia de includes transitivos.

Ele passou a incluir explicitamente:

```cpp
systems/audio/voice/ipc/memmap.h
systems/audio/voice/legacy_client/PacketStruct.h
systems/audio/voice/legacy_client/Packet.h
```

Essa mudança deve ser preservada.

---

# 9. Encoding dos fontes

Nem todos os fontes usam UTF-8.

Alguns ficheiros são:

- Latin-1;
- Windows-1252;
- encodings legados equivalentes.

Por isso scripts de migração não devem fazer indiscriminadamente:

```python
read_text()
write_text()
```

com UTF-8.

Para alteração de includes ASCII, a abordagem mais segura utilizada foi:

```python
read_bytes()
write_bytes()
```

Assim o restante do ficheiro permanece byte-idêntico.

---

# 10. Ordem de link e inicialização global

`cliente.sh` descobre fontes e gera listas ordenadas.

Mover um `.cpp` muda seu path.

Consequentemente pode mudar:

```text
posição no sort
→ posição na lista de objetos
→ ordem passada ao linker
```

Em software moderno isso normalmente não deveria alterar comportamento.

Este cliente, porém, contém:

- muitos objetos globais;
- inicialização estática;
- dependências entre translation units;
- código legado;
- ponteiros globais.

Portanto mudanças de path podem expor **undefined behavior preexistente** mesmo quando o conteúdo lógico do `.cpp` não muda.

Esse fato é importante ao investigar crashes após uma migration.

Um crash depois de mover um ficheiro não prova automaticamente que o path esteja errado.

Pode ser um bug latente exposto por novo layout/endereço/ordem.

---

# 11. O crash observado durante a Migration 048

A `CAutoCamera` inicialmente foi suspeita porque um crash apareceu depois da sua movimentação.

A Migration 048 foi revertida.

Mesmo com a câmera totalmente restaurada, o crash continuou.

Isso demonstrou que a simples relocação de `CAutoCamera` não era a causa suficiente para explicar o problema.

Os traces disponíveis localizaram a sequência problemática aproximadamente em:

```text
character creation
→ mudança de face
→ AddJobCharacter
→ ReadTextures
→ TextureSwap
→ OpenEachPlayer
→ FACE_SELECT_STATE
```

Uma construção especialmente perigosa encontrada no legado foi:

```cpp
memcpy(&chrEachPlayer, lpChar, sizeof(smCHAR));
```

`smCHAR` possui grande quantidade de ponteiros para:

- models;
- patterns;
- motions;
- textures;
- stage;
- targets;
- objects;
- dynamic patterns.

Logo, essa operação é uma **shallow binary copy de um objeto extremamente complexo**.

Isso é tecnicamente frágil e pode produzir ponteiros compartilhados/stale.

Também já existiam traces MinGW no baseline anterior à migração:

```text
MINGW_JOB_LOAD_TRACE
MINGW_CHAR_CREATE_TRACE
MINGW_CHANGEMOTION_NULL_GUARD
```

Portanto essa área já estava sendo investigada anteriormente.

Posteriormente o problema foi resolvido e a Migration 048 foi reaplicada.

A causa final exata/patcheada não ficou registrada neste processo; portanto não deve ser documentada como “confirmada”.

O que ficou confirmado é:

> Rollback da 048 não eliminou o crash, logo a simples alteração de path da câmera não era a causa raiz suficiente.

---

# 12. `stdafx.cpp` não era apenas PCH

Outra descoberta importante:

`stdafx.cpp/.h` parecia inicialmente infraestrutura de precompiled headers.

Mas `stdafx.cpp` implementa overrides globais de:

```cpp
operator new
operator new[]
operator delete
operator delete[]
```

e monitorização de allocations.

Portanto mover ou remover esse ficheiro pode afetar toda a memória do programa.

Isso explica por que ele foi tratado com muito mais cuidado que um PCH normal.

---

# 13. Shims que parecem mortos, mas não são

Alguns ficheiros não têm consumidores textuais claros.

Isso não significa que sejam inúteis.

Exemplos:

## `d3dx_font_alias.cpp`

Shim de compatibilidade/link D3DX9.

Pode existir exclusivamente para fornecer um símbolo ABI ao linker.

## `mingw_debug_entry.cpp`

Entrada/debug específica de MinGW.

A validação principal dele é:

```text
Game.exe linka
→ Game.exe inicia
```

Esses casos mostram por que “grep não encontrou consumidores” não é suficiente para declarar dead code.

---

# 14. Critério de migrations

Até a Migration 049 aproximadamente utilizámos:

```text
audit
→ build/link
→ send
→ smoke/runtime
```

A partir da Migration 050 foi acordada uma política mais rápida para migrations estritamente path-only:

```text
audit OK
→ compilação/link OK
→ migration aprovada
→ seguir
```

Isso foi adequado porque as migrations:

- não alteravam algoritmos;
- não alteravam símbolos;
- não alteravam estruturas;
- não alteravam protocolos;
- não alteravam comportamento intencionalmente.

## Importante

Isso é um critério de **avanço da reorganização**.

Não substitui a validação funcional final do cliente.

Depois de terminar a arquitetura é recomendável executar uma bateria completa de testes.

---

# 15. Níveis de validação

Para evitar confusão, existem quatro níveis diferentes.

## Nível 1 — Estrutural

```text
audit OK
```

Prova:

- source foi movido;
- caminho antigo desapareceu;
- caminho novo existe;
- includes esperados foram alterados;
- lista CPP foi atualizada;
- símbolos essenciais ainda existem.

## Nível 2 — Build

```text
cliente.sh OK
```

Prova:

- preprocessamento;
- compilação;
- resolução de headers;
- símbolos;
- link;
- ABI mínima;
- build graph.

## Nível 3 — Smoke

```text
Game.exe inicia
→ login
→ entra no jogo
→ não crasha imediatamente
```

## Nível 4 — Funcional

O sistema efetivamente migrado é exercitado.

Exemplo:

```text
CoinShop abre
→ dados aparecem
→ operação funciona
```

---

# 16. Validação FINAL recomendada para o cliente

Depois da Migration 126, o cliente não deve ser considerado definitivamente validado só porque compila.

A seguinte bateria deve ser executada pelo menos uma vez.

---

## 16.1 Build completamente limpo

Executar um clean build completo.

Objetivos:

- nenhum `.o` antigo;
- nenhum path legado mascarado;
- todas as translation units recompiladas;
- link integral.

Guardar:

```text
cpp_files_client_i686.txt
object_files_client_mingw.txt
build log
Game.exe
```

---

# 16.2 Startup

Testar:

- Game.exe abre;
- nenhuma DLL ausente;
- nenhum crash antes da janela;
- recursos carregam;
- fonts carregam;
- mouse/keyboard funcionam.

Valida especialmente:

- MinGW startup shim;
- resources;
- DirectX shims;
- stdafx/runtime;
- rendering initialization.

---

# 16.3 Opening / câmera

Testar:

- opening;
- transições;
- câmera cinematográfica;
- skip;
- entrada no login.

Valida:

- `CAutoCamera`;
- HoOpening;
- vídeo;
- sky/rendering;
- assets relativos.

---

# 16.4 Login

Testar:

- credenciais corretas;
- credenciais incorretas;
- reconexão;
- lista/seleção de servidor se existir;
- retorno ao login.

Valida:

- authentication;
- security;
- CurseFilter/ID validation;
- networking.

---

# 16.5 Character selection / creation

Este é obrigatório por causa do histórico do crash.

Testar repetidamente:

- abrir character selection;
- selecionar diferentes personagens;
- criar personagem;
- trocar classe;
- trocar face;
- voltar;
- avançar;
- repetir troca de face várias vezes;
- permanecer algum tempo nessa tela;
- entrar no jogo.

Observar especificamente:

- preview;
- animation;
- textures;
- motions;
- `smCHAR`.

---

# 16.6 Entrada no mapa

Testar:

- spawn;
- troca de field;
- warp;
- cidade;
- field externo;
- dungeon/mapa alternativo;
- retorno.

Valida:

- `field`;
- world runtime;
- networking;
- loaders;
- map resources.

---

# 16.7 Movimento e câmera de gameplay

Testar:

- andar;
- correr;
- câmera;
- zoom;
- rotação;
- target;
- FullZoomMap.

---

# 16.8 Rendering

Testar áreas com:

- personagens;
- mobs;
- NPC;
- weapons;
- armor;
- robes;
- shadows;
- partículas;
- transparency;
- sky;
- backgrounds;
- efeitos de skills;
- texturas animadas.

Verificar:

- corrupção visual;
- texturas brancas;
- flicker;
- objetos ausentes;
- crash tardio.

---

# 16.9 Fonts/texto

Abrir telas com:

- muito texto;
- diferentes fontes;
- chat;
- tooltip;
- mensagens;
- ranking;
- quest;
- shops.

Validar também caracteres especiais/localização.

---

# 16.10 Áudio

Testar:

- música;
- efeitos;
- ataques;
- skills;
- UI sounds;
- map ambience.

Se voice chat estiver disponível:

- microfone;
- volume;
- envio/recepção;
- IPC;
- reconnect.

---

# 16.11 Vídeo

Testar qualquer fluxo que utilize:

- AVI;
- cinematic;
- abertura;
- vídeo legado.

---

# 16.12 Combate básico

Testar:

- ataque normal;
- ataque ranged;
- skill;
- critical;
- miss;
- target;
- monstros;
- vários targets;
- range.

Valida:

```text
systems/combat/damage
SkillSub
network packets
damage checksum
```

---

# 16.13 Damage display

Confirmar:

- números aparecem;
- posição correta;
- animação;
- múltiplos damages;
- critical/diferentes cores se houver.

Valida:

```text
ShowDamage
DrawDamage
```

---

# 16.14 PvP

Quando houver outro jogador disponível:

Testar:

- PvP normal;
- kill;
- death;
- score;
- WarMode;
- kill streak;
- notifications;
- imagem/som de kill.

---

# 16.15 Inventário

Testar:

- abrir;
- fechar;
- arrastar item;
- equipar;
- desequipar;
- usar potion;
- stack;
- mover item;
- tooltip;
- storage se integrado;
- pickup/drop.

Este é especialmente importante porque `sinItem` e `Shared` causaram uma falha real durante a migração.

---

# 16.16 NPC

Testar:

- clicar NPC;
- abrir diálogo;
- fechar;
- shop;
- quests;
- funcionalidades específicas.

---

# 16.17 Shop / Trade

Testar:

- shop NPC;
- comprar;
- vender;
- quantidade;
- cancelar.

Se player trade existir:

- abrir trade;
- colocar/remover item;
- money;
- confirmar;
- cancelar.

---

# 16.18 Coin Shop

Testar:

```text
CoinShop
CoinShop_T / TimeShop
```

Verificar:

- abrir;
- carregar itens;
- saldo;
- páginas;
- detalhes;
- tentativa de compra.

Os dois sistemas são semelhantes, mas usam opcodes/assets diferentes.

---

# 16.19 Restoration

Abrir:

```text
cRestaureWindow
```

Testar:

- lista;
- preço;
- saldo;
- seleção;
- confirmação;
- cancelamento.

---

# 16.20 Mix Guide / crafting

Testar:

- Mix Guide;
- categorias;
- weapons;
- armor/robe;
- páginas.

---

# 16.21 Professions/JOBS

Testar os fluxos disponíveis de:

- mining;
- profession;
- crafting;
- market;
- sell;
- buy;
- blueprint/info.

---

# 16.22 Party

Com pelo menos dois jogadores:

- criar party;
- convidar;
- aceitar;
- líder;
- remover;
- sair;
- HP/MP;
- localização no mapa;
- comandos de party.

---

# 16.23 Clan

Testar:

- abrir clan;
- membros;
- menus;
- informação;
- crown/tag/customização se disponível;
- criação/edição onde possível.

---

# 16.24 Chat

Testar:

- normal;
- party;
- clan;
- whisper;
- caracteres especiais;
- filtro de palavras;
- nomes/IDs inválidos.

Para CurseFilter:

- palavra permitida;
- palavra bloqueada;
- textos próximos aos padrões bloqueados;
- locale específico se relevante.

---

# 16.25 Quests

Existem sistemas legacy e moderno.

Testar:

- abrir lista;
- aceitar;
- progresso;
- completar;
- recompensa;
- cancelar;
- spoiler/info;
- NPC quest.

---

# 16.26 Battle Pass

Testar:

- abrir;
- dados;
- progress;
- rewards;
- tabs;
- atualização.

---

# 16.27 Loot

Testar:

## Loot Filter

- ativar/desativar;
- categorias;
- drops relevantes.

## Roll

Se o sistema estiver disponível:

- Roll;
- Skip;
- resultado.

---

# 16.28 Pets

Testar:

- pet normal;
- LowLevelPet;
- PCBangPet;
- spawn;
- follow;
- efeitos;
- mensagem;
- desaparecimento/troca de map.

---

# 16.29 Eventos

Quando disponíveis:

## Hells Gate

- entrar;
- UI;
- timer;
- NPC;
- sair.

## Bellatra / SoD

- UI;
- pontuação;
- fonts;
- round/event;
- área.

## Outros eventos

Testar warp/event handlers quando o servidor permitir.

---

# 16.30 Rankings

Abrir:

- rank;
- ranking;
- class rank;
- listas;
- scroll;
- atualização.

---

# 16.31 Settings

Testar:

- abrir;
- alterar opções;
- aplicar;
- fechar;
- reiniciar cliente;
- confirmar persistência.

---

# 16.32 UI genérica

Como os widgets foram movidos em conjunto, exercitar:

- buttons;
- checkbox;
- combo;
- input;
- list;
- scroll vertical;
- scroll horizontal;
- images;
- text;
- title;
- nested windows.

A melhor forma é abrir várias telas modernas do cliente.

---

# 16.33 Confirmation/message boxes

Testar:

- OK;
- Cancel;
- Yes/No;
- mensagens de erro;
- caixas abertas por shop/trade/quest.

---

# 16.34 Network

Testar:

- login;
- entrada;
- movimentação;
- packets de combate;
- party;
- chat;
- shop.

Se possível:

- perda temporária da conexão;
- disconnect;
- reconnect;
- retorno ao login.

---

# 16.35 Security/anti-cheat

Não é necessário tentar provocar cheat.

O teste principal é:

```text
jogar normalmente por bastante tempo
→ nenhum falso positivo
→ nenhuma desconexão indevida
→ nenhum crash
```

---

# 16.36 Long-duration / soak test

Muito importante devido ao histórico de bugs tardios.

Recomendação:

```text
30–60 minutos mínimos
```

Idealmente mais.

Durante o soak:

- mudar maps;
- lutar;
- abrir interfaces;
- usar inventory;
- chat;
- NPC;
- morrer/respawn;
- retornar cidade.

Objetivo:

- memory corruption;
- dangling pointers;
- leaks extremos;
- crashes que não aparecem imediatamente.

---

# 17. Critério de aprovação FINAL

Eu consideraria o cliente tecnicamente validado quando existir evidência dos seguintes pontos:

```text
[ ] Migration 126 concluída
[ ] 0 C/C++ fora das raízes arquiteturais
[ ] clean build completo
[ ] link OK
[ ] send_to_client OK
[ ] startup OK
[ ] login OK
[ ] character selection/creation/face OK
[ ] entrada no jogo OK
[ ] troca de mapas OK
[ ] combate OK
[ ] inventory OK
[ ] NPC/shop OK
[ ] chat OK
[ ] party OK
[ ] rendering/effects OK
[ ] áudio OK
[ ] settings OK
[ ] quests OK
[ ] commerce/crafting principais OK
[ ] soak sem crash
[ ] anti-cheat sem falso positivo
```

Os sistemas que não puderem ser exercitados por falta de evento/jogadores devem ser registrados como:

```text
PENDING FUNCTIONAL TEST
```

e não como falha.

---

# 18. Histórico resumido das migrations

## 001–020

Fundação:

- diagnostics/crash;
- authentication;
- security;
- clan;
- voice;
- video;
- WinInt;
- AntiCheat;
- sound;
- UI;
- PvP;
- Settings.

## 021–040

Rendering/gameplay/platform:

- fonts;
- hooks;
- loot;
- resources;
- smLib3d;
- UI core;
- localization;
- AVI;
- damage display;
- PvP;
- WarMode;
- HellsGate;
- Loot Filter;
- SQL.

## 041–055

Sistemas modernos restantes da raiz:

- BattlePass;
- ClanCustom;
- registry;
- D3DX compatibility;
- MinGW startup;
- client/server stubs;
- damage core;
- camera;
- CoinShop;
- Restoration;
- MixGuide;
- Professions;
- Party;
- Quest UI;
- CurseFilter.

## 056–070

UI/rendering complementar:

- NPC;
- Target;
- Rank;
- Ranking;
- TCore;
- FullZoomMap;
- IME/STRCLASS;
- Bellatra;
- Drawsub;
- Particle;
- backgrounds;
- shadows;
- effectsnd;
- HoText;
- HoSky.

## 071–085

Decomposição de HoBaram e início Sin:

- Ho particles;
- NewEffect;
- scripts;
- effects foundation;
- AssaParticle;
- pets;
- party;
- HoLogin;
- HoOpening;
- inventory;
- shop/trade;
- quests;
- premium;
- character status.

## 086–100

Conclusão grande de sinbaram:

- help;
- Bellatra;
- skills;
- effects;
- events;
- messages;
- HUD;
- pets;
- utils;
- sinMain;
- sinLinkHeader;
- HoLinkHeader;
- misc;
- settings window;
- AreaServer.

## 101–115

Núcleo cliente:

- field;
- file loader;
- record;
- SkillSub;
- character;
- playmain;
- playsub;
- sockets;
- netplay;
- security misc;
- ActionGame;
- timers;
- legacy quest;
- checkdll.

A Migration 109 original foi deferida porque revelou a ambiguidade dos dois `opcodes.h`.

## 116–123

Fechamento da árvore normal:

- CChatBox;
- stdafx/targetver;
- HoOpeningOFF;
- amstream;
- Lua headers;
- Language;
- Lua source;
- Winmain.

## 124–126

Fechamento final planejado:

- resolver `opcodes.h`;
- mover legado Server usado pelo cliente;
- remover include roots legados e último `.inc`.

---

# 19. Falhas que melhoraram o processo

As falhas encontradas durante os batches foram úteis porque revelaram fraquezas reais na forma de migrar código legado.

## Migration 064

Audit inicialmente contou ficheiros consumidores em vez de número de diretivas.

Foi detectado antes da entrega.

## Migration 081

Consumidores em `Shared/` ficaram de fora.

Resultado:

```text
sinbaram/sinItem.h: No such file or directory
```

A infraestrutura passou a auditar `Shared`.

## Migration 109

Dois headers homônimos `opcodes.h`.

Resultado:

consumidores foram ligados ao header errado.

A conclusão correta foi **deferir**, não copiar símbolos.

## Migration 120

Dependência escondida por `-I`.

Resultado:

```text
Script/lua/Win32/lua.h: No such file or directory
```

O resolvedor passou a considerar include roots reais.

## Batch final

O primeiro runner usava:

```text
cliente.sh | tee
```

sem `pipefail`.

Isso poderia mascarar uma compilação com erro.

Foi corrigido antes da entrega final.

Esses incidentes produziram um pipeline significativamente mais confiável.

---

# 20. Como trabalhar com estes ficheiros daqui para frente

## Regra 1

Não voltar a organizar por pasta histórica.

Sempre perguntar:

> Qual responsabilidade funcional este código possui?

---

## Regra 2

Usar includes canônicos a partir de `SrcGame/src`.

Preferir:

```cpp
#include "systems/combat/damage/damage.h"
```

em vez de:

```cpp
#include "../../../damage.h"
```

---

## Regra 3

Não adicionar novos include roots específicos de sistema apenas para evitar corrigir includes.

Os paths devem ser explícitos.

---

## Regra 4

Não misturar numa mesma alteração:

```text
move de ficheiro
+
rename de classe
+
mudança de algoritmo
```

Fazer etapas separadas.

---

## Regra 5

Antes de mover um header homônimo, resolver seu alvo real.

Especialmente nomes genéricos como:

```text
opcodes.h
lua.h
language.h
timer.h
util.h
```

---

## Regra 6

Sempre verificar:

```text
SrcGame/src
Shared
include roots do cliente.sh
```

antes de concluir que conhecemos todos os consumidores.

---

## Regra 7

Preservar encoding.

Para rewrite exclusivamente ASCII:

```text
read bytes
→ substituir bytes
→ write bytes
```

---

## Regra 8

Toda alteração arquitetural deve poder voltar atrás.

O modelo de migration utilizado é:

```text
migration_xxx/
├── install.sh
├── audit.sh
├── rollback.sh
└── .state/
    ├── manifest
    └── snapshot
```

---

## Regra 9

Não usar Git como mecanismo da migration.

Foi decidido explicitamente usar:

```text
snapshot local
→ alteração
→ audit
→ build
→ rollback local se necessário
```

Git pode ser usado posteriormente pelo projeto se desejado, mas não deve fazer parte da mecânica das migrations.

---

## Regra 10

Scripts colados diretamente no terminal não devem conter:

```text
exit
set -e
```

que possam encerrar a sessão interativa.

Scripts `.sh` independentes podem e devem usar:

```bash
set -euo pipefail
```

---

# 21. Artefactos de build importantes

Os seguintes ficheiros são úteis para diagnosticar o cliente:

```text
cpp_files_client_i686.txt
object_files_client_mingw.txt
build_client_i686_compile_errors.log
cliente.sh
send_to_client.sh
```

O primeiro responde:

> Quais translation units estão realmente sendo compilados?

O segundo:

> Quais objetos serão linkados?

O log:

> Onde o build realmente falhou?

Isso é mais confiável que tentar inferir o build a partir de um `.vcxproj`.

---

# 22. `priston.vcxproj`

O projeto Visual Studio contém vários paths antigos.

Mas durante esta migração ele **não era o build ativo**.

O build Linux/MinGW através de:

```text
cliente.sh
```

foi tratado como fonte de verdade.

Portanto os paths históricos dentro do `.vcxproj` não foram misturados com estas migrations.

Se o Visual Studio voltar a ser utilizado no futuro, ele precisa de uma sincronização arquitetural própria.

---

# 23. O que NÃO devemos fazer imediatamente após concluir a 126

Não começar uma grande refatoração dos sistemas só porque agora as pastas estão organizadas.

Primeiro:

```text
terminar build
→ validar cliente
→ congelar baseline funcional
```

Só depois iniciar:

- remoção de dead code;
- eliminação de umbrellas;
- renames;
- unificação de protocolos;
- remoção de stubs;
- modernização de memória;
- redução de globals.

A nova estrutura deve tornar essas tarefas possíveis, mas elas são uma fase diferente.

---

# 24. Próxima dívida técnica recomendada

Depois do cliente validado, eu priorizaria:

## Prioridade 1

Comparar `client_server_stubs.cpp` com o Armazém.

## Prioridade 2

Revisar a fronteira:

```text
Shared
compat/client_server
SrcServer
```

e definir contratos reais.

## Prioridade 3

Unificar ou documentar corretamente os dois universos de opcodes.

Nunca simplesmente juntar os headers sem análise.

## Prioridade 4

Reduzir dependência de:

```text
HoLinkHeader
sinLinkHeader
```

quando isso puder ser feito com pequenas refatorações isoladas.

## Prioridade 5

Auditar globais/static initialization.

Especialmente porque alterações de link order podem expor UB.

## Prioridade 6

Revisar cópias binárias de objetos complexos.

Exemplo crítico:

```cpp
memcpy(... sizeof(smCHAR))
```

## Prioridade 7

Investigar dead/inactive code formalmente.

Não remover apenas porque “não apareceu no grep”.

Distinguir:

```text
source não compilado
source compilado sem consumidores
link shim
runtime reflection/export
verdadeiramente morto
```

---

# 25. Próxima fase: servidor

Quando o cliente estiver funcionalmente validado, começa a fase servidor.

A regra deve ser a mesma:

```text
inventário real
→ responsabilidade
→ pequenas migrations
→ snapshot
→ audit
→ build do servidor
→ somente depois avançar
```

Mas não devemos presumir que as classificações do cliente possam ser copiadas diretamente.

O servidor possui:

- implementações diferentes;
- headers homônimos;
- lógica autoritativa;
- versões próprias de sistemas;
- responsabilidades que no cliente aparecem apenas como stubs.

`Shared/` deverá ser tratado como **contrato entre os dois lados**, e não como “pasta auxiliar qualquer”.

---

# 26. Conclusão

A maior descoberta deste trabalho é que o SourceCode não tinha uma única arquitetura ruim; ele continha **várias gerações de arquitetura sobrepostas**.

Havia simultaneamente:

```text
arquitetura por programador
arquitetura por sistema
arquitetura por biblioteca
arquitetura por cliente/servidor
arquitetura por época
compatibilidade MinGW
compatibilidade DirectX
código third-party
código compartilhado
stubs
dead code
```

A reorganização conseguiu separar essas dimensões sem fazer uma reescrita funcional do cliente.

O resultado esperado após a Migration 126 é uma base em que um programador pode abrir:

```text
systems/combat
```

e encontrar combate,

abrir:

```text
systems/social
```

e encontrar sistemas sociais,

abrir:

```text
platform
```

e encontrar dependências de plataforma,

abrir:

```text
compat
```

e saber que está lidando com código mantido para compatibilidade,

e abrir:

```text
third_party
```

e saber que aquele código não pertence conceitualmente ao domínio do jogo.

Isso transforma futuras mudanças de:

> “descobrir onde alguém colocou um ficheiro há 15 anos”

para:

> “entrar no sistema responsável pela funcionalidade que quero alterar”.

Essa é a principal melhoria arquitetural produzida por esta fase.