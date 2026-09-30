# 5. CoinShop, TimeShop e economia

## 5.1 Estrutura

### CoinShop

Catálogo:

```text
CoinShopDB__ItemList
```

```text
szItemCode TEXT
ItemSection INTEGER
Price INTEGER
```

Saldo:

```text
CoinShopDB__UserInfo
```

```text
UserID
UserCoin
LastChange
```

### TimeShop

Catálogo:

```text
CoinShopDB__ItemList_T
```

Saldo:

```text
CoinShopDB__UserInfo_T
```

```text
UserID
UserMinutos
LastChange
```

## 5.2 Estado importado

Após importação:

```text
ItemList      = 110 itens
ItemList_T    = 34 itens
UserInfo      = 34 contas
UserInfo_T    = 34 contas
```

Isso corrigiu o sintoma onde as lojas apareciam vazias e os saldos tinham desaparecido.

## 5.3 Identidade do saldo

Migration 014 adiciona UNIQUE:

```text
CoinShopDB__UserInfo(UserID)
CoinShopDB__UserInfo_T(UserID)
```

Isso é importante porque o código assume **uma row de saldo por conta**.

Não remover esses índices.

## 5.4 Criação automática de saldo

Se uma conta não tem row, o runtime pode criar:

```sql
INSERT INTO UserInfo (UserID, UserCoin, LastChange)
SELECT ?, 0, datetime('now','localtime')
WHERE NOT EXISTS(...);
```

TimeShop faz o mesmo com `UserMinutos=0`.

Portanto:

- criar uma conta não exige pré-criar saldo;
- uma conta nova pode começar em 0;
- importar saldos históricos deve ocorrer antes de o jogador usar a loja, para evitar confusão operacional.

## 5.5 Adicionar item ao CoinShop

Exemplo:

```sql
INSERT INTO CoinShopDB__ItemList
(szItemCode, ItemSection, Price)
VALUES ('ITEM_CODE', 1, 100);
```

Antes disso verificar:

1. `szItemCode` existe no catálogo real de items do jogo;
2. `ItemSection` é uma seção que o cliente exibe;
3. preço é inteiro e na unidade correta;
4. o código de entrega suporta aquele item;
5. não existe duplicidade ambígua.

Consulta:

```sql
SELECT *
FROM CoinShopDB__ItemList
WHERE szItemCode='ITEM_CODE';
```

## 5.6 Adicionar item ao TimeShop

Mesmo conceito:

```sql
INSERT INTO CoinShopDB__ItemList_T
(szItemCode, ItemSection, Price)
VALUES (...);
```

O `Price` representa o custo em `UserMinutos`.

## 5.7 Atualizar saldo

Preferir o código/admin command correspondente quando existir.

Se for necessário manutenção manual, parar servidor e fazer backup.

Coin:

```sql
UPDATE CoinShopDB__UserInfo
SET UserCoin = UserCoin + ?,
    LastChange = datetime('now','localtime')
WHERE UserID = ?;
```

Tempo:

```sql
UPDATE CoinShopDB__UserInfo_T
SET UserMinutos = UserMinutos + ?,
    LastChange = datetime('now','localtime')
WHERE UserID = ?;
```

## 5.8 GM commands relacionados

O inventário administrativo inclui:

```text
/getcoins <char>
/addcoins <char> <qtd>
/removecoins <char> <qtd>
/settimelogado_coin ...
/setcoinlogado ...
```

Usar esses comandos quando eles representarem a regra de negócio desejada, em vez de SQL direto.

## 5.9 PendingPayments

Tabela importada:

```text
CoinShopDB__PendingPayments
```

Durante a integração foi detectado que dados históricos têm:

```text
CompletedDate = NULL
```

Isso é semanticamente correto para pagamento ainda não concluído.

O runtime atual foi ajustado para permitir:

```text
CompletedDate NULL
```

Não preencher com data fictícia.

Novos inserts devem:

- usar `NULL` enquanto pendente;
- preencher a data somente quando o fluxo realmente concluir.

## 5.10 Riscos econômicos

### Duplicação de saldo

Se perder o UNIQUE de `UserID`, podem existir múltiplas rows e o runtime pode ler uma row arbitrária.

### Compra não atômica

Fluxos antigos podem executar SELECT saldo, UPDATE saldo e entrega em etapas distintas. Em feature nova, preferir transação explícita.

### Código de item inválido

Saldo pode ser descontado antes de falhar a criação/entrega.

### Overflow/valores negativos

Validar quantidade e saldo no C++, não confiar apenas no SQLite.

### Conta por character vs account

Os saldos CoinShop são por:

```text
UserID/account
```

não por personagem.

## 5.11 Relação com quests

`QuestDB__Rewards` possui:

```text
Coin
TCoin
```

Uma quest pode creditar CoinShop/TimeShop. Ao criar reward nova, testar as duas integrações.
