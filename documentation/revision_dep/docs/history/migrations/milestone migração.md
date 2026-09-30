Preparei a Parte 1 como um documento de referência operacional e arquitetural. Registrei inclusive a diferença entre “intenção histórica” e “estado real da árvore”, porque o caso do `vtablehook.h` mostrou que isso é essencial para não confiar cegamente no histórico.

# Migração SourceCode — Parte 1

## Processo de Migração, Arquitetura Descoberta, Dependências e Validação

**Projeto:** SourceCode / cliente legado C/C++
**Diretório principal:** `/home/gabriel/Área de Trabalho/SourceCode`
**Diretório dos pacotes de migração:** `/home/gabriel/Área de Trabalho/migration`
**Escopo desta parte:** processo usado para criar as migrações, arquitetura já descoberta, relacionamento entre os componentes, método de rollback, build, auditoria e testes funcionais.

---

# 1. Objetivo da migração

O objetivo não é simplesmente mover arquivos para pastas mais bonitas.

A migração está transformando uma árvore de código legado, historicamente organizada por nomes antigos, autores, bibliotecas e agrupamentos pouco claros, em uma arquitetura baseada em responsabilidade.

Exemplos de fronteiras antigas que estão sendo eliminadas:

* `TJBOY`
* `Ygy`
* `Hook`
* `WinInt`
* `Sound`
* `Settings`
* `Security`
* `Login`
* `BattleEvents`
* `Resources`
* `smLib3d`

As novas fronteiras são orientadas ao papel real do código:

```text
SrcGame/src/
├── systems/
│   ├── audio/
│   ├── authentication/
│   ├── combat/
│   ├── configuration/
│   ├── gameplay/
│   ├── rendering/
│   ├── security/
│   ├── social/
│   └── ui/
├── platform/
│   └── windows/
├── compat/
│   └── legacy/
└── diagnostics/
```

A regra principal é:

> A localização física de um arquivo deve comunicar sua responsabilidade no sistema.

A migração atual é predominantemente **estrutural**. Ela não deve alterar comportamento do jogo de propósito.

---

# 2. Escopo atual: cliente primeiro

A migração está sendo feita primeiro no cliente.

O servidor não será reorganizado apenas porque arquivos antigos aparecem nele.

O diretório:

```text
SrcGame/src/Server
```

foi inspecionado e contém um bloco legado fortemente misturado.

Foram encontrados 19 arquivos, incluindo:

```text
AutoDropItem.h
BlessCastle.h
ClientFuncPos.cpp
ClientFuncPos.h
DllServer.cpp
DllServer.h
E_serverlog.h
onserver.h
OnSever.cpp
PartyMsg.h
PartySever.cpp
ServerMsg.h
Set_EventSpawn.h
svr_Damage.h
Svr_Damge.cpp
svrEventGame.h
svrEventSpawn.cpp
svrEventSpawn.h
svrHaQuest.h
```

Somente `DllServer.cpp` estava explicitamente presente na lista ativa de compilação do cliente durante a inspeção.

Entretanto, `DllServer.h`, `onserver.h` e outros arquivos possuem dependências internas suficientes para tornar uma movimentação isolada artificial.

Decisão atual:

> `Server` fica temporariamente congelado.

Ele somente será reorganizado quando sua responsabilidade puder ser separada de forma coerente.

---

# 3. O que chamamos de “patch”

Nesta migração, o patch não é um `git diff`.

Cada alteração é empacotada como uma **migration executável e reversível**.

Formato típico:

```text
/home/gabriel/Área de Trabalho/migration/
└── migration_028_client_ui_core/
    ├── install.sh
    ├── audit.sh
    ├── rollback.sh
    └── .state/
        ├── modified_files.txt
        └── snapshot/
```

A pasta `.state` é criada durante a instalação.

Cada migration possui três responsabilidades claramente separadas.

### `install.sh`

Faz a transformação:

1. valida o estado inicial;
2. determina arquivos que serão alterados;
3. cria snapshot;
4. move arquivos;
5. corrige includes;
6. atualiza listas de compilação;
7. atualiza include paths do build quando necessário;
8. chama a auditoria.

### `audit.sh`

Não modifica o source.

Ele verifica se a transformação terminou no estado esperado.

Exemplos:

```text
arquivo novo existe?
arquivo antigo desapareceu?
include antigo ainda existe?
quantas entradas existem na cpp list?
o include root antigo foi removido?
algum arquivo inativo entrou por engano no build?
```

### `rollback.sh`

Restaura exatamente os arquivos salvos antes da migration.

O rollback não tenta reconstruir o estado anterior por lógica inversa.

Ele usa uma cópia real dos arquivos anteriores.

---

# 4. Git deliberadamente não participa das migrations

Uma decisão operacional importante foi eliminar Git do processo de migração.

As migrations atuais não devem:

```text
consultar HEAD
usar git status
usar git diff
usar git add
criar index temporário
depender de staging
criar commits
restaurar arquivos usando Git
```

O rollback depende exclusivamente do snapshot local da própria migration.

Isso torna cada migration autocontida:

```text
migration_X/
└── .state/snapshot/
```

O princípio é:

> Uma migration deve ser capaz de desfazer somente aquilo que ela própria modificou.

Arquivos não relacionados não devem ser afetados pelo rollback.

---

# 5. Segurança dos comandos executados no terminal

Existe uma distinção importante entre comandos fornecidos diretamente para colar no terminal e scripts criados dentro de uma migration.

## Comandos colados diretamente

Os blocos destinados a serem colados no terminal interativo evitam:

```bash
exit
exit 1
set -e
set -euo pipefail
```

Isso é intencional porque um comando que finalize o shell pode fechar a sessão de terminal utilizada durante a migração.

## Scripts da migration

