Sim. Montei um sistema pensado especificamente para virar a camada de **asset registry entre o cliente do jogo e o Blender**, preservando a origem de cada arquivo para permitir round-trip depois.

A primeira passagem sobre a source atual encontrou **4.851 caminhos únicos de assets/configurações**, vindos de **14.661 referências no código**. Como você ainda não me passou a árvore completa de assets do cliente, a planilha representa aquilo que a source conhece; junto dela deixei um scanner que, quando apontado para a pasta real do jogo, inventaria também arquivos que existem no cliente mas nunca aparecem diretamente na source.

A classificação de personagens ficou assim: `A = Mechanician`, `B = Fighter`, `C = Pikeman`, `D = Archer`, `mA = Knight`, `mB = Atalanta`, `mC = Priestess` e `mD = Magician` estão comprovados diretamente pela source. `E = Assassin` aparece explicitamente na convenção antiga, inclusive `e001.ini...e007.ini` e `tfh-e*.inf`, mas esse trecho está legado/comentado no código atual. `mE = Shaman` eu deixei como **hipótese, sem auto-round-trip**, porque o Job Shaman existe, mas ainda não localizamos na source caminhos concretos que provem `meNNN.ini`.

O conceito central do sistema é que cada asset recebe um identificador estável, por exemplo:

```text
Asset_ID       AST-xxxxxxxxxxxx
Origin_URI     game://char/tmABCD/c001.ini
Class_Code     C
Owner          Pikeman
Representation Equipped Body
Blender_Target Characters/Pikeman/c001.ini
Roundtrip_Target char/tmABCD/c001.ini
```

Assim o artista pode reorganizar completamente a pasta de trabalho do Blender. O exporter **não tenta adivinhar de onde o arquivo veio**: ele consulta `Asset_ID`/`Roundtrip_Target`.

Também deixei explícita a separação que você mencionou para itens:

```text
ARMADURA X
│
├── Equipped Body
│   └── modelo/textura usados pelo personagem
│
├── World Drop
│   └── representação caída no chão
│
└── Inventory Icon
    └── imagem da interface
```

Essas três coisas não são tratadas como um único asset. A mesma arquitetura está preparada para armas: `Equipped Weapon`, `Weapon Drop`, `Weapon Icon` e `Weapon Effect` serão relacionados posteriormente por item code/tabelas quando fizermos a investigação específica de armas.

Para o **Pikeman** já deixei uma aba `Pikeman_Slice`. Ela reúne os assets que já conseguimos atribuir à classe `C` e, embaixo, deixa os pontos ainda pendentes: resolver o mesh a partir dos `cNNN.ini/INX`, texturas, ícone de defesa, representação DropItem, arma equipada, ícone da arma, drop da arma e efeitos. Assim dá para usar Pikeman como primeiro vertical slice sem transformar hipóteses em fatos.

O Excel possui `README`, `Dashboard`, `Class_Map`, `Rules`, `Asset_Catalog` e `Pikeman_Slice`. O `Asset_Catalog` já contém caminho original, categoria, subcategoria, representação, personagem associado, código da classe, confiança da classificação, ação prevista no Blender, referência source+linha, destino no workspace e destino de round-trip.

Além da planilha, criei o pipeline operacional:

* [Pacote completo — Excel + manifesto + scripts](sandbox:/mnt/data/Summit_Tale_Asset_Pipeline.zip)
* [Excel — Summit Tale Asset Catalog](sandbox:/mnt/data/Summit_Tale_Asset_Catalog.xlsx)
* [Manifesto JSON para o loader](sandbox:/mnt/data/summit_asset_manifest.json)
* [Scanner da pasta física do jogo](sandbox:/mnt/data/summit_scan_physical_assets.py)
* [Preparador de workspace Blender](sandbox:/mnt/data/summit_prepare_blender_workspace.py)
* [Round-trip seguro para devolver os assets](sandbox:/mnt/data/summit_roundtrip_restore.py)
* [README do pipeline](sandbox:/mnt/data/ASSET_PIPELINE_README.md)

Quando você tiver a pasta real do cliente, o primeiro comando seria:

```bash
python3 summit_scan_physical_assets.py /caminho/do/cliente --hash
```

Ele varre **todos os `.smd`, `.smb`, `.ase`, `.inx`, `.ini`, `.inf`, texturas, áudio, scripts etc.**, cruza o que encontra com o catálogo da source e também marca arquivos físicos **não referenciados pela source**. Isso é importante para descobrir assets órfãos, conteúdo antigo e justamente os monstros/armas que ainda não conseguimos catalogar pelo código.

Para montar só o Pikeman no workspace:

```bash
python3 summit_prepare_blender_workspace.py \
    /caminho/do/cliente \
    ./blender_pikeman \
    --class-code C \
    --category Characters \
    --include-low \
    --hash
```

Ele copia os arquivos sem mexer no jogo e cria dentro da pasta:

```text
workspace_manifest.json
```

Esse manifesto registra:

```text
arquivo no workspace
→ Asset_ID
→ caminho original no cliente
→ classe
→ categoria
→ representação
```

Depois do trabalho no Blender, você consegue visualizar primeiro o que seria devolvido:

```bash
python3 summit_roundtrip_restore.py ./blender_pikeman
```

Nada é alterado nesse modo. Quando estiver correto:

```bash
python3 summit_roundtrip_restore.py ./blender_pikeman --apply
```

O script faz **backup do arquivo original antes de sobrescrevê-lo**.

O próximo passo que mais aumentaria a qualidade desse catálogo é você me fornecer uma cópia/listagem da **árvore de assets do cliente**. Aí dá para transformar aqueles milhares de `Textures`, `3D/Config`, `Weapons/Items` e `Monsters` ainda genéricos em relações concretas, incluindo algo do tipo:

```text
ITEM CODE
   │
   ├── classe(s) que podem equipar
   ├── equipped mesh
   ├── equipped texture
   ├── inventory icon
   ├── drop mesh
   ├── drop texture
   ├── effect
   └── attachment bone
```

e, para monstros:

```text
MONSTER
   ├── family
   ├── model SMD
   ├── skeleton SMB
   ├── INX
   ├── textures
   ├── motions
   ├── effects
   └── sounds
```

Essa estrutura já está preparada no manifesto; falta agora alimentar a camada física.

