# SUMMIT — Contrato atualizado: Content / DAT / SQLite

Decisão funcional confirmada em 2026-11-02 pelo autor do projeto.

## Fontes de verdade

| Informação | Autoridade |
|---|---|
| Identidade, nome, configuração, elementos do jogo | `SourceCode/Content/canonical/` — JSON e recursos canónicos compilados |
| Receita, ingredientes, quantidades, produto, custos, profissão, regras de ativação | `crafting_v2/all_item_recipes_424.json` (JSON) |
| Estado de instâncias físicas na mochila, equipamento, identidade e quantidade de itens existentes em DAT | Registo DAT autoritativo |
| Recursos/saldos que **não** existem no DAT e são explicitamente virtuais | SQLite (somente estado do jogador, nunca definição da receita) |
| Operações interrompidas, ACK, idempotência, progresso técnico | Journal/recibos de transação, **não** réplica dos saldos DAT |

**Não guardar a mesma quantidade de material físico simultaneamente em DAT e SQLite.**

## Consequências para runtime

1. `/craft plan`, `/craft make`, o botão da UI e a validação no servidor devem ler a **mesma** projeção canónica JSON 424A/425A. O cliente nunca é autoridade do custo.
2. As 57 receitas históricas na migração 049 são artefactos de migração, não fonte atual de receita. A consulta do handler SQLite legado deve ser substituída por acesso ao registo 424A. **Este patch 428A NÃO realiza essa substituição.**
3. Não apagar imediatamente tabelas/migrações: o runtime antigo ainda contém leitores. Primeiro migrar leitores, validar comparações e desligar acessos; só depois retirar/arquivar definições obsoletas.
4. Cada ingrediente é classificado por identidade como `DAT_PHYSICAL` ou `SQLITE_VIRTUAL` com contrato explícito e sem sobreposição. Materiais recolhidos e existentes na mochila devem ser `DAT_PHYSICAL`.
5. Receitas `enabled=false` / quantidades zero continuam inativas. Não é possível fabricar gratuitamente por ter quantidades por definir.
6. Ao fabricar: reservar e verificar as instâncias DAT reais, validar custo JSON, criar item com identidade nativa, aplicar operação recuperável no DAT, persistir apenas efeitos virtuais em SQLite com idempotência e devolver ACK ao cliente. Não retirar a trava física enquanto este fluxo não estiver certificado.
7. Os nomes exibidos devem resolver pela identidade canónica; nunca usar o `ItemName` histórico como prova do item. Não reescrever DAT para corrigir só texto.

## Estado atual e próximos passos

- Nodes 3D (26): utilizador confirmou interação, drops, cooldown, respawn e relogin a funcionar. Registar como funcional, sem repetir patches de nodes.
- 419B/427B: Crafting mostra nomes CI corretos. O inventário V2 ainda pode usar `sItemInfo.ItemName`; 428A corrige essa camada em modo read-only, sem tocar no DAT.
- 423A `/craft check` e `SQLiteProfessionCraftRuntime378.h` ainda leem definições da migração SQLite: **dívida de runtime**.
- 428A não cria equipamento, não consome itens e não elimina dados SQL. Implementar transação DAT e convergência de receitas como operações separadas e verificáveis.

## Matriz de testes antes de ativar produção

- Receitas JSON com zero, identidade inexistente, conflito cliente/servidor ou item `enabled=false`: negar fabrico.
- Receitas com 1 unidade e dois ingredientes físicos existentes: selecionar instâncias exatas, sem débito antes de commit.
- Materiais distribuídos por páginas A/B: não admitir seleção ambígua nem duplicados.
- Mochila cheia, crash antes/depois do commit, envio repetido, logout durante save: ausência de perda e duplicação.
- Logout/relogin: nova imagem DAT preserva resultado e materiais debitados.
- Recurso virtual SQLite: saldo num único local e débito idempotente a partir do custo JSON.
- Screens Inventory/Crafting, tooltip e item legacy: nome canónico sempre associado ao código físico, sem alterar atributos do item.
