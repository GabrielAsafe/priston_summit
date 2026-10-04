# Milestone M02 — Mundo físico por phase: cidade destruída/construída, mobs e NPCs

## Objetivo

Materializar uma phase global em mudanças concretas do mundo:

- trocar uma cidade inteira por outra versão visual;
- habilitar/desabilitar NPCs;
- habilitar/desabilitar grupos de monstros;
- mudar gates/warps;
- redirecionar jogadores que estavam na versão antiga;
- garantir que o estado sobrevive a restart e é aplicado a quem entra depois.

Este milestone implementa a parte “Bully quando muda de estação”, mas usando a arquitetura de fields do Summit.

## Restrição descoberta na source

A geometria de um field é carregada no runtime legado em estruturas como `STG_AREA`/`sField`. Não há um mecanismo transacional existente para substituir toda a geometria de uma cidade já carregada apenas trocando uma `StoryWorldFlag`.

Por isso existem **duas estratégias diferentes**, conforme o tamanho da mudança.

## Estratégia A — overlay de conteúdo

Usar quando a geometria não muda.

Exemplos:

- guards novos;
- altar aparece;
- comerciantes desaparecem;
- monstros antigos deixam de spawnar;
- novos monstros entram no mapa;
- gate fica ativo/inativo.

A mesma `map_id` permanece. O runtime troca somente profiles de NPC/spawn/gate.

Vantagens:

- sem reload de stage 3D;
- transição pode ocorrer com players no mapa;
- menor risco.

## Estratégia B — map variant

Usar quando a geometria realmente muda.

Exemplos:

- Ricarten intacta;
- Ricarten destruída;
- Ricarten reconstruída.

Cada geometria é um field estático real e content-driven:

```text
map.ricarten.normal
map.ricarten.destroyed
map.ricarten.rebuilt
```

Um identificador lógico agrupa as variantes:

```text
logical_map = world_location.ricarten
```

A phase resolve qual `map_id` representa esse local naquele momento.

## Por que não alterar o `.smd` “por baixo” do mesmo field

Fazer hot swap do stage no mesmo índice enquanto players/mobs/NPCs possuem referências a estruturas antigas cria risco de:

- ponteiros inválidos;
- collision state inconsistente;
- spawns ligados ao stage anterior;
- warp/gate state divergente entre client/server;
- jogador permanecer em coordenada que não existe na nova geometria.

O design com variantes torna a transição explícita e testável.

## `WorldPhaseRuntime`

O POC incluído no bundle já modela:

```cpp
MapVariant {
  logicalMap,
  variantMap,
  transitionPolicy
}
```

```cpp
PhaseDefinition {
  value,
  key,
  maps,
  spawnProfiles,
  npcProfiles
}
```

O runtime rejeita regressão quando `monotonic=true` e só marca a phase ativa em memória depois de o gateway aplicar os efeitos.

## Gateway server real

Implementar `IWorldPhaseGateway` no servidor com quatro responsabilidades.

### 1. Map routing

Manter um resolver:

```text
logical location -> active map variant
```

Todo gate que representa uma ligação narrativa deve resolver o destino através desse resolver, não armazenar permanentemente a variante antiga.

Exemplo:

```text
warp para world_location.ricarten
          |
phase 0 -> map.ricarten.normal
phase 1 -> map.ricarten.destroyed
phase 2 -> map.ricarten.rebuilt
```

Durante a compatibility layer, o resultado final continua sendo `legacy_field_code`.

### 2. Spawn profiles

Não tratar o `.spm` inteiro como uma unidade opaca para world story. Criar identidade estável de grupos de spawn.

Exemplo:

```json
{
  "id": "spawn.ricarten.destroyed.invaders",
  "map": "map.ricarten.destroyed",
  "members": [ ... ]
}
```

Policies de remoção:

- `immediate`: despawn imediato;
- `on_death`: mobs atuais permanecem até morrer, mas não respawnam;
- `on_empty_area`: troca quando não houver player no field.

Default recomendado para mobs normais: `on_death`.

### 3. NPC profiles

Placements precisam de identidade estável. Não reconciliar NPC somente por XYZ.

Exemplo:

```text
placement.ricarten.blacksmith
placement.ricarten.covenant_altar
placement.ricarten.refugee_01
```

Na troca de phase:

- remover placements que deixaram de existir;
- manter os que continuam com mesmo ID;
- adicionar os novos;
- atualizar service/definition se a phase exigir.

### 4. Player relocation

Quando a variante muda:

1. bloquear novas entradas na variante antiga;
2. marcar o field como retiring;
3. para `relocate_players`, obter os players presentes;
4. resolver destino seguro na nova variante;
5. enviar warp/field transition normal, não teleportar memória local arbitrariamente;
6. liberar/retirar a variante antiga conforme política.

## Políticas de transição

O catálogo deve suportar pelo menos:

### `relocate_players`

Transição imediata. Adequada a evento narrativo explícito.

### `drain_then_retire`

Novos players já entram na variante nova, quem está na antiga pode terminar a sessão/instância; o field antigo é aposentado quando vazio.

