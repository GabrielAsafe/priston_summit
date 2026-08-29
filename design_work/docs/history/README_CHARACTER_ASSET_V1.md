# PT Character Asset V1

Primeira camada que trata um personagem como unidade operacional.

## Ideia

Antes:

```text
artista precisa saber:
MfbC01.smd
m5.smb
M5Bip.inx
MshC01 / MbhC01 / ...
char/tmABCD/MfbC01.smd
```

Agora:

```text
profile = Pikeman C01 / M5
        ↓
Character Asset
        ↓
modelo + skeleton + INX + LODs + texturas + registry
```

## Instalação

```bash
cd ~/Área\ de\ Trabalho/summit/design_work/PT_CHARACTER_ASSET_V1
bash install_character_asset_v1.sh ..
```

## 1. Inspect

```bash
cd ~/Área\ de\ Trabalho/summit/design_work

python3 pt_character_asset_v1.py inspect \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --workspace ./blender_workspace_pikeman
```

Gera:

```text
logs/characters/pikeman_c01_m5/character_asset_manifest.json
```

O manifest lista:
- model SMD;
- skeleton SMB;
- model-info INX;
- texturas;
- hashes/tamanhos;
- runtime targets;
- Asset_ID;
- estado no workspace.

## 2. Sincronizar o conjunto no registry

Dry/inspect primeiro. Depois:

```bash
python3 pt_character_asset_v1.py sync-workspace \
  --client-root "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  --workspace ./blender_workspace_pikeman
```

Assets já registrados são ignorados. Novos assets são registrados usando o Bridge seguro.

Nenhum arquivo do cliente é alterado.

## 3. Build do personagem

Produção:

```bash
python3 pt_character_asset_v1.py build \
  --artist blender/SEU_PERSONAGEM.blend \
  --name pikeman_c01_build
```

O Character Asset escolhe automaticamente:
- profile;
- SMD;
- SMB;
- INX;
- 3 LODs;
- 9 objetos;
- roundtrip target.

Se houver skinning ambíguo, o pipeline para em `REVIEW_REQUIRED`.

Teste de engenharia apenas:

```bash
python3 pt_character_asset_v1.py build \
  --artist blender/MfbC01_artist_free_mesh_v1.blend \
  --name pikeman_c01_test \
  --force-test
```

## 4. Build + publish

```bash
python3 pt_character_asset_v1.py build \
  --artist blender/SEU_PERSONAGEM.blend \
  --name pikeman_c01_build \
  --workspace ./blender_workspace_pikeman \
  --publish \
  --replace-published
```

Ainda não instala no cliente.

## 5. Deploy plan

```bash
python3 pt_character_asset_v1.py deploy-plan \
  --workspace ./blender_workspace_pikeman
```

## 6. Deploy

Somente depois de revisar o plano:

```bash
python3 pt_character_asset_v1.py deploy \
  --workspace ./blender_workspace_pikeman
```

O target `char/tmABCD/MfbC01.smd` vem do profile; o artista não precisa digitá-lo.

## Limite atual

`Character Asset V1` já agrega e resolve as texturas, mas ainda não as exporta/reescreve.

Essa é a próxima feature:

```text
Material + Texture workflow
```
