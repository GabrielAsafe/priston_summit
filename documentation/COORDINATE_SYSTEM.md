# Sistema de coordenadas — map content vs fixed NPC placement

## Conclusão

As coordenadas representam o mesmo mundo, mas há duas escalas legadas.

`FLOATNS = 8`, portanto estruturas de personagem/NPC usam frequentemente fixed-point 24.8:

```text
internal = world << 8 = world * 256
world = internal >> 8
```

Map `center`, `start_points` e gates são authored na escala world/debug. Fixed NPC placements atuais preservam a escala internal e são copiados diretamente por `STG_AREA::LoadCharInfoFixed`.

Por isso um placement pode parecer centenas de vezes maior que o valor exibido pelo overlay.

## Direção adotada

Todo authoring novo deve usar world coordinates iguais às mostradas no debug overlay. O compiler converte fixed NPC XYZ para ×256 ao materializar o formato legado.

Rotação `ax/ay/az` não é coordenada linear e não deve receber `/256` ou `*256`.

Bug report: `../bugreports/BR002_NPC_COORDINATE_SPACE.md`.
