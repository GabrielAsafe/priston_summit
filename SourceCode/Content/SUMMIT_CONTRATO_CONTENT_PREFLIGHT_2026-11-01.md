# SUMMIT — Contrato do Content e especificação do preflight de compilação

**Data:** 1 de novembro de 2026  
**Estado:** documento técnico para implementar um validador **somente de leitura**, antes da compilação e antes do deploy.  
**Escopo:** Content canónico, catálogos de itens, identidade nativa, packs binários, localização, assets de inventário/Crafting, publicação e diagnóstico de falhas.

> **Origem e grau de confiança.** Este documento combina (a) inspeção direta das fontes **reconstruídas anteriores aos patches 415A/415B** do ZIP `SUMMIT_RETOMADA_COMPLETA_2026-11-01_FONTES_E_DOCUMENTACAO`, (b) código das ferramentas 415D/415E/415F fornecidas nesta conversa e (c) os resultados **reais** reportados no Fedora: `compboth.sh RESULT=PASS`; arranque anterior com `DECODE_FAIL ... CI101`; 415D com `RUNTIME_CI101_IN_MAP=[]`; e, agora, o Mineiro das Cinzas a entregar um item visível na mochila **sem ícone**. A árvore Fedora atual (depois de 415A–415F) **não foi anexada**. Não é possível afirmar que um BMP concreto existe, que o `CI101` é exatamente a “pedra azul” exibida, nem que os 68 itens estão todos validados em jogo sem inspeção do cliente real.

## 0. Conclusões operacionais

1. **Compilar e linkar C++ não prova que o Content esteja utilizável nem que os assets possam ser carregados.** São portas de verificação distintas.
2. O erro `DECODE_FAIL GameServer/OpenItem/CI101.txt` foi rastreado à **ausência do código no mapa numérico runtime do servidor**. O SCFG e a localização estavam presentes/válidos no diagnóstico 415D. O parser de OpenItem procura `CODE` no catálogo `sItem`; se não encontra, a definição fica com `CODE=0` e o adaptador compilado rejeita-a.
3. No **inventário clássico**, o ícone não é selecionado pelo campo `icon_png` do JSON de profissões. A imagem é procurada como **BMP**, a partir da entrada `sITEM` do cliente:

   ```text
   cliente/image/Sinimage/Items/<ItemFilePath>/it<LastCategory>.bmp
   ```

   Para o item CI101, **apenas se** a entrada real declarar `ItemFilePath="Crafting"` e `LastCategory="CI101"`, o caminho esperado será `cliente/image/Sinimage/Items/Crafting/itCI101.bmp`. Não deduzir `ItemFilePath` a partir do código; **ler o valor real no `.inc` instalado**. A capitalização usada pelo loader é `Sinimage` (enquanto diretórios no disco podem usar outra grafia); o preflight deve detetar diferenças e não assumir portabilidade Linux/Wine.
4. O PNG `cliente/image/professions/v2/items/dummy/raw_ore_crystalline.png`, indicado pelo manifesto da interface de profissões, é um **asset de outra superfície visual**. A sua existência, por si só, **não satisfaz** o requisito do BMP da mochila.
5. **Hipóteses para o item sem ícone**, ainda não diferenciadas: BMP ausente; BMP corrompido/não carregável; diretório ou nome errado (incluindo capitalização); `ItemFilePath` errado/vazio; entrada `sITEM` do cliente não encontrada para o `CODE` numérico recebido; asset presente apenas no diretório de build, não no `cliente/` executado; cache/erro do carregador. O preflight deve distinguir estes casos, não alterar JSON aleatoriamente.
6. O `Servidor/Content/packs/item_code_map.json` tem prioridade sobre o fallback C++ legado **quando está presente**. Se estiver ausente, há fallback; se estiver presente mas inválido, o servidor **aborta**. Portanto, **não exigir CI101–CI168 no fallback C++ como bloqueio**, desde que haja um JSON runtime correto e íntegro; o 415E falhou por uma exigência demasiado forte, e o 415F corrigiu essa abordagem.
7. **Atenção ao deploy**: `SourceCode/tools/content_port/deploy_runtime_content.py` do snapshot publica uma lista de packs SCFG/SLOC/etc., mas **não lista `item_code_map.json`**. A distribuição deste mapa depende de outro publicador/patch; o preflight deve verificá-lo **separadamente** e rejeitar um deploy que o perdeu ou o substituiu.

## 1. Fluxo do Content: fonte → compilador → publicação → runtime

```text
SourceCode/Content/canonical/
    text/index.json e text/resources/**/*.json
    text/localization_overrides/pt-BR.json
    item_profiles/client.items.json, server.items.json
    item_profiles/client_item_code_map.json
    item_profiles/item_code_map.json
    professions_v2/... e manifestos de assets
    maps/, stage_spawns/, stage_start_points/, fixed_npcs/, commerce/ ...
          |
          | compCannonContent.sh / tools/content_port/build_from_canonical.py
          v
SourceCode/Content/packs/
    definitions_client.scfg          definitions_server.scfg
    localization.sloc                text_resources.sres
    field_runtime.sfld               maps_client.smap / maps_server.smap
    gameplay_config.sgcf             gameplay_reference.sgrf
    coin_shop.scsp                   skin_reforms.sskm
          |
          | publicadores; comparar hashes e papéis
          v
cliente/Content/packs/             Servidor/Content/packs/
cliente/image/... (assets fora SCFG) Servidor/Content/packs/item_code_map.json (*separado*)
cliente/Game.exe                   Servidor/Server.exe
```

