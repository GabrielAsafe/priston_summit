# Domain — Assets

## Responsabilidade

Gerir authoring, validação, compilação, distribuição e resolução de conteúdo estático.

## Fonte de verdade

A fonte de verdade é canonical estruturado. Runtime packs são artefactos derivados.

## Famílias relevantes

### Field runtime

```text
stage_spawns
stage_start_points
fixed_npcs
        ↓
field_runtime.sfld
        ├── SSPB
        ├── SPPT
        └── SPCN
```

### Definitions

NPCs, monstros, items e outras definições são compilados em definitions packs por perfil.

### Maps

Map metadata é compilado em `maps_client.smap` / `maps_server.smap`.

### Gameplay reference

Quest Codex e Mix Guide são conteúdo de referência, não estado dinâmico do jogador. A integração preparada utiliza `gameplay_reference.sgrf`.

## Regra

Gameplay não deve abrir JSON canonical. Gameplay solicita uma definição ao content/repository layer.
