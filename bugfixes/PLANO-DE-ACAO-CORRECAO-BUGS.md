# Plano de ação para correção dos bugs encontrados

**Projeto:** CLIENT / SERVER legado C/C++  
**Base da análise:** primeira bateria de revisão profunda  
**Escopo deste plano:** BUG-001 a BUG-014 + saneamento sistêmico associado ao BUG-010  
**Objetivo:** eliminar primeiro os riscos de execução remota/corrupção de memória, depois estabilizar parsers, ownership de memória e divergências CLIENT/SERVER.

---

## 1. Resumo executivo

A ordem de correção não deve seguir simplesmente a numeração dos bugs.

Há dependências importantes:

- `BUG-001` precisa ser corrigido antes de qualquer manutenção do mecanismo do `BUG-002`.
- `BUG-003` a `BUG-006` pertencem ao mesmo subsistema (`CurseFilter`) e devem ser tratados em um único pacote de mudança.
- `BUG-011` é uma implementação de string fundamentalmente insegura; enquanto ela permanecer ativa, crashes em áreas aparentemente independentes podem continuar acontecendo.
- `BUG-007`, `BUG-008` e `BUG-009` pertencem ao loader de animações e devem ser corrigidos juntos.
- `BUG-012` deve ser tratado como uma campanha de revisão, não como substituição global cega.
- `BUG-010` possui dezenas de ocorrências e precisa de revisão incremental, por arquivo/subsistema.

### Ordem recomendada

| Prioridade | Grupo | Bugs | Motivo |
|---|---|---|---|
| P0 | Fronteira de rede | BUG-001, BUG-002 | Corrupção de memória e execução de código recebido |
| P0 | CurseFilter | BUG-003, BUG-004, BUG-005, BUG-006 | Vários UB/overflows no mesmo fluxo |
| P1 | Classe de string | BUG-011 | Corrupção/leak potencialmente disseminados |
| P1 | Loader de animação | BUG-007, BUG-008, BUG-009 | Escrita fora de heap/stack e parsing inválido |
| P1 | Literais `'/0'` | BUG-012 | 47 ocorrências no CLIENT; strings podem não terminar |
| P2 | Parsing de efeitos | BUG-014 | String não terminada / leitura fora de limites |
| P2 | Estado não inicializado | BUG-013 | Undefined behavior e possível null dereference |
| P1-P3 | Ownership sistêmico | BUG-010 | 75 pares candidatos; corrigir por lotes |

---

# 2. Estratégia geral de execução

## Fase 0 — Preparação e proteção contra regressões

Antes das correções:

1. Criar uma branch exclusiva, por exemplo:
   `fix/security-memory-stability`.
2. Registrar uma build CLIENT e SERVER de referência.
3. Guardar hashes dos binários de referência.
4. Separar mudanças por subsistema:
   - rede/damage;
   - CurseFilter;
   - sinSTRING;
   - HoAnim;
   - AssaEffect;
   - ownership/new-delete;
   - autenticação/parsers afetados por `'/0'`.
5. Evitar um único commit gigantesco.
6. Para cada grupo, produzir:
   - commit de correção;
   - commit de testes/fixtures quando possível;
   - registro de arquivos modificados.

### Instrumentação recomendada

Se a toolchain permitir:

- AddressSanitizer (`/fsanitize=address`) em builds de teste;
- `/W4` ou nível de warning equivalente;
- `/analyze` do MSVC;
- runtime checks (`/RTC1`) em Debug quando compatível.

Para builds legadas que não aceitem ASan:

- Application Verifier;
- PageHeap / GFlags;
- debugger com break em heap corruption;
- builds Debug com heap checks;
- testes com inputs propositalmente inválidos.

### Regra de aceite global

Uma correção não deve ser considerada concluída apenas porque “compila”.

Ela deve satisfazer:

