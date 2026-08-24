# Relatório de ferramentas de shell usadas na investigação

Durante a investigação do cliente Summit Tale/Priston Tale compilado com MinGW, usamos principalmente ferramentas Unix simples combinadas em pipelines. A estratégia foi dividir o problema em algumas áreas: localizar código e ficheiros, inspecionar trechos específicos do source, analisar logs, examinar executáveis e objetos compilados, investigar headers/includes e automatizar patches.

Uma das vantagens dessa abordagem é que praticamente tudo pode ser reproduzido apenas pelo terminal, sem debugger gráfico.

## 1. `grep` — principal ferramenta de investigação do código

`grep` foi provavelmente a ferramenta mais usada. Serviu para descobrir onde funções, variáveis, mensagens Win32 e símbolos eram usados.

Um exemplo real foi a procura por `UserAccount`:

```bash
grep -Rnw SrcGame/src \
    -e "UserAccount" \
    --include="*.cpp" \
    --include="*.c" \
    --include="*.h"
```

As flags principais foram:

- `-R`: pesquisa recursivamente nos subdiretórios.
- `-n`: mostra o número da linha.
- `-w`: exige uma palavra inteira.
- `-e`: permite especificar um ou mais padrões.
- `--include="*.cpp"`: limita a pesquisa a determinados tipos de ficheiro.
- `--exclude`: ignora determinados ficheiros.
- `--exclude-dir`: ignora diretórios completos.

Também usamos `-F`:

```bash
grep -RnwF SrcGame/src \
    -e "char UserAccount[ACCOUNT_SIZE]"
```

`-F` significa **fixed strings**. O texto deixa de ser interpretado como expressão regular. Isso é útil quando temos caracteres como:

```text
[
]
*
(
)
```

que possuem significado especial em regex.

### Expressões regulares com `-E`

Usamos bastante:

```bash
grep -Rns -E 'ServerInfo|ServerPort|szServerIp|ConnectServer_InfoMain' SrcGame
```

`-E` ativa expressões regulares estendidas.

O caractere:

```text
|
```

passa a significar **OU**.

Portanto:

```regex
ServerInfo|ServerPort|ServerIP
```

significa:

> procure ServerInfo OU ServerPort OU ServerIP.

Outro padrão usado:

```bash
grep -E 'MSG=0x(0100|0101|0102|0104|0105|0109)'
```

permitiu selecionar somente mensagens Win32 relacionadas ao teclado.

### Contexto com `-A`, `-B` e `-C`

Usámos várias vezes:

```bash
grep -n -A100 -B20 "sod2INFOindex" arquivo.cpp
```

Significado:

- `-A100`: mostra 100 linhas **after**, depois da correspondência.
- `-B20`: mostra 20 linhas **before**, antes da correspondência.
- `-C20`: mostra contexto de 20 linhas dos dois lados.

Isso é especialmente útil para localizar uma função e imediatamente ler o seu corpo.

---

## 2. `find` — descoberta física de ficheiros

Quando não sabíamos onde estava um header ou um objeto, usamos `find`.

Por exemplo:

```bash
find . -type f -iname 'smPacket.h' -print
```

As partes importantes são:

- `.`: começa no diretório atual.
- `-type f`: somente ficheiros.
- `-iname`: nome sem diferenciar maiúsculas de minúsculas.
- `-name`: nome diferenciando maiúsculas/minúsculas.
- `-print`: imprime o resultado.

Encontrámos:

```text
./Shared/smPacket.h
./Shared2/smPacket.h
...
```

Também usamos:

```bash
find /usr /opt /mingw* -type f -iname 'd3dx9.h'
```

para descobrir quais versões dos headers DirectX estavam instaladas.

### `-print0`

Para nomes de ficheiros que podem possuir espaços:

```bash
find . -type f -name '*.o' -print0
```

e depois:

```bash
while IFS= read -r -d '' obj; do
    ...
done
```

O `-print0` separa nomes com byte NUL em vez de espaço ou newline. Isso evita problemas com caminhos como:

```text
Área de Trabalho
Summit Tale Client Full
```

---

## 3. `sed` — visualizar uma faixa exata do código

Foi usado constantemente para abrir partes pequenas de ficheiros grandes:

```bash
sed -n '930,1085p' SrcGame/src/HoBaram/HoOpening.cpp
```

Significado:

- `-n`: não imprime o ficheiro inteiro automaticamente.
- `'930,1085p'`: imprime apenas linhas 930 até 1085.

Isso é muito melhor que abrir um `.cpp` de milhares de linhas inteiro.

Também usamos:

```bash
sed -n '/smWINSOCK::ConnectSock/,/^}/p' SrcGame/src/smwsock.cpp
```

Aqui usamos padrões, não números.

Significa:

> imprime desde a linha contendo `smWINSOCK::ConnectSock` até uma linha que começa com `}`.

É muito útil para extrair aproximadamente o corpo de uma função.

---

## 4. `nl` — mostrar números das linhas

Usámos:

```bash
nl -ba SrcGame/src/HoBaram/HoOpening.cpp | sed -n '1208,1232p'
```

`nl` numera as linhas.

Flags:

- `-b a`: numera **all**, inclusive linhas vazias.

A combinação:

```text
nl -ba | sed
```

é excelente quando o compilador informa algo como:

```text
HoOpening.cpp:1222:47
```

porque conseguimos visualizar exatamente a linha 1222 e o contexto próximo.

---

## 5. `cat` — conteúdo completo de ficheiros pequenos e logs

Usámos principalmente em logs:

```bash
cat mingw_socket_recv.txt
cat mingw_login_thread.txt
cat mingw_useraccount_changes.txt
```

`cat` é apropriado quando o ficheiro é relativamente pequeno.

Para ficheiros grandes, `tail`, `head`, `grep` ou `sed` são melhores.

Um detalhe importante descoberto durante a investigação: isto está errado:

```bash
cat cd "/algum/caminho"
```

porque `cat` tenta abrir um ficheiro chamado `cd`.

O correto é:

```bash
cd "/algum/caminho" && \
cat arquivo.txt
```

---

## 6. `head` e `tail` — limitar grandes quantidades de saída

Para olhar as últimas mensagens de um log:

```bash
tail -n 120 mingw_login_input_deep.txt
```

Para limitar um `grep` gigantesco:

```bash
grep ... | head -n 200
```

Principais flags:

```text
-n 120
-n 200
```

indicam quantas linhas mostrar.

Também usamos:

```bash
tail -200 mingw_wndproc_all.txt
```

forma abreviada equivalente em muitos sistemas.

Essas ferramentas foram importantes porque vários `grep -R` retornavam milhares de linhas.

---

# 7. Pipelines `|`

Grande parte da investigação foi possível por combinar programas.

Por exemplo:

```bash
grep -Rnw SrcGame/src -e "UserAccount" |
grep -E "HoOpening|HoLogin"
```

O pipe:

```text
|
```

envia a saída do primeiro programa para a entrada do segundo.

Outro exemplo:

```bash
i686-w64-mingw32-nm -C -n Game.exe |
grep -i -C12 'UserAccount'
```

Primeiro listamos os símbolos do binário e depois filtramos somente a região próxima de `UserAccount`.

Essa filosofia foi usada constantemente:

```text
programa produtor
    |
filtro
    |
filtro
    |
limitador
```

---

# 8. `strings` — procurar texto dentro de executáveis

Foi essencial para confirmar se determinado patch realmente estava presente no `Game.exe`.

Exemplo:

```bash
strings Game.exe | grep -E \
'ConnectSock WSAAsyncSelect|WndProc hwnd=|WSAMessage_Read ENTER'
```

`strings` procura sequências de caracteres imprimíveis dentro de um binário.

Isso permitiu verificar se mensagens como:

```text
REAL_CONNECTSOCK_V2
POSTMESSAGE_TEST
HoRecvMessage code=
```

tinham sido realmente incluídas no executável enviado.

É uma ótima forma de responder:

> "Será que este `Game.exe` é mesmo o executável que acabei de compilar?"

Sem executar o programa.

---

# 9. `file` — identificar o formato do executável

No final da compilação usamos algo equivalente a:

```bash
file dist_mingw_client_i686/Game.exe
```

Resultado:

```text
PE32 executable for MS Windows ..., Intel i386
```

Isso confirmou:

- executável PE;
- arquitetura 32-bit;
- Intel i386.

É particularmente importante porque estávamos a investigar comportamento de ABI e stack em **i686/32 bits**.

---

# 10. `stat` — comparar timestamps e tamanho dos ficheiros

Usámos:

```bash
stat SrcGame/src/smwsock.cpp
stat "/.../cliente/Game.exe"
```

`stat` mostra:

- tamanho;
- data de modificação;
- inode;
- permissões;
- owner;
- timestamps.

Foi útil para confirmar se o `Game.exe` realmente tinha sido substituído após recompilar.

Quando existe dúvida entre:

```text
Game.exe antigo
```

e:

```text
Game.exe recém compilado
```

`stat` é muito útil.

---

# 11. `nm` — analisar símbolos do executável e objetos

