# Investigação de Compatibilidade Cliente/Servidor Summit Tale

## Objetivo

Validar se o cliente compilado a partir do código-fonte atual consegue comunicar com o servidor Summit Tale existente e determinar se existem incompatibilidades de versão, protocolo ou arquitetura.

---

# 1. Compilação do Cliente com MinGW

## Ambiente

Sistema de compilação:

* Fedora Linux
* MinGW-w64

Pacotes instalados:

```bash
sudo dnf install -y \
    mingw32-gcc \
    mingw32-gcc-c++ \
    mingw32-binutils \
    mingw32-winpthreads-static \
    mingw32-filesystem
```

Verificação:

```bash
which i686-w64-mingw32-g++
which i686-w64-mingw32-gcc
which i686-w64-mingw32-ar
which i686-w64-mingw32-nm
```

---

# 2. Resolução de Erros de Linkagem

Durante a compilação ocorreram diversos erros de símbolos indefinidos.

Foram criados stubs para funções pertencentes ao código do servidor que não devem ser compiladas para o cliente.

Exemplo:

```cpp
rsPLAYINFO::AddServerMoney(...)
```

Após adicionar os stubs necessários a linkagem passou com sucesso.

Resultado:

```text
LINK OK
SummitTaleClient.exe
```

---

# 3. Verificação do Executável Gerado

Foi confirmado:

```text
PE32+
x86-64
Subsystem: Windows GUI
```

Resultado do objdump:

```text
Subsystem 00000002 (Windows GUI)
```

---

# 4. Teste de Execução do Cliente Compilado

O executável gerado foi copiado para Windows.

Estrutura:

```text
SummitTaleClient.exe
libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

Ao executar:

```cmd
SummitTaleClient.exe
```

O processo terminava imediatamente.

Sem janela.
Sem erro.
Sem crash visível.

ERRORLEVEL:

```text
0
```

---

# 5. Instrumentação do WinMain

Foram inseridos vários pontos de debug.

Exemplo:

```cpp
MessageBoxA(
    NULL,
    "ENTROU NO WINMAIN",
    "DEBUG MINGW",
    MB_OK
);
```

Outros checkpoints:

```cpp
AFTER ANTICHEAT
AFTER READCONFIG
AFTER DIRECTSOUND
AFTER REGISTERCLASS
BEFORE CREATEWINDOWEX
```

Verificação:

```bash
strings SummitTaleClient.exe
```

Confirmou a presença de todas as mensagens.

---

# 6. Teste de Arquivos de Rastreamento

Foi adicionado código para criar:

```text
mingw_winmain_reached.txt
```

Logo no início do WinMain.

Resultado:

O arquivo nunca foi criado.

Conclusão:

```text
WinMain nunca foi executado.
```

---

# 7. Teste de Entry Point Personalizado

Foi criado:

```cpp
mingw_debug_entry.cpp
```

Com:

```cpp
DebugWinMainCRTStartup()
```

Esse código deveria criar:

```text
mingw_entry_reached.txt
```

antes de chamar WinMain.

Linkagem forçada:

```bash
-Wl,-e,DebugWinMainCRTStartup
```

Verificações:

```bash
nm SummitTaleClient.exe
```

Mostrava:

```text
DebugWinMainCRTStartup
```

Presente.

Strings:

```text
ENTROU NO DEBUG ENTRY
mingw_entry_reached.txt
```

Presentes.

Resultado:

Nenhum arquivo criado.

Conclusão:

```text
O processo termina antes de chegar ao nosso entrypoint.
```

---

# 8. Verificação de Dependências

DLLs encontradas:

```text
ADVAPI32.dll
d3d9.dll
d3dx9_43.dll
DSOUND.dll
GDI32.dll
IMM32.dll
IPHLPAPI.dll
KERNEL32.dll
ODBC32.dll
ole32.dll
SHELL32.dll
SHLWAPI.dll
USER32.dll
AVIFIL32.dll
MSVFW32.dll
WININET.dll
WINMM.dll
WS2_32.dll
```

No Windows:

```cmd
where d3dx9_43.dll
where d3d9.dll
where dsound.dll
```

Todas presentes.

---

# 9. Investigação do Cliente Original

Cliente encontrado:

```text
cliente/
└── Summit Tale Client Full/
    └── cliente/