- CLIENT compila;
- SERVER compila;
- fluxo funcional afetado ainda funciona;
- casos inválidos são rejeitados sem crash;
- nenhuma nova ocorrência equivalente é introduzida;
- warnings relevantes da área modificada são revisados;
- alteração CLIENT/SERVER é comparada quando houver código duplicado.

---

# 3. Fase 1 — Fronteira de rede e código dinâmico

## BUG-001 — Cópia de pacote sem validação de tamanho

**Prioridade:** P0  
**Severidade:** Crítica  
**Arquivos principais:**

- `SrcGame/src/Damage.cpp`
- `SrcServer/src/Damage.cpp`
- `Shared/smPacket.h`

### Objetivo

Garantir que nenhum tamanho ou offset vindo do pacote seja usado para:

- alocação;
- `memcpy`;
- cálculo de ponteiro;
- chamada de função;

sem validação prévia.

### Ações

1. Identificar todas as funções que recebem `TRANS_FUNC_MEMORY`.
2. Criar uma rotina central de validação, evitando verificações diferentes em cada receptor.
3. Validar:
   - ponteiro do pacote;
   - quantidade real de bytes recebidos;
   - `size > 0`;
   - `Param[0] >= 0`;
   - `Param[0] <= sizeof(szData)`;
   - `Param[0] <= região realmente recebida`;
   - coerência entre `size` e payload;
   - `Param[1]` dentro da região válida;
   - `Param[2]` dentro da região válida.
4. Rejeitar o pacote antes da alocação se qualquer campo for inválido.
5. Evitar confiar apenas no emissor legítimo.
6. Revisar casts signed/unsigned para impedir bypass por valor negativo convertido para `size_t`.
7. Registrar erro de protocolo sem reutilizar os valores inválidos para formatar/copiar memória.

### Forma recomendada

Criar algo equivalente a:

```cpp
bool ValidateTransFuncMemory(
    const TRANS_FUNC_MEMORY* packet,
    size_t receivedBytes,
    size_t* payloadSize);
```

A função deve ser a única responsável por definir se o pacote é seguro para consumo.

### Testes mínimos

- `size = 0`;
- `size < 0`;
- `Param[0] = -1`;
- `Param[0] = 0`;
- `Param[0] = 6000`;
- `Param[0] = 6001`;
- `Param[0] > size`;
- `Param[1] < 0`;
- `Param[1] == payloadSize`;
- `Param[2] > payloadSize`;
- pacote truncado na rede;
- pacote válido conhecido.

### Critério de aceite

Nenhum `memcpy` ou offset de função desse fluxo pode ocorrer antes de uma validação baseada **também no número real de bytes recebidos**.

### Dependência

Executar antes/junto do BUG-002.

---

## BUG-002 — Código de máquina recebido pelo protocolo

**Prioridade:** P0  
**Severidade:** Crítica de arquitetura/segurança  
**Arquivos principais:**

- `SrcGame/src/Damage.cpp`
- `SrcServer/src/Server/Svr_Damge.cpp`

### Objetivo

Eliminar, ou pelo menos reduzir drasticamente, a capacidade do protocolo de transformar bytes vindos da rede diretamente em código executável.

### Plano preferencial

#### Opção A — Correção definitiva

Remover código executável do protocolo.

O servidor deve enviar apenas:

- dados;
- seeds;
- chaves;
- parâmetros;
- identificadores de algoritmo;
- versões.

O CLIENT deve executar funções que já façam parte do binário validado.

### Plano de transição, se a remoção imediata for inviável

1. Aplicar primeiro todas as validações do BUG-001.
2. Definir versões exatas dos blobs aceitos.
3. Calcular/verificar hash forte do conteúdo permitido.
4. Autenticar a origem do payload.
5. Rejeitar payload desconhecido.
6. Não executar diretamente memória gravável.
7. Se o Windows alvo exigir execução dinâmica:
   - alocar RW;
   - copiar;
   - validar;
   - alterar para RX;
   - nunca manter RWX.
8. Invalidar/liberar a região quando o módulo deixar de ser necessário.
9. Auditar todos os call sites dos ponteiros de função recebidos.

