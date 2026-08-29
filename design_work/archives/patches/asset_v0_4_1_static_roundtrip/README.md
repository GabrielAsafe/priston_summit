# Summit Tale Asset Tool V0.4.1 — Static Authoring Roundtrip

Base: V0.4.0 (player head promotion tiers).

## Objetivo

Fechar o primeiro gate de authoring real para `SMD_MODEL_062_STATIC` simples, especialmente weapons/items:

`SMD original -> Blender -> edição witness -> writer SMD -> comparação -> capability reparse -> Blender reimport`.

O cliente nunca é modificado.

## Novo comando

```bash
python3 pt_asset.py validate roundtrip <static.smd>
```

Opção:

```bash
--witness-strength 0.08
```

A força é fração da diagonal do bounding box. A alteração é proposital e ocorre somente no `.blend` de authoring e no SMD reconstruído em `pt_build/`.

## Critérios automáticos

O gate V1 exige:

- família `SMD_MODEL_062_STATIC`;
- zero Physique;
- zero object-local animation;
- zero multilayer TexLink;
- exatamente um objeto authoring-ready no V1;
- material region byte-identical;
- nomes e contagem de objetos preservados;
- objetos não editados byte-identical;
- faces/UV/material semanticamente idênticos;
- contagens de vertices/faces/TexLinks preservadas;
- SMD reconstruído byte-diferente do original;
- somente os vertices registrados pelo witness podem mudar;
- cada posição alterada deve corresponder exatamente ao valor quantizado produzido no Blender;
- novo SMD deve passar novamente pelo static capability probe;
- novo SMD deve reimportar no Blender.

Após esses checks automáticos, o Blender abre o rebuild para inspeção humana de geometria e materiais.

## Certificação

- `STATIC_ROUNDTRIP_STRUCTURAL / AUTOMATIC`
- `STATIC_ROUNDTRIP / HUMAN_VISUAL`

Histórico permanece append-only.

## Representantes alvo

1. `image/Sinimage/Items/DropItem/itPR101.smd` — 1 objeto, 1 material, 5V/4F.
2. `image/Sinimage/Items/DropItem/itWA104.smd` — 1 objeto, 1 material, 86V/146F.
3. `image/Sinimage/Items/DropItem/itWH118.smd` — 1 objeto, 13 materiais, 94V/150F.

Esses três cobrem pequeno, geometria média e multi-material.

## Fora do V1

Continuam bloqueados, sem relaxar validators:

- mixed-Physique;
- object-local animation;
- multilayer TexLink;
- static multi-object authoring genérico.

Essas capacidades permanecem etapas próprias do roadmap.

## Segurança

Nenhuma etapa escreve no cliente. O artifact reconstruído fica apenas em:

`pt_build/pt_asset_tool/<ASSET_ID>/static_roundtrip/rebuilt_roundtrip.smd`

Publish/restore continuam separados e protegidos por política de SHA/CLIENT_DRIFT.
