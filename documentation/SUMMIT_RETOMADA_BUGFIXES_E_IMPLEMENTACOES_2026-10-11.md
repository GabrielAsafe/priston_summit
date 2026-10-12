# SUMMIT — Documento de retomada de bugfixes e implementações

**Data de corte:** 11 de outubro de 2026.  
**Destino:** novo chat responsável por defeitos visuais, NPCs, regressões de gameplay, performance e implementações pendentes.  
**Âmbito:** cliente e servidor do SUMMIT (Priston Tale legacy, Direct3D9, MinGW i686, Wine).  
**Estado:** documento de handoff, **não** é um patch; não afirma que os problemas estejam resolvidos.  
**Fonte da verdade:** repositório **atual** do utilizador, mais resultados de CLI e testes em jogo apresentados nesta conversa. O source inspecionado para apontar caminhos corresponde à árvore de trabalho integrada até ao PATCH 277, não substitui `SourceCode/` do utilizador.

---

## 0. Instruções essenciais para quem retomar

1. **Começar pelo bug visual branco (P0 de gameplay).** Nova informação importante: **todo o ecrã, incluindo o HUD, fica branco**. No mesmo contexto, com a câmara dirigida para a **cabeça/frente do personagem (180°)** há imagem; ao dirigir para as **costas do personagem (0°)** a imagem fica branca. O utilizador já mudou o céu sem efeito. O problema surgiu num mapa sem fog; não iniciar outro ciclo de modificações em céu/fog.
2. **Investigar os NPCs desaparecidos de Ricarten como segunda frente** (P1). Ainda não sabemos se os NPCs deixaram de ser criados/replicados ou se apenas não aparecem no render. Não relacionar automaticamente com o ecrã branco.
3. **Não duplicar a implementação do cooker ASE → SMD/SMB**. A linha de trabalho atual estava a tratar o P0 técnico de modelos, até PATCH 277. O outro chat deve conhecer o estado para não regredir os readers/serializers; pode tratar outros bugs em paralelo, coordenando alterações em ficheiros partilhados.
4. **Sem novos logs, trace, profiling flags ou modo de arranque adicional.** O utilizador pretende iniciar o jogo normalmente com `wine Game.exe`. Os únicos comandos especiais existentes são dos utilitários **offline** em `SourceCode/tools/`, não para o arranque de gameplay. Preferir alterações mínimas e testes visuais A/B reversíveis, se forem necessárias.
5. **Não modificar `SourceCode/Content/canonical/**`**. Essa área pertence a outro fluxo/chat (PT Asset Tool). Não fazer recuperação de assets, aplicar donors, mudar spawns autoritativos ou publicar novos assets por conta própria; apenas auditar e entregar evidência ao responsável.
6. **Preservar integralmente Git staged e os ficheiros locais do utilizador**. O utilizador mostrou `git status` com alterações staged sem lista completa; não executar `git reset`, `git stash`, `git add -A`, `git clean`, nem sobrepor ficheiros desconhecidos. Os patches devem fazer preflight SHA-256, backup, validação de symlinks, instalação idempotente, fail-closed e rollback.
7. **Não confundir testes sintéticos com teste Wine em jogo.** A maior parte dos regressions Python/C++ anteriores passou localmente, e cliente/servidor chegaram a compilar/linkar no PC do utilizador; isso não prova que a falha branca ou os NPCs estejam corrigidos.

## 1. Ambiente, estrutura e builds

| Item | Local/contrato |
|---|---|
| Raiz do Git | `~/Área de Trabalho/summit` |
| Fonte | `~/Área de Trabalho/summit/SourceCode` |
| Cliente de execução | `~/Área de Trabalho/summit/cliente/Game.exe` |
| DLLs cliente | `cliente/` contém DLLs do jogo e do runtime MinGW; executar sem o ambiente correto pode falhar |
| Servidor de execução | `~/Área de Trabalho/summit/Servidor/Server.exe` |
| Build integrado | `cd SourceCode && sh compboth.sh` |
| Binário de cliente compilado | `SourceCode/dist_mingw_client_i686/Game.exe` |
| Binário de servidor compilado | `SourceCode/dist_mingw_server_i686/Server.exe` |
| Patches e cópias de segurança | `bugfixes/SUMMIT_PATCH_*`, `bugfixes/SUMMIT_BUNDLE_*` |
| Cook de modelos, staging | `bugfixes/SUMMIT_NATIVE_MODELS_INCREMENTAL_263` |
| Ferramentas e testes | `SourceCode/tools/` |
| Assets sob outro proprietário | `SourceCode/Content/canonical/**` — **não editar** |