Arquivos executados como processos filhos, como:

```text
install.sh
audit.sh
rollback.sh
```

podem utilizar:

```bash
set -euo pipefail
```

e:

```bash
exit 1
```

porque a falha termina apenas aquele script.

Essa diferença deve ser preservada nas próximas partes da migração.

---

# 6. Snapshot antes da transformação

Antes de modificar qualquer arquivo, `install.sh` identifica quais arquivos serão tocados.

Esses arquivos são copiados para:

```text
migration_X/.state/snapshot/
```

mantendo o caminho relativo original.

Exemplo:

```text
.state/snapshot/
├── cpp_files_client_i686.txt
└── SrcGame/
    └── src/
        ├── UIText.cpp
        └── Hook/
            └── vtablehook.h
```

O manifesto:

```text
.state/modified_files.txt
```

registra quais arquivos pertencem ao snapshot.

Esse mecanismo resolve um problema importante de migrations grandes:

> O rollback não deve desfazer mudanças que não pertencem àquela migration.

---

# 7. Estado real da árvore é a autoridade

Um princípio importante surgiu durante a migração do `vtablehook.h`.

Historicamente, a Migration 023 havia sido considerada responsável por mover:

```text
Hook/vtablehook.h
→ compat/legacy/hooks/vtablehook.h
```

Entretanto, uma inspeção posterior revelou que o estado real era:

```text
SrcGame/src/Hook/vtablehook.h
```

existia, enquanto:

```text
SrcGame/src/compat/legacy/hooks/vtablehook.h
```

não existia.

Além disso, nenhuma referência externa utilizava o header.

Isso levou à Migration 027, que corrigiu o estado real.

Regra estabelecida:

> Histórico de migrations ajuda a entender intenção, mas o filesystem e o código atuais são a autoridade.

Antes de qualquer movimentação importante devem ser inspecionados:

* arquivos reais;
* consumidores;
* build ativo;
* include paths;
* dependências;
* cópias duplicadas.

---

# 8. Não misturar mudança estrutural com refactoring

Uma migration de diretório não deve aproveitar a oportunidade para também:

* renomear classes;
* renomear funções;
* modernizar APIs;
* corrigir estilo;
* alterar regras do jogo;
* alterar algoritmos;
* remover código aparentemente estranho;
* trocar estruturas de dados.

Exemplo:

```text
smLib3d/smRend3d.cpp
```

foi movido mantendo:

```text
smRend3d.cpp
```

O fato de o nome ser antigo não é motivo para renomeá-lo durante a movimentação.

O objetivo é reduzir o número de variáveis quando surgir um problema.

Se build ou runtime quebrar, precisamos saber que a mudança relevante foi essencialmente:

```text
PATH A → PATH B
```

e não dezenas de refactorings simultâneos.

---

# 9. Regra de uma responsabilidade por migration

Cada migration deve representar uma unidade arquitetural compreensível.

Exemplos válidos:

```text
systems/security/anticheat
systems/audio/playback/legacy
systems/ui/confirmation
systems/combat/pvp
systems/rendering/font
systems/gameplay/loot/roll
platform/windows/resources
```

Evita-se criar uma migration que mova simultaneamente, por exemplo:

```text
PvP
+ áudio
+ configuração
+ networking
```

Mesmo que tecnicamente seja possível.

Essa separação facilita:

* auditoria;
* rollback;
* diagnóstico;
* teste funcional;
* documentação;
* localização de regressões.

---

# 10. Estratégia usada para escolher a próxima migration

Antes de mover um diretório ou grupo de arquivos, geralmente são levantadas quatro informações:

### Conteúdo físico

```bash
find ...
```

Serve para descobrir arquivos reais, backups, assets e subdiretórios.

### Consumidores

São pesquisados todos os `#include` dos headers daquele módulo.

Isso mostra quem depende da fronteira atual.

### Participação no build

O principal arquivo observado é:

```text
cpp_files_client_i686.txt
```

Esse arquivo define quais `.cpp` entram no build ativo do cliente.

### Include roots do compilador

Também é inspecionado:

```text
build_client_mingw_i686_multicore.sh
```

para encontrar parâmetros como:

```text
-ISrcGame/src/AlgumDiretorio
```

Isso é importante porque um código pode compilar devido a um include root implícito mesmo sem usar um caminho claro.

---

# 11. Reescrita de includes

A arquitetura nova prefere includes a partir da raiz lógica:

```text
SrcGame/src
```

Por exemplo:

```cpp
#include "systems/ui/core/UIButton.h"
```

em vez de:

```cpp
#include "../../../UIButton.h"
```

ou:

```cpp
#include "../UI/UIButton.h"
```

ou:

```cpp
#include "UIButton.h"
```

quando o arquivo pertence a uma fronteira arquitetural explícita.

O objetivo é fazer o próprio include documentar a dependência.

---

# 12. Diferentes formas de caminho legado precisam ser detectadas

O caso do `smLib3d` mostrou que a mesma dependência aparecia sob muitas formas:

```cpp
#include "smLib3d/smd3d.h"
#include "../smLib3d/smd3d.h"
#include "../../smLib3d/smd3d.h"
#include "./smLib3d/smd3d.h"
```

Portanto, uma substituição literal simples não é suficiente em migrations maiores.

As migrations atuais usam Python para:

