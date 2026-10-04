# Milestone M07 — Balanceamento de EXP, drops e gold

## Objetivo

Centralizar parâmetros de balanceamento em conteúdo canônico e permitir simulação antes de publicar.

## Já fechado

- curva 1..161 extraída;
- 98 monstros e 7.003 entradas de drop analisadas;
- nomes dos itens ligados na planilha;
- `FallItem` confirmado como peso relativo;
- múltiplos códigos dividem peso por divisão inteira;
- `ExtraFallItem` usa roll independente base 10.000;
- gold possui peso + min/max + rate;
- party 7/8 ainda tinha fallback hardcoded;
- `srGetTotalExp` tinha intermediário `int` apesar de input INT64.

## Entregas

- planilha em `spreadsheets/`;
- canonical `gameplay_balance/experience.json`;
- patch INT64;
- patch party 2..8 configurável;
- materializador de ExpManager.

## Próximo estado

PT Asset Tool deve mostrar XP/h, kills/level, expected drops/hour e expected gold/hour com parâmetros de tempo de kill informados pelo designer.

## Critérios de aceite

- nenhuma party rate 2..8 fica hardcoded;
- matemática de EXP não overflowna no endgame;
- unidade de event EXP é única/documentada;
- alterações de balance passam por canonical e diff legível;
- simulador consegue reproduzir runtime para amostras conhecidas.
