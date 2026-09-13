# CONHECIMENTO CONSOLIDADO DA CONVERSA — Summit SQLite R17

Data: 2026-09-06

Este documento funciona como ledger das conclusões técnicas acumuladas durante toda a conversa. Ele complementa `RETOMADA_COMPLETA_R17.md` e o checkpoint original. O objetivo é impedir que uma nova sessão repita diagnósticos já resolvidos ou reintroduza regressões conhecidas.

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
