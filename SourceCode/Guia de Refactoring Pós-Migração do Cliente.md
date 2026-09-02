# Guia de Refactoring Pós-Migração do Cliente

## 1. Objetivo deste documento

A migração estrutural do cliente pode ser considerada concluída a partir do momento em que o cliente:

- compila integralmente no build atual;
- linka corretamente;
- inicia e executa os fluxos principais esperados;
- não depende acidentalmente de arquivos removidos durante a reorganização;
- mantém os comportamentos legados necessários.

O próximo trabalho não deve ser tratado como continuação da migração.

A partir daqui o objetivo passa a ser:

**reduzir dívida técnica sem reabrir desnecessariamente a migração já estabilizada.**

A regra principal para esta fase deve ser:

> primeiro entender contratos, dependências e comportamento; depois refatorar.

Especialmente neste código legado, ausência de referência encontrada por `grep` não é evidência suficiente para remoção.

---

# 2. Estado atual relevante

O build atual do cliente MinGW é independente do projeto Visual Studio.

O script de build:

- procura fontes em `SrcGame/src`;
- inclui também fontes de `modules/MD5`;
- exclui explicitamente alguns arquivos e diretórios;
- gera objetos separados;
- utiliza dependências `.d`;
- suporta compilação incremental;
- suporta `CLEAN=1`;
- compila em paralelo;
- produz uma lista final dos objetos efetivamente compilados.

Isso torna esse build uma fonte importante para descobrir o **compile set real do cliente**.

O projeto Visual Studio `priston.vcxproj`, por outro lado, deve ser tratado como uma representação possivelmente antiga da estrutura do projeto, e não como autoridade sobre o build atual.

---

# 3. Princípio para o refactoring

Antes de alterar qualquer módulo, classificar cada código encontrado em uma das seguintes categorias:

1. **Código ativo do cliente**
2. **Código compartilhado cliente/servidor**
3. **Código exclusivo do servidor**
4. **Compatibilidade ou shim de link**
5. **Stub temporário ou permanente**
6. **Código legado compilado mas aparentemente não utilizado**
7. **Código legado não compilado**
8. **Código acessado indiretamente**
9. **Código verdadeiramente morto**

Essa classificação deve ocorrer antes de remoções importantes.

---

# 4. Refactoring NECESSÁRIO

## 4.1. Validar formalmente `client_server_stubs.cpp`

### Prioridade 1

Este deve ser o primeiro trabalho depois da validação final do cliente.

O objetivo não é simplesmente reduzir o arquivo.

O objetivo é descobrir **por que cada símbolo existe**.

Para cada função, global ou símbolo definido em:

`client_server_stubs.cpp`

determinar:

- quem referencia o símbolo;
- se a referência ocorre em código do cliente;
- se a implementação real existe no Armazém;
- se existe equivalente em `SrcServer`;
- se o stub substituiu propositalmente código server-only;
- se o stub existe apenas para satisfazer o linker;
- se o símbolo possui algum efeito de runtime;
- se poderia ser substituído por uma interface mais explícita.

Criar uma tabela de auditoria semelhante a:

| Símbolo | Referenciado por | Implementação real | Motivo do stub | Ação |
|---|---|---|---|---|
| `X()` | módulo A | Armazém/... | server-only | manter |
| `Y` | módulo B | inexistente | link shim | investigar |
| `Z()` | ninguém conhecido | SrcServer/... | legado | candidato a remoção |

### Não fazer

Não substituir automaticamente todos os stubs pelas implementações encontradas no Armazém.

Uma implementação antiga pode:

- depender de estado exclusivo do servidor;
- alterar comportamento do cliente;
- introduzir dependências removidas durante a migração;
- depender de ordem de inicialização;
- pertencer a uma versão diferente do protocolo.

---

# 4.2. Definir a fronteira Cliente / Shared / Servidor

### Prioridade 2

Revisar em conjunto:

`Shared`

`compat/client_server`

`SrcServer`

A questão principal não deve ser:

> "onde este arquivo deveria ficar?"

A questão deve ser:

> "qual contrato existe entre estes sistemas?"

Idealmente a arquitetura futura deve convergir conceitualmente para:

```text
              Shared
                |
        ------------------
        |                |
      Client           Server
        |                |
 client adapters     server adapters
```

O diretório:

`compat/client_server`

deve ser tratado como uma **fronteira temporária ou explícita de compatibilidade**, e não como um local genérico onde qualquer dependência difícil é colocada.

Para cada componente encontrado nessa fronteira, classificar como:

### Shared real

Código que:

- possui comportamento idêntico em cliente e servidor;
- não depende de estado específico de nenhum lado;
- representa protocolo, estrutura de dados ou lógica realmente compartilhada.

### Client adapter

Código utilizado para adaptar uma API originalmente compartilhada ou server-oriented ao cliente.

### Server adapter

Equivalente do lado servidor.

### Link shim

Código cuja única função é satisfazer símbolos esperados pelo legado.

### Dívida arquitetural

Código que mistura responsabilidades de cliente e servidor e precisa de separação futura.

---

# 4.3. Formalizar os universos de opcodes

### Prioridade 3

Existem aparentemente dois universos históricos de opcodes.

Eles não devem ser simplesmente fundidos.

Antes de qualquer unificação, determinar:

- origem de cada conjunto;
- quais sistemas consomem cada header;
- valores numéricos;
- colisões;
- aliases;
- protocolos antigos versus atuais;
- direção da mensagem;
- cliente → servidor;
- servidor → cliente;
- login;
- game;
- inter-server, caso exista;
- versões históricas do protocolo.

Criar primeiro documentação semelhante a:

```text
Opcode
 ├── família
 ├── valor
 ├── direção
 ├── produtor
 ├── consumidor
 ├── packet associado
 └── origem histórica
```

Somente depois decidir se os dois sistemas podem compartilhar uma representação.

### Regra

**Nunca juntar os headers apenas porque ambos definem opcodes.**

Dois identificadores semanticamente diferentes podem ter:

- o mesmo nome;
- o mesmo valor;
- valores diferentes;
- lifecycle diferente;
- origem diferente.

O objetivo inicial é documentação e mapeamento.

Unificação é uma etapa posterior.

---

# 4.4. Reduzir dependência de `HoLinkHeader` e `sinLinkHeader`

### Prioridade 4

Esses headers devem ser considerados potenciais **dependency aggregators**.

Problemas comuns desse padrão:

- includes transitivos;
- ordem de include relevante;
- macros vazando entre módulos;
- dependências invisíveis;
- rebuilds excessivos;
- acoplamento entre subsistemas;
- dificuldade de descobrir a dependência real de um `.cpp`.

A estratégia deve ser incremental.

Para arquivos isolados:

1. identificar quais símbolos realmente são utilizados;
2. incluir diretamente os headers responsáveis;
3. remover `HoLinkHeader` ou `sinLinkHeader`;
4. recompilar;
5. testar;
6. avançar para outro arquivo.

Evitar uma remoção global.

### Meta

Transformar gradualmente:

```cpp
#include "HoLinkHeader.h"
```

em algo semelhante a:

```cpp
#include "HoEffect.h"
#include "HoRenderer.h"
#include "HoMath.h"
```

quando essas forem realmente as dependências usadas.

---

# 4.5. Auditar globais e static initialization

### Prioridade 5

Essa auditoria é necessária porque a migração e alterações na composição dos objetos podem modificar **link order**.

Código legado frequentemente depende involuntariamente de inicialização global.

Procurar especialmente:

```cpp
SomeType object;
```

em namespace/global scope.

E:

```cpp
static SomeType object;
```

quando `SomeType` possui construtor não trivial.

Investigar dependências como:

```cpp
GlobalA::GlobalA()
{
    GlobalB.DoSomething();
}
```

quando `GlobalB` está definido em outra translation unit.

Isso pode resultar no clássico:

**Static Initialization Order Fiasco.**

O comportamento pode variar conforme:

- linker;
- ordem dos objetos;
- build system;
- compilador;
- otimização;
- plataforma.

### Preferir

Inicialização sob demanda:

```cpp
SomeType& GetSomething()
{
    static SomeType instance;
    return instance;
}
```

ou inicialização explícita durante startup.

Não refatorar todos os globals de uma só vez.

Priorizar globals:

- com construtores;
- com ponteiros;
- que acessam outros globals;
- relacionados a rendering;
- networking;
- resources;
- game state;
- registries;
- managers.

---

# 4.6. Auditar cópias binárias de objetos complexos

### Prioridade 6

Procurar padrões como:

```cpp
memcpy(&a, &b, sizeof(smCHAR));
```

Isso é particularmente perigoso quando a estrutura deixou de ser um tipo trivial ao longo da evolução do projeto.

Antes de modificar, determinar:

```cpp
std::is_trivially_copyable_v<smCHAR>
```

e investigar se contém:

- ponteiros;
- containers;
- classes;
- objetos com construtor;
- objetos com destrutor;
- virtual functions;
- handles;
- ownership;
- referências internas.

