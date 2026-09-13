# CONHECIMENTO CONSOLIDADO DA CONVERSA — Summit SQLite R19

Data: 2026-09-11

Este documento funciona como ledger das conclusões técnicas acumuladas durante toda a conversa. Ele complementa `RETOMADA_COMPLETA_R19.md` e preserva o ledger histórico dos checkpoints anteriores. O objetivo é impedir que uma nova sessão repita diagnósticos já resolvidos ou reintroduza regressões conhecidas.

## A. Fontes de verdade e estratégia

- `pre_refactor` foi declarado funcional pelo utilizador e é a verdade de comportamento antiga.
- `final_07` é a verdade estrutural do porte/refactor desejado.
- R6.1 foi reconstruído para preservar a estrutura do porte com comportamento do `pre_refactor` e foi confirmado pelo utilizador como compilando, linkando e funcionando para cliente e servidor.
- R6.1 deve permanecer congelado como regression oracle enquanto SQLite estiver WIP.
- A migração deve trocar uma grande coisa de cada vez; não misturar correções de bugs legacy.

## B. Regressões que ensinaram a não reescrever gameplay/loaders

O primeiro porte introduziu problemas reais de linkage, loader, motion-cache, collision, rendering, preprocessador e ownership. Exemplos já diagnosticados:

- extern dentro de anonymous namespace gerando símbolos errados;
- provider `rsPutItemChecked` perdido;
- `GUIDEHELP_CPP` em ordem incorreta;
- loader dereferenciando pattern após falha;
- canonicalização/checksum `.inx` perdida;
- motion cache limitado a 30 apesar de storage 512;
- validações novas em SMD/material/OBJ incompatíveis com metadata legacy stale;
- mudanças em collision e rendering;
- `#pragma once` mudando reinclusão legacy.

Conclusão: na migração DB, preservar comportamento e interfaces, não aproveitar para “melhorar” esses subsistemas.

## C. SQLite como compatibilidade atrás das APIs existentes

A escolha que evitou reescrever o servidor inteiro foi manter:

- `SQLConnection`;
- singleton `SQL`;

mas trocar o backend para SQLite e usar rewriter para SQL Server compatível.

Isso reduz área de regressão e permite migrar callers especiais apenas onde T-SQL/control-flow ou contratos posicionais exigem.

## D. Uma DB física, muitos bancos lógicos

Um só `Database/summit.sqlite3`, com nomes físicos `<LogicalDB>__<Table>`.

Isso é necessário porque existem colisões reais como `GameMasters` em domínios diferentes.

O singleton sem `EDatabaseID` precisa de mapa explícito. O fallback para ServerDB foi removido porque era perigoso.

## E. SQL Server não pode ser traduzido mecanicamente em tudo

Descobertas importantes:

- `GETDATE()` vs UTC do SQLite;
- `IDENTITY` e `INSERT VALUES` positional;
- defaults/constraints podem não ser recuperados pelo catálogo simples;
- `SELECT *` transforma ordem de colunas em contrato;
- `INSERT OR IGNORE/REPLACE` só equivale com constraints adequadas;
- control-flow T-SQL genérico deve ser rejeitado, não interpretado superficialmente.

## F. Backups `.bak` foram a melhor fonte de schema

Os SQL scripts recebidos inicialmente eram incompletos e muitas migrations eram apenas `CREATE DATABASE`/ALTER.

Ler o catálogo real dos backups foi decisivo:

- 21 backups;
- 326 tabelas;
- 5.568 colunas;
- ordem/tipo/nullability/IDENTITY recuperados.

Isso corrigiu várias inferências anteriores e confirmou outras.

## G. AccountDB

A migration AccountDB foi comparada com o backup e bateu 69/69 tabelas e ordem de colunas.

O login local usa tabelas por inicial (`AGameUser`, etc.) e colunas nomeadas. Só o caso free comprovado é autorizado localmente.

## H. Billing

O schema existe, mas a máquina histórica não está provada.

BillingDB tem 43 tabelas, incluindo routing `<inicial>CHANNEL` e providers Excite/NetMarble/NTTBilling, mas os dados fornecidos não oferecem exemplos suficientes para reproduzir charging/expiry/PC-room.

Não confundir “schema completo” com “billing funcionalmente migrado”.

## I. Quest/Coin/PVP

- Quest `Complete` recebeu chave composta compatível com o upsert necessário.
- `PVP` recebeu unicidade por nome para `INSERT OR IGNORE` reproduzir not-exists.
- QuestMonsters seed foi convertido para SQLite idempotente.
- CoinShop usa seu schema real e o caminho JOBS foi movido de tabelas `Usuario` ausentes para `CoinShopDB.UserInfo` comprovada.

## J. Security/source-only

Algumas tabelas citadas pelo source não existem nos backups. A solução foi criar contratos mínimos source-derived **somente após remover dependências posicionais**. Elas são sempre marcadas como source-derived e não podem ser apresentadas como schema recuperado.