1. ler cada diretiva `#include`;
2. normalizar `\` para `/`;
3. identificar o basename;
4. reconhecer diferentes caminhos legados;
5. gerar o caminho canônico.

Isso reduz a possibilidade de deixar um consumidor esquecido.

---

# 13. Encoding do source

Uma descoberta importante é que nem todo o source pode ser tratado como UTF-8.

Existem arquivos legados compatíveis com Latin-1 / Windows-1252.

Por isso, as migrations evitam fazer:

```python
path.read_text()
```

indiscriminadamente.

Para substituições puramente ASCII, a abordagem preferida é:

```python
data = path.read_bytes()
...
path.write_bytes(data)
```

Quando o conteúdo precisa virar texto para análise:

```python
decode("latin-1")
```

é preferível nos arquivos legados.

Essa regra é importante para evitar uma migration aparentemente inocente modificar bytes de comentários, strings ou caracteres acentuados.

---

# 14. Build ativo do cliente

A cadeia de validação principal do cliente possui três componentes importantes.

## Lista de `.cpp`

```text
cpp_files_client_i686.txt
```

Toda migration que move um `.cpp` ativo precisa atualizar essa lista.

A auditoria costuma verificar contagem exata.

Exemplo:

```text
13 caminhos novos
0 caminhos antigos
```

é muito melhor que apenas verificar se “algum caminho novo existe”.

## Script de build

Há dependências relevantes em:

```text
build_client_mingw_i686_multicore.sh
```

especialmente include roots `-I`.

Um include root antigo deve ser removido quando sua fronteira deixa de existir.

## Comando utilizado para build

A validação executada pelo usuário é:

```bash
cd "/home/gabriel/Área de Trabalho/SourceCode"
sh ./cliente.sh
```

Depois de um build válido:

```bash
sh ./send_to_client.sh
```

envia o cliente para o ambiente utilizado no teste real.

---

# 15. Quando uma migration é considerada concluída

Uma migration não é considerada pronta só porque `install.sh` terminou.

O fluxo completo é:

```text
1. snapshot
2. install
3. audit OK
4. cliente.sh
5. compilação/link sem erro
6. send_to_client.sh
7. execução real do cliente
8. teste funcional relevante
```

O build verifica principalmente integridade estrutural e linkagem.

Ele não consegue confirmar sozinho que o comportamento em runtime continua correto.

---

# 16. O que o runtime testa que o compilador não testa

Alguns problemas somente surgem quando o cliente executa:

* ponteiro inválido;
* recurso não encontrado;
* textura não carregada;
* código de inicialização não executado corretamente;
* interação de UI quebrada;
* fluxo de rede incorreto;
* efeito gráfico incorreto;
* erro dependente de estado;
* callback não disparado;
* crash apenas ao abrir determinada tela;
* pacote recebido mas interpretado incorretamente.

Portanto:

> “Compilou” não significa “migration validada”.

O build é a primeira barreira.

O runtime é a segunda.

---

# 17. Arquitetura descoberta até agora

## 17.1 Authentication

Destino:

```text
systems/authentication/
```

O antigo `Login` foi movido para esse domínio.

O login também interage com componentes de segurança.

Teste funcional importante:

```text
abrir cliente
→ executar login
→ autenticar
→ entrar normalmente
```

---

## 17.2 Security

Estrutura construída:

```text
systems/security/
├── anticheat/
├── client_integrity/
└── integrity/
```

Foram separados conceitos anteriormente espalhados por:

```text
Security
TJBOY/isaocheck
AntiCheat
```

Os testes relevantes não devem buscar burlar o sistema.

Devem verificar que uso legítimo do cliente não provoca:

* crash;
* falso positivo;
* kick incorreto;
* falha de login.

---

# 18. Sistema social / clan

O antigo:

```text
TJBOY/clanmenu
```

foi levado para:

```text
systems/social/clan
```

Esse bloco possuía dezenas de arquivos e representa funcionalidade social real, não uma biblioteca genérica.

Testes eventualmente relevantes:

* abrir interface de clan;
* visualizar membros/listas;
* usar chat relacionado;
* abrir menus;
* navegar pelos estados da interface.

---

# 19. Áudio e voice

A organização atual começou a separar responsabilidades diferentes que anteriormente estavam misturadas.

## Playback legado

```text
systems/audio/playback/legacy
```

veio do antigo `Sound`.

Contém código relacionado a WAV e reprodução.

Teste:

* sons normais;
* efeitos;
* reprodução WAV;
* mudança de mapa/cenário com áudio.

## Voice / microfone

```text
systems/audio/voice/
```

inclui partes vindas de:

```text
TJBOY/park/MicVolume
TJBOY/Ygy
```

Foi identificado um mecanismo legado composto por:

```text
memmap
Packet
PacketStruct
Process
```

Existe comunicação por memória compartilhada/IPC e estruturas de pacote.

Esses componentes foram mantidos juntos por apresentarem acoplamento funcional.

Testes, caso o recurso esteja ativo:

* inicialização do voice;
* volume do microfone;
* troca de estado;
* comunicação de voz;
* entrada/saída sem travamento.

---

# 20. Download de áudio legado

O antigo:

```text
WinInt
```

foi reorganizado como:

```text
systems/audio/download/legacy_http
```

Inclui componentes relacionados a download HTTP e WAV.

Teste funcional eventual:

* situação que exige baixar áudio;
* conclusão do download;
* reprodução;
* ausência de travamento durante download.

---

# 21. Rendering / fontes

Foram identificadas responsabilidades diferentes anteriormente misturadas.

## Font

```text
systems/rendering/font/
```

recebeu:

```text
Font.cpp
Font.h
newFont.cpp
newFont.h
```

Uma descoberta foi que:

```text
Font.cpp
```

estava vazio e `Font.h` praticamente vazio.

A implementação realmente significativa está principalmente associada a `newFont`.

Teste importante:

* textos da UI;
* menus;
* chat;
* textos sobre personagens;
* fontes em diferentes telas.

---

# 22. Hook de renderização/fonte

`Hook.cpp/.h` foi classificado como código relacionado a interceptação GDI/D3DX/font, não como anticheat.

Destino:

```text
systems/rendering/font/legacy_gdi_hook
```

Esse detalhe é arquiteturalmente importante.

O nome “Hook” sozinho não comunica responsabilidade.

Mover simplesmente para:

```text
systems/security/hook
```

teria classificado o código incorretamente.

---

# 23. `vtablehook.h`

Existe também um helper independente:

```text
vtablehook.h
```

com implementação para alteração de entradas de vtable e proteção de memória.

Após inspeção foi confirmado que:

* não possuía consumidores;
* não estava no build;
* era a única cópia existente;
* o caminho canônico esperado historicamente não existia.

Ele foi então tratado como compatibilidade legada:

```text
compat/legacy/hooks/vtablehook.h
```

Como não existe consumidor ativo conhecido, sua validação é essencialmente estrutural.

Não há feature específica conhecida no jogo para testar esse header.

---

# 24. Motor 3D legado — `smLib3d`

Esse foi um dos maiores blocos migrados até agora.

Destino:

```text
systems/rendering/legacy_3d
```

A inspeção mostrou que `smLib3d` não era um conjunto aleatório.

Ele representa um motor/renderizador 3D legado fortemente acoplado.

Entre os componentes foram encontrados:

```text
smd3d.h
smDsx
smGeosub
smJpeg
smMap3d
smmatrix
smObj3d
smRead3d
smRend3d
smSin
smStage3d
smStgObj
smTexture
smType
```

O header:

```text
smd3d.h
```

funciona como um grande umbrella header para diversos componentes do motor.

Isso explica por que dezenas de arquivos do cliente dependiam dele.

Decisão:

> Não separar internamente `smLib3d` ainda.

Fazer isso durante a migração estrutural aumentaria muito o risco.

Primeiro o bloco inteiro ganhou uma fronteira arquitetural correta.

A subdivisão poderá acontecer em outra fase.

---

# 25. Comunicação dentro do motor 3D

O mecanismo predominante encontrado é compile-time coupling por headers.

`smd3d.h` agrega partes como:

```text
smDsx.h
smSin.h
smType.h
smTexture.h
smObj3d.h
smStgObj.h
smStage3d.h
smrend3d.h
smMap3d.h
smRead3d.h
smmatrix.h
smGeosub.h
smJpeg.h
```

Muitos `.cpp` do motor incluem diretamente esse umbrella.

Também existem consumidores externos em:

```text
root do cliente
HoBaram
sinbaram
Server
systems já migrados
```

Isso mostra que o motor 3D é atualmente uma infraestrutura transversal.

Ele não pode ainda ser considerado um módulo isolado com uma API estreita.

---

# 26. Dependência inesperada: rendering e packets

`smType.h` utiliza:

```text
Shared/smPacket.h
```

e também `NetType.h`.

Isso demonstra uma característica comum do código legado atual:

> fronteiras técnicas ainda se misturam.

Estruturas utilizadas pelo rendering podem depender de tipos compartilhados ou estruturas de comunicação.

Isso não deve ser “corrigido” durante a movimentação de diretórios.

Primeiro essa dependência deve ser tornada explícita.

Depois ela pode ser avaliada arquiteturalmente.

---

# 27. `FilterEffect`

Dentro de `smLib3d` foi encontrado:

```text
FilterEffect.cpp
FilterEffect.h
```

O código contém suporte a efeitos como:

```text
motion blur
bright/contrast
```

Entretanto, `FilterEffect.cpp` não estava presente na lista ativa de `.cpp` do cliente.

Também não foram encontrados consumidores externos relevantes.

Decisão:

* preservar o source;
* mover junto com o motor;
* **não adicionar ao build**.

Essa regra é importante:

> Uma migration não deve ativar código apenas porque o encontrou.

---

# 28. `smTexture_.h`

Outro arquivo encontrado:

```text
smTexture_.h
```

não possuía consumidores identificados.

Ele também foi tratado como source legado pertencente ao mesmo bloco e preservado sem ativação.

Código dormente e código inútil não são automaticamente a mesma coisa.

Remoção definitiva deve acontecer em uma etapa própria.

---

# 29. `ddraw.h`

Existe um arquivo chamado:

```text
smLib3d/ddraw.h
```

mas também existem includes no código como:

```cpp
#include <ddraw.h>
```

Essa forma sugere dependência de header Windows/toolchain.

Por segurança, esses includes não foram convertidos automaticamente para:

```cpp
#include "systems/rendering/legacy_3d/ddraw.h"
```

Isso evita transformar silenciosamente um include de sistema em um include local.

Esse caso mostra por que substituição baseada somente no nome do arquivo é perigosa.

---

# 30. Backup encontrado dentro do source

Dentro de `smLib3d` existia:

```text
smDsx.cpp.broken_windowed_800x600_wine_font
```

Esse arquivo não foi tratado como source ativo.

Na migração do motor:

* ele não foi levado para a nova arquitetura;
* foi preservado no snapshot;
* foi removido da árvore arquitetural ativa.

Regra futura:

> Backups e artefatos históricos não devem ganhar automaticamente uma casa dentro da nova arquitetura.

Se forem importantes, devem ser tratados explicitamente como backup/documentação, e não como código ativo.

---

# 31. UI base

Foi identificado no root um conjunto altamente coeso:

```text
UIButton
UICheckbox
UICombobox
UIElement
UIHorizontalScroll
UIImage
UIInput
UIItemFrame
UILinker
UIList
UIListItem
UIPlaceholder
UIText
UITitle
UIVerticalScroll
UIWindow
```

O conjunto possui 29 arquivos entre `.cpp` e `.h`.

Treze `.cpp` estavam ativos na lista do cliente.

Foi proposta a fronteira:

```text
systems/ui/core
```

## Comunicação interna

Uma característica extremamente clara é que praticamente todos os `.cpp` desse conjunto incluem:

```cpp
#include "UILinker.h"
```

`UILinker.h`, por sua vez, liga os diversos componentes da UI.

Isso faz dele uma espécie de ponto central/umbrella da camada de UI legada.

## Consumidores externos identificados

Entre eles:

```text
CChatBox.h
cSettingsWindow.h
systems/gameplay/loot/roll/CRollWindow.h
```

A Migration 028 foi preparada para mover esse conjunto, mas nesta documentação ainda não existe confirmação registrada do resultado de build/runtime da 028.

Portanto seu estado deve ser tratado como:

```text
PREPARADA / AGUARDANDO VALIDAÇÃO REGISTRADA
```

e não como concluída.

---

# 32. Settings

O antigo diretório `Settings` foi separado em duas responsabilidades.

## Configuração

```text
systems/configuration/client
```

contém o núcleo de settings.

Teste:

```text
alterar opção
→ salvar
→ fechar
→ reabrir
→ confirmar persistência
```

## UI de settings

```text
systems/ui/settings
```

contém a interface correspondente.

Teste:

* abrir settings;
* navegar;
* alterar opções;
* aplicar;
* fechar;
* reabrir.

Essa separação é um exemplo importante do objetivo arquitetural:

> armazenamento/configuração e interface não precisam pertencer ao mesmo diretório.

---

# 33. UI de confirmação

Destino:

```text
systems/ui/confirmation
```

Veio de:

```text
ConfirmationBox
```

Teste funcional:

* abrir caixa de confirmação;
* aceitar;
* cancelar;
* confirmar fechamento correto.

---

# 34. Message box

`HoMessageBox` foi separado do grande bloco `HoBaram` e movido para:

```text
systems/ui/messages
```

Isso demonstra a estratégia que será usada em `HoBaram`:

> não mover o diretório inteiro; extrair responsabilidades identificáveis.

Teste:

* provocar mensagens in-game;
* verificar texto;
* layout;
* fechamento;
* ausência de crash.

---

# 35. PvP

O antigo:

```text
PvP/Score.*
```

foi movido para:

```text
systems/combat/pvp
```

Teste:

* entrar em contexto PvP;
* observar score;
* atualização de valores;
* renderização da informação.

---

# 36. Loot / Roll

`BattleEvents` continha um conjunto relacionado ao sistema de Roll/Skip de itens.

Foi reorganizado como:

```text
systems/gameplay/loot/roll
```

Entre os arquivos:

```text
CRollHandler
CRollWindow
roll.h
```

Essa migration revelou um tipo de teste funcional que deve ser registrado daqui em diante.

Não basta “jogar alguns minutos”.

O teste ideal é:

```text
provocar um evento de item que abra Roll/Skip
→ verificar abertura da janela
→ verificar botões
→ interagir
→ verificar fechamento
→ observar resultado
→ observar possível fluxo de packet
```

Mesmo quando não for possível provocar facilmente o evento, essa recomendação deve permanecer documentada para testes futuros.

---

# 37. Recursos Windows

O antigo:

```text
Resources
```

continha:

```text
ds.ico
resource.h
Script1.aps
Script1.rc
```

Destino:

```text
platform/windows/resources
```

Isso foi classificado como integração de plataforma, não sistema de gameplay.

O `.rc` manteve referências locais como:

```text
resource.h
ds.ico
```

onde apropriado.

Teste eventual:

* inicialização do executável;
* ícone;
* resources do executável;
* metadados/version info, quando utilizados;
* ausência de erro de resource no build/link.

---

# 38. Diretórios legados ainda relevantes

No inventário mais recente foram encontrados grandes blocos ainda não reorganizados.

## `Language`

Aproximadamente:

```text
111 source files
1 cpp
110 headers
```

Provavelmente representa uma grande área de localização/textos, mas ainda precisa ser inspecionada antes de qualquer classificação definitiva.

Não deve ser movida cegamente.

## `Library`

Aproximadamente:

```text
53 source files
31 cpp
22 headers
```

O nome é genérico demais para virar uma fronteira nova.

Ele precisa ser decomposto por responsabilidade.

## `HoBaram`

Aproximadamente:

```text
78 source files
34 cpp
44 headers
```

É um grande agrupamento legado misto.

Já começamos a extrair responsabilidades dele, como `HoMessageBox`.

O plano é continuar fazendo extrações pequenas.

## `sinbaram`

Aproximadamente:

```text
89 source files
40 cpp
49 headers
```

Também é um bloco legado misto e não deve simplesmente virar:

```text
systems/sinbaram
```

Isso apenas renomearia o problema.

## `Server`

19 source files.

Temporariamente congelado pelas razões descritas anteriormente.

---

# 39. `Debug`

Foi encontrado um diretório:

```text
SrcGame/src/Debug
```

com aproximadamente 183 arquivos, mas nenhum `.cpp`, `.c`, `.h` ou `.hpp` no inventário utilizado.

Por isso ele não está sendo tratado como um domínio arquitetural de source.

Antes de qualquer alteração futura, será necessário identificar a natureza desses arquivos.

Eles podem ser:

* artefatos de compilação;
* dumps;
* logs;
* dados temporários;
* arquivos de IDE.

Não devem ser misturados automaticamente à migração do source.

---

# 40. Arquivos soltos no root

Esse é um dos maiores problemas estruturais restantes.

Antes da Migration 028, a lista de build indicava aproximadamente:

```text
71 .cpp ativos diretamente no root de SrcGame/src
```

Entre eles existem claramente vários domínios:

* networking;
* UI;
* gameplay;
* chat;
* quests;
* damage;
* camera;
* field/map;
* particles;
* rendering;
* ranking;
* battle pass;
* shop;
* character;
* Windows/client bootstrap.

A estratégia para o root será:

> identificar pequenos clusters coesos e removê-los progressivamente.

O primeiro grande candidato identificado foi o UI core.

Não será feita uma migration “move tudo do root”.

---

# 41. Como os sistemas se comunicam — visão atual

Ainda não foi produzido um call graph completo do cliente.

Portanto, a comunicação documentada até aqui é baseada em dependências concretas encontradas durante as migrations.

Existem pelo menos seis formas principais.

## 41.1 Includes C/C++

Essa é atualmente a forma mais visível de acoplamento.

Exemplo:

```text
CRollWindow
    ↓
