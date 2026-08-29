# Summit Tale Asset Tool V0.6.2 — Stage0.72 Texture Reference Classification

Base: V0.6.1.

## Motivo

O audit V0.6.1 percorreu os 63 `SMD Stage data Ver 0.72` e encontrou 45
referências de textura que não resolvem literalmente no cliente, em 16 mapas.
A inspeção dos SMDs físicos mostrou que esse conjunto não é homogêneo e não
deve ser tratado como 45 arquivos simplesmente ausentes.

No bundle físico V061 há quatro famílias:

- 32 referências com aparência normal de arquivo: `CLIENT_FILE_MISSING`;
- 8 referências de animação `field/Sod/f-flame_0..7.tga ` com um espaço ASCII
  final no nome serializado;
- 3 referências terminadas no byte `0xCC` (`Ì` em Latin-1), padrão de debug-fill
  do MSVC, portanto não são nomes de arquivo confiáveis;
- 2 referências cujo nome serializado termina no diretório (`field/castle/` e
  `field/Sod/`) sem basename.

Os oito `f-flame_*` também aparecem no corpus sem o espaço final. O alias V0.6.2
é propositalmente estreito: ele só é aceito quando remover **somente espaços
ASCII no final do último componente** leva a um arquivo físico que o resolver
consegue provar no cliente. Nenhuma troca por basename parecido, outro diretório
ou outra extensão é permitida.

A string serializada no SMD nunca é alterada.

## Mudanças

### `audit stage072-textures`

O relatório agora usa o formato `PT_STAGE072_TEXTURE_REFERENCE_AUDIT_V2` e
separa:

- `exact_present_unique_texture_references`
- `exact_missing_unique_texture_references`
- `runtime_alias_present_unique_texture_references`
- `serialized_invalid_unique_texture_references`
- `client_missing_unique_texture_references`
- `unclassified_unique_texture_references`

`texture_reference_classification_complete_candidate=true` significa que cada
referência não exata foi classificada sem adivinhar semântica. Isso é diferente
de `texture_presence_complete_candidate`, que continua `false` enquanto houver
arquivo realmente ausente ou nome serializado inválido.

### Preview Stage0.72

Para um trailing-space alias fisicamente provado, o overlay cria a entrada com o
**nome serializado exato** (incluindo o espaço) apontando para o arquivo físico
sem o espaço. Assim o `.blend` continua transportando a metadata original e o
preview não precisa inventar uma textura.

O summary distingue:

- `client_missing_texture_references`
- `serialized_invalid_texture_references`
- `runtime_texture_aliases`

Um nome inválido ou arquivo ausente deixa `material_preview_complete=false`, mas
não invalida geometria/UV/colors/lights. Decode/load quebrado de um arquivo que
existe continua fail-closed.

## SourceCode / semântica

`smMATERIAL_GROUP::LoadFile` preserva os strings serializados e registra os
handles. `ReadTextures` chama `new_smCreateTexture` posteriormente; a falha de
criação não aborta o parse do grupo de materiais. A V0.6.2 continua separando
essa condição de um erro estrutural Stage0.72.

O byte `0xCC` é preservado como evidência física. A ferramenta **não** tenta
transformá-lo em um filename.

## Instalação

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_6_2_stage072_texture_reference_classification.zip
./asset_v0_6_2_stage072_texture_reference_classification/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py doctor
```

## Gates

```bash
python3 pt_asset.py audit stage072-textures \
  --output pt_build/stage072_texture_reference_audit_v062.json \
  --bundle PT_STAGE072_TEXTURE_REFERENCE_REPS_V062.zip
```

No mesmo cliente auditado pela V0.6.1, é esperado:

```text
exact_missing=45
aliases=8
invalid=5
client_missing=32
classification_complete=true
```

Repita também o caso `quest_IV`:

```bash
python3 pt_asset.py validate stage072-preview \
  Field/Quest/quest_IV.smd
```

Ele deve manter o transporte `PASS`, mas registrar `field/quest/fw.bmp` como
`CLIENT_FILE_MISSING` e deixar o preview material incompleto.

Gate específico do alias/invalid historical name:

```bash
python3 pt_asset.py validate stage072-preview \
  Field/Sod/sod-2.smd
```

É esperado que os oito `f-flame_*.tga ` sejam resolvidos por aliases físicos e
que `field/Sod/` permaneça como referência serializada inválida, sem substituição.

## Segurança

- cliente somente leitura;
- SMD original nunca é reescrito por este bloco;
- alias somente por espaço ASCII final + arquivo físico comprovado;
- não há busca por basename nem substituição artística;
- nomes `0xCC` e directory-only permanecem evidência, não são “consertados”;
- arquivo existente com falha de decode/load continua FAIL;
- certificações continuam append-only.
