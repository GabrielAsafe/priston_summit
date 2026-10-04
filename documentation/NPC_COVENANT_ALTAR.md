# NPC placement e Covenant Altars

## Separação correta

Há duas camadas:

1. definição do NPC: modelo, nome, EventCode, serviços;
2. placement: `npc_id`, slot, posição, rotação e state no `.spc` canônico.

Uma definição de altar pode ser reutilizada em vários mapas.

## Implementação incluída

Foi criado `npc.covenant_altar` no conteúdo canônico com EventCode `12050`.

O modelo visual atual é deliberadamente temporário/reutilizado de `Teacher01`; a lógica do altar fica independente da arte. Trocar o `model_file/model_config_id` depois não exige mudar a interação.

`patches/covenant_altar/covenant_altar.patch` faz:

- define `COVENANT_ALTAR_EVENT_CODE = 12050`;
- adiciona `COVENANT_UI_RESULT_OPEN_REQUEST` sem alterar layout de packet;
- roteia o clique pelo `LegacyInteractionAdapter`;
- usa `SendCovenantUIStateLegacy` já existente;
- expõe `OpenCovenantsTab()` no client;
- abre a aba Covenant quando o state packet foi enviado pelo altar.

## Placement e coordenadas

O canonical atual de fixed NPC guarda X/Y/Z na unidade interna fixed-point (`world * 256`). O debug overlay usa a unidade humana (`pX >> 8`, `pZ >> 8`).

O novo workflow do PT Asset Tool deve sempre mostrar/editar **world coordinates** e converter no compiler/materializer.

Foi criado `Content/canonical/covenant_altars/placement_plan.json` para os 53 mapas. Cada row possui candidato X/Z derivado do primeiro start point, mas `y=null` e `validated=false`.

Não é seguro colocar todos os altares automaticamente no start point sem ground validation.

Passos:

1. ir ao mapa;
2. posicionar o GM onde o altar deve ficar;
3. copiar X/Y/Z do overlay;
4. preencher o row e marcar `validated=true`;
5. aplicar:

```bash
python tools/generate_covenant_altar_placements.py apply \
  Content/canonical/covenant_altars/placement_plan.json \
  Content/canonical
```

A ferramenta converte world -> fixed-point ×256 e cria/atualiza o `.spc.json` apropriado.