UILinker
    ↓
componentes UI
```

Outro:

```text
muitos arquivos do cliente
    ↓
smd3d.h
    ↓
motor 3D
```

---

## 41.2 Umbrella headers

Dois casos importantes:

```text
smd3d.h
UILinker.h
```

Esses headers agregam diversas dependências e aumentam muito o acoplamento transitivo.

Eles não estão sendo eliminados durante esta fase.

Mas devem ser considerados candidatos a redução de dependência em uma etapa posterior.

---

## 41.3 Estruturas compartilhadas

O código usa estruturas comuns fora da pasta do sistema.

Exemplo observado:

```text
systems/rendering/legacy_3d/smType.h
    ↓
Shared/smPacket.h
```

Isso demonstra comunicação através de estruturas compartilhadas entre áreas.

---

## 41.4 Networking / packets

Partes de voice possuem:

```text
Packet
PacketStruct
Process
```

e o cliente possui ainda arquivos centrais como:

```text
netplay
smwsock
opcodes
NetType
```

A topologia completa de networking ainda não foi mapeada.

Essa deve ser uma etapa específica futura, porque mudar estrutura de networking sem conhecer origem/destino dos packets seria de alto risco.

---

## 41.5 IPC / memória compartilhada

O antigo Ygy utiliza:

```text
memmap
```

associado ao voice legado.

Isso indica comunicação interprocesso ou compartilhamento de memória além do fluxo normal de chamadas C++.

Esse código deve receber teste específico se a funcionalidade de voice continuar ativa.

---

## 41.6 Estado global / legado

O source antigo possui muitos sinais de arquitetura baseada em headers amplos, globals e dependência transitiva.

Uma migration estrutural não deve tentar eliminar isso imediatamente.

Primeiro:

```text
tornar fronteiras físicas claras
```

Depois:

```text
medir dependências
```

Depois:

```text
reduzir acoplamento
```

---

# 42. Matriz de testes funcionais acumulada

Além do teste genérico “não crashou”, estes são os testes recomendados por área.

| Migration / sistema          | Validação funcional recomendada                                                          |
| ---------------------------- | ---------------------------------------------------------------------------------------- |
| Crash diagnostics            | verificar geração de dump se surgir crash natural; não provocar crash desnecessariamente |
| Authentication/Login         | login completo e entrada no jogo                                                         |
| Security integrity           | login/gameplay sem falso kick ou falso positivo                                          |
| Clan                         | abrir UI, listas e chat de clan                                                          |
| Voice / MicVolume            | testar volume/microfone se feature estiver ativa                                         |
| Video                        | reproduzir vídeo quando houver fluxo acessível                                           |
| Voice IPC / Packet / Process | comunicação de voice e mudança de estados                                                |
| HTTP audio download          | baixar/reproduzir áudio caso recurso ainda seja usado                                    |
| AntiCheat                    | gameplay normal sem falso positivo                                                       |
| Sound                        | sons comuns e WAV                                                                        |
| Confirmation                 | abrir, aceitar e cancelar                                                                |
| PvP                          | observar score e atualização                                                             |
| HoMessageBox                 | provocar message box e verificar renderização                                            |
| Settings core                | alterar, salvar e recarregar configuração                                                |
| Settings UI                  | abrir, aplicar opções e navegar                                                          |
| Font                         | telas com bastante texto, chat e menus                                                   |
| Legacy GDI hook              | telas dependentes de fontes/renderização de texto                                        |
| vtablehook helper            | validação estrutural; nenhum consumidor conhecido                                        |
| Loot/Roll                    | provocar Roll/Skip, interagir e observar resultado                                       |
| Windows resources            | startup, resources, ícone/metadados                                                      |
| Legacy 3D                    | mapa, personagens, monstros, câmera, texturas, efeitos e transições                      |
| UI core                      | botões, listas, input, scroll, janela, chat e Roll                                       |

---

# 43. Teste genérico recomendado após qualquer migration

Mesmo quando não existe uma feature específica conhecida, o teste mínimo recomendado é:

```text
1. iniciar cliente
2. fazer login
3. entrar no jogo
4. permanecer alguns minutos
5. movimentar personagem
6. mudar câmera
7. abrir interfaces comuns
8. utilizar chat
9. abrir inventário/janelas relevantes
10. observar rendering e áudio
11. trocar de mapa se possível
12. verificar se não houve crash
```

Esse smoke test não substitui testes específicos, mas detecta regressões amplas.

---

# 44. Testes que devem ser feitos eventualmente

Algumas funcionalidades não são fáceis de provocar toda vez.

Por isso, é importante manter uma lista de testes pendentes e executá-los quando surgir a oportunidade.

Exemplos:

```text
Roll/Skip de loot
PvP score
voice chat
download de áudio
reprodução de vídeo
confirmation boxes específicas
message boxes específicas
fluxos raros de anticheat
mudança de mapas
efeitos gráficos incomuns
```

A ausência de crash em um teste geral é um bom sinal, mas não confirma esses caminhos raros.

---

# 45. O que fazer quando uma migration falhar

Não corrigir dezenas de arquivos manualmente sem primeiro identificar a causa.

Ordem recomendada:

```text
audit falhou
    ↓
