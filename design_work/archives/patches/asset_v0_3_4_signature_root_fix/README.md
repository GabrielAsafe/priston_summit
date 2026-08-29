# Summit Tale Asset Tool V0.3.4 — Signature Root Fix

Corrige o `PASS_SIGNATURE_INCOMPLETE` universal observado na V0.3.3.

## Evidência do bug

No scan real de `char/tmABCD/CmfbB02.smd`:

- 958 SMBs testados pelo structural gate;
- 303 structural PASS;
- 0 skeletons assinados;
- 303/303 falharam com `TypeError: int() argument ... NoneType`.

A causa estava no extrator de assinatura: o parser legado usa `parent_index=None`
para roots em SMBs reais, enquanto a assinatura executava `int(parent_index)`.

## Correção

- `parent_index=None`/vazio de root é normalizado para `-1`;
- se um non-root não tiver índice numérico, `parent_name` recupera a hierarquia;
- `tm` aceita representação flat ou matriz aninhada;
- `tm_sort` ignora entradas nulas em vez de falhar;
- bind fallback (`tminvert`/`tmrotate`) é usado apenas como identidade de assinatura,
  nunca para pose/animação.

## Não reroda 958 gates

A V0.3.4 cria `probe_state_v4.json`. Se existir um `probe_state_v3.json`
com o mesmo SHA do SMD, importa os resultados PASS/FAIL estruturais e recalcula
somente as assinaturas dos PASSes.

Saída esperada:

    reused_v3_structural_results=958; signatures_will_be_recomputed
    resign_structural_passes=303
    resign_progress=50/303 ...
    ...

Depois o resultado pode ser:

- `UNIQUE_SKELETON`: binding automático permitido;
- `AMBIGUOUS_SKELETON_MATCH`: mais de um skeleton real continua compatível;
- `PASS_SIGNATURE_INCOMPLETE`: ainda há PASS estrutural que o parser não consegue assinar;
- `NO_MATCH`: apenas se nenhum SMB passou estruturalmente.

Tudo permanece fail-closed. Cliente, SMD, SMB e INX não são modificados.
