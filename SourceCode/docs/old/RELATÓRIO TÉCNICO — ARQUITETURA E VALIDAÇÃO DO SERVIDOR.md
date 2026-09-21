# RELATÓRIO TÉCNICO — ARQUITETURA E VALIDAÇÃO DO SERVIDOR

## 1. Estado atual da migração

A fase servidor começou depois da reorganização física do cliente.

O inventário inicial encontrado em:

```text
SrcServer/src
```

foi:

```text
514 ficheiros C/C++ físicos
179 translation units (.cpp/.c)
```

Dois ficheiros já estavam arquiteturalmente classificados:

```text
diagnostics/crash/WinDump.cpp
diagnostics/crash/WinDump.h
```

Portanto o contador inicial da fase servidor foi:

```text
512 ficheiros C/C++ por migrar
```

### Estado confirmado por build real

O último estado explicitamente confirmado por compilação/link real nesta documentação é:

```text
Migration 186
BATCH 172–186 CONCLUÍDO
Server.exe linkado com sucesso
267 ficheiros restantes
```

Depois disso foram preparados e validados offline os seguintes lotes:

```text
187–201
267 → 176

202–216
176 → 57

217–234
57 → 0
```

Esses batches só devem ser marcados como definitivamente concluídos quando houver confirmação do build/link real do servidor no ambiente principal.

Se todas as Migrations 187–234 forem aprovadas pelo compilador, o resultado físico será:

```text
0 ficheiros C/C++ do servidor fora das raízes arquiteturais
```

---

# 2. Princípio arquitetural do servidor

Assim como no cliente, a árvore original do servidor não representava uma arquitetura funcional uniforme.

Ela misturava:

- sistemas server-side reais;
- código copiado do cliente;
- código compartilhado;
- bibliotecas;
- nomes de programadores/equipas;
- componentes administrativos;
- infraestrutura Windows;
- DirectX;
- compatibilidade;
- gameplay;
- persistência;
- networking;
- ferramentas;
- código aparentemente inativo.

Pastas históricas como:

```text
TJBOY
HoBaram
sinbaram
smLib3d
Server
Language
```

não representam sistemas únicos.

O princípio adotado também no servidor é:

> Organizar pelo papel que o código desempenha no sistema, e não pelo local histórico onde foi criado.

As raízes arquiteturais utilizadas são principalmente:

```text
SrcServer/src/
├── core/
├── systems/
├── platform/
├── compat/
├── diagnostics/
└── third_party/
```

---

# 3. Diferença fundamental entre arquitetura cliente e servidor

O cliente pode possuir lógica de:

- apresentação;
- UI;
- efeitos;
- envio de requests;
- previsão;
- estado local.

O servidor deve ser tratado como lado **autoritativo**.

Ele possui ou deve possuir responsabilidades como:

- validar ações;
- aplicar regras;
- persistir estado;
- distribuir estado;
- impedir manipulação;
- controlar economia;
- validar combate;
- validar quests;
- controlar eventos;
- gerenciar sessões e jogadores;
- operar infraestrutura administrativa.

Por isso uma função que parece duplicada entre cliente e servidor não deve automaticamente ser unificada.

Um ficheiro com o mesmo nome pode ter:

```text
cliente:
representação/previsão/UI

servidor:
regra autoritativa
```

Essa diferença apareceu diversas vezes durante a migração.

---

# 4. O servidor não é apenas `SrcServer/src`

Uma das descobertas mais importantes foi que o build servidor depende simultaneamente de:

```text
SrcServer/src
SrcGame/src
Shared
Shared/Lua
Shared2
modules
modules/File
```

Isso significa que a arquitetura real atravessa várias árvores.

Uma alteração em:

```text
Shared/
```

pode modificar simultaneamente:

```text
cliente
+
servidor
```

Da mesma forma, uma reorganização do cliente pode alterar indiretamente a compilação do servidor.

Essa foi uma das principais fontes de erros encontrados durante os clean builds.

---

# 5. `Shared` deve ser tratado como contrato

`Shared/` não é apenas uma pasta genérica de headers.

Ele representa em vários pontos o contrato entre:

```text
cliente ↔ servidor
```

Durante a reorganização do cliente, alguns includes dentro de `Shared` foram canonicalizados para paths do cliente.

Isso inicialmente pareceu correto porque o cliente compilava.

No servidor, porém, descobrimos que alguns desses headers deveriam selecionar implementações diferentes dependendo do lado.

Exemplos críticos:

```text
Shared/smPacket.h
Shared/roll.h
Shared/items.h
```

---

# 6. Descoberta crítica — fronteira `sinItem`

Durante a migração do cliente, estes headers passaram a usar:

```text
systems/inventory/legacy/sinItem.h
```

Esse é o `sinItem.h` reorganizado do cliente.

No servidor existia originalmente outra implementação:

```text
SrcServer/src/sinbaram/sinItem.h
```

O clean build do servidor expôs um conflito envolvendo:

```text
LPDIRECT3DTEXTURE9
```

A versão do cliente continha uma construção semelhante a:

```cpp
#ifndef LPDIRECT3DTEXTURE9
typedef void *LPDIRECT3DTEXTURE9;
#endif
```

O problema é:

> `LPDIRECT3DTEXTURE9` é um typedef, não uma macro.

Logo o preprocessor não consegue usar `#ifndef` para descobrir se o tipo já foi declarado.

Como o servidor também carregava headers reais de Direct3D, surgia:

```text
conflicting declaration 'typedef void* LPDIRECT3DTEXTURE9'
```

