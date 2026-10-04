# SUMMIT — Manual de Expansão de Conteúdo, História Mundial e Balanceamento

Versão baseada na source fornecida — 03/10/2026.

Source + servidor + cliente + conteúdo canônico

Baseado nos pacotes fornecidos: source, Content, Configs, GameServer e árvore do cliente.

Análise consolidada em 03/10/2026


| EXISTE | Componente já presente na source e reutilizável. |
| --- | --- |


| ESTENDER | A source tem a base, mas a feature precisa de integração adicional. |
| --- | --- |


| CRIAR | Não existe abstração suficiente; é necessário código/dados novos. |
| --- | --- |

Objetivo: permitir que outra pessoa implemente as features sem depender de descobrir novamente onde cada parte do jogo vive. O documento diferencia fatos encontrados na source de propostas de arquitetura.


## 1. Resposta direta às perguntas iniciais


| Pergunta | Resposta técnica | Complexidade |
| --- | --- | --- |
| World quest / história global irreversível | É viável. Story V2 já possui SetWorldFlag e persistência global; falta um coordenador de objetivos globais e um runtime que aplique a fase do mundo a mapas, NPCs, spawns e gates. | Alta |
| Mundo mudar quando história avança | Mobs/NPCs podem ser filtrados/recarregados por fase. Geometria/cidade inteira deve usar variantes de field/mapa e redirecionamento de jogadores; a source atual não faz hot-swap de geometria via world flag. | Alta |
| Placement de NPC / altares Covenant | Placement já é data-driven em fixed_npcs/*.spc.json. O comportamento vem da definição do NPC. Covenant já existe; o ponto ideal é LegacyInteractionAdapter + EventCode/integração V2. | Média |
| Novo personagem/classe | A arquitetura V2 foi preparada para isso e existe ADDING_A_CLASS.md. Hoje as classes são conteúdo C++ compilado, não JSON. São necessários identidade de protocolo, stats, aparência, armaduras, motions, skills, ranks e assets/UI. | Alta |
| Novo mapa + warping gates | Suportado pelo catálogo canônico de mapas. O field 52 é um exemplo custom real. Há um bloqueio atual: loader espera exatamente 53 mapas; precisa ser alterado para adicionar field 53+. | Média/Alta |
| Capacetes/máscaras | Não há slot HEAD/HELMET/MASK. Para máscara visual, usar override cosmético de head é a solução de menor risco. Para capacete com stats, criar slot completo em client/server/save/render/UI. | Média ou Alta |
| Novo monstro | Definição lógica + assets + áudio + inclusão no stage spawn. O conteúdo canônico representa stats/drops; runtime consome definitions_server.scfg e field_runtime.sfld. | Média |
| Buff por set completo | Há precedente hardcoded no cliente para sets de boss. Para o sistema desejado, criar SetDefinition + cálculo autoritativo no servidor e snapshot para o cliente. | Média |
| Balancear dinheiro/items | Drop normal é peso relativo; ExtraFallItem é chance /10000; gold usa faixa do monstro × GoldServer. A planilha anexa já expande 98 monstros e 7003 entradas. | Média |
| Balancear EXP | A fórmula, curva 1–161, multipliers por nível e party foram rastreados. Há inconsistências que devem ser corrigidas antes de ajustar números. | Média |


### 1.1 Regra de leitura deste manual

Quando o texto diz “atual”, é comportamento encontrado nos arquivos fornecidos. Quando diz “proposto”, é a arquitetura recomendada para satisfazer seu objetivo; não é apresentada como algo que já exista. Caminhos de arquivos são relativos à raiz da source ou do Content.


## 2. Como o conteúdo chega ao jogo


| EXISTE | O projeto possui uma camada canônica de conteúdo e packs compilados consumidos pelo runtime. |
| --- | --- |

O ponto mais importante para criar conteúdo é separar “fonte editável” de “arquivo consumido pelo executável”. Nos materiais fornecidos, mapas, spawns, NPCs fixos e recursos textuais têm representações canônicas em JSON. O runtime, porém, carrega packs binários/compilados.


| Domínio | Fonte canônica observada | Pack/runtime |
| --- | --- | --- |
| Mapas | Content/canonical/maps/server.maps.json e client.maps.json | maps_server.smap / maps_client.smap |
| Spawns de field | Content/canonical/stage_spawns/spawns/GameServer/Field/*.spm.json | field_runtime.sfld |
| NPCs fixos | Content/canonical/fixed_npcs/npcs/GameServer/Field/*.spc.json | field_runtime.sfld |
| Start points | conteúdo de field runtime/canônico | field_runtime.sfld |
| Monstros/NPCs/items textuais | Content/canonical/text/resources/.../*.json | definitions_server.scfg / definitions_client.scfg e/ou packs de recursos |
| Config gameplay | Configs/*.ini + conteúdo compilado | gameplay_config.sgcf / configuração runtime, conforme subsistema |
| Localização | Content/canonical/text/localization | localization.sloc |


| RISCO | O compilador/script que transforma o conteúdo canônico nos packs não está presente nos materiais fornecidos. Portanto este manual não inventa um comando de build. A etapa correta é: editar a fonte canônica -> regenerar o pack com o pipeline do projeto -> distribuir pack atualizado. |
| --- | --- |


### 2.1 Packs que aparecem no cliente


```text
Content/packs/
  definitions_client.scfg
  field_runtime.sfld
  gameplay_config.sgcf
  gameplay_reference.sgrf
  localization.sloc
  maps_client.smap
  text_resources.sres
```

No servidor, a source resolve explicitamente packs como definitions_server.scfg, field_runtime.sfld e maps_server.smap por RuntimeContentPath. Isso significa que editar um JSON sem regenerar o pack não altera o runtime.


## 3. Sistema de World Story / quest global por servidor


| EXISTE | Story V2 já possui SetWorldFlag, persistência SQLite e publicação de StoryWorldFlagChangedEvent. |
| --- | --- |


| ESTENDER | StoryProgress é por jogador. Para “todos os jogadores avançam juntos”, é necessário estado/objetivos globais e contribuições server-wide. |
| --- | --- |


| CRIAR | WorldPhaseRuntime para aplicar a fase global a spawn/NPC/gates/map variants. |
| --- | --- |


### 3.1 O que já existe

- Shared/gameplay/story/v2/StoryTypes.h: StoryActionType inclui SetWorldFlag.
- Shared/gameplay/story/v2/StoryService.cpp: executa repository_.SetWorldFlag(key,value) e publica StoryWorldFlagChangedEvent.
- Database/migrations/033_progression_v2.sql: ServerDB__StoryWorldFlag(FlagKey, IntValue).
- ProgressionTriggerRuntime reage a eventos de progressão, mas não troca geometria/spawns/NPCs do field por si só.
Conclusão: a persistência de “fase do mundo” já tem uma fundação confiável. O que não existe é a camada que interpreta essa flag como estado físico do mundo.


### 3.2 Modelo recomendado

Use uma fase inteira monotônica por história global, por exemplo world.story.main.phase. A fase nunca diminui em produção. Jogadores contribuem para objetivos globais; quando todos os objetivos da fase terminam, o servidor avança a fase numa transação e publica um evento de mudança.


| Componente proposto | Responsabilidade |
| --- | --- |
| WorldStoryCoordinator | Receber contribuições, consultar estado global, decidir se a fase terminou e fazer avanço atômico. |
| WorldStoryRepository | Persistir fase, objetivos e ledger/idempotência. |
| WorldPhaseRuntime | Assinar WorldPhaseChanged/StoryWorldFlagChangedEvent e aplicar regras físicas do mundo. |
| WorldPhaseSnapshot packet | Enviar fase atual no login/map change para que o cliente apresente UI/efeitos corretos. |
| Phase-aware spawn/NPC filter | Habilitar/desabilitar atores e placements por min/max phase. |
| MapVariantRouter | Redirecionar players/gates para uma variante visual quando a geometria mudou. |


### 3.3 Persistência proposta

StoryWorldFlag é suficiente para guardar a fase, mas não para contabilizar objetivos globais com segurança. Abaixo está uma migration proposta; os nomes são novos e precisam ser adicionados ao seu sistema de migrations.


```text
CREATE TABLE IF NOT EXISTS ServerDB__WorldStoryState (
    StoryKey TEXT PRIMARY KEY,
    Phase INTEGER NOT NULL,
    Version INTEGER NOT NULL DEFAULT 0,
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime'))
);

CREATE TABLE IF NOT EXISTS ServerDB__WorldStoryObjective (
    StoryKey TEXT NOT NULL,
    Phase INTEGER NOT NULL,
    ObjectiveKey TEXT NOT NULL,
    Progress INTEGER NOT NULL DEFAULT 0,
    Target INTEGER NOT NULL,
    Completed INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (StoryKey, Phase, ObjectiveKey)
);

CREATE TABLE IF NOT EXISTS ServerDB__WorldStoryContribution (
    StoryKey TEXT NOT NULL,
    Phase INTEGER NOT NULL,
    ObjectiveKey TEXT NOT NULL,
    ReceiptKey TEXT NOT NULL,
    PlayerKey TEXT NOT NULL,
    Amount INTEGER NOT NULL,
    CreatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY (StoryKey, Phase, ObjectiveKey, ReceiptKey)
);
```

ReceiptKey torna eventos idempotentes: uma morte/entrega não pode ser contabilizada duas vezes após retry/reconnect. O avanço deve executar UPDATE ... WHERE Phase = expectedPhase e aceitar apenas nextPhase > currentPhase.


### 3.4 Como mudar o mundo


| Tipo de mudança | Implementação recomendada | Por quê |
| --- | --- | --- |
| Monstros aparecem/desaparecem | Filtro de spawn por fase + reavaliação/reload controlado do stage spawn | Não exige trocar geometria. |
| NPC aparece/desaparece | Filtro de FixedNpcPlacement por fase ou overlay runtime | Placement atual já é orientado a dados. |
| Warp abre/fecha | Phase-aware gate policy; servidor é autoridade | Evita cliente atravessar gate fechado. |
| Cidade destruída/construída | Dois ou mais fields/map variants; ao avançar, redirecionar jogadores e gates | LoadStage lê stage/ASE no carregamento; não existe hot-swap de geometria via flag. |
| Textura/sky/música | Variante de mapa ou snapshot de fase + mudança client-side quando seguro | Alguns elementos são puramente visuais, mas devem seguir o estado server. |
| Quest/NPC dialogue | Condições Story V2/flag global | Já está dentro do domínio de progressão. |


### 3.5 Exemplo de campanha em quatro fases


| Fase | Condição global | Mundo |
| --- | --- | --- |
| 0 — Normal | Inicial | Cidade intacta; mobs normais; altar disponível. |
| 1 — Cerco | 100.000 pontos de contribuição | Adicionar mobs invasores; fechar um gate; NPCs fogem. |
| 2 — Queda | Boss global derrotado | Roteamento para city_destroyed; remover merchants; novos monstros. |
| 3 — Reconstrução | Materiais globais entregues | Roteamento para city_rebuilt; merchants voltam; desbloquear dungeon. |


### 3.6 Fluxo no servidor


```text
MonsterKilled / ItemDelivered / QuestCompleted
    -> WorldStoryCoordinator::Contribute(..., receiptKey)
       -> transaction: insert receipt + increment objective
       -> if objective(s) complete:
            AdvancePhase(expected, next)   // monotonic CAS
            SetWorldFlag("world.story.main.phase", next)
            publish WorldPhaseChanged(next)
               -> spawn/NPC/gate runtime re-evaluate
               -> relocate players if old map variant is retired
               -> broadcast WorldPhaseSnapshot
```


### 3.7 Regras de segurança

- Nunca aceite a fase enviada pelo cliente. O cliente apenas recebe snapshot.
- AdvancePhase precisa ser atômico para dois kills simultâneos não avançarem duas vezes.
- No boot, leia a fase persistida antes de abrir fields aos jogadores.
- Depois de restart, reconstrua WorldPhaseRuntime a partir do banco; não dependa de estado em memória.
- Mantenha ferramenta GM para inspecionar fase/objetivos. Se houver comando de rollback, deixe-o exclusivo de ambiente de desenvolvimento; a regra de produção continua irreversível.

## 4. NPC placement e altares de Covenant


| EXISTE | FixedNpcPlacement já contém npc_id, posição, rotação, slot e state. O NPC reutiliza uma definição separada. |
| --- | --- |


| EXISTE | Todo clique em NPC passa pelo InteractionService V2 antes da cascata legada. |
| --- | --- |


| ESTENDER | Criar uma rota de interação para altar Covenant que abre a UI e/ou executa join/activate via CovenantService. |
| --- | --- |


### 4.1 Placement atual

Exemplo real: Content/canonical/fixed_npcs/npcs/GameServer/Field/fore-3.ase.spc.json contém placements como:


```text
{
  "slot": 0,
  "npc_id": "npc.tmcave_keeper",
  "x": -4224666,
  "y": 76472,
  "z": -1772786,
  "ax": 0,
  "ay": 5658,
  "az": 0,
  "state": 0
}
```

A definição npc.tmcave_keeper está em Content/canonical/text/resources/Servidor/GameServer/NPC/tmcave-keeper.npc.json. Ela contém model_file, level, name/linked_file etc. Outro NPC de loja mostra que serviços de venda também pertencem à definição, não ao placement.


### 4.2 Implementar um altar Covenant

A solução compatível de menor impacto é reservar um EventCode para o altar. EventCode é lido pelo parser e LegacyNpcAdapter o expõe ao InteractionService. Nos dados fornecidos foram encontrados códigos 100, 121 e 1001; escolha um valor reservado pelo projeto após auditoria de conflitos. Abaixo uso 12001 apenas como exemplo proposto.

1. Criar a definição canônica do NPC do altar, usando um model_file existente ou novo.
1. Adicionar event_code = 12001 (exemplo) à definição; o parser já possui suporte a EventCode.
1. No SrcServer/src/systems/gameplay/world/v2/legacy/LegacyInteractionAdapter.cpp, tratar eventCode == CovenantAltarEventCode.
1. Validar no servidor se o jogador pode usar o altar (distância, estado, map/phase, covenant disponível).
1. Enviar um packet pequeno “OpenCovenantAltar” ou reutilizar uma mensagem V2 que carregue o snapshot de Covenant e intenção de abrir a aba.
1. No cliente, expor uma função pública em LegacyUIV2Bridge para abrir CharacterTab::Covenants. ToggleCharacterTab já sabe operar a aba, mas atualmente é interna ao .cpp.
1. Adicionar placements do mesmo npc_id nos .spc.json dos mapas desejados; não duplique a definição do NPC.
1. Regenerar definitions/server + field_runtime e testar como jogador normal.

### 4.3 Exemplo de placement do altar


```text
{
  "slot": 4,
  "npc_id": "npc.covenant_altar",
  "x": 123456,
  "y": 7890,
  "z": -222333,
  "ax": 0,
  "ay": 2048,
  "az": 0,
  "state": 0
}
```

As coordenadas acima são fictícias. Para cada mapa, use coordenadas verificadas no mundo/GM. O slot deve ser livre naquele .spc. A definição npc.covenant_altar é compartilhada; só o placement muda.


### 4.4 Melhor evolução futura

EventCode funciona, mas é um número opaco. A evolução mais limpa é estender a definição/NpcSnapshot com interactionType = CovenantAltar e covenantId opcional. Isso torna o conteúdo autoexplicativo e elimina números mágicos. Faça essa mudança somente depois que o primeiro altar estiver funcional, porque ela toca schema/parser/pack.


## 5. Criar uma nova classe/personagem jogável


| EXISTE | A source contém PlayableClassRegistry e o guia Shared/gameplay/classes/v2/docs/ADDING_A_CLASS.md. |
| --- | --- |


| RISCO | Hoje classes são conteúdo C++ compilado via IClassContentSource; não há loader JSON/YAML/TOML de classes neste branch. |
| --- | --- |


### 5.1 Identidade que precisa existir antes de ativar

- stable class id;
- legacyJobCode se protocolo/save atual ainda o usa;
- item specialization jobBitCode;
- animation.legacyMotionJobMask;
- legacy skill group route se reutilizar layout de skill antigo;
- rank specialization bits.
Não derive um bitmask do outro. O próprio guia da source alerta que item specialization bit e motion applicability mask são contratos diferentes.


### 5.2 Onde registrar a classe

O runtime padrão é composto por LegacyEightClassesContentSource + ReservedClassesContentSource via CompositeClassContentSource em content/DefaultClassContent.cpp. Crie um novo IClassContentSource (preferível) e adicione-o à composição. Não coloque a classe nova no catálogo das oito clássicas apenas por conveniência.


```text
PlayableClassDefinition d;
d.id = ...;
d.legacyJobCode = ...;
d.jobBitCode = ...;
d.brood = ...;
d.key = "...";
d.displayKey = "...";
d.gender = CharacterGender::...;
d.enabled = true;

d.formula = ...;
d.creation.selectable = true;
d.creation.selectionOrder = ...;
d.creation.baseAttributes = ...;
d.creation.bodyModels = ...;
d.creation.faces = ...;
d.creation.presentation = ...;
d.appearance = ...;
d.armor = ...;
d.equipment = ...;
d.combat = ...;
d.animation = ...;
d.skills = ...;
d.ranks = ...;
```


### 5.3 Assets: quais pastas/arquivos você precisa


| Parte | Onde a source/cliente espera | O que criar/validar |
| --- | --- | --- |
| Body/model principal | char/tmABCD/... e paths configurados em bodyModels/bodyModelRoots | Modelos .ini/.inx + .smd/.smb e texturas associadas, de acordo com pipeline de assets. |
| Cabeças/faces/hair | headModels[face][hair] / defaultHeadModels; legado usa char/tmABCD/*.inf | Arquivos de head/skin compatíveis e previews da seleção. |
| Armaduras | armor.modelRoot + normalModels; variantes comic/newSet opcionais | Um modelo por tier/índice necessário. Não aumente LEGACY_ARMOR_MODEL_ROW_COUNT para classe data-driven. |
| Motions/animações | animation.legacyMotionJobMask e .ini/*APPLICABLE_JOB | Conjunto de animações/model motions compatível com armas e skills. |
| Ícone de kill | presentation; padrão legado game//kill//class%d.png | Imagem no formato/tamanho do restante do UI. |
| Party alive/dead | presentation; assets em game//Party//Class/... | Dois estados se o UI exige ambos. |
| SOD rank icon | image//Sinimage//help//sod_cr-%d.bmp | Ícone correspondente ao novo job/rank mapping. |
| Ranking | ranking/Class contém ícones das 8 classes atuais | Adicionar ícone e apontar presentation para ele. |
| Skills/effects | skills.treeKey + subsistema de skills + scripts/particles | Novas skills podem exigir handlers; efeitos ficam no pipeline de partículas/efeitos. |
| Áudio | subsistema de sound/effects conforme a habilidade/personagem | Criar WAV/BGM/efeitos somente quando usados; não existe uma única “pasta da classe” universal para todo áudio. |


### 5.4 Reserva Assassin existente

ReservedClassCatalog contém Assassin com id 9/legacyJobCode 9 e enabled=false. Isso prova que o registry aceita identidade reservada, mas não prova que os bits, stats, skill tree, models ou persistence estejam completos. Não habilite a entrada como atalho sem preencher os contratos descritos acima.


### 5.5 Checklist para uma classe nova

- Criar IClassContentSource e definição completa; rodar registry Validate().
- Definir seleção: brood, order, stats base, body, faces e apresentação.
- Definir aparência data-driven; evitar novo switch(JOB_CODE).
- Definir armadura: legacyModelIndex somente se reutilizar uma linha histórica; caso contrário -1 + modelRoot/models.
- Definir política de equipamentos e especializações.
- Definir motion mask e testar *APPLICABLE_JOB nos model .ini.
- Definir treeKey; integrar handlers de skills novas e T5 somente se intencional.
- Definir ranks e bits estáveis.
- Adicionar ícones de kill/party/SOD/ranking e assets de seleção.
- Compilar client/server MinGW i686.
- Testar create -> select -> login -> save -> logout -> relog.
- Testar trocar armaduras/armas, morte/ressurreição, party, ranking, quests/job change e qualquer evento legado suportado.

## 6. Criar um mapa e ligá-lo ao jogo


| EXISTE | Mapas são materializados a partir de catálogos canônicos server/client; incluem stage, start points, gates, warp gates, ambiente e routing. |
| --- | --- |


| RISCO | Embora FIELD_MAX seja 256, o loader atual espera exatamente 53 static fields e FinalizeLegacyFieldCatalog(53). Adicionar field 53 exige alterar essa expectativa. |
| --- | --- |


### 6.1 Exemplo real: map.dc1 / field 52


| Campo | Servidor | Cliente |
| --- | --- | --- |
| id | map.dc1 | map.dc1 |
| legacy_field_code | 52 | 52 |
| stage | boss//dark_boss.ASE | boss//dark_boss.ASE |
| map_image_id | dc1 | dc1 |
| routing | PLAY_SERVER_CODE_EXTEND | PLAY_SERVER_CODE_EXTEND |
| limit_level | 100 | 1  ← divergência atual |
| start point | 198279, 240452 | 198279, 240452 |
| warp de saída | map.village_2 / field 3 | map.village_2 / field 3 |

Essa divergência de limit_level é um ótimo exemplo do motivo para validar paridade server/client. GM pode conseguir entrar por comandos mesmo quando jogadores normais enfrentam regras diferentes.


### 6.2 Passos para field 53

1. Escolher id estável, por exemplo map.ruins_reborn, e legacy_field_code = 53.
1. Adicionar a mesma entrada estrutural a Content/canonical/maps/server.maps.json e client.maps.json. Campos autoritativos como limit_level precisam ser consistentes.
1. Alterar Shared/world/field/field_legacy_server.inl e field_legacy_client.inl: o carregamento chama LoadCompiledMapsIntoLegacyFields com expectativa de 53 e FinalizeLegacyFieldCatalog(53). Para um novo static field, a contagem passa para 54. Revise também dynamicFieldCode para não colidir com o novo código.
1. Adicionar stage asset. O catálogo usa path legado como boss//dark_boss.ASE; a árvore de assets mostra o formato convertido/cooked (por exemplo Boss/dark_boss.smd). Use o pipeline de conversão de assets do projeto.
1. Criar start point para o mapa. STG_AREA::LoadStage deriva .spp/.spc/.spm do stage e o field_runtime pack fornece esses dados.
1. Criar o arquivo canônico de spawn *.spm.json, mesmo que inicialmente não tenha mobs.
1. Criar fixed NPC *.spc.json, mesmo vazio, se o pipeline exigir seção para o logical_path.
1. Adicionar minimapa field/map/<map_image_id>.tga e título field/title/<map_image_id>t.tga no cliente.
1. Adicionar um warp_gates de entrada em um mapa existente e um warp de saída no mapa novo. Servidor valida acesso; cliente recebe a mesma topologia.
1. Regenerar maps_server.smap, maps_client.smap e field_runtime.sfld; distribuir assets client/server necessários.
1. Testar primeiro com GM, depois com uma conta normal exatamente no level limit, abaixo e acima dele.

### 6.3 Exemplo de mapa novo


```text
{
  "schema": "summit.map/v1",
  "id": "map.ruins_reborn",
  "legacy_field_code": 53,
  "stage": "ruins_reborn//ruins_reborn.ASE",
  "map_image_id": "ruins_reborn",
  "environment": {
    "state": {"id":"state.dungeon","legacy_value":1280,"legacy_symbol":"FIELD_STATE_DUNGEON"},
    "sky": [{"legacy_value":0},{"legacy_value":0},{"legacy_value":0}],
    "music": {"id":"music.dungeon","legacy_value":5,"legacy_symbol":"BGM_CODE_DUNGEON"},
    "event": {"legacy_value":0}
  },
  "server": {"routing":{"id":"server.extend","legacy_value":3,"legacy_symbol":"PLAY_SERVER_CODE_EXTEND"}},
  "limit_level": 100,
  "field_sight": 0,
  "center": {"x":10000,"z":10000},
  "start_points": [{"x":10000,"z":10000}],
  "stage_objects": [],
  "gates": [],
  "warp_gates": [],
  "ambient": []
}
```

O JSON acima é um molde, não um arquivo pronto: coordenadas, estado, música, routing e paths precisam corresponder ao mapa real.


### 6.4 Como um warp gate funciona

O catálogo armazena posição/volume do gate e uma lista out_gates. Cada saída aponta para target id + legacy_target_code e coordenadas de destino. O adapter valida que o target exista no catálogo estático ou corresponda ao dynamicFieldCode. Portanto o novo mapa deve existir no catálogo antes de ser alvo de um warp.


```text
"warp_gates": [{
  "x": 198289, "z": 240238, "y": 1543,
  "size": 32, "height": 32,
  "limit_level": 0,
  "special_effect": 0,
  "out_gates": [{
    "target": "map.village_2",
    "legacy_target_code": 3,
    "x": 2599, "z": -18235, "y": 236
  }]
}]
```


## 7. Sistema de capacetes e máscaras


| EXISTE | Client e server têm slots até COSTUME e EARRINGSL/EARRINGSR; não há HEAD/HELMET/MASK. |
| --- | --- |


| EXISTE | Há aparência de cabeça/máscara no conteúdo legado: Tmh-B24.inf é comentado como “mascara”, e assets 815mask_M/W existem no cliente. |
| --- | --- |


### 7.1 Opção A — máscara cosmética (recomendada quando não precisa de stats)

Trate a máscara como override da aparência de cabeça, não como novo slot de combate. Isso reaproveita o sistema de head model e reduz muito o risco de corromper inventário/save/protocolo.

- Criar CosmeticHeadDefinition (id, item/unlock opcional, model masculino/feminino ou por classe, regras).
- Persistir no servidor o cosmeticHeadId equipado pelo personagem.
- Validar propriedade/desbloqueio no servidor.
- Enviar cosmeticHeadId no snapshot de aparência.
- No cliente, ao resolver head model, aplicar override se presente; caso contrário usar headModels normal.
- Adicionar opção de equipar/remover na UI cosmética ou usar item para desbloquear.
- Não alterar stats; combate continua independente.
Essa rota também combina com a existência de máscaras como head variants no conteúdo atual.


### 7.2 Opção B — capacete real com stats

Se o capacete precisa ser um item equipado que concede defesa/atributos e aparece no inventário, então é necessário criar um slot completo. Use os brincos como precedente arquitetural, não como slot a ser reutilizado.


| Camada | Alteração necessária |
| --- | --- |
| Constantes client/server | Adicionar INVENTORY_POS_HEAD em bit livre após auditoria; adicionar ITEM_CLASS_HEAD/HELMET. |
| Item definitions/parser | Novo tipo/família e classe de inventário; editor/content precisa gerar o item corretamente. |
| UI inventário | Nova área de slot, drag/drop, hit test, tooltips e posição de sInven/equipped array. |
| Packet/equip | Enviar/receber posição; garantir que estruturas/capacidades aceitam mais uma posição. |
| Servidor | Estender CheckInvenItemPosition / CheckInvenItemPosition2 e validação autoritativa. |
| Persistência | Salvar/restaurar ItemPosition/slot sem quebrar personagens antigos. |
| Stats | Incluir capacete no recálculo server-side; cliente apenas espelha para display. |
| Render | Definir como modelo de capacete se ancora/substitui head/hair e replicar para outros players. |
| Compatibilidade | Versionar protocolo/save se necessário; testar cliente antigo deve ser bloqueado ou compatibilizado. |


### 7.3 Decisão prática

Se o objetivo é “usar máscaras/capacetes visuais”, implemente Opção A. Se o objetivo é criar uma nova categoria de loot com defesa/sets, implemente Opção B. Não use COSTUME para stats permanentes de combate apenas para evitar trabalho: isso mistura semânticas e complica validação server-side.


## 8. Criar novos monstros


| EXISTE | A definição do monstro inclui modelo, stats, EXP, áudio e loot; o spawn do mapa é separado. |
| --- | --- |


### 8.1 Anatomia de um monstro real

O canonical de 90_Nevasca.inf demonstra os campos efetivamente consumidos pelo parser:


| Campo | Exemplo Nevasca | Função |
| --- | --- | --- |
| model_file | char\monster\blizzardgiant\blizzardgiant.ini | Asset/model descriptor. |
| name | Nevasca | Nome usado também para resolução legada de spawn. |
| level | 89 | Nível do mob. |
| life | 9000 | HP. |
| attack_damage | 140–200 | Dano base. |
| attack_rating | 1510 | Acerto. |
| defence | 900 | Defesa. |
| block | 28 | Block. |
| experience | 537214 | EXP base antes das fórmulas server. |
| effect_sound | BLIZZARDGIANT | Família de áudio. |
| fall_item_max | 1 | Quantidade base de rolls normais por morte. |
| fall_item | lista ponderada | Gold/items/no-item conforme pesos. |


### 8.2 Assets do cliente

A árvore fornecida mostra o padrão real de monstros: pastas em char/monster/<nome> com .inx, .smb, .smd e texturas; também existe cooked/char/monster com texturas .dds. Áudio de monstros está organizado em famílias com arquivos attack, damage, dead, neutral e skill. O código de effect_sound precisa resolver para uma família conhecida ou você precisa integrar uma nova.

- Model descriptor (.ini/.inx conforme o monstro e pipeline legado).
- Mesh/model (.smd/.smb) e texturas (.bmp/.tga; cooked .dds quando aplicável).
- Animações/motions referenciadas pelo .inx/.ini.
- Sons: attack 1/2, damage, dead, neutral, skill quando a criatura usa esses estados.
- Efeitos/particles de skill se o AI/skill os dispara.

### 8.3 Processo completo para um novo monstro

1. Duplicar um monstro de comportamento parecido como referência, mas gerar um id canônico novo e nome único.
1. Definir model_file e colocar todos os assets no cliente (e no servidor somente se seu loader/server realmente requer os modelos para stage/colisão/metadata).
1. Preencher level, HP, ataque, attack rating, defesa, absorb/block, velocidade, range, race/size e skill fields conforme o comportamento desejado.
1. Definir experience pensando na curva de nível e no tempo para matar; não copie EXP de um mob com TTK diferente.
1. Definir fall_item_max, FallItem e ExtraFallItem conscientemente (ver capítulo 10).
1. Definir effect_sound e validar que a família de áudio existe no cliente.
1. Regenerar definitions_server.scfg/definitions_client.scfg ou packs de definição correspondentes.
1. Adicionar monster_id ao *.spm.json do mapa com weight desejado e, se boss, master/slave/open_hours.
1. Regenerar field_runtime.sfld.
1. Testar spawn, AI, colisão, animações, dano, morte, EXP, todos os drops e som com logging habilitado.

### 8.4 Exemplo de spawn


```text
{
  "monster_id": "monster.meu_novo_monstro",
  "legacy_name": "Meu Novo Monstro",
  "weight": 12,
  "resolution": "exact"
}
```

weight aqui é peso de seleção do ator no spawn do field, não a chance de item. limit_max, delay e open_limit controlam a densidade/ritmo do stage spawn.


## 9. Bônus por set completo de armadura


| EXISTE | Há dois sets de boss hardcoded no cliente. Eles contam anel esquerdo + anel direito + amuleto; >=2 aplica defesa e >=3 aplica vida. |
| --- | --- |


| RISCO | O bônus atual é calculado no cliente; o servidor envia apenas SetBoss*Bonus* do Sistemas.ini. Não use esse padrão como autoridade para um sistema novo. |
| --- | --- |


| CRIAR | EquipmentSetService server-side + conteúdo de sets + snapshot de bônus para UI. |
| --- | --- |


### 9.1 Requisito desejado

Seu requisito é simples: peças isoladas dão 0; somente quando todas as peças do conjunto estão equipadas o bônus X fica ativo. O servidor deve recalcular isso após qualquer alteração de equipamento e usar o resultado nos cálculos reais de combate.


### 9.2 Schema proposto de sets

Esse arquivo não existe hoje; é uma proposta de conteúdo para eliminar ifs hardcoded.


```text
{
  "id": "set.iron_vanguard",
  "required_items": ["DA201", "DB201", "DG201", "DS201"],
  "require_all": true,
  "bonuses": [
    {"stat_key":"defense", "operation":"flat", "value":150},
    {"stat_key":"max_life", "operation":"additive_percent", "value":10}
  ]
}
```


### 9.3 Implementação

1. Carregar SetDefinition no servidor.
1. Após equip/unequip validado, construir snapshot dos item codes realmente equipados a partir do inventário autoritativo, não do packet do cliente.
1. Para cada set, testar required_items. Se faltar qualquer peça e require_all=true, remover todos os modifiers daquele sourceKey.
1. Se completo, aplicar modifiers com sourceKey set:<id>. A source já possui GameplayModifier/ActiveModifierSet com Flat/AdditivePercent/MultiplicativePercent, usado por sistemas V2 como Covenant; reaproveitar esse conceito reduz duplicação.
1. Incluir os modifiers na recomposição de stats usada pelo combate do servidor.
1. Enviar ao cliente EquipmentSetSnapshot {setId, active, equippedCount, requiredCount, bonuses} para tooltips/status.
1. Cliente não deve decidir se o bônus existe; apenas apresenta o snapshot.

### 9.4 Testes obrigatórios

- 0 peças -> 0 bônus
- 1..N-1 peças -> 0 bônus
- N peças -> bônus exatamente uma vez
- remover uma peça -> bônus removido imediatamente
- relog com set equipado -> bônus reconstruído sem duplicar
- trocar duas peças rapidamente -> nenhum stacking residual
- packet adulterado -> servidor usa inventário persistido/validado

## 10. Balanceamento: drops, dinheiro e EXP

Foi gerada junto desta documentação a planilha Summit_Balanceamento_Atual.xlsx, contendo 98 monstros, 7003 entradas de drop expandidas, curva de EXP até o nível 161 e party rates atuais. Use a planilha como base quantitativa; este capítulo explica a matemática encontrada na source.


### 10.1 Drop normal — FallItem

CharacterMonsterReader soma o primeiro número de cada FallItem em FallItemPerMax. Na morte, srGetMonsterItem sorteia rnd % FallItemPerMax e percorre pesos cumulativos. Portanto o primeiro número é um peso relativo, não uma porcentagem absoluta.


```text
Chance(item por roll) = peso_efetivo_do_item / FallItemPerMax

Se FallItemMax = R rolls:
Chance de obter pelo menos 1 cópia = 1 - (1 - p)^R
```

Quando uma linha lista vários item codes, o parser faz percentage /= count usando divisão inteira e atribui esse peso a cada item, mas FallItemPerMax já recebeu o peso original. Se não for divisível, sobra um pequeno intervalo sem entrada — uma chance implícita de nenhum item.


### 10.2 NoItem e Money

- NoItem cria uma entrada de peso que retorna FALSE, isto é, roll sem drop.
- Money cria item gold com faixa min/max. Ao cair, valor = random(min,max) × GoldServer.
- Configs/Sistemas.ini atual: [Rate] Gold = 2. Portanto a faixa configurada é dobrada no drop base.

### 10.3 ExtraFallItem

ExtraFallItem não entra no denominador normal. Para cada entrada extra, a morte faz rand() % 10000 < Percentage. É uma rolagem independente. Exemplo: Percentage=50 significa 50/10000 = 0,5% por morte.


### 10.4 EXP base e diferença de nível

srGetTotalExp recebe EXP base do monstro e a diferença absoluta de level entre jogador e monstro. O multiplicador atual é:


| |Δ level| | Fator |
| --- | --- |
| 0–10 | 100% |
| 11–20 | 90% |
| 21–30 | 60% |
| >30 | 25% |

Depois, o resultado é multiplicado por Configs/ExpManager.ini -> [ExpConfig] Nv[level do jogador]. Na configuração fornecida:


| Level jogador | Nv[level] atual |
| --- | --- |
| 1–39 | 10 |
| 40–59 | 7 |
| 60–79 | 5 |
| 80–89 | 4 |
| 90–99 | 3 |
| 100–108 | 2 |
| 109–160 | 1 |


```text
EXP_pre_buffs = MonsterExperience × LevelDifferenceFactor × Nv[playerLevel]
```


| RISCO | srGetTotalExp recebe INT64 exp, mas usa int result internamente. Para endgame/EXP alta, troque o intermediário para INT64 antes de rebalancear para evitar overflow/truncamento. |
| --- | --- |


### 10.5 Party EXP atual

LoadPartyExpTable sobrescreve somente as linhas de 2 a 6 membros a partir de ExpManager.ini. As linhas 7 e 8 ficam hardcoded. Com a configuração fornecida:


| Membros | % killer | % cada membro | Total distribuído se todos elegíveis | Origem |
| --- | --- | --- | --- | --- |
| 1 | 100% | — | 100% | hardcoded |
| 2 | 100% | 100% | 200% | ExpManager.ini |
| 3 | 90% | 90% | 270% | ExpManager.ini |
| 4 | 80% | 80% | 320% | ExpManager.ini |
| 5 | 70% | 70% | 350% | ExpManager.ini |
| 6 | 60% | 60% | 360% | ExpManager.ini |
| 7 | 22% | 21% | 148% | hardcoded |
| 8 | 20% | 20% | 160% | hardcoded |


| RISCO | Há uma queda brusca entre 6 e 7 membros. Se parties de 7/8 são válidas, adicione 7 Membros/8 Membros à configuração e altere o loader para carregar todas as linhas. Além disso, comentários antigos na tabela não correspondem à soma real. |
| --- | --- |


### 10.6 Como balancear EXP de forma controlável

Não comece alterando MonsterExperience sem meta. Defina uma curva de tempo de progressão e derive a EXP necessária por kill.


```text
1. Para cada level L:
   RequiredXP(L) = LevelTable[L+1] - LevelTable[L]

2. Defina meta de tempo T(L) em horas para subir de nível.

3. Meça kills/hora K em um mapa apropriado (inclui deslocamento, respawn e TTK).

4. XP efetiva alvo por kill:
   EffectiveXPTarget = RequiredXP / (T × K)

5. Volte pela fórmula para obter MonsterExperience base:
   BaseXP ≈ EffectiveXPTarget / (DiffFactor × Nv[L] × buffs esperados)

6. Teste solo e party separadamente.
```

A planilha “EXP por nível” já contém EXP acumulada, EXP até o próximo nível, Nv multiplier, mediana de EXP dos monstros próximos e estimativa de kills. Use-a para detectar saltos e zonas mortas.


### 10.7 Como balancear gold


```text
Gold esperado por kill (base) ≈
FallItemMax × ChanceGoldPorRoll × média(GoldMin, GoldMax) × GoldServer
```

Depois compare gold/hora com os principais sinks: potion, repair, warp, crafting, aging/mix, shops etc. A economia deve ser balanceada em fluxo por hora, não somente em gold por monstro.


### 10.8 Como balancear loot

- Defina categorias por finalidade: consumível frequente, material, equipamento comum, raro, chase item.
- Defina chance efetiva por morte, não apenas pesos. Converta p por roll para p por kill quando FallItemMax > 1.
- Mantenha NoItem explícito se quiser uma taxa global de “nada”.
- Para itens muito raros e independentes, ExtraFallItem é mais fácil de raciocinar porque usa /10000.
- Calcule itens por 1.000 kills e por hora. Isso revela inflação de materiais.
- Bosses devem ser avaliados com frequência de spawn/open_hours além da chance de drop.
- Evite colocar muitos itens numa única linha ponderada se você precisa de probabilidades precisas: a divisão inteira por quantidade introduz arredondamento.

### 10.9 Correções recomendadas antes do tuning final


| Problema | Correção |
| --- | --- |
| INT64 -> int em srGetTotalExp | Usar INT64 result e multiplicação segura. |
| Party 7/8 não configurável | Carregar todas as linhas suportadas do INI ou migrar para tabela V2. |
| Multipliers de EXP lidos do disco durante cálculo | Carregar configuração uma vez/cachear, com reload controlado. |
| Evento EXP com branch/clamp antigo | Revisar semântica do parâmetro para evitar valor configurado parecer multiplicador diferente do real. |
| Solo/party possuem caminhos de buffs diferentes | Unificar composição de modificadores de EXP em um serviço único. |
| Set bonus client-authoritative | Mover cálculo real para servidor. |


## 11. Implementação técnica sugerida para WorldPhaseRuntime

Esta seção é proposta, construída para encaixar nas abstrações V2 que já existem.


### 11.1 Dados de fase para spawns/NPCs

Em vez de editar o pack em runtime, estenda os schemas canônicos para permitir condições de fase. Exemplo proposto:


```text
// actor em *.spm.json
{
  "monster_id": "monster.invader",
  "legacy_name": "Invader",
  "weight": 20,
  "world_phase": {"story_key":"main", "min":1, "max":2}
}

// NPC em *.spc.json
{
  "slot": 7,
  "npc_id": "npc.refugee",
  "x": 100, "y": 20, "z": 300,
  "ax": 0, "ay": 0, "az": 0, "state": 0,
  "world_phase": {"story_key":"main", "min":1, "max":2}
}
```

Isso exige versão nova/compatível do schema + compiler + FieldRuntimePack. Se você quer uma primeira entrega mais rápida, mantenha o pack sem conditions e crie overlays de fase em uma tabela/arquivo V2 carregado pelo WorldPhaseRuntime.


### 11.2 Geometria por variantes


```text
map.city_intact     -> field N
map.city_siege      -> field N+1
map.city_destroyed  -> field N+2
map.city_rebuilt    -> field N+3

WorldPhaseRouter:
 phase 0 => canonical destination map.city_intact
 phase 1 => map.city_siege
 phase 2 => map.city_destroyed
 phase 3 => map.city_rebuilt
```

Não precisa mostrar as quatro variantes simultaneamente no mundo. Os gates resolvem um “logical destination” para a variante ativa. Em transição, jogadores no field antigo recebem relocação segura para o start point da variante nova.


### 11.3 Ordem de transição

1. Concluir objetivo em transação.
1. Persistir Phase=N+1.
1. Atualizar StoryWorldFlag espelho, se mantiver ambos.
1. Publicar WorldPhaseChanged.
1. Desabilitar novas entradas no field antigo.
1. Atualizar spawn/NPC/gate policies.
1. Relocar players afetados para variante nova.
1. Broadcast snapshot/UI.
1. Persistir/auditar log da transição.

## 12. Checklist de criação e deploy


| Feature | Server | Client | Packs/dados |
| --- | --- | --- | --- |
| Novo NPC altar | Interaction adapter, validações, CovenantService | Abrir aba Covenants/feedback | NPC definition + fixed_npcs + definitions + field_runtime |
| Novo monstro | Definition runtime, AI/stats/drop, spawn | Modelo/animação/textura/som/effects | definitions_* + field_runtime |
| Novo mapa | maps_server + field runtime + gate authority | maps_client + stage + minimap/title/assets | maps_server.smap + maps_client.smap + field_runtime.sfld |
| Nova classe | Class content, persistence, combat/skills | selection, models, armor, UI, animations, icons | C++ content hoje + assets/packs aplicáveis |
| Máscara cosmética | persist/validate cosmeticHeadId | head override + UI | definition/snapshot + assets |
| Capacete real | novo slot + validate/save/stats | novo slot + UI/render | item definitions + protocol/save |
| Set completo | EquipmentSetService + modifiers | display snapshot/tooltips | novo set content proposto |
| World story | Coordinator/repo/phase runtime | snapshot/UI + visual state | DB migrations + story content + phase rules |


### 12.1 Sequência segura para qualquer conteúdo novo

1. Criar conteúdo com ID estável e sem reutilizar IDs existentes.
1. Validar dependências (asset, definition, spawn/placement, map target).
1. Regenerar packs necessários.
1. Subir servidor e verificar logs de carregamento/validation.
1. Testar como GM para localizar/inspecionar.
1. Testar como jogador normal para validar level, permission, routing e packets.
1. Testar relog/restart; qualquer feature persistente precisa sobreviver ao processo.
1. Testar cliente limpo com somente o pacote distribuível; isso encontra assets esquecidos.
1. Adicionar observabilidade: log do ID, map, phase, drop/EXP quando a feature é nova.
1. Só então habilitar em produção.

## 13. Índice de arquivos importantes


| Área | Arquivo |
| --- | --- |
| Story types/actions | Shared/gameplay/story/v2/StoryTypes.h |
| Story execution/world flag event | Shared/gameplay/story/v2/StoryService.cpp |
| Story repository | Shared/gameplay/story/v2/IStoryRepository.h + SQLiteStoryRepository |
| Story DB | Database/migrations/033_progression_v2.sql e migrations de content 035/037 |
| NPC interaction V2 | Shared/gameplay/world/v2/IInteractionService.h |
| NPC legacy adapter | SrcServer/src/systems/gameplay/world/v2/legacy/LegacyNpcAdapter.cpp |
| Interaction dispatch | SrcServer/src/systems/gameplay/world/v2/legacy/LegacyInteractionAdapter.cpp |
| NPC parser | Shared/formats/character_config/CharacterNpcReader.h |
| Map loader client/server | Shared/world/field/field_legacy_client.inl / field_legacy_server.inl |
| Map adapter | Shared/content/v1/map/legacy/LegacyMapAdapter.h |
| Field runtime pack | Shared/content/v1/field_runtime/... |
| Stage spawn adapter | Shared/content/v1/stage_spawn/legacy/LegacyStageSpawnAdapter.h |
| Monster parser | Shared/formats/character_config/CharacterMonsterReader.h |
| Drop runtime | Shared/server/legacy_runtime/OnSever_legacy_server.inl::srGetMonsterItem |
| EXP runtime | Shared/server/legacy_runtime/OnSever_legacy_server.inl::srGetTotalExp |
| Party EXP | SrcServer/src/core/server/legacy_runtime/PartySever.cpp |
| Level curve | Shared/LevelTable.h (incluído pelo gameplay legado) |
| Set bonus antigo | SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp |
| Item/slots client | Shared/inventory/legacy/sinItem_legacy_client.h |
| Item/slots server | Shared/inventory/legacy/sinItem_legacy_server.h |
| Legacy heads/mask | Shared/gameplay/legacy_support/PlayModelLegacyShared.h |
| Class guide | Shared/gameplay/classes/v2/docs/ADDING_A_CLASS.md |
| Class runtime/content | Shared/gameplay/classes/v2/.../DefaultClassContent.cpp, catalogs e ClassService |
| Covenant UI bridge | SrcGame/src/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp/.h |
| Maps canonical | Content/canonical/maps/server.maps.json / client.maps.json |
| Spawns canonical | Content/canonical/stage_spawns/spawns/GameServer/Field/*.spm.json |
| NPC placements canonical | Content/canonical/fixed_npcs/npcs/GameServer/Field/*.spc.json |
| Definitions canonical | Content/canonical/text/resources/Servidor/GameServer/{Monster,NPC}/... |
| EXP config | Configs/ExpManager.ini |
| Gold/set values | Configs/Sistemas.ini |


## 14. Plano de testes por feature


### 14.1 World Story

- dois jogadores completam o último objetivo simultaneamente -> uma única transição
- restart no meio da fase -> progresso preservado
- restart após transição -> mapa/spawns/NPCs da nova fase
- player offline durante avanço -> recebe fase correta no login
- player no mapa aposentado -> relocação segura
- attempt de packet client para fase antiga/nova -> ignorado

### 14.2 Mapa

- warp ida/volta
- spawn point após morte/relog
- minimap/title
- level limit server/client
- routing para server correto
- spawn/NPC packs resolvidos
- assets ausentes geram erro detectável

### 14.3 Monstro

- spawn density
- aggro/range
- damage/defense
- animation state completo
- sound family
- EXP solo/party
- 10000+ kills simulados para distribuição de loot
- boss open_hours se aplicável

### 14.4 Equipamento/set/máscara

- equip/unequip
- save/relog
- outro player vê aparência
- stats server coincidem com UI
- item incompatível rejeitado
- set não stacka após relog
- trade/drop enquanto equipado tratado corretamente

## 15. Achados que merecem correção antes de expansão grande


| Achado | Impacto |
| --- | --- |
| Map loader fixo em 53 | Bloqueia crescimento do catálogo mesmo com FIELD_MAX=256. |
| Server/client map.dc1 limit_level divergente | Comportamento inconsistente para jogador normal. |
| EXP INT64 reduzida a int | Risco de overflow em endgame/rates altas. |
| Party 7/8 hardcoded | Curva de party quebra de 6 para 7 membros. |
| Set boss client-side | Cliente pode virar autoridade de um bônus que deveria afetar regras server. |
| Content compiler ausente do pacote fornecido | Impossível documentar comando de geração de packs com evidência; pipeline deve ser recuperado/adicionado ao repo. |
| Mistura de legado e V2 | Ao criar features, preferir adapters/services V2 e manter legado apenas como compatibilidade. |


## 16. Ordem recomendada de implementação

Para minimizar retrabalho, implemente nesta ordem:

1. Corrigir infraestrutura de conteúdo: remover hardcode 53 e recuperar/documentar o content compiler.
1. Criar um NPC altar Covenant em um mapa de teste e depois replicar placements.
1. Criar um monstro novo completo e colocá-lo no mapa custom; isso valida definitions + assets + spawn + balance.
1. Formalizar planilha/metas de EXP/gold/drop e corrigir INT64/party rates antes de ajustar economia.
1. Criar EquipmentSetService server-side e migrar os dois boss sets antigos como primeiro teste.
1. Criar máscara cosmética; só criar slot HEAD se existir requisito real de stats.
1. Fechar pipeline de mapa 53 e usar como primeira variante de world story.
1. Implementar WorldStoryCoordinator + WorldPhaseRuntime; começar com troca de mobs/NPCs, depois variantes de geometria.
1. Adicionar uma classe nova por último, porque é a feature com maior superfície de protocolo/assets/skills/persistência.

## Apêndice A — Templates copiáveis


### A.1 Definição lógica de objetivo de world story (proposta)


```text
{
  "story_key": "main",
  "phase": 1,
  "objectives": [
    {"key":"kill_invaders", "type":"monster_kill", "target":100000},
    {"key":"deliver_wood", "type":"item_delivery", "item":"FOO123", "target":25000}
  ],
  "next_phase": 2
}
```


### A.2 Regras de fase (proposta)


```text
{
  "story_key":"main",
  "phase":2,
  "map_routes":{"map.city":"map.city_destroyed"},
  "enable_spawn_groups":["spawn.invaders_elite"],
  "disable_npcs":["npc.city_merchant"],
  "enable_npcs":["npc.refugee_leader"],
  "disabled_warps":["warp.city_north"]
}
```


### A.3 Set full-only (proposta)


```text
{
  "id":"set.example",
  "required_items":["ARM001","BOOT001","GLOVE001"],
  "require_all":true,
  "bonuses":[{"stat_key":"defense","operation":"flat","value":100}]
}
```


### A.4 Fórmulas rápidas


```text
normal_drop_p = effective_weight / FallItemPerMax
at_least_one_after_R_rolls = 1 - (1-normal_drop_p)^R
extra_drop_p = Percentage / 10000
base_gold_expected = R * gold_weight/FallItemPerMax * avg(min,max) * GoldServer
xp_pre_buffs = monster_exp * diff_factor * Nv[player_level]
kills_to_level ≈ required_xp / effective_xp_per_kill
```


## Apêndice B — O que ainda não pode ser documentado com exatidão

O único gap operacional relevante nos materiais fornecidos é o executável/script/comando do compilador que gera os packs canônicos. A source mostra claramente quais packs o runtime lê e os JSONs mostram a fonte editável, mas o gerador não foi encontrado. Assim que esse toolchain estiver disponível, acrescente ao capítulo 2 os comandos de build e uma validação CI que compare hashes/versões dos packs.
