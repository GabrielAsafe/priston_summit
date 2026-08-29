# Summit Tale — Investigação de Encoding, DSL de Models/Motions e Falha de Carregamento de Itens

**Status:** investigação ativa  
**Último estado funcional observado:** cliente/servidor iniciam sem crash após o bugfix 049, porém os itens não aparecem.  
**Prioridade imediata:** instrumentar o servidor para provar quais itens foram carregados, quantos foram carregados e em qual etapa ocorre a perda.  
**Objetivo deste documento:** preservar todo o raciocínio, evidências, decisões e próximos passos para que a investigação possa continuar sem repetir pesquisas nem reabrir hipóteses já descartadas.

---

## 1. Resumo executivo

A investigação começou a partir de um problema mais amplo de caracteres corrompidos no source legado do jogo. O código original possui partes historicamente escritas em coreano, principalmente em CP949, e várias cópias do source passaram por conversões ou edições que substituíram bytes não representáveis por `U+FFFD` (`EF BF BD` em UTF-8).

O problema deixou de ser apenas visual quando foram encontradas strings corrompidas participando diretamente de:

- comparações com `lstrcmp` / `lstrcmpi`;
- identificação de comandos de parser;
- identificação de categorias e estados;
- tabelas de monstros;
- configuração de model/motion;
- caminhos de assets;
- estruturas de item;
- lógica de NPC, shop e warehouse.

A melhor fonte de recuperação até agora é:

```text
/home/gabriel/armazem/priston/pristontale-1-master
```

Essa árvore preserva muitos bytes CP949 originais e permite recuperar a semântica de várias strings destruídas no Summit.

Foi construída uma auditoria semântica que:

1. percorre `SrcGame`, `SrcServer` e `Shared`;
2. separa `Language` de `LOGIC`;
3. identifica `U+FFFD` e outros literais não ASCII;
4. procura estrutura equivalente primeiro no PSTale Master;
5. consulta fontes secundárias somente quando necessário;
6. registra candidatos com confiança `HIGH`, `MEDIUM` ou `LOW`.

A auditoria revelou milhares de ocorrências, inclusive dentro de lógica. Porém também produziu falsos positivos em arquivos binários e candidatos `LOW`, o que levou à regra atual:

> **Nenhuma recuperação deve ser aplicada automaticamente. Candidatos servem como evidência; o fix só acontece depois de reconstruir a semântica e o contrato do subsistema.**

Durante essa investigação foi descoberto que `fileread.cpp` contém uma DSL interna de model/motion cujos comandos estruturais eram originalmente coreanos em CP949. No Summit, vários desses tokens já estavam destruídos. O bugfix 049 substituiu somente quatro comandos estruturais e quatro valores especiais por aliases ASCII:

```text
*착용무기   -> *EQUIPPED_WEAPON
*적용직업   -> *APPLICABLE_JOB
*해당위치   -> *LOCATION
*적용기술   -> *APPLICABLE_SKILL

모두         -> ALL
없음         -> NONE
마을         -> TOWN
필드         -> FIELD
```

O build/teste não causou crash, mas os itens continuam sem aparecer. Isso significa que:

- o fix 049 não resolveu o problema de itens sozinho;
- a causa pode estar em outra etapa do pipeline;
- é necessário medir o runtime antes de continuar alterando lógica.

A próxima ação correta é **instrumentar o servidor para observar o carregamento real dos itens**.

---

# 2. Princípios adotados durante a investigação

## 2.1. Idioma de UI não é idioma de lógica

Arquivos de localização podem e devem preservar seus idiomas:

```text
Language/Korean/
Language/Japanese/
Language/Brazil/
Language/English/
...
```

Por outro lado, keywords de parser, identificadores de protocolo, categorias técnicas, códigos de item e nomes usados em lookup não deveriam depender de coreano, português ou qualquer outra língua humana.

A direção arquitetural adotada é:

```text
LOCALIZATION
    texto humano
    pode ser coreano, japonês, português etc.

LOGIC / IDENTIFIERS
    ASCII / inglês / IDs numéricos
    independentes de locale e encoding
```

Isso reduz:

- corrupção por conversão de encoding;
- dependência de codepage;
- bugs causados por editor/compiler;
- dificuldade de manutenção;
- lookup quebrado por mojibake.

---

## 2.2. Não converter o source inteiro de encoding

Foi explicitamente evitada uma conversão global como:

```text
CP949 -> UTF-8 em toda a árvore
```

porque a árvore contém arquivos de diferentes origens e encodings.

Uma conversão global poderia:

- alterar bytes usados intencionalmente;
- quebrar arquivos japoneses/chineses;
- alterar recursos históricos;
- modificar milhares de linhas sem relação com o bug;
- criar regressões difíceis de localizar.

A estratégia correta é local:

```text
identificar
    ↓
provar a origem
    ↓
entender a semântica
    ↓
corrigir somente o ponto funcional
```

---

## 2.3. Bytes explícitos são evidência, não solução padrão

Foi testado que o MinGW consegue emitir bytes CP949 através de:

```cpp
"\xB4\xD4..."
```

e também através de:

```bash
-finput-charset=UTF-8
-fexec-charset=CP949
```

Porém guardar toda a lógica em escapes hexadecimais foi considerado ruim para manutenção.

Exemplo:

```cpp
"%s \xB4\xD4\xC0\xCC ..."
```

funciona tecnicamente, mas:

- esconde o significado humano;
- dificulta revisão;
- perpetua dependência de CP949;
- torna futuros patches mais arriscados.

A regra atual é:

