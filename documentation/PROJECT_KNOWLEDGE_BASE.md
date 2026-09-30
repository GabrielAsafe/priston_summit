# Summit Project Knowledge Base

**Snapshot:** 2026-09-30  
**Baseline:** source pós-REC174 + hotfix de login + Field Runtime + Quest Codex/Mix + correção Amanda/Enzo  
**Objetivo:** ser a fonte de consulta operacional para manutenção autónoma do projeto e handoff para outro chat/developer.

> Este documento distingue sempre quatro estados: **CONFIRMADO EM JOGO**, **VALIDADO ESTATICAMENTE**, **PREPARADO MAS NÃO INTEGRADO** e **PENDENTE**. Não promover um estado sem evidência.

---

# 1. Estado executivo

## 1.1 Confirmado em jogo

- Login voltou a funcionar após correção do `PacketRouter` de pré-autenticação.
- Amanda aparece em Ricarten e funciona como Instrutora.
- Enzo aparece em Pillai e funciona como Restaurador de Itens.
- A identidade Amanda/Enzo está separada em definition, modelo, texto, class e placement.

## 1.2 Validado no canonical e nos packs

Último ciclo de validação/build:

- canonical text: `5604` resources;
- NPC definitions: `175`;
- stage spawns: `54` mapas;
- stage start points: `65` mapas / `6604` pontos;
- fixed NPC maps: `24`;
- fixed NPC placements: `113` após remover o alias residual da Amanda em Pillai;
- maps client/server: `53` mapas estáticos;
- gameplay reference: `88` quests;
- quest runtime bindings suportados: `13`;
- mix recipes: `127`;
- quest full-entry capacity: `10`;
- TCoin reward path: presente e validado estaticamente.

Últimos packs observados após Amanda/Enzo:

| Pack | Estado / conteúdo |
|---|---|
| `definitions_client.scfg` | PASS, 2182 definitions |
| `definitions_server.scfg` | PASS, 2350 definitions |
| `text_resources.sres` | PASS |
| `localization.sloc` | PASS |
| `gameplay_config.sgcf` | PASS |
| `gameplay_reference.sgrf` | PASS, 88 quests / 13 supported / 127 mix |
| `field_runtime.sfld` | PASS, SSPB + SPPT + SPCN |
| `maps_client.smap` | PASS, 53 maps |
| `maps_server.smap` | PASS, 53 maps |

## 1.3 Ainda pendente

- Quest Codex: teste completo da UI em jogo.
- Mix Guide: teste completo da UI/NPC em jogo.
- Tier 5 / Rank5: validação end-to-end contra o comportamento esperado.
- Crash intermitente do cliente sob Wine: root cause ainda não fechada.
- Asset Tool: **migração canonical JSON ainda não terminou**. Ver secção 13.

---

# 2. Regra mental mais importante: as cinco camadas

O Summit não tem um único tipo de asset.

```text
1. CANONICAL AUTHORING
   SourceCode/Content/canonical/*.json
                    ↓
2. COMPILED CONTENT
   SourceCode/Content/packs/*
                    ↓ deploy
   cliente/Content/packs/*
   Servidor/Content/packs/*
                    ↓
3. PHYSICAL GAME ASSETS
   modelos / mapas / texturas / áudio / UI / animações
   fora de Content/
                    ↓
4. RUNTIME CODE / ADAPTERS
   SourceCode/Shared, SrcGame, SrcServer
                    ↓
5. PERSISTENT PLAYER/WORLD STATE
   Servidor/Database/summit.sqlite3
```

Nunca resolver um problema numa camada alterando outra sem confirmar a dependência.

Exemplos:

- NPC não devia aparecer numa cidade → normalmente **placement**, não apagar a NPC definition.
- NPC tem comportamento errado → definition/class/event binding, não placement.
- NPC aparece invisível → model/physical asset/path, não necessariamente definition.
- monstro errado nasce no mapa → stage spawn, não monster definition.
- mapa existe no catálogo mas falha a renderizar → physical SMD/material/texture, não necessariamente `maps_*.smap`.
- quest aparece no Codex mas não progride → Codex/reference é estático; progresso pertence ao quest runtime/persistência.

---

# 3. Layout do repositório

Layout operacional utilizado:

```text
summit/
├── SourceCode/
│   ├── Content/
│   │   ├── canonical/
│   │   └── packs/
│   ├── Shared/
│   ├── SrcGame/
│   ├── SrcServer/
│   ├── tools/
│   │   └── content_port/
│   └── docs/
├── cliente/
│   ├── Game.exe
│   ├── Content/packs/
│   └── assets físicos...
├── Servidor/
│   ├── Server.exe
│   ├── Database/summit.sqlite3
│   ├── Content/packs/
│   └── assets físicos...
└── bugfixes/
```

## 3.1 SourceCode/Content/canonical

É a fonte editável pretendida para dados estruturados.

Famílias atuais:

```text
Content/canonical/
├── text/
├── maps/
├── stage_spawns/
├── stage_start_points/
├── fixed_npcs/
├── item_profiles/
└── gameplay_reference/
```

