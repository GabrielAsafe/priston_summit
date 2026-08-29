# Documentação de Debug — Migração MSVC → MinGW no Cliente Priston/MMORPG

## 1. Objetivo do diagnóstico

O problema apareceu durante a migração de um cliente legado de MMORPG, originalmente compilado com MSVC, para MinGW.

Os sintomas principais foram:

- falha no login;
- usuário aparentemente corrompido;
- controles `EDIT` inicialmente problemáticos;
- comportamento diferente entre MSVC e MinGW;
- depois de algumas alterações, o botão de login e o Enter pareciam deixar de responder após uma tentativa.

A estratégia usada foi evitar alterações grandes e localizar exatamente **em que ponto os dados deixavam de estar corretos**.

---

# 2. Primeiros problemas resolvidos

Durante a migração, alguns problemas iniciais já haviam sido corrigidos:

- crash no `WinMain`;
- funcionamento dos controles `EDIT`;
- leitura correta dos campos usando `GetWindowTextA()`;
- início correto da thread responsável pelo login.

Isso permitiu reduzir o problema para o fluxo específico:

```text
EDIT do Windows
→ GetWindowTextA
→ UserAccount / UserPassword
→ KeyFilter
→ thread de login
→ requestLogin
→ montagem do pacote
→ socket
→ servidor
```

A ideia central do debug foi validar cada etapa individualmente.

---

# 3. Validação direta do conteúdo do EDIT

Foi adicionada instrumentação imediatamente após:

```cpp
GetWindowTextA(...)
```

A saída obtida foi:

```text
class=Edit
textLengthA=6
after GetWindowTextA
strlen=6
text=[123123]
bytes=31 32 33 31 32 33 00
```

Isso comprovou várias coisas ao mesmo tempo.

## 3.1 O controle Windows estava correto

O `EDIT` realmente continha:

```text
123123
```

Portanto, o problema não estava mais no controle visual.

## 3.2 A versão ANSI da API estava funcionando

Foi usado explicitamente:

```cpp
GetWindowTextA(...)
```

e não uma macro ambígua como:

```cpp
GetWindowText(...)
```

Isso foi importante porque uma migração MSVC/MinGW pode expor diferenças relacionadas a:

```cpp
UNICODE
_UNICODE
TCHAR
LPCTSTR
LPCWSTR
LPCSTR
```

Com `GetWindowTextA`, o contrato fica explícito:

```text
char*
ANSI
```

## 3.3 O tamanho da string estava correto

Foi verificado:

```cpp
strlen(UserAccount)
```

Resultado:

```text
6
```

## 3.4 Os bytes reais foram inspecionados

Em vez de confiar apenas em:

```cpp
printf("%s", UserAccount);
```

foram impressos os bytes:

```text
31 32 33 31 32 33 00
```

que correspondem exatamente a:

```text
1 2 3 1 2 3 \0
```

Essa técnica foi muito importante porque separa:

```text
"dado realmente corrompido"
```

de:

```text
"logger ou formatação exibindo o dado errado"
```

---

# 4. Validação antes e depois do KeyFilter

Também foi instrumentado o código em torno de:

```cpp
KeyFilter(...)
```

Antes:

```text
strlen=6
text=[123123]
bytes=31 32 33 31 32 33 00
```

Depois:

```text
strlen=6
text=[123123]
bytes=31 32 33 31 32 33 00
```

Conclusão:

```text
KeyFilter não estava corrompendo UserAccount.
```

Essa técnica é um exemplo de **debug por fronteiras**.

Em vez de estudar toda a implementação da função inicialmente, fazemos:

```text
estado antes
estado depois
```

Se forem iguais, a função pode ser temporariamente eliminada da lista de suspeitos.

---

# 5. Rastreamento de todas as definições de UserAccount

Como `UserAccount` era uma variável global, uma hipótese forte passou a ser:

```text
existem duas cópias diferentes de UserAccount no executável?
```

Foi usada busca textual:

```bash
grep -Rnw . -e "UserAccount"
```

Foram encontrados, entre outros:

```cpp
HoOpening.cpp
char UserAccount[ACCOUNT_SIZE];
```

e no header:

```cpp
extern char UserAccount[ACCOUNT_SIZE];
```

Também apareceu:

```cpp
TJBOY/clanmenu/tjclan.cpp
```