**Regras:** editar o **canónico**, não editar `.scfg`/`.sloc` diretamente; não assumir que um asset gráfico é embutido num pack; não publicar metade de um conjunto. A resolução dos packs no runtime dá prioridade ao `Content/packs` próximo do executável, com fallback de desenvolvimento para a árvore de fontes. Executar o servidor no diretório adequado e validar os caminhos efetivamente resolvidos. Não confundir `Content/packs` com `content/maps` antigo nem `SourceCode/Content/packs` com `Servidor/Content/packs`.

### 1.1 Caminhos que interessam para itens físicos

| Camada | Caminho relativo à raiz `summit/` | Função |
|---|---|---|
| Catálogo C++ cliente | `SourceCode/SrcGame/src/systems/inventory/legacy/item_catalog_recovered.inc` | `sITEM(ID, ..., CODE, largura, altura, ItemFilePath, ...)`; identidade **e** caminho do ícone da mochila |
| Snapshot de identidade cliente | `SourceCode/Content/canonical/item_profiles/client_item_code_map.json` | Mapa espelho verificável do `.inc`, com SHA-256 da fonte |
| Perfil de definição cliente | `SourceCode/Content/canonical/item_profiles/client.items.json` | Códigos e SHA da fonte de identidade |
| Mapa canónico servidor | `SourceCode/Content/canonical/item_profiles/item_code_map.json` | `CODE` textual ↔ ID numérico real |
| Perfil de definição servidor | `SourceCode/Content/canonical/item_profiles/server.items.json` | Códigos aceites e SHA da fonte do perfil |
| Mapa servidor efetivo | `Servidor/Content/packs/item_code_map.json` | Lido no arranque; **não integra necessariamente o deploy genérico** |
| Fallback servidor | `SourceCode/Shared/inventory/item_code_map_mingw_legacy_server.inc` | Só quando o JSON servidor está **ausente**; não criar IDs por adivinhação |
| Definição item | `SourceCode/Content/canonical/text/resources/Servidor/GameServer/OpenItem/CI101.txt.json` | Documento canónico que produz `GameServer/OpenItem/CI101.txt` no SCFG |
| Índice textual | `SourceCode/Content/canonical/text/index.json` | Liga ID, JSON e aliases de caminho de runtime |
| Localização mantida | `SourceCode/Content/canonical/text/localization_overrides/pt-BR.json` | Chaves `item.ci101.name`… `item.ci168.name` e demais keys do catálogo |
| Pack item servidor | `SourceCode/Content/packs/definitions_server.scfg` e cópia runtime | Definições compiladas lidas pelo servidor |
| Pack item cliente | `SourceCode/Content/packs/definitions_client.scfg` e cópia runtime | Definições compiladas para o cliente |
| Pack de nomes | `SourceCode/Content/packs/localization.sloc` e cópias runtime | `en` e `pt-BR`, entre outros idiomas |
| Drop canónico | `SourceCode/Content/canonical/professions_v2/gm_drops_415.json` | Política de drop de monstro por alias/recurso |
| Drop runtime | `Servidor/Content/crafting/gm_drops_415.json` | Configuração consumida pelo hook no servidor |
| Ícones da interface profissões | `cliente/image/professions/v2/items/dummy/*.png` | Ícones da UI V2 **não equivalentes ao BMP legado** |
| Ícones da mochila | `cliente/image/Sinimage/Items/<ItemFilePath>/it<CODE>.bmp` | Assets usados pelo inventário legacy, quando `LastCategory=CODE` |

Os ficheiros pós-415A podem existir **somente** na árvore Fedora atual; as fontes reconstruídas são antigas. Um validador executado sem a árvore completa deve devolver **SKIP/INCOMPLETE**, nunca `PASS`.

## 2. Identidade de item: contrato obrigatório antes da compilação

### 2.1 As identidades **não são** intercambiáveis

| Conceito | Exemplo | Onde aparece |
|---|---|---|
| Recurso Crafting | `raw_ore_crystalline` | Receitas, ecossistema, drop |
| Alias Crafting | `C_it0x0001` | Regras GM/catálogos de materiais |
| Código nativo | `CI101` | `sITEM`, JSON de IDs, OpenItem |
| ID numérico | **ler do `.inc` atual**; não presumir valor | `sITEMINFO.CODE`, protocolo, DAT |
| ID de documento | `item.ci101` | SCFG, localização |
| PNG UI | `image/professions/v2/items/dummy/raw_ore_crystalline.png` | Crafting V2 |
| BMP mochila | derivado de `sITEM.ItemFilePath` e `sITEM.LastCategory` | Inventory legacy |

