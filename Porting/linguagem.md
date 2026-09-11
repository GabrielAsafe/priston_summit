# Introdução a C++ para quem já programa

Este guia assume que você já entende conceitos como variáveis, funções, condicionais, loops, estruturas de dados e orientação a objetos, mas não assume experiência prévia com C, gerenciamento manual de memória ou programação de sistemas.

Os exemplos usam **C++ moderno**, evitando práticas antigas quando existe uma alternativa melhor.

---

## 1. O modelo mental de C++

C++ pode parecer familiar se você vem de Java, C#, JavaScript, Python, Go, Rust ou outras linguagens, mas existem algumas diferenças fundamentais.

Em C++, você precisa prestar bastante atenção a:

* **tipo e tempo de vida dos objetos**
* **valor vs. referência**
* **stack vs. heap**
* **cópia vs. movimento**
* **ownership de memória**
* **compilação**
* **headers e múltiplos arquivos**

Ao mesmo tempo, C++ moderno automatiza boa parte disso.

Uma boa regra inicial é:

> Prefira objetos normais, containers da biblioteca padrão e RAII. Use `new` e `delete` diretamente apenas quando souber exatamente por que precisa deles.

---

# 2. Primeiro programa

```cpp
#include <iostream>

int main() {
    std::cout << "Hello, world!\n";
    return 0;
}
```

Compilando com GCC:

```bash
g++ main.cpp -std=c++20 -Wall -Wextra -o programa
```

Executando:

```bash
./programa
```

Com Clang:

```bash
clang++ main.cpp -std=c++20 -Wall -Wextra -o programa
```

## O que está acontecendo?

```cpp
#include <iostream>
```

Inclui declarações da biblioteca de input/output.

```cpp
int main()
```

É o entry point do programa.

```cpp
std::cout
```

`cout` está dentro do namespace `std`.

O operador:

```cpp
<<
```

envia valores para o stream.

Por exemplo:

```cpp
std::cout << "idade = " << idade << '\n';
```

---

# 3. Tipos básicos

C++ é estaticamente tipado.

```cpp
int idade = 30;
double temperatura = 21.5;
bool ativo = true;
char letra = 'A';
```

Strings normalmente usam:

```cpp
#include <string>

std::string nome = "Maria";
```

Existe também inferência de tipos com `auto`:

```cpp
auto idade = 30;          // int
auto preco = 19.99;       // double
auto nome = std::string{"Ana"};
```

`auto` não significa tipagem dinâmica.

O compilador simplesmente deduz o tipo.

```cpp
auto x = 10;
```

continua sendo equivalente a:

```cpp
int x = 10;
```

Na prática, `auto` é bastante usado em C++ moderno.

---

# 4. Inicialização

Você verá várias formas de inicializar valores:

```cpp
int a = 10;
int b(10);
int c{10};
```

A forma com `{}` é bastante comum em C++ moderno:

```cpp
int idade{25};
std::string nome{"João"};
```

Ela também evita algumas conversões perigosas:

```cpp
int x{3.14}; // erro
```

enquanto:

```cpp
int x = 3.14;
```

pode simplesmente produzir `3`.

---

# 5. `const`

`const` aparece em todo lugar em C++.

```cpp
const int maximo = 100;
```

O valor não pode ser alterado:

```cpp
maximo = 200; // erro
```

Também é comum em funções:

```cpp
void imprimir(const std::string& texto) {
    std::cout << texto << '\n';
}
```

Aqui temos duas coisas importantes:

```cpp
const std::string&
```

* `&`: recebe uma referência ao objeto original, evitando cópia.
* `const`: a função promete não modificar o objeto.

Esse padrão é extremamente comum.

---

# 6. Valor vs. referência

Essa é uma das diferenças mais importantes de C++.

Considere:

```cpp
void mudar(int x) {
    x = 20;
}

int main() {
    int n = 10;
    mudar(n);

    std::cout << n;
}
```

Resultado:

```text
10
```

`x` é uma cópia de `n`.

## Passando por referência

```cpp
void mudar(int& x) {
    x = 20;
}
```

Agora:

```cpp
int n = 10;
mudar(n);

std::cout << n;
```

produz:

```text
20
```

O parâmetro:

```cpp
int&
```

significa "referência para um `int`".

---

## Referência somente-leitura

Para objetos maiores, você frequentemente verá:

```cpp
void imprimir(const std::string& nome) {
    std::cout << nome;
}
```

Em vez de:

```cpp
void imprimir(std::string nome)
```

O segundo caso cria uma cópia.

O primeiro normalmente não.

---

# 7. Ponteiros
Claro. Vou focar no que você precisa para entender código como `sqlite3* DB`, `&DB`, `*DB` e `sqlite3**`, em vez de entrar em todos os detalhes de ponteiros de uma vez.

## 1. Variável normal

Começando sem ponteiros:

```cpp
int numero = 10;
```

Mentalmente:

```text
memória

endereço      conteúdo
0x1000        10
              ↑
            numero
```

A variável `numero` contém `10`.

Você pode perguntar ao C++ qual é o endereço dela usando `&`:

```cpp
std::cout << &numero;
```

Algo como:

```text
0x1000
```

Então:

```cpp
numero   // valor
&numero  // endereço de numero
```

---

## 2. Um ponteiro guarda um endereço

Agora:

```cpp
int numero = 10;

int* ptr = &numero;
```

`ptr` é uma variável que guarda o endereço de um `int`.

```text
numero
┌──────────┐
│    10    │
└──────────┘
  0x1000
     ↑
     │
     │
ptr ─┘
```

Se:

```cpp
&numero == 0x1000
```

