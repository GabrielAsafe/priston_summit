# Summit Tale Asset Tool V0.3.5 — Physique Subset Binding

## Por que esta versão existe

O scan V0.3.4 do `char/tmABCD/CmfbB02.smd` terminou corretamente, mas mostrou que
`pt_character_structural_gate_v2.py` é um gate de compatibilidade, não um detector
de identidade: 303 SMBs passaram e eles representaram 225 skeletons completos
diferentes.

Para uma peça modular rigid-Physique isso não implica necessariamente 225 roots
válidos. A peça só pode ser deformada pelos bones aos quais seus vértices têm peso
e pelos ancestrais necessários para compor esses bones em world space. Ramos do
skeleton que a peça nunca usa são irrelevantes para o binding estático.

## Nova regra V0.3.5

`bindings --discover` agora, depois do scan estrutural completo:

1. reaproveita `probe_state_v4.json` quando disponível; não repete os 958 gates;
2. escolhe um SMB que já passou apenas como *probe* de importação;
3. importa a SMD uma vez com o importer rigid existente;
4. `extract_physique_usage.py` lê os vertex groups que realmente possuem peso;
5. os nomes desses groups são mapeados de volta aos índices Physique do SMB probe;
6. para cada SMB estruturalmente compatível é calculada uma assinatura somente de:
   - bones usados pela mesh;
   - todos os ancestrais desses bones;
   - nomes, parent relationship e bind matrices dessa closure;
7. diferenças em branches fora dessa closure são ignoradas.

Nenhuma semelhança de nome/diretório participa da decisão. O diretório do asset é
usado somente para escolher deterministicamente qual candidato já aprovado será
o probe inicial.

## Resultados possíveis

- `PHYSIQUE_SUBSET_EQUIVALENT`
  - todos os structural PASSes são equivalentes na closure que deforma a mesh;
  - o asset passa a ter `ROOT_SKELETON_BOUND` para structural/Blender geometry;
  - `animation_binding_ready=false`: a motion do personagem ainda depende do
    contexto de assembly e não é inventada.
- `AMBIGUOUS_PHYSIQUE_SUBSET`
  - existem duas ou mais closures realmente diferentes; continua fail-closed.
- `PHYSIQUE_USAGE_UNMAPPED`
  - o importer produziu weighted groups que não puderam ser mapeados ao SMB probe;
  - continua fail-closed.
- `PHYSIQUE_SUBSET_INCOMPLETE`
  - algum structural PASS não pôde ser reavaliado no subset; continua fail-closed.

Detalhes completos dos grupos ficam em
`pt_build/pt_asset_tool/<ASSET>/root_skeleton_discovery/physique_subset_v1/physique_subset_groups.json`
para evitar imprimir centenas de skeletons no terminal.

## Separação skeleton x motion

A V0.3.5 formaliza duas capacidades:

- `rigid_binding_ready`: existe skeleton suficiente e comprovado para structural e
  Blender geometry;
- `animation_binding_ready`: além do skeleton existe um SMB/INX de motion
  inequivocamente ligado ao asset/contexto.

Um config-only pode portanto ficar `ROOT_SKELETON_BOUND` e
`rigid_binding_ready=true` sem que `asset motions` ou `validate animation` sejam
liberados. Isso evita usar uma motion arbitrária de monster/NPC em peça modular.

## Comando principal

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py bindings --discover char/tmABCD/CmfbB02.smd
```

Se retornar `PHYSIQUE_SUBSET_EQUIVALENT`, rode:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py asset inspect char/tmABCD/CmfbB02.smd --physical
```

Esperado:

- `binding_mode = ROOT_SKELETON_BOUND`
- `binding_source = PHYSIQUE_SUBSET_EQUIVALENT`
- `rigid_binding_ready = true`
- `animation_binding_ready = false`

Depois já é seguro testar `validate structural` e `validate visual` para a peça.
