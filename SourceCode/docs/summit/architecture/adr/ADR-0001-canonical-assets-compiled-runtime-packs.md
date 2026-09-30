# ADR-0001 — Canonical JSON como authoring e packs compilados como runtime

**Estado:** Accepted  
**Data:** 2026-09-29

## Contexto

Milhares de ficheiros e formatos legacy misturavam authoring, ABI e runtime. Alterar conteúdo exigia conhecer layouts históricos e aumentava o risco de corrupção.

## Decisão

Canonical estruturado é a fonte de verdade editável. Validators e compilers geram packs binários consumidos por client/server.

## Alternativas consideradas

- manter leitura direta dos ficheiros legacy;
- fazer o runtime ler JSON diretamente;
- incorporar tudo em código C++.

## Consequências

Melhora auditabilidade, validação e startup. Introduz responsabilidade explícita de build/deploy de conteúdo.

## Impactos

Asset pipeline, runtime loaders, CI, Asset Tool, deployment e recovery.

## Migração

Importar legacy para canonical, validar equivalência, migrar loaders, testar runtime sem legacy, manter legacy apenas para import/recovery e remover depois.
