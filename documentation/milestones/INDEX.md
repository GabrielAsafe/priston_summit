# Milestones — ordem recomendada

## Fundação

1. **M01_WORLD_STORY_COLLECTIVE** — campanha global, contribuições, persistência, monotonicidade.
2. **M02_WORLD_PHASES_CITY_VARIANTS** — materialização física: cidade, mobs, NPCs, gates e relocation.
3. **M03_COVENANT_ALTARS_NPC_PLACEMENT** — altar reutilizável + placements.

## Conteúdo

4. **M04_MONSTER_AUTHORING_MODEL_CONFIG** — monster gameplay + model config JSON/INX como artefato.
5. **M05_EQUIPMENT_SETS_SERVER_AUTH** — sets genéricos no servidor.
6. **M06_HEAD_COSMETICS_MASKS** — máscara/head cosmetic e caminho para helmet real.
7. **M09_PLAYABLE_CLASS_WORKFLOW** — workflow completo de classe nova.

## Balance / migração / tooling

8. **M07_BALANCE_EXP_DROPS_GOLD** — canonical balance + simulador/parity.
9. **M08_CONFIGS_LEGACY_CLEANUP** — separar gameplay/runtime/secrets e aposentar resíduos.
10. **M10_PT_ASSET_TOOL_CONTENT_PLATFORM** — editor/validator/build/publish oficial sobre canonical.
11. **M11_SERVER_CHAR_HEADLESS** — reduzir assets no servidor depois que o loader estiver headless.

## Dependência externa ao milestone

Antes de usar muitos map variants, implementar `bugreports/BR001_MAP_CONTENT_DRIVEN_FIELD53.md`.
