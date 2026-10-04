# Audit — model configs de monsters/NPCs

## Resultado

As definições canônicas analisadas referenciam 364 model configs únicos a partir de 557 consumidores de monster/NPC definitions.

Inventário contra as árvores fornecidas:

- 356 presentes no servidor;
- 358 presentes no cliente;
- 8 ausentes no servidor;
- 6 ausentes no cliente;
- 5 ausentes nos dois.

Lista detalhada: `reports/model_config_inventory.json` e `reports/server_char_model_gaps.json`.

## Interpretação

Uma ausência na árvore não significa necessariamente que o conteúdo ativo quebra hoje: pode ser definition não utilizada, path/case legado diferente, conteúdo condicional ou asset realmente ausente. O PT Asset Tool deve mostrar isso como dependency warning/error conforme reachability real.

## Migração

`sync_model_config_catalog.py` cria/atualiza canonical `model_configs`. Quando executado na máquina com bytes reais, deve usar o parser INX existente do PT Asset Tool para materializar motions/link/job/skill metadata.

Não duplicar parser INX em `tools/content_port`.
