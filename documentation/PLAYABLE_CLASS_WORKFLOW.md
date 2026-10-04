# Criar nova classe/personagem jogável

A source já possui `PlayableClassRegistry`/V2 e um documento upstream `Shared/gameplay/classes/v2/docs/ADDING_A_CLASS.md`. Uma classe nova deve entrar como definição de classe + integrações explícitas, não por novos `switch(JOB_CODE)` espalhados.

## Contratos obrigatórios

Antes de habilitar:

- stable class id;
- legacy job code se wire/save ainda exigir;
- item specialization bit;
- motion applicability mask (não é o mesmo bit do item);
- brood/gender;
- stats base/fórmula;
- body/head appearance;
- armor models;
- weapon/equipment policy;
- skill tree/ranks;
- localization/presentation.

## Assets do client

Uma classe completa precisa, conforme os caminhos usados pelo registry:

- body/base models;
- faces/hair/head;
- armor normal e variantes suportadas;
- model configs/motions;
- skill icons/UI;
- party alive/dead icons;
- ranking/SOD/kill-notification icons quando usados;
- skill effects/particles;
- sounds das skills/ações quando definidos.

## Servidor

O servidor não precisa renderizar UI/icons, mas hoje ainda carrega parte do ecossistema de modelos. Até torná-lo headless, o deployment server precisa incluir os model configs/model files realmente referenciados pela classe.

## Testes mínimos antes de `enabled=true`

- registry Validate;
- create/select;
- login;
- save/relog;
- death/respawn;
- equip/unequip;
- specialization parsing;
- body/head/armor;
- motion applicability;
- skill tree e rank progression;
- party UI;
- ranking/SOD quando suportado;
- packets com job code novo;
- nenhuma indexação `jobCode - 1` nova;
- nenhuma suposição `MAX_JOBNUM == 8` nova.

O documento original `ADDING_A_CLASS.md` deve ser tratado como referência normativa da refatoração V2.
