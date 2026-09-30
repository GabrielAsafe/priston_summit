# 10. Checklist de desenvolvimento

## Antes de mexer em DB

- [ ] Estou na pasta `Servidor_R25_Wine` correta?
- [ ] `Server.exe` está parado para alteração estrutural?
- [ ] Tenho backup via SQLite backup API?
- [ ] `PRAGMA quick_check` retorna `ok`?
- [ ] Sei se a feature é SQLite ou DataServer file-backed?

## Ao criar tabela/migration

- [ ] Criei migration nova, sem editar migration já aplicada?
- [ ] Atualizei lista em `SQLiteConnection.cpp`?
- [ ] Atualizei `user_version` esperado via migration engine?
- [ ] PK/UNIQUE representam identidade lógica?
- [ ] NULLability foi provada por dados/código?
- [ ] Preservei ordem de colunas se há `SELECT *`?
- [ ] Verifiquei inserts posicionais?
- [ ] Criei índices necessários?

## Ao escrever query C++

- [ ] Usei binds?
- [ ] Evitei `IF/BEGIN/ELSE` T-SQL?
- [ ] Evitei `ISNULL()` e usei `COALESCE()`?
- [ ] Usei `datetime('now','localtime')`?
- [ ] INSERT nomeia as colunas?
- [ ] UPSERT tem UNIQUE/PK correspondente?
- [ ] Tenho `GetLastError()` num caminho de diagnóstico?

## Ao criar conta

- [ ] Usei `sqlite_local_admin.py create-account`?
- [ ] Auth mode é `local` se desejado?
- [ ] Verifiquei `show <account>`?
- [ ] Personagem será criado pelo fluxo normal do cliente?

## Ao criar GM

- [ ] Conta existe?
- [ ] `set-gm` aplicado?
- [ ] security correto 1..4?
- [ ] active=1?
- [ ] `/ativar` testado?

## Ao criar quest

- [ ] QuestID não colide?
- [ ] Categoria correta: single/daily/repeated?
- [ ] QuestKind correto?
- [ ] QuestType consistente em todas as tabelas?
- [ ] QuestMonsters inserido?
- [ ] Rewards inserido?
- [ ] ItemRewards, se necessário?
- [ ] Aceite cria Progress?
- [ ] Kills criam/atualizam MonsterKilled?
- [ ] Conclusão remove Progress/MonsterKilled?
- [ ] Complete recebe row?
- [ ] reward econômico testado?

## Ao adicionar shop item

- [ ] ItemCode existe?
- [ ] seção é exibida pelo cliente?
- [ ] preço correto?
- [ ] compra real testada?
- [ ] saldo decrementa uma vez?
- [ ] item chega uma vez?

## Antes de build

Servidor:

```bash
CLEAN=1 sh server.sh
```

Cliente, se necessário:

```bash
CLEAN=1 sh cliente.sh
```

- [ ] deploy terminou para `Servidor_R25_Wine`?
- [ ] hash dist/runtime confere?
- [ ] processo antigo não ficou vivo?

## Smoke pós-alteração

- [ ] server startup completo?
- [ ] porta 1997?
- [ ] login?
- [ ] logout?
- [ ] relogin imediato?
- [ ] personagem carrega?
- [ ] save persiste?
- [ ] CoinShop?
- [ ] TimeShop?
- [ ] quest alterada?
- [ ] GM command relevante?
- [ ] `quick_check=ok`?

## Antes de considerar concluído

- [ ] mudança está no source/migration, não só no DB runtime?
- [ ] existe rollback?
- [ ] documentação foi atualizada?
- [ ] bootstrap vazio reproduz o estado?
