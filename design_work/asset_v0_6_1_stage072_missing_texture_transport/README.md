# Summit Tale Asset Tool V0.6.1 — Stage0.72 Missing-Texture Transport Policy

Base: V0.6.0.

## Motivo

O gate V0.6.0 isolou um único bloqueio em `Field/Quest/quest_IV.smd`: a textura
serializada `field/quest/fw.bmp` não existe fisicamente no cliente. O próprio
SMD usa essa referência no material 6 (stage 0), e esse material é usado por 67
faces; portanto não é seguro substituí-la por outra textura por inferência de
nome.

O SourceCode carrega os nomes serializados em `smTEXTUREHANDLE`; a criação da
textura pode retornar erro sem abortar o carregamento do grupo de materiais.
O renderer então pode receber um `lpD3DTexture` nulo. Isso prova que uma imagem
faltante no cliente deve ser tratada separadamente de corrupção estrutural do
Stage0.72. A aparência exata do material faltante continua **não certificada**.

## Mudança de política

`validate stage072-preview` agora separa dois casos:

1. **MISSING** — o arquivo referenciado pelo SMD realmente não existe no cliente.
   O nome lógico é preservado exatamente, o Blender transporta geometria/UV/
   material metadata/vertex colors/lights normalmente, e o resultado é marcado:

   - `stage072_blender_transport=PASS`
   - `texture_prepare=PARTIAL_CLIENT_MISSING`
   - `texture_reference_presence=INCOMPLETE_CLIENT`
   - `material_preview_complete=false`

   Nenhuma textura substituta é criada.

2. **Falha de preparação de um arquivo que existe** (`BMP_PROTECTED_DECODE_FAIL`,
   `TGA_DECODE_FAIL`, `LOAD_ERROR`, etc.) — continua **fail-closed**.

Esta exceção é aplicada somente ao gate automático de transporte Stage0.72. Os
gates visuais Model0.62 não foram afrouxados.

## Novo audit do cliente

A V0.6.1 acrescenta:

```bash
python3 pt_asset.py audit stage072-textures \
  --output pt_build/stage072_texture_presence_audit_v061.json \
  --bundle PT_STAGE072_MISSING_TEXTURE_REPS_V061.zip
```

O audit percorre os 63 Stage0.72 físicos e verifica a existência de todas as
texturas serializadas pertencentes a materiais realmente usados por faces.
Materiais históricos sem faces são ignorados. Collisions de caixa ou erros de
resolver continuam sendo falha estrutural do audit; somente ausência física é
classificada como missing reference.

## Instalação

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_6_1_stage072_missing_texture_transport.zip
./asset_v0_6_1_stage072_missing_texture_transport/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py doctor
```

## Gates

Primeiro faça o audit de presença:

```bash
python3 pt_asset.py audit stage072-textures \
  --output pt_build/stage072_texture_presence_audit_v061.json \
  --bundle PT_STAGE072_MISSING_TEXTURE_REPS_V061.zip
```

Depois repita o mapa que antes abortava:

```bash
python3 pt_asset.py validate stage072-preview \
  Field/Quest/quest_IV.smd
```

Para o caso já conhecido, é esperado que o transporte persista e que a saída
registre explicitamente:

```text
PERSISTED_TRANSPORT=PASS
LIGHTS metadata=42
TEXTURES presence=INCOMPLETE_CLIENT missing_client=1 material_preview_complete=false
CLIENT_MISSING_TEXTURES=field/quest/fw.bmp
```

A ausência de `fw.bmp` não deve ser convertida em PASS visual. Ela permanece
como condição do cliente, registrada no summary/certificação automática.

## Segurança

- nunca escreve no cliente;
- nunca inventa/substitui textura ausente;
- nomes de textura serializados continuam preservados como metadata exata;
- decode/load failure de arquivo existente continua FAIL;
- certificações continuam append-only;
- iluminação Blender continua não sendo usada como prova de runtime;
- `FS_SCROLL*` continua sendo apenas frame 0 no preview.
