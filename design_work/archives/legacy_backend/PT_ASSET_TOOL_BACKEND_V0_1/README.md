# PT Asset Tool Backend V0.1

Primeira camada unificada sobre os cores já existentes em `design_work`.

## Objetivo

Não substitui os parsers/writers provados. O backend cria uma interface estável para:

- registry de assets do cliente;
- resolução de paths físicos com casing real;
- inspeção por ID/path;
- certificações persistentes;
- validação estrutural;
- validação visual assistida via Blender.

Nenhum comando desta V0.1 publica no cliente. O fluxo visual abre apenas `.blend` em `pt_build/pt_asset_tool`.

## Instalação

```bash
cd PT_ASSET_TOOL_BACKEND_V0_1
bash install_pt_asset_tool_backend_v0_1.sh ..
cd ..
```

## Primeiro smoke test

```bash
bash sh/run_pt_asset_backend_smoke_v0_1.sh
```

## Browser/registry

```bash
python3 pt_asset.py assets list --limit 30
python3 pt_asset.py assets list --category MOB_MONSTER --visual-only
python3 pt_asset.py families
```

## Inspecionar um asset

```bash
python3 pt_asset.py asset inspect 'char/monster/Monbagon/Monbagon-C.smd'
```

Para também resolver os arquivos reais do cliente:

```bash
python3 pt_asset.py asset inspect 'char/monster/Monbagon/Monbagon-C.smd' --physical
```

## Validação estrutural

```bash
python3 pt_asset.py validate structural 'char/monster/Monbagon/Monbagon-C.smd'
```

Isto não modifica o cliente.

## Validação visual assistida

```bash
python3 pt_asset.py validate visual 'char/monster/Monbagon/Monbagon-C.smd'
```

Ou pegar automaticamente o próximo asset suportado ainda sem PASS visual:

```bash
python3 pt_asset.py validate visual next
```

Fluxo:

1. roda gate estrutural;
2. importa o asset pristine em Blender background;
3. cria automaticamente um witness em alguns vértices de uma mesh visível;
4. salva `visual_geometry_witness.blend`;
5. abre o Blender normal;
6. você inspeciona e fecha;
7. terminal pergunta `y / n / skip`;
8. resultado é anexado em `certifications/assets.jsonl`.

O witness é deliberadamente local e não é publicado no cliente.

## Certificações

```bash
python3 pt_asset.py certification list --latest
python3 pt_asset.py certification list --asset 'Monbagon-C.smd'
```

## Estado desta versão

Implementado na interface:

- `SMD_MODEL_062_RIGID_PHYSIQUE` self-contained -> structural + visual Blender;
- `SMD_MODEL_062_STATIC` simples -> structural + visual Blender;
- INX `LEGACY_120` e `MODERN_172` através do core V2;
- persistência de certificações.

Ainda não ligado ao visual runner nesta V0.1:

- config-only -> class-root binding;
- mixed-Physique;
- Stage0.72;
- static object-local animation;
- multilayer TexLink;
- SMD/TGA/BMP outliers especiais;
- publish/runtime automation.

Esses entram como adapters mantendo a mesma CLI/registry.

## Segurança

V0.1 não possui comando `publish`. Nenhum gate desta versão escreve no cliente.