```

Executáveis:

```text
Game.exe
pstale.exe
psupdate.exe
psupdate.patch.exe
```

---

# 10. Descoberta do Endereço do Servidor

Arquivo:

```text
Settings.ini
```

Conteúdo:

```ini
[Connect]
IP=192.168.1.111
PORT=1997
```

Portanto o cliente original tenta conectar para:

```text
192.168.1.111:1997
```

---

# 11. Investigação do Servidor

Servidor iniciado com sucesso.

Log:

```text
Conectado com sucesso
```

para todas as bases de dados.

Mensagem final:

```text
Servidor pronto para receber conexoes
```

e

```text
O Servidor esta Ativo no IP : 192.168.1.111 : 1997
```

---

# 12. Validação de Rede

Na VM do servidor:

```cmd
netstat -ano | findstr :1997
```

Resultado:

```text
0.0.0.0:1997 LISTENING
```

No host Fedora:

```bash
nc -zv 127.0.0.1 1997
```

Sucesso.

Também:

```bash
nc -zv 192.168.1.111 1997
```

Sucesso.

Logo:

```text
A porta está acessível.
```

---

# 13. Teste do Cliente Original

No Windows cliente:

```powershell
Test-NetConnection 192.168.1.111 -Port 1997
```

Resultado:

```text
PingSucceeded = True
TcpTestSucceeded = False
```

Apesar disso o host Linux consegue conectar.

Isso sugere possível interferência de:

* Firewall
* NAT da VM
* Protocolo do servidor
* Cliente incompatível

---

# 14. Comparação de Binários

Comparação realizada:

Cliente original:

```text
Game.exe
PE32
i386
```

Cliente compilado:

```text
SummitTaleClient.exe
PE32+
x86-64
```

Diferença observada:

```text
32 bits vs 64 bits
```

Porém isso não explica a incompatibilidade do cliente original, pois o cliente original também não consegue comunicar corretamente.

---

# 15. Conclusão Atual

A hipótese inicial era:

```text
Cliente compilado incorretamente.
```

As evidências atuais apontam para outra direção.

O cliente original e o servidor existente aparentam não pertencer exatamente à mesma versão.

Indicadores:

* Cliente original não comunica corretamente.
* Cliente compilado também não comunica.
* Servidor abre normalmente.
* Porta está disponível.
* Configuração de IP parece correta.

Hipótese principal:

```text
Cliente e servidor pertencem a builds diferentes.
```

Possivelmente:

* Estruturas de pacotes diferentes.
* OpCodes diferentes.
* Checks de versão diferentes.
* Formato de login diferente.
* Cliente mais novo que o servidor.
* Servidor mais novo que o cliente.

---

# Próximos Passos

## Análise estática dos binários existentes

Gerar relatórios para:

```text
Game.exe
Server.exe
1.exe
2.exe
3.exe
4.exe
5.exe
6.exe
9.exe
10.exe
```

Extrair:

* versão
* linker
* timestamp
* arquitetura
* strings
* opcodes
* referências de protocolo
* endereços IP
* verificações de versão

Objetivo:

```text
Determinar se os binários cliente e servidor pertencem à mesma geração/protocolo.
```

## Comparação de protocolos

Comparar:

```text
Game.exe
vs
Server.exe
```

Buscando:

```text
opcode
protocol
version
packet
server
client
```

## Confirmar cadeia de inicialização

Identificar:

```text
Launcher -> Game.exe
```

ou

```text
pstale.exe -> Game.exe
```

ou

```text
psupdate.exe -> Game.exe
```

para determinar o fluxo real utilizado pelo cliente original.

