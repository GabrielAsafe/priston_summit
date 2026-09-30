# 9. Histórico da migração e mudanças

## 9.1 Baseline

R6.1 foi mantido como oracle funcional durante a migração.

O objetivo foi retirar dependência de SQL Server/Database.dll gradualmente sem reescrever todo o servidor de uma vez.

## 9.2 R18

Mudanças locais para economia de clã/castelo:

```text
ClanMoney
SiegeMoney
SiegeMoneyTax
```

local-first.

## 9.3 R19

Lifecycle de sessão SQLite:

- claim atômico com `BEGIN IMMEDIATE`;
- logout local-first;
- cleanup de sessões stale.

## 9.4 R20

Criada facade semântica para entitlement gameplay:

```text
rsHasPCBangGameplayEntitlement(...)
```

Substituiu efeitos acoplados diretamente a:

```text
Bl_RNo > 0
```

## 9.5 R21

Persistência local do entitlement.

## 9.6 R22

Sinks não autoritativos migrados para audit local.

Tabelas append-only locais para character/item events.

## 9.7 R23

Database.dll tornou-se opcional.

Introduzidos:

```text
LocalAuth__AccountPolicy
LocalAuth__RuntimePolicy
```

e ferramenta:

```text
tools/sqlite_local_admin.py
```

Auth modes:

```text
local
legacy
implicit
```

Logout local torna-se autoridade para contas locais.

GameMasters passou a ser validado em SQLite.

## 9.8 R24

Auditoria/classificação local-only e validação ampliada.

## 9.9 R25

Foco operacional:

- backup via SQLite backup API;
- suporte WAL correto;
- quick_check;
- foreign_key_check;
- schema count;
- `prepare-local-only`;
- controlled validation;
- schema v32 / 338 tabelas clean.

## 9.10 Integração no source/build Linux + Wine

Foi confirmado:

```text
server compile: 187/187
client compile: 203/203
PE32/i386
```

SQLite é carregada via `sqlite3.dll` x86.

Runtime oficial passou a ser:

```text
Servidor_R25_Wine
```

## 9.11 Fix de deploy

`send_to_s.sh` ainda apontava via SCP para um ambiente Windows antigo.

Foi alterado para deploy local em:

```text
Servidor_R25_Wine
```

com checks de:

- processo ativo;
- DLLs;
- `Database.dll`;
- `sqlite3.dll`;
- SHA do `Server.exe`.

## 9.12 Fix de logout/relogin

Problema:

```text
bSql_Logout(...)
```

era executado apenas se:

```text
szGPCode[0]
```

Uma conta local nova tinha `GPCode=""`.

Resultado:

```text
inuse=1
relogin bloqueado
```

Correção:

```text
sempre enfileirar bSql_Logout;
worker decide local-first/fallback.
```

Após correção, relogin funcionou.

## 9.13 Descoberta de dados ausentes

Após criar conta nova:

- comandos GM ausentes;
- CoinShop/TimeShop vazios;
- saldos ausentes;
- QuestDB insuficiente.

Audit mostrou que o SQLite tinha schema, mas várias tabelas tinham zero dados.

## 9.14 Export completo do SQL Server Docker

Resultado final:

```text
22 bancos
329 tabelas
1391 registros
329/329 exportadas
```

Foi necessário corrigir particularidades do `sqlcmd` Linux para JSON.

## 9.15 Merge completo no SQLite

Merge preservou:

- `LocalAuth__*`;
- eventos locais R25;
- conta local de teste;
- GM local;
- dados históricos.

Resultado:

```text
TABLES_BEFORE=338
TABLES_AFTER=339
INSERTED=1385
SKIPPED_EXACT=2
PRESERVED_ACCOUNT_IDENTITY=4
GM_ID_REMAPPED=2
CREATED_MISSING_NONEMPTY=1
QUICK_CHECK=ok
```

## 9.16 Fidelidade de NULL

Durante import apareceram constraints R25 mais restritivas que os dados reais.

Foi adotada regra:

```text
NULL legado permanece NULL
```

O staging relaxou constraints específicas em quatro tabelas.

Isso precisa ser consolidado numa migration futura.

## 9.17 CoinShop/TimeShop restaurados

Depois do import:

```text
CoinShop ItemList=110
TimeShop ItemList_T=34
saldos=34/34
```

A loja voltou a exibir itens.

## 9.18 `/skiptier`

Mesmo depois do import, `/skiptier` falhava.

As quests 10010–10013 não existem em `SingleQuest`, mas o bypass não deveria depender disso.

O source live tinha T-SQL incompatível.

A patch de diagnóstico alterou a função para SQLite-native:

```text
ISNULL -> COALESCE
IF/BEGIN/ELSE -> INSERT OR REPLACE
GETDATE -> datetime('now','localtime')
```

e adicionou tracing.

Depois disso o comando funcionou.

## 9.19 Estado final desta fase

Confirmado:

- login;
- relogin;
- criação de personagem;
- gameplay;
- evolução de level;
- CoinShop com itens;
- TimeShop data importado;
- saldos importados;
- GM configurável;
- `/skiptier` funcional;
- QuestDB populada;
- quick_check OK.

Pendências arquiteturais principais:

1. migration v33 para reproduzir schema runtime;
2. cleanup do logging `/skiptier` sem perder a correção SQL;
3. endpoint de rede final sem hardcode;
4. local-only final com fallback explicitamente decidido;
5. consolidar runtime `Servidor_R25_Wine` em nome definitivo se desejado.