A fronteira correta passou a ser:

```cpp
#ifdef _SERVER
#include "sinbaram/sinItem.h"
#else
#include "systems/inventory/legacy/sinItem.h"
#endif
```

nos headers compartilhados relevantes.

Assim:

```text
cliente
→ sinItem do cliente

servidor
→ sinItem do servidor
```

Essa separação deve ser preservada.

---

# 7. Descoberta crítica — `smwsock`, `NetType` e `_W_SERVER`

Outra falha ainda mais importante apareceu quando passámos a fazer clean builds do servidor.

`Shared/smPacket.h` tinha sido canonicalizado para:

```text
systems/network/transport/legacy/smwsock.h
```

isto é, o `smwsock.h` do cliente.

Esse header carrega o `NetType.h` do cliente.

Cliente e servidor possuíam `NetType.h` com o mesmo include guard:

```text
_NET_TYPE_HEADER_
```

A sequência problemática era:

```text
Shared/smPacket.h
↓
smwsock do cliente
↓
NetType do cliente
↓
define _NET_TYPE_HEADER_
↓
não define _W_SERVER

depois:
NetType do servidor tenta carregar
↓
guard já definido
↓
header inteiro é ignorado
↓
_W_SERVER nunca é definido
```

Isso causou uma cascata de erros aparentemente não relacionados.

Por exemplo:

```text
AgingLevelBlock não declarado
AgingLevelHit não declarado

unhandled_handler não declarado
GameMasters não declarado
```

Muitos desses símbolos estavam protegidos por:

```cpp
#ifdef _W_SERVER
```

A solução correta não foi adicionar artificialmente:

```text
-D_W_SERVER
```

ao compilador.

O mecanismo histórico correto já existia no:

```text
SrcServer/src/NetType.h
```

A correção foi restaurar a seleção por lado:

```cpp
#ifdef _SERVER
#include "smwsock.h"
#else
#include "systems/network/transport/legacy/smwsock.h"
#endif
```

No servidor:

```text
smwsock.h
→ SrcServer/src/smwsock.h
→ SrcServer/src/NetType.h
→ _W_SERVER
```

No cliente:

```text
systems/network/transport/legacy/smwsock.h
→ NetType do cliente
```

Essa descoberta é arquiteturalmente muito importante:

> O servidor possui uma configuração de compilação e uma identidade própria que não pode ser substituída silenciosamente por headers homônimos do cliente.

---

# 8. Por que o clean build se tornou obrigatório

Inicialmente o build incremental do servidor conseguia passar.

Depois de mover `GameMasters.cpp`, apenas esse ficheiro precisou recompilar e expôs uma inconsistência antiga.

Quando executámos um clean build, apareceram várias outras.

Isso demonstrou que `.o` antigos estavam mascarando incompatibilidades introduzidas ou expostas pela reorganização das fronteiras compartilhadas.

A partir daí ficou definida a regra:

```text
início de cada novo batch do servidor
→ CLEAN BUILD/LINK baseline

depois:
migration
→ audit
→ build/link
→ próxima migration
```

O baseline clean evita atribuir a uma migration nova um erro que já existia no estado anterior.

---

# 9. Arquitetura funcional descoberta

# 9.1 `systems/persistence`

## Database core

```text
systems/persistence/database/core
```

Recebe o antigo bloco:

```text
Database/
```

Responsabilidade:

- acesso a dados;
- queries;
- abstrações de persistência;
- integração com dados do servidor.

## ODBC

```text
systems/persistence/database/odbc
```

Responsável pelo wrapper SQL/ODBC legado.

A dependência de:

```text
ODBC32.dll
```

permanece visível no `Server.exe`.

---

# 9.2 `systems/security`

## Firewall

```text
systems/security/firewall
```

Responsável por regras/proteções de rede específicas do servidor.

## Integrity

```text
systems/security/integrity
```

Parte herdada de `TJBOY/isaocheck`.

## Packet filter

```text
systems/security/network/packet_filter
```

Responsável por filtragem/validação de packets.

## Segurança legacy/process

Partes como:

```text
cracker
checkdll
```

foram classificadas separadamente para evitar misturá-las com lógica de gameplay.

---

# 9.3 `systems/admin`

## Game Masters

```text
systems/admin/game_masters
```

Responsável por:

- lista/configuração de GMs;
- identificação;
- permissões;
- leitura de dados;
- consultas administrativas.

Funções vistas durante o link:

```text
GameMasters::getInstance()
GameMasters::print()
GameMasters::readFromDatabase()
GameMasters::getGameMaster()
GameMasters::isGameMaster()
```

## CLI

```text
systems/admin/cli
```

Interface administrativa do servidor.

Integra-se com `GameMasters`.

Esse sistema merece testes específicos porque pode executar comandos operacionais importantes.

---

# 9.4 `systems/community`

```text
systems/community/news
```

Responsável pelo sistema de notícias/informações distribuídas pelo servidor.

---

# 9.5 `systems/authentication`

Exemplo:

```text
systems/authentication/protocol
```

Recebe componentes como o antigo:

```text
Priston/Login.h
```

No servidor, autenticação significa principalmente:

- interpretar requests;
- validar identidade;
- controlar sessão;
- conectar conta/personagem ao estado server-side.

---

# 9.6 `systems/network`

A arquitetura server-side de rede foi separada em responsabilidades.

## Transport

```text
systems/network/transport
```

Inclui:

```text
smwsock
NetType
```

É infraestrutura de socket/transporte.

## Protocol

