# BUG — tornar o catálogo de mapas realmente content-driven

## Título

`[MAP][CONTENT] Remover limite fixo de 53 mapas, liberar field 53 e dimensionar StageArea pelo conteúdo`

## Problema

O catálogo já é authored em JSON e compilado para `maps_client.smap` / `maps_server.smap`, mas o runtime ainda fixa o contrato histórico:

- static fields: `0..52`;
- `static_field_count = 53`;
- field `53` reservado ao custom field do `smConfig`;
- `FIELD_MAX = 256`;
- `STAGE_AREA_MAX = 64`.

Assim, adicionar o próximo mapa pelo canonical não basta.

## Causa raiz

### Client/server field loaders

`Shared/world/field/field_legacy_client.inl` e `field_legacy_server.inl` passam literalmente `53, 53` para `LoadCompiledMapsIntoLegacyFields(...)` e chamam `FinalizeLegacyFieldCatalog(53)`.

### LegacyMapAdapter

O caller fornece `expectedDynamicFieldCode` e `expectedStaticFieldCount`. Isso inverte a autoridade: o executável dita a quantidade ao pack em vez de o pack informar o runtime.

### Schema/semanticizer

O schema de maps e a importação histórica ainda codificam 53/0..52/field53.

### StageArea

Mesmo removendo 53, o servidor ainda aloca `new STG_AREA[STAGE_AREA_MAX]` com `STAGE_AREA_MAX=64` e vários adapters usam esse limite para validar map id.

## Comportamento desejado

Regra única:

```text
static_field_count = quantidade de maps estáticos do pack
dynamic_field_code = static_field_count
```

Exemplos:

```text
maps 0..52  -> static=53, custom=53   (compatibilidade atual)
maps 0..53  -> static=54, custom=54
maps 0..54  -> static=55, custom=55
```

O custom field continua existindo, mas sempre ocupa o primeiro slot livre. O número 53 deixa de ter significado especial.

## Mudanças obrigatórias

### 1. Loader

Retirar `expectedStaticFieldCount/expectedDynamicFieldCode` da API pública. O loader deve devolver `staticFieldCount` e `dynamicFieldCode` obtidos do SMAP.

O client e o server passam o resultado para `FinalizeLegacyFieldCatalog(staticFieldCount)`.

### 2. Bounds

Com custom field habilitado, exigir:

```text
static_field_count < FIELD_MAX
```

Adicionar bounds check antes de `sField[cnt]` em `FinalizeLegacyFieldCatalog`.

### 3. Schema

Remover:

- `map_count const 53`;
- `minItems/maxItems 53`;
- `legacy_field_code maximum 52`;
- dynamic field `const 53`.

Enquanto `FIELD_MAX=256` e existir um custom field anexado, permitir no máximo 255 static fields (`0..254`).

### 4. Códigos contíguos

Enquanto a compatibility layer continuar usando `sField[fieldCode]`, exigir:

```text
legacy_field_code == 0..N-1 sem buracos
```

`compile_maps.py` deve validar isso.

### 5. Dynamic field no canonical

Em vez de armazenar número manual:

```json
{"legacy_dynamic_field":{"strategy":"append_after_static"}}
```

O compiler resolve o code.

### 6. `STAGE_AREA_MAX=64`

Não substituir cegamente 64 por 256. `STG_AREA` é grande.

Após `FieldCount` ser conhecido:

```cpp
StageAreaCount = FieldCount;
StageArea = new STG_AREA[StageAreaCount];
```

Reordenar bootstrap se necessário para carregar map catalog antes da alocação.

Trocar validações de `Area < STAGE_AREA_MAX` por `Area < StageAreaCount` quando acessam `StageArea[Area]`.

Auditar pelo menos:

- `Shared/server/legacy_runtime/OnSever_legacy_server.inl`;
- LegacyWorldAdapter;
- LegacyStageAdapter;
- loot/combat/event adapters que indexam StageArea.

### 7. `srReadStage`

Parar de descobrir quantidade por sentinel/string vazia. Iterar exatamente `0..FieldCount-1`.

### 8. PT Asset Tool

Remover conceitos especiais:

- `FIELD_53_LEGACY_READ_ONLY`;
- `dynamic_field_53`;
- `canonical:legacy:field53`.

A UI deve mostrar:

```text
Static maps: N
Static field range: 0..N-1
Legacy custom field: N (runtime only)
```

Novo mapa deve sugerir `max(field_code)+1`.

## SMAP

SMAP v1 já contém `static_field_count` e `dynamic_field_code`; não é necessário mudar formato binário se a semântica continuar compatível.

## Testes obrigatórios

1. catálogo atual: static=53/custom=53, sem regressão;
2. novo field53: static=54/custom=54;
3. custom field não sobrescreve field53;
4. catálogo sintético >=70 mapas carrega sem truncar em 64;
5. warp 52 -> 53 -> 3 funciona para jogador normal;
6. relog em field53 preserva área;
7. client/server recusam pack inconsistente;
8. PT Asset Tool cria/builda/publica field53 sem erro especial;
9. schema rejeita gaps como `0..52,54`.

## Critérios de aceite

O ticket termina quando adicionar um mapa novo requer somente conteúdo/assets + build/publish, sem editar C++ para aumentar contadores.
