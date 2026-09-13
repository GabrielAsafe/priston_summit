# Conhecimento consolidado — Summit SQLite R22

Data: 2026-09-11

- R6.1 continua sendo o oracle funcional/runtime.
- R22 é WIP estático; ainda falta compile/link/runtime Windows.
- SQLite: `Database/summit.sqlite3`, migrations 001..031, user_version 31, 336 tabelas.
- 326 tabelas vêm dos 21 backups SQL Server; as demais incluem contratos source-derived, entitlement local R21 e 3 audit tables R22.
- Login free/local é SQLite com claim atómico; paid/PC-room/ambíguo ainda cai para Database.dll.
- Logout local-first libera apenas sessão GUID/server local; stale local sessions são limpas seletivamente.
- `pcbang_gameplay` local é independente de Bl_RNo e alimenta o facade dos quatro perks; billing continua isolado.
- ClanMoney, SiegeMoney e SiegeMoneyTax são SQLite-first.
- R22 torna `WithSOD2RecordRanks` SQLite-first em `SoD2DB__SOD2RecBySandurr`.
- R22 torna CharLog/CharRecordLog local-first em `GameLogDB__LocalCharacterEvent`.
- R22 torna LogITEMRecord/Trade/Post local-first em `ItemLogDB__LocalItemEvent` + `LocalItemEventEntry`, transacionalmente.
- Esses audit sinks não são gameplay gates; fallbacks da DLL permanecem somente se o SQLite local falhar.
- Controlled suite R22: 31/31 PASS.
- Os quatro broad/frozen validators continuam falhando com as mesmas contagens do R21; não são regressões R22.
- Restante real da Database.dll está concentrado em auth/PC-room (`SQLLogon`, non-local `SQLLogout`, GetGPCode, PCUse, PCSearchReserv, PCRNoReservProduct, ServerDown/Exit) e fallbacks que só devem ser removidos após prova Windows/política final.
