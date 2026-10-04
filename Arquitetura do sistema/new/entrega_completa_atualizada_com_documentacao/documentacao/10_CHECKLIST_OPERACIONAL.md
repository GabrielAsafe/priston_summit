# Checklist operacional e comandos de validação

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. Antes de qualquer deploy

- [ ] backup da DB;
- [ ] `git diff`/patch revisado;
- [ ] build canónico completo;
- [ ] validadores abaixo em `PASS`;
- [ ] cliente e servidor usam o mesmo conjunto de Content;
- [ ] SHA/BUILD.json do Content arquivado;
- [ ] plano de rollback definido.

## 2. CoinShop

```bash
python tools/audit_coinshop.py \
  --catalog Content/canonical/commerce/coinshop.json \
  --items Content/canonical/item_profiles
```

Esperado no snapshot documentado:

- 142 itens;
- 109 `coin`;
- 33 `time`;
- nenhum código inválido;
- `QW103` e `QW106` ausentes.

## 3. NPC shops / OpenItem

```bash
python tools/content_port/validate_compiled_item_materialization.py --source-root .
```

Enquanto os itens legados não forem resolvidos, o relatório pode listar como `FILTERED`:

`BC101 TW106 WD113 WD114 WN113 WN114 WQ106`

O objetivo final é zerar essa lista ou manter somente exceções formalmente aprovadas.

## 4. Build canónico

```bash
python tools/content_port/build_from_canonical.py \
  --canonical-text Content/canonical/text \
  --canonical-spawns Content/canonical/stage_spawns \
  --canonical-maps Content/canonical/maps \
  --canonical-start-points Content/canonical/stage_start_points \
  --canonical-fixed-npcs Content/canonical/fixed_npcs \
  --canonical-item-profiles Content/canonical/item_profiles \
  --out-root Content
```

Esperado: `CANONICAL_BUILD=PASS`.

## 5. Localização

```bash
python tools/audit_localization.py Content/canonical/text/localization/pt-BR.json
```

Depois validar também igualdade de chaves/placeholders entre `en`, `pt-BR`, `ko`, `zh-TW` com o validator do pipeline.

## 6. Drops

O último relatório fornecido indicou 22 arquivos e 0 referências ausentes. Rodar novamente o auditor de drops sempre que item profiles ou Drops mudarem.

## 7. DB migration da CoinShop

Antes da migration 046:

- [ ] confirmar que o deploy contém `coinshop.json`;
- [ ] exportar/arquivar `ItemList` e `ItemList_T` por segurança;
- [ ] confirmar leitura do JSON em staging;
- [ ] backup de `summit.sqlite3`.

Depois:

- [ ] `CoinShopDB__ItemList` ausente;
- [ ] `CoinShopDB__ItemList_T` ausente;
- [ ] tabelas de saldo/histórico continuam presentes;
- [ ] comprar com coin;
- [ ] comprar com time coin;
- [ ] reiniciar e repetir consulta/compra.

## 8. Asas

- [ ] procurar `QW103`/`QW106` em catálogos premium;
- [ ] validar recipes QW101-QW106;
- [ ] executar cadeia completa;
- [ ] validar persistência.

## 9. Eventos

Antes de ativar uma season:

- [ ] schema válido;
- [ ] datas UTC válidas e não sobrepostas sem intenção;
- [ ] rewards resolvem para itens existentes;
- [ ] localization keys existem em todos locales;
- [ ] instance/claim tables acessíveis;
- [ ] restart test;
- [ ] idempotency test.

## 10. Economia

Antes de alterar XP/dinheiro:

- [ ] baseline por mapa coletado;
- [ ] XP/h P50/P90;
- [ ] riqueza líquida/h P50/P90;
- [ ] TTK;
- [ ] party vs solo;
- [ ] efeito de evento/buffs separado;
- [ ] mudança aplicada em staging e medida novamente.

## 11. Companions

- [ ] cada buff tem target policy;
- [ ] owner dentro/fora range;
- [ ] caster incluído/excluído;
- [ ] outros companions;
- [ ] dead target;
- [ ] recast/stacking;
- [ ] relog/teleport.

## 12. Mineração/lapidação

- [ ] mining persistence;
- [ ] profession requirement;
- [ ] raw resource debit atómico;
- [ ] gold debit atómico;
- [ ] OS item grant;
- [ ] rollback em falha;
- [ ] Aging/Mix aceita o OS;
- [ ] logs/audit;
- [ ] valor do minério incorporado à economia do mapa.

## 13. Runtime localization smoke

Testar pelo menos:

- [ ] login/character select;
- [ ] sequência inicial/pardal;
- [ ] Amanda;
- [ ] Ricarten NPC names/dialogs;
- [ ] vendedor normal;
- [ ] CoinShop e Time CoinShop;
- [ ] quest/craft de asa;
- [ ] eventos;
- [ ] mineração/lapidação;
- [ ] mensagens de erro e inventário cheio.
