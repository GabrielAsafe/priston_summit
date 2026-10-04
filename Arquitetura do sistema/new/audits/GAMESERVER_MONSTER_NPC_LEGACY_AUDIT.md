# Audit — `GameServer/Monster`, NPCs legados e pacote a portar para JSON

## Estado do runtime

A build analisada já possui `definitions_server.scfg` e o pipeline canonical/compiled definitions. Os `.inf`/`.NPC` físicos devem ser tratados como legado/import/diagnóstico quando strict compiled definitions é a autoridade.

## Pacote `GameserverFilesToPortJson.zip`

O audit automático no bundle registra:

- 100 definições de origem processadas;
- 84 já idênticas ao canonical;
- 16 preservadas como variantes inativas, sem alterar o runtime ativo.

Ver `reports/gameserver_files_to_port_json_audit.json`.

## Por que variantes foram preservadas

Arquivos com o mesmo nome/identidade aproximada nem sempre possuem os mesmos valores. O caso mais sensível é `Arma Antiga e Slab/120_Acero.inf`: ele não deve substituir automaticamente `monster.acero` porque o conteúdo difere materialmente.

Regra de migração:

```text
identical -> pode ser marcado como covered
same-ish identity + different data -> legacy_variant + decisão humana de identidade
unknown identity -> novo canonical id após validação
```

Nunca fazer merge por filename sozinho.

## NPCs

Definições NPC seguem a mesma regra: definition e placement são entidades distintas. O pacote contém NPCs que já têm correspondência evidente e pelo menos uma identidade que precisa reconciliação explícita antes de ativar.

## Estado alvo

- production runtime: packs compilados;
- canonical JSON: source of truth;
- `.inf/.NPC`: import fixtures/archive;
- PT Asset Tool: editor + import/diff/reconciliation.

## Critérios de remoção do legado de production

- coverage 100% das definições ativas;
- parity test de stats/drop/services;
- boot/spawn/NPC smoke usando somente packs;
- fallback legado desligado ou falha explícita quando strict mode está ativo;
- archive preservado fora do runtime deploy.