## 3.2 SourceCode/Content/packs

Artefactos derivados. Não editar manualmente.

```text
Content/packs/
├── text_resources.sres
├── definitions_client.scfg
├── definitions_server.scfg
├── localization.sloc
├── gameplay_config.sgcf
├── gameplay_reference.sgrf
├── field_runtime.sfld
├── maps_client.smap
└── maps_server.smap
```

`stage_spawns.sspb` é obsoleto como pack runtime independente depois da consolidação no `field_runtime.sfld`.

---

# 4. Pipeline geral de build

Fluxo normal:

```text
editar canonical
      ↓
validate canonical
      ↓
build_from_canonical.py
      ↓
compiler por domínio
      ↓
inspector por pack
      ↓
Content/packs
      ↓
validate runtime contract
      ↓
deploy_runtime_content.py
      ↓
cliente/Content/packs
Servidor/Content/packs
      ↓
restart runtime
      ↓
teste em jogo
```

Comando principal atual:

```bash
python3 SourceCode/tools/content_port/build_from_canonical.py \
  --canonical-text SourceCode/Content/canonical/text \
  --canonical-spawns SourceCode/Content/canonical/stage_spawns \
  --canonical-maps SourceCode/Content/canonical/maps \
  --canonical-start-points SourceCode/Content/canonical/stage_start_points \
  --canonical-fixed-npcs SourceCode/Content/canonical/fixed_npcs \
  --canonical-item-profiles SourceCode/Content/canonical/item_profiles \
  --out-root SourceCode/Content
```

O build deve acabar com:

```text
CANONICAL_BUILD=PASS
```

Deploy:

```bash
python3 SourceCode/tools/content_port/deploy_runtime_content.py \
  --repo-root .
```

Depois reiniciar `Server.exe` e `Game.exe`.

---

# 5. Mapas: existem duas coisas diferentes

## 5.1 Map catalog/config

Canonical:

```text
Content/canonical/maps/client.maps.json
Content/canonical/maps/server.maps.json
```

Compiler:

```text
tools/content_port/compile_maps.py
```

Inspect:

```text
tools/content_port/inspect_maps.py
```

Outputs:

```text
maps_client.smap
maps_server.smap
```

Runtime:

```text
Shared/content/v1/map/content/CompiledMapContentSource.h
Shared/content/v1/map/legacy/LegacyMapAdapter.h
```

O pack contém configuração lógica como mapas, gates, stage objects, warp gates, start metadata e strings.

Client e server são separados porque historicamente existem diferenças semânticas entre os dois perfis.

### Atenção

No baseline inspecionado, o field dinâmico 53 (`smConfig`) continuava com uma boundary legacy. Antes de refatorá-lo, verificar a source atual.

## 5.2 Physical map assets

O `.smap` **não contém a geometria do mapa**.

O runtime ainda carrega ficheiros físicos como:

```text
field/.../*.smd
field/.../*.ase   (authoring/reference quando aplicável)
textures/materials relacionados
```

Os logs do servidor mostram diretamente carregamento de `.smd`.

Consequência:

> Não apagar/quarentenar SMD/ASE/texturas só porque o catálogo foi migrado para JSON/SMAP.

O Asset Tool trata esses formatos físicos como uma linha de trabalho diferente do content pipeline.

---

# 6. Field Runtime: spawns, start points e fixed NPC placements

Esta é uma das migrações mais completas.

Canonical:

```text
stage_spawns/
stage_start_points/
fixed_npcs/
```

Compiler:

```text
tools/content_port/compile_field_runtime.py
```

Inspector:

```text
tools/content_port/inspect_field_runtime.py
```

Output único:

```text
field_runtime.sfld
```

Secções:

```text
SSPB = stage spawns
SPPT = stage start points
SPCN = fixed NPC placements
```

Estado validado após Amanda/Enzo:

```text
stage_spawns:
  maps=54
  actors=262
  bosses=16

stage_start_points:
  maps=65
  points=6604

fixed_npcs:
  maps=24
  placements=113
```

## 6.1 Legacy SPM/SPP/SPC

Historicamente:

```text
SPM = monstros/spawn do mapa
SPP = start points
SPC = NPCs fixos
```

Depois da migração:

- o runtime normal não deve gravar SPM/SPP/SPC;
- canonical JSON é authoring;
- `field_runtime.sfld` é runtime;
- readers/writers legacy ficam apenas para import/recovery quando necessário.

Nunca editar o binário SPC para uma alteração normal de NPC placement.

---

# 7. Procedimento autónomo: personagem/NPC aparece numa cidade e não devia

Este é o procedimento recomendado.

## 7.1 Primeiro decidir: definition ou placement?

Pergunta:

> Este NPC deve continuar a existir noutro local?

Se sim, **não apagar a definition**. Remover apenas o placement.

## 7.2 Encontrar o mapa

Exemplo Ricarten:

```bash
find SourceCode/Content/canonical/fixed_npcs \
  -type f -iname '*village-2*.json' -print
```

Pillai:

```bash
find SourceCode/Content/canonical/fixed_npcs \
  -type f -iname '*pilai*.json' -print
```

