# Refactoring Status — REC174

**Data:** 2026-09-29

## Confirmado

### Networking

- REC170–174 introduzem/refinam boundaries modernas.
- Regressão de login identificada no `PacketRouter`.
- Hotfix confirmado em jogo: unknown/pre-auth opcode cai novamente no fluxo legacy.
- TCP 1997 confirmado funcional.

### Persistence

- SQLite local abre no startup.
- Problema de login recente não era causado por password/account persistence.

### Asset runtime

- Definitions e maps são carregados por packs no startup.
- Field structures migradas para canonical/compiled runtime na linha de trabalho atual.
- Quarantine/recovery continua a ser a estratégia correta para provar ausência de reads legacy residuais.

### Quests

- Arquitetura v2 repository/view/lifecycle está presente no baseline REC174.
- Runtime reward path precisa continuar a ser testado em jogo.

## Preparado, não declarar production sem teste

- Quest Codex PT-BR.
- gameplay reference pack.
- Mix Guide local.
- Amanda/Enzo canonical fix.
- Asset Tool canonical-first.

## Pendências

1. Testar recompensas de quests.
2. Validar Tier 5.
3. Validar Codex contra quests runtime existentes.
4. Validar Mix Guide no NPC.
5. Aplicar/validar Amanda e Enzo.
6. Rebasear e integrar Asset Tool canonical-first.
7. Root-cause client Wine crash.
8. Completar CI de schema/pack/runtime contract.
9. Remover legacy apenas quando consumers residuais forem zero.

## Definition of Done desta transformação

Não considerar concluída apenas por build verde. Exigir documentação, tests, Asset Tool, validators, runbooks e observabilidade.