então conceitualmente:

```cpp
ptr == 0x1000
```

Portanto:

```cpp
int* ptr;
```

significa:

> `ptr` é um ponteiro para `int`.

---

## 3. O `&`

Nesse contexto:

```cpp
int numero = 10;

int* ptr = &numero;
```

`&numero` significa:

> me dê o endereço de memória de `numero`.

Então:

```cpp
numero
```

é:

```text
10
```

enquanto:

```cpp
&numero
```

é algo como:

```text
0x1000
```

---

## 4. O `*`

Aqui começa a parte que confunde bastante porque `*` aparece com dois significados relacionados.

Na declaração:

```cpp
int* ptr;
```

significa:

> `ptr` é um ponteiro para `int`.

Mas depois:

```cpp
*ptr
```

significa:

> vá até o endereço guardado em `ptr` e pegue o valor que existe lá.

Exemplo:

```cpp
int numero = 10;

int* ptr = &numero;

std::cout << ptr << '\n';
std::cout << *ptr << '\n';
```

Conceitualmente:

```text
ptr  → 0x1000
*ptr → 10
```

Porque:

```text
ptr
 ↓
0x1000
 ↓
┌────┐
│ 10 │
└────┘
```

`*ptr` é chamado de **desreferenciar o ponteiro**.

---

## 5. Você pode modificar através do ponteiro

Isso aqui é importante:

```cpp
int numero = 10;

int* ptr = &numero;

*ptr = 50;
```

Agora:

```cpp
std::cout << numero;
```

imprime:

```text
50
```

Por quê?

Porque você não mudou o endereço armazenado em `ptr`.

Você falou:

```cpp
*ptr = 50;
```

ou seja:

> vá para onde `ptr` aponta e coloque `50` lá.

E `ptr` aponta para `numero`.

```text
ANTES

ptr ──────┐
          ↓
       numero
       ┌────┐
       │ 10 │
       └────┘


*ptr = 50;


DEPOIS

ptr ──────┐
          ↓
       numero
       ┌────┐
       │ 50 │
       └────┘
```

---

# 6. Ponteiros e funções

Aqui está uma das principais razões para você estar vendo ponteiros na API do SQLite.

Considere:

```cpp
void mudar(int x)
{
    x = 50;
}
```

E:

```cpp
int numero = 10;

mudar(numero);

std::cout << numero;
```

Continua:

```text
10
```

Porque `x` recebeu uma cópia.

Conceitualmente:

```text
numero = 10

       cópia
         ↓
         x = 10

x = 50

numero continua 10
```

Mas podemos passar o endereço:

```cpp
void mudar(int* x)
{
    *x = 50;
}
```

E chamar:

```cpp
int numero = 10;

mudar(&numero);
```

Agora acontece:

```text
numero
┌────┐
│ 10 │
└────┘
  ↑
  │
  x
```

Dentro da função:

```cpp
*x = 50;
```

muda o objeto original.

Depois:

```cpp
numero == 50
```

---

# 7. Agora vamos chegar no SQLite

Você viu:

```cpp
sqlite3* DB = nullptr;
```

Não precisa saber como `sqlite3` funciona internamente.

Pense:

```cpp
sqlite3
```

é um tipo definido pela biblioteca.

Então:

```cpp
sqlite3* DB;
```

significa:

> `DB` é um ponteiro para um objeto `sqlite3`.

Ou, de maneira mais útil:

> `DB` vai apontar para minha conexão com o banco.

Inicialmente:

```cpp
sqlite3* DB = nullptr;
```

temos:

```text
DB
┌─────────┐
│ nullptr │
└─────────┘
```

`nullptr` significa:

> esse ponteiro atualmente não aponta para nenhum objeto.

---

# 8. O SQLite precisa modificar `DB`

Você chama:

```cpp
sqlite3_open("example.db", &DB);
```

Olha com atenção:

```cpp
DB
```

é:

```cpp
sqlite3*
```

Portanto:

```cpp
&DB
```

é o endereço de uma variável que é `sqlite3*`.

Logo o tipo de:

```cpp
&DB
```

é:

```cpp
sqlite3**
```

Esse `**` assusta no começo, mas significa literalmente:

> ponteiro para ponteiro de `sqlite3`.

---

# 9. Por que `sqlite3_open` quer `sqlite3**`?

Porque ela precisa modificar seu ponteiro.

Antes:

```text
DB
┌─────────┐
│ nullptr │
└─────────┘
```

Você passa:

```cpp
&DB
```

para o SQLite:

```cpp
sqlite3_open("example.db", &DB);
```

O SQLite cria/abre a conexão e altera `DB`.

Depois:

```text
DB
┌──────────┐
│ 0xABCD   │──────────┐
└──────────┘          │
                      ↓
                 ┌─────────┐
                 │ sqlite3 │
                 │ conexão │
                 └─────────┘
```

Essa é a razão do `**`.

---

# 10. Relacionando com sua `openDB`

Você tinha algo parecido com:

```cpp
int openDB(sqlite3** DB)
{
    int rc = sqlite3_open("example.db", DB);

    // ...
}
```

E chama:

```cpp
sqlite3* DB = nullptr;

openDB(&DB);
```

Vamos seguir os tipos.

No `main`:

```cpp
sqlite3* DB;
```

Portanto:

```cpp
DB
```

é:

```text
sqlite3*
```

e:

```cpp
&DB
```

é:

```text
sqlite3**
```

Sua função espera:

```cpp
int openDB(sqlite3** DB)
```

Perfeito.

Dentro dela:

```cpp
sqlite3_open("example.db", DB);
```

O `DB` da função **já é `sqlite3**`**, então não precisa colocar outro `&`.