```text
systems/network/protocol
```

Contém o universo de opcodes próprio do servidor.

Importante:

Os `opcodes.h` de:

```text
SrcGame
Shared
SrcServer
```

não devem ser considerados automaticamente equivalentes.

## Region / Area

```text
systems/network/region
```

ou responsabilidade equivalente associada a `AreaServer`.

---

# 9.7 `systems/combat`

## Skills

```text
systems/combat/skills/server_facade
systems/combat/skills/...
```

O servidor contém lógica e estruturas de skills próprias.

No `sinbaram` foram encontrados:

- `sinSkill`;
- `sinSkill_Info`;
- `SkillFunction`;
- `Morayion`;
- `Tempskron`;
- efeitos/auxiliares associados.

## Damage

```text
systems/combat/damage
```

No servidor, qualquer cálculo desta área deve ser tratado como potencialmente autoritativo.

Isso é diferente do cliente, onde alguns mecanismos podem existir para apresentação/previsão.

## PvP / roll / combate server-side

O antigo bloco `Server/` contém ainda handlers diretamente ligados ao combate e regras server-side.

---

# 9.8 `systems/gameplay`

Inclui vários domínios.

## Quests

```text
systems/gameplay/quests/server
```

Responsável por lógica server-side de quests.

Também há sistemas legacy provenientes de `sinbaram`.

## Events

```text
systems/gameplay/events/...
```

Inclui:

- Hells Gate;
- Bellatra / SoD;
- warp/eventos;
- FireCracker;
- spawn;
- Caravan;
- BlessCastle;
- auto drop;
- eventos específicos.

## Pets

```text
systems/gameplay/pets
```

Inclui:

- LowLevelPet;
- PCBangPet;
- pets Sin.

## Loot

```text
systems/gameplay/loot/alerts
```

Inclui sistemas de alerta e comportamento relacionado a loot/drop.

---

# 9.9 `systems/commerce`

Foram encontrados vários sistemas server-side.

## Coin Shop

```text
systems/commerce/coin_shop
```

Mantém separadamente:

```text
CoinShop
CoinShop_T
```

## Restoration

```text
systems/commerce/restoration
```

Handler server-side de restauração.

## VIP

```text
systems/commerce/vip
```

Sistema VIP server-side.

## Premium

Sistemas premium legados originários de `sinbaram`.

## Shop / Trade

O servidor contém regras próprias para:

- vendas;
- compras;
- trade;
- transações.

Essas funcionalidades devem ser consideradas críticas porque envolvem economia.

---

# 9.10 `systems/crafting`

```text
systems/crafting/professions
```

Recebe o antigo `JOBS`.

Assim como no cliente, `JOBS` não representa classes de personagem.

É ligado a:

- profissões;
- mineração;
- crafting;
- mercado.

---

# 9.11 `systems/social`

## Clan

```text
systems/social/clan/legacy
systems/social/clan/customization
```

O antigo `TJBOY/clanmenu` possui um grande número de ficheiros.

A migração deste bloco foi uma das maiores do início da fase servidor.

## Party

```text
systems/social/party/legacy
```

Lógica party histórica.

## Chat / CurseFilter

```text
systems/social/chat/filter
```

Validação de linguagem/texto/IDs.

---

# 9.12 `systems/rendering`

Embora seja servidor, o projeto ainda compila bastante código gráfico histórico.

Isso demonstra que a separação cliente/servidor no legado não é limpa.

Encontrámos:

```text
systems/rendering/legacy_3d
systems/rendering/font
systems/rendering/sky
systems/rendering/background
systems/rendering/shadows
systems/rendering/effects
systems/rendering/camera/cinematic
```

Isso inclui o antigo:

```text
smLib3d
HoBaram/NewEffect
Particle
Drawsub
makeshadow
pbackground
CAutoCamera
```

Alguma dessa lógica pode não ser utilizada funcionalmente no servidor, mas é compilada ou necessária por estruturas compartilhadas.

Ela não deve ser eliminada durante uma migration path-only.

---

# 9.13 `systems/audio`

Também existem partes históricas de áudio no servidor:

```text
systems/audio/voice
systems/audio/playback/legacy
systems/audio/download/legacy_http
systems/audio/effects/legacy
```

Isso inclui:

- MicVolume;
- WinInt;
- Sound;
- effectsnd;
- Ygy/voice.

Novamente, presença no servidor não significa necessariamente execução real de áudio.

Pode ser dívida da arquitetura compartilhada.

---

# 9.14 `systems/media`

```text
systems/media/video
systems/media/video/legacy_avi
```

Inclui:

- ParkDXPlayer;
- AVI;
- componentes DirectShow.

A presença desses componentes no servidor é uma indicação clara de acoplamento histórico com o cliente.

É candidata futura a limpeza depois da validação.

---

# 9.15 `systems/localization`

O servidor contém a árvore histórica de:

```text
Language/
```

Foram encontrados mais de cem headers.

A estrutura foi dividida em:

```text
systems/localization/config
systems/localization/messages
systems/localization/locales/...
compat/legacy/text
```

ou estrutura equivalente preservando os nomes históricos.

---

# 10. Case sensitivity dos locales

Durante a preparação das Migrations 202–216, tentei inicialmente normalizar nomes de directories para minúsculas.

Isso teria transformado alguns includes historicamente quebrados em includes válidos no Linux.

Exemplo conceitual:

```text
Chinese
vs
chinese
```

O projeto contém também o typo histórico:

```text
veitnam
```

A normalização foi rejeitada.

Regra estabelecida:

> Migration path-only não deve corrigir case, typos ou paths historicamente inválidos se isso puder modificar quais branches compilam.

A limpeza de nomes fica para uma futura refatoração específica.

---

# 11. Lua no servidor

Assim como no cliente, existem múltiplas cópias/versões Lua.

O bloco:

```text
HoBaram/NewEffect/Script/lua/Win32
```

possui headers modificados de Lua 5.0.2.

Eles não são automaticamente equivalentes aos existentes em:

```text
Shared/Lua
```

Por isso foram classificados separadamente como third-party/legacy headers.

Regra:

> Headers homônimos de bibliotecas third-party só devem ser consolidados depois de comparação real de conteúdo/API.

---

# 12. `smLib3d`

O antigo:

```text
smLib3d
```

também existe no servidor.

Foi classificado como:

```text
systems/rendering/legacy_3d
```

A existência desse bloco no servidor é mais um sinal de que várias structures/classes são compartilhadas entre cliente e servidor de maneira pouco modular.

No futuro pode ser possível reduzir o código gráfico realmente compilado pelo servidor, mas isso deve ser uma fase de limpeza posterior.

---

# 13. `HoBaram`

O bloco `HoBaram` do servidor possuía aproximadamente:

```text
75 ficheiros C/C++
```

Foi decomposto por responsabilidade.

Exemplos:

```text
HoTextFile
→ localization

HoSky
→ rendering/sky

HoParticle / HoNewParticle
→ rendering/effects/particles

NewEffect/math
→ rendering/effects/.../math

PiScript
→ rendering/effects/.../script

HoEffect / HoLogic / HoPhysics
→ rendering/effects/legacy_ho

AssaParticle
→ rendering/effects/assa

LowLevelPet / PCBangPet
→ gameplay/pets

HoParty
→ social/party

HoLogin
→ authentication/character_selection

HoOpening
→ presentation/opening

HoLinkHeader
→ legacy runtime/presentation boundary
```

Após a Migration 186:

```text
HoBaram C/C++:
75 → 0
```

Há um artefacto não-C++ residual de apenas 3 bytes:

```text
HoBaram/NewEffect/Script/Lua
```

conteúdo:

```text
lua
```

Ele não foi movido porque não havia evidência suficiente da sua função.

Deve entrar numa posterior limpeza de resíduos.

---

# 14. `sinbaram`

O bloco servidor possuía aproximadamente:

```text
91 ficheiros C/C++
```

Foi dividido em 15 responsabilidades.

Principais grupos:

```text
inventário
shop/trade
quests
premium
character status
help
Bellatra/SoD
skills
effects/particles
events/warp
messages
HUD/interface
PC Bang pet
DirectShow/amstream
runtime Sin + utilities + sinLinkHeader
```

Também existe:

```text
item_catalog_recovered.inc
```

usado por `sinItem.cpp`.

Esse `.inc` não aparecia no contador C/C++, mas é uma dependência real e foi tratado junto da migração de inventário.

Regra derivada:

> Contadores C/C++ são úteis para progresso, mas não substituem inventário de artefactos auxiliares como `.inc`, `.rc`, `.aps`, `.ico`, scripts e data files.

---

# 15. Paths hardcodeados dentro do build

O servidor apresentou mais paths hardcodeados do que o cliente.

Exemplo importante:

```text
build_server_mingw_i686_j8.sh
```

incluía explicitamente fontes de `TJBOY/Ygy`.

Posteriormente também foram encontrados `.cpp` de `sinbaram` hardcodeados.

Exemplos:

```text
sinInvenTory.cpp
sinSkill.cpp
sinSkill_Info.cpp
Morayion.cpp
Tempskron.cpp
```

Uma migration server-side deve portanto procurar por:

```text
includes
+
cpp list
+
find/source discovery
+
paths literais dentro do próprio script de build
```

Essa regra foi incorporada aos batches posteriores.

---

# 16. `resource.h` duplicado

Foi encontrado outro caso importante de headers homônimos.

Existiam dois:

```text
Resources/resource.h
SrcServer/src/resource.h
```

O primeiro foi movido com resources/RC.

O segundo não era uma cópia idêntica.

Possuía IDs adicionais ligados ao runtime/menu Win32 do servidor.

A decisão correta foi não fundir.

O segundo foi reservado para:

```text
platform/windows/resources/legacy_runtime/resource.h
```

Regra:

> Headers com mesmo nome não são duplicados até comparação provar que são duplicados.

---

# 17. `Server/` não representa todo o servidor

Existe uma pasta histórica:

```text
SrcServer/src/Server
```

Mas ela é apenas um subconjunto da aplicação server-side.

Contém componentes como:

- `OnSever`;
- `DllServer`;
- handlers;
- party;
- mensagens;
- eventos;
- damage;
- BlessCastle;
- logs;
- funções auxiliares.

A aplicação inteira também depende de dezenas de ficheiros fora dessa pasta.

Portanto:

> `Server/` é uma fronteira histórica, não a arquitetura do servidor.

No lote final, os ficheiros dessa pasta são separados por responsabilidade antes de o diretório desaparecer como fronteira arquitetural.

---

# 18. `Winmain.cpp`

O `Winmain.cpp` do servidor é deixado para a última migration física.

Isso é proposital.

Ele contém:

- entrada da aplicação;
- inicialização;
- globais;
- janelas;
- setup;
- handlers;
- integração administrativa;
- variáveis utilizadas por vários módulos.

Durante um clean build quebrado, a falha de compilação de `Winmain.cpp` gerou dezenas de undefined references:

```text
VRKeyBuff
quit
g_fWinSizeRatio_X
g_fWinSizeRatio_Y
hTextWnd
hFocusWnd
pCursorPos
```

Isso mostrou que muitos erros do linker eram consequências secundárias.

Regra de diagnóstico:

> Sempre corrigir primeiro os erros de compilação. Undefined references posteriores podem desaparecer automaticamente quando o translation unit que os define voltar a compilar.

---

# 19. Pipeline das migrations do servidor

Cada migration utiliza:

```text
migration_NNN_nome/
├── install.sh
├── audit.sh
├── rollback.sh
└── .state/
    ├── manifest
    └── snapshot
```

Fluxo:

```text
preflight
↓
snapshot
↓
move
↓
rewrite dos consumidores
↓
rewrite do build se necessário
↓
audit
↓
build/link
↓
próxima migration
```

---

# 20. Política de rollback

Rollback é local à migration.

Não usa Git.

O rollback deve restaurar apenas:

```text
ficheiros no manifest
```

e remover os destinos novos da migration.

Durante a validação offline dos batches foram exercitados ciclos:

```text
install
→ audit
→ rollback
→ comparação byte-a-byte
→ reinstall
```

Isso foi especialmente importante para migrations grandes como:

```text
TJBOY/clanmenu
smLib3d
HoBaram
sinbaram
Language
```

---

# 21. Regra do terminal

Scripts para serem colados diretamente no terminal interativo não devem conter:

```text
exit
set -e
```

Scripts independentes podem e devem usar:

```bash
set -euo pipefail
```

O runner dos batches também precisa de:

```bash
set -o pipefail
```

quando usa:

```bash
cliente.sh | tee
```

ou equivalente no servidor.

Durante o cliente foi encontrado um runner em que `tee` mascarava o exit code do compilador.

Essa falha foi corrigida antes da fase final e deve continuar sendo evitada.

---

# 22. Critério atual para avançar numa migration do servidor

Para relocation path-only:

```text
audit OK
+
build/link OK
=
migration aprovada
```

No servidor, porém, existe uma proteção adicional:

```text
CLEAN BUILD baseline
```

no início de cada batch.

Isso garante que o ponto anterior está realmente consistente.

---

# 23. Níveis de validação do servidor

## Nível 1 — estrutural

```text
audit OK
```

Confirma:

- ficheiros movidos;
- antigos removidos;
- includes reescritos;
- build paths atualizados;
- symbols essenciais preservados.

## Nível 2 — build

```text
Server.exe compila e linka
```

Confirma:

- preprocessing;
- includes;
- macros;
- estruturas;
- symbols;
- link.

## Nível 3 — startup

```text
Server.exe inicia
```

Confirma:

- DLLs;
- configuração;
- database initialisation;
- socket initialisation;
- runtime base.

## Nível 4 — integração

```text
cliente conecta
→ login
→ gameplay
```

## Nível 5 — funcional/autoritativo

Sistemas individuais do servidor são realmente exercitados.

---

# 24. Dependências observadas do `Server.exe`

Num build real aprovado após a Migration 141, o executável era:

```text
PE32 executable
Intel i386
console
aprox. 8 MB
```

DLLs declaradas incluíam:

```text
ADVAPI32.dll
ddraw.dll
GDI32.dll
KERNEL32.dll
libgcc_s_dw2-1.dll
libstdc++-6.dll
msvcrt.dll
ODBC32.dll
ole32.dll
OLEAUT32.dll
SHLWAPI.dll
USER32.dll
WININET.DLL
WINMM.DLL
WS2_32.dll
```

Isso mostra que o servidor ainda está fortemente ligado a:

- Win32;
- ODBC;
- WinSock;
- WinInet;
- multimedia;
- DirectDraw;
- COM/OLE.

Nem todas essas dependências necessariamente representam funcionalidades server-side desejáveis no futuro.

---

# 25. Testes necessários para validar o servidor

Depois da reorganização física, deve existir uma fase de validação funcional completa.

---

# 25.1 Clean build definitivo

Executar:

```text
clean compile de todas as translation units
→ link Server.exe
```

Guardar:

```text
cpp_files_server_i686.txt
object_files_server_mingw.txt
build_server_i686_compile_errors.log
server_link_i686.log
server_link_missing_i686.txt
Server.exe
```

O build incremental não deve ser usado como prova final.

---

# 25.2 Startup

Testar:

```text
Server.exe inicia
não fecha imediatamente
não lança exception
não reclama DLL ausente
```

Observar logs.

---

# 25.3 Database

Testar:

- conexão ODBC;
- autenticação;
- queries iniciais;
- leitura de configuração;
- leitura de GM;
- leitura de contas/personagens;
- escrita;
- reconnect.

Testar falha controlada:

```text
database indisponível
```

O servidor deve falhar de maneira previsível ou entrar no comportamento esperado.

---

# 25.4 GameMasters

Testar:

- carregamento;
- `readFromDatabase`;
- reconhecimento de GM;
- conta comum não reconhecida;
- comandos permitidos;
- comandos negados.

---

# 25.5 CLI administrativa

Testar:

- abrir CLI;
- comandos válidos;
- comando inválido;
- print/list;
- reload;
- comandos associados a GM;
- shutdown/maintenance apenas em ambiente seguro.

---

# 25.6 Firewall / packet filter

Testar:

- cliente normal conecta;
- packets normais passam;
- conexão inválida não crasha;
- packet inválido é rejeitado;
- packet curto/truncado;
- opcode desconhecido.

