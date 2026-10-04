# Milestone M01 — História coletiva/global por servidor

## Objetivo

Implementar uma campanha global em que ações de vários jogadores contribuem para objetivos compartilhados e, ao atingir um limiar, o servidor avança **uma única vez** para a próxima fase da história. O avanço é persistente por servidor, sobrevive a restart e não regride através de gameplay normal.

Este milestone é a base do requisito original: “os jogadores avançam na história em conjunto; quando uma parte avança o mundo muda; uma vez avançado não volta”.

## O que a source já fornece

A solução deve reutilizar o Story V2 existente, não criar um sistema de quest paralelo:

- `Shared/gameplay/story/v2/StoryTypes.h`: `StoryActionType::SetWorldFlag`;
- `Shared/gameplay/story/v2/IStoryRepository.h`: `GetWorldFlag` / `SetWorldFlag`;
- `SrcServer/src/systems/gameplay/story/v2/SQLiteStoryRepository.cpp`: persistência SQLite de `StoryWorldFlag`;
- `Shared/gameplay/story/v2/StoryService.cpp`: publica `StoryWorldFlagChangedEvent` após aplicar `SetWorldFlag`;
- `SrcServer/src/systems/gameplay/progression/v2/ProgressionTriggerRuntime.cpp`: já escuta `StoryWorldFlagChangedEvent` para reavaliar progressão dos jogadores online.

A lacuna atual é que `StoryWorldFlagChangedEvent` ainda não materializa mudanças físicas de mundo, e `StoryWorldFlag` armazena somente um valor, não um contador transacional de contribuição coletiva.

## Resultado esperado

Fluxo final:

```text
kill / quest / entrega / evento
          |
          v
WorldStoryContributionService
          |
          +--> receipt idempotente
          +--> contador global transacional
          |
          v
threshold alcançado?
          |
          v
Compare-and-set da phase global
          |
          v
StoryWorldFlagChangedEvent
          |
          v
WorldPhaseRuntime
          |
          +--> maps / variants
          +--> spawn profiles
          +--> NPC profiles
          +--> gates / routes
          +--> players online
```

## Modelo de domínio proposto

### Campaign

```text
id                   world.main_story
phase_flag           world.main.phase
monotonic            true
initial_phase        0
```

### Phase

Cada phase possui:

- `value`: inteiro monotônico;
- `key`: nome estável;
- objetivos globais que desbloqueiam a próxima phase;
- efeitos físicos delegados ao M02 `WorldPhaseRuntime`;
- ações Story opcionais após commit.

### Objective

Tipos iniciais recomendados:

- `kill_monster`;
- `turn_in_item`;
- `complete_quest`;
- `interact_npc`;
- `event_score`;
- `custom_counter`.

Cada objective precisa de:

- ID estável;
- target/subject;
- quantidade necessária;
- phase à qual pertence;
- política de contribuição por jogador quando aplicável.

## Persistência

`StoryWorldFlag` continua como **estado final da phase**. Não usar o mesmo registro como contador de objetivo.

Adicionar tabelas server-only, por exemplo:

```sql
WorldStoryState(
  CampaignId TEXT PRIMARY KEY,
  Phase INTEGER NOT NULL,
  Revision INTEGER NOT NULL,
  UpdatedAt INTEGER NOT NULL
)
```

```sql
WorldStoryObjectiveProgress(
  CampaignId TEXT NOT NULL,
  Phase INTEGER NOT NULL,
  ObjectiveId TEXT NOT NULL,
  CurrentValue INTEGER NOT NULL,
  Revision INTEGER NOT NULL,
  PRIMARY KEY(CampaignId, Phase, ObjectiveId)
)
```

```sql
WorldStoryContributionReceipt(
  ReceiptKey TEXT PRIMARY KEY,
  CampaignId TEXT NOT NULL,
  Phase INTEGER NOT NULL,
  ObjectiveId TEXT NOT NULL,
  PlayerKey TEXT,
  Amount INTEGER NOT NULL,
  CreatedAt INTEGER NOT NULL
)
```

`ReceiptKey` deve ser derivado do evento autoritativo que gerou a contribuição. Uma retransmissão ou retry não pode contar duas vezes.

## Regra de avanço irreversível

O `SetWorldFlag` atual faz UPSERT sem comparar o valor anterior. Para uma campanha monotônica isso é insuficiente sozinho.

Implementar uma operação específica no repository, por exemplo:

```cpp
AdvanceWorldFlagMonotonic(flagKey, expectedCurrent, nextValue)
```

Semântica:

```text
UPDATE ...
SET IntValue = next
WHERE FlagKey = key
  AND IntValue = expected
  AND next > IntValue
```

Se zero rows forem alteradas, recarregar o estado. Isso resolve duas threads/processos tentando completar o mesmo threshold.

