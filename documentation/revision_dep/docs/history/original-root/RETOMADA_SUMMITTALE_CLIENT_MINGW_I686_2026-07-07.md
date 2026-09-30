# SummitTale — Retomada Cliente MinGW i686 + Diagnóstico Atual

Data: 2026-07-07  
Ambiente: Fedora Linux + MinGW-w64 i686  
Projeto: SummitTale / SourceCode Summit Tale

---

## 1. Objetivo atual

Continuar a migração do cliente SummitTale para compilar no Linux com MinGW-w64 e gerar um executável Windows funcional.

Foco atual:

1. Manter o cliente **32-bit / i686 / PE32** funcionando.
2. Corrigir o estado do `Winmain.cpp` após o último patch ruim.
3. Investigar por que a janela do cliente some, mas o processo continua vivo.
4. Depois voltar ao problema de login.
5. Depois iniciar/continuar o build do servidor.

---

## 2. Estado confirmado

O build **x64** gerava executável, mas o processo aparentemente não chegava corretamente ao entrypoint/WinMain.

O build **i686 / 32-bit** resolveu isso.

Executável gerado com sucesso:

```text
dist_mingw_client_i686/SummitTaleClient.exe
```

Formato confirmado:

```text
PE32 executable for MS Windows GUI, Intel i386
```

Hash de um build anterior válido:

```text
ee7486e704906b4c249584ff7c6e5fc39ae7e40c394622b5bc5c861c412dc0f3
```

Depois de patches posteriores, o hash mudou. Revalidar sempre após rebuild.

---

## 3. Build i686

Diretório base:

```bash
cd "/home/gabriel/Área de Trabalho/SummitTale/SourceCode Summit Tale"
```

Compilar:

```bash
rm -rf build_mingw_client_i686 dist_mingw_client_i686

bash ./build_client_mingw_i686.sh 2>&1 | tee build_client_i686.log
```

Linkar:

```bash
bash ./link_client_mingw_i686.sh 2>&1 | tee link_client_i686.log
```

Ver erros:

```bash
grep -Ei "error:|undefined reference|multiple definition|cannot find|ld returned|collect2" \
build_client_i686.log link_client_i686.log | tail -200
```

Ver executável:

```bash
file dist_mingw_client_i686/SummitTaleClient.exe
```

---

## 4. Correção importante no link i686

O script `link_client_mingw_i686.sh` inicialmente apontava para DirectX x64:

```bash
-LDependency/DirectX9/Lib/x64
```

Isso estava errado para i686.

Foi corrigido para:

```bash
-L"Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Lib/x86"
```

Também foi necessário forçar:

```bash
-Wl,--undefined=_D3DXCreateFontIndirectA@12
```

A linha ficou semelhante a:

```bash
-ld3dx9 -Wl,--undefined=_D3DXCreateFontIndirectA@12
```

Com isso a linkagem i686 funcionou.

---

## 5. DLLs necessárias para testar no Windows

O `.exe` i686 precisa de DLLs 32-bit.

Copiar para `dist_mingw_client_i686`:

```bash
cp -v /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_dw2-1.dll dist_mingw_client_i686/
cp -v /usr/i686-w64-mingw32/sys-root/mingw/bin/libwinpthread-1.dll dist_mingw_client_i686/
```

Extrair `d3dx9_43.dll` 32-bit:

```bash
cd dist_mingw_client_i686

cabextract -F d3dx9_43.dll \
../Dependency/DirectXSDK/Microsoft\ DirectX\ SDK\ \(June\ 2010\)/Redist/Jun2010_d3dx9_43_x86.cab
```

Atenção:

```text
Não usar D3dx9d_43.dll.
Não usar libstdc++-6.dll x64.
```

Se o cliente pedir `libstdc++-6.dll`, instalar/copiar a versão i686 correta.

---

## 6. Enviar arquivos para a VM Windows

A pasta correta do cliente na VM é:

```text
C:\Users\server\Desktop\cli
```

Não enviar para:

```text
C:\Users\server
```

Comando correto:

```bash
cd "/home/gabriel/Área de Trabalho/SummitTale/SourceCode Summit Tale/dist_mingw_client_i686"

scp -P 2223 \
  SummitTaleClient.exe \
  libgcc_s_dw2-1.dll \
  libwinpthread-1.dll \
  d3dx9_43.dll \
  server@127.0.0.1:"C:/Users/server/Desktop/cli/"
```

---

## 7. Problema 0xc000007b resolvido

O erro:

```text
unable to start correctly 0xc000007b
```

foi causado por mistura de DLL 32-bit/64-bit ou DLL errada.

Correção:

1. Usar `libgcc_s_dw2-1.dll` i686.
2. Usar `libwinpthread-1.dll` i686.
3. Extrair `d3dx9_43.dll` do CAB x86.
4. Não usar `D3dx9d_43.dll`.
5. Não usar `libstdc++-6.dll` x64.

---

## 8. Direct3D: erro encontrado e correção temporária

Inicialmente o cliente chegava em:

```text
ENTROU NO WINMAIN
AFTER ANTICHEAT
AFTER READCONFIG
AFTER DIRECTSOUND
AFTER REGISTERCLASS
```

