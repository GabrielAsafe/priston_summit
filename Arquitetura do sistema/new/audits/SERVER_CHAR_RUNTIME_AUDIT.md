# Audit — `/Servidor/char`

## Resposta curta

**O servidor atual precisa de parte desses arquivos. Não apagar a pasta.**

A source do servidor usa o mesmo ecossistema legado de model config/model loader do cliente. `smPATTERN::LoadCharactor` chama `smModelDecode`, que tenta carregar o cache `.inx` e, a partir dele, abre os arquivos de modelo/motion (`.smd`/`.smb`). O servidor também possui caminhos explícitos para `char/tmABCD`, e monstros/NPCs do catálogo apontam para `char/monster/...` e `char/npc/...`.

Além disso, o servidor atual não é estritamente headless: em caminhos onde existe `renderDevice`, modelos podem chamar `ReadTextures()`. Por isso remover BMP/TGA hoje pode quebrar uma build/configuração server-side mesmo que gameplay não precise visualmente deles.

## Inventário recebido

A árvore fornecida contém aproximadamente:

- 396 diretórios;
- 9.403 arquivos;
- 2.523 `.inx`;
- 2.380 `.smd`;
- 965 `.smb`;
- 1.653 `.tga`;
- 1.200 `.bmp`;
- 627 `.inf`;
- 23 `.ase`;
- 8 `.ini`.

## O que é necessário hoje

### Model configs/cache

`.inx` é consumido diretamente por `smModelDecode`. Quando o `.inx` não está disponível ou não é aceito, o loader pode ler o model config textual e produzir cache.

Portanto `.inx/.ini` referenciados por conteúdo ativo são runtime dependencies hoje.

### Mesh/motion

Os model configs apontam para `.smd/.smb`; o servidor abre esses modelos para personagens/mobs/NPCs. Eles também são runtime dependencies enquanto o server continuar usando o loader legado.

### `char/tmABCD`

Não é apenas conteúdo visual do cliente. A source server possui referências explícitas a esse diretório para modelos de personagens e equipamentos. Ele não deve ser removido antes da migração do model loader server-side.

### Texturas

Não se deve declarar todas indispensáveis para gameplay, mas a build atual ainda pode executar leitura de texturas. Por isso a limpeza segura usa dois estágios:

1. `render-safe`: mantém todos os arquivos do diretório de cada model config referenciado;
2. `runtime-core/headless`: mantém somente config/model/motion depois que `ReadTextures` e dependências gráficas forem removidas do servidor.

## Relação com conteúdo canônico

Foram encontrados 557 consumidores de model config em definições canônicas de mobs/NPCs, apontando para 364 configs únicos.

Audit contra as árvores fornecidas:

- 356/364 presentes no servidor;
- 358/364 presentes no cliente;
- 8 ausentes no servidor;
- 6 ausentes no cliente;
- 5 ausentes nos dois inventários.

As 8 referências ausentes no servidor são:

1. `char/monster/g_wolverin/wolverin_a.ini` — `monster.pet_60_wolverine_sombrio`;
2. `char/monster/hongky/hongky.ini` — `monster.chefe_59_hunki_reverso`;
3. `char/monster/Nihil/Nihil1.ini` — `npc.soul`;
4. `char/npc/bt03/battleportal.INI` — `npc.sod_05`;
5. `char/npc/npc-04.ini` — `npc.ricarden_civilian3`;
6. `char/npc/npc-09.ini` — `npc.ricarden_civilian2`;
7. `char/npc/npc-10.ini` — `npc.navisko_warehouse`;
8. `char/npcXms/ChTree_Longbo.ini` — `npc.xmasguy`.

Ver detalhes em `reports/model_config_inventory.json`.

## Nova regra recomendada

Não distribuir `Servidor/char` como cópia integral do cliente por hábito. Gerar o conteúdo server-side por **dependency manifest**.

Use:

```bash
python tools/build_server_char_manifest.py \
  --canonical Content/canonical \
  --server-char /caminho/Servidor/char \
  --pt-asset-tool-root /caminho/pt_asset_tool-main \
  --mode render-safe \
  --output server_char_manifest.json
```

Depois da migração headless do model loader, trocar para `--mode runtime-core`.

## Estado final desejado

O servidor deve conhecer:

- bounding/geometry necessária a colisão/combat;
- motion metadata necessária a estados/frames;
- model identity necessária a protocolo/spawn.

Ele não deveria depender de textura, material, preview ou outro asset puramente visual. Essa remoção exige patch do loader, não apenas apagar arquivos.
