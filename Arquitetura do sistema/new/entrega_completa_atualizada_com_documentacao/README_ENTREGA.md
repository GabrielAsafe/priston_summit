# Entrega consolidada

Este pacote contém os patches, árvores já corrigidas, base migrada, documentação e relatórios.

## Aplicação recomendada

1. Faça backup da DB de produção.
2. Revise `coinshop_catalog_export.csv` e `coinshop_audit.json`.
3. Copie o `Content` corrigido ou aplique `content.patch`.
4. Aplique `source.patch` ao source equivalente.
5. Rode os auditores em `src/tools`.
6. Só depois aplique a migration 046 ou use a cópia de DB migrada como referência.

A migration 046 remove somente o catálogo estático `ItemList` e `ItemList_T`; saldos, pagamentos e transações continuam em DB.

As asas `QW103` e `QW106` foram removidas da CoinShop migrada e permanecem suportadas pelo fluxo legado de Wing Quest/Craft.

## Limite conhecido

A migração de **todo** texto hardcoded do C++ não foi automatizada cegamente porque os relatórios também capturam SQL, paths, formatos e identificadores. Os arquivos `hardcoded_*.json` são o backlog auditado para migração segura de textos efetivamente visíveis ao jogador.