Depois falhava com:

```text
D3DERR_DEVICELOST
```

Foi instrumentado `SrcGame/src/smLib3d/smDsx.cpp`.

HRESULT real encontrado:

```text
HRESULT = 0x8876086C
```

Parâmetros problemáticos:

```text
BackBuffer=1400x1050
Windowed=0
BackBufferFormat=0
DepthStencil=1
DepthStencilFormat=80
PresentationInterval=-2147483648
BehaviorFlags=0x00000052
```

Conclusão:

```text
O cliente estava tentando criar Direct3D em fullscreen com BackBufferFormat inválido/UNKNOWN.
```

Patch temporário aplicado antes de `CreateDevice` em `smDsx.cpp`:

```cpp
// TEMP_MINGW_I686_D3D_FIX:
// Fullscreen com BackBufferFormat=D3DFMT_UNKNOWN gera D3DERR_INVALIDCALL.
// Forçar windowed para validar runtime do cliente.
d3dpp.Windowed = TRUE;
d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
```

Resultado:

```text
O erro Direct3D deixou de aparecer.
```

---

## 9. Console/cmd que abria no início

Foi encontrada esta linha em `Winmain.cpp`:

```cpp
auto ipexts = system("powershell (iwr ipinfo.io/ip -UseBasicParsing).content > image/system.txt");
```

Ela abria uma janela de cmd/powershell.

Foi substituída por:

```cpp
/* TEMP_MINGW_DISABLE_EXTERNAL_IP_CHECK */ int ipexts = 0;
```

Resultado:

```text
O cmd deixou de abrir.
```

Essa alteração não deveria quebrar o `WinMain`.

---

## 10. Comportamento atual no Windows

Após remover o cmd e corrigir D3D:

```text
O jogo não mostra mais erro Direct3D.
O cmd não abre mais.
A janela/tela do jogo aparece ou tenta aparecer.
Depois a tela some.
O processo continua vivo.
```

Foi confirmado por:

```cmd
tasklist | findstr /I "SummitTaleClient Game 1Game"
```

Foram vistas duas instâncias em alguns testes:

```text
PID 1932  ParentProcessId 4888 = explorer.exe
PID 5620  ParentProcessId 8804 = cmd.exe
```

Conclusão provisória:

```text
O cliente não está necessariamente crashando.
A janela some ou não fica como MainWindowHandle, mas o processo continua vivo.
```

---

## 11. MainWindowHandle

Comando usado:

```cmd
powershell -NoProfile -Command "$p=Get-Process SummitTaleClient; $p | Format-List Id,ProcessName,MainWindowTitle,MainWindowHandle,Responding,StartTime,Path"
```

Resultado observado:

```text
MainWindowTitle  :
MainWindowHandle : 0
Responding       : True
```

Significado:

```text
Processo vivo, mas sem janela principal registrada.
```

Hipóteses:

1. Janela principal é criada e destruída.
2. Janela é escondida.
3. Janela não chega a ser registrada como principal.
4. Processo secundário fica vivo após a janela do processo inicial fechar.
5. Algum fluxo de launcher/verificação está interferindo.

---

## 12. Cuidado: último patch quebrou o Winmain.cpp

Foi tentado um patch automático grande para logar lifecycle da janela.

Problema visto:

```text
marker CreateWindowEx !hwnd não encontrado
```

Depois disso o build passou de:

```text
202 objetos
```

para:

```text
201 objetos
```

E a linkagem falhou com símbolos indefinidos:

```text
undefined reference to WinSizeX
undefined reference to hwnd
undefined reference to MouseButton
undefined reference to pCursorPos
...
```

Esses símbolos são definidos em `Winmain.cpp`.

Depois foi confirmado:

```bash
find build_mingw_client_i686 -iname "*Winmain*.o"
```

não retornou nada.

Também:

```bash
grep -n "int WINAPI WinMain" SrcGame/src/Winmain.cpp
```

não encontrou a assinatura esperada.

Conclusão:

```text
O último patch automático danificou ou alterou o Winmain.cpp de forma que ele deixou de compilar/gerar objeto.
```

---

## 13. Próximo passo obrigatório

Restaurar `Winmain.cpp` para um backup bom:

```bash
cd "/home/gabriel/Área de Trabalho/SummitTale/SourceCode Summit Tale"

cp SrcGame/src/Winmain.cpp.bak_log_quit_reason SrcGame/src/Winmain.cpp
```

Confirmar:

```bash
grep -n "WinMain" SrcGame/src/Winmain.cpp | head -20
grep -n "TEMP_MINGW_DISABLE_EXTERNAL_IP_CHECK\|system("powershell" SrcGame/src/Winmain.cpp | head -20
```

Rebuild:

```bash
rm -rf build_mingw_client_i686 dist_mingw_client_i686

bash ./build_client_mingw_i686.sh 2>&1 | tee build_client_i686.log

find build_mingw_client_i686 -iname "*Winmain*.o"

bash ./link_client_mingw_i686.sh 2>&1 | tee link_client_i686.log

grep -Ei "error:|undefined reference|multiple definition|cannot find|ld returned|collect2" \
build_client_i686.log link_client_i686.log | tail -120
```

