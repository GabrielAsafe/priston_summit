# Relatório de Debug – Port do Servidor SummitTale para MinGW

## Objetivo

Efetuar o port do servidor SummitTale (Priston Tale) para MinGW, eliminando incompatibilidades com o Visual C++ original e garantindo que o servidor executa corretamente em Windows utilizando binários compilados com GCC/MinGW.

---

# Problemas resolvidos

## 1. Compatibilidade ODBC

Durante a migração surgiram diversas incompatibilidades entre o compilador original e o MinGW relativamente às estruturas e funções ODBC.

Foram efetuadas as adaptações necessárias para permitir:

* compilação sem erros;
* inicialização correta do ambiente ODBC;
* ligação bem sucedida ao SQL Server.

Resultado:

```
Ligacao SQL confirmada...
```

---

## 2. Crash em local_GetWord()

O servidor terminava prematuramente durante o carregamento.

Após instrumentação verificou-se que a falha ocorria em:

```
local_GetWord()
```

Foi corrigida a causa da leitura inválida.

Resultado:

* carregamento prossegue normalmente.

---

## 3. std::bad_alloc durante carregamento dos mapas

Posteriormente surgiu:

```
std::bad_alloc
```

Foram adicionados logs em todas as fases do carregamento.

A instrumentação demonstrou que:

* todos os StageArea são alocados;
* todos os mapas são carregados.

Resultado:

```
53 mapas carregados com sucesso
```

---

## 4. Instrumentação do InitAll()

Foi adicionada instrumentação detalhada:

```
[MAIN] ANTES InitAll
...
[INIT] INICIALIZACAO CONCLUIDA
```

O objetivo foi confirmar exatamente onde ocorria a falha.

Resultado:

```
InitAll termina completamente.
```

---

## 5. Problema da CLI

Inicialmente existia:

```cpp
cin >> input;
input.back();
```

Quando stdin era fechado:

* `cin` falhava;
* `input` ficava vazio;
* ocorria assert/crash.

Foi substituído por:

```cpp
if (!(cin >> input))
{
    cerr << "[CLI] stdin fechado ou erro de leitura; encerrando CLI" << endl;
    return false;
}
```

Resultado:

* fim do stdin deixa de provocar crash.

---

## 6. Descoberta importante

Inicialmente parecia que o servidor terminava imediatamente.

Após várias verificações concluiu-se que isso era falso.

Foram usados:

```
tasklist
```

e

```
netstat
```

Verificou-se que:

* o processo permanecia ativo;
* memória continuava a crescer;
* SQL mantinha ligações abertas;
* porta 9000 encontrava-se em LISTENING.

Conclusão:

O servidor nunca estava a terminar.

---

## 7. Captura completa dos logs

Foi criada uma forma de executar o servidor redirecionando stdout/stderr.

```
Server.exe > run_stdout.txt 2> run_stderr.txt
```

e leitura contínua através do PowerShell.

Isso permitiu obter todo o processo de inicialização.

O log completo mostrou:

```
Ligacao SQL confirmada
...
Carregando NPCS...
...
[INIT] INICIALIZACAO CONCLUIDA
Servidor pronto para receber conexoes
server>
```

---

## 8. Erro Winsock 10048

Durante os testes surgiu:

```
10048 is the error bind(sock) failed
```

Foi confirmado posteriormente que já existia outra instância do servidor em execução.

Não era um problema da compilação.

---

## 9. Procura de relançamento do processo

Foi investigada a possibilidade de o servidor:

* criar outro processo;
* libertar a consola;
* arrancar como serviço.

Foram pesquisadas chamadas como:

```
CreateProcess
ShellExecute
FreeConsole
AllocConsole
DETACHED_PROCESS
CREATE_NO_WINDOW
StartServiceCtrlDispatcher
```

Resultado:

No código do servidor apenas existiam:

```
system("cls")
```

Não existe qualquer código que:

* destaque a consola;
* relance o servidor;
* execute como serviço.

---

## 10. Comparação entre binário original e MinGW

Foi utilizado:

```
objdump -p Server.exe
```

### Executável original

```
Subsystem 00000003 (Windows CUI)
```

Aplicação de consola.

### Executável compilado inicialmente

```
Subsystem 00000002 (Windows GUI)
```

Aplicação gráfica.

Este era precisamente o motivo pelo qual:

```
Server.exe
```

devolvia imediatamente o prompt do cmd.

---

## 11. Correção do linker

Foi alterado o script de link.

Antes:

```
-mwindows
```

Depois:

```
-mconsole
```

O script passou a utilizar:

```bash
i686-w64-mingw32-g++

-m32
-mconsole
```

Resultado do comando file:

```
PE32 executable for MS Windows 4.00 (console)
```

O novo executável passou a ser:

```
Windows Console
```

tal como o original.

---

## 12. Problema na cópia do executável

O novo binário não chegava ao Windows.

O SCP apresentava:

```
scp:
dest open
Failure
```

Assim, durante vários testes ainda estava a ser executado o executável antigo.

Foi identificado que o executável compilado corretamente nunca tinha substituído o existente em:

```
C:\SummitTale\Servidor\Server.exe
```

---

## 13. Instrumentação do ciclo principal

Foi instrumentado o ciclo principal de:

```
ServerWinMain()
```

Foram adicionados logs antes e depois de:

* SetupDefWindow
* PeekMessage
* DispatchMessage
* WaitMessage

Resultado observado:

```
mensagem = 275
```

correspondente a:

```
WM_TIMER
```

O servidor encontra-se num ciclo normal de mensagens Windows:

```
WaitMessage()

↓

WM_TIMER

↓

PeekMessage()

↓

DispatchMessage()

↓

WaitMessage()
```

Não existe crash.

Não existe bloqueio.

Não existe saída inesperada.

O spam de logs deve-se apenas à instrumentação.

---

## 14. Estado atual do servidor

Atualmente verifica-se que:

* SQL inicializa corretamente;
* mapas carregam corretamente;
* itens carregam corretamente;
* NPCs carregam corretamente;
* StageArea inicializa;
* InitAll termina;
* servidor abre sockets;
* servidor entra no loop principal;
* recebe continuamente mensagens WM_TIMER;
* não termina.

---

# Melhorias identificadas

## Thread da CLI

Atualmente:

```cpp
std::thread cmd(...);

cmd.detach();
```

O ideal será:

```
join()
```

em vez de

```
detach()
```

para evitar destruição de recursos enquanto a CLI continua ativa.

---

## Espera do loop principal

Em vez de:

```cpp
WaitMessage();
```

foi proposta:

```cpp
MsgWaitForMultipleObjectsEx(...)
```

para permitir verificar periodicamente:

```
done == true
```

e terminar corretamente quando solicitado.

---

## Remover logs temporários

Os logs de:

```
WM_TIMER
WaitMessage
DispatchMessage
```

devem ser removidos após o debug, pois apenas geram grandes quantidades de texto sem indicar qualquer problema.

---

# Estado atual

Até ao momento foi confirmado que:

* O servidor compila integralmente em MinGW.
* Todos os 186 ficheiros são compilados com sucesso.
* O link é efetuado corretamente.
* O executável final é uma aplicação de consola.
* A inicialização completa do servidor funciona.
* A ligação SQL funciona.
* O carregamento dos mapas funciona.
* O servidor entra corretamente no ciclo principal de mensagens Windows.
* O processo permanece em execução.

O próximo passo consiste em garantir que o executável de consola recém-compilado substitui efetivamente o binário existente no Windows e remover toda a instrumentação temporária de debug após validação final.