### Testes

- blob válido;
- um byte alterado no blob;
- versão desconhecida;
- offsets válidos e inválidos;
- payload truncado;
- reconexão/reenvio;
- servidor incompatível;
- teste com DEP/NX ativo.

### Critério de aceite

**Ideal:** nenhum código executável é transmitido.

**Transicional:** somente blobs previamente autenticados e explicitamente reconhecidos podem chegar à etapa de execução.

---

# 4. Fase 2 — Correção integral do CurseFilter

Os BUG-003 a BUG-006 devem ser tratados no mesmo ciclo porque compartilham buffers e parsing.

---

## BUG-003 — Delimitadores e terminação incorretos

**Prioridade:** P0  
**Arquivos:**

- `SrcGame/src/CurseFilter.cpp`
- `SrcServer/src/CurseFilter.cpp`

### Ações

1. Corrigir os literais que pretendem representar caracteres de controle:
   - `'/n'` → `'\n'`;
   - `'/t'` → `'\t'`;
   - `'/0'` → `'\0'`.
2. Comparar CLIENT e SERVER lado a lado.
3. Remover divergências acidentais entre as implementações.
4. Reescrever `getField()` para receber:
   - ponteiro de origem;
   - tamanho disponível;
   - destino;
   - capacidade do destino;
   - delimitador.
5. Nunca depender apenas de `'\0'` para garantir que a origem está dentro do pacote/arquivo.
6. Garantir avanço do ponteiro em todas as iterações.
7. Evitar a expressão compacta:
   ```cpp
   while(ptr = getField(...), line[0])
   ```
   e separar assignment/teste para facilitar validação.

### Testes

- arquivo vazio;
- uma linha;
- várias linhas;
- tabs;
- última linha sem newline;
- linhas vazias;
- campos maiores que o buffer;
- arquivo sem delimitador;
- input malformado.

### Critério de aceite

O parser deve:

- terminar;
- nunca escrever fora do destino;
- nunca ficar preso no mesmo ponteiro;
- produzir os mesmos campos em CLIENT e SERVER.

---

## BUG-004 — `sprintf` inseguro e capacity check tardio

**Prioridade:** P0  
**Arquivos:** mesmos do CurseFilter.

### Ações

1. Remover todos os:
   ```cpp
   sprintf(ptr, variable);
   ```
2. Não substituir apenas por:
   ```cpp
   sprintf(ptr, "%s", variable);
   ```
   porque isso não resolve capacidade.
3. Usar escrita limitada.
4. Calcular espaço restante **antes** da escrita.
5. Tratar retorno de `snprintf`/equivalente.
6. Garantir terminador NUL em caso de truncamento.
7. Revisar:
   - `curse_string`;
   - `curse_stringID`;
   - variantes Debug;
   - qualquer função semelhante no mesmo arquivo.
8. Se a toolchain não possuir `snprintf` adequado, criar helper seguro único.

### Critério de aceite

Nenhuma escrita de texto variável no filtro usa API sem capacidade explícita.

---

## BUG-005 — `filterChar()` lê `srcp - 1`

**Prioridade:** P0/P1

### Ações

1. Mover qualquer acesso a `srcp - 1` para depois de confirmar:
   ```cpp
   srcp > src
   ```
2. Adicionar capacidade do destino à assinatura.
3. Impedir escrita após `dstEnd`.
4. Definir comportamento para:
   - input vazio;
   - input iniciado por espaço;
   - input de 1 byte;
   - caracteres multibyte.
5. Revisar a lógica de DBCS/encoding original antes de simplificar.

### Testes

- `""`;
- `" "`;
- `" a"`;
- primeiro caractere presente em `chars`;
- strings no limite do buffer;
- caracteres de dois bytes usados pelo jogo.

### Critério de aceite

Nenhum caminho lê antes do primeiro byte ou escreve após o final do destino.

---