**Fato verificado pelo utilizador:** os SHA-256 de `SourceCode/dist_mingw_client_i686/Game.exe` e `cliente/Game.exe` eram idênticos (`de4b9e77a95106498ac39a158198a684175ac89b1554b2e284d3fe6399591e5b`) na comparação de 11/10. Isto excluiu a hipótese de discrepância entre esses dois ficheiros **naquele momento**; os executáveis podem entretanto ter mudado. As DLLs em `cliente/` importam para a execução Wine e não devem ser apagadas.

**Compatibilidade:** código Win32/Direct3D9, MinGW i686. O utilizador executa cliente e conversor offline no Wine (observado `wine-staging 11.0`). Alguns builds efetuados no PC mostraram 268/268 ficheiros de cliente e 299/299 de servidor, com link OK. Não assumir ausência de regressões após esses builds.

## 2. Matriz de prioridades — pendências que passam ao outro chat

| ID | Prioridade | Problema ou implementação | Evidência/estado | Critério de fecho |
|---|---|---|---|---|
| VIS-01 | **P0** | **Ecrã completamente branco dependente do ângulo da câmara** | Reproduzido pelo utilizador; HUD também desaparece; frente 180° funciona, costas 0° branco; trocar céu não altera; ocorreu sem fog no mapa | Causa localizada no pipeline e correção mínima; alternar 0°/180° sem branco, HUD preservado; smoke de efeitos e mapas |
| NPC-01 | **P1** | **NPCs de Ricarten desapareceram** | Reportado, sem classificação funcional vs visual | NPCs esperados aparecem e interagem após login, relog, warp e rotação; origem do problema documentada |
| PERF-01 | P1 | **Pausas/stutter na seleção, Ricarten, warp/map loading e primeiros efeitos** | Histórico real de gaps de ~7–8 s e uploads legacy lentos; mitigadores implementados | Comparação reproduzível do gameplay com baseline; sem novos freezes perceptíveis e sem regressão visual |
| COOK-01 | **P0 técnico, proprietário atual** | Catálogo total ASE → SMD/SMB ainda incompleto | Pilotos reais passaram; cook completo parou repetidas vezes; PATCH 277 preparado, execução completa após 277 **ainda não reportada** | `MODEL_COOK_263=PASS ... complete=True`; gate 267 passa; visual QA antes de publicar |
| COOK-02 | P1 técnico | Publicação/integração de modelos cozinhados | **Bloqueada**, staging apenas; identidade/hash e readbacks obrigatórios | Publicação controlada autorizada e validada visualmente; sem substituir originais indiscriminadamente |
| ASSET-01 | P2 | Remanescentes DDS/texturas cooked e seus fallbacks | 242 publicou 40 DDS por saída anterior; cobertura e uso efetivo não completamente demonstrados | Para hotpaths reproduzidos, DDS lido corretamente ou fallback identificado; sem overwrite de ambiguidades |
| UI-01 | P2/P3 | Textos/fontes cortados ou clipping de UI | Backlog histórico, não revalidado neste turno | Testar no ecrã real e corrigir pontualmente |
| UI-02 | P2/P3 | Comparação de equipamento no hover | Funcionalidade pendente/histórica, não provar falha atual sem teste | Hover mostra comparação correta sem conflito UI V2/legacy |
| NPC-02 | P2/P3 | Diálogos legacy/V2 NPC/Newter, comércio NPC e venda de poções por quantidade | Backlog anterior, alguns contratos já foram implementados | Smoke funcional por NPC, compra/venda e transação autoritativa |
| GAME-01 | P2/P3 | Aging, Mix, Fundição, Fabricação (implementações/integrações futuras) | Intenção/backlog histórico; **não afirmar todas quebradas** | Requisitos revisados, compatibilidade com dados reais, fluxo end-to-end aprovado |
| GAME-02 | P2/P3 | Armazém, inventário e persistência | Houve correções anteriores; smoke final não confirmado | Depósito/retirada, stacking, itens e relog sem perdas/duplicações |
| GAME-03 | P2/P3 | Combate, skills, SinEffect2 e Companions/pets/táticas | Patches antigos, smoke de combate/stress incompleto | Teste em jogo de combate prolongado sem crash, fuga de recursos ou skill dessincronizada |
| SHOP-01 | P2/P3 | CoinShop/TimeShop, premium ACK, debit/delivery e persistência | Patches 229–235, incluindo 233R e 235; não provado teste real completo | Uma entrega/um débito, sem replay/duplicação, relog correto |
| UI-03 | P3 | UI V2: estado, timers, hovers, companions, janelas, input | Módulos implementados e regressões estáticas; validação runtime por cenário em aberto | Testar gameplay, UI e regressão dos caminhos legacy |
| WORLD-01 | P3, outro proprietário em assets | Spawns/mapas/identidade NPC e localizações JSON, recursos em falta | Conteúdo canónico gerido noutro chat; duas reports de packs eram historicamente divergentes | Contratos client/server coerentes, handoff de dados ao proprietário, sem editar canonical aqui |
| STAB-01 | P3 | `Mesh Pattern Error` / warnings Wine/D3D | Histórico intermitente, sem stack ou causalidade confirmada | Reproduzir e demonstrar resolução, sem suprimir erros |
| AUDIO-01 | P3 | Troca de WAV/DirectSound e efeitos/som ambiente | Patches 247, 250, 253, 258 mitigaram; teste prolongado pendente | Audio estável nas transições/combate sem pausas ou buffers perdidos |

