# Autonomous Content Operations Cookbook

## Antes de qualquer alteração

```bash
cd ~/Área\ de\ Trabalho/summit
git status --short
```

Não usar comandos Git globais destrutivos.

## Remover NPC de cidade

```bash
find SourceCode/Content/canonical/fixed_npcs -type f -iname '*MAPA*.json'
jq '.npcs' ARQUIVO
```

Remover a row correta pelo `npc_id`.

Validar, build, deploy.

## Procurar uma NPC definition

```bash
grep -RIl '"id": "npc.EXEMPLO"' \
  SourceCode/Content/canonical/text/resources
```

## Procurar onde um NPC está colocado

```bash
grep -RIn '"npc_id": "npc.EXEMPLO"' \
  SourceCode/Content/canonical/fixed_npcs
```

## Procurar monster definition

```bash
grep -RIn '"family": "monster_definition"' \
  SourceCode/Content/canonical/text/resources
```

O melhor identificador é o stable canonical ID, não display name.

## Rebuild seguro

```bash
python3 SourceCode/tools/content_port/build_from_canonical.py \
  --canonical-text SourceCode/Content/canonical/text \
  --canonical-spawns SourceCode/Content/canonical/stage_spawns \
  --canonical-maps SourceCode/Content/canonical/maps \
  --canonical-start-points SourceCode/Content/canonical/stage_start_points \
  --canonical-fixed-npcs SourceCode/Content/canonical/fixed_npcs \
  --canonical-item-profiles SourceCode/Content/canonical/item_profiles \
  --out-root SourceCode/Content
```

## Deploy

```bash
python3 SourceCode/tools/content_port/deploy_runtime_content.py --repo-root .
```

## Closure validator

```bash
python3 SourceCode/tools/content_port/validate_gameplay_closure.py --repo-root .
```

## Recompilar executáveis

Só necessário quando C++ mudou:

```bash
cd SourceCode
sh compboth.sh
cd ..
```

Mudança somente de canonical/packs normalmente não exige recompilar `Game.exe`/`Server.exe`, mas exige rebuild dos packs + deploy + restart.