## BUG-006 — Overflow em `ConvertStringTHAI`

**Prioridade:** P0/P1

### Ações

1. Remover o loop baseado em `strcat`.
2. Determinar qual tamanho é semanticamente correto:
   - tamanho da palavra proibida (`curse`);
   - ou tamanho de outro trecho substituído.
3. Limitar explicitamente o número de `'*'`.
4. Aumentar buffer apenas se a semântica realmente exigir; não usar “buffer maior” como correção principal.
5. Preferir construção por índice:
   ```cpp
   memset(replace, '*', n);
   replace[n] = '\0';
   ```
6. Garantir `n < sizeof(replace)`.
7. Revisar substituição final no buffer original para capacidade.

### Critério de aceite

Nenhuma quantidade derivada da string completa pode escrever mais que a capacidade de `replace`.

---

# 5. Fase 3 — Corrigir `sinSTRING`

## BUG-011 — Ownership e capacidade quebrados

**Prioridade:** P1  
**Severidade:** Crítica/Alta  
**Arquivos:**

- `SrcGame/src/sinbaram/sinUtil.cpp`
- equivalente SERVER
- header onde `sinSTRING` é declarado

### Decisão necessária

Existem duas estratégias.

### Estratégia A — recomendada

Substituir a implementação interna por `std::string`, mantendo a interface externa quando necessário.

Benefícios:

- elimina `new[]`/`delete[]`;
- assignment redimensiona automaticamente;
- construtor vazio fica válido;
- copy semantics corretas;
- elimina leak estrutural.

### Estratégia B — compatibilidade ABI estrita

Se layout/ABI impedir `std::string`:

1. inicializar sempre `m_aString`;
2. usar `new char[1]` no vazio;
3. escrever `'\0'`;
4. destrutor usar `delete[]`;
5. copy constructor fazer deep copy;
6. assignment alocar buffer com tamanho suficiente;
7. tratar self-assignment;
8. tratar `nullptr`;
9. não usar `strcpy` em capacidade desconhecida;
10. documentar ownership.

### Testes obrigatórios

- construtor default + `GetLength`;
- construção de `nullptr`;
- cópia de string vazia;
- cópia de string preenchida;
- assignment curta → longa;
- assignment longa → curta;
- self-assignment;
- milhares de criações/destruições;
- uso em `std::vector`/containers, se houver;
- comparação de comportamento CLIENT/SERVER.

### Critério de aceite

A classe não pode:

- ler memória não inicializada;
- escrever por ponteiro não alocado;
- vazar em destruição;
- copiar para capacidade insuficiente.

### Observação

Corrigir BUG-012 nas ocorrências dentro de `sinSTRING` no mesmo patch.

---

# 6. Fase 4 — Loader de animações

Os BUG-007, BUG-008 e BUG-009 devem ser resolvidos em conjunto.

---

## BUG-007 — Último token truncado

**Prioridade:** P1/P2

### Ações

1. Remover o parser manual duplicado para:
   - `ImageNum`;
   - `Delay`;
   - `BlendValue`;
   - `Size`;
   - `Angle`.
2. Criar função única para separar valores.
3. Não controlar `start/end` manualmente de forma diferente para vírgula e fim da string.
4. Converter somente tokens NUL-terminated.
5. Validar erro de conversão.

### Forma recomendada

Usar um helper conceitualmente equivalente a:

```cpp
bool ParseIntList(
    const char* text,
    std::vector<int>& values);
```

Depois validar a quantidade antes de copiar para `AnimFrame`.

### Testes

- `"1"`;
- `"1,2"`;
- `"1,2,3"`;
- `"10,20,30"`;
- valores com mais de um dígito;
- vírgula final;
- item vazio;
- espaços;
- valor inválido.

---

## BUG-008 — Listas com quantidades diferentes escrevem fora de `AnimFrame`

**Prioridade:** P1

### Ações