> bytes explícitos podem ser usados em testes, evidências de equivalência ou compatibilidade legada, mas não devem ser a representação principal da lógica nova.

---

# 3. Fontes usadas para recuperação

## 3.1. Fonte principal

```text
/home/gabriel/armazem/priston/pristontale-1-master
```

Até agora é a melhor referência por:

- manter partes originais em CP949;
- possuir estruturas próximas do Summit;
- preservar várias strings que no Summit viraram `U+FFFD`;
- conter versões antigas dos mesmos parsers.

---

## 3.2. Fontes secundárias

Também foram consideradas:

```text
/home/gabriel/armazem/priston/ExMachina

/home/gabriel/armazem/priston/drive-download-20260818T103311Z-1-002/fPT-rPT-Project-main

/home/gabriel/armazem/priston/drive-download-20260818T103311Z-1-002/priston-main
```

Uso recomendado:

```text
1. PSTale Master
2. fPT/rPT
3. priston-main
4. ExMachina
```

A ordem importa porque versões mais modernas podem:

- renomear sistemas;
- reestruturar classes;
- remover partes legadas;
- substituir tabelas por sistemas novos.

Elas são ótimas como confirmação secundária, mas não devem automaticamente prevalecer sobre uma fonte estruturalmente mais próxima.

---

# 4. Auditoria semântica de encoding

Foi criada uma ferramenta de auditoria com a intenção de evitar buscas manuais repetitivas.

Ela varre:

```text
SourceCode/SrcGame/src
SourceCode/SrcServer/src
SourceCode/Shared
```

e classifica resultados em dois grandes grupos:

```text
Language
LOGIC
```

O grupo `LOGIC` é o mais importante para bugs funcionais.

## 4.1. Problemas detectados

A auditoria encontrou um volume muito grande de strings suspeitas.

Entre os resultados observados:

- milhares de linhas suspeitas;
- milhares fora de `Language`;
- centenas de recuperações `HIGH` dentro de lógica;
- forte presença em:
  - `fileread.cpp`;
  - `record.cpp`;
  - `OnSever.cpp`;
  - crafting;
  - item;
  - monster/spawn;
  - NPC;
  - shop;
  - warehouse.

---

## 4.2. Limitação descoberta: candidatos LOW

Candidatos `LOW` podem ser completamente errados.

Exemplo conceitual:

```text
target corrompido:
"���"

candidatos LOW:
돈
없음
메카닉
...
```

Isto ocorre porque sem contexto suficiente várias linhas antigas possuem estruturas semelhantes.

Regra:

```text
HIGH    -> evidência forte, ainda revisar
MEDIUM  -> pista
LOW     -> somente indicação de onde investigar
```

Nunca executar substituição automática baseada apenas em `LOW`.

---

# 5. Falsos positivos em arquivos binários

Durante a investigação foram procurados bytes CP949 de palavras como:

```text
모두
없음
마을
필드
```

em assets.

Alguns `.smb` apresentaram a sequência de bytes correspondente a `없음`.

Inicialmente isso parecia indicar que os tokens da DSL estavam embutidos nos arquivos.

A análise do contexto mostrou que os bytes apareciam no meio de estruturas binárias, provavelmente floats/matrizes/vértices, sem:

- delimitador;
- string NUL coerente;
- contexto textual;
- estrutura de configuração.

Conclusão:

> Encontrar uma sequência CP949 dentro de um binário não prova que ela seja uma string.

A auditoria futura deve distinguir:

```text
TEXT_MATCH
LIKELY_BINARY_STRING
RAW_BINARY_SEQUENCE
```

---

# 6. Descoberta do cache `.inx`

`fileread.cpp` mostrou que a saída serializada de `smMODELINFO` utiliza extensão:

```cpp
static char *szFileExt = "inx";
```

A cadeia observada é:

```text
arquivo textual de definição
        ↓
AddModelDecode()
        ↓
smMODELINFO
        ↓
ModelKeyWordEncode / MotionKeyWordEncode
        ↓
fwrite(sizeof(smMODELINFO))
        ↓
arquivo .inx
```

O loader também valida o tamanho:

```text
dwFileLen == sizeof(smMODELINFO)
```

e pode carregar o cache diretamente.

Foram observados milhares de `.inx`.

Consequência crítica:

> Alterar o parser textual não implica que todos os assets existentes serão reprocessados.

Isso também significa que não devemos regenerar `.inx` em massa durante uma investigação.

---

# 7. DSL de model/motion recuperada do Master

No PSTale Master foi recuperada a tabela original:

```text
0  *동작파일
1  *동작모음
2  *모양파일
3  *모양이름
4  *파일연결
5  *보조동작파일
6  *정밀모양
7  *보통모양
8  *저질모양
9  *XX걷는동작
10 *XX서있기동작
11 *착용무기
12 *적용직업
13 *해당위치
14 *적용기술
15 *표정파일연결
16 *표정파일
17 *표정모음
```

Proposta semântica de documentação:

| ID | Legacy | Nome técnico sugerido |
|---:|---|---|
| 0 | `*동작파일` | `*MOTION_FILE` |
| 1 | `*동작모음` | `*MOTION_SET` |
| 2 | `*모양파일` | `*MODEL_FILE` |
| 3 | `*모양이름` | `*MODEL_NAME` |
| 4 | `*파일연결` | `*FILE_LINK` |
| 5 | `*보조동작파일` | `*AUX_MOTION_FILE` |
| 6 | `*정밀모양` | `*DETAIL_MODEL` |
| 7 | `*보통모양` | `*DEFAULT_MODEL` |
| 8 | `*저질모양` | `*LOW_MODEL` |
| 9 | `*XX걷는동작` | unresolved legacy |
| 10 | `*XX서있기동작` | unresolved legacy |
| 11 | `*착용무기` | `*EQUIPPED_WEAPON` |
| 12 | `*적용직업` | `*APPLICABLE_JOB` |
| 13 | `*해당위치` | `*LOCATION` |
| 14 | `*적용기술` | `*APPLICABLE_SKILL` |
| 15 | `*표정파일연결` | `*FACE_FILE_LINK` |
| 16 | `*표정파일` | `*FACE_FILE` |
| 17 | `*표정모음` | `*FACE_SET` |

