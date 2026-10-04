# Milestone M05 — Bônus de set server-authoritative

## Objetivo

Permitir sets novos por conteúdo, incluindo a regra “1 peça = 0; somente conjunto completo = X bonus”.

## Base

O legado possui dois sets hardcoded no client. Isso serve como referência de comportamento, não como arquitetura nova.

## Solução

- canonical `equipment_sets.json`;
- stable item IDs e quantidade requerida;
- thresholds opcionais;
- `require_complete_set` para o caso do usuário;
- servidor reconcilia modifiers em login/equip/unequip;
- client só apresenta bonus/stats;
- migrar os dois boss sets legados antes de remover código antigo.

## Artefato

`patches/equipment_sets/EquipmentSetService.*` + smoke test PASS.

## Critérios de aceite

- subset não dá bonus quando set é complete-only;
- equip final ativa no servidor;
- remover qualquer peça retira imediatamente;
- relog preserva resultado derivado do equipamento, sem persistir modifier duplicado;
- client adulterado não consegue ganhar bonus sem set real.
