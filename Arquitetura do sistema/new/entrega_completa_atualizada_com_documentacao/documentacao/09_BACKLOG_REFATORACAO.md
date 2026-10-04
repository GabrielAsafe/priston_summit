# Backlog de refatoração das outras mecânicas

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. Escopo auditado

A árvore V2 já contém domínios para classes, conditions, covenants, events, gathering, hunting, mining, modifiers, professions, progression, quests, reputation, rewards, skills e story, além de runtimes server-side correspondentes para várias dessas áreas.

O backlog abaixo separa **pendências encontradas/documentadas no source** de **novas tarefas decorrentes dos objetivos atuais**.

## 2. P0 — bloqueadores de release/consistência

### P0.1 Build nativo e runtime smoke V2

O próprio `Playable Classes V2 — Migration Status` ainda lista como gates externos:

- MinGW i686 client compile/link;
- MinGW i686 server compile/link;
- startup/restart de `Server.exe`;
- create/select/login/relog das oito classes;
- smoke de model/head/armor/equipment;
- smoke de item specialization/OpenItem;
- skill/rank smoke.

Sem estes gates, o source está “merge-ready” no domínio, mas não plenamente aceito em runtime.

### P0.2 CoinShop JSON em produção

- garantir que `coinshop.json` está no path esperado no deploy;
- aplicar migration 046 somente depois de backup;
- validar 109 coin + 33 time;
- validar saldo/histórico preservados;
- smoke de compra.

### P0.3 Asas

- confirmar ausência da CoinShop;
- smoke completo da cadeia QW101-QW106.

### P0.4 Localização player-facing crítica

Classificar primeiro UI/diálogos ativos. Não tentar migrar cegamente os 4.949 candidatos.

## 3. P1 — sistemas necessários para o design pedido

### P1.1 Vendedor NPC authoring

Criar ferramenta para editar `sell_weapon`/`sell_defence`/`sell_etc` sem manipular `raw_values_hex` manualmente.

Resolver os 7 códigos únicos atualmente filtrados.

### P1.2 Events/Seasons

Criar canonical event catalog + scheduler + persistência de instance/progress/claim.

### P1.3 Economia por mapa

Criar telemetria e catálogo de targets; só então rebalancear XP/dinheiro/drop value.

### P1.4 CompanionTargetPolicy

Remover semântica de target espalhada por skill code e tornar alvo explícito/configurável.

### P1.5 Lapidação

Criar ponte gathering resource -> `OSxxx` com transação atómica e integração ao Aging existente.

## 4. P2 — redução progressiva de legado

### P2.1 Class content file-backed

O status de Classes V2 informa que `IClassContentSource` já desacopla o domínio, mas as sources built-in ainda são C++ compilado. Migrar para conteúdo file-backed pode ser feito depois dos gates nativos.

### P2.2 Skill handlers concretos

O `MERGE_READINESS` deixa explícito que rewrite de handlers concretos de skill não é considerado completo. Não empurrar essa lógica para `ClassDefinition`.

### P2.3 Eventos/localização históricos

Nem toda tabela antiga de evento/localização foi reescrita. Migrar por subsystem, com tests, em vez de uma substituição global.

### P2.4 NPC definitions author-friendly

O formato canónico atual ainda preserva bytes legados. Evoluir para um schema onde `semantic` seja a verdadeira fonte de authoring e os bytes sejam gerados, não editados manualmente.

## 5. Dívida que não deve voltar

- class ranges/tables hardcoded em sistemas genéricos;
- conteúdo sazonal com datas no C++;
- catálogo CoinShop na DB estática;
- item code hardcoded em UI;
- traduções dentro de source;
- regra especial por companion skill quando uma policy de dados resolveria;
- novo sistema de pedra incompatível com OS/Aging existente.

## 6. Critério para migrar uma mecânica

Uma mecânica só deve ser marcada como migrada quando tiver:

1. autoridade definida (Content/DB/source);
2. schema/versionamento;
3. validator;
4. build/deploy path;
5. runtime path ativo;
6. persistência, se necessária;
7. observabilidade/logs;
8. smoke test;
9. rollback/migration plan;
10. remoção ou isolamento do caminho legado antigo.

## 7. Ordem técnica sugerida

`CoinShop/vendor integrity -> localization critical path -> events/seasons -> economy telemetry -> companions targeting -> lapidation -> remaining legacy content authoring -> class file-backed/skill cleanup`

Essa ordem reduz primeiro falhas de conteúdo visíveis e cria telemetria antes de mexer em economia.