ler exatamente qual invariant falhou
    ↓
inspecionar estado
    ↓
corrigir o pacote da migration
```

Se o estado ficou confuso ou a correção exigiria muitas intervenções:

```text
rollback.sh
```

e reconstruir a migration.

A capacidade de voltar para um estado conhecido é mais importante que “salvar” uma migration problemática a qualquer custo.

---

# 46. Invariants arquiteturais importantes

Cada migration deve tentar garantir:

```text
[ ] os arquivos esperados existem no destino

[ ] os arquivos antigos desapareceram

[ ] includes antigos desapareceram

[ ] includes novos usam caminho canônico

[ ] cpp_files_client_i686.txt aponta para caminhos novos

[ ] número de cpp ativos não mudou acidentalmente

[ ] código dormente não entrou no build

[ ] include roots antigos foram eliminados quando possível

[ ] arquivos não relacionados não foram modificados

[ ] snapshot existe antes da alteração

[ ] rollback restaura apenas a migration

[ ] audit passa

[ ] cliente compila e linka

[ ] send_to_client executa

[ ] runtime não apresenta regressão conhecida
```

---

# 47. Migrations realizadas / planejadas até aqui

## 001 — Crash diagnostics

```text
Dump
→ diagnostics/crash
```

## 002 — Authentication

```text
Login
→ systems/authentication
```

## 003 — Client integrity

```text
Security
→ systems/security/client_integrity
```

## 004 — Integrity legado

```text
TJBOY/isaocheck
→ systems/security/integrity
```

## 005 — Clan

```text
TJBOY/clanmenu
→ systems/social/clan
```

## 006 — Voice MicVolume

```text
TJBOY/park/MicVolume
→ systems/audio/voice
```

## 007 — Video

```text
TJBOY/park/ParkDXPlayer
→ systems/media/video
```

## 008 — Voice IPC

```text
TJBOY/Ygy/memmap
→ systems/audio/voice/ipc
```

## 009 — Voice legacy client

```text
TJBOY/Ygy Packet + Process
→ systems/audio/voice/legacy_client
```

## 010 — Remoção do YgyHeader umbrella legado

Includes diretos passaram a substituir o umbrella antigo.

## 011 — Ygy Util

```text
TJBOY/Ygy/Util
→ compat/legacy/ygy
```

## 012 — Download HTTP/WAV

```text
WinInt
→ systems/audio/download/legacy_http
```

## 013 — Limpeza final de referência TJBOY

Remoção de bloco morto `#if 0`.

