# Drops e diagnóstico de itens inexistentes

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. Resultado da auditoria de Drops

Foram analisados **22 arquivos de drops** contra **1.985 códigos conhecidos de item**. O resultado foi:

- referências inexistentes: **0**;
- portanto, os drops fornecidos não possuem, nesta base, código apontando para item ausente do catálogo conhecido.

Isso não garante sozinho que todo item apareça visualmente no cliente: um item pode existir no perfil mas ainda depender de asset, modelo, ícone, definição OpenItem ou pack incompatível. A validação de código é apenas o primeiro gate.

## 2. Pipeline recomendado de validação

Para cada código presente em drop, vendor ou reward, validar nesta ordem:

1. **Código canónico:** existe no perfil client/server?
2. **OpenItem:** definição é materializável?
3. **Pack:** código está presente no `definitions_client.scfg` e/ou `definitions_server.scfg` conforme uso?
4. **Assets:** ícone/modelo/efeitos necessários existem?
5. **Runtime:** o item pode ser criado e enviado ao cliente sem erro?
6. **Gameplay:** pode ser dropado, recolhido, equipado/consumido e persistido?

## 3. Diferenciar três tipos de falha

### Código não existe

O drop ou shop referencia um código que não está no catálogo. A correção é remover a referência ou criar/materializar o item.

### Código existe, mas foi filtrado

É exatamente o que ocorre com alguns NPC shops legados. O compilador pode filtrar tokens que não têm OpenItem executável para preservar o comportamento histórico de “pular item inválido”. Consulte `01_ITENS_VENDEDORES_E_COINSHOP.md`.

### Código e definição existem, mas o item não aparece corretamente

Investigue assets, campos de definição, perfil client/server e o caminho de criação/envio. Não altere o código da loja para “mascarar” esse problema sem descobrir a causa.

## 4. Regra para remoção de item

Antes de apagar definitivamente um item, pesquise o código em:

- Drops;
- NPC shops;
- CoinShop JSON;
- quests/rewards;
- crafting/mix/aging;
- eventos;
- inventários/DB existentes;
- scripts/legacy compatibility.

Um código “não visível” pode continuar sendo ingrediente ou reward. A remoção correta exige análise de referências.

## 5. Validação contínua recomendada

Adicionar ao CI/build de Content três gates:

- `drop reference audit`: todo código em drops precisa resolver;
- `shop reference audit`: todo código de vendedor ativo precisa materializar ou estar explicitamente marcado como removido;
- `reward reference audit`: toda recompensa configurada precisa resolver em cliente/servidor.

Falha silenciosa deve virar erro de build para conteúdo novo. Para conteúdo legado, manter uma allowlist temporária com owner e prazo de remoção.