---

# 11. E por que `createTable` recebe apenas `sqlite3*`?

Compare:

```cpp
int openDB(sqlite3** DB)
```

com:

```cpp
int createTable(sqlite3* DB)
```

Existe uma diferença conceitual importante.

`openDB` precisa fazer:

> "Quero modificar o ponteiro do chamador para fazê-lo apontar para uma conexão."

Por isso:

```cpp
sqlite3**
```

Já `createTable` só precisa:

> "Me dê a conexão existente para eu usá-la."

Então:

```cpp
sqlite3*
```

é suficiente.

No `main`:

```cpp
sqlite3* DB = nullptr;

openDB(&DB);

createTable(DB);
```

Repare:

```text
                 tipo

DB               sqlite3*

&DB              sqlite3**
```

Por isso:

```cpp
openDB(&DB);
```

mas:

```cpp
createTable(DB);
```

---

# 12. O erro que você tinha antes

Você tinha:

```cpp
createTable(*DB);
```

Mas se:

```cpp
DB
```

é um:

```cpp
sqlite3*
```

então:

```cpp
*DB
```

é o objeto `sqlite3` para o qual ele aponta.

Só que `createTable` quer:

```cpp
sqlite3*
```

Portanto você passa:

```cpp
createTable(DB);
```

e não:

```cpp
createTable(*DB);
```

---

# 13. Uma regra mental útil

Quando vir:

```cpp
Tipo* x;
```

leia:

> `x` aponta para um `Tipo`.

Quando vir:

```cpp
&x
```

leia:

> endereço de `x`.

Quando vir:

```cpp
*x
```

leia:

> objeto/valor para o qual `x` aponta.

Então:

```cpp
int numero = 42;
int* ptr = &numero;
```

temos:

```text
numero      42

&numero     endereço de numero

ptr         endereço de numero

*ptr        42
```

---

## E `**`?

Só aplique a mesma lógica duas vezes.

```cpp
int numero = 42;

int* ptr = &numero;

int** ptr2 = &ptr;
```

Visualmente:

```text
ptr2
 │
 ↓
ptr
 │
 ↓
numero
 │
 ↓
42
```

Então:

```cpp
ptr2
```

é ponteiro para ponteiro.

```cpp
*ptr2
```

é `ptr`.

E:

```cpp
**ptr2
```

é `numero`, portanto:

```text
42
```

Isso é exatamente o conceito que aparece em:

```cpp
sqlite3** DB
```

Não existe nenhuma "mágica especial do SQLite" aí. É ponteiro para ponteiro porque uma função precisa conseguir alterar um `sqlite3*` que pertence ao chamador.

Se você fixar bem a relação:

```text
x      valor
&x     endereço de x

ptr    endereço
*ptr   valor apontado
```

o `sqlite3*`/`sqlite3**` começa a ficar bem menos misterioso.

---

# 8. Referência ou ponteiro?

Regra prática:

Use referência quando:

```cpp
T&
```

o objeto **precisa existir**.

Use ponteiro quando:

```cpp
T*
```

"não existe objeto" é um estado válido, normalmente representado por:

```cpp
nullptr
```

Por exemplo:

```cpp
void processar(Usuario& usuario);
```

significa essencialmente:

> preciso receber um usuário.

Enquanto:

```cpp
void processar(Usuario* usuario);
```

pode significar:

> talvez exista um usuário.

---

# 9. Arrays: prefira `std::array` e `std::vector`

C++ possui arrays estilo C:

```cpp
int numeros[3] = {10, 20, 30};
```

Mas normalmente você preferirá containers da Standard Library.

## Tamanho fixo

```cpp
#include <array>

std::array<int, 3> numeros{10, 20, 30};
```

## Tamanho variável

```cpp
#include <vector>

std::vector<int> numeros{10, 20, 30};

numeros.push_back(40);
```

Iterando:

```cpp
for (int n : numeros) {
    std::cout << n << '\n';
}
```

Esse é um **range-based for**.

---

# 10. `std::vector`

`std::vector` provavelmente será uma das estruturas mais utilizadas no seu código.

```cpp
std::vector<int> numeros;

numeros.push_back(10);
numeros.push_back(20);
numeros.push_back(30);
```

Acesso:

```cpp
numeros[0];
```

Também existe:

```cpp
numeros.at(0);
```

A diferença é que `.at()` verifica os limites e lança uma exceção em caso de índice inválido.

Tamanho:

```cpp
numeros.size();
```

Verificar vazio:

```cpp
numeros.empty();
```

---

# 11. Outras estruturas da STL

A biblioteca padrão de C++ possui vários containers.

## `std::vector`

Array dinâmico.

```cpp
std::vector<int>
```

## `std::array`

Array de tamanho fixo.

```cpp
std::array<int, 10>
```

## `std::map`

Mapa ordenado.

```cpp
std::map<std::string, int>
```

## `std::unordered_map`

Hash map.

```cpp
std::unordered_map<std::string, int>
```

## `std::set`

Conjunto de elementos únicos.

```cpp
std::set<int>
```

## `std::unordered_set`

Hash set.

```cpp
std::unordered_set<int>
```

## `std::optional`

Valor que pode ou não existir.

```cpp
std::optional<Usuario>
```

---

# 12. Funções

Função comum:

```cpp
int soma(int a, int b) {
    return a + b;
}
```

Tipo de retorno vem primeiro:

```cpp
int soma(...)
```

Função que não retorna nada:

```cpp
void imprimir() {
    std::cout << "Olá";
}
```

Também existe dedução:

```cpp
auto soma(int a, int b) {
    return a + b;
}
```

