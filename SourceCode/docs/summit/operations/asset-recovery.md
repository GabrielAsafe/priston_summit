# Runbook — Asset Recovery

## Sintoma

Runtime deixa de funcionar após remoção/quarantine de ficheiros legacy.

## Procedimento

1. Registar path/erro exato.
2. Identificar família canonical correspondente.
3. Confirmar pack compilado.
4. Confirmar loader/binding.
5. Procurar read direto residual no source.
6. Corrigir consumer.
7. Rebuild/retest.
8. Só restaurar a família necessária se o diagnóstico exigir.

## Não fazer

- restaurar toda a árvore legacy imediatamente;
- misturar assets visuais com config;
- assumir que existência no canonical significa que todo callsite já usa pack.