O pré-validador precisa de seguir a relação completa **recurso → alias → CODE → ID do cliente/servidor → OpenItem → localização → ícone BMP → deploy**. Existência isolada de qualquer parte não prova integridade do conjunto.

### 2.2 Formatos e limites de IDs

**Mapa servidor `item_code_map.json`** — o leitor C++ 381B é rigoroso:

```json
{
  "schema": "summit.item-code-map/v1",
  "entries": [
    {"code": "CI101", "numeric": "0xXXXXXXXX"}
  ]
}
```

`0xXXXXXXXX` acima é **um marcador**, não um valor válido; substituir pelo ID real do catálogo C++ cliente. **Não usar este fragmento de exemplo como ficheiro completo.** Restrições no runtime:

- `code`: **duas letras ASCII maiúsculas + três dígitos**, exatamente cinco caracteres; exemplo `CI101`.
- `numeric`: string `0x` seguida de **oito dígitos hexadecimais maiúsculos**, valor != `0` e representável em 32 bits.
- Chaves/ordem de campos **exatas**: topo `schema`, `entries`; por entrada `code`, `numeric`. O parser C++ **não é um parser JSON genérico**: não tolera campos adicionais, reordenação de chaves, escapes ou comentários. Gerar por serializador controlado e testar com o contrato estrito.
- Apenas ASCII imprimível para tokens do manifesto; ficheiro runtime **até 256 KiB**; entrada total **1..MAX_ITEM**, com `MAX_ITEM=2000` nas fontes inspeccionadas.
- `CODE` sem duplicação. O leitor de runtime verifica duplicados de `code`; o **preflight também deve impedir colisões de IDs numéricos entre códigos distintos**, pois o runtime não faz essa validação cruzada sozinho.
- Não renumerar nenhum ID já publicado: um item existente em DAT poderá tornar-se outro item. Registar baseline imutável e tratar mudança de ID como migração explícita, não correção automática.
- Para itens físicos novos: o `CODE` deve existir **exatamente uma vez**, com um só ID, no **catálogo cliente real**. O mapa do servidor tem de apontar para o **mesmo** ID. Perfis e snapshots devem estar sincronizados. O fallback C++ do servidor pode não incluir esse código.
- Cliente e servidor têm **capacidade e composição de catálogos diferentes**: não exigir números de entradas iguais, nem impor `loaded=1798` como valor eterno. Só impor capacidade e coerência de cada código necessário.

**Snapshot cliente (`client_item_code_map.json`)** segue `summit.client-item-code-map/v1` com `source.path`, `source.sha256` e `entries`. O `source.sha256` precisa coincidir com os bytes atuais de `item_catalog_recovered.inc`; as correspondências `(CODE, ID)` e a ordem das entradas também têm de coincidir. O perfil `client.items.json` precisa referenciar esse snapshot e respetivo SHA quando esta política está ativa. O perfil `server.items.json` referencia a origem autorizada e o seu hash. Validar **pelo `source.path` declarado**, sem fixar uma origem antiga.

### 2.3 `runtime_profiles` e OpenItem

- Para itens novos, confirmar que o JSON canónico OpenItem tem `kind="open_item_definition"`, ID estável (`item.ci101`), aliases de runtime válidos e diretiva de código equivalente a `*코드 "CI101"` (token legado em CP949: `2a c4 da b5 e5`).
- A propriedade canónica **`runtime_profiles: ["client", "server"]`**, quando explícita, tem precedência sobre as listas legadas de perfil. Se não existir, a inclusão é decidida pelo perfil legado do executável. Validar o resultado **efetivo** para os dois lados, não apenas a lista literal.
- A diretiva `code` é a que governa a materialização; não confiar só no texto do nome do ficheiro. O loader traduz o CODE textual por comparação com `sItem[i].LastCategory` e obtém `sItem[i].CODE` numérico. Se não existir correspondência, `CODE=0` e `ApplyCompiledItemDefinition` retorna `false`.
- Para itens físicos normais: confirmar as semânticas de `unique` adequadas (não confundir objetos `*Unique` com objetos de inventário normal) e que a definição é selecionada no perfil/pack. Um `CODE` duplicado em OpenItem direto pode produzir ambiguidade, ainda que o catálogo numérico não tenha duplicados.
- O compilador `compile_definitions.py` materializa `*linked_file` dentro do SCFG. Verificar alias relativo, destino indexado e ausência de referência não resolvida. Um `GameServer/OpenItem/*.txt` antigo em disco não substitui a definição canónica selecionada.

## 3. SCFG: integridade e materialização

**Formato observado:** `definitions_{client,server}.scfg`, magic `SCFG`, versão `1`, little-endian, header `'<4sHH10I32s'`, record `'<5I'`, diretiva `'<3I'`, digest SHA-256 do corpo. OpenItem = `kind=3`.

Verificar **depois de gerar os packs**, antes de distribuir:

1. Magic/versão, tamanhos de registos, contagens e offsets exatos; SHA-256 do corpo; flags/reservados canónicos; limites dos offsets e das strings NUL.
2. Cada `GameServer/OpenItem/CI101.txt` esperado aparece **uma vez no perfil correto** com `content_id=item.ci101`, `kind=3` e diretiva `CODE="CI101"` decodificável.
3. A definição aparece de facto no `definitions_client.scfg` e no `definitions_server.scfg` conforme `runtime_profiles`/perfil efetivo; não basta aparecer em `text/index.json`.
4. Todos os códigos diretos do pack resolvem para IDs no catálogo correspondente; não há `CODE=0`, colisões de alias, duplicados inesperados nem OpenItems indevidamente filtrados.
5. Conferir `.scfg.report.json` do compilador com `profile` esperado e dados de filtragem. Verificar que nomes e entradas de NPC Shop referenciam itens materializáveis quando isso é exigido por regras de loja.
6. Comparar SHA-256 do pack compilado com o publicado no papel correspondente **depois do deploy**. Um pack SCFG com hash correto não atesta os assets BMP externos.

**Evidência do problema anterior (415D):** `RUNTIME_RECORDS=2440`, `RUNTIME_CI101_MATCHES=1`, token `CODE="CI101"`, `RUNTIME_ITEMMAP_ENTRIES=1730`, `RUNTIME_CI101_IN_MAP=[]`. Packs builder/runtime tinham mesmo SHA; não era um SCFG em falta. O bloqueio estava na **resolução do CODE**.

## 4. Localização: índices, cobertura, encoding e capacidade

- Fonte mantida: `SourceCode/Content/canonical/text/localization_overrides/pt-BR.json` (não editar apenas `text/localization/pt-BR.json`, que é gerado).
- `materialize_localization_catalog.py` exige, quando existe o override de `pt-BR`, **cobertura exata** das chaves do catálogo inglês gerado: sem `missing` e sem `extra`; chaves únicas e textos não vazios. Foi precisamente a ausência de 68 chaves que bloqueou inicialmente o Content pós-415A.
- `compile_localization.py` gera `localization.sloc`: magic `SLOC`, versão `2`; exige fallback `en` para todas as chaves de runtime, rejeita NUL e produz relatório do pack. Validar integridade e versão após build.
- A localização do nome é pesquisada por `<content_id>.name` (ex.: `item.ci101.name`); no modo estrito, falta de chave, catálogo indisponível ou conversão que não cabe pode causar `ApplyCompiledItemDefinition=false`.
- `sITEMINFO.ItemName` tem **32 bytes totais**, incluindo `\0`: comprimento codificado **menor que 32**. A ponte do cliente/servidor usa CP1252 para `pt-BR`, quando representável, e não o número de caracteres Unicode nem só os bytes UTF-8. O preflight verifica codificação e tamanho, com fallback legacy quando a política o permite; não assumir que todos os nomes Unicode cabem.
- Diagnóstico 415D: `Minério Cristalino Bruto` tinha **24 bytes CP1252**, e estava presente em `en` e `pt-BR` do SLOC. Logo, esse nome **não explica** o `DECODE_FAIL` observado.
- Não reduzir requisitos de localização para esconder uma falha de ID; são verificações independentes.

## 5. Assets: requisito que teria detetado o item sem ícone

### 5.1 O caminho é derivado do catálogo C++ do cliente

No cliente, `sinInvenTory.cpp::InvenItemLoad` e `sinSubMain.cpp::LoadItemImage` usam o mesmo padrão:

```cpp
wsprintf(szFilePath,
         "image//Sinimage//Items//%s//it%s.bmp",
         sItem[j].ItemFilePath,
         sItem[j].LastCategory);
```

O `LoadItemImage` procura primeiro **igualdade numérica** entre o `sITEMINFO.CODE` recebido e `sItem[j].CODE`; depois carrega a imagem com `LoadDibSurfaceOffscreen`. A falta do código e a falta do ficheiro são problemas **diferentes**. `sITEM` contém `ItemFilePath[64]`, `LastCategory[16]`, `w`, `h`, e ponteiro de textura. O bitmap é um ficheiro do cliente, **não** uma diretiva `icon_png` do SCFG.

**Verificação obrigatória para cada item novo/físico:**

1. Extrair **todos os campos** do `sITEM(...)` **real**, não apenas `ID` e `CODE`: ID, código, largura/altura, `ItemFilePath`, outras flags relevantes; recusar registo truncado ou parser que ignore uma entrada. O regex simples dos validadores 415E/415F recolhe os dois primeiros campos úteis mas **não prova que os campos gráficos estejam corretos**.
2. `ID` não nulo e único, `CODE` não repetido, `w>0` e `h>0`; `ItemFilePath` e `LastCategory` não vazios; componentes sem `..`, separadores, drive letter ou bytes de controlo que escapem ao diretório permitido.
3. Gerar o caminho exatamente segundo o formato do C++ e verificar que existe **em `cliente/`**, não apenas em `SourceCode/`, no ZIP antigo ou numa pasta de assets de desenvolvimento.
4. Validar conteúdo BMP (assinatura `BM`, header DIB e offsets/tamanhos consistentes; dimensões >0; tipo/compressão aceites pelo carregador real). O preflight pode verificar a estrutura básica; **só um smoke test no cliente prova compatibilidade com DirectDraw/Direct3D/Wine**. Reportar dimensões reais contra `w/h` do catálogo como aviso, até haver uma regra comprovada de equivalência.
5. Confirmar capitalização/path case no Linux, distinguir correspondência exata de correspondência case-insensitive e detetar nomes ambíguos. Não confiar que Wine compensará sempre discrepâncias.
6. Conferir que o **BMP está presente na distribuição final** (diretório ou arquivo que efetivamente fornece assets ao cliente); verificar SHA-256 no build e deploy, se há mecanismo de cópia. O SCFG compilar com sucesso não copia magicamente este BMP.
7. Separadamente, validar PNG de Crafting V2 pelo manifesto `professions_v2/ecosystem_366/assets_manifest_366.json` e o binding que a interface V2 usa. `dummy_only=true` e `require_client_binding=true` nesse manifesto demonstram que a referência gráfica de profissões **não deve ser usada como prova** da imagem legacy.
8. Não gerar um BMP opaco, um placeholder ou renomear um PNG para `.bmp` apenas para fazer o preflight passar. É necessária conversão real e teste de carregamento antes de promover asset definitivo.

