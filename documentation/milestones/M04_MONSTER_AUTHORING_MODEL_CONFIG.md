# Milestone M04 — Monstros data-driven e model config JSON

## Objetivo

Fechar o workflow “criar monstro” sem depender de editar `.inf/.ini/.inx` manualmente.

## Estado atual

Gameplay de monstro já pode vir de canonical definitions compiladas para `definitions_server.scfg`. Os `.inf` físicos são legado/import quando strict compiled definitions está ativo.

O model config ainda usa o ecossistema legado: `.inx` é cache binário e o loader pode cair para config textual quando necessário. SMD/SMB/texturas continuam sendo assets binários válidos.

## Workflow final

```text
monster definition JSON
 + model_config JSON
 + SMD/SMB/textures/audio
 + spawn profile
        ↓
PT Asset Tool validation
        ↓
compile definitions/model-config cache
        ↓
server/client runtime
```

## Entregue no bundle

- catálogo de 364 model configs referenciados por 557 definições;
- `model_config_id` ligado ao conteúdo;
- inventory server/client e gaps;
- `sync_model_config_catalog.py` reutiliza parser INX do PT Asset Tool;
- schema proposto de model config.

## Migração em duas etapas

1. JSON passa a ser source of truth e ainda gera `.inx` para compatibilidade.
2. depois, criar pack próprio de model metadata e retirar loose INX do runtime.

Não transformar SMD/SMB em JSON.

## Critérios de aceite

- novo monstro pode ser criado por canonical + assets;
- stats/drop/EXP são editáveis em texto;
- model/motion config é editável em JSON;
- INX é artefato de build, não autoria;
- spawn é uma etapa explícita separada da definição;
- dependency validator encontra assets ausentes antes do publish.