**Importante:** a matriz inclui **bugs observados**, **regressões por validar** e **implementações desejadas**; são categorias diferentes. A prioridade pode mudar quando o novo chat receber novas evidências. Não declarar que uma feature está quebrada apenas por constar no backlog.

---

## 3. VIS-01 — Ecrã branco dependente da orientação da câmara

### 3.1 Reprodução relatada pelo utilizador (nova informação, prioritária)

- O ecrã fica **totalmente branco**; não é uma névoa parcial nem apenas o cenário a desaparecer.
- **Desaparecem também HUD, chat/barras e a interface visível**.
- Ao olhar para a **frente/cabeça do personagem, aproximadamente 180°**, a imagem reaparece/está visível.
- Ao olhar para as **costas do personagem, aproximadamente 0°**, o ecrã torna-se branco.
- Trocar os céus não produz alterações.
- Foi observado num mapa **sem fog**. O utilizador descarta fog como explicação; fog e céu ficam **despriorizados**. Uma configuração global de D3D pode teoricamente afetar outros mapas, mas não há evidência que justifique voltar a mexer nela.
- Ainda **não existe correção, callstack, frame capture, draw call ou textura identificada** para VIS-01.

**Interpretação prudente:** a orientação de câmara condiciona o conteúdo visível ou os estados de renderização. O desaparecimento do HUD pode resultar de um draw de tela inteira **depois** da UI, de uma geometria/efeito descontrolado ou de um estado/render target que impede as passagens seguintes. **Não prova** que exista um overlay 2D nem que a falha ocorra obrigatoriamente depois do HUD; há várias explicações compatíveis.

### 3.2 Pontos de entrada concretos no source

Caminhos **relativos a `SourceCode/`**, verificados na árvore integrada de 11/10:

- `SrcGame/src/platform/windows/application/Winmain.cpp` — loop/frame principal, `DrawGameState()` e lógica de mundo/câmara. Procurar `BeginRender()`, `DrawGameState()`, `EndRender()`, posições/rotações, ordem do HUD e da geometria (regiões observadas aprox. linhas 4178, 5080–5290).
- `SrcGame/src/systems/rendering/legacy_3d/smDsx.cpp` — `InitRender()`, `BeginRender()`, `EndRender()`, `DrawTexture()`. **`BeginRender()` limpa a target a preto (`D3DCOLOR_RGBA(0,0,0,255)`)**, logo o branco não corresponde à cor padrão dessa limpeza. Verificar estados alpha/Z/blend e retornos D3D9.
- `SrcGame/src/systems/rendering/legacy_3d/smRend3d.cpp` — seleção/render de materiais, caminhos de blending/transparência e `D3DRS_ZWRITEENABLE`; vários ramos alternam `ALPHABLENDENABLE`, `SRCBLEND`, `DESTBLEND`.
- `SrcGame/src/systems/rendering/legacy_3d/FilterEffect.cpp` — render targets e filtros de ecrã (`RenderFilterEffect`, `SetFilterEffect`), incluindo `ClearViewport`. **Busca estática no snapshot não encontrou chamadas ativas a `RenderFilterEffect()`/`SetFilterEffect()` fora do próprio módulo**; verificar no source atual antes de os culpar.
- `SrcGame/src/systems/rendering/effects/legacy_sin/AssaEffect.cpp`, `AssaParticle7.cpp`, `AssaUtil.cpp`; `systems/rendering/effects/legacy_ho/core/HoEffect.cpp`; `systems/rendering/effects/legacy_new/core/HoEffect*`; `systems/rendering/effects/particles/ho/HoNewParticle*` — efeitos/partículas e materiais que podem ser desenhados ao orientar a câmara.
- `SrcGame/src/systems/ui/v2/rendering/Dx9UIRenderer.cpp` e `systems/ui/v2/core/UIRuntime.cpp` — render da UI V2; comparar ordem com HUD legacy.
- `SrcGame/src/systems/rendering/sky/legacy/HoSky.cpp` — só como referência histórica, **não primeira hipótese**.
- `Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl` — seleção de texturas DDS/fallback e estados materiais; não recriar texturas por suposição.

O source inspecionado já contém `lowHPWarningImage` e um `dsDrawTexImage(... getWidth(), getHeight(), ...)` em `Winmain.cpp` no final da fase de mundo. Isto é **um candidato de passagem full-screen a auditar**, não prova de culpa: depende de `sinGetLife()` e estado do personagem; pode ser desenhado antes do HUD. Não alterar esse efeito sem confirmar condição de execução.