---

# 8. Tokens internos da DSL

Também foram recuperados tokens auxiliares:

```text
반복
동작
뛰는
*서있기
*정지
*걷는
공격
죽기
죽는
타격
데미지
막기
울기
워프
가끔
떨어
큰착지
작은착지
다시시작
먹기
기술
만세
```

E uma sublinguagem de expressões:

```text
표정
아표정
이표정
오표정
눈깜빡표정
무표정(작동않함)
웃는표정
화난표정
슬픈표정
놀란표정
고유표정
특별표정
```

Esses tokens ainda **não foram migrados**.

É importante manter o escopo dos fixes pequeno.

---

# 9. Bugfix 049 — escopo

O bugfix 049 foi aberto para tratar somente:

```text
case 11
case 12
case 13
case 14
```

e os valores especiais usados nos cases 11 e 13.

## 9.1. Comandos alterados

Antes, no Summit, os comandos estavam visualmente destruídos.

Depois:

```cpp
"*EQUIPPED_WEAPON",
"*APPLICABLE_JOB",
"*LOCATION",
"*APPLICABLE_SKILL",
```

em:

```text
SrcGame/src/fileread.cpp
SrcServer/src/fileread.cpp
```

---

## 9.2. Case 11 — equipamento / categorias de item

Master:

```text
*착용무기
```

Valores especiais:

```text
모두
없음
```

Bytes CP949 confirmados:

```text
*착용무기 = 2a c2 f8 bf eb b9 ab b1 e2
모두      = b8 f0 b5 ce
없음      = be f8 c0 bd
```

O comportamento legado observado é:

```cpp
if (ALL)
{
    ItemCodeCount = 0;
    break;
}

if (NONE)
{
    ItemCodeList[ItemCodeCount++] = 0xFF;
}
else
{
    procurar sItem[cnt].LastCategory
}
```

Não interpretar semanticamente `0xFF` pelo nome. A correspondência foi recuperada do Master, portanto a ordem histórica é a fonte de verdade.

---

## 9.3. Case 12 — profissão

Comando:

```text
*적용직업 -> *APPLICABLE_JOB
```

O conteúdo interno já utiliza identificadores ASCII:

```text
Fighter
Mechanician
Archer
Pikeman
Atalanta
Knight
Magician
Priestess
...
```

Isto foi uma evidência importante de que a própria implementação original já misturava:

```text
comando coreano
+
identificador técnico inglês
```

Portanto mover a camada de controle para ASCII é consistente com a arquitetura existente.

---

## 9.4. Case 13 — localização

Master:

```text
*해당위치
```

Valores:

```text
마을 -> TOWN
필드 -> FIELD
```

Bytes CP949:

```text
*해당위치 = 2a c7 d8 b4 e7 c0 a7 c4 a1
마을      = b8 b6 c0 bb
필드      = c7 ca b5 e5
```

Comportamento:

```text
TOWN  -> bit 1
FIELD -> bit 2
```

Quando nenhum bit é identificado, ou em determinadas motions de skill:

```text
MapPosition = 3
```

---

## 9.5. Case 14 — skill

Comando:

```text
*적용기술 -> *APPLICABLE_SKILL
```

O conteúdo é comparado contra:

```cpp
SkillDataCode[cnt].szSkillName
```

Ou seja, novamente a keyword estrutural estava em coreano, enquanto os identificadores consumidos pelo sistema são técnicos.

---

# 10. `LastCategory` — decisão importante

Durante o case 11 surgiu a dúvida se `sItem[].LastCategory` deveria ser traduzido.

A investigação mostrou que **não**.

`LastCategory` é usado em vários subsistemas como identificador técnico:

```text
lookup de item
shop
trade
inventory
costume/present
asset path
```

Há inclusive construção de paths no estilo:

```cpp
"...//Items//%s//it%s.bmp",
sItem[j].ItemFilePath,
sItem[j].LastCategory
```

Conclusão:

> `LastCategory` não pertence à localization e não deve ser renomeado sem uma auditoria independente de todos os consumidores.

O bugfix 049 preservou esse campo sem alteração.

---

# 11. Validações feitas no bugfix 049

Depois da alteração:

```text
SrcGame case 11 U+FFFD = 0
SrcGame case 13 U+FFFD = 0

SrcServer case 11 U+FFFD = 0
SrcServer case 13 U+FFFD = 0
```

O grep confirmou:

```text
*EQUIPPED_WEAPON
*APPLICABLE_JOB
*LOCATION
*APPLICABLE_SKILL

ALL
NONE
TOWN
FIELD
```

nos dois parsers.

A estrutura ao redor dos cases não foi alterada.

---

# 12. Resultado funcional atual

Após compilar/testar:

```text
servidor não crashou
cliente/servidor iniciaram
itens não aparecem
```

Este é o estado mais importante deste documento.

Não interpretar “não crashou” como “fix resolvido”.

O 049 demonstrou apenas que:

1. os quatro comandos podem ser representados em ASCII;
2. os cases compilam e executam sem crash imediato;
3. a corrupção desses literais específicos foi removida;
4. o desaparecimento dos itens possui outra causa ou uma causa adicional.

---

# 13. O que sabemos agora

## Confirmado

- O source Summit contém `U+FFFD` dentro de lógica.
- O PSTale Master preserva vários originais CP949.
- Existem keywords coreanas usadas como comandos internos.
- `fileread.cpp` é um parser de DSL de model/motion.
- `CmdNum` é escolhido comparando contra `szIniCommand[]`.
- `case 11` manipula `ItemCodeCount` e `ItemCodeList`.
- `case 13` manipula `MapPosition`.
- `case 12` utiliza nomes de jobs ASCII.
- `case 14` utiliza nomes técnicos de skills.
- `.inx` é cache/binário de `smMODELINFO`.
- Existem milhares de `.inx`.
- Matches CP949 em `.smb` podem ser falsos positivos binários.
- `LastCategory` é um identificador técnico usado em vários sistemas.
- O fix 049 não causa crash imediato.
- Os itens continuam não aparecendo.

---

# 14. O que NÃO sabemos ainda

Não sabemos ainda:

- quantos itens o servidor realmente carregou;
- se `sItem[]` está completamente vazio ou apenas parcialmente preenchido;
- em qual função a tabela de itens é populada;
- se o parser principal do item está falhando antes do fim;
- se os códigos (`CODE`) estão sendo preenchidos;
- se `LastCategory` está preenchido;
- se `ItemFilePath` está preenchido;
- se existem duplicatas;
- se o servidor possui a mesma contagem esperada do cliente;
- se algum arquivo de definição de item está ausente;
- se há outro token estrutural corrompido bloqueando o parser;
- se o problema ocorre ao carregar, transmitir ou apresentar os itens;
- se o sistema usa cache derivado que precisa ser invalidado;
- se o problema está relacionado a `record.cpp`, `sinItem.cpp`, `fileread.cpp` ou outro loader.

Portanto a próxima etapa deve ser observabilidade, não mais substituição.

---

# 15. Próximo passo obrigatório — instrumentação do carregamento de itens no servidor

## 15.1. Objetivo

Responder com evidência:

```text
Quantos itens o servidor carregou?
Quais itens?
Qual foi o primeiro item?
Qual foi o último?
Há buracos?
Há duplicatas?
LastCategory está vazio?
CODE está vazio?
Em qual etapa a tabela deixa de ser preenchida?
```

---

# 16. Filosofia da instrumentação

A instrumentação deve:

```text
observar
não alterar comportamento
não criar fallback
não fabricar item
não corrigir automaticamente
não esconder erro
```

O próximo fix deve ser tratado como diagnóstico.

Nome sugerido:

```text
bugfixes/050_server_item_loading_instrumentation
```

ou, se a equipe quiser separar diagnóstico de fix:

```text
audits/050_server_item_loading_runtime
```

Preferência: manter como auditoria enquanto não houver alteração funcional.

---

# 17. Onde instrumentar

A investigação deve localizar três pontos.

## Ponto A — início do carregamento

Exemplo conceitual:

```cpp
LoadItemTable(...)
{
    LOG("begin item load");
```

Registrar:

```text
arquivo de entrada
path resolvido
size
open success/failure
```

---

## Ponto B — criação/população de cada item

Quando um item é aceito:

```text
index
CODE
LastCategory
ItemFilePath
nome técnico disponível
```

Não registrar milhares de linhas por padrão.

Estratégia:

```text
primeiros 20
últimos 20
erros
duplicatas
resumo final
```

---

## Ponto C — fim do carregamento

Gerar uma linha-resumo:

```text
ITEM_LOAD_SUMMARY
loaded=N
empty=M
duplicate_code=X
duplicate_category=Y
invalid=Z
```

---

# 18. Métricas mínimas

A instrumentação deve produzir pelo menos:

```text
MAX_ITEM
loaded_count
nonzero_code_count
empty_lastcategory_count
empty_filepath_count
duplicate_code_count
duplicate_lastcategory_count
highest_loaded_index
```

Se existirem campos relevantes:

```text
item name
type
class restriction
level
price
weight
```

podem ser incluídos somente na amostra, não em todas as linhas.

---

# 19. Definição de “item carregado”

Não usar apenas:

```cpp
index < MAX_ITEM
```

Uma definição provisória melhor é:

```text
CODE != 0
OU
LastCategory[0] != 0
```

Mas a regra exata deve ser confirmada olhando a estrutura e inicialização de `sItem`.

Instrumentação deve registrar os dois critérios para que não escondamos diferenças históricas.

---

# 20. Duplicatas

É importante procurar:

```text
mesmo CODE em índices diferentes
mesmo LastCategory em índices diferentes
```

porque um parser corrompido pode:

- sobrescrever entradas;
- gerar categorias iguais;
- deixar códigos default;
- quebrar lookup mesmo quando a contagem parece correta.

---

# 21. Logs sugeridos

Evitar:

```text
printf por frame
printf dentro de loops runtime frequentes
```

Preferir log apenas no startup/load.

Formato:

```text
[ITEMAUDIT] BEGIN path=...
[ITEMAUDIT] ITEM idx=0 code=... category=... path=...
[ITEMAUDIT] ITEM idx=1 code=... category=... path=...
...
[ITEMAUDIT] WARNING duplicate_code ...
[ITEMAUDIT] SUMMARY loaded=... empty=... highest=...
```

---

# 22. Limite de volume

O log não deve despejar todos os itens por padrão.

Sugestão:

```text
sample_first = 25
sample_last  = 25
all_errors   = true
summary      = true
```

Se for necessário dump completo:

```text
ITEM_AUDIT_VERBOSE
```

como flag temporária.

---

# 23. Log em arquivo

Preferência:

```text
logs/item_load_audit.log
```

ou arquivo já usado pelo servidor.

Evitar inventar um segundo sistema de logging se o servidor já possui logger funcional.

Primeiro localizar:

```bash
grep -RniE \
  --include='*.cpp' \
  --include='*.h' \
  'Log|WriteLog|ServerLog|ErrorLog|DebugLog' \
  SrcServer/src 2>/dev/null |
head -300
```

Depois reutilizar a infraestrutura mais simples e estável.

---

# 24. Pesquisa inicial para a instrumentação

Executar na próxima sessão:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"

grep -RniE \
  --include='*.cpp' \
  --include='*.h' \
  'sITEM[[:space:]]+sItem|sItem\[MAX_ITEM\]|MAX_ITEM' \
  SrcServer/src 2>/dev/null |
head -300
```

Depois:

```bash
grep -RniE \
  --include='*.cpp' \
  --include='*.h' \
  'LastCategory|ItemFilePath|\.CODE' \
  SrcServer/src/sinbaram \
  SrcServer/src/fileread.cpp \
  SrcServer/src/record.cpp \
  2>/dev/null |
head -500
```

Objetivo: achar quem **escreve**, não quem só lê.

---

# 25. Procurar writes em `sItem`

Priorizar padrões:

```bash
grep -RniE \
  --include='*.cpp' \
  --include='*.h' \
  '(strcpy|strcpy_s|lstrcpy|memcpy|sscanf|fscanf|GetWord|GetString).*sItem|sItem\[[^]]+\]\.[A-Za-z_]+[[:space:]]*=' \
  SrcServer/src 2>/dev/null |
head -500
```

Depois revisar manualmente somente os arquivos candidatos.

---

# 26. Contagem runtime sugerida

Pseudo-código:

```cpp
int loaded = 0;
int emptyCategory = 0;
int emptyCode = 0;
int highest = -1;

for (int i = 0; i < MAX_ITEM; ++i)
{
    bool hasCode = sItem[i].CODE != 0;
    bool hasCategory = sItem[i].LastCategory[0] != 0;

    if (hasCode || hasCategory)
    {
        loaded++;
        highest = i;
    }

    if (!hasCode)
        emptyCode++;

    if (!hasCategory)
        emptyCategory++;
}
```

A regra final deve respeitar os tipos reais.

---

# 27. Dump de amostra

Pseudo-código:

```cpp
for (int i = 0; i < MAX_ITEM; ++i)
{
    if (!IsLoadedItem(sItem[i]))
        continue;

    if (printed < 25)
    {
        Log(
            "[ITEMAUDIT] idx=%d code=%08X category=%s path=%s",
            i,
            sItem[i].CODE,
            sItem[i].LastCategory,
            sItem[i].ItemFilePath
        );

        printed++;
    }
}
```

Não assumir ainda que `CODE` é `DWORD`; usar o tipo real.

---

# 28. Instrumentação em etapas

Não inserir tudo de uma vez.

## Etapa 1

Somente:

```text
BEGIN
END
COUNT
```

Se count = 0:

```text
investigar parser/load anterior
```

Se count > 0:

```text
investigar transmissão/uso posterior
```

Essa bifurcação economiza muito tempo.

---

## Etapa 2

Se count = 0:

adicionar logs em checkpoints dentro do loader:

```text
file opened
header read
first record parsed
records accepted
end reached
```

---

## Etapa 3

Se count > 0:

instrumentar consumidores:

```text
shop
warehouse
NPC item list
packet send
client receive
```

---

# 29. Árvore de decisão

```text
Servidor inicia
    |
    v
ITEM_LOAD_SUMMARY
    |
    +-- loaded == 0
    |      |
    |      +--> problema no loader/parser/path/input
    |
    +-- loaded > 0
           |
           v
      shop/NPC list count
           |
           +-- zero
           |     |
           |     +--> problema na seleção / lookup / NPC config
           |
           +-- > 0
                 |
                 v
            packet enviado?
                 |
                 +-- não -> servidor/protocolo
                 |
                 +-- sim
                       |
                       v
                  cliente recebeu?
                       |
                       +-- não -> protocolo/socket/opcode
                       |
                       +-- sim -> UI/client item rendering
```

Essa árvore deve orientar os próximos fixes.

---

# 30. Relação com NPC/shop/warehouse

Anteriormente um fix de path/literal resolveu parcialmente NPCs que não abriam shop/warehouse.

Isso mostra que:

```text
NPC existe
```

não implica:

```text
configuração do NPC carregou corretamente
```

e:

```text
janela abriu
```

não implica:

```text
lista de itens foi carregada
```

A instrumentação deve separar essas fases.

---

# 31. Riscos atuais do working tree

O `git status` observado contém muitas alterações que não pertencem ao bugfix atual, incluindo:

```text
SourceCode_rebase_20260824
analysis/
binary-analysis/
docker/
documentation/
web/
...
```

Regra obrigatória:

> Nunca usar `git add .` para fechar estes bugfixes.

Sempre:

```bash
git add \
  SourceCode/SrcGame/src/fileread.cpp \
  SourceCode/SrcServer/src/fileread.cpp \
  bugfixes/049_motion_parser_ascii_structural_tokens