---

# 13. Overloading

C++ permite funções com o mesmo nome e parâmetros diferentes.

```cpp
int soma(int a, int b) {
    return a + b;
}

double soma(double a, double b) {
    return a + b;
}
```

Então:

```cpp
soma(1, 2);
```

chama a primeira versão.

Enquanto:

```cpp
soma(1.5, 2.5);
```

chama a segunda.

---

# 14. Estruturas

```cpp
struct Pessoa {
    std::string nome;
    int idade;
};
```

Uso:

```cpp
Pessoa pessoa{
    "Maria",
    30
};

std::cout << pessoa.nome;
```

Você também pode definir métodos:

```cpp
struct Pessoa {
    std::string nome;
    int idade;

    void apresentar() const {
        std::cout << "Sou " << nome << '\n';
    }
};
```

---

# 15. `struct` vs `class`

Em C++, ambos podem ter:

* atributos
* métodos
* construtores
* herança
* métodos virtuais
* operadores

A diferença principal é o acesso padrão.

Em:

```cpp
struct Pessoa {
    int idade;
};
```

`idade` é `public`.

Em:

```cpp
class Pessoa {
    int idade;
};
```

`idade` é `private`.

Uma convenção comum é usar:

```cpp
struct
```

para tipos simples/dados.

E:

```cpp
class
```

para tipos com invariantes ou encapsulamento.

Mas isso é uma convenção, não uma regra da linguagem.

---

# 16. Classes

```cpp
class Conta {
private:
    double saldo_;

public:
    Conta(double saldo)
        : saldo_{saldo} {
    }

    void depositar(double valor) {
        saldo_ += valor;
    }

    double saldo() const {
        return saldo_;
    }
};
```

Uso:

```cpp
Conta conta{100};

conta.depositar(50);

std::cout << conta.saldo();
```

---

# 17. Construtores

```cpp
class Pessoa {
public:
    Pessoa(std::string nome, int idade)
        : nome_{std::move(nome)},
          idade_{idade} {
    }

private:
    std::string nome_;
    int idade_;
};
```

Esta parte:

```cpp
: nome_{...}, idade_{...}
```

é a **member initializer list**.

Ela inicializa os campos diretamente.

Em C++ isso geralmente é preferível a:

```cpp
Pessoa(std::string nome, int idade) {
    nome_ = nome;
    idade_ = idade;
}
```

---

# 18. Destrutores e RAII

C++ tem um conceito extremamente importante chamado:

**RAII — Resource Acquisition Is Initialization**

A ideia é associar um recurso ao tempo de vida de um objeto.

Por exemplo:

```cpp
{
    std::vector<int> numeros{1, 2, 3};
}
```

Quando o bloco termina, `numeros` é destruído automaticamente.

Sua memória também é liberada.

Você não precisa fazer:

```cpp
free(...)
```

ou:

```cpp
delete ...
```

Isso vale para vários recursos:

* memória
* arquivos
* sockets
* locks
* handles
* conexões

Um objeto gerencia o recurso e o libera em seu destrutor.

Esse é um dos conceitos centrais de C++ moderno.

---

# 19. `new` e `delete`

É possível fazer:

```cpp
Pessoa* pessoa = new Pessoa{"Ana", 25};

delete pessoa;
```

Mas você raramente deve escrever código assim em C++ moderno.

Existem alternativas melhores.

---

# 20. Smart pointers

Quando realmente precisamos de objetos dinamicamente alocados, normalmente usamos smart pointers.

## `std::unique_ptr`

Ownership exclusivo:

```cpp
#include <memory>

auto pessoa = std::make_unique<Pessoa>("Ana", 25);
```

Quando `pessoa` deixa de existir, o objeto é automaticamente destruído.

Você não chama:

```cpp
delete
```

## `std::shared_ptr`

Ownership compartilhado:

```cpp
auto pessoa = std::make_shared<Pessoa>("Ana", 25);
```

O objeto permanece vivo enquanto pelo menos um `shared_ptr` estiver apontando para ele.

Regra prática:

> Prefira `unique_ptr`. Use `shared_ptr` somente quando ownership compartilhado realmente fizer parte do modelo.

---

# 21. Cópia

Considere:

```cpp
std::string a = "hello";

std::string b = a;
```

Agora existem duas strings independentes.

```cpp
b += "!";

std::cout << a; // hello
std::cout << b; // hello!
```

C++ possui **value semantics** em muitos lugares.

Isso é diferente de linguagens em que objetos são referências por padrão.

---

# 22. Move semantics

C++ também pode mover recursos em vez de copiá-los.

```cpp
std::string a = "uma string enorme";

std::string b = std::move(a);
```

Conceitualmente:

```text
a ---- recursos
```

vira algo próximo de:

```text
b ---- recursos
```

em vez de copiar todos os dados.

Depois de:

```cpp
std::move(a)
```

`a` continua sendo um objeto válido, mas seu conteúdo geralmente não deve ser assumido.

Move semantics é uma das coisas que vale estudar depois que os fundamentos estiverem confortáveis.

---

# 23. Stack e heap

Observe:

```cpp
Pessoa pessoa{"Ana", 30};
```

Esse objeto possui duração automática.

Você não precisa destruí-lo manualmente.

Agora:

```cpp
auto pessoa = std::make_unique<Pessoa>("Ana", 30);
```

o objeto `Pessoa` está dinamicamente alocado, mas o `unique_ptr` gerencia sua vida.

Uma ideia importante:

> Em C++, "objeto" não significa automaticamente "objeto no heap".

É perfeitamente normal ter objetos completos diretamente como variáveis locais:

