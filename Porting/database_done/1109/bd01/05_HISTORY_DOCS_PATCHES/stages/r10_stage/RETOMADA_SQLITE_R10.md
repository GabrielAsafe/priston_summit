# RETOMADA — SQLite WIP R10

Data: 2026-09-06

## Estado seguro

- **R6.1 continua sendo a última baseline funcional comprovada.**
- R10 é WIP; não promover antes de compile/link/runtime Windows.
- Schema permanece `user_version=17`, 113 tabelas.

## Delta R9 -> R10

`isKflag.asp` e `isPFlag.asp` foram retirados do bridge servidor.

`ISAOCHECK::isKPFlag` e `ISAOCHECK::isPFlag` preservam localmente o contrato de acknowledgement (`1`) e não criam writes artificiais em `CL.PFlag/KFlag` ou `UL.PFlag/KFlag`.

## Validações

PASS: migrations, SQL coverage, clan contract, transaction API, source graph, build scripts, structure 1517/1517 e compiler adaptation.

## Próxima ordem

1. migrar leitura de clan marks para storage local controlado pelo servidor;
2. manter criação de clan bloqueada até existir uma política/default mark local comprovada;
3. provar `clanChipChange` antes de migrar liderança;
4. não inventar semântica para warehouse/updateFlag;
5. depois atacar login/SoD/outros HTTP por contrato comprovado.