**Rastreio opcional já existente no código do snapshot:** `SUMMIT_ITEM_IMAGE_TRACE=1` habilita registo em `LoadItemImage_debug.txt`, com `[LOOKUP]`, `[MATCH]` e `[IMAGE_FAIL]` se a versão de `Game.exe` em execução mantiver este instrumentador. Interpretar:

- `exactMatches=0` → cliente **não reconheceu o ID numérico**; investigar `.inc`/Game.exe/deploy.
- `[MATCH] ... Path="..." Cat="..."` → usa estes valores para reconstruir o BMP correto.
- `[IMAGE_FAIL] path="..."` → falha ao carregar o bitmap efetivamente procurado. Verificar ficheiro e formato; não mexer no mapa de ID sem evidência.
- Se não houver log, confirmar que o executável foi recompilado com o código de rastreio e a variável de ambiente chegou ao processo; silêncio não equivale a sucesso.

### 5.2 Observação atual e diagnóstico pendente

**Observado pelo utilizador:** ao morrer o Mineiro das Cinzas, surge uma “pedra azul” no inventário **sem ícone**. Isso é boa evidência de que o caminho de entrega chegou à mochila e de que a falha inicial de arranque já não está a ocorrer naquela sessão. **Não prova** a identidade numérica do item, o carregamento correto de texturas, a persistência DAT ou o funcionamento de todas as entradas CI101–CI168.

Para confirmar a causa exata no Fedora: extrair do catálogo real o registo da “pedra azul”/`CI101` e o ID numérico; identificar `ItemFilePath` e `LastCategory`; verificar o caminho BMP publicado e executar o cliente com `SUMMIT_ITEM_IMAGE_TRACE=1` (se disponível). **A afirmação “o JSON aponta para o asset errado” continua uma hipótese**, não diagnóstico confirmado.

## 6. Drops físicos: contrato adicional (não confundir com assets)

Os ficheiros `gm_drops_415.json` canónico e runtime ligam modelo do monstro, recurso e alias à entrega de um código nativo. Exemplo de **uma entrada** (não o ficheiro completo):

```json
{
  "model_contains": "undeadminer",
  "resource_key": "raw_ore_crystalline",
  "item_alias": "C_it0x0001",
  "storage": "physical",
  "quantity": 1,
  "chance_bp": 10000
}
```

Regras do 415A documentadas no runbook: parser de JSON restrito e dependente das propriedades que aceita; primeiro `model_contains` correspondente, ASCII minúsculo; `chance_bp` em 0..10000; em `physical`, quantidade 1 no estado referido; regras carregadas no arranque/cache; mudar JSON runtime exige **reiniciar o servidor**. Para códigos nativos novos é necessário atualizar catálogos/definições e recompilar. O preflight deve verificar *todos* os aliases de `gm_drops_415.json` contra a tabela real de aliases, recursos existentes, `storage` conhecido e a cadeia resolúvel até um ID nativo sem colisão. Regra que entrega item físico não pode simultaneamente creditar SQLite por fallback silencioso.

Os checks de drop não substituem os de imagem (servidor não renderiza o inventário) nem a persistência DAT (um item visto online pode desaparecer após relogin).

## 7. Publicação: dois contratos independentes

**Packs de Content:** `deploy_runtime_content.py` define para cliente/servidor packs comuns (`field_runtime.sfld`, `skin_reforms.sskm`, `text_resources.sres`, `localization.sloc`, `gameplay_config.sgcf`, `gameplay_reference.sgrf`) e packs específicos (definitions/mapas, e `coin_shop.scsp` no servidor). Faz hashes de cópia e gera `Content/RUNTIME_CONTENT.json`. O `validate_runtime_pack_resolution.py` confirma vários desses packs e a precedência de resolução, mas **não é validação completa de todos os packs nem do JSON de IDs**.

**Ficheiros extra-packs:** a lista genérica acima **não inclui** `Servidor/Content/packs/item_code_map.json`, `Servidor/Content/crafting/gm_drops_415.json` nem `cliente/image/...`. São contratos adicionais que o script de pré-deploy tem de comprovar, independentemente de `CANONICAL_BUILD=PASS` e `compboth.sh RESULT=PASS`.

