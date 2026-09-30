# Runbook — Rollback

## Código

Preferir revert explícito do patch/commit afetado. Não usar resets amplos em worktrees com alterações intercaladas.

## Packs

Rebuild a partir do canonical anterior ou restaurar pack conhecido por hash.

## Assets em quarantine

Usar `MANIFEST.json` e ferramenta de restore. Não copiar manualmente ficheiros isolados sem diagnóstico.

## Database

Restaurar apenas a partir de backup/migration rollback compatível. Nunca “corrigir” login apagando dados persistentes sem evidência de corrupção.

## Critério

Rollback deve restaurar:

- build;
- startup;
- login;
- runtime contract;
- smoke do domínio.