## 7.3 Ver placements ativos

Com `jq`:

```bash
jq '.npcs[] | {slot,npc_id,x,y,z,ax,ay,az,state}' \
  SourceCode/Content/canonical/fixed_npcs/.../village-2.ase.spc.json
```

Ou procurar uma identidade:

```bash
grep -RIn '"npc_id": "npc\.' \
  SourceCode/Content/canonical/fixed_npcs
```

## 7.4 Descobrir a definition do NPC

```bash
grep -RIl '"id": "npc.ID_AQUI"' \
  SourceCode/Content/canonical/text/resources
```

Inspecionar:

```bash
jq '.id,.semantic.fields,.semantic.directives' ARQUIVO.json
```

Campos importantes:

- `class`;
- `model_file`;
- IDs/aliases;
- localization binding;
- server NPC/legacy provenance quando existente.

## 7.5 Remover somente o placement

Editar o array `npcs` daquele mapa e remover apenas a entrada correta.

Não renumerar slots desnecessariamente. Slots fazem parte da compatibilidade histórica e de auditoria.

## 7.6 Validar

```bash
python3 SourceCode/tools/content_port/validate_canonical_schemas.py \
  --canonical-text SourceCode/Content/canonical/text \
  --canonical-spawns SourceCode/Content/canonical/stage_spawns \
  --canonical-maps SourceCode/Content/canonical/maps \
  --canonical-start-points SourceCode/Content/canonical/stage_start_points \
  --canonical-fixed-npcs SourceCode/Content/canonical/fixed_npcs
```

## 7.7 Rebuild + deploy

Usar os comandos da secção 4.

## 7.8 Verificação

Comparar `inspect_field_runtime.py` antes/depois.

Se removeu um placement, é normal `SPCN items` cair em 1.

Depois testar visualmente na cidade.

---

# 8. NPC definitions: identidade, aparência, comportamento e texto

Um NPC não é uma única entrada.

```text
NPC identity
  ├─ definition            -> definitions_*.scfg
  ├─ localization/text     -> localization.sloc / text_resources.sres
  ├─ fixed placement       -> field_runtime.sfld / SPCN
  ├─ model path            -> physical char/npc asset
  └─ client/server logic   -> class/event/quest dispatch C++
```

Canonical definition pertence à família `npc_definition` dentro de:

```text
Content/canonical/text/resources/
```

Compiler:

```text
compile_definitions.py
```

Outputs:

```text
definitions_client.scfg
definitions_server.scfg
```

Runtime bridge:

```text
Shared/content/v1/definition/
```

## 8.1 Amanda / Enzo: exemplo definitivo

Amanda:

```text
canonical id: npc.ji_woo
class: 1014
model: char\npc\ji_woo\jiwoo.INI
name: Instrutora Amanda
placement:
  Ricarten / village-2
  slot 20
  x=-175442
  y=52480
  z=-4438784
  ay=1000
```

Alias histórico:

```text
npc.amanda
```

Deve permanecer semanticamente compatível, mas **não possuir placement ativo**.

Enzo:

```text
canonical id: npc.enzo_o_restaure
class: 1024
model: char\npc\Enzo\Enzo.ini
name: Enzo, o Restaurador de Itens
placement:
  Pillai / pilai
  slot 21
  x=268949
  y=167424
  z=19485184
  ay=1579
```

A causa do bug foi uma mistura de identidade:

- Amanda herdou `class=1024`;
- `1024` abre o restaurador;
- alias Amanda ainda tinha modelo/texto de Enzo;
- havia um placement residual de `npc.amanda` em Pillai.

Correção validada:

```bash
python3 SourceCode/tools/content_port/fix_amanda_enzo_identity_v3.py \
  --repo-root . --apply
```

Segundo run:

```text
CHANGES=0
RESULT=PASS
```

Validador:

```bash
python3 SourceCode/tools/content_port/validate_gameplay_closure.py \
  --repo-root .
```

Amanda e Enzo foram posteriormente confirmados em jogo.

---

# 9. Monstros

Há pelo menos três camadas.

## 9.1 Monster definition

Canonical:

```text
Content/canonical/text/resources/
family = monster_definition
```

Compilado em:

```text
definitions_client.scfg
definitions_server.scfg
```

Contém propriedades/config da entidade.

## 9.2 Spawn placement

Canonical:

```text
Content/canonical/stage_spawns/
```

Compilado na secção SSPB do:

```text
field_runtime.sfld
```

Um monster definition existir não significa que deva nascer num mapa.

## 9.3 Physical monster asset

Modelos, motions, skeletons, textures e áudio permanecem físicos, por exemplo famílias:

```text
char/monster/...
*.smd
*.inx
*.smb
textures
audio
```

Esses assets não são materializados pelo canonical content pack.

O PT Asset Tool possui reverse engineering separado para SMD/INX/SMB e workflows de Blender/candidate authoring.

## 9.4 Navisko

O bug “nightmare” foi fechado ligando diretamente a replacement enquanto `monster.35_minigue_prateado` materializa em `chrMonsterList[]`.

Resultado confirmado anteriormente: NPCs indevidos do caso desapareceram.

