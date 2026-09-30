# ADR-0004 — Consolidar SPM/SPP/SPC em field_runtime.sfld

**Estado:** Accepted  
**Data:** 2026-09-29

## Contexto

Spawns, start points e fixed NPCs dependiam de três famílias legacy separadas e estruturas binárias históricas.

## Decisão

Authoring separado por domínio em canonical; compilation consolida SSPB, SPPT e SPCN num único `field_runtime.sfld`.

## Alternativas consideradas

- manter três ficheiros runtime legacy;
- um pack separado por família;
- JSON direto no servidor.

## Consequências

Deploy simples e runtime determinístico. Alterações canonical exigem recompilação do pack.

## Impactos

Field loaders, build_from_canonical, deploy, validation e Asset Tool.

## Migração

Importar legacy, validar contagens/equivalência, migrar runtime, desativar mutations legacy e testar com ficheiros físicos em quarentena.
