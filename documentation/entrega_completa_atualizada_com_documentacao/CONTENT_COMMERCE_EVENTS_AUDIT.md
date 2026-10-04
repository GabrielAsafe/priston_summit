# Auditoria e refactor — Content, CoinShop, Drops, localização e backlog

## Estado desta entrega

Esta revisão usa os quatro pacotes originais e a base adicional `Database(1).zip`.

### CoinShop

A base adicional contém o catálogo real:

- `CoinShopDB__ItemList`: **110** linhas.
- `CoinShopDB__ItemList_T`: **34** linhas.
- Total na DB: **144** itens.
- Todos os 144 códigos existem nos perfis canónicos de itens do cliente e do servidor.
- Não há códigos duplicados por moeda, preço <= 0 ou seção fora de `1..7`.

O catálogo estático foi migrado para `Content/canonical/commerce/coinshop.json`.

Duas entradas não foram levadas para a loja:

- `QW106` — **Asa Imperial** — coin / seção 4 / preço 500.
- `QW103` — **Asa de Ouro** — time / seção 3 / preço 300.

O motivo é o objetivo de voltar a obter asas por quest. O source já possui o fluxo legado de Wing Quest/Craft e a sequência de asas em `Shared/inventory/legacy/SinWingItemLegacyShared.inl`, incluindo `QW103` e `QW106`. Portanto, a alteração necessária na CoinShop é retirar essas asas da venda, sem apagar o sistema de quest/craft existente.

Resultado do JSON: **142 itens** (`109 coin` + `33 time`).

### O que sai da DB

A migration `046_coinshop_catalog_to_json.sql` remove apenas:

- `CoinShopDB__ItemList`
- `CoinShopDB__ItemList_T`

`CoinShopDB__UserInfo` e `CoinShopDB__UserInfo_T` permanecem em DB porque são saldo/tempo mutáveis por jogador. `PendingPayments` e `Transactions` também são estado transacional e não devem ir para JSON.

A entrega inclui uma cópia migrada de `summit.sqlite3` com apenas as duas tabelas estáticas removidas. Os backups históricos não são alterados.

## Como associar um item a um vendedor

### CoinShop

1. O código do item precisa existir em `Content/canonical/item_profiles/client.items.json` e `server.items.json`.
2. Adicione uma entrada em `Content/canonical/commerce/coinshop.json`:
   `{"code":"CODIGO","section":1,"price":100,"currency":"coin"}`.
3. `currency` pode ser `coin` ou `time`.
4. `section` deve estar entre 1 e 7.
5. Rode:
   `python tools/audit_coinshop.py --catalog Content/canonical/commerce/coinshop.json --items Content/canonical/item_profiles`
6. Reinicie/recarregue a CoinShop pelo caminho que já chama `cCoinShop.LoadItems()` / `cCoinShop_T.LoadItems()`.

### Vendedores NPC normais

A CoinShop é um subsistema separado. NPCs normais dependem da definição do NPC/shop do runtime legado. Para um item novo aparecer neles, primeiro ele deve existir no catálogo de itens e depois precisa ser referenciado pela configuração/lista do vendedor correspondente. Não use `coinshop.json` para lojas normais.

## Como descobrir itens que não carregam na CoinShop

Use `tools/audit_coinshop.py`.

Ele verifica:

- existência no perfil de itens do cliente;
- existência no perfil de itens do servidor;
- códigos duplicados;
- moeda inválida;
- seção fora de 1..7;
- preço inválido.

Além disso, o patch protege `RecvGetItemShop` contra `lpDefItem == nullptr`. Em vez de fazer `memcpy` de um item inexistente, ele registra `CoinShop invalid item code: <code>` e ignora a entrada.

Na DB atual **nenhum dos 144 itens está ausente dos perfis client/server**. Se um item ainda não aparecer visualmente, a próxima checagem deve ser a definição runtime (`DefaultItems`) e os assets do item, não o código da CoinShop.

## Drops

Foram verificados os **22 arquivos `.inf`** recebidos.

Resultado atual:

- **1.985 códigos canónicos conhecidos** no perfil de itens.
- **0 referências ausentes** detectadas nos drops.