---

# 10. Items

Items têm uma pipeline própria e não devem ser tratados como simples texto.

## 10.1 Canonical definition

Dentro de `Content/canonical/text/resources` existe a família:

```text
open_item_definition
```

## 10.2 Item profile

Canonical:

```text
Content/canonical/item_profiles/client.items.json
Content/canonical/item_profiles/server.items.json
```

Os profiles refletem o universo de codes realmente conhecido por cada executável.

Ferramenta:

```text
materialize_item_profiles.py
validate_item_profile_pack.py
```

## 10.3 Definition compiler

```bash
compile_definitions.py ... --item-profile client.items.json
compile_definitions.py ... --item-profile server.items.json
```

O compiler filtra o que pertence a cada lado.

No último build:

```text
client profile codes = 1813
server profile codes = 1726
MISSING_CODE_ALL = 0
OUT_OF_PROFILE_ALL = 0
RESULT = PASS
```

`DUPLICATE_DIRECT_CODES` é informativo porque UniqueItem variants podem partilhar CODE.

## 10.4 Physical item assets

Ícones/modelos/texturas não ficam necessariamente em `Content`.

O Asset Tool distingue, por exemplo:

- inventory icons;
- `DropItem` world assets;
- UI image trees;
- source/model assets.

Alterar item definition não cria automaticamente o asset visual.

---

# 11. Text, localization e gameplay config

`Content/canonical/text` é uma árvore grande que contém vários tipos semânticos.

Famílias observadas:

```text
generic_text
ini_config
localization_resource
message_catalog
monster_definition
npc_definition
open_item_definition
sin_resource
```

Pipeline:

```text
canonical text
    ↓ materialize_localization_catalog.py
validate_canonical_text.py
validate_canonical_schemas.py
    ↓
compile_text_resources.py      -> text_resources.sres
compile_definitions.py         -> definitions_client/server.scfg
compile_localization.py        -> localization.sloc
compile_gameplay_config.py     -> gameplay_config.sgcf
```

Importante:

O corpus legacy misturava encodings. Não fazer rewrites genéricos dos `raw_values_hex` nem normalizar encoding sem prova.

O canonical guarda semantic fields + provenance/raw evidence justamente para preservar fidelidade.

---

# 12. Quests, Codex e Mix

## 12.1 Estado dinâmico da quest

É autoridade do servidor.

Inclui:

- start;
- accepted/in progress/completed;
- monster counters;
- item objectives;
- repeatability;
- reward grant;
- persistence.

A source REC174 já contém architecture v2 para repository/view/lifecycle.

Capacidade:

```text
QUEST_FULL_ENTRY_MAX = 10
```

TCoin:

```text
AddPlayerCoin(...)
```

está presente no reward path.

## 12.2 Quest Codex

Não é a QuestDB.

Canonical:

```text
Content/canonical/gameplay_reference/
├── index.json
└── quests/
    ├── priston_tale_quests.pt-BR.json
    └── runtime_bindings.json
```

Compiler:

```text
compile_gameplay_reference.py
```

Inspector:

```text
inspect_gameplay_reference.py
```

Output:

```text
gameplay_reference.sgrf
```

Estado:

```text
quests = 88
supported runtime bindings = 13
```

O Codex só deve apresentar como “suportada” uma quest vinculada ao runtime real.

## 12.3 Mix Guide

Canonical:

```text
gameplay_reference/mix/priston_tale_mix_table.pt-BR.json
```

Mesmo pack:

```text
gameplay_reference.sgrf
```

Estado:

```text
recipes = 127
weapon = 35
armor_or_robe = 32
shield = 30
orbital = 30
```

O Mix NPC usa EventCode `1001`; isto é namespace diferente de quest/class `wPlayClass[0] == 1001`.

O cliente V2 deve interceptar `OPCODE_cMIXGUIDE` e abrir o catálogo compilado local.

## 12.4 Tier 5

Existe implementação Rank5/Tier5 na source, mas ainda não declarar equivalência total com o guia PT-BR.

Status:

```text
implementation IDs: PASS
end-to-end runtime: PENDING
```

---

# 13. PT Asset Tool: estado real da migração para canonical JSON

## Resposta curta

**Não terminámos.**

Há duas linhas diferentes:

1. PT Asset Tool físico/reverse-engineering;
2. Summit canonical content authoring.

A versão V2.0 que foi preparada adiciona canonical-first **somente ao Field Editor SPM/SPP/SPC**.

Ela passou:

```text
PATCH_APPLY_CHECK=PASS
PY_COMPILE=PASS
JAVASCRIPT_NODE_CHECK=PASS
TARGETED_TESTS=10 PASS
CANONICAL_PREFERRED=PASS
LEGACY_RECOVERY_FORM_PRESERVED=PASS
```

Mas ficou pendente:

- rebase/integrar essa V2.0 na versão atual real do PT Asset Tool;
- NPC definition editor canonical;
- Monster definition editor canonical;
- OpenItem/item definition editor canonical;
- localization/text editor canonical;
- gameplay reference/quest editor canonical;
- integração schema-driven completa;
- eliminação gradual dos forms legacy de authoring normal.