## K. Clan

A reconstrução do clan avançou por parser + schema + scripts Sandurr históricos.

Contratos provados:

- roles 0/1/2/5;
- sub-chief = `UL.Permi=2`;
- leader = `CL.ClanZang`;
- member list CR-delimited;
- join/kick/leave transacionais;
- banish antigo só usa `clwon1`;
- sub-chief appoint/release;
- dissolution com archive;
- create clan com LI allocator e mark default best-effort;
- K/P routes eram gates de sucesso, não prova de update de flag;
- marks e tag assets podem ser storage local.

Único contrato clan importante não reconstruído: liderança via `clanChipChange.asp`.

## L. Web legacy

No início havia dezenas de URLs em `isaocheck`. A maior parte era persistência/clan/assets/serviços externos.

Ao longo da conversa:

- leituras e mutações clan migradas;
- SoD2 migrado;
- marks/tag assets migrados;
- notice ativo já era SQLite;
- login web antigo foi classificado como dead, enquanto login real usa `SQLLogon`;
- NPCSound queue existe mas sem produtor de produção encontrado;
- warehouse/updateFlag wrappers/queues existem, mas sem caller produtor encontrado e sem semântica persistente provada.

R17 neutraliza esses caminhos no bridge servidor e deixa apenas `clanChipChange.asp` como URL HTTP literal ativa.

## M. Cliente vs servidor

Não confundir strings existentes no cliente com dependência ativa do servidor. A migração web desta fase é server-side. O cliente continua contendo código legacy e deve ser tratado separadamente, sempre sem acesso direto ao SQLite.

## N. Assets

Clan mark:

- local `Database/clan_marks`;
- tag source `Database/clan_tag_assets`;
- BMP com `BM` e limite 64 KiB;
- não sintetizar default mark ausente.

NPCSound:

- endpoint externo antigo identificado;
- nenhum caller produtor de produção confirmado no servidor;
- R17 desabilita downloader no bridge servidor.

## O. SoD2

O script histórico provou que o UI SoD usa ClanDB CL/UL para index 1/3. Não criar representação paralela desnecessária em SoD2 table para esse contrato.

## P. Notice

O caminho efetivo é `News::msgNotice`, agora com projeção `Message,Type`. O HTTP `ReadNotice` não é o mecanismo ativo encontrado.

## Q. Concurrency

WAL foi escolhido para permitir readers concorrentes e um writer. Ainda precisam de política operacional:

- retry/busy além do timeout;
- backup consistente;
- VACUUM/compactação;
- migrations exclusivas;
- coordenação se backend futuro escrever em paralelo.

## R. Build/estrutura

Nunca quebrar o contrato de translation units. Uma tentativa inicial colocou `SqliteQueryCompat.cpp` como segunda TU nova e foi revertida para `.inl` porque o source graph autoriza somente `SQLiteConnection.cpp` como nova TU servidor.

Build/link scripts continuam byte-frozen nos validadores.

## S. O que os validadores NÃO substituem

Eles provam estrutura, schema, SQL coverage e contratos unitários, não o executável real no Windows.

Sem compile/link/runtime target, não promover o WIP.

## T. Passos que mais provavelmente encerram o projeto

1. resolver ou formalmente substituir `clanChipChange`;
2. decidir billing histórico vs novo modelo;
3. decidir/importar dados históricos;
4. remover WinInet/Database.dll/ODBC quando zero callers;
5. Windows compile/link;
6. runtime regression completa;
7. promover nova baseline somente após comparação com R6.1.

---

## U. Decisões de produto/escopo após R17

- Migração/import dos dados históricos SQL Server foi explicitamente retirada do escopo. Uma DB limpa é aceitável.
- Depois que o servidor SQLite estiver funcional, devem ser fornecidas queries simples e comprovadas para criar contas `user`, `GM` e `admin`.
- Depois da estabilização, deve existir um mapa/guia prático de como usar a base SQLite.
- Objetivo futuro não é reconstruir um MMO comercial completo: multiplayer é suficiente e exportação de personagens pode ser adicionada depois.
- Apesar disso, subsistemas chamados “billing/MMO” não podem ser apagados por nome: primeiro é obrigatório provar se algum estado deles muda gameplay.

## V. Database.dll fornecida e fronteira gameplay/billing — R18

O utilizador forneceu uma `Database.dll` concreta. SHA-256:

`c9fd54668cba6e00fa760bda541587635f51c962ab6ef2b19179d0651e7220a7`

Ela mistura quatro responsabilidades diferentes:

1. autenticação/sessão;
2. PC-room/billing/accounting;
3. persistência de gameplay (`ClanMoney`, `SiegeMoney`, `SiegeMoneyTax`);
4. logs/auditoria.