## 014 — AntiCheat

```text
AntiCheat
→ systems/security/anticheat
```

## 015 — Sound

```text
Sound
→ systems/audio/playback/legacy
```

## 016 — ConfirmationBox

```text
ConfirmationBox
→ systems/ui/confirmation
```

## 017 — PvP

```text
PvP
→ systems/combat/pvp
```

## 018 — HoMessageBox

```text
HoBaram/HoMessageBox
→ systems/ui/messages
```

## 019 — Settings core

```text
Settings.cpp/.h
→ systems/configuration/client
```

## 020 — Settings UI

```text
Settings/UI.cpp/.h
→ systems/ui/settings
```

## 021 — Font

```text
Font/*
→ systems/rendering/font
```

## 022 — Legacy GDI hook

```text
Hook.cpp/.h
→ systems/rendering/font/legacy_gdi_hook
```

## 023 — Intenção histórica de vtablehook

Registrada como movimentação para compatibilidade, mas posteriormente verificou-se que o filesystem real não refletia esse estado.

## 024 — Loot Roll

```text
BattleEvents
→ systems/gameplay/loot/roll
```

## 025 — Windows Resources

```text
Resources
→ platform/windows/resources
```

## 026 — Legacy 3D

```text
smLib3d
→ systems/rendering/legacy_3d
```