Portanto:

> Não assumir que abrir “NPC”, “Monster” ou “Item” na ferramenta atual já grava o novo JSON canonical.

---

# 14. Como o Field Editor canonical-first do Asset Tool deve funcionar

Patch preparado:

```text
SUMMIT_ASSET_TOOL_CANONICAL_V2_0.patch
```

Módulo novo principal:

```text
pt_asset_tool/summit_canonical.py
```

Integrações:

```text
pt_asset_tool/ui_app.py
pt_asset_tool/ui_static/app.js
pt_asset_tool/ui_static/index.html
```

## 14.1 Detecção

Quando `source_root` aponta para uma source que contém:

```text
Content/canonical
tools/content_port
```

o backend canonical deve ganhar prioridade.

Caso contrário, o field editor legacy só aparece como recovery.

## 14.2 Workflow da UI

```text
Select Field
   ↓
Open SPM / SPP / SPC
   ↓
Create canonical draft
   ↓
Save canonical
   ↓
Build packs
   ↓
Deploy packs
   ↓
Test in game
```

Drafts ficam em:

```text
pt_build/summit_canonical/
```

e não alteram source automaticamente.

`Save canonical` exige:

- workspace/build armado;
- confirmação explícita `SAVE_CANONICAL`;
- proteção por SHA-256 para evitar escrever sobre source modificada.

## 14.3 Mapeamento dos botões

```text
SPM -> Content/canonical/stage_spawns
SPP -> Content/canonical/stage_start_points
SPC -> Content/canonical/fixed_npcs
```

SPC canonical usa:

```text
npc_id = npc.*
```

e não “nome livre + model + server .NPC” como autoridade.

Definition/comportamento do NPC continua em canonical text.

## 14.4 Build button

Chama:

```text
SourceCode/tools/content_port/build_from_canonical.py
```

## 14.5 Deploy button

Chama:

```text
SourceCode/tools/content_port/deploy_runtime_content.py
```

com `client_root` e `server_root`.

---

# 15. O que o outro chat precisa fazer no Asset Tool

Este é o handoff recomendado.

## Fase AT-1 — rebase

Rebasear:

```text
SUMMIT_ASSET_TOOL_CANONICAL_V2_0.patch
```

sobre a versão atual do PT Asset Tool.

Não substituir módulos inteiros cegamente. Preservar evoluções posteriores da ferramenta.

Rodar targeted tests e testes baseline.

## Fase AT-2 — transformar summit_canonical em repository genérico

Hoje ele é muito orientado a Field.

Criar operações comuns:

```text
discover canonical resource
load by stable id
create draft
semantic validate
atomic save with sha guard
build
inspect
deploy
```

## Fase AT-3 — NPC editor

Editar:

```text
family=npc_definition
```

Campos semanticamente seguros:

- stable ID;
- class/event role;
- model_file;
- name/localization binding;
- references explícitas.

Nunca gerar `raw_values_hex` de forma genérica.

Quando alterar campo conhecido:

1. atualizar semantic value;
2. atualizar raw directive preservando encoding;
3. atualizar payload/provenance usando codec/função oficial.

## Fase AT-4 — Monster editor

Mesmo princípio para:

```text
monster_definition
```

Separar sempre:

```text
monster definition != stage spawn != physical model
```

## Fase AT-5 — Item/OpenItem editor

Editar:

```text
open_item_definition
```

respeitando profiles client/server e `validate_item_profile_pack.py`.

Não permitir save que introduza code fora do profile sem ação explícita.

## Fase AT-6 — text/localization

Usar as chaves/catalogs oficiais.

Não fazer conversão global de encoding.

## Fase AT-7 — quests/reference

Editor de `gameplay_reference` deve editar somente reference/catalog.

Não expor progresso de personagem como asset.

## Fase AT-8 — validation contract

Cada editor deve oferecer:

```text
Draft
Validate
Save canonical
Build family/all
Inspect pack
Deploy
```

e mostrar erros do schema com path/campo/valor.

---

# 16. PT Asset Tool físico: o que ele faz fora do canonical Content

O PT Asset Tool já tem uma base extensa de reverse engineering para assets físicos.

Exemplos das famílias mencionadas no snapshot:

- SMD models;
- SMD stage geometry;
- INX motion/config;
- SMB skeleton/animation keys;
- `.part` effects;
- textures;
- UI image trees;
- Monster/NPC physical entity catalog;
- item visual relations;
- Blender import/review/candidate workflows.

A release documentada mantém production writes safety-locked para várias famílias.

Conceito:

```text
Canonical Content Tooling
  = config/definitions/placements/reference

Physical Asset Tooling
  = geometry/models/motions/textures/effects/UI/audio
```

Eles devem coexistir; um não substitui o outro.

---

# 17. Ficheiros do cliente/servidor fora de Content

Isto é esperado.

## 17.1 Physical character assets

Exemplos:

```text
char/npc/...
char/monster/...
*.SMD
*.SMB
*.INX
textures
audio
```

