# Summit Tale Asset Tool V0.6.3 — Stage0.72 Strict UV Roundtrip

Esta versão fecha o primeiro writer seguro para `SMD Stage data Ver 0.72`.

## Escopo certificado nesta versão

O writer parte do SMD físico original, copia todos os bytes e altera somente um
`float` UV explicitamente autorizado pelo witness Blender. Depois o artefato é
reparseado e reimportado no Blender.

O gate exige byte-identidade para:

- header `smSTAGE3D`;
- grupo/materials;
- vertices/geometria;
- faces/material ids/topologia;
- `hTexture` e `NextTex` de todos os TexLinks;
- lights;
- StageArea completo;
- qualquer suffix opaque após StageArea.

Somente os quatro bytes do `u` ou `v` escolhido pelo witness podem mudar.

O witness escolhe a face renderizável de maior profundidade. Assim um mapa com
3 layers edita e prova o stage 2, não apenas os stages 0/1.

## Por que geometria ainda NÃO é escrita

Stage0.72 possui `StageArea`, um índice espacial de faces usado pelo runtime.
Mover um vertex e preservar StageArea byte-idêntico não é automaticamente seguro:
a alteração pode mudar a associação espacial necessária para aquela face.

Por isso V0.6.3 é fail-closed:

`GEOMETRY_WRITE=DEFERRED_STAGEAREA_REBUILD_NOT_PROVEN`

O transporte Blender de posições continua certificado, mas o writer recusa qualquer
mudança geométrica até a regra de reconstrução/validação de StageArea ser provada
contra o SourceCode e o corpus físico.

## Instalação

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_6_3_stage072_uv_roundtrip_strict.zip
./asset_v0_6_3_stage072_uv_roundtrip_strict/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py doctor
```

## Gates recomendados

```bash
# DWORD32 + 97 lights
python3 pt_asset.py validate stage072-roundtrip \
  Field/dungeon/dun-1.smd

# depth 3: witness deve escolher stage 2
python3 pt_asset.py validate stage072-roundtrip \
  Field/endless/dun-7.smd

# único StageArea WORD16 do corpus
python3 pt_asset.py validate stage072-roundtrip \
  Field/Lost/lost.smd

# DWORD32 + suffix opaque de 124344 bytes
python3 pt_asset.py validate stage072-roundtrip \
  cSelect/select.smd
```

Saída esperada em cada caso:

```text
ROUNDTRIP=PASS ... StageArea_suffix_exact=True hTexture_NextTex_exact=True non_witness_bytes_exact=True
REIMPORT_TRANSPORT=PASS ...
GEOMETRY_WRITE=DEFERRED reason=StageArea spatial partition must be rebuilt/proven before vertex moves are authorized
```

Nenhum arquivo do cliente é modificado.