### `next_entry`

Não move ninguém. O player troca de variante quando sair/entrar novamente. Útil para alterações não urgentes.

## Safe points

Cada variante que pode receber jogadores por phase transition deve possuir um start/safe point dedicado.

Nunca presumir que `x,z` da cidade antiga continua válido na geometria nova.

O canonical pode declarar:

```json
{
  "transition_spawn": "start.ricarten.rebuilt.story_entry"
}
```

## Gates

Hoje `gates`/`warp_gates` fazem parte do map content. Para world phases, adicionar uma camada de resolução sem destruir o contrato atual.

Duas opções:

1. gate de cada variante já aponta ao destino correto daquela phase;
2. para ligações narrativas compartilhadas, usar `target_logical_map`, resolvido para `target_map` durante compile/materialization.

A segunda é preferível quando o mesmo gate precisa acompanhar várias fases.

## Client

O client não decide a phase.

Ele recebe/descobre somente o field autorizado pelo server e carrega os assets desse variant.

Para transição dramática pode receber um evento UI/cinematic adicional, mas a autoridade de qual mapa está ativo permanece server-side.

## Assets

Cada variante visual precisa do mesmo checklist de qualquer mapa:

- stage geometry (`ASE` lógico / cache `SMD` conforme pipeline);
- materiais/texturas;
- minimap `field/map/<map_image_id>.tga`;
- title `field/title/<map_image_id>t.tga`;
- map catalog client/server;
- start points;
- field runtime spawn/NPC data;
- gates/warp gates;
- música/ambiente se diferente.

Não converter `SMD` para JSON. O JSON referencia o asset binário.

## Coordenadas

Para authoring, padronizar tudo em **world/overlay coordinates**.

O legado usa fixed-point com `FLOATNS=8` para certas estruturas de personagem/NPC:

```text
runtime_internal = world_coordinate * 256
world_coordinate = runtime_internal / 256
```

Placements de NPC atualmente preservam valores internos. Isso é ruim para edição porque o debug overlay mostra a escala world.

Novo canonical de placement deve declarar `coordinate_space: "world"`; `compile_field_runtime.py` converte para fixed-point somente no pack/runtime.

Não aplicar essa conversão às rotações `ax/ay/az`.

## Sequência de phase transition

Exemplo phase 0 -> 1:

```text
1. M01 persiste world.main.phase = 1
2. publica StoryWorldFlagChangedEvent
3. WorldPhaseRuntime localiza phase 1
4. gateway ativa routing da nova variante
5. gateway ativa novos spawn profiles
6. gateway ativa novos NPC profiles
7. gateway aplica gate routing
8. players da variante aposentada são relocados conforme policy
9. runtime marca phase 1 reconciliada
10. telemetria confirma conclusão
```

Se uma etapa falhar antes do mundo ficar consistente, registrar erro fatal/operacional e manter o sistema em estado reconciliável. A operação precisa ser idempotente para poder ser repetida.

## Restart durante/apos transição

O estado persistente é a phase, não o estado efêmero de cada `STG_AREA`.

Após restart:

```text
DB phase -> WorldPhaseRuntime -> materializa tudo de novo
```

Isso evita precisar persistir “NPC X já foi removido” individualmente quando ele é derivável da phase.

## Relação com o bug de field53

Variantes consomem novos fields. Portanto o bug report `BR001_MAP_CONTENT_DRIVEN_FIELD53.md` deve ser resolvido antes de uma campanha começar a multiplicar variantes de cidades.

## Testes

### Overlay-only

- phase 0 possui mob A/NPC A;
- phase 1 remove A e ativa mob B/NPC B;
- não troca geometry;
- player online vê a mudança;
- relog mantém phase 1.

### Geometry variant

- player está em `ricarten.normal`;
- phase avança;
- novas entradas resolvem `ricarten.destroyed`;
- player presente é movido para safe point do variant novo;
- field normal fica inacessível por gameplay;
- restart retorna em destroyed.

### Gate routing

- gate de outro mapa para Ricarten antes da transition -> normal;
- depois -> destroyed;
- nenhum client packet pode escolher a variante antiga.

### Spawn lifecycle

- mob antigo com `on_death` não desaparece abruptamente;
- depois de morrer não respawna;
- mob novo começa a spawnar;
- boss/event mob fora do profile não é afetado.

## Critérios de aceite

- mudanças pequenas não exigem duplicar geometria;
- mudanças grandes usam variants seguros;
- jogadores nunca entram numa variante aposentada por gate/login;
- restart reconstrói o mundo somente a partir da phase persistida + conteúdo;
- server é autoridade;
- não há regressão de phase;
- transição é idempotente;
- conteúdo pode adicionar nova phase sem adicionar um `if (phase == X)` no C++.

## Artefatos já incluídos

- `patches/world_phases/WorldPhaseRuntime.h/.cpp`;
- smoke test correspondente;
- `content/Content/canonical/world_phases/world_phases.json`;
- schema `world_phases.schema.json`;
- bug report de maps no bundle.