Evitar testes destrutivos no ambiente de produção.

---

# 25.7 Login

Testar pelo cliente:

```text
conta válida
conta inválida
password errado
conta inexistente
conta já conectada
disconnect/reconnect
```

---

# 25.8 Character selection

Testar:

- listar personagens;
- selecionar;
- entrar;
- personagem inexistente;
- criação;
- exclusão, se disponível.

---

# 25.9 Persistência do personagem

Alterar estado no jogo:

- EXP;
- level;
- gold;
- inventory;
- equipamento;
- posição;
- quest.

Desconectar.

Reconectar.

Confirmar persistência.

---

# 25.10 Inventário

Testar autoritativamente:

- pickup;
- drop;
- equip;
- unequip;
- stack;
- potion;
- storage;
- trade;
- venda;
- compra.

Verificar duplicação de item.

---

# 25.11 Shop

Testar:

```text
comprar
vender
quantidade
gold insuficiente
item inexistente
request inválido
```

O servidor nunca deve confiar apenas no valor enviado pelo cliente.

---

# 25.12 Trade entre jogadores

Com dois clientes:

- abrir;
- adicionar/remover item;
- gold;
- aceitar;
- cancelar;
- disconnect durante trade.

Testar especialmente condições de duplicação.

---

# 25.13 Coin Shop / premium / VIP

Testar:

- saldo;
- item;
- compra;
- saldo insuficiente;
- request repetido;
- cancelamento;
- persistência.

Esses sistemas envolvem economia real do jogo e têm prioridade alta.

---

# 25.14 Restoration

Testar:

- recuperar item válido;
- item inexistente;
- créditos insuficientes;
- operação repetida;
- persistência.

---

# 25.15 Crafting / professions

Testar:

- requisitos;
- materiais;
- sucesso;
- falha;
- consumo de itens;
- resultado;
- market;
- mining;
- profession state.

---

# 25.16 Combat

Teste obrigatório.

Com cliente normal:

- ataque físico;
- ranged;
- skill;
- critical;
- miss;
- defesa;
- monstros;
- PvP.

Validar que:

```text
cliente não consegue definir damage arbitrário
```

O servidor deve recalcular/validar os valores relevantes.

---

# 25.17 Skills

Testar diferentes classes.

Especialmente:

```text
Morayion
Tempskron
```

porque aparecem explicitamente no build e no sistema histórico de SkillFunction.

Testar:

- cooldown;
- mana;
- range;
- target;
- damage;
- buffs;
- debuffs.

---

# 25.18 Movement

Testar:

- movimento normal;
- velocidade;
- warp;
- teleporte;
- coordenadas inválidas;
- troca de mapa.

---

# 25.19 Field / AreaServer

Testar:

- spawn;
- mudança de field;
- área;
- transferência;
- NPC/mobs do field;
- mapa inválido;
- desconexão durante transição.

---

# 25.20 Monster/spawn

Testar:

- spawn;
- respawn;
- morte;
- drops;
- AI básica;
- limpeza;
- eventos com múltiplos mobs.

---

# 25.21 Loot/drop

Testar:

- drop normal;
- ownership;
- pickup;
- loot rules;
- alertas;
- auto drop/event drop.

---

# 25.22 Quests

Testar:

- aceitar;
- progresso;
- kill count;
- entrega;
- reward;
- cancelamento;
- reconnect;
- duplicação de reward.

---

# 25.23 Party

Com dois ou mais clientes:

- criar;
- convidar;
- aceitar;
- líder;
- sair;
- kick;
- disconnect;
- EXP;
- localização;
- loot/party rules.

---

# 25.24 Clan

Testar:

- leitura do clan;
- criação se disponível;
- membros;
- permissões;
- tag;
- customização;
- persistência.

---

# 25.25 Chat

Testar:

- normal;
- party;
- clan;
- whisper;
- filtro;
- spam;
- caracteres especiais.

---

# 25.26 Bellatra / SoD

Quando disponível:

- entrada;
- pontuação;
- rounds;
- mobs;
- reward;
- saída;
- reconnect.

---

# 25.27 Hells Gate

Testar:

- entrada;
- evento;
- timer;
- progressão;
- recompensa;
- saída.

---

# 25.28 BlessCastle / Siege

Quando disponível:

- estado do castelo;
- clan;
- scoring;
- início/fim;
- rewards;
- persistência.

É sistema de alta prioridade por ser altamente stateful.

---

# 25.29 Outros eventos

Testar conforme disponibilidade:

- Caravan;
- FireCracker;
- warp events;
- spawn events;
- auto-drop;
- eventos temporários.

---

# 25.30 Pet systems

Testar:

- LowLevelPet;
- PCBangPet;
- pets Sin;
- spawn/despawn;
- mapas;
- reconnect.

---

# 25.31 News

Testar:

- carregamento;
- envio ao cliente;
- conteúdo vazio;
- conteúdo grande;
- reload se existir.

---

# 25.32 Reconnect / disconnect

Muito importante.

Testar:

```text
cliente fecha abruptamente
internet cai
socket encerra
login repetido
reconnect rápido
```

Verificar se:

- sessão antiga é limpa;
- personagem não fica preso;
- trade é cancelado;
- party atualiza;
- locks são liberados.

---

# 25.33 Concorrência

Testar com vários clientes simultaneamente.

Áreas principais:

- login;
- database;
- inventory;
- trade;
- party;
- combat;
- chat.

O servidor é muito mais sensível que o cliente a problemas de:

```text
race condition
shared state
global state
reentrancy
```

---

# 25.34 Soak test

Executar o servidor continuamente por várias horas.

Durante o período:

- conectar/desconectar clientes;
- trocar mapas;
- lutar;
- usar shops;
- quests;
- trade;
- party;
- eventos.

Monitorar:

```text
RAM
handles
threads
CPU
database connections
socket count
logs
```

---

# 25.35 Restart

Testar:

```text
shutdown normal
→ restart
```

Confirmar:

- banco consistente;
- personagens persistidos;
- eventos resetados corretamente;
- locks removidos;
- Server.exe sobe novamente.

---

# 25.36 Crash recovery

Se houver mecanismo de dump/log:

- verificar criação;
- verificar caminho;
- garantir que um crash gere informação utilizável.

Não é necessário provocar crash em produção.

---

# 26. Critério de validação final do servidor

Uma checklist recomendada:

```text
[ ] migrations físicas concluídas
[ ] 0 C/C++ fora das raízes arquiteturais
[ ] clean build completo
[ ] link Server.exe OK
[ ] startup OK
[ ] database OK
[ ] GameMasters/CLI OK
[ ] login OK
[ ] character load/save OK
[ ] movement/field OK
[ ] combat OK
[ ] skills OK
[ ] inventory OK
[ ] shop OK
[ ] trade OK
[ ] quests OK
[ ] party OK
[ ] clan OK
[ ] chat OK
[ ] CoinShop/premium/VIP OK
[ ] restoration OK
[ ] crafting/professions OK
[ ] loot/drop OK
[ ] pets OK
[ ] eventos principais OK
[ ] reconnect/disconnect OK
[ ] restart OK
[ ] soak test OK
[ ] sem falso positivo de segurança
[ ] sem duplicação/economia inconsistente
```

Sistemas indisponíveis temporariamente devem ser marcados:

```text
PENDING FUNCTIONAL TEST
```

e não como automaticamente aprovados.

---

# 27. Segurança — prioridades especiais

O servidor é autoridade.

Qualquer input vindo do cliente deve ser considerado não confiável.

Revisões futuras devem priorizar:

```text
quantidades
gold
coins
item IDs
damage
skill IDs
posição
map ID
quest state
trade state
admin commands
opcodes
packet sizes
strings
```

Uma reorganização de paths não garante a segurança dessas regras.

Ela apenas cria uma arquitetura em que será mais fácil encontrá-las e auditá-las.

---

# 28. Economia — prioridade crítica

Depois de validar o build, os seguintes sistemas merecem auditoria transacional:

```text
CoinShop
CoinShop_T
VIP
Premium
Restoration
Shop
Trade
Crafting
Inventory
Drops
```

Procurar:

- duplicação;
- double-spend;
- race conditions;
- rollback incompleto;
- operações repetidas;
- confiança excessiva no cliente.

---

# 29. Database — futura revisão

O servidor contém tanto:

```text
database core
ODBC wrapper
```

quanto lógica de persistência espalhada.

Futuramente seria recomendável separar:

```text
domain logic
↓
repository/storage interface
↓
ODBC/database implementation
```

No momento, isso não deve ser feito junto com a migration física.

---

# 30. Globais e static initialization

Assim como no cliente, o servidor possui muitos globais.

O caso de `Winmain.cpp` revelou vários.

Mover translation units pode alterar:

```text
path
→ ordenação
→ ordem de object files
→ layout/link order
```

Isso pode expor undefined behavior.

Por isso um crash após relocation não significa necessariamente que o novo path esteja semanticamente errado.

Pode indicar dependência indevida de ordem de inicialização global.

Essa é uma dívida técnica futura importante.

---

# 31. Código gráfico no servidor

Uma futura auditoria deve responder:

> Quais partes de rendering/audio/media são realmente necessárias para o servidor?

O servidor ainda contém ou linka:

- DirectDraw;
- UI;
- font;
- particle;
- sky;
- audio;
- AVI;
- DirectShow.

Algumas dessas dependências podem existir apenas porque structures compartilhadas puxam código cliente.

Depois de estabilizar o servidor, vale tentar reduzir essa superfície.

Mas não antes.

---

# 32. Arquitetura cliente ↔ servidor desejada no futuro

A arquitetura ideal deveria evoluir para algo semelhante a:

```text
Shared/
├── protocol/
├── data_contracts/
├── serialization/
└── common_types/

Client/
├── presentation
├── prediction
└── input

Server/
├── authority
├── validation
├── persistence
└── simulation
```

Atualmente algumas responsabilidades atravessam essas fronteiras.

A reorganização física feita agora é o primeiro passo para poder corrigir isso sem trabalhar num monólito desorganizado.

---

# 33. Histórico resumido das migrations servidor

## 127–141 — fundação

Incluiu:

- Database;
- SQL;
- Firewall;
- GameMasters;
- CLI;
- News;
- Mouse;
- Resources;
- Skills;
- TJBOY integrity;
- Clanmenu;
- MicVolume;
- video;
- WinInt;
- Sound.

Durante essa fase foram descobertas as fronteiras problemáticas:

```text
Shared/sinItem
Shared/smwsock
NetType
_W_SERVER
```

---

## 142–156 — sistemas server-side claros

Incluiu:

- restante TJBOY/Ygy;
- smLib3d;
- messages;
- authentication protocol;
- Joi stub;
- loot alerts;
- PacketFilter;
- server compat;
- BattlePass;
- ClanCustom;
- CoinShop;
- QuestHandler;
- RestaureHandler;
- VipHandler;
- HellsGate.

