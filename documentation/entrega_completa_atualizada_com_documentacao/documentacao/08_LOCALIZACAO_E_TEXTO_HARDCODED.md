# Localização, JSON e remoção de texto hardcoded

**Projeto:** Summit / refactor de conteúdo e gameplay  
**Base auditada:** source + Content + Drops + `Database(1).zip`  
**Data da auditoria:** 30/09/2026  

## 1. Regra obrigatória

Cada locale deve conter somente o idioma da própria localização:

- `pt-BR.json` -> português;
- `en.json` -> inglês;
- `ko.json` -> coreano;
- `zh-TW.json` -> chinês tradicional.

Na base canónica auditada estes são os quatro locales ativos. Não existe locale espanhol canónico ativo. Quando espanhol for adicionado, deve entrar como locale completo (`es`/`es-ES`, conforme decisão), e não como frases soltas misturadas em outro catálogo.

## 2. Estado do pt-BR

Depois das correções anteriores, `tools/audit_localization.py` reportou **0 issues** no `pt-BR` para as regras atuais de script estrangeiro/mojibake.

Isso é um gate importante, mas não detecta todos os erros semânticos. Uma frase em inglês puro pode parecer válida em Unicode e ainda estar errada. Nomes próprios incorretos também podem passar.

Por isso a localização deve ter dois níveis de QA:

1. automático/estrutural;
2. semântico/runtime.

## 3. Casos que precisam de smoke específico

Com base no backlog informado, testar explicitamente:

- sequência inicial / “pardal”;
- diálogos da Amanda;
- nomes e diálogos de personagens de Ricarten;
- NPC shop labels;
- textos de CoinShop/Time CoinShop;
- quests de asa;
- mensagens de mineração/lapidação;
- eventos sazonais.

A Amanda estava em português no Content auditado, mas ainda deve ser testada no runtime para excluir fallback/cópia antiga.

## 4. Infraestrutura canónica existente

O projeto já possui runtime de localização em `Shared/content/v1/localization/...` e o pack `Content/packs/localization.sloc` é compilado a partir do conteúdo canónico.

O build canónico executa:

- materialização do catálogo;
- validação;
- `compile_localization.py`;
- inspeção do pack.

## 5. Hardcodes: estado real

A busca ampla encontrou:

- **2.989 candidatos** no cliente;
- **1.960 candidatos** no servidor;
- total bruto: **4.949**.

Isso **não significa 4.949 textos para traduzir**. A busca inclui SQL, paths, logs, formatos, nomes de protocolo, third-party/Lua e mensagens internas.

Exemplos claramente player-facing encontrados em `LegacyContextGateway.cpp` incluem `Nivel`, `Forca`, `Mercador`, `Reparar tudo`, `Coin Shop`, `Time Coin Shop`, `Restauracao`, `Aging`, `Fundicao`, `Manufactura`, entre outros. Esses são candidatos reais à migração.

## 6. Classificação obrigatória antes de migrar

Cada literal deve cair em uma categoria:

### A. Player-facing -> migrar

UI, mensagem, diálogo, nome de opção, tooltip, erro mostrado ao jogador.

### B. Interno -> não localizar

SQL, path, nome de arquivo, chave de config, identificador de protocolo, comando, símbolo.

### C. Log de operador -> decisão separada

Logs podem permanecer em um idioma operacional ou receber chave própria, mas não devem ser misturados com localização do jogador.

### D. Third-party -> não alterar diretamente

Mensagens de bibliotecas devem ser tratadas no boundary, se precisarem ser expostas ao jogador.

## 7. Processo seguro para remover um hardcode

1. identificar o literal e o contexto;
2. confirmar que é player-facing;
3. criar uma chave semântica estável, por exemplo `ui.vendor.repair_all`;
4. adicionar a chave em **todos os locales suportados**;
5. preservar placeholders e tipos (`%d`, `%s`, `{0}`, etc.);
6. substituir o literal no source pelo lookup de localização;
7. definir comportamento para missing key;
8. recompilar `localization.sloc`;
9. rodar auditor automático;
10. executar smoke no locale.

## 8. Política de fallback

Para cumprir a regra “cada localização somente no seu idioma”, não usar fallback silencioso para inglês em produção como solução permanente.

Recomendação:

- development: mostrar `[[missing:key]]` + log forte;
- CI: falhar build se locale obrigatório estiver sem chave;
- production: usar fallback somente como mecanismo de segurança controlado, com telemetria, nunca como forma normal de completar tradução.

## 9. Placeholders

O validator deve comparar, por chave, o conjunto de placeholders entre locales. Exemplo:

`pt-BR`: `Você recebeu %d moedas de %s.`  
`en`: `You received %d coins from %s.`

Se uma tradução trocar `%d` por `%s`, o build deve falhar.

## 10. Nomes de NPC e dados legados

Muitas NPC definitions ainda carregam payload legado e campos como nomes coreanos. Nem todo campo de definição é necessariamente texto final apresentado: o nome visível pode vir de outro recurso/localização.

Portanto, para nomes quebrados de Ricarten:

1. capturar o nome realmente exibido in-game;
2. localizar a origem via binding/resource ID;
3. corrigir a fonte canónica correta;
4. evitar editar somente uma cópia sem efeito no pack;
5. testar cliente com cache/pack atualizado.

## 11. Auditoria semântica recomendada

Além de detectar coreano/mojibake no pt-BR, adicionar:

- dicionário/heurística de frases inglesas comuns em pt-BR;
- allowlist para nomes próprios e termos do jogo;
- comparação de chaves entre locales;
- placeholders equivalentes;
- detecção de valores idênticos a `en` acima de um limiar, com allowlist;
- relatório de chaves nunca usadas e hardcodes player-facing ainda ativos.

## 12. Definition of Done

- [ ] 100% das chaves obrigatórias presentes em todos os locales;
- [ ] nenhum placeholder incompatível;
- [ ] pt-BR sem inglês/coreano acidental;
- [ ] nenhuma UI player-facing nova aceita literal direto no C++;
- [ ] backlog de hardcodes existente classificado;
- [ ] Amanda, início/pardal e Ricarten validados in-game;
- [ ] build canónico e localization audit passam.
