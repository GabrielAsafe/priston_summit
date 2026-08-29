# Summit Tale Asset Tool V0.3.7 — Explicit Assembly Graph Binding

## Por que esta versão existe

A V0.3.6 provou no cliente real que `char/tmABCD/Cmb002.inx` não contém
referências literais `.smd/.smb/.inx` capazes de decidir o skeleton de
`CmfbB02.smd`.

A V0.3.7 sobe um nível: reconstrói um grafo usando somente relações já existentes
no `inx_physical_pairing.csv`.

## Grafo permitido para binding

As únicas arestas que decidem binding são:

- `INX -> SMD` quando a linha de pairing possui `model_resolved=true`;
- `SMD -> INX` quando o **mesmo path canônico exato de SMD** aparece em outra
  linha do pairing.

O resolver parte de todos os INX que referenciam o asset alvo, percorre o
componente conectado e coleta somente `motion_smb` presentes em linhas daquele
componente.

Não são arestas de binding:

- mesmo diretório;
- basename parecido;
- prefixos M1/M2/Mfb/Cmb;
- similaridade de skeleton;
- índice Physique entre SMBs diferentes;
- overlap de nomes de objetos High/Default/Low.

Overlap de slots LOD é gravado apenas como `DIAGNOSTIC_ONLY_NOT_BINDING`.

## Regra fail-closed

Cada `motion_smb` encontrado no componente precisa já existir no scan estrutural
anterior como `PASS` e ter `skeleton_signature` válida.

Resultados:

- `ASSEMBLY_COMPONENT_SKELETON_UNIQUE`
  - todo o componente explícito converge para uma única assinatura;
  - promove apenas o binding estático do skeleton;
  - `rigid_binding_ready=true`;
  - `animation_binding_ready=false`;
  - `motions=[]`.
- `AMBIGUOUS_ASSEMBLY_COMPONENT_SKELETON`
  - o componente alcança mais de uma assinatura;
  - permanece bloqueado.
- `ASSEMBLY_COMPONENT_EVIDENCE_INCOMPLETE`
  - há motion explícita no componente que não possui PASS/assinatura comprovada;
  - permanece bloqueado.
- `ASSEMBLY_COMPONENT_UNRESOLVED`
  - o componente não alcança nenhuma motion/skeleton;
  - permanece bloqueado.

## Correção de segurança adicional

Durante o teste completo foi encontrada uma herança parcial antiga: um asset com
várias linhas de config podia ter uma linha temporariamente resolvida por
`INX_SHARED_ROOT` e, depois, receber um binding skeleton-only pelo cache. Isso
podia deixar uma motion residual e promover `animation_binding_ready=true`.

V0.3.7 limpa `inherited_by_row` sempre que o binding veio de discovery
skeleton-only (`PHYSIQUE_SUBSET`, `CONFIG_EXPLICIT` ou `ASSEMBLY_GRAPH`).
Assim o discovery nunca libera animação por acidente.

## Reuso do scan anterior

Se existir `probe_state_v6.json` completo e compatível com o SHA da SMD, V0.3.7
migra os resultados para `probe_state_v7.json`. Os 958 structural gates não são
repetidos.

Cache novo:

`logs/asset_family_refine_v2/root_skeleton_bindings_v7.json`

Relatório novo:

`pt_build/pt_asset_tool/<ASSET_ID>/root_skeleton_discovery/assembly_graph_v1/assembly_graph_audit.json`

## Comando principal

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py bindings --discover char/tmABCD/CmfbB02.smd
```

Se retornar `ASSEMBLY_COMPONENT_SKELETON_UNIQUE`:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py asset inspect char/tmABCD/CmfbB02.smd --physical
```

Depois, somente se `rigid_binding_ready=true`:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate structural char/tmABCD/CmfbB02.smd
```

E então:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate visual char/tmABCD/CmfbB02.smd
```
