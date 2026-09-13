# RETOMADA COMPLETA — Summit / SQLite WIP R21

Data: 2026-09-11

R21 continua R20. **R6.1 permanece a baseline funcional comprovada**; R21 é WIP
até compile/link/runtime Windows.

## Estado consolidado

- SQLite físico: `Database/summit.sqlite3`;
- WAL, FK ON, synchronous NORMAL, busy timeout 5000 ms, FULLMUTEX;
- migrations `001..030`, `user_version=30`, 333 user tables;
- 332 tabelas reconstruídas do catálogo histórico + 1 tabela local R21;
- login free/local SQLite com claim atómico de sessão;
- logout local-first e stale-session cleanup seletivo;
- ClanMoney, SiegeMoney e SiegeMoneyTax local-first em SQLite;
- paid/PC-room/ambiguous auth ainda tem fallback para Database.dll;
- gameplay PCBang passa pelo facade semântico criado no R20.

## Mudança R21

Migration 030 cria `LocalAuth__GameplayEntitlement` e o entitlement
`pcbang_gameplay` passa a ser uma origem local real.

O facade agora é logicamente:

`Bl_RNo > 0 || local_pcbang_gameplay`

O estado local não é gravado em `rsPLAYINFO`: ele usa um cache por slot no
servidor para não alterar layout legado. O cache é carregado após login e limpo
na alocação/desconexão.

Billing/reservation continua deliberadamente isolado: entitlement local não
aciona `PCUse`, `PCSearchReserv`, `PCRNoReservProduct`, `/time` ou accounting.

## Validação

Conjunto controlado R21: **29/29 PASS**.

Inclui prova de upgrade v29->v30 preservando dados existentes, schema/constraints
da nova tabela, expiry, cache lifecycle, login/session, Clan/Siege, query coverage,
transações e todos os validators estruturais ainda usados como gates WIP.

Quatro validators amplos/frozen continuam falhando por condições já reproduzidas
no ZIP R20 original. Ver `docs/R21_PREEXISTING_VALIDATOR_LIMITATIONS.txt`.

## Próximo passo seguro

1. compilar/linkar R21 no Windows;
2. smoke test do upgrade 29->30 numa cópia da DB real;
3. testar grant/revoke/expiry do `pcbang_gameplay` em runtime;
4. se o target for local-only, definir explicitamente a política para contas
   paid/PC-room/ambiguous e então retirar reservation/accounting/Database.dll em
   checkpoints separados;
5. migrar ou eliminar apenas os logs/estatísticas legados que ainda forem úteis.
