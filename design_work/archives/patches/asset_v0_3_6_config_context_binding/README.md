# Summit Tale Asset Tool V0.3.6 — Config Context Binding

## Motivo da correção

A V0.3.5 conseguiu extrair os grupos realmente ponderados de `CmfbB02.smd`, mas o
resultado real mostrou que comparar os mesmos índices numéricos entre SMBs
independentes é semanticamente inválido. O índice 46/50/52 pode apontar para bones
completamente diferentes em skeletons diferentes, ou nem existir.

Portanto V0.3.6 não usa mais `Physique index == candidate bone index` como evidência
de identidade do root.

## Regra V0.3.6

Quando o gate estrutural deixa vários skeletons possíveis, `bindings --discover`
passa a auditar o **contexto explícito do INX**:

1. reaproveita `probe_state_v5.json` e os 958 structural gates já executados;
2. lê os bytes do(s) INX associados ao asset;
3. extrai referências literais `.smb`, `.smd` e `.inx` em ASCII e UTF-16LE;
4. resolve uma referência somente por:
   - path canônico exato;
   - path relativo ao próprio INX;
   - basename globalmente único no inventário;
5. se o INX referencia um SMD root explícito, somente o binding já comprovado desse
   SMD (ou suas linhas de pairing exatas) pode fornecer SMB candidato;
6. cada SMB explicitamente referenciado ainda precisa já ser `STRUCTURAL PASS` e
   ter skeleton signature válida;
7. somente uma única skeleton signature entre as referências explícitas pode gerar
   binding.

Não entram na decisão automática:

- similaridade de filename;
- estar no mesmo diretório;
- prefixos como `m1`, `m5`, `Mfb...`;
- nomes de bone inferidos a partir de um probe arbitrário;
- índices Physique comparados entre skeletons diferentes.

## Binding skeleton-only

`CONFIG_EXPLICIT_SKELETON_UNIQUE` promove o asset para:

- `binding_mode=ROOT_SKELETON_BOUND`
- `binding_source=CONFIG_EXPLICIT_REF_UNIQUE`
- `rigid_binding_ready=true`
- `animation_binding_ready=false`

A motion continua bloqueada até existir contexto de assembly/motion independente.

## Diagnóstico Physique

A informação da V0.3.5 não é descartada. Se
`physique_subset_v1/physique_usage.json` já existir, V0.3.6 grava:

`config_context_v1/candidate_local_physique_index_map.json`

Esse relatório mostra como os índices de vertex group do probe são interpretados
por cada SMB do mesmo diretório. Ele é marcado explicitamente como
`DIAGNOSTIC_ONLY_CANDIDATE_LOCAL` e nunca decide o binding.

## Resultados possíveis

- `CONFIG_EXPLICIT_SKELETON_UNIQUE`
  - existe uma única identidade de skeleton comprovada por referência literal do
    config;
  - pode seguir para `asset inspect`, structural e Blender geometry.
- `AMBIGUOUS_CONFIG_EXPLICIT_SKELETON`
  - o próprio config referencia skeletons realmente diferentes;
  - continua fail-closed.
- `CONFIG_REFERENCE_EVIDENCE_INCOMPLETE`
  - existe referência explícita, mas algum SMB referido não é um structural PASS
    assinado;
  - continua fail-closed.
- `CONFIG_CONTEXT_UNRESOLVED`
  - o INX não contém referência literal suficiente para decidir;
  - o próximo passo passa a ser reconstruir o assembly graph, sem heurística de
    nome.

## Comando

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py bindings --discover char/tmABCD/CmfbB02.smd
```

Se retornar `CONFIG_EXPLICIT_SKELETON_UNIQUE`:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py asset inspect char/tmABCD/CmfbB02.smd --physical
```

Depois:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate structural char/tmABCD/CmfbB02.smd
```

E, se estrutural passar:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate visual char/tmABCD/CmfbB02.smd
```
