# Milestone M10 — PT Asset Tool como frontend oficial de conteúdo

## Objetivo

Usar o programa criado pelo projeto como editor/validator/build/publisher, mantendo `Content/canonical/**` como source of truth versionável.

## Responsabilidades

PT Asset Tool deve:

- abrir/editar canonical JSON com schema-aware forms;
- criar map e sugerir próximo `legacy_field_code` após BR001;
- mostrar coordenadas world iguais ao debug overlay;
- editar NPC placements visualmente;
- editar monster stats/drop/model refs;
- decodificar/editar model config e gerar INX de compatibilidade;
- editar equipment sets, world phases, experience balance, head cosmetics;
- mostrar dependency graph;
- localizar referências antes de rename/delete;
- executar `tools/content_port` em staging;
- validar packs;
- publicar client/server de forma transacional.

## Dependency graph prioritário

```text
map
 ├─ stage asset
 ├─ minimap/title
 ├─ gates -> map
 ├─ fixed NPC -> NPC definition -> model_config -> assets
 └─ spawn group -> monster definition
                    ├─ drops -> item definition -> assets
                    └─ model_config -> SMD/SMB/textures/audio
```

## Regra de arquitetura

A UI nunca deve guardar estado exclusivo que não possa ser reconstruído dos arquivos canônicos. Fechar o PT Asset Tool não pode tornar o conteúdo impossível de editar via Git/texto.

## Critérios de aceite

- create/edit/build/publish de conteúdo novo sem editor legado externo;
- build falha antes de publish em referência quebrada;
- diff canonical é legível;
- geração de packs usa os compiladores reais em `tools/content_port`;
- nenhum segredo do servidor é importado para canonical client-visible.