```

ou lista explícita equivalente.

Antes de commit:

```bash
git diff --cached --name-only
```

e revisar todos os nomes.

---

# 32. Política de bugfix

Cada fix deve continuar seguindo o formato:

```text
bugfixes/NNN_nome/
├── before/
├── after/
├── evidence/
├── base_commit.txt
├── hashes_before.sha256
├── hashes_after.sha256
├── before_after.diff
└── README.md
```

---

# 33. Conteúdo mínimo do README de cada fix

```text
Sintoma
Hipótese
Evidência
Root cause
Arquivos alterados
Mudança
Validações estáticas
Build
Teste runtime
Resultado
Riscos
Rollback
Próximo passo
```

---

# 34. Critério para fechar um bug

Não fechar apenas porque:

```text
compilou
```

ou:

```text
não crashou
```

Fechar somente quando houver uma prova ligada ao sintoma.

Exemplo:

```text
bug: itens não aparecem

evidência de resolução:
loaded_count correto
shop_count correto
packet_count correto
itens visíveis
```

---

# 35. Estado recomendado do bugfix 049

O 049 deve ser tratado como:

```text
IMPLEMENTED
BUILD/RUNTIME PARTIAL VALIDATION
NOT PROVEN AS ITEM FIX
```

Ele pode ser válido como limpeza funcional da DSL, mas não deve ser descrito como a solução para o desaparecimento dos itens.

---

# 36. Próximos bugs relacionados já encontrados

O grep revelou outros tokens estruturais corrompidos, incluindo grupos do tipo:

```text
*PK.......
```

com aliases ASCII já existentes como:

```text
*PK_FIELD
*PK_FIELD_ALL
```

Isto é uma evidência muito interessante porque mostra outra parte do source em migração parcial:

```text
legacy token corrompido
OR
ASCII alias
```

Mas esses tokens **não devem ser corrigidos antes de medir os itens**.

Prioridade atual:

```text
1. item loading instrumentation
2. item pipeline
3. NPC/shop/warehouse item selection
4. só depois voltar para outros tokens estruturais
```

---

# 37. Melhorias futuras da auditoria

A auditoria de encoding deve receber novos modos:

## `--logic-only`

Ignorar `Language`.

## `--comparisons-only`

Procurar:

```text
strcmp
lstrcmp
lstrcmpi
strstr
strchr
Find*
Get*
Search*
```

## `--binary-context`

Em binários, classificar o contexto.

## `--sibling-priority`

Quando:

```text
SrcGame corrompido
SrcServer preservado
```

ou vice-versa, isso deve ter prioridade maior que fontes externas.

Classificação sugerida:

```text
SIBLING_EXACT
MASTER_EXACT
SECONDARY_EXACT
STRUCTURAL_HIGH
LOW
```

---

# 38. Estratégia para encoding daqui para frente

## Para arquivos Language

Preservar o idioma e recuperar encoding conscientemente.

## Para lógica nova

Usar ASCII/inglês.

## Para compatibilidade com dados antigos

Se necessário:

```text
legacy token bytes
OR
ASCII token
```

mas somente quando existir um consumidor real do formato legado.

## Para identificadores como `LastCategory`

Não traduzir sem auditoria de consumidores.

---

# 39. Não fazer

Evitar:

```text
iconv recursivo no SourceCode
sed global em mojibake
substituição automática de HIGH
substituição automática de LOW
regenerar todos os .inx
converter todos os literals coreanos
traduzir LastCategory
git add .
```

---

# 40. Fazer

Preferir:

```text
evidence first
byte inspection
Master comparison
small patch
before/after
hashes
build
runtime observation
instrumentation
```

---

# 41. Próxima sessão — ordem exata

A próxima sessão deve começar assim:

### Passo 1

Localizar o loader da tabela `sItem`.

### Passo 2

Descobrir o ponto onde o carregamento termina.

### Passo 3

Adicionar somente:

```text
BEGIN
SUMMARY
```

### Passo 4

Compilar servidor.

### Passo 5

Iniciar servidor.

### Passo 6

Capturar:

```text
loaded_count
nonzero_code_count
nonempty_category_count
highest_index
```

### Passo 7

Decidir o próximo ramo:

```text
loaded == 0
ou
loaded > 0
```

Não fazer outra correção antes dessa medição.

---

# 42. Resultado esperado da próxima instrumentação

Exemplo saudável:

```text
[ITEMAUDIT] BEGIN
[ITEMAUDIT] SUMMARY max=XXXX loaded=XXXX code=XXXX category=XXXX highest=XXXX
```

Exemplo de falha clara:

```text
[ITEMAUDIT] BEGIN
[ITEMAUDIT] SUMMARY max=XXXX loaded=0 code=0 category=0 highest=-1
```

O segundo caso provaria que o problema está antes de shop/NPC/UI.

---

# 43. Se itens estiverem carregados

Se:

```text
loaded > 0
```

o próximo ponto é medir uma loja/NPC específico:

```text
NPC name/code
shop record
item count
item indices/codes
```

Depois medir o packet.

---

# 44. Se itens NÃO estiverem carregados

Se:

```text
loaded == 0
```

investigar:

```text
arquivo de itens
path resolvido
fopen
GetWord/GetString
parser token
record count
first accepted record
early return
```

Nesse cenário, `fileread.cpp` volta a ser o alvo principal.

---

# 45. Handoff resumido

```text
PROBLEMA ATUAL:
itens não aparecem.

O QUE JÁ FOI FEITO:
- auditoria de encoding;
- PSTale Master identificado como fonte principal;
- DSL coreana de model/motion recuperada;
- falsos positivos binários identificados;
- .inx identificado como cache de smMODELINFO;
- LastCategory classificado como identificador técnico;
- bugfix 049 normalizou comandos 11-14 e valores especiais para ASCII;
- build/runtime sem crash imediato.