com algo semelhante a:

```cpp
#ifdef ...
extern char UserAccount[ACCOUNT_SIZE];
#else
char UserAccount[ACCOUNT_SIZE];
#endif
```

Isso levantou a suspeita de uma **duplicate global definition** dependendo das macros de compilação.

---

# 6. Por que somente grep não era suficiente

A presença textual de:

```cpp
char UserAccount[ACCOUNT_SIZE];
```

em dois arquivos não prova que duas variáveis chegaram ao binário.

Preprocessamento pode transformar:

```cpp
#ifdef ...
extern ...
#else
char ...
#endif
```

em apenas uma das alternativas.

Por isso, o próximo passo foi consultar diretamente os símbolos produzidos pelo compilador e linker.

---

# 7. Uso de nm para investigar símbolos reais

Foi usado:

```bash
nm
```

nos `.o` e no `Game.exe`.

Resultado relevante:

```text
Game.exe
B UserAccount
```

Nos objetos:

```text
HoOpening.o      B UserAccount
outros objetos   U UserAccount
tjclan.o         U UserAccount
```

Significado simplificado:

```text
B = símbolo realmente definido em BSS
U = símbolo undefined/referenciado externamente
```

Isso provou que havia apenas **uma definição real** de:

```cpp
UserAccount
```

na build atual.

Conclusão:

```text
A hipótese de duas instâncias globais diferentes de UserAccount foi descartada.
```

---

# 8. Técnica importante: verificar o binário, não apenas o fonte

Essa etapa foi especialmente útil na migração.

Código-fonte pode sugerir uma coisa, mas:

```text
preprocessador
compilador
linker
flags
bibliotecas
build antiga
```

podem produzir outra.

Ferramentas como:

```bash
nm
objdump
strings
```

permitem consultar diretamente o artefato final ou os objetos.

---

# 9. Inspeção de requestLogin

Foi examinada a função:

```cpp
requestLogin()
```

Ela fazia essencialmente:

```cpp
strcpy_s(
    packet.account,
    sizeof(packet.account),
    username.c_str()
);

strcpy_s(
    packet.password,
    sizeof(packet.password),
    password.c_str()
);
```

Não havia transformação aparente que justificasse o caractere estranho.

Isso eliminou mais uma região do código como suspeita imediata.

---

# 10. Surgimento do caractere estranho no log

Em determinado momento o log mostrava:

```text
before requestLogin user=►
```

apesar de o campo anteriormente conter:

```text
123123
```

Esse tipo de contradição levou a duas hipóteses:

1. `UserAccount` estava sendo corrompido entre a leitura do EDIT e a thread;
2. o próprio logger estava exibindo `%s` de forma enganosa ou estávamos executando uma build diferente.

---

# 11. Auditoria do logger variádico

O logger utilizado era:

```cpp
static void MINGW_LoginTrace(const char* fmt, ...)
{
    FILE* f = fopen("mingw_login_thread.txt", "a");
    if (!f) return;

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    fprintf(f, "\n");
    va_end(args);

    fflush(f);
    fclose(f);
}
```

A implementação, isoladamente, parecia válida:

```cpp
va_start
vfprintf
va_end
```

Mas como o objetivo era eliminar completamente o logger como variável, foi proposta uma técnica melhor.

---

# 12. Bypass do logger

Em vez de:

```cpp
MINGW_LoginTrace(
    "before requestLogin user=%s",
    UserAccount
);
```

foi sugerido escrever diretamente no arquivo:

```cpp
{
    FILE *f = fopen("mingw_login_thread.txt", "a");

    if (f)
    {
        fprintf(
            f,
            "before requestLogin addr=%p strlen=%u text=[%s] bytes=",
            (void *)UserAccount,
            (unsigned)strlen(UserAccount),
            UserAccount
        );

        for (unsigned i = 0; i < 32; ++i)
            fprintf(
                f,
                "%02X ",
                (unsigned char)UserAccount[i]
            );

        fprintf(f, "\n");
        fclose(f);
    }
}
```

Essa instrumentação mede simultaneamente:

- endereço da variável;
- comprimento;
- representação textual;
- bytes brutos.

Exemplo esperado:

```text
before requestLogin
addr=00ABCDEF
strlen=6
text=[123123]
bytes=31 32 33 31 32 33 00 ...
```

---

