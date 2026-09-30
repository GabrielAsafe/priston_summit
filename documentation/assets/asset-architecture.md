# Asset Architecture

## Source-of-truth

```text
Content/canonical/
```

O canonical deve ser organizado por famílias de domínio, não por structs históricas.

## Pipeline

```text
canonical
  ↓
text/schema validation
  ↓
semantic validation
  ↓
family compiler
  ↓
Content/packs/
  ↓
inspect
  ↓
runtime contract validation
  ↓
deploy
```

## Packs conhecidos

- definitions client/server;
- maps client/server;
- localization;
- text resources;
- gameplay config;
- field runtime;
- gameplay reference quando integrado.

## Política de ficheiros legacy

Classificar cada família como:

1. runtime-required;
2. import/recovery-only;
3. redundant;
4. physical media/geometry that remains external.

Só a categoria 3 deve ser removida sem replacement.
