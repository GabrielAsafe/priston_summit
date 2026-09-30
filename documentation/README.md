# Summit — Documentação de Arquitetura e Operação

**Baseline documental:** REC174 + hotfix de routing pré-auth confirmado em jogo  
**Data:** 2026-09-29  
**Escopo:** cliente, servidor, networking, persistência, assets, quests, ferramentas e operação.

Esta árvore documenta o estado atual e a arquitetura alvo do Summit seguindo quatro perguntas:

1. **Por quê?** — princípios, restrições, atributos de qualidade e ADRs.
2. **Como está organizado?** — contexto, domínios, dados, interfaces, deployment e segurança.
3. **Como utilizo ou modifico?** — assets, testes, debugging e runbooks.
4. **Como chegamos do antigo ao novo?** — migração, compatibilidade, estado da refatoração e remoção do legado.

## Estado resumido

### Confirmado

- Runtime servidor usa SQLite local em `Database/summit.sqlite3`.
- Servidor inicializa definitions pack e maps pack antes de abrir a porta de jogo.
- Listener de jogo opera em `0.0.0.0:1997`.
- REC170–REC174 introduzem fronteiras v2 para networking e outros domínios.
- Hotfix de pré-autenticação restaura o fallback de opcodes legacy antes de existir `PlayerId`.
- Field runtime portado utiliza canonical authoring e runtime compilado em packs.
- Quests possuem camada v2 de repository/view/lifecycle sobre a persistência existente.

### Preparado / em validação

- Quest Codex PT-BR compilado como gameplay reference.
- Mix Guide local compilado, sem dependência de internet em runtime.
- Fix canonical de Amanda/Enzo.
- Refatoração canonical-first do Asset Tool.

### Known issues em investigação

- `_invalid_parameter` observado sob Wine em startup/runtime.
- Cliente já apresentou `wined3d_swapchain_resize_buffers` com back buffer retido.
- Houve pelo menos um crash Wine/SEH que não está root-caused.
- Recompensas de quests e equivalência da Tier 5 ainda requerem validação funcional completa.

## Índice

- `architecture/` — visão estratégica e estrutural.
- `assets/` — canonical JSON, compilação, packs, migração e Asset Tool.
- `development/` — testes e debugging.
- `operations/` — deploy, rollback, recovery e known issues.
- `refactoring/status-rec174.md` — estado factual da transformação.
