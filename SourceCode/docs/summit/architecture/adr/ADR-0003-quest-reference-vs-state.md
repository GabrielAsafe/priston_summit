# ADR-0003 — Separar catálogo de quests do estado dinâmico

**Estado:** Accepted  
**Data:** 2026-09-29

## Contexto

O cliente precisa de lore, NPCs, mapas e recompensas legíveis, mas dados de guia externo não podem substituir o estado autoritativo da QuestDB.

## Decisão

Quest catalog/reference é conteúdo estático compilável. Progresso, elegibilidade, counters e reward grant pertencem ao servidor.

## Alternativas consideradas

- usar o guia como banco de quests autoritativo;
- manter UI apenas com strings legacy;
- duplicar todo o estado no cliente.

## Consequências

Permite um Codex rico sem criar quests inexistentes ou comprometer autoridade do servidor.

## Impactos

Quest UI, gameplay reference pack, quest repositories e tooling.

## Migração

Vincular catálogo apenas a quests runtime identificadas. Aumentar coverage gradualmente e validar Tier5 separadamente.
