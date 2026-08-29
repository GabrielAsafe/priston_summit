# SummitTale — Estado Atual da Migração MinGW (Julho 2026)

## Objetivo

Portar o cliente Summit Tale para compilação fora do Visual Studio utilizando MinGW-w64 no Fedora Linux, produzindo um executável Windows funcional.

---

# Estado Atual

## Compilação

Status:

```text
OK
```

Objetos compilados:

```text
203 objetos
```

Build:

```bash
./build_client_mingw.sh
```

Resultado:

```text
Compilação concluída sem erros bloqueantes.
```

---

# Linkagem

Status:

```text
OK
```

Executável gerado:

```text
dist_mingw_client/SummitTaleClient.exe
```

Informações:

```text
PE32+
x86-64
Windows GUI
~8.7 MB
```

Verificação:

```bash
file dist_mingw_client/SummitTaleClient.exe
```

Resultado:

```text
PE32+ executable for MS Windows
Subsystem: Windows GUI
```

---

# Problemas Resolvidos

## 1. Lua

### Situação inicial

O projeto dependia de:

```text
Lua.lib
Lualib.lib
```

compiladas para MSVC antigo.

Os símbolos exportados eram:

```text
_lua_open
_lua_close
```

incompatíveis com:

```text
x86_64-w64-mingw32
```

### Solução

Compilação de Lua nativa MinGW:

```text
build_mingw_lua/liblua.a
```

Fontes:

```text
SrcGame/src/Library/LuaSrc/Lua
```

### Correções aplicadas

Arquivos:

```text
llex.c
lobject.c
```

Foram corrigidos diversos caracteres corrompidos:

```c
/0 -> \0
/n -> \n
/r -> \r
/t -> \t
```

e várias strings inválidas.

Resultado:

```text
Lua integrada com sucesso.
```

---

## 2. D3DXCreateFontIndirect

### Erro

```text
undefined reference to D3DXCreateFontIndirect
```

### Causa

MinGW exporta:

```text
D3DXCreateFontIndirectA
D3DXCreateFontIndirectW
```

mas o cliente chamava:

```text
D3DXCreateFontIndirect
```

### Solução

Arquivo criado:

```text
SrcGame/src/d3dx_font_alias.cpp
```

Wrapper:

```cpp
D3DXCreateFontIndirect()
    -> D3DXCreateFontIndirectA()
```

Resultado:

```text
Linkagem concluída.
```

---

## 3. Símbolos Server-side

Durante a migração surgiram dezenas de símbolos ausentes:

```text
rsProcessQuest
rsGetLinkChar
Serv_ConnectPlayer
Serv_DisconnectPlayer
GetClanCode
...
```

Foi criado:

```text
client_server_stubs.cpp
```

para permitir linkagem inicial.

Posteriormente:

```text
OnSever.cpp
PartySever.cpp
Svr_Damge.cpp
DllServer.cpp
ClientFuncPos.cpp
svrEventSpawn.cpp
```

foram investigados e parcialmente reintegrados.

Atualmente:

```text
client_server_stubs.cpp ainda existe no build.
```

---

## 4. ChatBuffCnt

### Problema

Erro:

```text
multiple definition of ChatBuffCnt
```

### Causa

Definições conflitantes:

```cpp
netplay.cpp
OnSever.cpp
```

### Correção

OnSever passou a utilizar:

```cpp
extern int ChatBuffCnt;
```

Resultado:

```text
Linkagem normalizada.
```

---

# Diagnóstico Atual

## Sintoma

O executável:

```text
SummitTaleClient.exe
```

é gerado corretamente.

No Windows:

```cmd
SummitTaleClient.exe
```

retorna imediatamente ao prompt.

Sem:

```text
janela
MessageBox
erro
crash visível
```

---

# Instrumentação Aplicada

Foram adicionados checkpoints em:

```cpp
WinMain()
```

com:

```cpp
MessageBoxA(...)
```

e criação de arquivos:

```text
mingw_winmain_reached.txt
```

Checkpoints:

```text
ENTROU NO WINMAIN
AFTER ANTICHEAT
AFTER READCONFIG
AFTER DIRECTSOUND
AFTER REGISTERCLASS
BEFORE CREATEWINDOWEX
```

---

# Resultado

Strings existem dentro do executável:

```text
DEBUG MINGW
ENTROU NO WINMAIN
AFTER ANTICHEAT
AFTER READCONFIG
AFTER DIRECTSOUND
AFTER REGISTERCLASS
BEFORE CREATEWINDOWEX
```

Confirmado por:

```bash
strings SummitTaleClient.exe
```

---

# Teste do EntryPoint

Foi criado:

```text
SrcGame/src/mingw_debug_entry.cpp
```

com:

```cpp
DebugWinMainCRTStartup()
```

Este entrypoint:

```cpp
cria mingw_entry_reached.txt
mostra MessageBox
```

---

## Linkagem

O linker foi configurado para:

```bash
-Wl,-e,DebugWinMainCRTStartup
```

Confirmações:

```bash
nm
objdump
```

mostram:

```text
DebugWinMainCRTStartup
```

como entrypoint válido.

---

# Resultado do Teste

Mesmo com:

```text
DebugWinMainCRTStartup
```

forçado como entrypoint:

```text
mingw_entry_reached.txt não é criado
mingw_winmain_reached.txt não é criado
nenhuma MessageBox aparece
```

