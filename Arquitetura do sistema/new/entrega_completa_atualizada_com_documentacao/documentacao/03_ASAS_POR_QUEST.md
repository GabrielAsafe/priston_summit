# Asas por quest/craft em vez de créditos

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. Objetivo

Remover o atalho premium das asas e preservar a progressão original de quest/craft.

## 2. Estado atual confirmado

Na DB original havia duas asas vendidas em lojas premium:

| Código | Nome pt-BR | Loja | Seção | Preço original |
|---|---|---|---:|---:|
| `QW106` | Asa Imperial | coin | 4 | 500 |
| `QW103` | Asa de Ouro | time | 3 | 300 |

As duas foram excluídas de `Content/canonical/commerce/coinshop.json`. O catálogo migrado ficou com 142 itens.

## 3. Cadeia existente no source

`Shared/inventory/legacy/SinWingItemLegacyShared.inl` contém a progressão:

1. `GP101 + GP102 -> QW101`, custo 3.000;
2. `QW101 + GP102 + GP103 -> QW102`, custo 5.000;
3. `QW102 + GP103 + GP104 -> QW103`, custo 10.000;
4. `QW103 + GP105 + GP106 -> QW104`, custo 20.000;
5. `QW104 + GP106 + GP107 -> QW105`, custo 40.000;
6. `QW105 + GP107 + GP108 -> QW106`, custo 80.000.

No servidor, `sinSubMain.cpp` valida `NeedItemCode` e, quando a combinação é válida, define o resultado com `ChangeItemCode`.

## 4. O que deve e não deve ser alterado

### Manter

- receitas/cadeia `QW101..QW106`;
- validação server-side da transformação;
- ingredientes `GPxxx` necessários;
- criação do item resultante pelo caminho legado já existente.

### Remover/evitar

- `QW103` e `QW106` em CoinShop/Time CoinShop;
- qualquer novo atalho premium que permita pular a cadeia sem decisão explícita de design;
- duplicação da mesma asa em reward de evento sem verificar impacto da progressão.

## 5. Teste de aceitação

Executar em servidor de teste com personagem limpo:

- [ ] CoinShop não lista `QW103` nem `QW106`;
- [ ] Time CoinShop não lista `QW103` nem `QW106`;
- [ ] ingredientes GP podem ser obtidos pelo fluxo esperado;
- [ ] cada transformação QW produz exatamente a próxima asa;
- [ ] custo é debitado uma única vez;
- [ ] falha de receita não consome ingredientes indevidamente;
- [ ] relog preserva a asa;
- [ ] ícone/modelo/nome estão corretos no locale ativo;
- [ ] `QW103` e `QW106` continuam materializáveis apesar de não estarem na CoinShop.

## 6. Observação de design

O mecanismo existente é tecnicamente uma cadeia de craft/quest de asas. Se o objetivo final exigir também **estado narrativo de quest** (pré-requisitos, diálogo, flags de conclusão), isso deve ser ligado ao sistema de quests sem substituir a tabela de receitas. A loja premium não deve voltar a ser a fonte das asas.