### 3.3 Plano de isolamento seguro (sem novos logs nem flags)

**A. Reproduzir apenas com o jogo normal.** Na mesma posição do mapa, com o personagem parado, rodar a câmara entre os dois ângulos, observando: o branco surge imediatamente em 0°? desaparece imediatamente em 180°? há movimento de NPC, HUD ou áudio durante o branco? Manter a mesma conta, mapa, hora, efeitos e binário. Não alterar céu, fog, resolução e DDS simultaneamente.

**B. Controlos sem source:** verificar se o branco depende da presença de uma entidade, efeito ou objeto específico ao manter o personagem parado e alterar **só o ângulo**; repetir numa área próxima do mesmo mapa e noutro mapa, se seguro. Não é necessário usar comandos secretos ou abrir modos de debug.

**C. Auditoria estática por ordem de desenho:** marcar em papel/numa revisão de código os passes de mundo, sky, stage, personagens, effects/particles, HUD legacy, UI V2, cursor, `EndRender()`/`Present()`. Confirmar quais podem enviar quadriláteros em coordenadas de ecrã e quem guarda/restaura estados D3D9. Atenção a `Clear` fora de `BeginRender`, target errado, alpha blend opaco, textura branca/desconhecida, UV/face com coordenadas fora de escala, `ZWRITEENABLE`/`ZENABLE`, `D3DRS_COLORWRITEENABLE` e `SetTextureStageState` persistentes.

**D. Se a leitura estática não bastar:** patch de **isolamento visual binário A/B**, um único pass desativado por build (não via novas flags), preservando o baseline e com rollback. Começar por overlays/effects de ecrã e particulados sensíveis ao ângulo, não pelos céus. Testar primeiro regressions/compilação, depois `wine Game.exe` normal; comparar 0°/180°. Não manter alterações temporárias no código final. Se um pass for removido e a falha desaparecer, isolar dentro dele com alteração menor, até identificar o draw exato.

**E. Critério de aceitação:** branco não aparece na rotação 0°↔180° em locais anteriormente afetados; HUD permanece funcional; efeitos, transparências e objetos não ficam invisíveis ou opacos; outros mapas e seleção de personagem sem regressão.

**Evitar falsos diagnósticos:** (1) «se HUD desaparece, é necessariamente pós-processamento» — falso; (2) «o mapa não tem fog, logo nenhum estado fog global existe» — não demonstrado; (3) «trocar sky corrigirá» — já contrariado pelo teste do utilizador; (4) «é causado pelos patches de cook» — não há evidência.

## 4. NPC-01 — NPCs ausentes em Ricarten

### 4.1 Factos e incógnitas

- O utilizador comunicou o desaparecimento de NPCs de Ricarten durante os testes recentes.
- **Ainda não se sabe se continuam interativos**, se foram removidos do mundo no servidor, se a replicação falha, se o modelo/texture não aparece, se o `Field` está a usar dados de mapas incorretos ou se é um efeito do branco no ecrã.
- Os PATCHES 259–277 destinam-se sobretudo ao **cook offline**. A maioria não deve escrever NPCs, mas isso **não prova** ausência de regressão indireta em loaders, content runtime ou no build. Não culpar nem absolver uma série de patches sem comparar o source/estado real.

### 4.2 Pontos de código/documentos para ler

- `Shared/server/legacy_runtime/OnSever_legacy_server.inl` — integração de NPCs fixos e `LoadFixedNpcs` (na árvore revista aproximadamente 8600–8830). Versão cliente também em `OnSever_legacy_client.inl` (aprox. 6390–6590).
- `SrcServer/src/systems/gameplay/world/v2/legacy/LegacyNpcAdapter.cpp`, `LegacySpawnAdapter.cpp` — mapeamento das entidades legacy.
- `Shared/world/field/field_legacy_server.inl`, `Shared/world/field/field_legacy_client.inl` — world/stage e instanciação/representação.
- `SrcGame/src/systems/network/client/netplay.cpp`, `SrcGame/src/systems/gameplay/character/core/character.cpp` e `CharacterModelStreamingV3.cpp` — receção e visualização de entidades/modelos.
- `SrcGame/src/systems/gameplay/npc/ui/NPCinfo.cpp`, `SrcGame/src/systems/ui/v2/legacy/LegacyNpcDialogWindow.cpp` — UI/interação.
- Validadores existentes: `tools/validate_spawn_boundary_v1.py`, `validate_target_map_spawn_coverage_v1.py`, `validate_monster_npc_authority_v1.py`, `regression_gates/check_207_target_map_spawn_reconstruction.py`, `content_port/inspect_stage_spawns.py` (usar **somente** modos read-only autorizados).
- Dados de campo e packs compilados: autoridade em `Content/canonical` pertence ao outro chat. Em auditoria de 10/10, `Content/packs/field_runtime.sfld` e `localization.sloc` não correspondiam aos `*.report.json` associados **naquele snapshot**; pode ter sido apenas relatório desatualizado. Verificar novamente antes de inferir causa dos NPCs.