Se `smCHAR` não for trivialmente copiável, `memcpy` pode causar:

- double free;
- aliasing de ponteiros;
- corrupção de ownership;
- estado interno inválido;
- comportamento indefinido.

Classificar cada `memcpy` encontrado em:

### Seguro

POD/trivially-copyable usado como estrutura binária.

### Histórico mas suspeito

Estrutura atualmente trivial, porém semanticamente complexa.

### Incorreto

Objeto não trivially-copyable.

### Serialização

Código que na realidade está tentando serializar um objeto e deveria possuir uma representação explícita para transporte.

---

# 4.7. Auditar dead/inactive code formalmente

### Prioridade 7

Não utilizar apenas:

```bash
grep
```

para decidir que código está morto.

Distinguir pelo menos:

## A. Source não compilado

Existe no repositório, mas não participa de nenhum build relevante.

Pode ser:

- código histórico;
- backup;
- plataforma antiga;
- experimento;
- código abandonado.

## B. Source compilado sem consumidores óbvios

Pode conter:

- inicialização estática;
- registration;
- side effects;
- exports;
- callbacks registrados indiretamente.

## C. Link shim

O símbolo existe apenas para satisfazer dependências de link.

Pode não ser chamado em runtime e ainda assim ser necessário para construir.

## D. Runtime reflection / registration / export

Pode não aparecer em referências estáticas normais.

Exemplos:

- `GetProcAddress`;
- nomes de função como strings;
- factories;
- registries;
- callbacks;
- scripts;
- exports;
- loaders.

## E. Verdadeiramente morto

Somente depois de excluir as categorias anteriores considerar remoção.

### Processo recomendado

Para cada candidato:

1. confirmar se é compilado;
2. identificar símbolo gerado;
3. procurar referências no link;
4. procurar registries;
5. procurar acesso por nome/string;
6. procurar exports;
7. verificar inicialização estática;
8. verificar build Visual Studio antigo;
9. verificar versões históricas quando necessário;
10. remover isoladamente;
11. clean build;
12. teste funcional.

---

# 5. Refactoring OPCIONAL

Os seguintes trabalhos são desejáveis, mas não deveriam bloquear a estabilização da arquitetura atual.

## 5.1. Reorganizar fisicamente diretórios

Somente após os contratos serem compreendidos.

Não mover arquivos apenas por estética.

Uma organização futura poderia convergir para algo semelhante a:

```text
SrcGame/
    client/

SrcServer/
    server/

Shared/
    protocol/
    model/
    math/
    utility/

compat/
    client_server/
```

Mas essa estrutura só faz sentido depois da auditoria da fronteira.

---

# 5.2. Criar namespaces mais claros

Código legado frequentemente depende fortemente do namespace global.

Introduzir namespaces pode melhorar isolamento, mas possui impacto amplo.

Deve ocorrer módulo a módulo.

Exemplo futuro:

```cpp
namespace client {}
namespace server {}
namespace protocol {}
namespace legacy {}
```

Não considero prioridade imediata.

---

# 5.3. Reduzir macros globais

Macros provenientes de headers agregadores podem ser convertidas gradualmente em:

```cpp
constexpr
enum class
inline functions
templates
```

quando adequado.

Esse trabalho é útil, mas deve vir depois da estabilização das dependências.

---

# 5.4. Modernizar ownership

Onde existirem padrões antigos:

```cpp
new
delete
malloc
free
```

avaliar modernização gradual para RAII.

Não fazer conversões automáticas em estruturas ligadas a:

- protocolo;
- buffers binários;
- APIs C;
- DirectX legado;
- estruturas cuja ABI seja relevante.

---

# 5.5. Melhorar nomes de compatibilidade

Depois da auditoria, nomes genéricos como:

`client_server_stubs.cpp`

podem ser substituídos por arquivos que expressem intenção.

Exemplos conceituais:

```text
server_api_client_shims.cpp
legacy_server_symbols.cpp
client_protocol_compat.cpp
```

Isso só deve acontecer depois de descobrir a função real dos stubs.

---

# 6. `priston.vcxproj`

## Situação

O projeto Visual Studio contém paths antigos.

Durante esta migração ele não era o build ativo.

O build MinGW atual determina suas próprias fontes e inclui arquivos diretamente a partir da árvore atual do projeto.

Portanto:

**não considero necessário corrigir imediatamente todos os paths do `priston.vcxproj` para declarar a migração concluída.**

Porém considero que ele deve ser revisado.

---

# 7. Faz sentido revisar o `priston.vcxproj`?

Sim.

