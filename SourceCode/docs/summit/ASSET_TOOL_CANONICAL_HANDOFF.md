# Asset Tool Canonical Migration — Handoff

## Estado

**INCOMPLETO.**

O patch `SUMMIT_ASSET_TOOL_CANONICAL_V2_0.patch` foi desenvolvido e validado isoladamente.

Passou:

- patch apply check;
- Python compile;
- JavaScript syntax;
- 10 targeted tests;
- canonical preferred;
- legacy recovery preserved.

## O que implementa

Canonical-first somente para o Field Editor:

```text
SPM -> stage_spawns
SPP -> stage_start_points
SPC -> fixed_npcs
```

Módulo:

```text
pt_asset_tool/summit_canonical.py
```

UI/API alteradas:

```text
pt_asset_tool/ui_app.py
pt_asset_tool/ui_static/app.js
pt_asset_tool/ui_static/index.html
```

Workflow:

```text
Draft
 -> Save canonical
 -> Build packs
 -> Deploy packs
```

Safety:

- drafts fora da source;
- write arm obrigatório;
- `SAVE_CANONICAL`;
- SHA guard;
- canonical não grava `.SPM/.SPP/.SPC`;
- legacy field editor fica recovery-only.

## Gap principal

A ferramenta ainda não é um authoring frontend completo para o novo JSON contract.

Faltam editors canonical para:

- `npc_definition`;
- `monster_definition`;
- `open_item_definition`;
- localization/text;
- gameplay config quando apropriado;
- gameplay reference quests/mix.

## Design recomendado

Criar:

```text
CanonicalRepository
CanonicalDraft
CanonicalValidator
CanonicalPublisher
CanonicalBuildJob
CanonicalDeployJob
```

e adapters por família.

Nunca gerar bytes históricos com uma serialização genérica.

`raw_values_hex`, payload Base64 e encoding precisam de codecs domain-specific.

## Physical assets

SMD/SMB/INX/textures/effects continuam uma linha independente.

Não tentar converter os workflows Blender/reverse-engineering em canonical JSON.

O canonical deve referenciar stable paths/IDs para os physical assets.
