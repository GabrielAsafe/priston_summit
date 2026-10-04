# Summit Expansion Documentation Bundle

Este pacote é a entrega consolidada do trabalho de expansão/content-driven.

## Começar por aqui

1. `REPORT_STATUS.md` — o que está pronto e o que exige aplicação na checkout real.
2. `milestones/INDEX.md` — ordem recomendada dos milestones.
3. `milestones/M01_WORLD_STORY_COLLECTIVE.md` — história coletiva/global irreversível.
4. `milestones/M02_WORLD_PHASES_CITY_VARIANTS.md` — cidade destruída/construída, mobs/NPCs/gates por phase.
5. `bugreports/BR001_MAP_CONTENT_DRIVEN_FIELD53.md` — remover 53/field53 e limite prático de StageArea.
6. `docs/INITIAL_REQUIREMENTS_CLOSURE.md` — cruzamento com a lista original.

## Estrutura

- `milestones/` — documentação de implementação e definition of done.
- `bugreports/` — bugs/limitações concretas da source.
- `audits/` — status de `Servidor/char`, `Configs`, `GameServer/Monster`, `Field`, model configs e paridade de maps.
- `patches/` — POCs e patches aplicáveis/revisáveis.
- `content/` — canonical content proposto/gerado e catálogos auxiliares.
- `tools/` — content pipeline original + ferramentas adicionadas.
- `pt_asset_tool/` — módulos usados/propostos para integrar autoria canonical e INX.
- `reports/` — inventários e resultados estruturados.
- `spreadsheets/` — balanceamento com relação dos nomes de itens.
- `docs/` — documentação temática anterior, mantida como referência complementar.

## Princípio arquitetural

`Content/canonical/**` é a source of truth versionável. O PT Asset Tool é o frontend de edição/validação/build/publish. Packs binários e `.inx` são artefatos derivados quando possível. Assets pesados como SMD/SMB/texturas/audio continuam binários e são referenciados pelo conteúdo.

## Segurança

Nenhum arquivo original de configuração contendo credenciais foi incluído no bundle. A documentação apenas registra que configs legados misturam secrets e recomenda separação server-only.