1. Nunca escrever diretamente em `AnimFrame[count]` durante parsing sem limite.
2. Parsear cada propriedade para container temporário.
3. Definir uma política única:
   - todas as listas devem ter exatamente N elementos; ou
   - propriedades ausentes têm default explícito.
4. Rejeitar configurações inconsistentes.
5. Validar antes de alocar/copiar.
6. Não permitir que `Delay`, `Size`, `Angle`, etc. redefinam implicitamente a quantidade de frames.

### Critério de aceite

Nenhuma lista pode provocar acesso `AnimFrame[index]` com:

```cpp
index >= AnimFrameCount
```

---

## BUG-009 — `AnimationCount`, `szEffect[30]` e `delete`

**Prioridade:** P1

### Ações imediatas

1. Validar:
   ```cpp
   0 < AnimationCount && AnimationCount <= 30
   ```
   se o limite 30 fizer parte do formato.
2. Trocar liberações de arrays para `delete[]`.
3. Garantir inicialização de todos os slots.
4. Tratar falha parcial de alocação/carregamento sem leak.

### Correção estrutural

Eliminar:

```cpp
char* szEffect[30];
```

e as alocações individuais.

Preferir:

```cpp
std::vector<std::string> effects;
```

ou uma estrutura equivalente compatível com a toolchain.

### Testes

- Count = -1;
- Count = 0;
- Count = 1;
- Count = 30;
- Count = 31;
- falha no item intermediário;
- filename vazio;
- lista menor/maior que Count.

### Critério de aceite

Nenhum valor do INI pode controlar uma escrita fora de stack ou heap.

---

# 7. Fase 5 — Campanha `'/0'` no CLIENT

## BUG-012 — 47 ocorrências de `'/0'`

**Prioridade:** P1  
**Target:** CLIENT

### Importante

**Não executar substituição global sem revisão.**

Cada ocorrência deve ser classificada.

### Categorias

#### A. Intenção clara de terminador NUL

Exemplo:

```cpp
buffer[0] = '/0';
```

Corrigir para:

```cpp
buffer[0] = '\0';
```

#### B. Truncamento de string

Exemplo:

```cpp
filename[i] = '/0';
```

Também tende a ser `'\0'`, mas revisar bounds.

#### C. Comparação

Exemplo hipotético:

```cpp
if (x == '/0')
```

Confirmar intenção antes de corrigir.

#### D. Texto literal

Strings como:

```cpp
"/0"
```

não devem ser alteradas automaticamente.

### Processo

1. Gerar lista completa das 47 ocorrências.
2. Atribuir categoria A/B/C/D.
3. Corrigir por arquivo.
4. Após cada lote, buscar novamente `'/0'`.
5. Comparar com implementação SERVER quando houver equivalente.
6. Procurar também:
   - `'/n'`;
   - `'/t'`;
   - possíveis `'/r'`.

### Testes prioritários

Áreas já identificadas:

- autenticação;
- `sinSTRING`;
- `AssaEffect`;
- CurseFilter.

### Critério de aceite

Zero ocorrências não justificadas de literais multicaractere usados como substitutos de escapes.

---

# 8. Fase 6 — Parsing de efeitos

## BUG-014 — `AssaEffect.cpp`

**Prioridade:** P2/P1 quando input for externo/modificável

### Ações

1. Alterar:
   ```cpp
   char szIndex[2];
   ```
   para buffer com terminador:
   ```cpp
   char szIndex[3] = {};
   ```
2. Evitar `memcpy` sem NUL quando o destino será passado a `atoi`.
3. Corrigir `'/0'` para `'\0'`.
4. Validar `i + 1` de forma explícita antes de consumir dois caracteres.
5. Validar se os dois caracteres são dígitos.
6. Substituir `atoi` por conversão com detecção de erro, se viável.
7. Validar tamanho do filename antes de copiar para `TempFileName`.
8. Revisar todas as cópias subsequentes para `FirstFileName`.

### Testes

