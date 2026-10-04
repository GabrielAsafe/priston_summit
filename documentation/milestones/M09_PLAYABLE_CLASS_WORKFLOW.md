# Milestone M09 — Nova classe/personagem jogável

## Objetivo

Permitir adicionar uma classe sem procurar manualmente todos os arrays/IDs/assets.

## Base

`Shared/gameplay/classes/v2/docs/ADDING_A_CLASS.md` e `PlayableClassRegistry` já definem a direção da refatoração. Ainda existem contratos legados com job code/job bit/motion mask que precisam continuar distintos.

## Workflow

1. stable class id;
2. legacy job code/protocol value;
3. item job bit e motion job mask;
4. base stats/brood/selection order;
5. body/head/faces;
6. armor model roots/variants;
7. motion coverage;
8. skills/tree/ranks;
9. icons/presentation/party/ranking;
10. create/select/login/save/relog;
11. dependency validation pelo PT Asset Tool.

## Critérios de aceite

Nenhum passo novo exige inserir mais uma suposição `jobCode - 1` ou `MAX_JOBNUM == 8`; assets ausentes bloqueiam publish com mensagem clara.
