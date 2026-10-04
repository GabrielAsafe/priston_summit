# Visão geral, decisões e estado atual

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. Objetivo

Esta documentação consolida as decisões e os procedimentos necessários para terminar a migração de conteúdo e as mecânicas relacionadas a vendedores, CoinShop, drops, asas, economia, eventos, companions, mineração/lapidação e localização.

A regra arquitetural adotada é simples:

- **conteúdo estático e versionável** deve morar em `Content/canonical/...`;
- **estado mutável de jogador/servidor** deve morar na base de dados;
- **regras de domínio** devem ficar no source, sem carregar catálogos de conteúdo hardcoded;
- **texto mostrado ao jogador** deve vir da localização, e não de literais espalhados pelo C++.

## 2. Estado confirmado pela auditoria

| Área | Estado confirmado |
|---|---|
| CoinShop original na DB | 144 linhas: 110 `coin` + 34 `time` |
| CoinShop migrada para JSON | 142 linhas: 109 `coin` + 33 `time` |
| Asas retiradas da CoinShop | `QW103` e `QW106` |
| Itens inválidos no catálogo CoinShop original | 0 |
| Drops analisados | 22 arquivos |
| Referências de drops a itens inexistentes | 0 |
| Itens conhecidos pelos perfis auditados | 1.985 códigos |
| Auditoria automática de pt-BR | 0 ocorrências de script estrangeiro/mojibake segundo as regras atuais |
| Candidatos a texto hardcoded no cliente | 2.989 |
| Candidatos a texto hardcoded no servidor | 1.960 |
| Migração completa de hardcodes | **não concluída**; a lista ainda precisa de classificação semântica |
| Mapas canónicos | 53 no cliente e 53 no servidor |
| NPC definitions server | 174 documentos auditados |
| Diretivas de venda NPC | 162 |
| Códigos de shop NPC resolvidos | 306 |
| Códigos legados de shop filtrados | 7 |

## 3. Decisões principais

### CoinShop

O catálogo estático deve ficar em:

`Content/canonical/commerce/coinshop.json`

A DB deve continuar contendo somente estado mutável e histórico, como saldo, pagamentos e transações. A migration `046_coinshop_catalog_to_json.sql` remove somente `CoinShopDB__ItemList` e `CoinShopDB__ItemList_T`.

### Vendedores NPC normais

Não usam `coinshop.json`. O inventário de vendedor é definido nas NPC definitions canónicas em:

`Content/canonical/text/resources/Servidor/GameServer/NPC/*.json`

As diretivas relevantes são `sell_weapon`, `sell_defence` e `sell_etc`.

### Asas

As asas devem permanecer no fluxo de quest/craft existente e não em CoinShop. `QW103` e `QW106` foram excluídas do catálogo migrado. O source contém a cadeia `QW101 -> ... -> QW106` e deve ser preservado.

### Eventos

A melhor separação para este projeto é:

- JSON: definição do evento, season, agenda, recompensas, mapas, regras configuráveis;
- DB: progresso, claims, cooldowns, ranking, overrides operacionais e auditoria;
- source: motor/scheduler/event bus, validação e execução das regras.

### Companions

Não deve existir uma única regra global de alvo para todo buff. A política de alvo deve ser explícita por skill. O comportamento atual é misto: buffs genéricos aplicam no companion caster, enquanto várias skills de suporte têm propagação específica para owner/companions.

### Mineração e lapidação

Mineração deve produzir minério bruto. Lapidação deve consumir o bruto e produzir **os itens shelton já existentes (`OSxxx`)**, para que Aging/Mix continuem usando o sistema legado sem criar uma segunda família incompatível de pedras.

### Localização

Cada locale é isolado. `pt-BR` só deve conter português, `en` só inglês, `ko` coreano, `zh-TW` chinês tradicional. Não há locale espanhol canónico ativo na base auditada; um futuro `es`/`es-ES` deve ser criado completo, e não por mistura parcial.

## 4. Ordem recomendada de execução

1. Congelar e validar o catálogo CoinShop JSON em produção.
2. Resolver os 7 códigos de vendedores NPC filtrados.
3. Executar smoke test das asas sem CoinShop.
4. Criar o catálogo/scheduler de seasons e eventos.
5. Instrumentar XP, ouro e valor esperado de loot por mapa antes de rebalancear.
6. Formalizar target policy de companions.
7. Criar a ponte `minério bruto -> lapidação -> OSxxx`.
8. Classificar os 4.949 candidatos de hardcode e migrar apenas texto realmente player-facing.
9. Executar QA de localização automático + smoke de cenas/NPCs.
10. Fechar gates nativos do backlog V2 (compile/link/startup/in-game).

## 5. Definition of Done geral

O trabalho deve ser considerado concluído quando:

- nenhum catálogo estático da CoinShop depender de `ItemList`/`ItemList_T`;
- nenhum item configurado em shop ativo for silenciosamente inválido;
- asas premium não puderem ser compradas e a cadeia de quest/craft funcionar do início ao fim;
- XP/riqueza por mapa tiver metas mensuráveis e telemetria de validação;
- eventos puderem ser ativados por season sem recompilar servidor;
- cada buff de companion tiver alvo configurado/testado;
- mineração gere matéria-prima e lapidação gere sheltons consumíveis pelo Aging/Mix atual;
- cada literal player-facing tiver chave de localização;
- todas as locales suportadas passarem auditoria de idioma, placeholders e runtime smoke.