Validação:
`python tools/audit_drops.py --drops <pasta Drops> --items Content/canonical/item_profiles`

## Localização

O `pt-BR.json` recebido continha entradas com script coreano e mojibake/encoding quebrado. As ocorrências detectadas pelo auditor PT-BR foram corrigidas.

Resultado:
`audit_localization.py` => **0 problemas no pt-BR** segundo as regras de script estrangeiro e mojibake.

Amanda já estava em português no catálogo fornecido.

O Content canónico presente possui `en`, `pt-BR`, `ko` e `zh-TW`. Não há catálogo espanhol canónico ativo no material recebido. Portanto não foi criado um espanhol artificialmente.

Observação: a limpeza completa de todas as línguas exige revisão semântica por língua; o patch automatizado desta entrega fecha o PT-BR solicitado e fornece auditor para evitar regressões.

## XP, dinheiro e valor por mapa

A proposta de balanceamento é centralizar os alvos de economia por mapa em conteúdo canónico, em vez de multiplicadores espalhados no C++.

Para cada mapa, registrar pelo menos:

- faixa de nível;
- XP/h alvo;
- gold/h alvo;
- custo médio de consumíveis;
- custo de aging/mix;
- disponibilidade e valor esperado de sheltons;
- tempo desejado até o próximo marco de progressão.

Depois os multiplicadores devem ser derivados destes alvos. Isso permite balancear mapas entre si sem ajustar XP e dinheiro isoladamente.

## Eventos por season

Separação recomendada:

- JSON canónico: definição da season e dos eventos.
- DB: apenas estado operacional e por jogador.

Estrutura sugerida:

- `Content/canonical/events/seasons.json`
- `Content/canonical/events/<event_id>.json`

Campos úteis: `season_id`, janela de ativação, mapas, spawns, regras, recompensas, flags e referências de localização.

Persistir em DB apenas progresso, cooldowns, claims, rankings ou auditoria. Assim uma season pode ser versionada/deployada como conteúdo sem migrations de dados estáticos.

## Companions

A decisão de buff deve ser expressa como política de targeting e não como exceção específica em cada skill.

Modelo sugerido:

- `owner`
- `self_companion`
- `party_companions`
- `owner_and_companions`

O source já tem implementação e testes específicos de companions. Depois de escolher a regra de design, ela deve entrar no catálogo/config de skills e nos testes de targeting.

## Mineração, lapidação, sheltons e aging

Fluxo recomendado:

`mineração -> minério bruto -> lapidação -> pedra/shelton -> aging/mix`

A migration `044_gathering_mining_runtime.sql` já cria persistência para mineração. A integração deve reutilizar o inventário e os contratos existentes de sheltons/aging, evitando criar uma segunda família incompatível de pedras.

A mineração controla aquisição do bruto; a lapidação converte bruto em materiais utilizáveis, podendo aplicar tier/qualidade/chance; o aging continua consumindo os sheltons existentes.

## Hardcoded text no source

A auditoria encontrou milhares de literais C/C++, mas nem todo literal é texto de UI: SQL, paths, formatos, nomes de assets, protocolos e identificadores também aparecem como strings.

Por isso a entrega inclui `tools/audit_hardcoded_text.py` para gerar candidatos, mas não faz substituição cega de todos eles.

Fluxo seguro para cada texto realmente visível:

1. Confirmar que é texto apresentado ao jogador.
2. Criar/usar uma chave no JSON de localização.
3. Garantir a chave para cada locale suportado.
4. Preservar placeholders `%s`, `%d`, etc.
5. Trocar o literal pela consulta do runtime de localização.
6. Rodar build/testes.

Os relatórios `hardcoded_client.json` e `hardcoded_server.json` são o backlog inicial dessa migração.

## Arquivos de auditoria desta entrega

- `coinshop_audit.json`: validação do catálogo real e exclusões das asas.
- `coinshop_catalog_export.json` / `.csv`: export completo das 144 linhas originais com nomes e status.
- `drop_audit.json`: referências de drops ausentes.
- `pt_audit.json`: problemas detectados no PT-BR.
- `hardcoded_client.json` e `hardcoded_server.json`: candidatos a texto hardcoded.