```cpp
std::string nome;
std::vector<int> numeros;
Pessoa pessoa;
Conta conta;
```

---

# 24. `std::optional`

Em vez de retornar `nullptr`, valores especiais como `-1` ou lançar uma exceção, muitas funções podem representar ausência com `std::optional`.

```cpp
#include <optional>

std::optional<int> buscar_id(const std::string& nome) {
    if (nome == "Ana") {
        return 42;
    }

    return std::nullopt;
}
```

Uso:

```cpp
auto id = buscar_id("Ana");

if (id) {
    std::cout << *id;
}
```

Ou:

```cpp
if (auto id = buscar_id("Ana")) {
    std::cout << *id;
}
```

---

# 25. Structured bindings

Suponha:

```cpp
std::pair<std::string, int> pessoa{"Ana", 25};
```

Você poderia escrever:

```cpp
auto [nome, idade] = pessoa;
```

Isso também é útil com mapas:

```cpp
std::map<std::string, int> idades{
    {"Ana", 25},
    {"Pedro", 31}
};

for (const auto& [nome, idade] : idades) {
    std::cout << nome << ": " << idade << '\n';
}
```

---

# 26. Lambdas

C++ possui funções anônimas:

```cpp
auto dobro = [](int x) {
    return x * 2;
};

std::cout << dobro(10);
```

Elas são muito utilizadas com algoritmos.

```cpp
std::vector<int> numeros{1, 2, 3, 4, 5};

std::for_each(
    numeros.begin(),
    numeros.end(),
    [](int n) {
        std::cout << n << '\n';
    }
);
```

---

# 27. Capturas de lambdas

Uma lambda pode usar variáveis externas.

```cpp
int multiplicador = 10;

auto f = [multiplicador](int x) {
    return x * multiplicador;
};
```

Isso captura `multiplicador` por valor.

Por referência:

```cpp
auto f = [&multiplicador](int x) {
    return x * multiplicador;
};
```

Você também verá:

```cpp
[=]
```

capturar valores externos por valor.

E:

```cpp
[&]
```

capturar por referência.

É melhor evitar capturas genéricas indiscriminadamente em lambdas grandes, porque fica menos claro quais dados estão sendo usados.

---

# 28. Templates

Templates são uma das ferramentas mais poderosas de C++.

```cpp
template<typename T>
T maior(T a, T b) {
    return a > b ? a : b;
}
```

Agora:

```cpp
maior(10, 20);
```

gera uma versão para `int`.

E:

```cpp
maior(1.5, 3.7);
```

gera uma versão para `double`.

Você verá templates em praticamente toda a Standard Library:

```cpp
std::vector<int>
std::vector<std::string>
std::optional<Usuario>
std::unique_ptr<Arquivo>
```

Os valores dentro de `< >` são argumentos do template.

---

# 29. Algoritmos da Standard Library

Em vez de implementar tudo manualmente, C++ possui `<algorithm>`.

Exemplo:

```cpp
#include <algorithm>
#include <vector>

std::vector<int> numeros{5, 2, 8, 1};

std::sort(numeros.begin(), numeros.end());
```

Buscar:

```cpp
auto it = std::find(
    numeros.begin(),
    numeros.end(),
    8
);
```

Testar uma condição:

```cpp
bool existe = std::any_of(
    numeros.begin(),
    numeros.end(),
    [](int n) {
        return n > 10;
    }
);
```

---

# 30. Iteradores

Código como:

```cpp
numeros.begin()
```

e:

```cpp
numeros.end()
```

usa **iteradores**.

Um iterador é uma abstração parecida com um ponteiro para percorrer containers.

```cpp
auto it = numeros.begin();

std::cout << *it;
```

Normalmente você não precisa pensar profundamente neles no começo, mas eles aparecem constantemente nas APIs da STL.

---

# 31. Namespaces

Imagine duas bibliotecas definindo:

```cpp
class Client;
```

Namespaces evitam conflitos:

```cpp
namespace database {

class Client {
};

}
```

Uso:

```cpp
database::Client client;
```

A Standard Library usa:

```cpp
std
```

por isso:

```cpp
std::string
std::vector
std::cout
```

Evite colocar isto em headers:

```cpp
using namespace std;
```

E, em projetos reais, é perfeitamente razoável continuar escrevendo explicitamente:

```cpp
std::string
```

---

# 32. Headers e arquivos `.cpp`

C++ geralmente separa declarações e implementações.

Por exemplo:

```text
projeto/
    main.cpp
    pessoa.cpp
    pessoa.hpp
```

## `pessoa.hpp`

```cpp
#pragma once

#include <string>

class Pessoa {
public:
    Pessoa(std::string nome);

    void apresentar() const;

private:
    std::string nome_;
};
```

## `pessoa.cpp`

```cpp
#include "pessoa.hpp"

#include <iostream>
#include <utility>

Pessoa::Pessoa(std::string nome)
    : nome_{std::move(nome)} {
}

void Pessoa::apresentar() const {
    std::cout << nome_ << '\n';
}
```

## `main.cpp`

```cpp
#include "pessoa.hpp"

int main() {
    Pessoa pessoa{"Ana"};
    pessoa.apresentar();
}
```

Compilação:

```bash
g++ main.cpp pessoa.cpp -std=c++20 -o programa
```

---

# 33. O que `#include` realmente faz?

Diferentemente de algo como:

```python
import modulo
```

ou:

```javascript
import x from "./modulo.js"
```

`#include` é uma diretiva do preprocessador.

Conceitualmente:

```cpp
#include "pessoa.hpp"
```

faz o conteúdo daquele header ficar disponível naquela unidade de compilação.