A definition compilada guarda o **path**; o ficheiro físico continua na árvore do runtime.

## 17.2 Stage/map physical assets

Exemplos vistos pelo runtime:

```text
field/forest/fore-3.smd
field/ricarten/village-2.smd
field/forever-fall/*.smd
```

O maps pack não substitui a geometria.

## 17.3 UI/frontend

Árvores físicas tratadas pelo Asset Tool incluem exemplos como:

```text
StartImage/Opening/npc
Image/Sinimage/NPC
ButtonInfo
sinGage
SkillIcon
ReStart
```

Esses ficheiros não são NPC runtime definitions.

## 17.4 Legacy server authoring paths

Historicamente:

```text
GameServer/NPC/*.NPC
GameServer/Field/*.spm
GameServer/Field/*.spp
GameServer/Field/*.spc
```

Para domínios já migrados, estes devem ser tratados como:

```text
import / recovery / provenance
```

e não como fonte de verdade runtime.

Não confundir `.spc` legacy com `.smd` de geometria: o primeiro foi substituído pelo field pack; o segundo ainda é asset físico.

---

# 18. Tools de Content Port e para que servem

## Orquestração

### `build_from_canonical.py`

Build completo de canonical para packs.

### `deploy_runtime_content.py`

Copia packs corretos para client/server e gera manifest/hash.

### `consolidate_canonical_content.py`

Normalizações/consolidação da árvore canonical. Usar com cuidado; é uma operação de conteúdo, não um editor casual.

## Validation

### `validate_canonical_text.py`

Integridade semântica/IDs do canonical text.

### `validate_canonical_schemas.py`

Schemas das famílias canonical.

### `validate_item_profile_pack.py`

Confere item definitions compiladas contra code map/profile de cada executável.

### `validate_internal_runtime_contract.py`

Guarda contracts de runtime e resolução de packs.

### `validate_runtime_pack_resolution.py`

Confere que client/server usam packs standalone corretos.

### `validate_gameplay_closure.py`

Contrato que adicionámos para:

- Amanda/Enzo;
- compiled definition identity;
- Quest Codex counts;
- runtime bindings;
- Mix recipes;
- quest capacity;
- TCoin;
- Tier5 status.

## Text

- `materialize_localization_catalog.py`
- `compile_text_resources.py`
- `inspect_text_resources.py`
- `compile_localization.py`
- `inspect_localization.py`
- `compile_gameplay_config.py`
- `inspect_gameplay_config.py`
- `compile_definitions.py`
- `inspect_definitions.py`

## Maps

- `semanticize_maps.py`
- `compile_maps.py`
- `inspect_maps.py`
- `extract_legacy_maps.py`
- `compare_map_shadows.py`

## Field

- `compile_field_runtime.py`
- `inspect_field_runtime.py`
- `import_field_runtime_legacy.py`
- `migrate_field_runtime.py`
- `compile_stage_spawns_canonical.py` — histórico/utility; runtime final usa field pack.
- `inspect_stage_spawns.py`

## Gameplay reference

- `compile_gameplay_reference.py`
- `inspect_gameplay_reference.py`

## Recovery

- `recover_forever_fall_spawns_v2.py`
- `recover_forever_fall_spawns_v3.py`
- `recover_forever_fall_spawns_v4.py`

V2/V3 apply standalone SSPB foi desativado depois de `field_runtime.sfld`. V4 é a linha correta quando recovery precisar reconstruir o field pack.

## Amanda/Enzo

- `fix_amanda_enzo_identity_v3.py`
- `validate_gameplay_closure.py`

---

# 19. Recipes de manutenção autónoma

## 19.1 Mover um NPC

1. Não alterar definition se identidade/comportamento continuam iguais.
2. Localizar `fixed_npcs` do mapa antigo.
3. Remover placement.
4. Localizar mapa novo.
5. Adicionar placement com `npc_id`.
6. schema validation.
7. build.
8. inspect field runtime.
9. deploy.
10. teste.

## 19.2 Remover NPC apenas de uma cidade

Só remover placement.

## 19.3 Desativar NPC globalmente

Decisão mais séria:

- remover todos placements;
- confirmar que nenhum quest/shop/system depende dele;
- só depois considerar deprecating definition.

## 19.4 Trocar modelo de NPC

Alterar `model_file` na NPC definition.

Depois confirmar que o physical asset existe no client.

Build definitions + deploy.

## 19.5 Trocar comportamento de NPC

Identificar `class`, EventCode ou dispatcher C++ correspondente.

Não escolher números “parecidos”. Procurar consumer real no client/server.

Amanda/Enzo provou por que isto importa.

## 19.6 Alterar spawn de monstro

Editar `stage_spawns`.

Não mexer em monster definition se o problema é quantidade/mapa.

Build `field_runtime.sfld`.

## 19.7 Alterar stats/config de monstro

Editar `monster_definition`.

Build definitions.

## 19.8 Alterar item

Editar OpenItem canonical, respeitar item profile.

Build definitions e rodar `validate_item_profile_pack.py`.

## 19.9 Alterar lore/descrição do Codex

Editar gameplay reference.

Não altera quest state.

