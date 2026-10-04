# Audit — `/Servidor/Field` e `GameServer/Field`

## Duas coisas diferentes

### `/Servidor/Field`

É principalmente repositório de assets de stage: SMD/ASE e materiais/texturas. O servidor usa stage geometry para o mundo/collision; esses binários não são “config que deveria virar JSON”.

JSON deve referenciar o asset:

```json
{"stage":"boss//dark_boss.ASE"}
```

O runtime/caches podem resolver a representação binária apropriada.

### `GameServer/Field` legado

Historicamente contém `.spm/.spp/.spc` de gameplay/placement. Na arquitetura atual, seus equivalentes já foram migrados para canonical e compilados em `field_runtime.sfld`:

- stage spawns;
- stage start points;
- fixed NPCs.

O runtime atual lê o pack e o save/edit legado de placement está desabilitado no caminho modernizado.

## Decisão

- manter geometry/assets binários;
- manter canonical JSON para semântica de map/spawn/NPC/gate;
- arquivar old `.spm/.spp/.spc` depois de parity final;
- PT Asset Tool deve editar o canonical, não o arquivo legado.

## Server headless futuro

Mesmo num servidor headless ainda pode ser necessário manter geometry/collision de stage. “Headless” significa remover render/material/texture dependencies desnecessárias, não remover a geometria que o server usa para navegação/altura/colisão.