Regras para publicar:

- Fazer preflight de fonte **antes** de `compCannonContent.sh` (sem depender de packs antigos).
- Fazer validação pós-geração dos SCFG/SLOC e dos metadados SHA.
- **Não distribuir** se o source/pack não tiver coerência de identidade ou localização.
- Fazer validação **pós-deploy** dos packs por papel, do mapa servidor de IDs, das regras GM runtime e dos assets do cliente, com comparação SHA-256 e checks de leitura.
- Confirmar `Game.exe` e `Server.exe` compilados na versão desejada; não assumir que o publicador de Content instala automaticamente executáveis ou imagens.
- Parar se hashes mudarem entre `inspect` e `apply`; operações de mutação e rollback só pertencem aos instaladores/publicadores, **não** ao preflight.
- Não usar `|| true`, não esconder `FAIL`, não renumerar um `CODE` que possa existir em DAT.

## 8. Matriz concreta de regras para implementar no preflight

**Legenda:** `BLOCK` = recusar a fase; `WARN` = alertar, sem fingir certificação; `SKIP` = componente indisponível numa execução parcial. As regras de origem podem ser executadas *antes* de gerar packs; as de packs só podem ser `BLOCK` em `post-build`/`pre-deploy`, não num build limpo anterior à geração.

| ID | Fase | Severidade | Verificação | Erro que deteta |
|---|---|---|---|---|
| SRC001 | source | BLOCK | `SourceCode/Content/canonical/` e JSON/índices exigidos existem, JSON UTF-8 válido, paths internos não escapam à raiz, sem symlinks suspeitos nos alvos críticos | fonte ausente/insegura |
| SRC002 | source | BLOCK | IDs/aliases de `text/index.json` únicos e os JSON referenciados existem; `kind`, `runtime_paths` e `id` consistentes | definições órfãs/alias colidido |
| ID001 | source | BLOCK | `item_catalog_recovered.inc` reconhecido integralmente; código, ID, dimensões, `ItemFilePath`, `LastCategory` lidos corretamente; sem novo item parcial | registo cliente incompleto |
| ID002 | source | BLOCK | Novos códigos físicos com exatamente um ID não nulo, sem colisões numéricas com códigos existentes | perda/duplicação de identidade |
| ID003 | source | BLOCK | `client_item_code_map.json` = projeção do `.inc` atual, em entradas/ordem e hash de origem; perfil cliente sincronizado | snapshot 383A desatualizado |
| ID004 | source | BLOCK | `item_code_map.json` canónico com schema **e ordem** estritos 381B, regex, ≤256 KiB, ≤`MAX_ITEM`, IDs não repetidos | `INVALID_SCHEMA_OR_ID`/abort |
| ID005 | source | BLOCK | Para cada item novo que será publicado, ID no mapa servidor = ID no catálogo cliente; perfil servidor/source SHA consistentes | `DECODE_FAIL` por `CODE=0` |
| ID006 | source | WARN | Fallback C++ tem ID de CI? Se tiver, exigir igualdade e inexistência de colisão; se não tiver, marcar dependência obrigatória de JSON runtime | falso `BLOCK` do 415E |
| DEF001 | source | BLOCK | OpenItem novo com código correto, ID, kind, diretiva legacy e `runtime_profiles` elegível em ambos os lados; sem duplicação | pack filtra CI ou materializa 0 |
| DEF002 | source | BLOCK | `linked_file` e alias de destino indexados, sem colisão/path traversal | definições incompletas |
| LOC001 | source | BLOCK | `pt-BR` override cobre **exatamente** keys de `en` geradas (sem duplicados, vazios ou extras) | bloqueio das 68 traduções |
| LOC002 | source | BLOCK | `item.<code>.name` presente para novos itens, compatível com CP1252 quando requerido, `len(encoded)<32` | falha estrita / buffer de nome |
| AST001 | source/pre-deploy | BLOCK | Para cada novo item físico: BMP legacy presente no caminho **derivado do `sITEM`** em `cliente/`/payload de distribuição; sem placeholder | mochila sem ícone |
| AST002 | source/pre-deploy | BLOCK | BMP com header e tamanhos válidos, dimensões positivas, caminhos seguros e capitalização não ambígua | BMP inválido/nome errado |
| AST003 | source/pre-deploy | WARN/BLOCK | PNG de profissões V2 existe e está ligado à UI; **BLOCK apenas quando a funcionalidade UI V2 é escopo obrigatório** | Crafting V2 sem ícone |
| DROP001 | source | BLOCK | JSON GM canónico sintaticamente aceite, aliases/recurso/modelo coerentes; quantidade/probabilidade nos limites | drop inexistente/indevido |
| PACK001 | post-build | BLOCK | SCFG/SLOC com magic, versão, SHA do corpo, offsets e contagens válidos | pack corrompido |
| PACK002 | post-build | BLOCK | CI101–CI168 (ou escopo de novos itens) efetivamente presentes nos perfis exigidos, CODE, ID e `content_id` coerentes | `DECODE_FAIL` no arranque |
| PACK003 | post-build | BLOCK | Relatórios e `BUILD.json` coerentes com SHA do pack e fontes esperadas; builds determinísticos quando exigidos | pack desatualizado |
| PUB001 | pre-deploy/pós-deploy | BLOCK | SHA dos packs compilados = packs em `cliente/` e `Servidor/` para o respetivo papel | publicou Content antigo |
| PUB002 | pre-deploy/pós-deploy | BLOCK | SHA de `Servidor/Content/packs/item_code_map.json` = canónico; contém todos códigos novos e passa parser estrito | mapa de IDs perdido pelo deploy |
| PUB003 | pre-deploy/pós-deploy | BLOCK | `Servidor/Content/crafting/gm_drops_415.json` presente/validado; conteúdo esperado ou divergência explicitamente autorizada | drop runtime ausente/velho |
| PUB004 | pós-deploy | BLOCK | Assets BMP/PNG obrigatórios presentes no cliente efetivamente executado; SHA e caminho corretos | só existiam na árvore de fontes |
| RT001 | smoke test | BLOCK para certificação | Servidor inicia sem `DECODE_FAIL`/`FATAL`; `ITEM_CODE_JSON loaded` e drops válidos | falha de arranque |
| RT002 | smoke test | BLOCK para certificação | Cliente abre mochila com ícone de item e ID corretos; sem `[IMAGE_FAIL]` | imagem não carregou apesar de preflight |
| RT003 | integração | BLOCK para certificação DAT | Morte única → item físico único → relogin → mesmo item; sem duplo crédito SQLite | regressão de persistência/antidupe |

