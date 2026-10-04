# Gerenciamento de eventos e seasons

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. O que existe hoje

Há duas coisas diferentes chamadas “eventos” no projeto:

### Event bus V2

`Shared/gameplay/events/v2/GameplayEventHub.h` fornece `Subscribe`/`Publish`. O servidor publica `MonsterKilledEvent` e consumidores como quests/hunting/progression reagem a ele.

Isso é **infraestrutura de eventos de domínio**, não um sistema de temporadas.

### Eventos de gameplay legados

Existem módulos separados para Bellatra, Firecracker, Hell's Gate, Bless Castle, event spawn, caravan e outros. Configuração e conteúdo ainda estão dispersos.

Não foi encontrado um catálogo canónico de `seasons` que seja hoje a autoridade para agenda/ativação desses eventos.

## 2. Decisão recomendada: JSON + DB, cada um com responsabilidade diferente

Colocar toda definição de evento apenas na DB facilitaria edição operacional, mas pioraria versionamento, review, diff e deploy reproduzível. Colocar tudo apenas em JSON tornaria progresso/claims e alterações operacionais inconvenientes.

A separação recomendada é:

### JSON — conteúdo estático/versionável

- ID do evento;
- season à qual pertence;
- período padrão;
- mapas/áreas;
- regras;
- reward tables;
- multiplicadores;
- assets/localization keys;
- pré-requisitos;
- feature flags default.

### DB — estado mutável

- season/event instance ativa;
- override de início/fim quando necessário;
- progresso por jogador/conta;
- claims;
- cooldowns;
- ranking/score;
- auditoria;
- override operacional emergencial.

### Source — motor

- scheduler;
- validação;
- handlers;
- integração com event bus;
- aplicação server-authoritative de rewards;
- persistência.

## 3. Estrutura proposta

```text
Content/canonical/events/
  index.json
  seasons.json
  definitions/
    halloween.json
    christmas.json
    anniversary.json
```

Exemplo conceitual:

```json
{
  "schema": "summit.seasons/v1",
  "seasons": [
    {
      "id": "season.2026.q4",
      "starts_at": "2026-10-01T00:00:00Z",
      "ends_at": "2026-12-31T23:59:59Z",
      "events": ["event.halloween", "event.christmas"]
    }
  ]
}
```

Cada event definition deve referenciar localization keys, nunca texto player-facing embutido.

## 4. Precedência de configuração

Adotar regra explícita:

1. definição canónica JSON;
2. instance/override autorizado na DB;
3. feature switch operacional;
4. source nunca contém datas/recompensas sazonais hardcoded, exceto fallback de segurança.

Registrar no log qual camada decidiu o estado final.

## 5. Scheduler

O scheduler deve calcular estados claros:

- `scheduled`;
- `active`;
- `grace_period` (opcional para claims);
- `ended`;
- `disabled`.

A troca deve ser idempotente. Reiniciar o servidor no meio de uma season não pode duplicar spawn, reward ou reset.

## 6. Integração com GameplayEventHub

O `GameplayEventHub` deve continuar sendo usado para fatos de domínio (`MonsterKilledEvent`, futuro `ItemCraftedEvent`, `LapidationCompletedEvent`, etc.).

O season manager não substitui o event bus. Ele decide **quais regras/handlers estão ativos** e os handlers reagem aos eventos publicados.

## 7. Migração dos eventos legados

Para cada evento legado:

1. mapear entrada/saída e estado global;
2. separar constantes configuráveis de algoritmo;
3. mover constantes/rewards/textos para definition JSON;
4. encapsular o handler legado atrás de um `event.id`;
5. persistir apenas o estado necessário;
6. adicionar idempotency key para rewards;
7. testar restart durante evento;
8. somente depois remover config/hardcodes antigos.

## 8. Tabelas sugeridas

Não é obrigatório usar exatamente estes nomes, mas o modelo deve separar definição de estado:

- `EventSeasonInstance` — season ativa e overrides;
- `EventInstance` — execução de um evento;
- `EventPlayerProgress` — progresso;
- `EventClaim` — reward claim com unique key;
- `EventRanking` — quando aplicável;
- `EventAudit` — transições/admin actions.

Não duplicar a definition JSON inteira na DB.

## 9. Acceptance tests

- [ ] ligar/desligar evento sem recompilar;
- [ ] server restart não duplica reward/spawn;
- [ ] início e fim em UTC são determinísticos;
- [ ] locale de cada mensagem é resolvido por chave;
- [ ] reward inválido impede ativação no preflight;
- [ ] map/item IDs inexistentes impedem build/deploy;
- [ ] claim repetido é rejeitado de forma idempotente;
- [ ] season pode agrupar múltiplos eventos;
- [ ] override de DB fica auditado.
