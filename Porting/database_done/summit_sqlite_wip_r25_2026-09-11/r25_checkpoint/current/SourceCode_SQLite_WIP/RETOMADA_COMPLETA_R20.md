# RETOMADA COMPLETA — Summit / SQLite WIP R20

Data: 2026-09-11

R20 continua o R19. **R6.1 permanece a baseline funcional comprovada**; R20 é
WIP e não deve ser promovido apenas por validação estática.

## Estado herdado

- SQLite local em `Database/summit.sqlite3`;
- WAL, FK ON, synchronous NORMAL, busy timeout 5000 ms, FULLMUTEX;
- migrations `001..029`, `user_version=29`, 332 user tables;
- login free/local SQLite com claim atómico de sessão;
- logout local-first e stale-session cleanup seletivo;
- `ClanMoney`, `SiegeMoney` e `SiegeMoneyTax` local-first em SQLite;
- paid/PC-room/ambiguous auth continua fallback para `Database.dll`;
- `clanChipChange.asp` continua o único HTTP funcional conhecido no servidor.

## Mudança R20

R20 cria a fronteira semântica:

```cpp
rsHasPCBangGameplayEntitlement(lpPlayInfo)
```

Ela ainda devolve exatamente `Bl_RNo > 0`, portanto não muda regras.

Foram migrados para essa fronteira apenas os quatro efeitos de gameplay
comprovados:

- `+50` no acumulador de drop;
- `+20%` EXP solo;
- `+20%` EXP party;
- ativação do PCBang Pet.

Não foram alterados:

- `PCUse`, `PCSearchReserv`, `PCRNoReservProduct`;
- `/time`;
- timers `DisuseDay`, `dwPeriodTime`, `dwPeriodTime2`;
- login/logout billing parameters;
- logs/accounting;
- payload/token inter-server do PCBang Pet.

Isto evita que uma futura origem local de entitlement exija voltar a editar as
fórmulas de EXP/drop/pet.

## Schema

R20 não altera schema. Continua:

- migrations `001..029`;
- `user_version=29`;
- 332 user tables.

## Validação

Novo validator:

- `validate_gameplay_entitlement_boundary.py`.

Conjunto controlado R20: 28/28 PASS. Ver:

- `SQLITE_WIP_R20_VALIDATION.md`;
- `docs/R20_VALIDATION_RAW.txt`;
- `docs/R20_PREEXISTING_VALIDATOR_LIMITATIONS.txt`.

Dois validadores estruturais antigos já falham no ZIP R19 original por manifests
stale; não são regressão R20 e não foram "corrigidos" fora de escopo.

## Próximo passo seguro

1. compilar/linkar R20 no Windows e executar smoke tests de login/session;
2. provar em runtime que os quatro benefícios mantêm comportamento idêntico;
3. só depois escolher a representação do entitlement local (sem reutilizar
   `Bl_RNo` falso nem bits de billing);
4. então substituir a implementação do facade e começar a retirar PC-room
   accounting/reservation de forma controlada.

Nunca reconstruir charging histórico só para manter um perk de gameplay.
