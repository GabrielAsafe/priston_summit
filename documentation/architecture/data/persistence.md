# Persistence

## Implementação observada

SQLite local:

```text
Servidor/Database/summit.sqlite3
```

No startup observado, o servidor abre o SQLite e anuncia os contextos lógicos antes de inicializar gameplay.

## Ownership

- Accounts/Auth: conta, bloqueio, password policy e sessão.
- Characters: dados persistentes de personagem.
- Quests: estado dinâmico e progresso.
- Economy: moedas e operações económicas.

## Regra

Nenhum domínio deve assumir que o modelo de persistência é igual ao modelo de domínio ou ao modelo de transporte.

## Repositories

Novas boundaries devem encapsular SQL e mapear:

```text
Persistence Model
      ↓
Repository
      ↓
Domain / Application Service
```

## Migrations

Toda mudança de schema persistente deve possuir:

- versão;
- objetivo;
- SQL/migration explícita;
- validação;
- rollback ou estratégia compensatória;
- compatibilidade por fases quando necessário.
