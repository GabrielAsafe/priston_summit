# Asset Migration

## Estratégia

### Fase 1 — inventário

Identificar loaders, writers, paths físicos e dependências.

### Fase 2 — import

Converter legacy para canonical de forma determinística.

### Fase 3 — validação

Comparar contagens, IDs, coordenadas, referências e semântica relevante.

### Fase 4 — compilation

Gerar pack runtime e inspecioná-lo independentemente.

### Fase 5 — runtime switch

Substituir leitura legacy pelo content source/pack.

### Fase 6 — quarantine

Mover ficheiros substituídos para fora do runtime, com manifest, hash e restore.

### Fase 7 — cleanup

Quando runtime tests passarem, remover dependências legacy e manter importers apenas quando houver valor de recovery.

## Rollback

Quarantine deve ser reversível por manifest. Nunca restaurar ficheiros individualmente antes de identificar a dependência que falhou.
