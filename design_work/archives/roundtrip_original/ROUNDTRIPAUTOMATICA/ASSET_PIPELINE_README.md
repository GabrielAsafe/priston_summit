# Summit Tale Asset Catalog / Blender Round-Trip

## Arquivos
- `Summit_Tale_Asset_Catalog.xlsx`: catálogo humano, filtros, classes, regras, source refs e unresolved queue.
- `summit_asset_manifest.json`: manifesto machine-readable usado pelos scripts.
- `summit_scan_physical_assets.py`: inventaria TODOS os assets físicos da instalação do jogo e cruza com o manifesto da source.
- `summit_prepare_blender_workspace.py`: copia um subconjunto para um workspace Blender, sem alterar a instalação original.
- `summit_roundtrip_restore.py`: devolve arquivos editados ao caminho original; dry-run por padrão e backup antes de sobrescrever.

## 1. Inventariar a instalação real
```bash
python3 summit_scan_physical_assets.py /caminho/do/cliente --hash
```
Isso gera `Summit_Physical_Asset_Inventory.csv` e `.json` no diretório atual.

## 2. Preparar um vertical slice Pikeman
```bash
python3 summit_prepare_blender_workspace.py /caminho/do/cliente ./blender_workspace \
  --class-code C --category Characters --include-low --hash
```
O workspace recebe `workspace_manifest.json`. Esse arquivo é a chave do round-trip.

## 3. Outros exemplos
Knight:
```bash
python3 summit_prepare_blender_workspace.py /caminho/do/cliente ./knight_workspace --class-code mA --category Characters
```

Todos os stages já reconhecidos pela source:
```bash
python3 summit_prepare_blender_workspace.py /caminho/do/cliente ./stage_workspace --category Stages/Maps
```

## 4. Devolver arquivos editados
Primeiro veja o plano sem escrever nada:
```bash
python3 summit_roundtrip_restore.py ./blender_workspace
```
Depois, quando estiver certo:
```bash
python3 summit_roundtrip_restore.py ./blender_workspace --apply
```
Arquivos existentes são copiados para uma pasta de backup antes da sobrescrita.

## Class map atual
- A: Mechanician — provado pela source
- B: Fighter — provado pela source
- C: Pikeman — provado pela source
- D: Archer — provado pela source
- E: Assassin — convenção presente em código legado/comentado; confirmar na árvore física
- mA: Knight — provado pela source
- mB: Atalanta — provado pela source
- mC: Priestess — provado pela source
- mD: Magician — provado pela source
- mE: Shaman — job existe, mas o prefixo de asset ainda NÃO foi provado; não automatizar round-trip até localizar assets reais

## Regra fundamental
O caminho do workspace Blender nunca é usado para decidir o caminho de retorno. O retorno é sempre determinado por `asset_id` / `roundtrip_target` gravados no manifesto.
