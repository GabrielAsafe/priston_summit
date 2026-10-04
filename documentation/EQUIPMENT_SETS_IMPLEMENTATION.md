# Bônus por conjunto de equipamento

## Estado legado

Há dois sets hardcoded no client:

- OR125 x2 + OA125 (Valento/Kelvezu);
- OR126 x2 + OA126 (Shy/Tulla).

O client conta peças e altera defesa/vida. Os valores vêm do servidor, mas a decisão de ativação está client-side.

Isso não deve ser replicado para conteúdo novo.

## Conteúdo canônico

`Content/canonical/equipment_sets/equipment_sets.json` contém a migração explícita dos dois sets existentes e serve de base para novos sets.

Os nomes foram reconciliados com o catálogo:

- OR125 = Anel do Valento;
- OA125 = Colar do Kelvezu;
- OR126 = Anel do Shy;
- OA126 = Colar do Tulla.

## Serviço incluído

`patches/equipment_sets/EquipmentSetService.*` implementa:

- registro/validação do set;
- contagem por item/cópias;
- thresholds;
- `requireCompleteSet`;
- geração de `GameplayModifier` usando a abstração já utilizada por Covenants;
- reconciliação idempotente por source key `equipment-set:<id>`.

Smoke test compilado neste bundle: PASS.

## Integração server-authoritative

No servidor, após qualquer mudança de equipamento:

1. obter os item IDs realmente equipados;
2. `EquipmentSetService::ReconcileModifiers`;
3. recalcular os stats server-side;
4. enviar snapshot/stat update ao client.

O client deve apenas exibir set/bonus e os stats resultantes.

Para o requisito “1 peça = 0, todas = X”, criar somente um bonus com:

```json
{
  "required_piece_count": 4,
  "require_complete_set": true,
  "modifiers": [...]
}
```

Não criar thresholds parciais.