Por isso headers precisam ser tratados com um pouco mais de cuidado do que módulos em muitas outras linguagens.

C++ moderno também possui **modules**, mas headers continuam extremamente comuns no ecossistema.

---

# 34. Compilação e linking

C++ não costuma executar diretamente um arquivo fonte.

O processo simplificado é:

```text
.cpp
 ↓
preprocessador
 ↓
compilador
 ↓
object file
 ↓
linker
 ↓
executável
```

Por exemplo:

```text
main.cpp    → main.o
pessoa.cpp  → pessoa.o
```

Depois:

```text
main.o + pessoa.o
        ↓
      linker
        ↓
     programa
```

Esse modelo explica vários erros que inicialmente parecem estranhos.

### Erro de compilação

Algo está errado no código ou nos tipos.

### Erro de linking

Uma declaração existe, mas o linker não encontrou a implementação correspondente.

Exemplo:

```cpp
void foo();

int main() {
    foo();
}
```

Isso pode compilar individualmente, mas falhar durante linking porque nenhuma implementação de `foo` existe.

---

# 35. Erros e exceções

C++ possui exceções:

```cpp
throw std::runtime_error{"algo deu errado"};
```

Capturando:

```cpp
try {
    alguma_operacao();
}
catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
}
```

Mas nem toda API usa exceções.

Dependendo do projeto, erros também podem ser representados por:

```cpp
std::optional<T>
```

status codes,

```cpp
enum class Error
```

ou tipos semelhantes a `Result`.

Portanto, não assuma que exceções são o único mecanismo de tratamento de erros em C++.

---

# 36. `enum class`

Prefira:

```cpp
enum class Estado {
    parado,
    executando,
    erro
};
```

Uso:

```cpp
Estado estado = Estado::executando;
```

Em vez do `enum` antigo:

```cpp
enum Estado {
    parado,
    executando,
    erro
};
```

`enum class` oferece melhor isolamento de nomes e checagem de tipos.

---

# 37. `switch`

```cpp
switch (estado) {
    case Estado::parado:
        break;

    case Estado::executando:
        break;

    case Estado::erro:
        break;
}
```

Lembre que `break` normalmente é necessário para evitar fallthrough.

---

# 38. Comparação com linguagens de garbage collection

Em Java/C#/JavaScript/Python você frequentemente pensa:

```text
criar objeto
↓
usar objeto
↓
runtime limpa depois
```

Em C++ o modelo típico é:

```text
objeto entra em escopo
↓
construtor
↓
objeto é usado
↓
objeto sai de escopo
↓
destrutor
```

Por exemplo:

```cpp
void executar() {
    std::vector<int> dados;

    // ...
}
```

Quando `executar()` termina, `dados` é destruído imediatamente.

Esse comportamento determinístico é muito importante.

---

# 39. Herança e métodos virtuais

C++ suporta polimorfismo clássico.

```cpp
class Animal {
public:
    virtual void falar() const {
        std::cout << "???\n";
    }

    virtual ~Animal() = default;
};
```

Implementação:

```cpp
class Cachorro : public Animal {
public:
    void falar() const override {
        std::cout << "Au!\n";
    }
};
```

Uso:

```cpp
void emitir_som(const Animal& animal) {
    animal.falar();
}
```

`override` é recomendado porque permite que o compilador verifique se você realmente está sobrescrevendo um método.

---

# 40. Não use herança para tudo

C++ permite OOP, mas C++ moderno não é uma linguagem em que tudo precisa ser uma classe hierárquica.

Composição é frequentemente preferível:

```cpp
class Carro {
    Motor motor_;
};
```

a hierarquias profundas como:

```text
Objeto
 ↓
Veiculo
 ↓
VeiculoTerrestre
 ↓
VeiculoMotorizado
 ↓
Carro
```

Templates, funções livres e composição são extremamente idiomáticos em C++.

---

# 41. Operator overloading

C++ permite definir operadores.

```cpp
struct Vec2 {
    double x;
    double y;
};

Vec2 operator+(const Vec2& a, const Vec2& b) {
    return {
        a.x + b.x,
        a.y + b.y
    };
}
```

Agora:

```cpp
Vec2 a{1, 2};
Vec2 b{3, 4};

Vec2 c = a + b;
```

É poderoso, mas operadores devem preservar expectativas intuitivas.

---

# 42. `constexpr`

`constexpr` indica que algo pode ser calculado durante compilação quando possível.

```cpp
constexpr int quadrado(int x) {
    return x * x;
}

constexpr int valor = quadrado(10);
```

Para começar, pense em `constexpr` como:

> "esta operação pode fazer parte de computação em compile time".

O sistema de compile-time programming de C++ vai muito além disso, mas você não precisa dominá-lo inicialmente.

---

# 43. Conversões

C++ possui várias conversões explícitas.

A mais comum:

```cpp
static_cast<int>(3.14)
```

Exemplo:

```cpp
double x = 3.14;
int y = static_cast<int>(x);
```

Prefira casts explícitos de C++ a:

```cpp
(int)x
```

quando possível.

Existem também:

```cpp
dynamic_cast
const_cast
reinterpret_cast
```

mas eles têm propósitos mais específicos.

`reinterpret_cast`, em particular, não é algo que você deveria precisar em código C++ normal de aplicação.

---

# 44. `std::string_view`

Quando uma função só precisa observar uma string e não precisa possuir os dados, existe:

```cpp
std::string_view
```

Exemplo:

```cpp
#include <string_view>

void imprimir(std::string_view texto) {
    std::cout << texto << '\n';
}
```

Isso permite:

```cpp
imprimir("hello");

std::string nome = "Maria";
imprimir(nome);
```

