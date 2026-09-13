# RETOMADA — SQLite WIP R9

Data: 2026-09-06

## Estado seguro

- **R6.1 continua sendo a última baseline funcional comprovada pelo utilizador.**
- R9 é WIP de persistência/remoção de web e ainda exige compile/link/runtime Windows.
- Não alterar gameplay/refactor enquanto esta migração estiver em curso.

## Estado SQLite

- ficheiro físico alvo: `Database/summit.sqlite3`;
- migrations automáticas: `001` até `017`;
- `PRAGMA user_version`: 17;
- tabelas de utilizador após DB limpa: 113;
- WAL + foreign_keys + synchronous NORMAL + busy timeout permanecem configurados;
- `SQLConnection` e singleton `SQL` continuam como interfaces de compatibilidade;
- singleton não faz fallback silencioso de tabela desconhecida para ServerDB.

## Clan migrado para SQLite direto até R9

Já não dependem dos ASP correspondentes no bridge servidor:

- `isCheckClanJang`
- `isCheckClanName`
- `ReadClanInfo` / check by mark
- `isCheckClanMember`
- `isClanMember`
- `read_ClanData` / GetAllMyClanMember
- `joinClanMember`
- `Banishiment`
- `Secession`
- `AppointSubChip`
- `ReleaseSubChip`
- `BreakUPClan`

`BreakUPClan` arquiva dados nas tabelas reais `ClanMainDeleted` e `ClanSubDeleted` e faz a operação atomicamente.

## Schema ClanDB comprovado

`CL`, `UL`, `ClanMainDeleted` e `ClanSubDeleted` são baseados no catálogo extraído de `ClanDB202405120913.bak`.

Não mudar ordem/nome/tipo por conveniência sem nova evidência.

## Criação de clan ainda bloqueada

O SQL do `clanInsert.aspx` histórico pode ser reconstruído e o backup prova a tabela `LI`, mas a criação também copia o BMP default do clan mark. Nenhum `1000000000.bmp` foi encontrado nos materiais recebidos.

Não converter `makeClan` para retornar sucesso enquanto o armazenamento/default mark local não estiver definido e disponível.

## Validação R9

PASS:

- migrations v17 / 113 tabelas;
- query coverage;
- clan SQLite contract, incluindo dissolution/archive;
- transaction API;
- source graph;
- build/link scripts congelados;
- estrutura 1517/1517;
- compiler adaptation;
- patch R8 -> R9 aplicado em árvore limpa e byte-equivalente.

## Próxima ordem recomendada

1. resolver armazenamento local de clan marks/default mark;
2. migrar `makeClan` usando `CL`, `UL`, `LI` em uma transaction e asset local;
3. provar/migrar `clanChipChange`;
4. provar/migrar PFlag/KFlag e warehouse rights;
5. migrar clan-mark read/write para storage local + metadata SQLite;
6. retirar demais HTTP de serviço (notice/SoD/login/audio) por contrato comprovado;
7. só remover WinInet/ODBC do linker depois de zero callers ativos;
8. executar compile/link/runtime Windows e regressão contra R6.1.
