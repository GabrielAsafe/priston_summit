# Milestone M06 — Máscaras/capacetes

## Objetivo

Criar suporte de cabeça sem confundir assets de máscara existentes com um slot realmente implementado.

## Decisão

Fase 1: `head_cosmetic`, sem stats.

Fase 2 opcional: `head_armor`, com stats, somente depois da auditoria de inventory/network/save/render.

## Motivo

O legado não possui HEAD/HELMET/MASK; possui COSTUME e earrings. Há assets como `815mask_M/W`, porém o character pattern nativo trabalha principalmente com body + head, então o caminho de menor risco é head replacement cosmético, preservando o head base para restauração.

## Critérios de aceite fase 1

- server valida ownership/equip;
- save/relog;
- replicação para outros players;
- fallback quando asset não existe;
- máscara não altera combate;
- não conflita com earrings/costume;
- PT Asset Tool valida variantes male/female/class quando necessárias.