### 4.3 Teste diferencial recomendado

1. Confirmar num ponto conhecido de Ricarten se NPC visível está em falta. Registar apenas nome/posição aproximada do NPC afetado — não inventar uma lista de NPCs ausentes.
2. Tentar interação no local esperado e comparar o resultado com NPCs que aparecem noutro mapa. **Se interage mas não se vê:** priorizar render/modelo/replicação visual; **se não interage:** priorizar spawn/entidade/mapa/replicação e packs runtime.
3. Testar após sair/entrar no mapa e, se possível, após novo login. Observar se a ausência acompanha um mapa, personagem ou estado de sessão.
4. Auditar runtime pack/placement **sem escrever no canonical**. Se o defeito for de dados, reportar a outro proprietário. Se for C++ runtime, entregar um patch mínimo e teste de paridade client/server.
5. Considerar VIS-01 como possível explicação compartilhada apenas **se** a ausência variar com ângulo/estado de render e se os NPCs continuarem interativos.

**Aceitação:** NPCs que constam do conteúdo autorizado surgem e interagem em Ricarten de forma estável; mesmo resultado após warp/relogin e com HUD correto; não foram criados NPCs fictícios para passar o teste.

---

## 5. Performance, texturas cooked e efeitos — trabalho remanescente

### 5.1 O que já foi implementado ou mitigado

- PATCHES **236–242**: identidades cooked, recuperação conservadora, política de filas e preparações focadas de DDS; PATCH 242 publicou **40 DDS** conforme saída real `PUBLISH=PASS`, preservando manifest e originais. Um handoff anterior ao PATCH 242 estava desatualizado ao dizer que este não tinha sido aplicado.
- PATCH **243**: preparação global segura de texturas materiais; **244**: cooked-first de texturas usadas por efeitos; **245–246**: auditoria de hotpaths e papéis/rotas SMD/SMB.
- PATCHES **247–258**: slots de WAV e lifetimes DirectSound; guards de efeitos, caches de meshes por identidade, HoEffect assíncrono/prewarm de modelos e lookahead de texturas, limites de áudio. Foram compilações/links reportados para vários lotes; testes funcionais exaustivos permanecem pendentes.
- PATCH **260**: proteções de recursos de partículas.
- **Streaming V3**: regressões estáticas/CLI repetidamente **171/171 PASS**; isso não representa FPS medido ou ausência de defeitos visuais.

### 5.2 O que continua sem prova de fecho

- Gaps medidos historicamente perto de **7–8 segundos** em transições; uma amostra tinha maior upload de **~662 ms**. Não atribuir um gap de 8 s a um único upload de 0,66 s sem evidência temporal.
- Uploads síncronos de textura em first-use (`AssetLoadV2LoadTextureNow`, D3DX9) ainda possíveis, mesmo com DDS presente. Cache/residência, dimensões e fallback podem exigir trabalho no render thread.
- Casos de caminho ambíguo por caixa e URL lógico inválido (18 `source_ambiguous_case` e 1 `invalid_logical` na auditoria da sessão 241) não devem ser cozinhados automaticamente.
- Performance e correção visual de prewarm/efeitos, alteração de mapa, NPCs e seleção de personagem no jogo real; regressões prolongadas ainda não completamente realizadas.
- DIB/GDI legacy (clan, inventário, UI) e WAV podem permanecer síncronos; otimizar **após** identificar o callsite crítico, sem uma alteração global às cegas.

**Critério de aceitação de performance:** comparação no mesmo conteúdo, conta e movimento, com FPS e stutters observados sem regressão visual. O utilizador não quer mais trace/logs no jogo; não impor DIAG 241/239, profiling flags ou ficheiros de log novos para retomar VIS-01. Os relatórios antigos podem servir como histórico, **não** como medida atual.

## 6. Implementações e regressões de jogabilidade — fichas de trabalho

### 6.1 NPCs, comércio e poções (NPC-02)

**Objetivo:** garantir que os caminhos legacy e V2 convergem em comportamento autoritativo. Rever diálogos de NPC/Newter, apresentação da loja, compra, venda e venda por quantidade de poções. Patches anteriores 227–235 introduziram contratos e invariantes; não os sobrescrever. Usar testes `validate_npc_shop_*` em `tools/`, seguidos de transações com conta de teste. Invariantes: estoque/quantidade correta, item e moeda debitados/creditados uma vez, erro não modifica inventário, repetição de request não duplica entrega.

### 6.2 CoinShop/TimeShop e premium (SHOP-01)