---

## 157–171 — periféricos da raiz

Incluiu:

- TextMessage;
- FontImage;
- AVI;
- CurseFilter;
- Damage;
- Rank;
- FullZoomMap;
- IME/STRCLASS;
- BellatraFontEffect;
- Drawsub;
- Particle;
- background;
- shadow;
- effectsnd;
- smReg.

---

## 172–186 — HoBaram

Removeu `HoBaram` como fronteira arquitetural C/C++.

Após 186:

```text
HoBaram C/C++ = 0
267 ficheiros restantes
```

---

## 187–201 — sinbaram

Preparado para dividir:

- inventory;
- commerce;
- quests;
- premium;
- status;
- help;
- Bellatra;
- skills;
- effects;
- events;
- messages;
- HUD;
- pets;
- media compat;
- Sin runtime.

Resultado esperado:

```text
267 → 176
sinbaram C/C++ → 0
```

---

## 202–216 — Language + legacy root

Preparado para:

- localization config;
- JTS;
- locales;
- camera;
- JOBS;
- timer;
- sinHaQuest.

Resultado esperado:

```text
176 → 57
```

---

## 217–234 — núcleo final

Preparado para:

- network transport;
- protocol;
- loaders;
- persistence;
- field;
- AreaServer;
- play support;
- ActionGame;
- skills;
- character;
- netplay;
- playmain;
- security;
- resources;
- server damage;
- events;
- server runtime;
- Winmain.

Resultado esperado:

```text
57 → 0
```

---

# 34. Coisas que NÃO devem ser feitas imediatamente após fechar a migração

Não começar imediatamente a:

```text
renomear classes
apagar código
fundir cliente e servidor
fundir opcodes
fundir Lua
remover globals
modernizar sockets
trocar database API
remover DirectX
```

Primeiro:

```text
terminar migrations
→ clean build cliente
→ clean build servidor
→ validar integração
→ soak
→ congelar baseline funcional
```

Depois começa uma nova fase.

---

# 35. Dívidas técnicas prioritárias após validação

## Prioridade 1 — `Shared`

Auditar header por header:

```text
o que é realmente compartilhado?
o que seleciona implementação por lado?
o que deveria ter interface comum?
```

---

## Prioridade 2 — protocolos/opcodes

Existem vários universos de opcode.

É necessário documentar:

```text
client-only
server-only
shared
legacy
```

antes de tentar consolidar.

---

## Prioridade 3 — `_SERVER` / `_W_SERVER`

Documentar claramente:

```text
onde cada macro nasce
o que controla
qual deve ser usado
```

O problema de `NetType` mostrou que esse mecanismo é crítico.

---

## Prioridade 4 — economia

Auditar todas as transações.

---

## Prioridade 5 — client/server stubs e compat

Comparar com implementações autoritativas.

---

## Prioridade 6 — código cliente compilado no servidor

Determinar o que pode ser removido com segurança.

---

## Prioridade 7 — globais

Reduzir dependências ocultas de inicialização/link order.

---

## Prioridade 8 — loaders e persistência

Criar interfaces claras entre domínio e storage.

---

## Prioridade 9 — dead code

Só remover depois de verificar:

```text
build
link
exports
runtime
scripts
reflection
packets
```

---

# 36. Como trabalhar com o servidor daqui para frente

## Regra 1

Sempre identificar se uma alteração pertence a:

```text
server authority
client presentation
shared contract
compatibility
```

antes de editar.

## Regra 2

Nunca assumir que um header homônimo representa a mesma coisa.

## Regra 3

Auditar `Shared` sempre que um header público for movido.

## Regra 4

Considerar todos os include roots reais do build.

## Regra 5

Executar clean build depois de mudanças de fronteira.

## Regra 6

Separar relocation de refactor.

## Regra 7

Manter rollback local.

## Regra 8

Não confiar em objetos incrementais como prova de consistência.

## Regra 9

Alterações em economia, combate ou persistência exigem testes funcionais específicos.

## Regra 10

Qualquer dado vindo do cliente é não confiável até validação server-side.

---

# 37. Conclusão

A arquitetura original do servidor sofre do mesmo problema histórico do cliente, mas com uma dificuldade adicional:

> O servidor está fortemente acoplado ao código do cliente e às estruturas compartilhadas.

A migração revelou simultaneamente:

```text
sistemas realmente server-side
código gráfico legado
bibliotecas compartilhadas
duplicações de headers
protocolos sobrepostos
macros de compilação dependentes de include order
paths hardcodeados
dependências ocultas por include roots
objetos incrementais mascarando problemas
```

As falhas encontradas durante clean build não foram apenas obstáculos.

Elas revelaram onde a arquitetura antiga efetivamente dependia de:

```text
ordem de include
homônimos
macros implícitas
cache de objetos
paths históricos
```

Ao final da Migration 234, caso todos os gates compilem, o servidor deixa de ser uma coleção de pastas históricas e passa a estar organizado por responsabilidade.

Nesse ponto, a pergunta:

> “Onde está o código do servidor?”

deixa de ser respondida com:

```text
talvez em Server
talvez em sinbaram
talvez em HoBaram
talvez em TJBOY
talvez na raiz
```

e passa a ser respondida por domínio:

```text
persistence
security
network
combat
gameplay
commerce
social
admin
platform
compat
```

Essa é a base necessária para a próxima fase: validar funcionalmente cliente e servidor juntos e, depois, começar a remover a dívida técnica em vez de apenas reorganizá-la.