Build SGRF.

## 19.10 Alterar quest real

Não basta editar Codex.

Precisa auditar:

- quest repository;
- eligibility;
- objectives;
- persistence;
- reward grant;
- packet/view;
- UI;
- reference binding.

---

# 20. Build/deploy por domínio

## Só definitions/text

Mesmo que só um domínio mude, hoje o caminho mais seguro é rodar o build completo.

Motivo:

- localization catalog pode ser materializado;
- definitions dependem de item profiles;
- packs possuem cross-contracts.

## Só placement/spawn

Também pode rodar compiler específico durante desenvolvimento:

```bash
python3 SourceCode/tools/content_port/compile_field_runtime.py \
  --canonical-spawns SourceCode/Content/canonical/stage_spawns \
  --canonical-start-points SourceCode/Content/canonical/stage_start_points \
  --canonical-fixed-npcs SourceCode/Content/canonical/fixed_npcs \
  --canonical-text SourceCode/Content/canonical/text \
  SourceCode/Content/packs/field_runtime.sfld

python3 SourceCode/tools/content_port/inspect_field_runtime.py \
  SourceCode/Content/packs/field_runtime.sfld
```

Para release, preferir build completo.

---

# 21. Runtime pack resolution

O runtime resolve packs ao lado do executável em:

```text
Content/packs/
```

A arquitetura foi validada como:

```text
PACK_RESOLUTION=STANDALONE_EXECUTABLE_CONTENT
```

Isto significa:

```text
cliente/Game.exe
cliente/Content/packs/...

Servidor/Server.exe
Servidor/Content/packs/...
```

Não depender do current working directory ou de `SourceCode/Content` em produção.

---

# 22. Persistence e account data

DB:

```text
Servidor/Database/summit.sqlite3
```

O content pipeline não é a DB pipeline.

Amanda/Enzo, maps, items definitions e packs são content.

Quest progress/account/character são persistence.

No incidente de login:

- conta estava válida;
- TCP 1997 estava aberto;
- causa foi routing de pré-auth;
- não foi corrupção de account persistence.

---

# 23. Networking: regressão REC170 que não pode voltar

REC170 colocou o gameplay router antes do legacy switch.

Bug:

```text
invalid PlayerId
  ↓
router returned InvalidPacket
  ↓
pre-auth legacy packet was swallowed
```

Contrato corrigido:

```text
known modern opcode + valid player
  -> handler

known modern opcode + invalid player
  -> InvalidPacket

unknown/legacy opcode + valid player
  -> UnknownOpcode

unknown/legacy opcode + invalid/pre-auth player
  -> UnknownOpcode
```

Patch confirmado:

```text
SUMMIT_REC174_LOGIN_ROUTER_PREAUTH_HOTFIX_V2_2026-09-29.patch
```

Não voltar a validar `PlayerId` antes de verificar ownership do opcode.

---

# 24. Histórico das migrações importantes

## Field V1–V1.6

- SPM/SPP/SPC → canonical;
- field_runtime SFLD;
- NPC interaction state/life hotfix;
- Navisko direct bind.

## Quest V1.7

Corrigia 10 slots e TCoin no código antigo.

**Não reaplicar** sobre REC174: REC174 já absorveu esses semantics.

## Codex V1.8

88 quests, 13 bindings.

Na source REC174/rebase atual foi integrado por patch novo.

## Mix V1.9

127 recipes.

## Amanda/Enzo V1.11–V1.11.3

- corrigiu class/model/text/placement;
- tornou finder canonical robusto;
- removeu alias placement residual;
- adicionou gameplay closure validator;
- corrigiu validation do gameplay reference.

---

# 25. Patches obsoletos / cautela

Não aplicar cegamente patches antigos sobre a árvore atual.

Especialmente:

```text
SUMMIT_QUEST_RUNTIME_V1_7_FULL_DATA_REWARDS.patch
SUMMIT_QUEST_CODEX_V1_8_PTBR.patch
SUMMIT_MIX_GUIDE_V1_9_COMPILED_REFERENCE.patch
SUMMIT_REC174_REBASE_FIELD_QUEST_CODEX_MIX_AMANDA_2026-09-29.patch
```

Os três primeiros já foram absorvidos/rebaseados.

O grande `REC174_REBASE...` antigo conflita porque a árvore atual já contém Field Runtime e outras mudanças.

A linha incremental funcional atual foi:

```text
REC170
REC171
REC172
REC173
REC174
login router hotfix V2
POST_LOGIN_CODEX_MIX_AMANDA_REBASE_V2
V1.11 Amanda/Enzo tools
V1.11.1 canonical map resolver
V1.11.2 fixed NPC discovery
V1.11.3 gameplay reference validator
```

---

# 26. Git safety

O repo contém trabalho intercalado.

Evitar:

```bash
git add .
git reset --hard
git checkout .
git restore .
```

sem scope.

Preferir:

```bash
git diff -- path/especifico
git add path/especifico
git apply --check ...
```

Antes de patch:

```bash
git apply --check --directory="SourceCode/" bugfixes/PATCH.patch
```

Depois só aplicar se passar.