**Objetivo:** fechar smoke end-to-end para ACK de compra, nonce/replay, wallet, débito/entrega e persistência após relog. **233R substituiu o 233 inicial**; 234 corrigiu header/opcode; 235 tratou identidade de compras repetidas. Distinguir `compila` de `compra validada`. Verificar cliente/servidor e SQL com dados de teste, sem inventar saldos. Não reabrir o modelo de carteira sem caso reproduzido.

### 6.3 UI V2 e usabilidade (UI-01/02/03)

**Objetivo:** fechar hovers, tooltips, comparação de equipamento, timers, status/party/companion, clipping de fontes e janelas de diálogos. Ler `SrcGame/src/systems/ui/v2/` antes de alterar `sinInterFace`/legacy: há ownership transferido para a UI V2 e regressões por desenhar duas vezes. Validar foco, input, texto em português, scaling e estado de janela ao abrir/fechar sob gameplay. Não duplicar áreas da UI legacy na V2 sem uma regra clara.

### 6.4 Aging/Mix/Fundição/Fabricação (GAME-01)

**Natureza:** lista de implementação/integração histórica, não quatro bugs confirmados. Primeira tarefa do outro chat é recolher contratos e prioridades existentes (`documentation/CONTENT_CREATION_WORKFLOWS.md`, documentação de crafting e código `SrcGame/src/systems/crafting/`). Testar compatibilidade do servidor e invariantes de itens/recursos; projetar um sistema por vez. **Não alterar formatos de conteúdo canónico** sem coordenação.

### 6.5 Armazém, inventário e persistência (GAME-02)

**Objetivo:** smoke com múltiplos stacks, depósito/retirada, space limits, desconexão/relogin, rollback em falha, inventário cheio e itens premium. Tratar qualquer novo bug com reprodução e teste; não assumir que patches antigos falharam sem evidência.

### 6.6 Combate, skills, Companions e efeitos (GAME-03 / AUDIO-01)

**Objetivo:** sessões de combate prolongadas e transições de áreas, habilidades legadas `sinEffect2`, modelos assa/HoEffect, partículas, WAV/DirectSound; inspecionar buffers e lifetimes se houver crash, stutter ou `Mesh Pattern Error`. 247–260 já contêm proteções. Provar que efeitos aparecem corretamente sem texturas opacas a tapar o ecrã (ligação potencial a VIS-01, ainda não demonstrada).

### 6.7 Spawns, mapas, localização, packs e assets (WORLD-01)

**Limite de propriedade:** outro chat gere `Content/canonical`. Bugfix runtime pode validar leitura/ABI/versão/estados de fallback, mas não reautorizar NPCs, inserir `canonical` faltante ou recuperar mapas/monstros por donors. Um snapshot de 10/10 tinha reports de packs inconsistentes para `field_runtime.sfld` e `localization.sloc`; confirmar o estado atual com o responsável antes de atribuir sintomas a isso. Não alterar céu ou fog como alternativa.

---

## 7. Cooker ASE → SMD/SMB: contexto necessário, mas proprietário separado

**Requisito inegociável do utilizador:** se um SMD ou SMB desaparecer ou ficar corrompido, é indispensável **reconstruí-lo integralmente a partir do ASE original**. Não fechar este trabalho através da reutilização obrigatória de caches antigas. Todos os artefactos devem passar por validação nativa do servidor e do cliente e ser publicados primeiro num staging isolado.

### 7.1 Arquitetura existente

- `Server.exe --summit-cook-model-one mesh|bone <relative.ASE>` faz conversão nativa offline, gerando SMD/SMB em scratch.
- `Game.exe --summit-verify-model mesh|bone <relative.smd|smb>` verifica o readback nativo no cliente.
- `SourceCode/tools/cook_native_models_incremental_263.py` calcula identidades SHA-256 de ASE, `Server.exe`, `Game.exe`, cooker e resultados das auditorias; reconverte apenas quando necessário; usa staging externo e faz fail-closed/rollback quando há erro.
- `SourceCode/tools/verify_native_model_stage_267.py` valida cobertura e artefactos. **Só faz sentido esperar PASS se o cook completo tiver produzido `complete=True` com identidades atuais.**
- `--client-exe` (PATCH 272) seleciona binário de verificação, mas não remove necessidade de DLLs no ambiente de execução.
- PATCH 277 permite **uma repetição adicional por timeout real** em scratch novo. Não repete parser error/hash mismatch; preserva staging quando ambas falham. Sem checkpoints persistentes entre falhas de um catálogo.

### 7.2 Incidentes reais e resolução até à última mensagem