Uma das descobertas mais importantes veio de:

```bash
i686-w64-mingw32-nm -C -n \
    dist_mingw_client_i686/Game.exe
```

Flags:

- `-C`: demangle dos nomes C++.
- `-n`: ordena numericamente pelos endereços.

Depois filtramos:

```bash
... | grep -i -C12 'UserAccount'
```

Isso revelou a disposição real da memória:

```text
00b4c8c0 B UserAccount
00b4c8e0 B UserPassword
00b4c900 B ratio
00b4c904 B BackGround
00b4c908 B bIsWidescreen
00b4c90c B CaretCount
00b4c910 B End
```

Foi uma descoberta importante porque mostrou que:

```text
UserPassword - UserAccount
= 0x20
= 32 bytes
```

confirmando o tamanho real da área reservada para `UserAccount`.

### Letras mostradas pelo `nm`

Alguns exemplos:

```text
T
B
U
b
t
```

Em termos gerais:

- `T`: símbolo na seção de código/texto, global.
- `t`: código/texto local.
- `B`: variável global na `.bss`.
- `b`: símbolo local na `.bss`.
- `U`: símbolo undefined naquele objeto; será fornecido por outro objeto/biblioteca.

Assim:

```text
B UserAccount
```

indica armazenamento global real.

Já:

```text
U UserAccount
```

significa que aquele `.o` apenas referencia a variável definida noutro lugar.

---

# 12. `objdump` — inspeção de PE, imports e assembly

Usámos ou planeámos usar:

```bash
i686-w64-mingw32-objdump -d -C Game.exe
```

Flags:

- `-d`: disassemble das seções executáveis.
- `-C`: demangle de símbolos C++.

Também é possível analisar imports com:

```bash
i686-w64-mingw32-objdump -p Game.exe
```

e então:

```bash
grep 'DLL Name'
```

Foi assim que o build mostrou dependências como:

```text
d3d9.dll
d3dx9_43.dll
WS2_32.dll
USER32.dll
```

Isso foi importante durante a investigação do DirectX.

---

# 13. `addr2line` — transformar endereço de crash em source

Quando temos um endereço:

```text
0x00501234
```

podemos fazer:

```bash
i686-w64-mingw32-addr2line \
    -f -C -p \
    -e Game.exe \
    0x00501234
```

Flags:

- `-e Game.exe`: executável que contém o endereço.
- `-f`: mostra o nome da função.
- `-C`: demangle de C++.
- `-p`: apresentação legível numa linha.

Com informações de debug disponíveis, pode produzir algo como:

```text
HoOpening::Draw() at SrcGame/src/HoBaram/HoOpening.cpp:1126
```

Essa ferramenta é extremamente útil quando não queremos usar debugger.

---

# 14. Pré-processador do `g++`: `-E`

Usámos o próprio compilador para descobrir **o código que o compilador realmente vê**.

Exemplo:

```bash
i686-w64-mingw32-g++ \
    -E \
    SrcGame/src/Drawsub.cpp
```

`-E` significa:

> apenas execute o pré-processador.

Ou seja:

- resolve `#include`;
- expande macros;
- processa `#ifdef`;
- não compila.

Foi assim que conseguimos investigar a declaração efetiva de `ID3DXFont::DrawTextA`.

Também usamos:

```bash
-dD
```

para manter definições de macros na saída pré-processada.

---

# 15. `g++ -H` — descobrir quais headers estão realmente sendo incluídos

Outro comando muito útil foi:

```bash
i686-w64-mingw32-g++ \
    -H \
    -E \
    SrcGame/src/Drawsub.cpp \
    -o /dev/null
```

`-H` mostra a árvore de headers incluídos.

A saída possui níveis como:

```text
. header
.. outro-header
... outro
```

Isso foi importante para descobrir se o compilador estava usando:

```text
Dependency/DirectX9/Include/d3dx9core.h
```

ou:

```text
/usr/i686-w64-mingw32/.../d3dx9core.h
```

Essa técnica é muito melhor que simplesmente executar:

```bash
find / -name d3dx9.h
```

porque `find` mostra **o que existe**, enquanto `g++ -H` mostra **o que aquela compilação realmente escolheu**.

---

# 16. `g++ -fsyntax-only` — testar sem gerar objeto

Usámos:

```bash
i686-w64-mingw32-g++ \
    ... \
    -fsyntax-only \
    SrcGame/src/Drawsub.cpp
```

`-fsyntax-only` faz parsing e verificação sem produzir `.o`.

Isso é muito útil durante patches porque evita recompilar os 203 ficheiros só para descobrir um erro simples.

Também usamos:

```text
-fpermissive
```

porque este source antigo contém construções C++ que GCC moderno considera inválidas ou problemáticas, mas que podemos temporariamente rebaixar a warnings.

---

# 17. Includes com `-I`

Exemplo:

```bash
-I SrcGame/src
-I SrcGame/src/smLib3d
-I Shared
-I Dependency/DirectX9/Include
```

`-I` adiciona um diretório à pesquisa por headers.

A ordem é importante.

Se tivermos:

```bash
-I Dependency/DirectX9/Include
-I /usr/include
```

e ambos possuírem:

```text
d3dx9.h
```

o primeiro encontrado pode vencer.

Esse detalhe foi central na investigação dos headers DirectX.

---

# 18. `tee` — ver e guardar ao mesmo tempo

Exemplo:

```bash
comando 2>&1 |
tee /tmp/drawsub_syntax_test.log
```

`tee` duplica a saída:

```text
terminal
   +
ficheiro
```

Isso permite observar imediatamente o erro e guardar uma cópia para fazer:

```bash
grep ...
```

depois.

---

# 19. Redirecionamento de stdout e stderr

Usámos bastante:

```bash
>/tmp/output.txt
2>/tmp/errors.txt
```

Significados:

- `>`: stdout.
- `2>`: stderr.
- `2>&1`: junta stderr ao stdout.
- `>>`: acrescenta ao fim em vez de substituir.

Exemplo:

```bash
wine Game.exe > wine_crash_full.txt 2>&1
```

captura tudo num só ficheiro.

---

# 20. `2>/dev/null`

Em buscas onde esperávamos vários erros de permissão:

```bash
find /usr /opt ... 2>/dev/null
```

`/dev/null` descarta dados.

Portanto:

```text
2>/dev/null
```

significa:

> descarte mensagens de erro.

Útil em descoberta, mas deve ser usado com cuidado porque também pode esconder erros importantes.

---

# 21. `sort`

Usámos:

```bash
... | sort -u
```

- `sort`: ordena.
- `-u`: remove duplicados.

Bom para resultados de `find` que podem repetir caminhos.

---

# 22. `xargs`

Exemplo:

```bash
find ... -print0 |
xargs -0 grep ...
```

`xargs` transforma a entrada em argumentos para outro programa.

`-0` combina com `find -print0`, permitindo nomes com espaços de forma segura.

---

# 23. `wc`

Foi útil para verificar se um header tinha sido destruído:

```bash
wc -c Dependency/DirectX9/Include/d3dx9core.h
```

- `-c`: conta bytes.

Durante a tentativa de escrever o wrapper de `d3dx9core.h`, o Python abriu o ficheiro, truncou-o e depois falhou por encoding. Verificar o tamanho com `wc -c` é uma forma rápida de detectar esse tipo de problema.

---

# 24. `cp -a` — backups antes de patches

Antes de modificar código usamos frequentemente:

```bash
cp -a arquivo.cpp arquivo.cpp.backup
```

`-a` é archive mode e preserva o máximo possível de:

- permissões;
- timestamps;
- metadados.

Isso foi importante porque fizemos várias experiências destrutivas.

Uma prática que adotámos foi usar nomes como:

```text
.bak_before_fix_drawtext_6args
.bak_safe_account_draw_YYYYMMDD_HHMMSS
```

permitindo voltar rapidamente a um estado conhecido.

---

# 25. Python como ferramenta de patch automático

Em vez de usar `sed -i` para alterações complexas, começámos a usar:

```bash
python3 - <<'PY'
...
PY
```

Isto é um **here-document**.

O shell envia todo o conteúdo até `PY` como stdin para o Python.

Foi útil para:

- criar backups;
- localizar padrões;
- substituir código;
- verificar se o padrão apareceu exatamente uma vez;
- abortar se o source estivesse diferente do esperado;
- preservar encoding;
- tornar patches idempotentes.

Exemplo conceitual:

```python
old = "texto antigo"
new = "texto novo"

if text.count(old) != 1:
    raise SystemExit("não vou alterar")

text = text.replace(old, new)
```

Isso é muito mais seguro que:

```bash
sed -i 's/coisa/outra/'
```

num source antigo grande.

---

# 26. `ssh` e `scp`

Para comunicar com a VM Windows:

```bash
ssh -p 2223 server@127.0.0.1
```

- `-p 2223`: porta SSH.

Para enviar build:

```bash
scp -P 2223 Game.exe server@127.0.0.1:"C:/Users/server/..."
```

No `scp`, a flag da porta é `-P` maiúsculo, diferente do `ssh`, que usa `-p`.

---