## 027 — Correção real do vtablehook

```text
Hook/vtablehook.h
→ compat/legacy/hooks/vtablehook.h
```

Também elimina resíduo textual antigo de `Hook/Hook.h`.

## 028 — UI core

Planejada/preparada:

```text
UI*.cpp/.h selecionados
→ systems/ui/core
```

**Status nesta versão da documentação:** ainda sem resultado de build/runtime registrado.

---

# 48. Risco por tipo de migration

Uma forma útil de classificar as migrations futuras:

## Baixo risco

Exemplo:

* header não usado;
* resource isolado;
* arquivo morto;
* componente com 1–2 consumidores.

## Médio risco

Exemplo:

* UI;
* settings;
* clan;
* áudio;
* pequena feature de gameplay.

## Alto risco

Exemplo:

* rendering;
* networking;
* personagem;
* mapa;
* `HoBaram`;
* `sinbaram`;
* `Library`;
* servidor;
* estruturas compartilhadas.

Quanto maior o risco, mais detalhada deve ser a inspeção anterior.

---

# 49. Dívidas arquiteturais já identificadas

A migração revelou alguns problemas que não devem ser resolvidos imediatamente, mas precisam ficar registrados.

## Umbrella headers muito amplos

Principalmente:

```text
smd3d.h
UILinker.h
```

## Dependências entre domínios

Exemplo:

```text
rendering → Shared/smPacket.h
```

## Muitos arquivos ativos no root

Esse é um dos maiores pontos pendentes.

## Grandes diretórios históricos mistos

```text
HoBaram
sinbaram
Library
Server
```

## Networking ainda sem mapa completo

Arquivos como:

```text
netplay
smwsock
NetType
opcodes
```

merecem uma análise própria antes da reorganização.

## Arquivos dormentes

Exemplos:

```text
FilterEffect.cpp
smTexture_.h
```

Precisam ser classificados posteriormente como:

```text
necessário
legado preservado
deprecado
removível
```

