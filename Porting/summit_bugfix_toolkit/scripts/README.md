# Scripts rápidos

Todos os scripts aceitam `--help` apenas se indicado no código; para ver uso, execute sem argumentos.

```text
sgrep.sh          regex no source, com filtro de extensões
context.sh        grep com -B/-A em um arquivo conhecido
find_refs.sh      referência literal no source (-F)
fasset.sh         nome de asset/arquivo
fdirs.sh          diretórios candidatos
compare_assets.sh comparação de dois diretórios de assets
probe_asset.sh    file/stat/sha256/identify
git_origin.sh     origem histórica de uma string
new_bugfix.sh     cria esqueleto de bugfix
```

Exemplo de fluxo:

```bash
./scripts/sgrep.sh 'StaminaReduce|STAMINA_REDUCE_P' SrcGame/src
./scripts/find_refs.sh 'UpKeepItemTGAImage[12]' SrcGame/src
./scripts/context.sh 'S_Reduce|M_Reduce' SrcGame/src/sinbaram/HaPremiumItem.cpp 5 12
./scripts/fasset.sh S_Reduce '/caminho/do/cliente'
./scripts/probe_asset.sh '/caminho/S_Reduce.dds'
```