| Ficheiro/caso | Falha observada | Resultado posterior |
|---|---|---|
| `Effect/NewEffect/Res/Object/123.ASE` | Wine crash, depois `geometry_missing_uv` | **Piloto PASS** após PATCH 266; 10 objetos sem UV |
| `char/monster/death_knight/b_death_knight.ASE` | `geometry_material_reference` em bone; posterior timeout de readback com `Game.exe` | **Piloto PASS** após 268/272, `converted=2` |
| `char/monster/death_night/death_knight.ASE` | `geometry_face_material` | Cook avançou para o modelo seguinte após 269–270 |
| `char/tmABCD/MfhB03bt.ASE` | `Server.exe` timeout aos 180 s | Hierarquia válida: 48 ossos, 1 raiz; **piloto PASS** após 273 |
| `Field/ani/test-ani.ASE` | `geometry_material_reference`, `std::bad_array_new_length`, `material_registry_empty` | Tabela explícita com 0 materiais; **piloto PASS** após 274–276 |
| `char/monster/MonSRazorArcher/MonSRazorArcher-model.ASE` | `Server.exe` timeout aos 180 s no cook completo | Hierarquia válida: 27 ossos, 1 raiz; **piloto isolado PASS** antes do 277 |

**Último resultado real do cook completo:** timeout no Razor Archer; o piloto isolado passou. **PATCH 277 foi entregue com testes locais, mas ainda não existe nesta conversa confirmação de instalação nem resultado real do catálogo completo após 277.** Não declarar `complete=True` nem gate 267 aprovado.

### 7.3 Comandos de referência (offline, não usar durante gameplay)

Na raiz SUMMIT, depois de o PATCH 277 estar efetivamente instalado:

```bash
python3 SourceCode/tools/cook_native_models_incremental_263.py \
  --client ./cliente \
  --server ./Servidor/Server.exe \
  --client-exe ./SourceCode/dist_mingw_client_i686/Game.exe \
  --out ./bugfixes/SUMMIT_NATIVE_MODELS_INCREMENTAL_263
```

**Apenas** se `MODEL_COOK_263=PASS ... complete=True`:

```bash
python3 SourceCode/tools/verify_native_model_stage_267.py \
  --client ./cliente \
  --server ./Servidor/Server.exe \
  --client-exe ./SourceCode/dist_mingw_client_i686/Game.exe \
  --out ./bugfixes/SUMMIT_NATIVE_MODELS_INCREMENTAL_263
```

Não copiar SMD/SMB de staging para `cliente/`, não substituir cache antiga para esconder conversões com falha, e não tornar estes comandos parte do `wine Game.exe` normal. Como o cooker é executado em outro fluxo, **o novo chat não deve modificá-lo sem coordenação**.

### 7.4 Cronologia resumida dos patches relevantes

- 236–242: cooked DDS, recuperação/filas/medição/cook focalizado; 242 publicou 40 DDS.
- 243–246: assets/efeitos cooked, hotpaths e auditoria de papéis de modelos.
- 247–258: som, efeitos, streaming/prewarm e guards.
- 259–263: cook nativo, verificador cliente, inicialização e incremental/identidade.
- 264–266: guards parser, materiais opcionais e UV ausentes.
- 267: verificador de catálogo em staging.
- 268–271: bones sem materiais, IDs e alinhamento de faces, malha sem materiais.
- 272: seleção `Game.exe`/preflight verificador offline.
- 273: guarda hierarquia de ossos no modo offline.
- 274–276: tabela explicitamente vazia, layout SMD e registo nulo legítimo; piloto `test-ani` passou.
- 277: repetição limitada por timeout Wine; **validação de catálogo no PC ainda pendente**.

## 8. Segurança de alterações e critérios para novos patches

1. Ler a versão atual dos **ficheiros envolvidos**, não reconstruí-los a partir de snapshots anteriores. Ver `git status --short` e `git diff --cached --name-only` **só leitura**, sem limpar staged.
2. Para bug visual: criar primeiro reprodução 0°/180° com o jogo normal. É permitido fazer patch A/B **no código**, com fallback e rollback, se isso for a opção mais simples; não adicionar novas flags de diagnóstico/logs.
3. Alterações de `SourceCode/Content/canonical` são proibidas neste fluxo. Em áreas partilhadas (ex.: `Shared/rendering/legacy_3d`), verificar que não se desfazem os PATCHES 259–277.
4. Cada patch: número seguinte livre, escopo mínimo, ZIP com README/VALIDACAO_CLI, instalador fail-closed SHA-256, backup, idempotência, symlink protection, staged Git preservado, testes automáticos reproduzíveis, comandos de compilação e smoke Wine. Se o conteúdo do utilizador divergir, pedir/obter a versão atual do ficheiro e rebasear; não forçar.
5. Nunca usar `git reset`, `stash`, `clean`, `add -A` nem copiar entire source antigo por cima do novo. Nunca usar alterações a fog/sky como placebo.
6. Distinguir explicitamente nos resultados: **(a) source auditado, (b) regressões locais PASS, (c) compilou no PC, (d) executou Wine, (e) bug observado como resolvido.**
7. Não adicionar logs permanentes nem profiling quando o utilizador quer só `wine Game.exe`. Warnings de Wine por si só não provam causa.
8. Para NPCs, se o problema for conteúdo/spawn canónico, produzir relatório para o chat responsável; não publicar alterações nesses assets a partir deste chat.

