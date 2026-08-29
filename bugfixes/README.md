# Summit Tale - Bugfix History

## Regra

**Um bug por vez. Um fix por vez. Um commit por vez.**

Não corrigir hipóteses como se fossem causas confirmadas. Distinguir sempre:

* **HIPÓTESE** — possibilidade ainda não comprovada.
* **PROVADO** — causa sustentada por evidência concreta.

## Fluxo obrigatório

```text
PROBLEMA
→ INVESTIGAÇÃO
→ CAUSA PROVADA
→ IDENTIFICAR FICHEIROS
→ REGISTAR COMMIT-BASE
→ BEFORE + SHA-256
→ PATCH MÍNIMO
→ AFTER + SHA-256
→ DIFF/PATCH
→ git diff --check
→ BUILD
→ TESTE NO JOGO
→ APROVAÇÃO
→ COMMIT ISOLADO
→ PUSH
→ PRÓXIMO BUG
```

## Estrutura

Cada fix fica em:

```text
/home/gabriel/Área de Trabalho/summit/bugfixes/NNN_nome_do_fix/
```

```text
README.md
hashes_before.sha256
hashes_after.sha256
change.patch
before/
after/
```

## Source

Árvore ativa:

```text
/home/gabriel/Área de Trabalho/summit/SourceCode
```

Referência histórica:

```text
/home/gabriel/armazem/priston/Source Summit Tale 


essa pasta tem srcclient e srcserver

#pasta para source alternativo mas não serve como base de verdade
/home/gabriel/armazem/priston/HISTORICO/SourceCode_rebase_20260824


#outras versões do jogo. util para verificar divergencias em source, buscar assets, etc
/home/gabriel/armazem/priston/drive-download-20260818T103311Z-1-002/priston-main
/home/gabriel/armazem/priston/drive-download-20260818T103311Z-1-002/fPT-rPT-Project-main
/home/gabriel/armazem/priston/ExMachina
/home/gabriel/armazem/priston/pristontale-1-master

```

Árvores antigas servem apenas para comparação e para retirar hunks específicos já compreendidos. Nunca substituir a árvore ativa nem copiar árvores antigas inteiras.

## Procedimento por fix

Antes de alterar qualquer ficheiro funcional:

1. provar a causa;
2. identificar todos os ficheiros que serão alterados;
3. registar o commit-base;
4. guardar cada ficheiro em `before/`;
5. calcular `hashes_before.sha256`.

Depois:

1. aplicar somente a alteração mínima necessária;
2. guardar os ficheiros resultantes em `after/`;
3. calcular `hashes_after.sha256`;
4. gerar `change.patch`;
5. gerar/comparar o diff BEFORE/AFTER;
6. executar:

```bash
git diff --check
```

7. documentar o fix no `README.md`.

## README do fix

Registar:

* problema;
* evidências da investigação;
* causa provada;
* alteração realizada;
* ficheiros alterados;
* commit-base;
* hashes BEFORE/AFTER;
* build realizado;
* teste realizado;
* resultado;
* estado: `APROVADO` ou `REPROVADO`;
* commit final, quando aprovado.

## Build

Só fazer build depois de existir **causa provada + patch concreto**.

Cliente:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
sh cliente.sh
```

Servidor:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
sh server.sh
```

Configurações necessárias ao runtime devem fazer parte do deploy.

## Aprovação e commit

O fix só é **APROVADO** depois de testado no jogo.

O cliente e o servidor testados devem corresponder exatamente ao estado documentado e commitado.

Depois da aprovação:

1. atualizar documentação e validar hashes;
2. stage somente dos ficheiros do fix;
3. deixar `bugfixing.txt` fora;
4. criar um commit exclusivo;
5. `git push`;
6. só então iniciar o próximo bug.

## Restrições

* Não misturar fixes.
* Não alterar `items/drop/merchant` enquanto essa área estiver congelada.
* Não remover instrumentação sem um fix específico de cleanup.
* Não fazer alterações oportunistas no mesmo patch.
* Se surgir outro bug, criar outro fix.
* Se um patch falhar ou piorar algo, **não empilhar outro patch por cima**: voltar ao último commit funcional e reinvestigar.



---

# Referência consolidada — experiência adquirida nos bugfixes

Esta seção resume o conhecimento reutilizável acumulado durante as investigações. Ela não substitui os `README.md`, auditorias, patches ou evidências de cada diretório; serve como índice rápido para evitar repetir pesquisas já feitas.

## Bugs resolvidos que servem como referência