Principalmente porque já foi observado anteriormente que o projeto Visual Studio incluía arquivos que depois foram retirados da árvore ativa e substituídos por stubs ou compatibilidade.

Essa informação torna o `.vcxproj` útil como **evidência histórica da composição antiga do cliente**.

A revisão não deve começar tentando fazer o Visual Studio compilar.

Deve começar fazendo uma comparação.

Criar três conjuntos:

```text
A = arquivos presentes no priston.vcxproj
B = arquivos compilados atualmente pelo MinGW
C = arquivos presentes no repositório
```

Depois calcular conceitualmente:

```text
A - B
B - A
A ∩ B
```

## `A - B`

Arquivos que o Visual Studio antigo esperava, mas o build atual não compila.

Esse é provavelmente o conjunto mais interessante.

Para cada arquivo:

- ele foi removido propositalmente?
- foi substituído por stub?
- migrou para outro módulo?
- era código server-only?
- virou `compat/client_server`?
- era dead code?
- continua existindo no Armazém?

## `B - A`

Arquivos usados atualmente pelo MinGW que não estavam no projeto Visual Studio antigo.

Podem indicar:

- código introduzido pela migração;
- novos adapters;
- stubs;
- compatibilidade;
- módulos que anteriormente entravam de outra maneira no build.

## `A ∩ B`

Compile set compartilhado.

Ajuda a identificar o núcleo mais estável do cliente.

---

# 8. O que NÃO fazer com `priston.vcxproj`

Não assumir:

> "está no `.vcxproj`, portanto precisa voltar para o cliente."

Nem:

> "não está no build MinGW, portanto está morto."

O `.vcxproj` deve ser utilizado como:

**mapa arqueológico do build anterior.**

Depois da comparação, podemos decidir se vale a pena:

1. restaurá-lo como build suportado;
2. atualizá-lo apenas como documentação;
3. regenerá-lo;
4. arquivá-lo;
5. removê-lo futuramente.

---

# 9. Ordem recomendada de execução

## Fase 1 — Stubs

Auditar:

`client_server_stubs.cpp`

contra:

- Armazém;
- `SrcServer`;
- referências atuais.

Entrega:

`client_server_stubs_audit.md`

---

## Fase 2 — Fronteiras

Auditar:

```text
Shared
compat/client_server
SrcServer
```

Entrega:

`client_server_boundary.md`

Com classificação por arquivo:

```text
SHARED
CLIENT
SERVER
ADAPTER
SHIM
LEGACY
UNKNOWN
```

---

## Fase 3 — Protocolos

Mapear os dois universos de opcodes.

Entrega:

`protocol_opcode_map.md`

Sem unificar headers nesta fase.

---

## Fase 4 — Dependências históricas

Auditar:

```text
HoLinkHeader
sinLinkHeader
```

Objetivo:

descobrir quais dependências transitivas podem ser removidas localmente.

---

## Fase 5 — Runtime safety

Auditar:

- globals;
- static initialization;
- dependências entre translation units.

---

## Fase 6 — Memory safety

Auditar:

```text
memcpy
memmove
memset
```

em objetos complexos.

Prioridade especial para:

`smCHAR`

---

## Fase 7 — Dead code

Executar auditoria formal de código inativo.

Somente nessa fase começar remoções significativas.

---

# 10. Regra de validação para cada refactoring

Cada alteração deve preferencialmente seguir:

```text
1 mudança conceitual
        ↓
clean build
        ↓
link
        ↓
startup
        ↓
teste funcional relacionado
        ↓
commit
```

Para o build atual, usar:

```bash
CLEAN=1 ./build_client_mingw_i686_multicore.sh
```

ou o nome equivalente do script atualmente existente.

Evitar acumular várias remoções arquiteturais antes do próximo clean build.

---

# 11. Guardrails para os próximos trabalhos

Não remover código apenas porque:

- não apareceu no grep;
- não existe call direta;
- parece server-only;
- está comentado em algum projeto antigo;
- existe equivalente em outro diretório.

Não mover código para `Shared` apenas porque cliente e servidor possuem versões semelhantes.

Não substituir stub por implementação real sem comparar os contratos.

Não unificar opcodes apenas pelos nomes.

Não alterar layouts de structs envolvidos em networking sem verificar ABI e serialização.

Não modernizar `memcpy` mecanicamente sem compreender a semântica original.

Não fazer refactoring global de headers agregadores.

---

# 12. Critério para considerar cada dívida encerrada

Uma dívida não está encerrada quando o código "ficou mais bonito".

Está encerrada quando existe uma resposta objetiva para:

```text
O que era?
Por que existia?
Quem dependia disso?
Qual contrato ele representa?
O que foi alterado?
Por que a alteração é segura?
Como foi validada?
```

---

# 13. Próxima dívida técnica recomendada

A ordem recomendada é:

### Prioridade 1
Comparar `client_server_stubs.cpp` com o Armazém.

### Prioridade 2
Revisar a fronteira:

```text
Shared
compat/client_server
SrcServer
```

e definir contratos reais.

### Prioridade 3
Unificar ou documentar corretamente os dois universos de opcodes.

**Nunca simplesmente juntar os headers sem análise.**

### Prioridade 4
Reduzir dependência de:

```text
HoLinkHeader
sinLinkHeader
```

quando isso puder ser feito com pequenas refatorações isoladas.

### Prioridade 5
Auditar globals e static initialization.

Especial atenção a mudanças de link order e comportamento indefinido.

### Prioridade 6
Revisar cópias binárias de objetos complexos.

Exemplo crítico:

```cpp
memcpy(... sizeof(smCHAR))
```

### Prioridade 7
Investigar dead/inactive code formalmente.

Distinguir:

```text
source não compilado
source compilado sem consumidores
link shim
runtime reflection/export
verdadeiramente morto
```

---

# 14. Ponto recomendado para iniciar o próximo chat

O próximo trabalho deve começar exclusivamente pela Prioridade 1.

## Objetivo

Auditar completamente:

`client_server_stubs.cpp`

e comparar cada símbolo com o código original disponível no Armazém.

## Perguntas que o próximo trabalho deve responder

Para cada stub:

1. Qual símbolo ele satisfaz?
2. Quem referencia esse símbolo?
3. A referência pertence ao cliente ou a código compartilhado?
4. Existe implementação original no Armazém?
5. Existe implementação atual em `SrcServer`?
6. A implementação original possui efeitos colaterais?
7. O cliente deveria possuir esse comportamento?
8. O stub pode permanecer?
9. O stub deveria virar adapter?
10. O símbolo deveria desaparecer através de refactoring?
11. Existe dependência transitiva causada por `HoLinkHeader` ou `sinLinkHeader`?
12. Há implicações de protocolo ou ABI?

## Resultado esperado

Produzir uma tabela:

```text
Símbolo
Origem
Referências
Implementação original
Implementação servidor
Comportamento esperado no cliente
Classificação
Risco
Ação recomendada
```

Classificações permitidas:

```text
KEEP_STUB
REPLACE_WITH_SHARED
CREATE_CLIENT_ADAPTER
REMOVE_DEPENDENCY
SERVER_ONLY
LEGACY_COMPAT
UNKNOWN
```

Não modificar código enquanto a classificação inicial não estiver concluída.

---

# 15. Prompt pronto para iniciar outro chat

Estou continuando uma migração/refactoring de um cliente C++ legado.

A migração estrutural já foi concluída e o build ativo do cliente é atualmente Linux/MinGW, através do script `build_client_mingw_i686_multicore.sh`.

Não quero reabrir a migração. Quero iniciar a fase de redução de dívida técnica.

A primeira tarefa é auditar `client_server_stubs.cpp`.

Quero comparar todos os símbolos existentes nesse arquivo com:

- o código atual;
- `Shared`;
- `compat/client_server`;
- `SrcServer`;
- e versões originais existentes no diretório que chamaremos de **Armazém**.

Para cada símbolo, quero descobrir:

- quem referencia;
- por que o linker precisa dele;
- implementação original;
- implementação no servidor, se existir;
- se é comportamento realmente compartilhado;
- se é server-only;
- se é link shim;
- se deve continuar como stub;
- se deveria virar adapter;
- se podemos eliminar a dependência.

Classifique cada símbolo em uma destas categorias:

`KEEP_STUB`

`REPLACE_WITH_SHARED`

`CREATE_CLIENT_ADAPTER`

`REMOVE_DEPENDENCY`

`SERVER_ONLY`

`LEGACY_COMPAT`

`UNKNOWN`

Não faça substituições automáticas.

Não assuma que a implementação existente no Armazém deve voltar para o cliente.

Não remova código simplesmente porque não há uma chamada encontrada por grep.

Considere possíveis:

- static initialization;
- registrations;
- exports;
- callbacks;
- acesso indireto;
- link shims;
- ABI;
- protocolo.

Ao final quero uma tabela de auditoria e uma recomendação por símbolo.

Depois dessa etapa iremos, separadamente, revisar a fronteira entre `Shared`, `compat/client_server` e `SrcServer`.