`BLOCK` aqui significa **bloquear entrega/compilação da funcionalidade abrangida**, não prometer que o compilador C++ já falha naturalmente: por exemplo, `AST001` é *novo gate de qualidade* que deve rejeitar a build mesmo que o C++ consiga compilar.

## 9. Interface/contrato de saída do futuro script

**Nome sugerido** (ainda **não implementado**): `SourceCode/tools/content_port/preflight_content.py`.

```text
python3 SourceCode/tools/content_port/preflight_content.py --root . --phase source
python3 SourceCode/tools/content_port/preflight_content.py --root . --phase post-build
python3 SourceCode/tools/content_port/preflight_content.py --root . --phase post-deploy
```

Saída humana recomendada, com ficheiro exato, valor encontrado, valor esperado e ação corrigível:

```text
SUMMIT_PREFLIGHT phase=source mode=READ_ONLY
[PASS] ID005 CI101 client=0x<REAL> server=0x<REAL>
[FAIL] AST001 code=CI101 client_path=cliente/image/Sinimage/Items/<DIR>/itCI101.bmp reason=NOT_FOUND
[WARN] ID006 server_fallback=ABSENT runtime_json_required=YES
RESULT=FAIL blocks=1 warnings=1 skips=0
```

O exemplo é **ilustrativo**, não prova o ID real nem o diretório. Para consumo automatizado, emitir também `--format json` com `{schema, phase, status, checks:[{id,status,severity,path,expected,actual,action}]}` e códigos de saída **0 = PASS**, **1 = BLOCK/FAIL**, **2 = erro interno/inputs inválidos**. `--root` é a raiz com `SourceCode/`, `cliente/` e `Servidor/`; nunca depender do utilizador `gabriel` hardcoded. `--phase source` não deve escrever/reformatar JSON, materializar localizações, gerar packs ou modificar hashes. `--phase post-build` e `post-deploy` inspecionam sem corrigir silenciosamente.

### Ordem sugerida no build (cada comando só após `PASS` anterior)

```bash
cd "$HOME/Área de Trabalho/summit"
set -e
# 1: futuro validador somente leitura (depois de implementado)
python3 SourceCode/tools/content_port/preflight_content.py --root . --phase source
# 2: construção de Content
sh compCannonContent.sh
# 3: validação dos packs produzidos
python3 SourceCode/tools/content_port/preflight_content.py --root . --phase post-build
# 4: binários
sh SourceCode/compboth.sh
# 5: publicar, após fechar cliente e servidor
sh SourceCode/send_to_s.sh
sh SourceCode/send_to_client.sh --apply
# 6: validação do conteúdo publicado e assets
python3 SourceCode/tools/content_port/preflight_content.py --root . --phase post-deploy
```

**Atenção:** os comandos `preflight_content.py` são apenas o **contrato proposto**; o ficheiro ainda não existe neste documento. A sequência `send_to_*` depende da versão real dos scripts do Fedora. O validador deve parar antes da publicação se os assets obrigatórios não estiverem disponíveis; é recomendável correr também a porta `AST001` em `source` ou `pre-deploy`.

## 10. Critérios mínimos de aceite para a “pedra azul”

Uma alteração de Content/asset só pode ser considerada fechada quando:

- [ ] Identificado o **ID numérico real** da pedra no `sITEMINFO` recebido e no `.inc` do cliente.
- [ ] Identificado o `CODE` efetivo (`CI101`, **se comprovado**, ou outro) e a entrada `sITEM` correta.
- [ ] Confirmado o caminho BMP completo **construído a partir do `.inc`**, sem suposição.
- [ ] Confirmado ficheiro BMP válido na pasta **do cliente executado**, além do PNG da UI V2 quando aplicável.
- [ ] Preflight `ID*`, `DEF*`, `LOC*`, `AST*`, `PACK*`, `PUB*` passa.
- [ ] Arranque do servidor sem falha e cliente mostra ícone no inventário clássico (smoke test).
- [ ] Testado relogin/DAT e ausência de recompensa duplicada; **é requisito de certificação funcional**, não do parser gráfico.
- [ ] Extensão do mesmo contrato a **CI101–CI168**, não só ao primeiro código.

## 11. Referências técnicas inspecionadas

Estes caminhos são relativos a `SourceCode/` do snapshot do ZIP de retomada e indicam **onde foi confirmada cada regra**. Ao implementar o preflight, validar as versões atuais do Fedora antes de fixar números de linha ou heurísticas de parser.

| Evidência | Código/fonte |
|---|---|
| Caminho e carregamento de BMP da mochila | `SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp` (`InvenItemLoad`, cerca das linhas 468–507) |
| Lookup `sITEMINFO.CODE`, `[MATCH]`, `[IMAGE_FAIL]`, variável de debug | `SrcGame/src/core/client/legacy_sin_runtime/sinSubMain.cpp` (`LoadItemImage`, cerca das linhas 833–990) |
| Campos `sITEM`, `MAX_ITEM=2000`, `ItemName[32]` | `Shared/inventory/legacy/sinItem_legacy_client.h`; `sinItem_legacy_server.h` |
| Fonte da identidade e dos campos gráficos | `SrcGame/src/systems/inventory/legacy/item_catalog_recovered.inc` |
| Parser runtime estrito de IDs | `Shared/inventory/legacy/ItemCodeJsonRuntime381B.h` |
| Prioridade do JSON e fallback C++ | `Shared/inventory/legacy/sinItem_legacy_server.inl` (`MingwPopulateItemCodeMap`) |
| Leitura da diretiva CODE e inicialização segura | `Shared/formats/item_config/ItemDefinitionReader.inl` |
| Rejeição `CODE=0` e localização no item | `Shared/content/v1/definition/legacy/CompiledDefinitionAdapter.inl` (`ApplyCompiledItemDefinition`) |
| Fonte vs snapshot do catálogo cliente | `tools/content_port/client_profile_map_383a.py`; `sync_client_profile_383a.py` |
| Política de perfis explícitos | `tools/content_port/item_profile_policy.py` |
| Compilação de SCFG e `linked_file` | `tools/content_port/compile_definitions.py`; `validate_item_profile_pack.py` |
| Cobertura estrita de `pt-BR` | `tools/content_port/materialize_localization_catalog.py` |
| Compilação SLOC e regra de 32 bytes | `tools/content_port/compile_localization.py`; `validate_legacy_localization_bridge.py` |
| Pipeline dos packs | `tools/content_port/build_from_canonical.py`; `validate_build_reproducibility.py` |
| Lista de deploy genérico (sem o mapa JSON) | `tools/content_port/deploy_runtime_content.py`; `validate_runtime_pack_resolution.py` |
| PNG da interface de Crafting | `Content/canonical/professions_v2/ecosystem_366/assets_manifest_366.json` |
| Outras regras de visuais (não necessariamente aplicadas aos materiais) | `tools/content_port/compile_item_visual_registry.py` — suporta trajes/slot 14 nesta versão, **não** pressupor binding CI101 |
| Contratos 415A/415B e persistência | `ARQUITETURA_CONTENT_E_CRAFTING.md`; `EXECUCAO_E_VERIFICACOES.md` do pacote de retomada |
| Mapas e perfis reais do Fedora | Saídas reportadas de `diagnostico_ci101.py` (415D), e README/instalador 415F |

## 12. Limites do diagnóstico e próximos dados necessários

O ZIP de retomada é uma reconstrução **pré-415A/415B** e não inclui a árvore corrente pós-415F nem a distribuição `cliente/image/` completa. Assim, **não foi inspecionado o registo gráfico CI101 atual**, nem a imagem efetivamente usada. O output 415D anterior provou o mapa antigo sem CI101; o relato atual prova que a situação do jogo evoluiu, mas não substitui as provas de identidade/asset. Não descrever como “ficheiro BMP em falta confirmado” até correr `LoadItemImage` com trace ou inspeção equivalente. O propósito do preflight é transformar estas incertezas em erros concretos, reprodutíveis e corrigíveis **antes** da próxima compilação/publicação.

**Resumo para a implementação:** `CODE` e `ID` resolvem a identidade; **SCFG** define o item; **SLOC** dá o nome; **sITEM** determina as características gráficas e o **BMP da mochila**; o **PNG de profissões** é independente; **packs e JSON extra-packs** precisam de deploy; o **DAT** só fica certificado por teste de persistência. O script deve validar **a cadeia inteira** e impedir releases com assets ou mapas desfasados, mesmo quando o compilador C++ indica `PASS`.