---

# 27. Quarantine legacy

Existe ferramenta:

```text
quarantine_legacy_runtime_content.py
```

Mas não usar de forma ampla sem prova.

Regra:

1. verificar que canonical existe;
2. build pack;
3. runtime consumer usa pack;
4. procurar reads diretos;
5. quarantine uma família;
6. testar;
7. manifest/restore.

Não quarantine assets físicos apenas porque config foi migrada.

---

# 28. Crash do cliente sob Wine

Mensagens observadas:

```text
msvcrt:_invalid_parameter
wined3d_swapchain_resize_buffers Something's still holding back buffer
SEH invalid frame
Exception frame is not in stack limits
```

Ainda não existe root cause.

`_invalid_parameter` sozinho não prova crash; já apareceu no server e o startup continuou.

Hipótese do back buffer ainda é investigação.

Quando reproduzir:

1. guardar terminal completo;
2. registar ação exata: login/map/resize/alt-tab/NPC/UI;
3. obter stack/backtrace;
4. instrumentar device reset/render path;
5. comparar com Windows nativo se disponível.

Não alterar content/persistence por causa deste erro sem correlação.

---

# 29. Como testar antes de declarar “closed”

## Content change

```text
canonical schema PASS
compiler PASS
inspector PASS
pack resolution PASS
deploy PASS
runtime test PASS
```

## NPC

```text
definition
model
name
class/event
placement
interaction
```

## Monster

```text
definition
spawn
physical model
runtime behavior
```

## Item

```text
definition
profile
pack
inventory behavior
visual asset when relevant
```

## Quest

```text
eligibility
start
objective progress
persistence
reward
UI/reference
repeatability
```

---

# 30. Estado do Asset Tool para outro chat — resumo copiável

```text
PT ASSET TOOL / SUMMIT CANONICAL HANDOFF

Current fact:
- PT Asset Tool itself is a mature reverse-engineering/physical-asset tool.
- Production writes remain safety-gated for multiple physical formats.
- SUMMIT_ASSET_TOOL_CANONICAL_V2_0.patch was prepared and tested.
- That patch makes Field SPM/SPP/SPC authoring canonical-first when source_root
  exposes SourceCode/Content/canonical.
- Flow: Draft -> Save canonical -> Build packs -> Deploy packs.
- Legacy field binary writer remains recovery fallback only.
- Drafts: pt_build/summit_canonical.
- Save uses explicit confirmation + source SHA race guard.
- Build hooks build_from_canonical.py.
- Deploy hooks deploy_runtime_content.py.

NOT DONE:
- V2.0 has not been fully rebased/integrated into the latest real PT Asset Tool line.
- NPC definition editor is not canonical-first.
- Monster definition editor is not canonical-first.
- OpenItem/item editor is not canonical-first.
- text/localization editor is not canonical-first.
- quest/gameplay_reference editor is not canonical-first.
- schema-driven forms for those families are not complete.

Next implementation:
1. rebase V2.0 preserving newer PT Asset Tool code;
2. extract generic canonical repository/draft/save contract;
3. add family-specific editors;
4. preserve raw_values_hex/legacy byte evidence;
5. use official schemas and semantic validators;
6. reuse Build/Deploy hooks;
7. add tests for every family;
8. legacy editing only under explicit Recovery surface.
```

---

# 31. Checklist para outro ChatGPT/dev iniciar trabalho

Antes de alterar qualquer coisa:

```text
1. Ler este documento.
2. Obter git status.
3. Não aplicar patches antigos automaticamente.
4. Identificar qual das 5 camadas contém o problema.
5. Localizar canonical ID antes de usar nomes humanos.
6. Encontrar runtime consumer correspondente.
7. Fazer dry-run quando tool disponibiliza.
8. Alterar a menor superfície possível.
9. Build + inspectors.
10. Deploy + runtime test.
```

Perguntas obrigatórias:

```text
É definition ou placement?
É config ou physical asset?
É reference de quest ou quest state?
É client-only, server-only ou shared?
O pack já é runtime-authoritative?
Há um legacy reader residual?
```

---

# 32. Source of truth hierarchy

Para dados migrados:

```text
1. canonical JSON
2. compiled pack generated from it
3. runtime consumer
4. legacy file only as provenance/recovery
```

Para estado de jogador:

```text
1. server domain logic
2. repository/persistence
3. SQLite
4. packets/views
5. client presentation
```

Para physical assets:

```text
1. physical runtime asset
2. proven Asset Tool contract
3. source/authoring asset if available
```

---

# 33. Conclusão

O projeto já deixou de ser uma simples árvore Priston legacy.

A arquitetura atual é híbrida, em migração:

```text
Legacy physical engine/assets
        +
Modern canonical configuration
        +
Compiled versioned packs
        +
V2 domain/network boundaries
        +
Adapters para runtime histórico
```

O objetivo não é apagar o legacy indiscriminadamente.

O objetivo é:

```text
canonical authoring
   + explicit contracts
   + deterministic compilation
   + safe deployment
   + legacy only where physically/runtime necessary
```

Este princípio deve orientar todos os próximos passos.
