# CONHECIMENTO CONSOLIDADO DO CHAT — R20

Data: 2026-09-11

## Baseline e política

- R6.1 é a última baseline comprovada em compile/link/runtime pelo utilizador.
- R20 é WIP experimental e não substitui R6.1.
- Não importar histórico SQL Server; DB limpa é aceitável.
- Cliente nunca abre SQLite.
- Não corrigir gameplay/UB/render/collision não relacionado durante a migração.

## SQLite

- `Database/summit.sqlite3`;
- migrations 001..029;
- `user_version=29`;
- 332 tabelas de utilizador;
- API `SQLConnection` preservada sobre SQLite.

## R18/R19 já concluído em WIP

- ClanMoney/SiegeMoney/SiegeMoneyTax local-first;
- login free/local SQLite;
- claim atómico `inuse=1`, ServerName, SNo/GUID e timestamps;
- logout SQLite local-first por GUID/server;
- stale-session cleanup seletivo no startup/ServerExit;
- paid/PC-room continua fallback.

## R20

`Bl_RNo` era usado diretamente em quatro efeitos de gameplay. R20 introduz
`rsHasPCBangGameplayEntitlement()` e troca somente esses quatro call sites:

- +50 drop;
- +20% EXP solo;
- +20% EXP party;
- PCBang Pet.

O helper ainda é `Bl_RNo > 0`, então R20 é behavior-preserving. Billing,
reservas, expiry, logs e token inter-server continuam usando `Bl_RNo` cru.

A intenção é permitir que um entitlement local futuro substitua a origem do
benefício sem tocar novamente nas fórmulas de gameplay.

## Validação

- 28 validators do escopo R20 PASS;
- novo `validate_gameplay_entitlement_boundary.py` PASS;
- dois validadores estruturais legados têm defeitos preexistentes comprovados
  também no ZIP R19 original e ficaram fora do gate R20.

## Bloqueio principal

Falta compile/link/runtime Windows. Antes disso, não alterar layout de
`rsPLAYINFO`, não inventar `Bl_RNo` sintético e não reutilizar `Bl_Meter` como
entitlement local.