O DLL possui 101 exports. O bridge servidor pede 37 símbolos; só 21 desses existem nesta DLL fornecida. Portanto vários caminhos legacy já eram provider-opcionais.

### Estado PC-room que realmente altera gameplay

`Bl_RNo > 0` foi provado como sinal de gameplay:

- +20 EXP solo;
- +20 EXP party;
- +50 no acumulador de chance/quantidade de drop;
- habilita PCBang Pet;
- também entra em fluxos `/time`, reserva e billing-time.

`DisuseDay`, `dwPeriodTime`, `dwPeriodTime2`, `PCUse` e `PCSearchReserv` controlam avisos/expiração/desconexão e podem atualizar estado durante a sessão.

Conclusão: não apagar PC-room simplesmente zerando os campos. A simplificação correta é separar o **efeito de jogo/entitlement** da antiga infraestrutura comercial de charging.

`GpType/GpCode`, no source ativo observado, alimentam sobretudo monitorização/registro PC-room por IP. O bônus direto de gameplay encontrado usa `Bl_RNo`.

## W. Gameplay persistence retirada da Database.dll no R18

### ClanMoney / SiegeMoney

O SQL embutido e o fluxo x86 da DLL foram inspecionados. R18 implementa caminho SQLite local-first preservando peculiaridades:

- nenhum check de saldo suficiente nessa camada;
- saldo pode ficar negativo;
- SELECT inicial por `UserID + ClanZang`;
- mutação historicamente por `UserID`;
- `ClanMoney` withdraw sem `DelActive`;
- deposits e SiegeMoney com `DelActive=0`;
- log `01` negativo / `02` positivo.

### SiegeMoneyTax

A DLL prova:

- total zero falha;
- clan mark precisa ter >=9 chars;
- `CL.IDX` = últimos 6 dígitos decimais do clan mark;
- crédito de imposto = `Total - round(Total/(1 + Tax*0.01f))` para tax não-zero;
- `CL.SiegeMoney += crédito` por IDX;
- insert em `SiegeMoneyTax`;
- insert em `SiegeMoneyLog` com `Server/Server/GS/02`.

R18 implementa isso local-first e mantém fallback DLL apenas se o backend SQLite falhar estruturalmente.

## X. Nova ordem de simplificação

1. manter R6.1 como oracle;
2. congelar R18;
3. separar sessão/local auth de charging histórico;
4. decidir representação simples do antigo bônus PC-room (`Bl_RNo`) sem alterar gameplay inadvertidamente;
5. migrar/neutralizar logs opcionais conforme utilidade no servidor simplificado;
6. resolver `clanChipChange.asp`;
7. retirar WinInet quando zero HTTP ativo;
8. retirar Database.dll quando zero responsabilidade necessária depender dela;
9. retirar ODBC após zero callers e build/link target;
10. validar Windows runtime completo;
11. só depois documentar administração final da DB e entregar queries user/GM/admin.


## W. Sessão local/free — R19

R19 fecha uma lacuna concreta do login SQLite R18: o caminho local validava `inuse=0`, mas não reclamava a sessão na DB. Duas tentativas podiam, em princípio, ler a conta como livre sem um write atómico de ownership.

A `Database.dll` fornecida prova os writes históricos essenciais de sessão:

- no claim/login: `StartDay`, `EditDay`, `ServerName`, `inuse='1'`, `SNo`, `RNo=NULL` para o caso free;
- no release/logout: `LastDay`, `inuse='0'`, `RNo=NULL`, `SNo=NULL`, filtrado por `userid + ServerName`.

R19 implementa isso atrás da mesma API legacy:

- `TrySQLiteLocalLogOn` passa a usar `BEGIN IMMEDIATE` e só devolve sucesso após gravar o claim da sessão;
- `TrySQLiteLocalLogout` trata localmente apenas a conta free cuja `SNo` e `ServerName` confirmam ownership; caso contrário mantém fallback `Database.dll!SQLLogout`;
- `ResetSQLiteLocalSessionsForServer` limpa sessões stale apenas da assinatura local/free no arranque e no `ServerExit` final;
- `ServerDown` não foi transformado em cleanup, porque o DLL observado só regista o server-down e o source chama essa função ainda no início do countdown de shutdown.

A limpeza stale exige simultaneamente: server atual, `inuse='1'`, `SNo` não vazio, GPCode zero/empty, `UsePeriod=0`, `BNum=0`, Channel zero/empty e `RNo` zero/null. Isto evita apagar estado paid/PC-room por aproximação.

Novo validator: `validate_sqlite_local_session.py`. Ele prova claim/release, rejeição de segundo claim, proteção por GUID/server e reset seletivo.

R19 não muda `Bl_RNo`, EXP/drop/pet, expiry timers, schema v29, `clanChipChange.asp` ou logging opcional. R6.1 continua oracle até compile/link/runtime Windows.