Não permitir que gameplay chame uma operação genérica que escreva uma phase menor.

Rollback administrativo, caso exista, deve ficar em ferramenta de manutenção separada, com audit log explícito.

## Transação de conclusão de phase

A sequência deve ser:

1. validar contribution;
2. inserir receipt idempotente;
3. incrementar objective;
4. detectar se todos os objectives obrigatórios foram concluídos;
5. CAS `phase -> phase+1`;
6. commit DB;
7. publicar `StoryWorldFlagChangedEvent`;
8. reconciliar mundo através do M02.

O evento nunca deve anunciar uma phase que ainda não foi persistida.

## Reconcile de boot

Antes do servidor aceitar jogadores:

1. carregar catálogo `world_phases`;
2. ler phase persistida de cada world/campaign;
3. validar que a phase existe no conteúdo da build atual;
4. aplicar a phase no `WorldPhaseRuntime`;
5. somente depois abrir login/world entry.

Se a DB disser phase 4 e o conteúdo da build só possuir phases 0..3, o servidor deve falhar de forma explícita. Não fazer fallback silencioso para phase 0.

## Reconcile de login

Ao jogador entrar:

1. consultar phase global já materializada;
2. validar o field salvo do personagem;
3. se ele estiver numa variante aposentada, resolver redirect para a variante ativa;
4. aplicar safe start point;
5. enviar estado necessário ao client;
6. liberar movimento.

Isso garante que alguém que não estava online quando a história avançou vê o mundo atual imediatamente.

## API sugerida

```cpp
class IWorldStoryRepository {
public:
    virtual LoadCampaignResult LoadCampaign(...) = 0;
    virtual ContributionResult AddContribution(...) = 0;
    virtual AdvanceResult TryAdvancePhase(...) = 0;
};
```

```cpp
class WorldStoryContributionService {
public:
    ContributionResult Contribute(const ContributionRequest& request);
};
```

A service não deve saber detalhes de `STG_AREA`, NPCs ou mapas. Ela só conclui a phase e publica o evento. A materialização física pertence ao M02.

## Conteúdo canônico

Criar um catálogo separado do catálogo físico de phase, por exemplo:

```text
Content/canonical/world_story/campaigns.json
```

O arquivo deve conter somente regras da campanha e objectives. O `world_phases.json` continua responsável pelo efeito físico daquela phase.

Essa separação permite que uma phase seja avançada por quest hoje e por evento sazonal amanhã sem mudar o materializador do mundo.

## Telemetria obrigatória

Registrar pelo menos:

- contribution accepted/rejected;
- receipt duplicate;
- objective progress before/after;
- threshold reached;
- CAS applied/conflict;
- phase transition published;
- world reconcile success/failure;
- player relocation por causa de phase.

Não usar logs como fonte de verdade; são diagnóstico.

## Testes

### Unit

- contribuição única incrementa 1x;
- mesmo receipt repetido não incrementa;
- contribuição para phase antiga é rejeitada;
- contribuição após objective completo é idempotente;
- phase não regride;
- dois `TryAdvancePhase` concorrentes: exatamente um vence.

### Integration

- vários players completam um objective compartilhado;
- server reinicia no meio do objective e mantém progresso;
- server reinicia depois da phase e retorna na mesma phase;
- player offline entra depois e recebe phase atual;
- DB phase inexistente na build falha no boot com diagnóstico claro.

### Acceptance scenario

1. servidor inicia em phase 0;
2. 10 jogadores somam 1.000 kills;
3. contador chega exatamente a 1.000;
4. phase passa de 0 para 1 uma única vez;
5. `StoryWorldFlagChangedEvent(world.main.phase=1)` é publicado;
6. M02 materializa a nova cidade/mobs/NPCs;
7. restart;
8. servidor volta diretamente em phase 1;
9. nenhuma ação normal consegue voltar a 0.

## Critérios de aceite

- phase global é persistente e monotônica;
- contribuição é idempotente;
- concorrência não duplica transição;
- restart/relog preservam estado;
- Story V2 continua sendo a integração oficial;
- não há dependência direta de map/NPC internals no contribution service;
- M02 consegue consumir o evento de phase sem conhecer como ela foi desbloqueada.

## Dependências

- M02 — materialização física da phase;
- bug report de maps 53/StageArea antes de usar muitas variantes novas;
- SQLite migration para progresso global coletivo.

## Artefatos já fornecidos no bundle

- `patches/world_phases/WorldPhaseRuntime.*` — núcleo puro de reconciliação;
- `patches/world_phases/world_phase_runtime_smoke.cpp` — smoke test;
- `content/Content/canonical/world_phases/world_phases.json` — primeiro contrato de phase;
- `tools/content_port/schemas/world_phases.schema.json`.
