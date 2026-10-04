# World Story / progressão coletiva irreversível

## Base já existente

A source V2 já possui:

- `StoryActionType::SetWorldFlag`;
- persistência de world flags em SQLite;
- `StoryWorldFlagChangedEvent` após commit;
- event hub server-side;
- `ProgressionTriggerRuntime` já inscrito no evento para reavaliar jogadores online.

Portanto não é necessário inventar outro quest framework global.

## Componente adicionado no bundle

`patches/world_phases/WorldPhaseRuntime.*` implementa a regra de fase:

- catálogo de worlds/phases;
- phase flag;
- monotonicidade;
- rejeição de regressão;
- aplicação de map variants;
- spawn profiles;
- NPC profiles;
- commit em memória somente depois que adapters aplicam tudo.

Smoke test: PASS.

`Content/canonical/world_phases/world_phases.json` define o formato authoring.

## Arquitetura recomendada

```text
Story/quest coletiva
  -> SetWorldFlag("world.main.phase", 2)
  -> SQLite commit
  -> StoryWorldFlagChangedEvent
  -> ServerWorldPhaseRuntime.ReconcileFlag
  -> map/spawn/NPC/gate reconciliation
  -> jogadores online relocados/sincronizados
  -> jogador que loga depois recebe fase atual antes de entrar no mundo
```

## Irreversibilidade

A regra deve existir em duas camadas:

1. `WorldPhaseRuntime` rejeita `newPhase < currentPhase` quando `monotonic=true`;
2. repository/action gateway também deve impedir regressão no write, para restart/process race não depender apenas da memória.

Admin rollback, se existir, deve ser uma ferramenta de manutenção separada e explícita, nunca a ação normal de gameplay.

## Mudança de cidade/mapa

A geometria do field é carregada em `STG_AREA` e não existe um hot swap transacional seguro para trocar toda a cidade em place.

Para alterações grandes:

- `map.village_2` = fase antiga;
- `map.village_2_rebuilt` = fase nova;
- ao avançar a fase, novos warps resolvem para a variante;
- jogadores presentes são relocados de forma controlada;
- variante antiga fica indisponível para entrada normal.

Para alterações pequenas, não duplicar mapa: trocar/reconciliar somente spawn/NPC profile.

## Mobs que aparecem/desaparecem

Não matar/respawnar o mapa inteiro de forma cega. O profile deve ter identidade de spawn group. Na mudança:

- impedir novos spawns dos grupos removidos;
- despawn opcional dos antigos conforme política (`immediate`, `on_death`, `on_empty_area`);
- ativar grupos novos;
- manter bosses/event mobs fora da reconciliação se não pertencem à world phase.

## NPCs

Mesma estratégia: cada phase referencia profile(s). Reconcile por identidade de placement, não por posição apenas.

## Login/restart

Bootstrap obrigatório:

1. carregar world flags persistidas;
2. determinar phase ativa;
3. materializar state de world phase;
4. somente depois aceitar jogadores.

Login obrigatório:

1. validar o map salvo do personagem contra a phase atual;
2. se o map/variant não existir mais, aplicar redirect seguro;
3. enviar estado consistente ao client.

## Concorrência

O avanço coletivo deve usar compare-and-set/version no repository. Dois jogadores completando o threshold simultaneamente não podem executar a transição duas vezes.

Efeitos da fase devem ser idempotentes: aplicar phase 2 duas vezes precisa resultar no mesmo mundo.
