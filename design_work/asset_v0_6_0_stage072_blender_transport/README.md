# Summit Tale Asset Tool V0.6.0 — Stage0.72 Blender Transport

Base: V0.5.9.

## Objetivo

Primeiro importer Blender automático para `SMD Stage data Ver 0.72`, agora que o corpus físico 63/63 está fechado.

O gate V0.6.0 não depende de julgamento humano. Ele prova transporte persistido de:

- geometria Stage0.72 em escala fixa `fONE=256`;
- material id por face;
- cadeia TexLink de profundidade 0/1/2/3;
- UVs independentes para stages 0/1/2;
- vertex colors `sDef_Color[4]`;
- dados de `smLIGHT3D` como metadata exata;
- nomes de textura, `TextureStageState` e `TextureFormState` por material/stage.

O `.blend` salvo é reaberto em um segundo processo Blender e comparado novamente contra o SMD original. Assim, o gate não certifica apenas objetos em memória: certifica o transporte persistido no arquivo Blender.

## Limites deliberados

- `FS_SCROLL*` é preview de frame 0. A velocidade/movimento do runtime continua não certificada.
- `smLIGHT3D` é transportado de forma exata como metadata/empties. A iluminação do viewport Blender **não** é declarada equivalente à iluminação do runtime.
- Combiner de textura no frame 0 implementa os estados observados nos representantes físicos: serialized `0` => `D3DTOP_MODULATE`, e `7` => `D3DTOP_ADD`. Outros estados permanecem diagnosticáveis/fail-closed para certificação exata.
- `StageArea` e qualquer trailing opaque continuam preservados apenas no SMD físico; esta versão ainda não escreve Stage0.72.

## Instalação

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_6_0_stage072_blender_transport.zip
./asset_v0_6_0_stage072_blender_transport/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py doctor
```

## Gate de universo de stage/form state

A V0.6.0 acrescenta agregados `stage_state_usage` e `form_state_usage` ao audit físico. Rode uma vez:

```bash
python3 pt_asset.py audit stage072-corpus \
  --output pt_build/stage072_corpus_audit_v060.json \
  --bundle PT_STAGE072_REPS_V060.zip
```

## Gates Blender automáticos

```bash
# pequeno, vertex color + multilayer
python3 pt_asset.py validate stage072-preview \
  Field/Room/office.smd

# lights + vertex colors
python3 pt_asset.py validate stage072-preview \
  Field/Quest/quest_IV.smd

# UV stage 1 realmente distinto
python3 pt_asset.py validate stage072-preview \
  Field/Fall_Game/fall_game.smd

# prova obrigatória de 3 texture stages
python3 pt_asset.py validate stage072-preview \
  Field/endless/dun-7.smd
```

Resultado esperado em cada um:

```text
PERSISTED_TRANSPORT=PASS
position_max_error=0
uv_max_error=0
color_max_error=0
```

Pequenos erros de ponto flutuante abaixo da tolerância também são aceitos e reportados numericamente.

## Segurança

- cliente original nunca é escrito;
- texturas são preparadas em overlay temporário;
- certificações permanecem append-only;
- falha de textura requerida aborta o gate;
- falha de geometria/UV/color/light/material metadata aborta o gate;
- nenhuma iluminação Blender é usada como prova de runtime.
