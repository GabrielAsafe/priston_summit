# Security

## Trust boundaries

```text
Client -> Network -> Server
Filesystem/canonical -> Validators -> Compiler -> Runtime
Admin tool -> Persistence
```

## Network

- client packets são não confiáveis;
- handlers modernos validam sessão/player;
- pre-auth mantém boundary correta;
- unknown opcode não deve ser tratado como authenticated gameplay.

## Assets

- canonical não validado não deve chegar ao runtime;
- schema + semantic validation precedem compilation;
- runtime pack deve validar header/version/sections.

## Persistence

- passwords não devem ser expostas em logs;
- administrative scripts devem operar explicitamente sobre uma DB selecionada;
- ações destrutivas precisam de confirmação/backup/runbook.

## Tooling

Asset Tool não deve possuir lógica de negócio divergente do runtime. Deve utilizar schemas/validators/compilers oficiais.
