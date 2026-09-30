# Debugging Guide

## Regra 1 — separar camadas

Para “falha ao conectar”:

1. listener existe?
2. TCP conecta?
3. pre-auth packet chega?
4. router consome?
5. authentication retorna?
6. character/session inicia?

Não alterar password/database antes de separar TCP de protocolo.

## Regra 2 — Wine não é automaticamente causa raiz

Mensagens `fixme:` ou `_invalid_parameter` podem coexistir com operação normal. Correlacionar sempre com:

- thread;
- momento;
- última fase do jogo;
- reproducibilidade;
- crash dump/backtrace;
- alteração recente.

## Regra 3 — conteúdos

Para asset failure:

- confirmar pack presente;
- confirmar inspect;
- confirmar hash;
- confirmar binding/ID;
- confirmar loader utilizado;
- só depois restaurar legacy.

## Regra 4 — logs com contexto

Preferir tags:

```text
[NETWORK][PREAUTH]
[CONTENT][FIELD]
[QUEST][REWARD]
[ASSET][VALIDATION]
```
