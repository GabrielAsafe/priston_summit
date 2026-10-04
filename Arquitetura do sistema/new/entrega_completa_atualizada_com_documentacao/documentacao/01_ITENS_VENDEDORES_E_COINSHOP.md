# Itens, vendedores NPC e CoinShop

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. Existem dois sistemas de vendedor

### 1.1 CoinShop / Time CoinShop

O catálogo é carregado por `Shared/content/v1/commerce/CoinShopCatalog.h` a partir de:

`Content/canonical/commerce/coinshop.json`

Cada item tem:

```json
{
  "code": "ITEMCODE",
  "section": 1,
  "price": 100,
  "currency": "coin"
}
```

`currency` aceita o fluxo `coin` ou `time`; `section` deve estar entre 1 e 7; `price` deve ser maior que zero.

No servidor, `CoinShop.cpp` carrega `coin` e `CoinShop_T.cpp` carrega `time`. Na compra, o preço é revalidado contra o catálogo carregado, em vez de confiar no preço enviado pelo cliente.

### 1.2 Vendedor NPC normal

O inventário de NPC normal vem das NPC definitions:

`Content/canonical/text/resources/Servidor/GameServer/NPC/*.json`

As três listas relevantes são:

- `sell_weapon` — armas;
- `sell_defence` — armaduras/defesa;
- `sell_etc` — itens diversos.

Exemplo real: `toolspilai.NPC.json` vende códigos `TW101`, `TW102`, etc. `Sheltons.NPC.json` contém `OS103`, `OS104`, `OS105` em `sell_etc`.

`Content/canonical/fixed_npcs/...` define **onde** o NPC é colocado. Não é ali que se define o inventário da loja.

## 2. Como criar um item e associá-lo a um vendedor

### Passo A — materializar o item

Um código só pode ser vendido se existir no catálogo executável de itens. As definições canónicas de OpenItem ficam sob:

`Content/canonical/text/resources/{Servidor|Cliente}/.../OpenItem/`

Os perfis que limitam o que é materializável são:

- `Content/canonical/item_profiles/client.items.json`
- `Content/canonical/item_profiles/server.items.json`

Para um item novo, cliente e servidor precisam concordar sobre o código e a definição necessária. Não basta colocar o código na lista do vendedor.

### Passo B — escolher o vendedor

Para CoinShop, adicione a entrada em `coinshop.json`.

Para NPC normal, adicione o código à diretiva `sell_weapon`, `sell_defence` ou `sell_etc` do NPC apropriado.

### Passo C — atenção ao formato canónico atual dos NPCs

As NPC definitions atuais preservam `semantic.directives[].values` **e** os bytes legados em `raw_values_hex`. O compilador `tools/content_port/compile_definitions.py` renderiza as diretivas a partir de `raw_values_hex`.

Portanto, no estado atual do pipeline, alterar somente `semantic.fields.sell_etc` ou somente `values` não garante mudança no pack compilado. A alteração deve manter sincronizados:

1. `semantic.fields.<lista>`;
2. a `semantic.directives[]` da mesma chave;
3. `values`;
4. `raw_values_hex` correspondente.

Os códigos de item são ASCII; por exemplo, `TW118` vira `5457313138` em hexadecimal.

**Melhoria recomendada:** criar um pequeno authoring tool (`edit_npc_shop.py`) que receba `npc_id`, lista e item code, e atualize todas essas representações de forma atómica. Isso evita edição manual de hex.

### Passo D — recompilar packs

O build canónico completo é feito com `tools/content_port/build_from_canonical.py`. Exemplo a partir da raiz do source:

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

O comando recompila `definitions_client.scfg`, `definitions_server.scfg`, localização, mapas e outros packs e termina com `CANONICAL_BUILD=PASS` quando tudo é válido.

## 3. Como descobrir item de NPC que não carrega

Execute:

```bash
python tools/content_port/validate_compiled_item_materialization.py --source-root .
```

No snapshot auditado o resultado foi `PASS`, mas foram encontrados 7 tokens legados que não são materializáveis e, por isso, são filtrados do pack:

| Código | NPC/origem | Lista |
|---|---|---|
| `BC101` | `Bcn01.npc.json` | `sell_etc` |
| `TW106` | `toolspilai.NPC.json` | `sell_etc` |
| `TW106` | `toolsric.NPC.json` | `sell_etc` |
| `WD113` | `Donation Box.NPC.json` | `sell_weapon` |
| `WD114` | `Donation Box.NPC.json` | `sell_weapon` |
| `WN113` | `Donation Box.NPC.json` | `sell_weapon` |
| `WN114` | `Donation Box.NPC.json` | `sell_weapon` |
| `WQ106` | `premium.npc.json` | `sell_etc` |

A lista tem 7 **códigos únicos** e 8 ocorrências porque `TW106` está em dois NPCs.

### Correção A — remover referência obsoleta

Use quando o item não deve mais existir. Remova o código de todos os NPCs que o referenciam, mantendo `values` e `raw_values_hex` sincronizados, depois recompile.

### Correção B — materializar/criar o item

Use quando o item deve existir. Crie/corrija a definição OpenItem para cliente e servidor, faça o código entrar nos perfis canónicos apropriados, recompile e rode novamente o validator. O item só está resolvido quando não aparece em `filtered_shop_tokens` e existe no pack `definitions_server.scfg` como item normal materializável.

## 4. Como descobrir item da CoinShop que não carrega

Execute:

```bash
python tools/audit_coinshop.py \
  --catalog Content/canonical/commerce/coinshop.json \
  --items Content/canonical/item_profiles
```

Na base auditada:

- 144 registros originais da DB eram válidos em cliente e servidor;
- 0 códigos inválidos;
- 0 duplicados case-insensitive;
- 0 preço/seção inválidos;
- 142 ficaram no JSON após retirar `QW103` e `QW106`.

O runtime também foi protegido: se um código não resolver para `DefaultItems`, o servidor registra `CoinShop invalid item code: <code>` e ignora a entrada em vez de usar um ponteiro inválido.

## 5. DB versus JSON da CoinShop

A migration `Database/migrations/046_coinshop_catalog_to_json.sql` remove:

- `CoinShopDB__ItemList`
- `CoinShopDB__ItemList_T`

Ela **não** remove saldos ou histórico. Devem permanecer na DB:

- `UserInfo` / `UserInfo_T` (ou os namespaces físicos equivalentes);
- pagamentos pendentes;
- transações/histórico.

O princípio é: catálogo estático em JSON; dinheiro e fatos transacionais em DB.

## 6. Checklist para publicar um item novo

- [ ] código existe no cliente;
- [ ] código existe no servidor;
- [ ] item materializa no pack correspondente;
- [ ] nome/descrição possuem localização correta;
- [ ] preço de venda/atributos estão coerentes;
- [ ] vendedor correto foi alterado;
- [ ] build canónico passa;
- [ ] validator de NPC/CoinShop passa;
- [ ] abrir vendedor in-game e comprar/vender item;
- [ ] relogar e validar persistência/serial/checksum do item.