- filename esperado;
- índice de 1 dígito;
- índice de 2 dígitos;
- sem índice;
- extensão ausente;
- filename curto;
- filename próximo de 64 bytes;
- caracteres não numéricos.

### Critério de aceite

Nenhuma API de string/conversão recebe buffer não terminado.

---

# 9. Fase 7 — Variável não inicializada / null safety

## BUG-013 — `ClientFuncPos.cpp`

**Prioridade:** P2

### Ações

1. Reestruturar as funções para retornar cedo:

```cpp
if (!lpPlayInfo || !lpPlayInfo->lpsmSock)
    return FALSE;
```

2. Inicializar retorno:
   ```cpp
   int val = FALSE;
   ```
3. Não acessar `lpPlayInfo` depois de caminho em que ele pode ser nulo.
4. Aplicar a mesma correção às duas funções identificadas.
5. Comparar CLIENT-tree/SERVER e manter comportamento equivalente.

### Testes

- `lpPlayInfo == nullptr`;
- `lpsmSock == nullptr`;
- `Send()` retorna 0;
- `Send()` retorna sucesso;
- flags `dwClientFuncChk_*` ligadas/desligadas.

### Critério de aceite

Nenhum caminho lê `val` sem definição e nenhum caminho de erro dereferencia `lpPlayInfo`.

---

# 10. Fase 8 — Saneamento sistêmico de `new[]` / `delete`

## BUG-010 — 75 pares candidatos

**Prioridade:** P1 a P3 por ocorrência

Esse bug não deve ser tratado em um patch único de dezenas de arquivos sem testes.

### Estratégia por lotes

#### Lote A — ocorrências já confirmadas

Começar por:

- `record.cpp`;
- `checkdll.cpp`;
- `smLib3d/smTexture.cpp`;
- `HoBaram/HoLinkHeader.cpp`;
- `HoAnimData.cpp`.

Para cada ponteiro:

1. localizar a alocação;
2. confirmar se é `new[]`;
3. localizar todas as liberações;
4. confirmar ownership;
5. mudar `delete` → `delete[]`;
6. confirmar que não existe alias liberado em outro lugar;
7. verificar double-free;
8. testar ciclo de criação/destruição.

#### Lote B — engine 3D

Revisar juntos:

- `smLib3d/smObj3d.cpp`;
- `smLib3d/smRead3d.cpp`;
- `smLib3d/smStage3d.cpp`;
- `smLib3d/smTexture.cpp`.

Motivo: ownership tende a atravessar as mesmas estruturas.

#### Lote C — gameplay/UI/trade

Revisar:

- `sinbaram/sinTrade.cpp`;
- arquivos relacionados detectados no anexo.

#### Lote D — servidor

Revisar:

- `Server/OnSever.cpp`;
- duplicatas SERVER;
- demais candidatos.

### Não fazer

Não executar regex:

```text
delete X -> delete[] X
```

globalmente.

Um mesmo nome pode receber `new` e `new[]` em fluxos distintos, ou não ser dono da memória.

### Modernização recomendada

Quando o risco de ABI for baixo:

- array temporário → `std::vector<T>`;
- ownership de array → `std::unique_ptr<T[]>`;
- buffer local fixo → `std::array` quando tamanho é realmente fixo;
- recurso com lifetime de escopo → RAII.

### Validação

Para cada lote:

- build;
- smoke test;
- heap verification;
- abrir/fechar repetidamente o subsistema;
- carregar/descarregar assets;
- conectar/desconectar quando aplicável.

### Critério de aceite

Cada ocorrência do anexo deve terminar com um estado:

- **confirmada e corrigida**;
- **falso positivo documentado**;
- **ownership ambíguo — refatoração necessária**.

Nenhuma ocorrência deve ficar simplesmente “ignorada”.

---

# 11. Matriz de dependências