## 9. Plano sugerido em sessões curtas, sem sobreposição

**Sessão A — VIS-01 (prioritária):** reproduzir branco 0°/180°, auditar ordem dos passes e estados D3D9; fazer um único patch A/B reversível se a causa não for evidente; testar em jogo; corrigir callsite confirmado. **Não começar por fog, céu ou assets.**

**Sessão B — NPC-01:** confirmar se NPC de Ricarten continua interativo apesar de invisível; examinar cadeia fixed NPC → spawn/packet → modelo → render, packs runtime sem alterar canonical; entregar bugfix ou handoff de dados.

**Sessão C — PERF-01 / ASSET-01:** após o bug visual, regressão prática de seleção/warp/Ricarten/combate. Distinguir stutter restante de upload síncrono, I/O de modelos e awaits; usar dados históricos existentes e observação do jogo, não obrigar novas flags/logs.

**Sessão D — funcionalidades/bugs funcionais:** aceitar uma feature por vez (poções/NPC shop, UI hover, crafting, wallet/premium, armazém), atualizar matriz quando houver reprodução e teste end-to-end. Evitar mutações simultâneas do mesmo source que está a ser editado pelo fluxo de cook.

**Em paralelo, noutro fluxo:** concluir 277 + cook total + gate 267 + validação visual de SMD/SMB. A marca `complete=True` não significa automaticamente que modelos e efeitos estejam visualmente corretos.

## 10. Dados adicionais que o novo chat deve obter apenas se forem essenciais

- Para VIS-01: **mapa e local aproximado** onde ocorreu o branco, e se a rotação 0°/180° o reproduz imediatamente; não é necessário pedir de novo o que já foi informado (HUD desaparece, troca de céu não ajuda, mapa sem fog).
- Para NPC-01: nome e posição aproximada de **um** NPC desaparecido, e se a interação no local ainda funciona. Não assumir quais são os NPCs em falta.
- Para bugs históricos: confirmar se continuam reproduzíveis na versão atual antes de os classificar como regressões.
- Fonte atual relevante, quando diferir do snapshot integrado: ficheiros precisos ou ZIP atualizado, não o repositório inteiro se não for necessário.

## 11. Mensagem inicial pronta para colar noutro chat

> Estou a desenvolver o SUMMIT (Priston Tale, cliente/servidor MinGW i686 sob Wine). O documento `SUMMIT_RETOMADA_BUGFIXES_E_IMPLEMENTACOES_2026-10-11.md` contém o estado detalhado. **Começa por VIS-01:** a imagem inteira fica branca, incluindo HUD e chat; com a câmara virada para a frente/cabeça do personagem (~180°) há imagem, ao virar para as costas (~0°) fica branca. Mudar os céus não altera nada; ocorre num mapa sem fog. Quero investigação concreta da ordem de render, estados D3D9 e efeitos de ecrã, com patches mínimos e testes normais `wine Game.exe`, **sem novos logs/flags**. Depois trata os NPCs ausentes de Ricarten, distinguindo spawn de invisibilidade, e segue as restantes pendências por prioridade. **Não edites `SourceCode/Content/canonical`, não limpes Git staged e não mexas no cooker ASE/SMD/SMB (PATCHES 259–277), que é acompanhado noutro chat.** Pede apenas o source atual estritamente necessário e entrega ZIPs fail-closed com backups e testes verificáveis.

---

## 12. Registo de certezas e limitações

**Confirmado pelo utilizador:** ecrã integral branco, HUD desaparece, relação 0°/180° com câmara, sky alterado sem efeito, mapa sem fog; queixa de NPCs ausentes em Ricarten; vários pilotos nativos `MODEL_COOK_263=PASS` e último cook completo interrompido por timeout Razor Archer; compilação/link de diversas séries anteriores; staged Git existente. 

**Confirmado por leitura do source nesta preparação:** caminhos de render, `BeginRender()` com `Clear` a preto, funções de filtros definidas mas sem chamada externa encontrada na árvore inspecionada, integração fixed NPC e adaptadores, utilitários offline. Estes achados são **pontos de investigação, não causas**.

**Não confirmado:** qual draw call cria o branco; se o bug está em 2D, 3D, device ou driver; se NPCs de Ricarten estão invisíveis ou ausentes no servidor; se todas as funcionalidades históricas ainda falham; se o PATCH 277 já foi instalado; `complete=True` e gate 267; resultados visuais após publicação do catálogo. 

**Fim do handoff.**
