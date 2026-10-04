# Workflows de criação de conteúdo

Este arquivo é o índice operacional. Para detalhes/arquitetura, seguir os milestones.

## Criar NPC / altar

1. criar/reutilizar NPC definition canonical;
2. atribuir stable `npc_id` e service/EventCode apropriado;
3. criar placement por mapa em world coordinates;
4. validar Y/colisão em jogo;
5. compilar `field_runtime.sfld`;
6. clicar/interagir e validar autorização server-side.

Covenant: usar M03 e o patch fornecido.

## Criar monstro

1. monster definition canonical: identity/stats/EXP/drop/model ref;
2. model_config canonical;
3. garantir SMD/SMB/texturas/audio referenciados;
4. criar/adicionar spawn group no mapa;
5. validar dependency graph;
6. compilar definitions + field runtime;
7. testar spawn, combate, EXP, drops, som/animações no client.

Ver M04 e planilha de balanceamento.

## Criar mapa

1. implementar BR001 antes do primeiro field >52;
2. criar map entry client/server canonical;
3. stage asset + `map_image_id`;
4. start points/center;
5. gates/warp gates;
6. spawn + fixed NPC data;
7. minimap `field/map/<id>.tga` e title `field/title/<id>t.tga`;
8. build `maps_*.smap` + `field_runtime.sfld`;
9. parity check client/server;
10. entrar como jogador normal, não apenas GM.

## Criar classe

Seguir `M09_PLAYABLE_CLASS_WORKFLOW.md` e a referência V2 `ADDING_A_CLASS.md`: IDs/bits, creation, body/head, armor, motions, skills, UI/icons e create/login/relog.

## Criar máscara/capacete

Primeiro implementar M06. Depois cadastrar cosmetic/item e assets. Não assumir que a existência de `815mask_M/W` significa que há slot de máscara pronto.

## Criar set completo

1. cadastrar peças estáveis no `equipment_sets.json`;
2. criar um único threshold igual à soma das peças;
3. `require_complete_set=true`;
4. integrar `EquipmentSetService` ao equip/unequip server-side;
5. validar subset=0 e complete=X.

## Criar nova phase da história

1. definir objective/campaign no M01;
2. definir efeito físico no `world_phases` M02;
3. se muda geometria, criar map variant real;
4. se só muda NPC/mob/gate, usar profiles;
5. testar avanço, restart, jogador offline e impossibilidade de regressão.