```text
BUG-001
   |
   +----> BUG-002

BUG-003
   |
   +----> BUG-004
   +----> BUG-005
   +----> BUG-006
         (mesmo pacote de CurseFilter)

BUG-012
   +----> BUG-003
   +----> BUG-011
   +----> BUG-014

BUG-007
   |
   +----> BUG-008
   +----> BUG-009
         (mesmo pacote HoAnim)

BUG-010
   +----> inclui correções pontuais do BUG-009
   +----> deve ser executado incrementalmente
```

---

# 12. Organização recomendada dos commits

Evitar misturar tudo.

Sugestão:

```text
fix/net-validate-trans-func-memory
fix/net-restrict-dynamic-code
fix/cursefilter-parser
fix/cursefilter-bounds
fix/sinstring-ownership
fix/hoanim-parser-and-bounds
fix/client-invalid-null-literals-batch-01
fix/assaeffect-filename-parser
fix/clientfuncpos-uninitialized-state
fix/delete-array-batch-01
fix/delete-array-smLib3d
fix/delete-array-server
```

Cada commit deve conter:

- descrição do bug;
- arquivos tocados;
- risco;
- como testar;
- referência ao relatório `BUG-XXX`.

---

# 13. Plano de testes de regressão

## Rede

- login/conexão normal;
- recebimento dos módulos válidos;
- pacote truncado;
- tamanho maior/menor;
- offsets inválidos;
- reconexão;
- CLIENT/SERVER de versões incompatíveis.

## Chat / CurseFilter

- palavra proibida no início/meio/fim;
- texto limpo;
- linhas múltiplas no arquivo;
- tabs;
- arquivo vazio;
- conteúdo muito longo;
- texto iniciando por espaço;
- input > 128;
- caracteres THAI/DBCS usados pelo projeto.

## Strings

- string vazia;
- assignment crescente;
- assignment decrescente;
- cópias;
- self-assignment;
- criação/destruição em loop;
- `nullptr`.

## Animação

- configuração válida;
- Count = 0/1/30/31;
- listas com tamanhos diferentes;
- um token;
- múltiplos tokens;
- último token com múltiplos dígitos;
- tokens inválidos.

## Assets / efeitos

- nomes válidos;
- nomes sem número;
- número de dois dígitos;
- filename máximo;
- extensão inválida.

## Heap

Executar ciclos repetidos de:

- carregar/descarregar mapa;
- carregar/descarregar modelos/texturas;
- abrir/fechar telas;
- conectar/desconectar;
- entrar/sair do jogo;
- trade/inventário se os lotes correspondentes forem alterados.

---

# 14. Gates para merge

Uma PR só deve ser aprovada se:

- [ ] compila no CLIENT;
- [ ] compila no SERVER, quando afetado;
- [ ] não introduz warnings novos relevantes;
- [ ] possui teste manual ou automatizado descrito;
- [ ] trata input inválido de forma segura;
- [ ] revisa a implementação duplicada no outro target;
- [ ] não usa `strcpy`, `sprintf`, `strcat` novo sem capacidade garantida;
- [ ] não introduz novo `new[]` com ownership manual sem necessidade;
- [ ] não introduz `'/0'`, `'/n'`, `'/t'` quando a intenção é escape;
- [ ] possui rollback simples;
- [ ] relatório `BUG-XXX` correspondente foi marcado como resolvido.

---

# 15. Definição de “corrigido”

Para este projeto, “corrigido” deve significar quatro coisas:

1. **A causa raiz foi removida.**  
   Não apenas o crash observado.

2. **O input inválido é rejeitado antes do acesso perigoso.**

3. **Os targets duplicados foram comparados.**  
   CLIENT e SERVER possuem muito código equivalente com divergências.

4. **Há evidência de validação.**  
   Build, teste, log de sanitizer/verifier ou reprodução negativa.

---

# 16. Backlog sugerido

