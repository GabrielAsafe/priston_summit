# Balanceamento de XP, dinheiro e valor por mapa

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. O problema deve ser medido por mapa, não só por monstro

O jogador sente a economia como **resultado por tempo de jogo**. Portanto, “XP do monstro” e “ouro do monstro” isoladamente não bastam.

Para cada mapa, medir:

- XP/hora;
- ouro bruto/hora;
- valor NPC dos drops/hora;
- valor esperado dos materiais/sheltons/hora;
- consumíveis gastos/hora;
- tempo médio para matar;
- mortes/risco;
- densidade e respawn;
- nível real dos jogadores que usam o mapa.

A métrica económica principal deve ser:

`riqueza líquida/h = ouro bruto + valor esperado de loot - consumíveis/custos`

## 2. Onde o XP está hoje

O loader de monstros lê `Experience` para `smMonsterInfo.GetExp`. O fluxo de party em `PartySever.cpp` aplica, entre outros:

- XP base do monstro;
- diferença de nível via `srGetTotalExp`;
- taxa de party;
- buffs/prime items;
- modo de XP por horário;
- modificador de covenant/progression;
- `AddExp` no jogador.

Além disso, a tabela de party ainda é carregada de `Configs//ExpManager.ini`.

Portanto, um rebalanceamento de XP precisa distinguir:

1. XP base do monstro;
2. multiplicadores globais;
3. multiplicadores de party;
4. buffs/eventos;
5. modificadores V2.

Não compense um multiplicador quebrado alterando todos os monstros.

## 3. Mapas canónicos

A base auditada possui 53 mapas em `Content/canonical/maps/client.maps.json` e 53 em `server.maps.json`. Os mapas possuem dados de level limit, stage, ambiente, gates etc., mas **não possuem hoje uma seção de economia**.

## 4. Modelo recomendado

Criar um catálogo canónico versionado, por exemplo:

`Content/canonical/economy/maps.json`

Exemplo de esquema proposto:

```json
{
  "schema": "summit.map-economy/v1",
  "maps": [
    {
      "map": "map.fore_3",
      "recommended_level": {"min": 1, "max": 15},
      "targets": {
        "xp_per_hour": 100000,
        "raw_gold_per_hour": 15000,
        "loot_vendor_value_per_hour": 20000,
        "material_expected_value_per_hour": 5000
      },
      "modifiers": {
        "xp": 1.0,
        "gold": 1.0,
        "drop_value": 1.0
      }
    }
  ]
}
```

Os números acima são apenas exemplo de estrutura, **não são valores recomendados de balanceamento**.

## 5. Processo de balanceamento

### Fase 1 — baseline

Coletar pelo menos sessões representativas por faixa de nível/mapa. Registrar por jogador:

- mapa e duração;
- kills por tipo;
- XP recebido;
- ouro recebido;
- item code e quantidade de cada drop;
- preço NPC/valor de referência de cada drop;
- consumíveis utilizados.

### Fase 2 — definir curva alvo

Definir quanto tempo deve levar cada faixa de nível e quanto poder de compra o jogador deve ganhar. A curva deve ser monotónica e evitar saltos bruscos ao trocar de mapa.

### Fase 3 — ajustar a causa correta

- XP/h baixo com TTK correto: ajustar XP base/multiplicador do mapa;
- XP/h baixo por TTK alto: revisar HP/defesa/nível dos mobs antes de inflar XP;
- riqueza alta por item raro: ajustar drop/valor do item, não só ouro;
- party supera solo muito além do desejado: revisar `ExpManager.ini`/party rate;
- evento multiplica demais: revisar multiplicador de evento, não conteúdo base.

### Fase 4 — validar

Comparar P50 e P90 de XP/h e riqueza/h por mapa. Não usar somente média: outliers de build/party podem esconder problemas.

## 6. “Valor das coisas em cada mapa”

O valor de um mapa deve considerar quatro canais:

1. **ouro direto**;
2. **itens vendáveis ao NPC**;
3. **materiais úteis em crafting/aging/lapidação**;
4. **itens raros com valor económico indireto**.

Ao integrar mineração, o minério bruto e os sheltons lapidados entram nesse cálculo. Caso contrário, um mapa de mineração pode parecer pobre em ouro mas ser o melhor mapa do jogo em valor total.

## 7. Regras de segurança de economia

- servidor é autoridade para rewards;
- valores não devem vir do cliente;
- qualquer conversão minério -> pedra deve ser atómica;
- eventos sazonais devem ter multiplicadores auditáveis;
- mudanças de economia devem ter versão/data e rollback simples;
- evitar hardcodes de multiplicador por map ID no C++; usar conteúdo canónico.
