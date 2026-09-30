# Summit Tale SQLite — Handbook R25

**Estado documentado:** 12 de setembro de 2026  
**Runtime de referência:** Linux + Wine  
**Servidor de referência:** `/home/gabriel/Área de Trabalho/summit/Servidor_R25_Wine`  
**Source de referência:** `/home/gabriel/Área de Trabalho/summit/SourceCode`

Este conjunto de documentos descreve a base SQLite usada pelo servidor Summit após a migração do SQL Server, incluindo o estado real validado em runtime, a camada de compatibilidade SQL, contas, autenticação, GM, personagens, quests, CoinShop/TimeShop, migrations, riscos e procedimentos de desenvolvimento.

## Estado atual em uma página

O servidor usa **um único ficheiro físico**:

```text
Servidor_R25_Wine/Database/summit.sqlite3
```

Os antigos bancos lógicos do SQL Server são representados por prefixos de tabela:

```text
AccountDB.dbo.GameMasters  -> AccountDB__GameMasters
QuestDB.dbo.Progress       -> QuestDB__Progress
CoinShopDB.dbo.UserInfo    -> CoinShopDB__UserInfo
ServerDB.dbo.PVP           -> ServerDB__PVP
```

A camada C++ continua a expor as APIs legadas `SQLConnection` e `SQL`, mas internamente ambas usam SQLite.

O bootstrap R25 original tem:

```text
PRAGMA user_version = 32
338 tabelas de utilizador
migrations 001..032
```

Depois da importação completa dos dados do SQL Server no runtime validado:

```text
user_version = 32
339 tabelas
329 tabelas exportadas do SQL Server
1391 registros exportados/validados
1385 linhas inseridas no merge
2 duplicados exatos ignorados
4 identidades locais de conta preservadas
2 IDs de GameMasters remapeados
1 tabela não existente criada: ADM__loginGM
quick_check = ok
```

A diferença `338 -> 339` e algumas alterações de NULLability ainda **não estão consolidadas numa migration oficial**. Isso é uma dívida técnica importante e está descrito em `06_MIGRATIONS_SCHEMA_E_EXTENSOES.md`.

## Regra operacional principal

Neste momento, o runtime oficial é:

```bash
cd "/home/gabriel/Área de Trabalho/summit/Servidor_R25_Wine"
wine Server.exe
```

Não executar o `Server.exe` a partir da pasta antiga `Servidor` como se fosse equivalente. O servidor usa caminhos relativos para DB, DataServer, LOGS, configs e outros recursos.

## Índice

1. `01_ARQUITETURA_E_ESTADO_ATUAL.md`  
   Arquitetura geral, ficheiro físico, namespaces, WAL, SQLite DLL, APIs e mapa de dados.

2. `02_RUNTIME_DEPLOY_BACKUP_E_OPERACAO.md`  
   Runtime oficial, build/deploy, backups consistentes, WAL, health checks e recuperação.

3. `03_CONTAS_AUTH_GM_E_PERSONAGENS.md`  
   Criação de conta com Python, auth local, GM, sessões e diferença entre conta e personagem.

4. `04_QUESTDB_E_CRIACAO_DE_QUESTS.md`  
   Estrutura completa de QuestDB, como adicionar quests e o caso `/skiptier`.

5. `05_COINSHOP_TIMESHOP_E_ECONOMIA.md`  
   Catálogos, saldos, identidade lógica, novos itens e riscos econômicos.

6. `06_MIGRATIONS_SCHEMA_E_EXTENSOES.md`  
   Como criar migration nova, alterar schema, constraints, índices e proposta de v33.

7. `07_SQL_COMPAT_E_PADROES_DE_CODIGO.md`  
   SQL Server dialect vs SQLite, o que o compat layer aceita e o que deve ser migrado manualmente.

8. `08_RISCOS_BUGS_E_TROUBLESHOOTING.md`  
   Onde pode quebrar, sintomas, causas prováveis e diagnóstico.

9. `09_HISTORICO_DA_MIGRACAO_E_MUDANCAS.md`  
   Resumo R18–R25 e mudanças feitas durante esta integração.

10. `10_CHECKLIST_DE_DESENVOLVIMENTO.md`  
    Checklist curto para alterações futuras.

11. `11_SQLITE_CHEATSHEET.md`  
    Consultas úteis de inspeção, saúde, quests, shops e contas.

## Filosofia para alterações futuras

As regras que mais evitam bugs são:

- não editar o banco runtime como substituto de uma migration;
- preservar `NULL` históricos como `NULL`;
- não mudar ordem de colunas em tabelas lidas por `SELECT *`;
- não assumir que SQLite guarda o personagem completo;
- não introduzir SQL Server control-flow (`IF/BEGIN/ELSE`) em queries SQLite;
- não usar `INSERT OR REPLACE` sem uma chave `UNIQUE`/`PRIMARY KEY` que represente a identidade lógica correta;
- fazer backup pela SQLite backup API, nunca copiar apenas `summit.sqlite3` com WAL ativo;
- testar migrations num banco vazio e também numa cópia do banco real;
- manter Linux + Wine como runtime de aceitação.
