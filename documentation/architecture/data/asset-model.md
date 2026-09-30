# Asset Model

## Camadas

```text
Canonical Asset
      ↓
Validation DTO / semantic model
      ↓
Compiler
      ↓
Runtime Pack
      ↓
Runtime Definition
```

## Canonical

Características:

- legível e editável;
- versionável;
- validável;
- IDs estáveis;
- referências explícitas;
- sem dependência de offsets de structs C++ legacy.

## Runtime pack

Características:

- determinístico;
- rápido de carregar;
- versionado;
- inspecionável;
- com contrato explícito de secções;
- reproduzível a partir de canonical.

## Provenance

Sempre que possível o pipeline deve registar:

- input canonical;
- versão de schema;
- compilador;
- hash do output;
- perfil client/server;
- data/build id.
