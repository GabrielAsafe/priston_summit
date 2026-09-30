# Quality Attributes

## Reliability

- Nenhum pacote de login deve ser descartado por routing incorreto.
- Recompensas devem ser idempotentes quando a operação puder repetir.
- Loaders devem rejeitar packs inválidos antes de materializar estado parcial.

## Maintainability

- Novos módulos devem reduzir dependência de monólitos legacy.
- APIs devem expor comportamento do domínio, não detalhes de ficheiros/SQL.
- Dados canonical devem possuir schemas e validators.

## Testability

Mudanças estruturais devem privilegiar:

- unit tests;
- smoke tests portáteis;
- contract tests;
- schema validation;
- migration tests;
- integration tests;
- regressões de rede e persistência.

## Operability

Logs devem permitir identificar:

- fase de startup;
- pack carregado;
- listener aberto;
- routing/network boundary;
- falha de persistência;
- erro de asset;
- erro Wine distinto de erro da aplicação.

## Security

- Dados do cliente são não confiáveis.
- Opcode reconhecido pelo router moderno deve validar sessão/player.
- Pacote desconhecido não deve ser promovido automaticamente a pacote válido.
- Dados canonical devem ser validados antes de compilar/deployar.

## Performance

- Runtime deve preferir packs compilados a parsing repetido de milhares de ficheiros textuais.
- O formato canonical prioriza authoring e auditabilidade; o pack prioriza startup/runtime.
