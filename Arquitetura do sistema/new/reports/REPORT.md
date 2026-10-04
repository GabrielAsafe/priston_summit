# Summit — Relatório consolidado de expansão/content-driven

## Resultado

A lista inicial foi convertida em arquitetura, milestones, bug reports, audits, conteúdo canônico proposto, ferramentas e POCs/patches. Não ficou nenhuma área inicial sem caminho técnico definido.

Os dois eixos de história são milestones independentes e prioritários:

- `milestones/M01_WORLD_STORY_COLLECTIVE.md` — progresso coletivo global, persistente e irreversível;
- `milestones/M02_WORLD_PHASES_CITY_VARIANTS.md` — mudanças físicas do mundo: cidade, map variants, mobs, NPCs, gates e players online/offline.

## Mapas

`bugreports/BR001_MAP_CONTENT_DRIVEN_FIELD53.md` especifica a remoção do contrato fixo 53/field53 e do limite prático `STAGE_AREA_MAX=64`. Depois desse ticket, novos maps devem entrar por conteúdo + build/publish, sem aumentar contador em C++.

`bugreports/BR002_NPC_COORDINATE_SPACE.md` resolve a diferença entre coordenadas do overlay/map e placements de NPC: authoring passa a world units e o compiler converte para fixed-point legado.

## Servidor/char

O servidor atual ainda precisa de uma parte real de `/Servidor/char`; não é seguro apagar a pasta. O bundle contém inventário, gaps e um plano em duas fases para gerar manifest e depois tornar o model loader headless.

## Conteúdo/legacy

- monster/NPC definitions: canonical + compiled definitions é a direção ativa;
- 100 definições do pacote a portar foram auditadas: 84 equivalentes e 16 variantes preservadas sem sobrescrever o runtime;
- model configs: 364 configs únicos catalogados; INX deve virar artefato de build a partir de JSON;
- `/Servidor/Field`: assets binários legítimos; não converter geometry para JSON;
- configs: separar gameplay canonical, runtime mutable e secrets server-only.

## Gameplay novo

- altar Covenant: patch + placement plan;
- equipment sets: serviço server-authoritative + canonical;
- máscaras/capacetes: milestone de head cosmetic primeiro, helmet com stats depois;
- classes: workflow completo baseado no registry V2;
- balance: planilha com monstros/drops/itens/EXP + patches de correção runtime.

## Validação do bundle

Ver `reports/validation/FINAL_VALIDATION.txt`.

Na geração final:

- 369 documentos do conteúdo estendido passaram JSON Schema;
- 6.183 JSONs foram parseados;
- smoke `WorldStoryContributionService`: PASS;
- smoke `WorldPhaseRuntime`: PASS;
- smoke `EquipmentSetService`: PASS;
- scan básico de assignments de secrets no bundle: 0;
- planilha final presente.

## Aplicação

O bundle não modifica automaticamente a checkout local. Usar `docs/APPLY_AND_VALIDATE.md` e `milestones/INDEX.md` como ordem de execução. POCs precisam de review/integration com os adapters reais antes de produção.
