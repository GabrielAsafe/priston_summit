# Asset Tool

## Estado alvo

O Asset Tool edita canonical e chama as mesmas validações/compilers utilizadas no pipeline oficial.

```text
UI
 ↓
Canonical Repository
 ↓
Schema/Semantic Validation
 ↓
Save atomic
 ↓
Build packs
 ↓
Inspect
 ↓
Deploy
```

## Recovery legacy

Editors/writers de SPM/SPP/SPC/NPC/INF/TXT legacy só devem existir sob uma área explicitamente marcada como Import/Recovery.

## Requisitos

- dry-run;
- atomic save;
- hash/race protection;
- schema-driven validation;
- provenance;
- build output visível;
- deploy explícito;
- rollback/recovery;
- não duplicar regras de gameplay.

## Próximas famílias

Depois de Field canonical-first:

- NPC;
- Monster;
- OpenItem/items;
- quests/reference;
- localization/text.

Conversões que manipulam `raw_values_hex` ou encoding histórico precisam preservar bytes/semântica até existir normalização comprovada.
