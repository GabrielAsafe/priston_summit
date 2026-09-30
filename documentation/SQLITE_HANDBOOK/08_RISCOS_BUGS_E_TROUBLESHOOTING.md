# 8. Riscos, bugs e troubleshooting

## 8.1 Matriz rápida de sintomas

| Sintoma | Causa provável |
|---|---|
| Login 1 funciona, relogin falha | `inuse=1` não liberado |
| CoinShop vazio | catálogo `ItemList` sem dados ou DB errada |
| TimeShop vazio | `ItemList_T` sem dados |
| saldo 0 inesperado | `UserInfo`/`UserInfo_T` ausente ou conta diferente |
| GM commands somem | `GameMasters` sem row/active/security |
| `/skiptier` falha | T-SQL não suportado / QuestDB operation falhou |
| personagem some | `userinfo` aponta para `userdata` inválido |
| level reseta | record completo não foi persistido |
| servidor usa dados antigos | runtime/CWD errado |
| migration não aplica | `user_version`, arquivo ausente, SQL inválido |
| DB parece corrupta após cp | cópia ignorou WAL |
| `sqlite3.dll` não carrega | DLL errada, 64-bit, caminho errado |

## 8.2 Risco: runtime errado

Rodar:

```text
Servidor/Server.exe
```

em vez de:

```text
Servidor_R25_Wine/Server.exe
```

pode usar:

- DB diferente;
- DataServer diferente;
- configs diferentes;
- DLLs diferentes.

Sempre confirmar CWD.

## 8.3 Risco: schema runtime != migrations

Estado atual:

```text
runtime: 339 tabelas
clean R25 v32: 338 tabelas
```

Há também divergência de NULLability.

Até existir migration v33, um bootstrap limpo não é bit-for-bit equivalente.

## 8.4 Risco: NULLability reconstruída errada

Foram encontrados dados históricos `NULL` em colunas que R25 havia marcado `NOT NULL`.

Casos confirmados:

```text
ClanDB__SiegeMoneyLog.RegistDay
ClanDB__SiegeMoneyTax.RegistDay
CoinShopDB__PendingPayments.CompletedDate
GameLogDB__CharGameRecordLg.RegistDay
GameLogDB__CharGameRecordLg.DisuseDay
GameLogDB__CharGameRecordLg.EditDay
GameLogDB__CharGameRecordLg.Active
```

Política:

```text
NULL histórico permanece NULL.
```

## 8.5 Risco: SELECT * e ordem de colunas

Grande parte do código legado usa leitura posicional.

Uma migration que reordena colunas pode:

- ler Level como Exp;
- ler string como integer;
- corromper lógica sem crash;
- produzir bugs difíceis de rastrear.

## 8.6 Risco: INSERT positional

Tabelas como `MonsterKilled` ainda recebem:

```sql
INSERT ... VALUES(...)
```

Adicionar coluna no meio quebra o insert.

## 8.7 Risco: REPLACE sem UNIQUE

`INSERT OR REPLACE` só substitui quando existe conflito de constraint.

Sem UNIQUE correto, ele apenas adiciona outra row.

Sintomas:

- quests duplicadas;
- saldos duplicados;
- estado ambíguo.

## 8.8 Risco: case sensitivity de nomes de database

No inventário houve diferenças como:

```text
BillingDb vs BillingDB
SodDb vs SodDB
Jobs vs JOBS
```

SQLite trata identificadores de modo permissivo em muitos contextos, mas ferramentas de migração não devem assumir string exata sem normalização.

## 8.9 Risco: ADM__loginGM

A tabela existe no runtime atual porque foi importada.

Mas não existe `DATABASEID_ADM` no backend.

Se uma feature futura tentar usar:

```cpp
GetConnection(DATABASEID_ADM)
```

não existe hoje.

Decidir explicitamente se:

- é apenas dado histórico;
- deve ser migrada para outro domínio;
- deve nascer um logical database ADM.

## 8.10 Risco: passwords legadas

Conta e GameMaster usam campos de texto do sistema legado.

Não expor dumps dessa DB.

Não publicar:

```text
Passwd
GameMasters.password
tokens
emails
IPs
```

## 8.11 Risco: sessões stale

Se o processo cai antes do logout, `inuse` pode permanecer.

R19 introduziu cleanup de sessões stale.

Ao diagnosticar, verificar primeiro:

```sql
SELECT userid,inuse,ServerName,SNo,StartDay,LastDay
FROM AccountDB__?GameUser
WHERE userid=?;
```

## 8.12 Risco: personagem não é transacional com SQLite

É possível ter:

- conta criada na SQLite;
- slot em `userinfo`;
- save `userdata` inválido.

Não existe uma transação ACID única englobando DB + ficheiro de personagem.

Testes de criação devem validar ambos.

## 8.13 Risco: diagnóstico virar correção funcional sem perceber

Foi exatamente o caso do `/skiptier`.

A patch chamada “diag” também:

- trocou `ISNULL` por `COALESCE`;
- trocou T-SQL control-flow por `INSERT OR REPLACE`.

O comando começou a funcionar.

Ao remover logging, manter essas mudanças.

## 8.14 Risco: legacy_billing_fallback

Migration 032 semeia `1`.

Se o objetivo é local-only:

- auditar contas;
- confirmar zero sessões;
- backup;
- mudar para `0`;
- restart.

Não mudar durante outro teste não relacionado e depois confundir causas.

## 8.15 Risco: SQLOLEDB/Wine warnings

Warnings antigos de OLE DB não devem ser declarados como causa de crash sem correlação.

O servidor já executou sessão real estável sem crash enquanto esses warnings existiam.

## 8.16 Procedimento de diagnóstico

1. reproduzir uma vez;
2. provar qual camada falha;
3. inspecionar DB/file state;
4. instrumentar a operação exata;
5. build clean pelo wrapper oficial;
6. validar deploy hash;
7. repetir um caso controlado;
8. só então patchar semântica.
