# ADR-0002 — Routing moderno verifica ownership do opcode antes de autenticação

**Estado:** Accepted  
**Data:** 2026-09-29

## Contexto

REC170 introduziu routing v2 antes do switch legacy. Durante login ainda não existe PlayerId; validar PlayerId antes de ownership descartava packets de pré-auth.

## Decisão

PacketRouter procura primeiro um handler do opcode. Só opcodes possuídos pelo router moderno exigem PlayerId válido. Unknown opcodes retornam UnknownOpcode para a boundary legacy.

## Alternativas consideradas

- registar PlayerId artificial antes do login;
- duplicar handlers de login no router moderno imediatamente;
- desativar router v2 no servidor.

## Consequências

Restaura login e mantém validação forte para gameplay moderno. Preserva migração incremental.

## Impactos

Networking, authentication/session boundary e testes de regressão.

## Migração

Manter teste com unknown opcode + invalid PlayerId. Migrar login para boundary moderna apenas através de ADR/contrato separado.