sem necessariamente copiar a string.

Mas existe uma consideração importante:

> `string_view` não possui os caracteres para os quais aponta.

Portanto, é preciso prestar atenção ao lifetime do dado original.

---

# 45. O problema de lifetime

Esse código é perigoso:

```cpp
const std::string& criar_nome() {
    std::string nome = "Ana";
    return nome;
}
```

`nome` é destruído quando a função termina.

Então a referência retornada aponta para um objeto que já não existe.

Esse tipo de erro é chamado de **dangling reference**.

Lifetime é um dos tópicos mais importantes para desenvolver intuição em C++.

---

# 46. Undefined Behavior

C++ possui o conceito de **Undefined Behavior**, ou UB.

Por exemplo:

```cpp
std::vector<int> numeros{1, 2, 3};

std::cout << numeros[999];
```

Acessar memória fora dos limites pode resultar em comportamento indefinido.

O programa pode:

* parecer funcionar
* imprimir lixo
* crashar
* corromper memória
* se comportar de maneira completamente imprevisível

Outro exemplo:

```cpp
int* ptr = nullptr;

std::cout << *ptr;
```

Também é comportamento indefinido.

Esse é um contraste importante com linguagens que necessariamente lançariam uma exceção.

---

# 47. Templates e erros gigantes

Você inevitavelmente encontrará algo como:

```text
error: no matching function for call to ...
candidate template ignored ...
substitution failure ...
```

seguido de dezenas de linhas.

Não tente ler tudo de cima para baixo imediatamente.

Procure primeiro:

1. a primeira referência ao seu arquivo;
2. a linha onde o template foi instanciado;
3. os tipos envolvidos;
4. a primeira mensagem do tipo `no matching`, `cannot convert` ou `constraints not satisfied`.

Essa habilidade poupa bastante sofrimento em C++.

---

# 48. Warnings são seus amigos

Compile com warnings.

GCC/Clang:

```bash
-Wall -Wextra -Wpedantic
```

Por exemplo:

```bash
g++ main.cpp \
    -std=c++20 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -o programa
```

Para desenvolvimento, também pode ser útil:

```bash
-Wconversion
```

Embora possa ser bastante verboso.

Evite simplesmente ignorar warnings.

---

# 49. Sanitizers

Uma das melhores ferramentas para aprender e depurar C++ são sanitizers.

Com GCC ou Clang:

```bash
g++ main.cpp \
    -std=c++20 \
    -fsanitize=address,undefined \
    -g \
    -o programa
```

Isso consegue detectar vários problemas, como:

* acesso fora de limites
* use-after-free
* algumas formas de undefined behavior
* problemas de memória

Durante desenvolvimento, sanitizers são extremamente úteis.

---

# 50. Debug vs Release

Uma compilação de desenvolvimento pode ser:

```bash
g++ main.cpp \
    -std=c++20 \
    -Wall \
    -Wextra \
    -g \
    -O0 \
    -o programa
```

`-g` inclui informações para debugger.

`-O0` desabilita otimizações importantes, facilitando debugging.

Release poderia usar:

```bash
-O2
```

ou:

```bash
-O3
```

dependendo do projeto.

---

# 51. Debuggers

Dois comuns são:

```text
gdb
lldb
```

Por exemplo:

```bash
gdb ./programa
```

Ou usando integração de IDEs como:

* Visual Studio
* CLion
* VS Code

Aprender pelo menos o básico de breakpoint, call stack e inspeção de variáveis é extremamente útil em C++.

---

# 52. Build systems

Projetos pequenos podem ser compilados manualmente:

```bash
g++ main.cpp foo.cpp bar.cpp
```

Mas projetos reais normalmente usam um build system.

Um dos mais comuns é **CMake**.

Estrutura:

```text
meu_projeto/
├── CMakeLists.txt
└── src/
    ├── main.cpp
    ├── pessoa.cpp
    └── pessoa.hpp
```

Exemplo mínimo:

```cmake
cmake_minimum_required(VERSION 3.20)

project(meu_projeto)

set(CMAKE_CXX_STANDARD 20)

add_executable(
    meu_projeto
    src/main.cpp
    src/pessoa.cpp
)
```

Build:

```bash
cmake -S . -B build
cmake --build build
```

Não precisa aprender CMake profundamente antes de aprender C++, mas rapidamente você encontrará projetos que o utilizam.

---

# 53. Uma pequena aplicação

Vamos juntar vários conceitos.

```cpp
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

struct Usuario {
    std::string nome;
    int idade;
};

int main() {
    std::vector<Usuario> usuarios{
        {"Ana", 29},
        {"Carlos", 17},
        {"Maria", 35},
        {"João", 15}
    };

    std::vector<Usuario> adultos;

    std::copy_if(
        usuarios.begin(),
        usuarios.end(),
        std::back_inserter(adultos),
        [](const Usuario& usuario) {
            return usuario.idade >= 18;
        }
    );

    std::sort(
        adultos.begin(),
        adultos.end(),
        [](const Usuario& a, const Usuario& b) {
            return a.idade < b.idade;
        }
    );

    for (const auto& usuario : adultos) {
        std::cout
            << usuario.nome
            << " - "
            << usuario.idade
            << '\n';
    }
}
```

Aqui aparecem vários conceitos típicos de C++:

```text
std::vector
struct
const T&
lambdas
iteradores
algoritmos
auto
range-based for
```

---

# 54. Algumas coisas que você provavelmente não precisa aprender primeiro

C++ é enorme.

Não tente começar por:

* template metaprogramming avançado
* allocators customizados
* metaprogramação pesada
* SFINAE
* custom memory arenas
* lock-free programming
* coroutines complexas
* detalhes da ABI
* `reinterpret_cast`
* placement `new`
* overloads extremamente avançados
* macros sofisticadas

São recursos reais e importantes em alguns domínios, mas não são pré-requisitos para escrever bom C++.

---

# 55. Coisas que você deve aprender cedo

Priorize:

1. `std::string`
2. `std::vector`
3. `const`
4. referências
5. ponteiros
6. lifetime
7. RAII
8. `std::unique_ptr`
9. `auto`
10. range-based `for`
11. lambdas
12. STL algorithms
13. headers e `.cpp`
14. compilation e linking
15. debugging e sanitizers

Depois:

16. move semantics
17. templates
18. polymorphism
19. concepts
20. ranges

---

# 56. Regras práticas de C++ moderno

Como ponto de partida, estas regras evitam muitos problemas.

### 1. Não use `new` e `delete` diretamente por padrão

Prefira:

```cpp
auto x = std::make_unique<T>();
```

ou simplesmente:

```cpp
T x;
```

---

### 2. Prefira containers da biblioteca padrão

Em vez de:

```cpp
int* valores;
```

provavelmente você quer:

```cpp
std::vector<int> valores;
```

---

### 3. Use `const` quando algo não precisa mudar

```cpp
void imprimir(const Usuario& usuario);
```

---

### 4. Passe objetos grandes por `const&`

```cpp
void processar(const std::vector<int>& dados);
```

---

### 5. Mas não passe tipos minúsculos por referência sem motivo

Isso:

```cpp
void foo(const int& x);
```

normalmente não traz benefício.

Prefira:

```cpp
void foo(int x);
```

---

### 6. Prefira `nullptr`

```cpp
Usuario* usuario = nullptr;
```

---

### 7. Prefira `enum class`

```cpp
enum class Status {
    sucesso,
    erro
};
```

---

### 8. Use RAII

O lifetime de um recurso deveria estar associado a algum objeto.

---

### 9. Compile com warnings

```bash
-Wall -Wextra -Wpedantic
```

---

### 10. Durante desenvolvimento, considere sanitizers

```bash
-fsanitize=address,undefined
```

---

# 57. C++ não é "C com classes"

É possível escrever C++ assim:

```cpp
char* buffer = new char[1000];

// ...

delete[] buffer;
```

Mas código C++ moderno normalmente seria mais parecido com:

```cpp
std::string buffer;
```

ou:

```cpp
std::vector<char> buffer;
```

Da mesma forma:

```cpp
malloc
calloc
free
```

raramente são apropriados em C++ de alto nível.

Muito C++ antigo encontrado na internet não representa como projetos modernos deveriam necessariamente ser escritos.

---

# 58. O que muda dependendo da linguagem de onde você vem

## Se você vem de Python ou JavaScript

As principais novidades serão:

* tipos estáticos
* compilation
* referências
* ponteiros
* lifetime
* value semantics
* templates
* gerenciamento determinístico de recursos

---

## Se você vem de Java ou C#

A sintaxe será familiar, mas tome cuidado com uma diferença enorme:

Em Java/C#:

```text
variável de objeto → geralmente uma referência
```

Em C++:

```cpp
Pessoa a;
Pessoa b = a;
```

normalmente cria uma **cópia real**.

Também não existe garbage collector como mecanismo padrão da linguagem.

---

## Se você vem de Go

Vai encontrar conceitos familiares como:

* structs
* interfaces conceitualmente semelhantes a abstrações
* value semantics

Mas C++ possui:

* overload
* templates muito mais poderosos
* RAII
* constructors/destructors
* inheritance
* operator overload
* references
* manual control de memória quando necessário

---

## Se você vem de Rust

Alguns conceitos serão familiares:

* stack/heap
* ownership
* RAII
* references
* move semantics
* zero-cost abstractions

Mas C++ normalmente não verifica ownership e borrowing de maneira tão rígida em compile time.

É sua responsabilidade evitar:

* dangling references
* data races
* use-after-free

---

# 59. Um exercício bom para começar

Implemente um pequeno catálogo de livros:

```cpp
struct Livro {
    std::string titulo;
    std::string autor;
    int ano;
};
```

Mantenha-os em:

```cpp
std::vector<Livro>
```

Implemente funções como:

```cpp
void adicionar_livro(...);

std::optional<Livro> buscar_por_titulo(...);

std::vector<Livro> livros_depois_de(int ano);

void ordenar_por_ano(...);
```

Isso força você a praticar:

* structs
* vector
* funções
* `const&`
* optional
* algorithms
* lambdas
* value semantics

sem precisar entrar em detalhes complicados de memória.

---

# 60. Modelo mental resumido

Se você guardar apenas algumas ideias desta introdução, guarde estas:

```text
C++ trabalha muito com valores.
```

```text
Objetos possuem lifetime determinístico.
```

```text
Referências (&) dão acesso a objetos existentes.
```

```text
Ponteiros (*) podem representar endereços e ausência.
```

```text
RAII gerencia recursos através do lifetime dos objetos.
```

```text
std::vector, std::string e outros containers devem ser preferidos
a gerenciamento manual de memória na maioria dos casos.
```

```text
Cópia e movimento são operações explícitas do modelo da linguagem.
```

```text
Compilação e linking fazem parte da experiência de programar em C++.
```

E talvez a regra mais útil:

> Em C++ moderno, você tem acesso a controle de baixo nível, mas não precisa programar em baixo nível o tempo todo.

A linguagem permite escrever abstrações bastante seguras e de alto nível enquanto mantém a possibilidade de controlar memória, layout e performance quando necessário.