O QUE AINDA NÃO FOI PROVADO:
- se o servidor carrega zero, alguns ou todos os itens.

PRÓXIMA AÇÃO:
instrumentar o carregamento de sItem no servidor.

REGRA:
não fazer outra migração de encoding antes de obter ITEM_LOAD_SUMMARY.
```

---

# 46. Conclusão

A investigação mostrou que o problema de encoding no Summit Tale não deve ser tratado como “corrigir caracteres quebrados”.

Existem pelo menos três classes diferentes:

```text
1. localization visual
2. keywords estruturais de parser
3. identificadores técnicos compartilhados entre subsistemas
```

Cada classe exige uma estratégia diferente.

A descoberta mais importante até aqui é que o jogo possui lógica estrutural historicamente dependente de coreano/CP949. Porém a migração deve ser incremental e baseada em comportamento.

O bugfix 049 removeu uma parte dessa dependência sem causar crash, mas os itens ainda não aparecem.

Neste ponto, continuar alterando strings sem observar o runtime seria voltar a trabalhar por hipótese.

O próximo avanço deve vir de telemetria simples e objetiva:

> **Quantos itens o servidor realmente carregou?**

A resposta a essa pergunta determina todo o restante da investigação.
## Confirmação da corrupção do parser de itens — comparação binária com o master

### Objetivo

Depois de confirmar que os arquivos `OpenItem`, como `WA101.txt`, contêm
tokens coreanos CP949 válidos, foi necessário determinar se o problema estava
nos dados carregados pelo servidor ou nos literais utilizados pelo parser
`DecodeItemInfo()`.

Foi realizada uma comparação binária entre:

- `pristontale-1-master/src/fileread.cpp`
- `SourceCode/SrcServer/src/fileread.cpp`
- `SourceCode/SrcGame/src/fileread.cpp`

A comparação procurou:

1. ocorrências do caractere Unicode de substituição U+FFFD em sua
   representação UTF-8 `EF BF BD`;
2. tokens conhecidos do formato OpenItem codificados diretamente em CP949;
3. ocorrências de sequências textuais `\x2A`, provenientes de tentativas de
   representar os tokens através de escapes hexadecimais.

---

### Resultado

#### pristontale-1-master

O `fileread.cpp` do master apresenta:

- tamanho: 196513 bytes;
- ocorrências de `EF BF BD`: **0**;
- ocorrências literais de `\x2A`: **0**.

Todos os tokens CP949 testados foram encontrados diretamente no arquivo:

| Token | Estado |
|---|---|
| `*코드` | encontrado |
| `*이름` | encontrado |
| `*공격력` | encontrado |
| `*명중력` | encontrado |
| `*공격속도` | encontrado |
| `*크리티컬` | encontrado |
| `*내구력` | encontrado |
| `*무게` | encontrado |
| `*가격` | encontrado |
| `*연결파일` | encontrado |
| `*특화` | encontrado |
| `*특화랜덤` | encontrado |

Isso demonstra que o master preserva uma cópia binariamente íntegra dos
literais utilizados pelo parser original.

---

#### Summit Server

O `SrcServer/src/fileread.cpp` apresenta:

- tamanho: 153946 bytes;
- **1398 ocorrências de `EF BF BD`**;
- **94 ocorrências textuais de `\x2A`**.

Dos tokens analisados, apenas `*특화` ainda foi encontrado diretamente em
CP949.

Todos os seguintes tokens estavam ausentes em sua representação CP949
original:

- `*코드`
- `*이름`
- `*공격력`
- `*명중력`
- `*공격속도`
- `*크리티컬`
- `*내구력`
- `*무게`
- `*가격`
- `*연결파일`
- `*특화랜덤`

---

#### Summit Game

O `SrcGame/src/fileread.cpp` apresenta:

- tamanho: 155227 bytes;
- **1966 ocorrências de `EF BF BD`**;
- nenhuma ocorrência textual de `\x2A`.

Assim como no servidor, apenas `*특화` entre os tokens testados ainda existe
diretamente em CP949.

Os demais tokens testados estão ausentes.

---

## Conclusão técnica

A corrupção dos literais do `fileread.cpp` está agora comprovada
binariamente.

Não se trata apenas de:

- configuração incorreta do terminal;
- fonte utilizada pelo editor;
- interpretação incorreta feita pelo `file`;
- diferença de locale do Linux;
- problema exclusivo do compilador MinGW.

Os arquivos Summit contêm fisicamente sequências UTF-8:

    EF BF BD

correspondentes ao caractere Unicode U+FFFD (`�`).

U+FFFD é utilizado quando uma conversão Unicode não consegue representar ou
interpretar corretamente os bytes de origem.

Uma vez que o byte original foi substituído por U+FFFD, a informação original
não pode ser recuperada matematicamente a partir do arquivo Summit.

Entretanto, neste projeto a recuperação é possível porque o
`pristontale-1-master` preserva os bytes CP949 originais.

---

## Relação com a falha do sistema de itens

Os arquivos OpenItem analisados continuam utilizando os tokens históricos
CP949.

Por exemplo, `WA101.txt` contém:

    *코드
    *이름
    *공격력
    *명중력
    *공격속도
    *크리티컬
    *내구력
    *무게
    *가격
    *연결파일

Os bytes desses tokens correspondem corretamente ao CP949 esperado.

O parser `DecodeItemInfo()` utiliza comparações de strings para reconhecer
esses comandos.

Portanto, quando o arquivo contém:

    2A C4 DA B5 E5

correspondente a:

    *코드

mas o literal correspondente dentro do executável foi compilado a partir de
uma string já destruída contendo `EF BF BD`, a comparação não pode ser
verdadeira.

Consequentemente, campos importantes da estrutura do item deixam de ser
preenchidos.

Esse mecanismo fornece uma explicação técnica direta para o comportamento
observado durante a inicialização:

- arquivos OpenItem são encontrados;
- os arquivos podem ser abertos;
- o servidor não necessariamente sofre crash;
- porém o conteúdo não é interpretado corretamente;
- itens não são registrados adequadamente;
- sistemas dependentes da tabela de itens podem posteriormente operar com
  dados ausentes ou incompletos.

Isso também explica por que a falha pôde permanecer silenciosa durante tanto
tempo.

---

## Descoberta adicional: estado inconsistente das tentativas de recuperação

O `SrcServer/src/fileread.cpp` contém 94 ocorrências textuais de `\x2A`.

Isso indica que pelo menos parte do arquivo já passou por alguma tentativa de
recuperação ou substituição baseada em escapes hexadecimais.

Essa situação deve ser tratada separadamente.

É importante distinguir:

    "\x2A\xC4\xDA..."

em código C/C++, que gera bytes específicos durante a compilação,

de um arquivo contendo incorretamente uma sequência que posteriormente seja
tratada como texto literal.

Antes de qualquer recuperação em massa, essas 94 ocorrências precisam ser
classificadas para determinar quais são intencionais e quais são resíduos de
alterações anteriores.

---

## Fonte de recuperação escolhida

Para `fileread.cpp`, a fonte primária de recuperação passa a ser:

    /home/gabriel/armazem/priston/pristontale-1-master/src/fileread.cpp

Motivos:

1. contém zero U+FFFD;
2. contém os tokens CP949 procurados;
3. preserva a implementação histórica do parser;
4. corresponde estruturalmente à linhagem do código Summit;
5. já forneceu evidências corretas para outros tokens analisados.

Outras versões do cliente e servidor permanecem fontes secundárias de
validação.

Elas devem ser utilizadas quando:

- o master não possuir determinado código;
- houver divergência estrutural significativa;
- existirem extensões posteriores ao master;
- não for possível identificar com segurança o literal original.

---

## Estratégia de recuperação

Não substituir `fileread.cpp` inteiro pelo arquivo do master.

O Summit contém alterações posteriores importantes, incluindo:

- correções já realizadas;
- alterações específicas de cliente/servidor;
- APIs mais seguras;
- mudanças de build;
- instrumentação;
- possíveis funcionalidades adicionadas depois da versão master.

A recuperação deve ocorrer no nível dos dados perdidos.

Fluxo recomendado:

    Summit atual
         |
         +-- identificar literal contendo U+FFFD
         |
         +-- localizar contexto estrutural
         |
         +-- procurar equivalente no master
         |
         +-- validar correspondência
         |
         +-- recuperar valor original
         |
         +-- preservar código Summit ao redor
         |
         +-- compilar
         |
         +-- instrumentar
         |
         +-- validar em runtime

Nenhuma recuperação deve ser aplicada automaticamente apenas porque duas
linhas parecem semelhantes.

---

## Política de encoding proposta

A investigação também reforça uma decisão arquitetural importante.

A lógica moderna do projeto deve tender a utilizar identificadores e comandos
internos em inglês/ASCII.

Idiomas devem permanecer nos recursos destinados ao usuário.

Exemplo conceitual:

    parser / protocolo / configuração interna
        -> ASCII / inglês

    Language/Korean
        -> coreano

    Language/Japanese
        -> japonês

    Language/Chinese
        -> chinês

Isso reduz a dependência da lógica do programa de code pages históricas.

Entretanto, essa migração NÃO deve ser misturada com a recuperação atual.

Primeiro precisamos restaurar o comportamento histórico e confirmar que o
servidor funciona.

Somente depois disso os formatos podem ser migrados de maneira controlada.

---

## Prioridade atual

A prioridade continua sendo corrigir os bugs existentes, e não modernizar
todo o sistema de localização.

Ordem atual:

1. recuperar o parser de itens;
2. instrumentar a pipeline;
3. comprovar carregamento dos itens;
4. testar lojas, armazém e sistemas dependentes;
5. identificar outros parsers quebrados por encoding;
6. recuperar esses parsers utilizando fontes históricas;
7. somente posteriormente planejar a migração para formatos internos
   independentes de idioma.

---

## Próximo passo imediato

O próximo alvo é `DecodeItemInfo()`.

Deve ser criada uma auditoria automática que compare os literais relevantes
do `DecodeItemInfo()` Summit com o master e produza uma tabela contendo:

- localização no Summit;
- bytes atuais;
- presença de U+FFFD;
- localização equivalente no master;
- bytes do master;
- interpretação CP949;
- nível de confiança da correspondência;
- estado da recuperação.

Somente depois dessa tabela devemos aplicar a recuperação do parser completo.

Após a recuperação:

1. recompilar o servidor;
2. iniciar com a instrumentação da pipeline;
3. registrar quantidade de arquivos OpenItem processados;
4. registrar quantidade de itens efetivamente decodificados;
5. registrar códigos dos primeiros itens;
6. registrar falhas de `DecodeItemInfo`;
7. confirmar presença de `WA101`;
8. confirmar população das tabelas globais de itens;
9. testar NPC shops e warehouse.

Essa instrumentação será a prova de runtime de que a recuperação resolveu a
pipeline, em vez de apenas fazer o servidor compilar.