Critério de sucesso:

```text
build_mingw_client_i686/SrcGame_src_Winmain.o
LINK OK
```

---

## 14. Depois que Winmain.cpp voltar a compilar

Não usar patch automático grande.

Instrumentar manualmente e em pequenos blocos.

Pontos recomendados:

### 14.1 Após CreateWindowEx

Ver trecho real:

```bash
sed -n '610,650p' SrcGame/src/Winmain.cpp
```

Adicionar log manual depois da chamada e antes do retorno se `hwnd == NULL`.

### 14.2 ShowWindow real

A linha real encontrada foi:

```cpp
ShowWindow(hwnd, SETTINGSHANDLER->getStartMaximized() ? SW_MAXIMIZE : nCmdShow);
```

Não era:

```cpp
ShowWindow(hwnd, SW_SHOW);
```

Por isso o patch automático não funcionou.

### 14.3 WM_CLOSE e WM_DESTROY

Trechos reais:

```text
Winmain.cpp:1108 case WM_CLOSE:
Winmain.cpp:1889 case WM_DESTROY:
```

Instrumentar manualmente.

### 14.4 Loop principal

O loop sai quando:

```cpp
if (quit != 0 && !dwTradeMaskTime)
```

Logar:

```text
quit
GameMode
smWsockServer
QuitSave
dwTradeMaskTime
dwPlayTime
```

No teste anterior não apareceu `mingw_quit_reason.txt`, então talvez o loop não esteja saindo por esse caminho.

---

## 15. Comandos de diagnóstico no Windows

Na pasta correta:

```cmd
cd /d C:\Users\server\Desktop\cli
```

Matar instâncias antigas:

```cmd
taskkill /F /IM SummitTaleClient.exe 2>NUL
```

Rodar:

```cmd
SummitTaleClient.exe
```

Ver processos:

```cmd
tasklist /V /FI "IMAGENAME eq SummitTaleClient.exe"
```

Ver árvore:

```cmd
wmic process where "name='SummitTaleClient.exe'" get ProcessId,ParentProcessId,ExecutablePath,CommandLine
```

Ver Explorer/CMD/cliente:

```cmd
wmic process get ProcessId,ParentProcessId,Name | findstr /I "explorer cmd powershell Summit"
```

Ver janela:

```cmd
powershell -NoProfile -Command "$p=Get-Process SummitTaleClient; $p | Format-List Id,ProcessName,MainWindowTitle,MainWindowHandle,Responding,StartTime,Path"
```

Ver logs:

```cmd
dir mingw_*.txt
type mingw_window_lifecycle.txt
type mingw_quit_reason.txt
```

---

## 16. Estado do login descoberto anteriormente

O problema de login é separado da migração gráfica.

Fluxo descoberto:

```text
Cliente
  -> Server.exe
     -> rsLogAccount()
        -> LogAccountThreadProc()
           -> bSql_LogOn()
              -> BillSql.SQLLogon()
                 -> Database.dll
                    -> AccountDB
```

O servidor recebe login/senha corretamente:

```text
ID( admin ) - Pw( 1234 )
```

O ponto crítico é o retorno de:

```cpp
SQLLogon()
```

Esse valor vira:

```cpp
cbCnt
```

E é enviado ao cliente em:

```cpp
OPCODE_FAILCONNECT
WParam = cbCnt
EParam = 2
```

Como o servidor ainda não compila, a estratégia futura é instrumentar o cliente para logar `WParam` quando receber `OPCODE_FAILCONNECT`.

---

## 17. Próxima sequência recomendada

1. Restaurar `Winmain.cpp` do backup bom.
2. Confirmar que `Winmain.cpp` gera `.o`.
3. Confirmar que o cliente volta a linkar com 202 objetos.
4. Reenviar para `C:\Users\server\Desktop\cli`.
5. Confirmar que o processo continua vivo e que a janela some.
6. Instrumentar manualmente:
   - `CreateWindowEx`
   - `ShowWindow(hwnd, SETTINGSHANDLER->getStartMaximized() ? SW_MAXIMIZE : nCmdShow)`
   - `WM_CLOSE`
   - `WM_DESTROY`
   - `quit`
7. Corrigir o motivo da janela sumir.
8. Voltar ao login.
9. Iniciar/continuar servidor.

---

## 18. Regras de trabalho

- Não aplicar patch grande automático em `Winmain.cpp`.
- Sempre salvar backup antes.
- Após mexer no `Winmain.cpp`, confirmar:
  ```bash
  grep -n "WinMain" SrcGame/src/Winmain.cpp | head
  find build_mingw_client_i686 -iname "*Winmain*.o"
  ```
- Se o número de objetos cair de 202 para 201, parar imediatamente.
- Não usar `libstdc++-6.dll` x64.
- Enviar sempre para:
  ```text
  C:\Users\server\Desktop\cli
  ```
- Testar sempre com instâncias antigas mortas:
  ```cmd
  taskkill /F /IM SummitTaleClient.exe 2>NUL
  ```