---

# Conclusão Atual

O processo aparentemente não chega a:

```text
DebugWinMainCRTStartup
WinMain
```

Portanto o bloqueio ocorre antes.

---

# Investigação de Construtores Globais

Foram listados os:

```text
_GLOBAL__sub_I_*
```

presentes no binário.

Entre eles:

```text
_GLOBAL__sub_I_AnimDataMgr
_GLOBAL__sub_I_WebDB
_GLOBAL__sub_I_ParkDXPlay
_GLOBAL__sub_I_smStage
_GLOBAL__sub_I_security
_GLOBAL__sub_I_settings
```

---

## AnimDataMgr

Definição encontrada:

```cpp
HoAnimDataMgr AnimDataMgr;
```

Arquivo:

```text
SrcGame/src/HoBaram/HoAnimData.cpp
```

Construtor:

```cpp
HoAnimDataMgr::HoAnimDataMgr()
{
    Init();
}
```

Aparentemente apenas limpa buffers.

No momento não há evidência de que seja o responsável.

---

## ParkDXPlay

Encontrado:

```cpp
CParkPlayer ParkDXPlay;
```

em:

```text
SrcGame/src/TJBOY/park/ParkDXPlayer.h
```

Tentativa de converter para:

```cpp
extern CParkPlayer ParkDXPlay;
```

causou centenas de símbolos ausentes.

Alteração revertida.

---

# Estado dos Imports

DLLs detectadas:

```text
ADVAPI32.dll
d3d9.dll
d3dx9_43.dll
DSOUND.dll
GDI32.dll
IMM32.dll
IPHLPAPI.dll
KERNEL32.dll
msvcrt.dll
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

libgcc_s_seh-1.dll
libstdc++-6.dll
```

No Windows:

```cmd
where d3dx9_43.dll
where d3d9.dll
where dsound.dll
```

retornam corretamente.

Não há evidência atual de DLL ausente.

---

# Hipótese Atual Mais Forte

O problema não parece estar dentro do WinMain.

Os testes indicam que a execução não chega ao entrypoint customizado.

As hipóteses mais prováveis são:

```text
1. TLS callbacks
2. Inicialização CRT
3. Construtor global executado antes do WinMain
4. Problema específico do port x64
```

---

# Hipótese Arquitetural (Alta Prioridade)

Cliente original:

```text
PE32
i386
32-bit
```

Build MinGW atual:

```text
PE32+
x86-64
64-bit
```

Durante a compilação existem centenas de warnings do tipo:

```cpp
(int)pointer
(DWORD)pointer
(char*) -> int
```

Exemplos:

```cpp
(DWORD)ptr
(int)lpData
```

O código claramente foi escrito assumindo:

```cpp
sizeof(int) == sizeof(pointer)
```

o que é válido apenas em:

```text
32-bit
```

Portanto existe forte possibilidade de:

```text
corrupção de ponteiros
travamento em inicialização
```

antes mesmo do WinMain.

---

# Próximos Passos

## 1. Confirmar integridade do executável no Windows

Comparar:

```bash
sha256sum dist_mingw_client/SummitTaleClient.exe
```

com:

```powershell
Get-FileHash SummitTaleClient.exe
```

---

## 2. Investigar TLS

Inspecionar:

```bash
objdump -p SummitTaleClient.exe
```

e callbacks TLS.

---

## 3. Investigar construtores globais

Principais suspeitos:

```text
ParkDXPlay
WebDB
AnimDataMgr
smStage
security
settings
```

---

## 4. Build 32-bit

Prioridade elevada.

Compilador já instalado:

```bash
i686-w64-mingw32-g++
i686-w64-mingw32-gcc
```

Objetivo:

```text
produzir PE32 i386
```

compatível com:

```text
Game.exe original
```

e verificar se o problema desaparece sem necessidade de corrigir milhares de casts de ponteiro.


<!-- BEGIN AUTO:connection-status -->
## Estado atual do port

### Concluído

- Compilação dos 186 ficheiros do servidor.
- Linkagem do servidor em MinGW i686.
- Compatibilidade ODBC.
- Conexão com SQL Server.
- Correção de `local_GetWord()`.
- Correção do `std::bad_alloc` durante o carregamento.
- Carregamento completo dos mapas.
- Inicialização completa do servidor.
- Correção do subsistema Windows GUI para Windows Console.
- Execução persistente do servidor.
- Loop de mensagens Windows validado.
- Processamento periódico de `WM_TIMER` validado.
- Porta TCP do servidor aberta e em escuta.

### Em andamento

- Conexão do cliente com o servidor.
- Validação do IP e da porta usados pelo cliente.
- Validação do protocolo inicial de conexão.
- Comparação dos opcodes entre cliente e servidor.
- Diagnóstico do handshake e do fluxo de login.

### Próximo marco

O cliente deverá estabelecer uma conexão TCP com a porta `9000` e o servidor deverá registar o socket remoto aceite e o primeiro pacote recebido.

### Critério de conclusão

- IP e porta usados pelo cliente;
- resultado da chamada `connect()`;
- chamada `accept()` no servidor;
- IP e porta remotos aceites;
- tamanho do primeiro pacote recebido;
- opcode do primeiro pacote;
- resposta inicial enviada pelo servidor.
<!-- END AUTO:connection-status -->
