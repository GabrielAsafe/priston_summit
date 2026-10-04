# Audit — diferenças client/server no catálogo de mapas

Foram encontradas **20 diferenças de primeiro nível** entre os 53 maps canônicos client/server.

Distribuição:
- `environment`: 1
- `legacy_orphan_warp_properties`: 2
- `limit_level`: 12
- `warp_gates`: 5

Isso não significa que todas são bugs: algumas diferenças podem ser deliberadamente server-authoritative. O problema é não existir hoje uma classificação declarativa que diga quais diferenças são permitidas.

## Diferenças detectadas

- field 20 `map.forever_fall_01` — `environment`
- field 34 `map.greedy` — `limit_level`
- field 35 `map.ice2` — `limit_level`
- field 36 `map.boss` — `limit_level`
- field 37 `map.lost` — `limit_level`
- field 38 `map.losttemple` — `limit_level`
- field 40 `map.dun_7` — `limit_level`
- field 40 `map.dun_7` — `warp_gates`
- field 41 `map.dun_8` — `warp_gates`
- field 43 `map.dun_9` — `limit_level`
- field 43 `map.dun_9` — `warp_gates`
- field 44 `map.mine_1` — `limit_level`
- field 44 `map.mine_1` — `warp_gates`
- field 46 `map.slab` — `limit_level`
- field 46 `map.slab` — `warp_gates`
- field 47 `map.ancientw` — `legacy_orphan_warp_properties`
- field 47 `map.ancientw` — `limit_level`
- field 48 `map.seaa` — `legacy_orphan_warp_properties`
- field 48 `map.seaa` — `limit_level`
- field 52 `map.dc1` — `limit_level`

Detalhes completos (valores client/server): `reports/map_client_server_differences.json`.

## Ação

Adicionar ao build uma allowlist/schema de paridade: `must_match`, `server_authoritative`, `profile_only`. Divergência não classificada deve falhar o build ou, no mínimo durante migração, gerar warning bloqueante no publish.