| Fix | Problema / causa provada | Técnica ou referência útil |
| --- | --- | --- |
| `001_server_hotuk_deploy` | Login/entrada quebrados por `GAME_SERVER` incorreto e deploy que não levava `hotuk.ini`. | Comparar configuração usada no runtime com a árvore ativa e testar uma única alteração isolada. |
| `002_npc_field_path` | NPCs ausentes por comparação incorreta com literal multicaractere `'//'`. | Seguir a cadeia `field -> .spc -> LoadCharInfoFixed() -> OpenNpc()` e corrigir separadores como `'/'` ou `'\\'`. |
| `008_client_logs` / `009_server_logs` | Logs de diagnóstico eram escritos na raiz. | Auditoria de todos os writers, comparação com estado antigo e validação no runtime da pasta `LOGS`. |
| `014_client_startup_debug_boxes` | Instrumentação temporária interrompia o startup com `MessageBox`. | Remoção/neutralização mínima da instrumentação e teste completo de login/entrada no mapa. |
| `036_smchardecode_cp949_npc_fix` | Tokens legados de `smCharDecode()` tinham sido destruídos por conversão de encoding. | Comparação byte a byte, busca por `EF BF BD`, recuperação de hunks pré-reset e preservação explícita dos bytes CP949 necessários. |
| `041_monster_spawn_recovery_fix` | Spawn de monstros exigia recuperar `DecodeOpenMonster()` e impedir dereferência de `smMotionInfo == NULL`. | Instrumentação do parser `.spm`, symbolização do crash e reaplicação seletiva de hunks já comprovados. |
| `045_legacy_multichar_path_separator_comparisons` | 24 comparações usavam incorretamente `'//'`. | Busca global por um padrão de bug já provado, correção mecânica limitada e nova auditoria confirmando zero ocorrências. |
| `047_cracker_multichar_path_separator_assignment` | `'//'` era atribuído a `char` e dependia de truncamento implícito. | Tratar warnings de constantes multicaractere como possível defeito real; usar separador explícito. |
| `055_restore_canonical_sitem_catalog` | O cliente recebia itens válidos, mas `sItem[]` estava zerado porque o catálogo estático histórico havia desaparecido. | Instrumentação ponta a ponta, auditoria de Git/fontes externas e recuperação do dataset diretamente de um `Game.exe` histórico por análise PE/disassembly. |

Diretórios marcados como `EXPERIMENTAL`, `PENDENTE DE TESTE` ou apenas `audit` são evidência de investigação, **não** prova de resolução. Antes de reutilizar uma conclusão, conferir o estado documentado no diretório correspondente.

## Onde procurar, em ordem prática

1. **Runtime que realmente falha** — logs, arquivos carregados, configurações implantadas, assets presentes e comportamento observável. O runtime tem prioridade sobre suposições feitas apenas pelo source.
2. **Árvore ativa (`SourceCode`)** — localizar a cadeia completa que produz o sintoma: entrada, parser, transformação, lookup, loader e consumidor final.
3. **`bugfixes/NNN_*` anteriores** — procurar bugs da mesma família, patches mínimos, traces e hipóteses já descartadas.
4. **Working tree / patch pré-reset recuperado** — usar como fonte de hunks específicos quando há evidência de que aquele estado funcionava. Não restaurar árvores inteiras.
5. **Histórico Git** — verificar quando um arquivo, include, asset ou comportamento desapareceu; distinguir rename de remoção real.
6. **Fontes históricas externas** — principalmente `pristontale-1-master` para semântica legada/CP949; usar `fPT-rPT`, `priston-main` e `ExMachina` como referências secundárias, nunca como verdade automática do Summit.
7. **Binários e assets históricos** — quando o source perdeu dados gerados/estáticos, o executável antigo pode preservar strings, IDs, tabelas, chamadas de construtor e ordem dos registros.

## Técnicas que mais deram resultado

### 1. Reconstruir a pipeline antes de editar

Sempre transformar o sintoma em uma cadeia concreta. Exemplos já úteis:

```text
field -> .spc -> parser -> NPC
.spm -> DecodeOpenMonster() -> lista de monstros -> spawn
NPC config -> DefaultItems -> packet -> DecodeCompress() -> LoadItemImage() -> sItem[] -> shop UI
char/*.ini -> smCharDecode() -> model/motion -> loader
```

Instrumentar pontos de fronteira da cadeia é normalmente mais eficiente que alterar lógica por tentativa.

### 2. Usar controles positivos e negativos

Comparar algo que funciona com algo que falha: NPC funcional vs. NPC quebrado, árvore CURRENT vs. pré-reset, arquivo esperado vs. carregado, CODE encontrado vs. não encontrado. Uma diferença observada vale mais que uma hipótese baseada só no nome da função.

### 3. Comparar conteúdo real, não apenas texto renderizado

Em código legado, diferenças gigantes podem ser só encoding ou finais de linha. Quando necessário:

- comparar SHA-256;
- contar bytes `EF BF BD` (`U+FFFD` em UTF-8);
- inspecionar bytes CP949 originais;
- normalizar encoding/EOL apenas para a **auditoria**, não para regravar a árvore inteira;
- fazer substituição byte-level quando uma edição textual alteraria milhares de linhas sem necessidade.

Nunca converter todo o source para outro encoding como tentativa de correção.

### 4. Recuperar somente hunks compreendidos