# 27. `virsh`

A VM do servidor foi controlada por:

```bash
virsh --connect qemu:///session list --all
virsh --connect qemu:///session start summit-server
virsh --connect qemu:///session dominfo summit-server
```

Também havia port forwarding:

```bash
virsh --connect qemu:///session \
    qemu-monitor-command summit-server --hmp \
    "hostfwd_add tcp::2223-:22"
```

Isso conecta:

```text
host 127.0.0.1:2223
        ↓
VM Windows:22
```

---

# 28. `nc`

Para testar rapidamente se a porta SSH da VM estava disponível:

```bash
nc -zv 127.0.0.1 2223
```

Flags:

- `-z`: apenas verifica conexão, não troca dados.
- `-v`: verbose.

---

# 29. Comandos Windows usados pela sessão SSH

Embora não sejam Bash, fizeram parte da mesma investigação.

### `dir`

Equivalente aproximado ao `ls`:

```cmd
dir
dir /s /b *.ini
```

- `/s`: recursivo.
- `/b`: formato simples, apenas caminhos.

### `type`

Equivalente simples ao `cat`:

```cmd
type sql.reg
```

### `tasklist`

Lista processos:

```cmd
tasklist
```

Com filtro:

```cmd
tasklist | findstr /I "mysql maria"
```

### `netstat`

Usámos:

```cmd
netstat -ano | findstr ":1433"
```

Flags:

- `-a`: todas as conexões/listeners.
- `-n`: endereços numéricos.
- `-o`: PID do processo.

Foi assim que verificámos conexões SQL Server estabelecidas entre:

```text
10.0.2.15
```

e:

```text
10.0.2.2:1433
```

### `findstr`

Equivalente aproximado ao `grep` no Windows:

```cmd
findstr /I "sql mysql maria"
```

`/I` ignora maiúsculas/minúsculas.

---

# 30. Wine e `WINEDEBUG`

Para investigar crashes sem um debugger gráfico, propusemos executar:

```bash
WINEDEBUG=+seh,+tid,+timestamp \
wine ./Game.exe
```

Categorias:

- `+seh`: Structured Exception Handling — exceções Windows.
- `+tid`: identifica thread.
- `+timestamp`: timestamp nas mensagens.

Depois filtramos por:

```bash
grep -iE \
'exception|unhandled|page fault|access violation|stack overflow'
```

A ideia é obter algo como:

```text
Unhandled page fault ...
```

e um endereço para posteriormente usar com:

```text
addr2line
objdump
```

Isso forma uma espécie de debugger textual:

```text
Wine
 ↓
endereço do crash
 ↓
addr2line
 ↓
função/linha
 ↓
objdump
 ↓
instrução
```

---

# 31. Estratégia geral que usamos

A investigação acabou por seguir um padrão muito útil para projetos C/C++ antigos:

```text
1. Descobrir
   find
   grep

2. Isolar
   sed
   nl
   head
   tail

3. Instrumentar
   logs com fprintf
   Python para patches automáticos

4. Compilar
   build completo
   ou g++ -fsyntax-only

5. Confirmar o binário
   stat
   file
   strings

6. Investigar símbolos/memória
   nm
   objdump

7. Investigar crash
   WINEDEBUG
   addr2line
   objdump

8. Comparar o resultado com o código
   grep
   nm
   logs
```

## Um dos exemplos mais fortes da utilidade dessa metodologia

Começámos apenas sabendo que o login não funcionava.

Com instrumentação e ferramentas de shell conseguimos estabelecer que o texto digitado realmente chegava corretamente ao buffer:

```text
UserAccount = "teste"
```

e observar alterações byte a byte no mesmo endereço. Depois, usando `nm`, descobrimos a disposição das variáveis globais no PE:

```text
00b4c8c0 UserAccount
00b4c8e0 UserPassword
00b4c900 ratio
...
```

Isso permitiu passar de uma investigação vaga de "o login está quebrado" para uma investigação concreta de corrupção/uso de memória em endereços conhecidos.

## Comandos que considero essenciais para manter como kit de investigação

Se tivermos de repetir uma investigação semelhante, este seria o conjunto mínimo:

```bash
find
grep
sed
nl
head
tail
cat
strings
stat
file
wc
sort
tee

i686-w64-mingw32-nm
i686-w64-mingw32-objdump
i686-w64-mingw32-addr2line
i686-w64-mingw32-g++

python3

wine
```

Juntos, eles cobrem praticamente tudo o que fizemos: descoberta do source, análise estrutural, instrumentação, compilação seletiva, inspeção do PE e localização de crashes sem depender de um debugger gráfico.