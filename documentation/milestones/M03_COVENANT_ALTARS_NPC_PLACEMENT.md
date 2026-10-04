# Milestone M03 — NPC placement e altares Covenant

## Objetivo

Transformar “colocar um altar em cada mapa” em conteúdo reutilizável: uma definição de NPC/altar, placements separados por mapa, e interação que abre a UI Covenant existente sem duplicar regra de negócio.

## Base na source

- fixed NPC placement já é separado da definição do NPC;
- `FieldPlacementDefinition` possui `npcId`, `definitionPath`, XYZ, rotação, slot/state;
- `STG_AREA::LoadCharInfoFixed` materializa placements do `field_runtime.sfld`;
- `LegacyInteractionAdapter` é chamado no fluxo de interação e já classifica `EventCode`;
- Covenant V2 já possui protocolo/state/UI; o README do sistema admite abertura futura por NPC/altar.

## Implementação

- reservar `EventCode 12050` para altar Covenant (POC deste bundle);
- definition canonical única `npc.covenant_altar`;
- no clique, server valida target/map/distância e envia snapshot Covenant autoritativo;
- client abre a tab `Covenants` existente;
- nenhum join/activate é aceito apenas porque o client abriu a UI.

## Placement

`content/.../covenant_altars/placement_plan.json` contém candidatos para os 53 maps, todos `validated=false` até coordenada/Y ser confirmada em jogo.

Padronizar authoring em world coordinates e converter ×256 somente no compiler de fixed NPC.

## Critérios de aceite

- uma definition de altar é reutilizada;
- placement por map é data-driven;
- clique abre UI Covenant;
- servidor continua autoridade de ações;
- altitude/colisão de cada placement é validada antes de publish;
- PT Asset Tool permite adicionar/mover altar e recompilar `field_runtime.sfld`.
