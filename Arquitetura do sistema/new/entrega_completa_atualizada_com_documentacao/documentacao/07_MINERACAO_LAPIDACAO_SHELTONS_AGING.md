# Mineração, lapidação, sheltons e Aging

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. O que já existe

O domínio V2 de gathering/mining já possui:

- `GatheringKind::Mining`;
- `ResourceDefinition`;
- `GatheringNodeDefinition`;
- validação de profissão/nível/posição/respawn;
- `MiningService`;
- evento `MiningNodeHarvestedEvent`;
- persistência criada nas migrations `044` e `045`;
- recursos seed como `iron_ore`, `copper_ore` e `mystic_crystal`.

O MiningService também já prevê materiais especiais/refinados como stacks virtuais com chaves `special_material:<resource>` e `refined_material:<resource>`.

O comentário no próprio serviço deixa a ponte para item IDs legados como extensão futura.

## 2. Objetivo de design

Fluxo desejado:

`minerar -> minério bruto -> lapidar -> pedra/shelton existente -> Aging/Mix`

O ponto central é **não criar uma segunda economia de pedras paralela ao sistema OS existente**.

## 3. Autoridade de inventário

Antes de implementar lapidação, escolher uma autoridade para minério bruto:

### Opção A — Gathering inventory virtual

Prós: simples para profissão, stack grande, menos dependência do inventário legado.  
Contras: precisa UI/bridge para o jogador perceber e usar o material.

### Opção B — Item físico no inventário

Prós: visual, negociável e compatível com UI existente.  
Contras: exige códigos OpenItem/assets e aumenta pressão de slots.

Recomendação para o estado atual: manter minério bruto na camada de gathering enquanto o sistema amadurece, mas a lapidação deve ser transação server-side capaz de consumir esse saldo e entregar um item `OSxxx` real.

## 4. Lapidação como serviço próprio

Criar domínio, por exemplo:

```text
Shared/gameplay/lapidation/v1/
SrcServer/src/systems/gameplay/lapidation/v1/
Content/canonical/crafting/lapidation.json
```

Recipe proposta:

```json
{
  "id": "lapidation.iron.tier1",
  "input": {"resource": "iron_ore", "amount": 10},
  "requirements": {"profession": "miner", "level": 5},
  "cost": {"gold": 1000},
  "outputs": [
    {"item_code": "OS103", "weight": 100}
  ]
}
```

Os valores/códigos são apenas exemplos de schema; o mapping final deve usar a progressão de sheltons escolhida pelo design.

## 5. Transação segura

A operação deve ser atómica:

1. validar recipe e versão;
2. validar saldo do bruto;
3. validar profissão/nível;
4. validar ouro/custos;
5. reservar/consumir insumos;
6. sortear resultado server-side;
7. criar o item OS pela mesma infraestrutura segura usada pelo inventário atual;
8. se a criação falhar, rollback de insumos/custo;
9. registrar audit/event;
10. publicar `LapidationCompletedEvent`.

Nunca confiar em `item_code`, chance ou resultado enviados pelo cliente.

## 6. Integração com Aging/Mix

O sistema legado já referencia muitos `sinOS*`/itens shelton. A lapidação deve terminar em um código `OSxxx` que o sistema atual reconhece.

Assim:

- não é necessário reescrever Aging para entender “minério”;
- não é necessário duplicar recipes de Aging;
- todas as restrições/efeitos existentes continuam aplicáveis;
- o novo sistema entra somente como **nova origem** para os mesmos itens.

## 7. Vendedor de sheltons

Existe `Sheltons.NPC.json` vendendo `OS103`, `OS104`, `OS105`.

Se a intenção de design for fazer mineração/lapidação ser a fonte principal dessas pedras, decidir explicitamente uma destas políticas:

- manter vendedor como catch-up/gold sink;
- limitar vendedor a tiers baixos;
- remover alguns sheltons do vendedor;
- rotacionar vendedor por season.

Não remover silenciosamente antes de medir impacto de economia.

## 8. XP da profissão

Separar:

- XP do personagem;
- XP da profissão Miner;
- quantidade do minério;
- qualidade/chance de produto lapidado.

Uma mudança no rendimento de minério não deve obrigatoriamente alterar XP de profissão.

## 9. Acceptance tests

- [ ] node exige profissão/nível correto;
- [ ] distância e respawn são server-authoritative;
- [ ] brute resource é persistido após relog;
- [ ] lapidação rejeita saldo insuficiente;
- [ ] operação simultânea não duplica materiais;
- [ ] falha ao criar OS faz rollback;
- [ ] item OS resultante funciona no Aging/Mix atual;
- [ ] logs permitem rastrear input/output por operação;
- [ ] economia por mapa incorpora valor esperado do minério.