Árvores antigas e recovery patches são fontes de evidência. O procedimento seguro é:

```text
localizar hunk -> entender a função -> provar relação com o sintoma -> aplicar só o hunk -> diff/check/build/runtime
```

Um recovery inteiro pode conter instrumentação antiga, experiências e regressões junto com código útil.

### 5. Instrumentar o runtime para localizar a primeira quebra

Adicionar logs temporários nos limites da pipeline e registrar valores que permitam provar o contrato: contador, CODE/opcode, path, retorno, tamanho, índice e resultado de lookup. Depois comparar o último ponto correto com o primeiro ponto incorreto.

Exemplo da investigação dos merchants: servidor montava a lista, enviava o packet, cliente recebia/decodificava CODEs corretamente e a primeira falha real estava em `LoadItemImage()` porque `sItem[]` tinha zero entradas válidas.

### 6. Symbolizar crashes antes de adicionar guards

Para crash nativo, preservar endereço/RVA e mapear para símbolo/instrução usando as ferramentas disponíveis (`nm`, `objdump`, `addr2line` e disassembly). No caso dos monstros, isso levou de um crash genérico até `smCHAR::FindMotionCountFromCode()` e provou `smMotionInfo == NULL` antes da correção.

### 7. Tratar warnings C/C++ legados como pistas

Literais como `'//'` são constantes multicaractere, não strings nem caracteres de path. Depois que esse padrão foi provado em um bug, uma busca global encontrou outras ocorrências reais. Só generalizar um padrão depois de existir uma causa concreta e manter cada correção isolada.

### 8. Recuperar dados perdidos de binários quando houver prova estrutural

No Fix 055, o catálogo cliente não existia no Git nem nas fontes externas corretas. O `Game.exe` histórico foi usado como fonte porque havia evidência estrutural reproduzível:

```text
strings -> referências em .text -> chamadas do construtor sITEM
-> 11 argumentos -> stride 0x320 -> índices 0..1818
-> 181 defaults -> MAX_ITEM 2000
```

O dataset recuperado foi validado contra os CODEs atuais e preservou ordem e duplicados. Não inferir campos que podem ser extraídos de uma fonte real.

## Encoding e parsers legados

O projeto mistura código moderno com conteúdo histórico CP949. `U+FFFD` dentro de texto de UI pode ser apenas corrupção visual; dentro de `lstrcmp`, `lstrcmpi`, tabelas de comandos, paths, categorias ou parsers pode alterar lógica.

Regra prática:

```text
texto humano/localização -> pode depender do idioma
identificador/token de lógica -> preferir ASCII/ID estável quando o formato permitir
compatibilidade com arquivo legado CP949 -> preservar os bytes exigidos pelo formato
```

Escapes `\xNN` são úteis para provar equivalência e preservar tokens legados sem depender do encoding do editor, mas não devem ser usados para inventar traduções ou substituir um protocolo que ainda não foi compreendido.

## Assets, paths e loaders

Antes de modificar um loader global:

1. confirmar o path final produzido;
2. confirmar diretório de trabalho/root do processo;
3. verificar se o asset existe exatamente naquele caminho;
4. conferir extensão e case quando relevante;
5. comparar com uma base funcional;
6. localizar a primeira chamada que retorna falha.

Problemas simples de configuração/path/asset devem ser corrigidos antes de reescrever loaders. O caso `ChatDefault-1.png` vs. `.tga` é uma boa referência de auditoria semântica mínima, mas deve respeitar o status de validação registrado no próprio fix.

## O que evitar

- não copiar uma árvore histórica inteira sobre a atual;
- não empilhar patches sobre um estado que piorou;
- não assumir que um diff enorme representa uma grande mudança sem verificar encoding/EOL;
- não tratar toda ocorrência de `U+FFFD` como bug funcional;
- não adivinhar IDs, metadata ou assets ausentes quando existe possibilidade de recuperação real;
- não alterar simultaneamente parser, rede, loader e UI quando a instrumentação já provou quais etapas funcionam;
- não promover uma auditoria ou experimento a `RESOLVIDO` sem build e validação no jogo.

## Checklist mental para novas investigações

```text
1. Reproduzir o sintoma.
2. Desenhar a pipeline do dado/evento.
3. Instrumentar fronteiras até achar a primeira divergência.
4. Procurar padrão semelhante nos bugfixes existentes.
5. Comparar CURRENT, runtime e uma referência confiável.
6. Provar a causa com evidência concreta.
7. Aplicar o menor patch ou restaurar o menor dado possível.
8. Validar diff, encoding/EOL e SHA-256 quando necessário.
9. Buildar somente o componente afetado.
10. Testar no jogo e verificar regressões próximas.
11. Documentar também hipóteses descartadas que evitem repetir trabalho.
```

A regra mais importante permanece: **o objetivo não é fazer o sintoma desaparecer; é localizar a primeira quebra comprovada da pipeline e corrigir somente essa causa.**