mas não durante a movimentação estrutural.

---

# 50. Critério para remover código

A regra atual deve permanecer conservadora.

Um arquivo não deve ser removido apenas porque:

```text
não está na cpp list
não encontramos include
parece antigo
nome é estranho
```

Primeiro é necessário verificar:

```text
referências textuais
build scripts
includes
link
uso dinâmico
resources
eventuais #include indiretos
outras configurações
```

Somente depois é possível criar uma migration especificamente destinada à remoção.

---

# 51. Critério para considerar uma fronteira nova boa

Uma pasta nova deve responder facilmente à pergunta:

> “Que tipo de responsabilidade vive aqui?”

Bom:

```text
systems/rendering/font
systems/ui/settings
systems/combat/pvp
systems/audio/voice
platform/windows/resources
```

Ruim:

```text
systems/misc
systems/old
systems/common2
systems/tjboy
systems/hobaram
systems/sinbaram
```

A arquitetura nova não deve preservar nomes históricos sem significado técnico.

---

# 52. Direção para as próximas migrations

Depois do UI core, a estratégia recomendada continua sendo atacar clusters pequenos no root.

Possíveis áreas a investigar futuramente incluem:

```text
chat
damage
ranking
battle pass
coin shop
quests
camera
character
field/map
particles
networking
client bootstrap
```

A escolha deve sempre ser feita depois de medir:

```text
arquivos
consumidores
includes internos
cpp ativos
include roots
dependências externas
```

---

# 53. Quando atacar `HoBaram` e `sinbaram`

Eles não devem ser tratados como uma migration única.

O método deve ser:

```text
HoBaram
    ↓
identificar subresponsabilidade
    ↓
extrair
    ↓
build
    ↓
runtime
    ↓
repetir
```

O mesmo vale para:

```text
sinbaram
```

À medida que componentes claros forem removidos, o núcleo residual ficará menor e mais compreensível.

---

# 54. Quando atacar `Library`

O nome `Library` não deve sobreviver automaticamente.

Antes da movimentação é necessário descobrir se contém:

* codecs;
* estruturas;
* gráficos;
* compressão;
* arquivos de plataforma;
* rede;
* matemática;
* utilidades;
* dependências de terceiros.

O destino pode acabar sendo dividido entre:

```text
core/
platform/
compat/
third_party/
systems/
```

Mover tudo para:

```text
core/library
```

seria apenas esconder a dívida.

---

# 55. Localização / `Language`

O diretório `Language` é um caso especial porque possui muitos headers e pouquíssimos `.cpp`.

Antes de migrá-lo é necessário descobrir:

* se contém somente constantes;
* strings compiladas;
* idiomas diferentes;
* tabelas de tradução;
* seleção condicional;
* dependências por preprocessor.

Somente depois deve ser decidido se pertence a algo como:

```text
systems/localization
```

Esse nome é plausível, mas ainda não deve ser assumido sem inspeção.

---

# 56. Métrica de progresso

Uma estimativa conservadora feita durante a migração, após a Migration 024, colocava o cliente em aproximadamente:

```text
25%
```

da reorganização arquitetural pretendida.

Essa porcentagem não significa “25% dos arquivos”.

Ela representa uma estimativa de progresso arquitetural.

Grandes blocos ainda permanecem, portanto o percentual não deve aumentar proporcionalmente ao número de migrations.

Depois de concluir e validar novos blocos relevantes, deve ser feito outro inventário para recalcular essa estimativa.

---

# 57. Registro de validação funcional

A documentação final de cada migration deve possuir, sempre que possível, um campo:

```text
Validação funcional recomendada
```

e outro:

```text
Validação realizada
```

Exemplo:

```text
Migration 024 — Roll

Validação recomendada:
Provocar evento de loot Roll/Skip.

Validação realizada:
Cliente iniciou sem crash.
Feature específica ainda não provocada.
```

Isso evita transformar:

```text
“não vi crash”
```

em:

```text
“feature completamente validada”
```

São níveis de confiança diferentes.

---

# 58. Níveis de confiança sugeridos

Para documentação futura, cada migration pode usar quatro estados.

### Estruturalmente validada

```text
audit OK
```

### Build validado

```text
cliente compilou e linkou
```

### Smoke test validado

```text
cliente executou e gameplay básico funcionou
```

### Funcionalmente validada

```text
feature específica da migration foi exercitada
```

Isso permite saber exatamente quão forte é a evidência de que uma migration está correta.

---

# 59. Informação importante que deve continuar sendo registrada

Para cada migration futura, registrar:

```text
número
nome
responsabilidade
origem
destino
arquivos movidos
arquivos modificados
includes alterados
cpp list alterada?
build script alterado?
código inativo envolvido?
snapshot criado?
audit
build
send
runtime
teste funcional recomendado
teste funcional realizado
problemas encontrados
decisões especiais
rollback disponível
```

Esse registro será especialmente valioso quando começarem as migrations grandes.

---

# 60. Regra central desta migração

A estratégia adotada pode ser resumida assim:

> Inspecionar primeiro.
> Mover somente uma responsabilidade.
> Preservar comportamento.
> Tornar dependências explícitas.
> Auditar mecanicamente.
> Compilar.
> Testar no cliente real.
> Manter rollback independente.
> Confiar no estado atual do source, não em suposições históricas.

Esse processo deve continuar sendo aplicado até que os grandes agrupamentos legados deixem de representar fronteiras arquiteturais do cliente.

Essa Parte 1 já pode servir como documento de retomada caso a migração seja interrompida por bastante tempo. Na Parte 2, o mais útil será montar o **mapa técnico do cliente**, com cada subsistema, arquivos principais, dependências entre sistemas, direção dos fluxos e uma tabela de status de cada migration.