# 13. Por que imprimir endereço também é útil

Foi usado:

```cpp
(void *)UserAccount
```

com:

```cpp
%p
```

Isso serve para investigar situações como:

```text
duas variáveis com o mesmo nome
ponteiro alterado
buffer diferente sendo observado
memória thread-local inesperada
```

Mesmo quando o linker indica um único símbolo, acompanhar o endereço durante o fluxo é uma técnica útil.

---

# 14. Fluxo da thread de login

A instrumentação mostrou:

```text
CLICK_PLAY before LoginDataServer
ENTER LoginDataServer
before CreateThread
after CreateThread
CLICK_PLAY after LoginDataServer
ENTER HoLoginThreadProc
before ConnectServer_InfoMain
after ConnectServer_InfoMain
before requestLogin
after requestLogin result=0
```

Isso foi extremamente importante porque mostrou que:

```text
o clique estava sendo recebido
LoginDataServer estava sendo chamado
CreateThread funcionava
HoLoginThreadProc realmente começava
ConnectServer_InfoMain retornava
requestLogin era chamado
```

Portanto, dizer simplesmente:

```text
"o botão não funciona"
```

não correspondia ao fluxo interno.

O botão funcionava até determinado ponto.

---

# 15. Debug de fluxo usando marcadores ENTER / BEFORE / AFTER

Uma técnica muito usada durante o diagnóstico foi colocar logs simples como:

```cpp
MINGW_LoginTrace("ENTER LoginDataServer");
MINGW_LoginTrace("before CreateThread");
MINGW_LoginTrace("after CreateThread");
```

O mesmo para outras funções:

```text
ENTER função
before chamada importante
after chamada importante
```

Essa técnica é extremamente eficiente em código legado porque não exige debugger interativo.

Ela permite descobrir:

```text
a função foi chamada?
onde parou?
uma chamada retornou?
qual foi o último ponto executado?
```

---

# 16. Interpretação correta de result=0

O log mostrou:

```text
after requestLogin result=0
```

Foi importante não interpretar isso automaticamente como:

```text
login aceito
```

Dependendo da implementação:

```text
0
```

pode significar apenas:

```text
a chamada de envio foi executada
o pacote foi aceito pela camada local
não houve erro imediato
```

O servidor ainda pode:

- rejeitar o login;
- não responder;
- interpretar o pacote incorretamente;
- receber estrutura com layout errado;
- esperar outro protocolo.

Portanto:

```text
requestLogin retornou
```

não é o mesmo que:

```text
autenticação bem-sucedida.
```

---

# 17. Investigação de StartConnectServerFlag

Quando Enter e botões aparentemente pararam de responder, foi feita busca por:

```bash
grep -Rnw SrcGame/src/HoBaram \
    -e "StartConnectServerFlag" \
    -e "hHoLoginThread" \
    -e "VK_RETURN" \
    -e "LoginDataServer()" \
    --include="*.cpp" \
    --include="*.h"
```

Isso localizou condições como:

```cpp
if (
    hFocusWnd &&
    ConnectError == ERROR_NONE &&
    StartConnectServerFlag == FALSE
)
```

Também foram encontradas várias regiões que faziam:

```cpp
StartConnectServerFlag = TRUE;
```

ou:

```cpp
StartConnectServerFlag = FALSE;
```

Isso levou à hipótese:

```text
Após uma tentativa, StartConnectServerFlag pode permanecer TRUE.
```

Se isso acontecer, novos cliques ou Enter são bloqueados pelas próprias condições da interface.

---

# 18. Investigação da thread com hHoLoginThread

Também apareceu:

```cpp
HANDLE hHoLoginThread = 0;
```

e lógica semelhante a:

```cpp
if (!hHoLoginThread)
{
    hHoLoginThread = CreateThread(...);
}
```

Depois:

```cpp
hHoLoginThread = 0;
```

Isso é relevante porque, se o handle nunca for zerado devido a algum fluxo inesperado:

```text
LoginDataServer não criará uma nova thread.
```

Ao diagnosticar botões que "param", é importante verificar não apenas input, mas também estados internos usados como locks.

---

# 19. Técnica de busca reversa pelo estado

Em vez de perguntar:

```text
"por que o botão não funciona?"
```

foi procurado:

```text
"quais variáveis impedem LoginDataServer de ser chamado?"
```

Nesse caso:

```cpp
StartConnectServerFlag
ConnectError
hHoLoginThread
LoginInputState
```

Esse método é especialmente eficaz em código legado orientado a flags.

---

# 20. Descoberta crítica: código-fonte novo, executável antigo

O ponto mais importante da última etapa foi comparar o código-fonte, o binário compilado e o log do Windows.

No fonte havia:

```cpp
"before requestLogin addr=%p strlen=%u text=[%s] bytes="
```

O comando:

```bash
grep -n \
    -e "before requestLogin addr=" \
    -e "before requestLogin user=" \
    SrcGame/src/HoBaram/HoOpening.cpp
```

encontrou a nova instrumentação.

Depois:

```bash
strings dist_mingw_client_i686/Game.exe |
grep -E "before requestLogin (addr|user)"
```

retornou:

```text
before requestLogin addr=%p strlen=%u text=[%s] bytes=
```

Portanto:

```text
o Game.exe recém-compilado continha o código novo.
```

Entretanto, no Windows o log ainda mostrava:

```text
before requestLogin user=►
```

Essa string pertencia à instrumentação anterior.

Conclusão:

```text
O executável que estava rodando no Windows não correspondia ao Game.exe recém-compilado.
```

---

# 21. Técnica: procurar strings dentro do executável

O comando:

```bash
strings Game.exe
```

é muito útil para verificar se uma build contém determinada alteração.

Exemplo:

```bash
strings dist_mingw_client_i686/Game.exe |
grep "before requestLogin"
```

Isso responde rapidamente:

```text
esta mensagem existe realmente neste executável?
```

É uma forma prática de identificar:

- build antiga;
- executável copiado errado;
- diretório de saída errado;
- recompilação que não ocorreu;
- programa sendo executado de outra pasta.

---

# 22. Verificação definitiva por hash

Foi proposta comparação SHA-256 entre Linux e Windows.

No Linux:

```bash
sha256sum dist_mingw_client_i686/Game.exe
```

Além disso:

```bash
stat -c \
    'size=%s modified=%y file=%n' \
    dist_mingw_client_i686/Game.exe
```

No Windows:

```bat
certutil -hashfile Game.exe SHA256
```

e:

```bat
dir Game.exe
```

Os hashes devem ser idênticos.

Essa é a maneira mais confiável de responder:

```text
"Este arquivo é exatamente o mesmo binário?"
```

Sem hash, comparar apenas:

```text
nome
data
tamanho
```

não é prova absoluta.

---

# 23. Verificação de build diretamente no Windows

Também foi sugerido procurar a nova string dentro do `Game.exe` do Windows:

```bat
powershell -command "$b=[System.IO.File]::ReadAllBytes('Game.exe'); $s=[System.Text.Encoding]::ASCII.GetString($b); if($s.Contains('before requestLogin addr=')){'NEW BUILD'}else{'OLD BUILD'}"
```

Resultado esperado para a nova build:

```text
NEW BUILD
```

Caso retornasse:

```text
OLD BUILD
```

estaria comprovado que a cópia em execução estava desatualizada.

---

# 24. Eliminar processos antigos antes de testar

Também foi recomendado:

```bat
taskkill /F /IM Game.exe
```

Isso evita situações nas quais:

- um processo antigo ainda está rodando;
- outro `Game.exe` continua aberto;
- o binário não pode ser substituído corretamente;
- o teste está sendo feito em processo diferente do esperado.

---

# 25. Limpeza dos logs antes de cada teste

Outra técnica importante:

```bat
del mingw_login_thread.txt
del mingw_login_input_deep.txt
```

antes de iniciar uma nova sessão.

Sem limpar os logs, é fácil misturar:

```text
execução antiga
execução nova
build anterior
build atual
```

e interpretar uma linha velha como se tivesse acabado de acontecer.

---

# 26. Executar pelo diretório explicitamente

Foi recomendado:

```bat
cd /d C:\Users\server\Desktop\cli
Game.exe
```

em vez de abrir um atalho ou clicar em uma cópia desconhecida.

Isso reduz a possibilidade de executar:

```text
Game.exe de outra pasta
atalho antigo
launcher apontando para outra instalação
```

---

# 27. Técnica geral usada: redução progressiva do espaço de busca

A investigação inteira seguiu aproximadamente esta estratégia:

```text
1. O Windows EDIT tem os dados corretos?
        sim

2. GetWindowTextA copia corretamente?
        sim

3. UserAccount está correto?
        sim

4. KeyFilter altera?
        não

5. Há duas definições globais?
        não

6. requestLogin altera obviamente os dados?
        aparentemente não

7. A thread realmente começa?
        sim

8. A conexão chega a ConnectServer_InfoMain?
        sim

9. requestLogin é realmente chamado?
        sim

10. O log estranho prova corrupção?
        ainda não

11. Estamos sequer executando a build instrumentada?
        aparentemente não
```

Essa metodologia é muito mais eficiente do que modificar aleatoriamente funções.

---

# 28. Técnicas de debug que mais ajudaram

## 28.1 Imprimir os bytes, não somente `%s`

Use:

```cpp
for (unsigned i = 0; i < N; ++i)
{
    fprintf(
        f,
        "%02X ",
        (unsigned char)buffer[i]
    );
}
```

Isso identifica:

- terminador `00`;
- caracteres invisíveis;
- UTF-16 interpretado como ANSI;
- buffer não inicializado;
- corrupção real.

---

## 28.2 Imprimir strlen junto com a string

Exemplo:

```cpp
fprintf(
    f,
    "strlen=%u text=[%s]\n",
    (unsigned)strlen(buffer),
    buffer
);
```

Se:

```text
strlen
texto
bytes
```

não concordarem, isso é um sinal importante de corrupção ou uso inválido da memória.

---

## 28.3 Imprimir endereços

```cpp
fprintf(
    f,
    "addr=%p\n",
    (void *)buffer
);
```

Útil para detectar se diferentes pontos estão realmente acessando a mesma memória.

---

## 28.4 Instrumentação antes/depois

Exemplo:

```cpp
log("before KeyFilter");
KeyFilter(...);
log("after KeyFilter");
```

ou:

```cpp
log("before requestLogin");
result = requestLogin(...);
log("after requestLogin");
```

É uma das técnicas mais úteis para código legado.

---

## 28.5 Marcadores de entrada de função

```cpp
log("ENTER HoLoginThreadProc");
```

permite distinguir:

```text
função nunca chamada
```

de:

```text
função chamada mas travou internamente.
```

---

## 28.6 Verificar resultados imediatamente

Exemplo:

```cpp
HANDLE h = CreateThread(...);

fprintf(
    f,
    "CreateThread handle=%p threadId=%lu\n",
    h,
    threadId
);
```

Não basta registrar:

```text
before CreateThread
```

É importante registrar também:

```text
after CreateThread
```

---

## 28.7 Buscar todos os usos de uma variável de estado

Exemplo:

```bash
grep -Rnw . -e "StartConnectServerFlag"
```

Depois analisar:

```text
quem seta TRUE?
quem seta FALSE?
quem testa?
qual caminho pode deixar a flag presa?
```

Muito útil em engines antigas baseadas em estado global.

---

## 28.8 Verificar símbolos com nm

Exemplo:

```bash
nm arquivo.o | grep UserAccount
```

Ajuda a diferenciar:

```text
D/B = definição
U = referência externa
```

Isso é superior a depender somente de `grep`.

---

## 28.9 Inspecionar o executável com strings

```bash
strings Game.exe | grep "texto único"
```

Útil para provar que determinada alteração realmente chegou ao binário.

---

## 28.10 Comparar binários com hash

Linux:

```bash
sha256sum Game.exe
```

Windows:

```bat
certutil -hashfile Game.exe SHA256
```

Essa técnica deveria fazer parte do processo sempre que existe:

```text
compilação em uma máquina
execução em outra máquina
cópia manual do executável
```

---

# 29. Armadilhas encontradas

## 29.1 Confiar demais em um único printf

Ver:

```text
user=►
```

não prova por si só que a memória contém `►`.

Antes de concluir corrupção, verificar:

```text
strlen
bytes
endereço
logger
build
```

---

## 29.2 Confundir fonte com binário executado

O fonte pode conter a correção, e o binário compilado também, mas o usuário pode estar rodando:

```text
outra cópia
outro diretório
atalho antigo
build anterior
```

Isso aconteceu durante o diagnóstico.

---

## 29.3 Interpretar retorno local como resposta do servidor

```text
requestLogin result=0
```

não significa necessariamente:

```text
credenciais aceitas.
```

É preciso seguir o fluxo até a resposta de rede.

---

## 29.4 Considerar botão "morto" quando existe uma flag bloqueando-o

Interfaces legadas frequentemente fazem:

```cpp
if (!busy)
    ProcessClick();
```

Se uma flag como:

```cpp
StartConnectServerFlag
```

ficar presa, a interface visual continua viva, mas determinadas ações deixam de ser aceitas.

---

## 29.5 Suspeitar de duplicate global apenas pelo grep

Código condicional pode fazer:

```cpp
#ifdef X
extern ...
#else
char ...
#endif
```

A verdade final está no objeto/linker, não somente no texto.

---

# 30. Estado atual do diagnóstico

Até o ponto documentado, foi provado que:

- `EDIT` está funcionando;
- `GetWindowTextA()` retorna corretamente;
- `UserAccount` contém os bytes corretos logo após a leitura;
- `KeyFilter()` não altera `UserAccount`;
- existe apenas uma definição real de `UserAccount` na build atual;
- a thread de login é criada;
- `HoLoginThreadProc` é executada;
- `ConnectServer_InfoMain` retorna;
- `requestLogin()` é chamado;
- `requestLogin()` retorna `0`;
- a interface possui bloqueios baseados em `StartConnectServerFlag`;
- o `Game.exe` recém-compilado possui a nova instrumentação;
- o log gerado no Windows ainda correspondia à instrumentação antiga.

O último ponto é crítico:

```text
a build sendo executada no Windows aparentemente não era a mesma build verificada no Linux.
```

Antes de investigar corrupção adicional, o próximo teste precisa garantir a identidade dos executáveis por SHA-256.

---

# 31. Próximas etapas recomendadas

Depois de confirmar que o `Game.exe` do Windows é exatamente o mesmo gerado pelo MinGW:

1. limpar os logs;
2. executar o cliente;
3. inserir um usuário conhecido, por exemplo:

```text
123123
```

4. acionar login;
5. verificar a nova linha:

```text
before requestLogin addr=...
strlen=6
text=[123123]
bytes=31 32 33 31 32 33 00
```

Se o conteúdo estiver correto dentro da thread, a investigação deve avançar para:

```text
requestLogin
→ estrutura do pacote
→ tamanho da estrutura
→ packing/alignment
→ Send
→ bytes enviados pela rede
→ interpretação pelo servidor
```

Se estiver incorreto dentro da thread, então o intervalo de corrupção está reduzido a:

```text
leitura correta do EDIT
→ lógica do botão/login
→ criação da thread
→ início de HoLoginThreadProc
```

---

# 32. Possível próximo foco da migração MSVC → MinGW

Se `UserAccount` continuar correto até `requestLogin`, uma das suspeitas mais relevantes passa a ser **layout de estruturas de rede**.

Migrações MSVC → GCC/MinGW podem expor diferenças em:

```cpp
sizeof(struct)
padding
alignment
#pragma pack
tipos com tamanho dependente da plataforma
bool
enum
long
ponteiros
calling convention
```

Para pacotes enviados diretamente por socket, devem ser registrados:

```cpp
sizeof(packet)
offsetof(packet, account)
offsetof(packet, password)
```

e os bytes finais enviados.

O princípio é o mesmo usado até agora:

```text
não assumir que o pacote está errado;
medir exatamente o que existe na memória e o que sai pelo socket.
```

---

# 33. Resumo da metodologia

A técnica geral aplicada foi:

```text
OBSERVAR
↓
ISOLAR
↓
MEDIR
↓
ELIMINAR HIPÓTESES
↓
REDUZIR O INTERVALO DO ERRO
↓
VALIDAR O BINÁRIO REAL
```

Em código legado, especialmente durante migração de compilador, esta abordagem é mais confiável do que tentar corrigir o código baseado apenas no sintoma.

As ferramentas que mais ajudaram foram:

```text
grep
nm
strings
sha256sum
certutil
logs FILE/fprintf
strlen
dump hexadecimal
impressão de endereços
marcadores ENTER/before/after
```

O princípio mais importante de todo o debug foi:

> Nunca concluir que um dado está corrompido com base apenas na forma como ele apareceu em um log. Verificar o comprimento, os bytes, o endereço, o binário executado e o ponto exato do fluxo onde a mudança ocorreu.