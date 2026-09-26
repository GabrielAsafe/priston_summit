# Companion V43 — Loot runtime

## Objetivo

V43 adiciona o primeiro runtime completo de **Companion Loot** ao snapshot assistant-side, sem criar inventário próprio para o companion. A decisão de procurar/mover-se até um drop é server-authoritative e o recebimento final reutiliza o contrato legado do próprio jogador (`TRANS_ITEMINFO` + `OPCODE_PUTITEM` + security code).

O recurso é **opt-in**. Sem configuração, `LootEnabled=0` mantém exatamente o comportamento V42.

## Regras de decisão

O companion só entra em loot depois de lifecycle, resource/consumable, buffs/heal e retreat. Em modo Aggressive ele não troca combate por loot quando existe um hostile visível; em Follow/Passive pode recolher quando não há target ativo. O alvo de loot possui timeout, scan interval e pickup cooldown.

A procura usa o `STG_AREA` real do companion e respeita:

- ownership (`dwExclusiveOwnerCode` vazio ou igual ao dono);
- `LootRadius` a partir do companion;
- `LootOwnerLeash` a partir do jogador dono;
- `dwCreateTime`/delay de visibilidade do drop;
- `dwLoseTime` para não perseguir item prestes a expirar;
- reserva in-memory entre companions geridos, evitando dois dos até 4 companions do runtime perseguirem o mesmo `STG_ITEMS`;
- `LootMaxChaseMs`, que abandona perseguições longas/stuck.

## Categorias e filtros

As opções abaixo são lidas de `.//Configs//Companion.ini`, seção `[Companion]`. Valores mostrados são defaults compilados:

```ini
[Companion]
LootEnabled=0
LootRadius=260
LootOwnerLeash=520
LootPickupDistance=36
LootScanMs=350
LootPickupCooldownMs=300
LootMaxChaseMs=4500
LootSpawnDelayMs=650

LootGold=1
LootCrystals=1
LootQuestItems=0
LootConsumables=1
LootEquipment=1
LootOther=0
LootUnique=1
LootMinimumPrice=0
LootMinimumDropLevel=0
```

Classificação V43:

- Gold/currency: `sinGG1|sin01` e `sinGG1|sin05`;
- Crystals/core family: `sinEC1`;
- Quest: `ITEM_KIND_QUEST`, `ITEM_KIND_QUEST_WEAPON` ou família `sinQT1`;
- Consumables: família principal `0x04` (mesma semântica de potion do legado);
- Equipment: famílias principais `0x01`, `0x02`, `0x03`;
- Other: tudo o que não cair nas anteriores.

`LootMinimumDropLevel` usa `sITEMINFO.Level`, não o campo `STG_ITEMS.Level` (que no legado é bookkeeping do stage). `LootMinimumPrice` usa `sITEMINFO.Price` como filtro de valor mínimo.

Quest loot fica desligado por default. `sinQT136` é sempre excluído do V43 porque o Tier 5 / Rank5 continua fora do contrato atual e esse item possui side effects de token/quest específicos.

## Pickup e anti-loss/anti-dup

`rsCompanionPickupStageItem()` valida novamente o `STG_ITEMS` e ownership no momento do pickup. Ele não grava diretamente em qualquer inventário do companion. Em vez disso:

1. preserva o splitter server-authoritative de party gold;
2. preserva o caso especial SoD;
3. monta o mesmo `TRANS_ITEMINFO` usado pelo pickup manual;
4. gera security code por `rsRegist_ItemSecCode`;
5. envia ao socket do owner;
6. mantém high-item logging;
7. remove o `STG_ITEMS` original pelo fluxo do stage.

No cliente, o contrato já existente processa `OPCODE_PUTITEM`/`OPCODE_GETITEM`. Quando o inventário está cheio ou o peso não permite armazenar, `sinSetInvenItem()` usa `AutoSetInvenItem()` e, em falha, `sinThrowItemToFeild()`. Portanto V43 herda o mesmo comportamento do pickup manual: o item volta ao chão, em vez de existir um caminho paralelo silencioso que o perde.

## Concorrência

A reserva de target é global ao runtime Companion protegido pelo mutex já existente de `CompanionSystem`. Isso impede concorrência entre os 4 companions geridos no mesmo processo. O pickup valida `state` e ownership novamente no instante final, então um item removido por outro fluxo deixa de ser elegível e a reserva é descartada.

A concorrência real com vários jogadores/party, reconexão e stress com 4 companions ainda precisa de soak no servidor real.

## Debug

Com `SUMMIT_COMPANION_DEBUG=1`, V43 acrescenta:

- `LOOT_TARGET ...` quando um companion reserva um drop;
- `LOOT_PICKUP ... result=...` ao tentar concluir o pickup.

## Validação assistant-side

Validator dedicado:

```bash
python3 SourceCode/tools/validate_companion_loot_v43.py
```

Ele verifica ownership, reserva, leash/filtros, defaults opt-in, exclusão Rank5, bridge de security code e o fallback legado de inventory full.

Como nos handoffs anteriores, validator estático não substitui compile/runtime. Ainda precisam ser executados na SRC real reconciliada:

- clean build/link MinGW i686 cliente e servidor;
- deploy real;
- smoke Wine;
- teste de inventário cheio/peso cheio;
- party gold;
- drops exclusivos e públicos;
- map transition/dismiss durante chase;
- 4 companions simultâneos disputando muitos drops;
- soak/performance/concurrency.