| Ordem | Item | Esforço estimado | Risco da mudança | Prioridade |
|---:|---|---|---|---|
| 1 | BUG-001 validação de pacote | M | Médio | P0 |
| 2 | BUG-002 remover/restringir código dinâmico | L/XL | Alto | P0 |
| 3 | BUG-003 parser CurseFilter | M | Médio | P0 |
| 4 | BUG-004 capacity/format CurseFilter | M | Médio | P0 |
| 5 | BUG-005 underread filterChar | S/M | Baixo/Médio | P0 |
| 6 | BUG-006 ConvertStringTHAI | S/M | Médio | P0/P1 |
| 7 | BUG-011 reimplementar sinSTRING | M/L | Alto | P1 |
| 8 | BUG-007 parser HoAnim | M | Médio | P1 |
| 9 | BUG-008 bounds HoAnim | M | Médio | P1 |
| 10 | BUG-009 AnimationCount/ownership | S/M | Médio | P1 |
| 11 | BUG-012 campanha `'/0'` | M/L | Médio | P1 |
| 12 | BUG-014 AssaEffect | S/M | Baixo/Médio | P2 |
| 13 | BUG-013 ClientFuncPos | S | Baixo | P2 |
| 14 | BUG-010 lote A confirmado | M | Médio | P1 |
| 15 | BUG-010 engine 3D | L | Alto | P1/P2 |
| 16 | BUG-010 restante | L | Médio/Alto | P2/P3 |

> **S:** alteração localizada.  
> **M:** múltiplas funções/testes.  
> **L:** múltiplos arquivos/subsistemas.  
> **XL:** mudança arquitetural/protocolo.

---

# 17. Ordem de deploy recomendada

Se CLIENT e SERVER forem distribuídos separadamente:

### Release 1 — estabilização crítica

- BUG-001
- BUG-003
- BUG-004
- BUG-005
- BUG-006
- correções seguras já confirmadas do BUG-010

### Release 2 — memória e loaders

- BUG-011
- BUG-007
- BUG-008
- BUG-009
- BUG-014
- lote inicial do BUG-012

### Release 3 — hardening

- restante do BUG-012
- BUG-013
- lotes adicionais do BUG-010

### Release arquitetural

- BUG-002, caso a remoção do código dinâmico exija alteração coordenada do protocolo.

Se o BUG-002 puder ser removido sem incompatibilidade, ele deve ser antecipado.

---

# 18. Acompanhamento

Manter uma tabela viva durante a implementação:

| Bug | Status | Commit/PR | CLIENT | SERVER | Testado | Observações |
|---|---|---|---|---|---|---|
| BUG-001 | Aberto | — | — | — | — | — |
| BUG-002 | Aberto | — | — | — | — | — |
| BUG-003 | Aberto | — | — | — | — | — |
| BUG-004 | Aberto | — | — | — | — | — |
| BUG-005 | Aberto | — | — | — | — | — |
| BUG-006 | Aberto | — | — | — | — | — |
| BUG-007 | Aberto | — | — | — | — | — |
| BUG-008 | Aberto | — | — | — | — | — |
| BUG-009 | Aberto | — | — | — | — | — |
| BUG-010 | Aberto | — | — | — | — | 75 candidatos |
| BUG-011 | Aberto | — | — | — | — | — |
| BUG-012 | Aberto | — | CLIENT | N/A | — | 47 ocorrências encontradas |
| BUG-013 | Aberto | — | — | — | — | — |
| BUG-014 | Aberto | — | CLIENT | N/A | — | — |

---

# 19. Resultado esperado

Depois dessas fases, o projeto deverá ter eliminado os principais padrões já confirmados de:

- heap buffer overflow;
- stack buffer overflow;
- out-of-bounds read;
- ponteiro não inicializado;
- strings não terminadas;
- format string;
- parsing sem limite;
- `new[]` / `delete` incompatível;
- execução de payload de rede sem controles adequados;
- divergências perigosas entre CLIENT e SERVER.

Esse plano cobre os **14 relatórios atuais**. Novos achados da continuação da análise devem ser adicionados ao backlog seguindo a mesma priorização: primeiro fronteiras externas e corrupção de memória, depois lógica/robustez e finalmente modernização estrutural.
