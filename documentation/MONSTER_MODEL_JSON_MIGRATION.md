# Model configs de monstros/NPCs — `.ini/.inx` -> canonical JSON

## Diagnóstico

O `.inx` é cache binário de `smMODELINFO`, não uma boa fonte de autoria. A própria source consegue ler config textual e gerar `.inx`. O PT Asset Tool já possui parser robusto para os layouts observados:

- `MODERN_172`, 95.268 bytes;
- `LEGACY_120`, 67.084 bytes;
- até 512 motions;
- state/start/end/event frames;
- item applicability;
- job mask;
- skill applicability;
- map position;
- repeat/key/motion frame.

Ele também possui writer preservation-aware que não normaliza bytes desconhecidos.

## Integração feita neste bundle

Foi criado `Content/canonical/model_configs/`.

Cada mob/NPC que possuía `semantic.fields.model_file` recebeu também:

```json
"model_config_id": "model.char.monster.hopy.hopy"
```

O catálogo possui 364 model configs únicos.

Como nesta conversa só estava disponível a árvore dos assets, e não os bytes de todos os INX, os docs atualmente preservam:

- logical config;
- legacy cache esperado;
- consumers;
- presença server/client;
- source status.

Para popular o JSON sem inventar dados, rode localmente:

```bash
python tools/content_port/sync_model_config_catalog.py Content/canonical \
  --server-char "/home/gabriel/Área de Trabalho/summit/Servidor/char" \
  --client-char "/home/gabriel/Área de Trabalho/summit/cliente/char" \
  --pt-asset-tool-root /caminho/pt_asset_tool-main \
  --patch-definitions
```

Com os INX reais o script preenche também:

- layout;
- model file;
- motion file;
- submodel;
- high/default/low model groups;
- motion count;
- link file;
- talk link/motion;
- todas as motions decodificadas;
- hashes do source.

## Estratégia de runtime

### Fase 1 — JSON como autoria + INX como cache

Fluxo:

`canonical model JSON -> PT Asset Tool -> INX/cache -> runtime legado`

Essa fase é a de menor risco. O INX passa a ser build artifact.

### Fase 2 — pack próprio

Depois da equivalência comprovada, criar `models.smcfg` (ou equivalente) e um adapter `smMODELINFO` que carregue diretamente o pack. Somente então remover INX do deployment.

## O que continua binário

Não converter para JSON:

- `.smd/.smb` de mesh/motion;
- DDS/TGA/BMP de textura;
- WAV/BGM;
- geometria de mapa.

Esses são assets binários legítimos. JSON deve descrever identidade e referências, não serializar vértices/texturas.
