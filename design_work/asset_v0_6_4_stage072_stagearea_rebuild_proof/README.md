# Summit Tale Asset Tool V0.6.4 — Stage0.72 StageArea Rebuild Proof

Esta versão fecha a pré-condição física para escrita geométrica de `SMD Stage data Ver 0.72`.

## O que foi provado

O SourceCode fornecido implementa `smSTAGE3D::getPolyAreas()` / `SetupPolyAreas()` usando:

- coordenadas fixas `Vertex.x/z >> FLOATNS` (`FLOATNS=8`);
- grade de mapa de 64 unidades (`>> 6`);
- todas as células do bounding-box X/Z do triângulo, inclusivas;
- wrap de célula com `& 0xFF`;
- faces inseridas em ordem crescente do índice da face;
- células serializadas em ordem `y` externa / `x` interna.

A auditoria física local contra os 63 Stage0.72 fornecidos reconstruiu **byte por byte**:

- o conjunto de células e a lista ordenada de faces de cada célula: 63/63;
- `StageMapRect = min/max X/Z` dos vértices: 63/63;
- o array serializado `StageArea[256][256]`: 63/63;
- `wAreaSize`: 63/63;
- o payload StageArea consumido pelo runtime: 63/63.

### Bug histórico de capacidade

No SourceCode atual `StageArea[x][y]` é `DWORD*`, mas o primeiro passe de `SetupPolyAreas()` executa `StageArea[x][y]++` enquanto o ponteiro está sendo usado como contador. Assim cada face soma **4**, não 1. Depois o valor do ponteiro é convertido para `int` e vira capacidade.

Os 62 arquivos físicos DWORD reproduzem exatamente:

`wAreaSize = sum(4 * face_count_da_celula + 1)`

O único `Field/Lost/lost.smd` WORD16 reproduz a variante física legada equivalente:

`wAreaSize = sum(2 * face_count_da_celula + 1)`

A V0.6.4 não converte uma família na outra.

## O que ainda NÃO é autorizado

A V0.6.4 ainda não grava deformação arbitrária de vértice no cliente/artifact writer principal. O próximo passo é usar este reconstrutor no writer geométrico e tratar corretamente dados derivados da geometria, especialmente `smSTAGE_FACE::VectNormal`.

Não há alteração de arquivos do cliente.

## Instalação

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
unzip -o asset_v0_6_4_stage072_stagearea_rebuild_proof.zip
./asset_v0_6_4_stage072_stagearea_rebuild_proof/install.sh \
  "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py --version
python3 pt_asset.py doctor
```

## Gate do corpus

```bash
python3 pt_asset.py audit stage072-stagearea \
  --output pt_build/stage072_stagearea_rebuild_audit_v064.json \
  --bundle PT_STAGE072_STAGEAREA_REPS_V064.zip
```

Resultado esperado no corpus físico conhecido:

```text
stage072_stagearea_rebuild_audit=PASS audited=63/63 rebuilt=63 \
encodings={'DWORD32': 59, 'DWORD32_TRAILING_OPAQUE': 3, 'WORD16': 1} \
capacity_models={'DWORD_POINTER_COUNTER_STEP4': 62, 'WORD16_LEGACY_POINTER_COUNTER_STEP2': 1} \
max_span=2254/4096 complete_candidate=true
```

Os três suffixes opaque continuam fora do core reconstruído e devem ser preservados byte-identicamente por qualquer writer posterior.
