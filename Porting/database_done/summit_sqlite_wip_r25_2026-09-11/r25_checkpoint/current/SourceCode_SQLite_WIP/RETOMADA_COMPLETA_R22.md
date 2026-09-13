# RETOMADA COMPLETA — Summit / SQLite WIP R22

Data: 2026-09-11

R22 continua o R21. **R6.1 permanece a baseline funcional comprovada**; R22 é
WIP até compile/link/runtime Windows.

## Estado atual

- SQLite local: `Database/summit.sqlite3`;
- WAL, FK ON, synchronous NORMAL, busy timeout 5000 ms, FULLMUTEX;
- migrations `001..031`;
- `user_version=31`;
- 336 user tables = 332 pré-R21 + 1 entitlement local + 3 audit locais R22;
- login free/local SQLite com claim atómico;
- logout local-first + stale-session cleanup seletivo;
- entitlement `pcbang_gameplay` local independente de `Bl_RNo`;
- drop +50, EXP solo +20%, EXP party +20% e PCBang Pet passam pelo facade;
- ClanMoney, SiegeMoney e SiegeMoneyTax local-first;
- SoD result statistics local-first;
- character new/delete/logout audit local-first;
- item/trade/gift audit local-first transacional;
- paid/PC-room/ambiguous auth continua fallback para `Database.dll`.

## Mudança R22

### 1. SoD statistics

`GAMELOG_SQL_SOD` grava primeiro em `SoD2DB__SOD2RecBySandurr`. O fallback
`WithSOD2RecordRanks` só é chamado se SQLite falhar.

### 2. Character audit

Migration 031 cria `GameLogDB__LocalCharacterEvent`. Eventos 1/2/3 preservam o
payload local de criação, exclusão e logout; `CharRecordLog`/`CharLog` ficam
fallback-only.

### 3. Item audit

Migration 031 cria `ItemLogDB__LocalItemEvent` e
`ItemLogDB__LocalItemEventEntry`. Item/trade/post são persistidos na worker
existente em transação única. A fila não depende mais da presença dos exports
`LogITEMRecord`/`LogITEMTradeRecord` para funcionar localmente.

## Validação

Controlled suite: **31/31 PASS**. Ver `SQLITE_WIP_R22_VALIDATION.md` e
`docs/R22_VALIDATION_RAW.txt`.

## O que ainda impede remover Database.dll

O restante relevante está concentrado em política/runtime de auth e PC-room:

- `SQLLogon` para contas paid/PC-room/ambíguas;
- `SQLLogout` para sessões não locais;
- `GetGPCode`;
- `PCUse`;
- `PCSearchReserv`;
- `PCRNoReservProduct`;
- `ServerDown` / `ServerExit` para provider legado;
- fallbacks de Clan/Siege/SoD/audit enquanto não houver prova Windows.

Vários exports solicitados pelo código nem existem na DLL fornecida e já eram
pointer-guarded; não devem ser reconstruídos sem necessidade explícita.

## Próximo passo seguro

1. compile/link R22 no Windows;
2. smoke de login/session + entitlement + SoD/audit workers;
3. decidir política final para contas paid/PC-room;
4. se o alvo for servidor totalmente local, retirar `PCUse`/reservas/accounting
   e converter o auth restante para uma política local explícita;
5. depois remover os fallbacks da DLL já cobertos por SQLite.

Não reconstruir charging histórico apenas para manter gameplay.
