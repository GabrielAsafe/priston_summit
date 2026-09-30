# Retomada técnica — Summit Tale / port MinGW i686
Data: 2026-08-21

## Objetivo atual

Continuar o debug do cliente/servidor Priston Tale derivado, compilado em MinGW i686.

Prioridades atuais:

1. Corrigir o **offset / escala do mouse**.
2. Confirmar se a seleção e o texto da classe passam a funcionar corretamente depois do mouse.
3. Corrigir a renderização do **cursor customizado** do jogo.
4. Só depois limpar instrumentações/guards temporários e consolidar os patches.

---

# Estado geral

## Login / rede / autenticação — RESOLVIDOS

Não voltar a investigar rede/auth sem nova evidência.

Fluxo saudável já comprovado:

- Cliente conecta na porta 1997.
- QEMU encaminha corretamente para o Windows.
- `Server.exe` escuta `0.0.0.0:1997`.
- Pacote de login `0x48470085`, 412 bytes, chega ao servidor.
- Auth SQL funciona.
- `SQLLogon` retorna `-5`, que o server normaliza para sucesso.
- Cliente recebe:
  - `0x700000F9`
  - `0x484700C1`
  - `0x484700C2..C8`
  - `0x70000069` SETUSERINFO
  - `0x700000D0` SERVER_INFO
- Cliente entra na seleção de personagem.

Correções já feitas nessa área:

- Low-level receiver passou a aceitar explicitamente o opcode legado de login.
- `SERVER_TCP_PORT` compatível em 1997 sem quebrar `TCP_GAMEPORT=9000`.
- Firewall persistente removido.
- `Database.dll` tinha `Data Source=PTL\SQLEXPRESS` hardcoded.
- DLL foi patchada para usar `Data Source=%s`.
- Teste ADODB x86 comprovou acesso ao SQL Server correto.

---

# Crash de fonte DirectX — RESOLVIDO

Causa:

A definição local de `ID3DXFont` em `d3dx9core.h` tinha layout incompatível com `d3dx9_43.dll`.

Consequência:

A chamada que deveria ir para `DrawTextA` caía em `GetTextMetricsA`, causando crash.

Correções:

- Interface `ID3DXFont` ajustada para o layout June 2010.
- Chamadas legacy de DrawText convertidas para assinatura de seis argumentos com `NULL` pSprite.
- Arquivos:
  - `SrcGame/src/Drawsub.cpp`
  - `SrcGame/src/BellatraFontEffect.cpp`
  - `SrcGame/src/Hook/Hook.cpp`

Resultado:

- Build cliente 203/203.
- Texto voltou a aparecer.
- Crash antigo eliminado.

---

# Clan / HoLogin — RESOLVIDO

Crash antigo em `clanMENU_INIT(0)` foi atravessado.

Correções importantes:

- `InitAssaEffect`: `memset(cAssaEffect, 0, sizeof(cAssaEffect));`
- `CreateFile` testado contra `INVALID_HANDLE_VALUE`.
- `LoadClanMessage` com +1 para NUL e terminação segura.
- `clanText.msg` -> `ClanText.msg`.
- `CreateSaveData_dir` sem `wsprintf` com buffer sobreposto.
- `ReadMessage` refeito.
- Bug `'/n'` corrigido para `'\n'`.
- `MsgXsu_Ysu` protegido contra overflow dos buffers locais.

Runtime já comprovou:

- `clanMENU_INIT LEAVE`
- `STEP 45 AFTER clanMENU_INIT`
- `STEP 99 LEAVE HoLogin::Init SUCCESS`

Não voltar a esse crash salvo nova evidência.

---

# Crash ao entrar na criação de personagem — CAUSA ENCONTRADA E ATRAVESSADA

## Sintoma original

Selecionar qualquer tribo/classe acabava em crash.

Crash original:

- endereço `0x00411567`
- função `smCHAR::ChangeMotion(int,int)`
- leitura em `0x0000084C`

Análise do assembly provou:

- `smMotionInfo == NULL`
- `ChangeMotion(10)` apenas foi a primeira dereferência fatal.

Foi colocado temporariamente:

- `MINGW_CHANGEMOTION_NULL_GUARD_V1`

Esse guard é diagnóstico, não solução final.

## `SetLoadPattern()` falhava

Trace comprovou:

```text
ADD NEW AFTER SetLoadPattern index=0 result=0
motion=00000000
motion2=00000000
pattern=00000000
pattern2=00000000
dina1=00000000
dina2=00000000
```

Fluxo:

```text
AddJobCharacter
  -> SetLoadPattern
      -> smPattern.LoadCharactor
          -> smModelDecode
```

## Assets físicos

O source usa identificadores lógicos como:

```text
char//tmABCD//a001.ini
char//tmABCD//tmh-a01.inf
```

Mas a distribuição contém caches compilados como:

```text
A001.inx
A001t.inx
tmh-A01.inf
tmh-a01.inx
```

Não renomear `.ini` para `.inx` no HoLogin.

Outras versões do source também usam `.ini` como identificador lógico.

## `smMODELINFO`

O arquivo `A001.inx` mede:

```text
95268 bytes
```

O executável MinGW também usa:

```text
sizeof(smMODELINFO) == 0x17424 == 95268
```

Portanto NÃO há incompatibilidade de tamanho/packing dessa struct.

## Keyword do `.inx`

O `.inx` é válido.

O código interno espera checksum calculado sobre:

```text
char\tmABCD\a001.inx
```

O valor esperado/reconstruído foi:

```text
0x70143798
```

O problema real:

`smFindFile()` preservava o caminho vindo do source:

```text
char//tmABCD//a001.inx
```

e `smModelDecode()` fazia:

```cpp
GetSpeedSum(lpFile)
```

`GetSpeedSum()` é case-insensitive para ASCII, porém NÃO normaliza `/`, `//` e `\`.

Logo um cache `.inx` perfeitamente válido era rejeitado por checksum por causa do formato do path.

## Patch aplicado

Marker:

```text
MINGW_MODEL_PATH_CHECKSUM_V2
```

O patch canonicaliza **somente o caminho usado no checksum**:

- `/` -> `\`
- múltiplos separadores -> um único `\`

Exemplo esperado:

```text
input=[char//tmABCD//a001.inx]
canonical=[char\tmABCD\a001.inx]
code=0x70143798
keyword=1
```

Também foi colocado:

```text
MINGW_ADDJOB_LOAD_FAIL_GUARD_V2
```

para impedir `AddJobCharacter()` de continuar dereferenciando `lpDinaPattern` caso um load realmente falhe.

Resultado informado pelo usuário:

> o patch passou e **os personagens aparecem**.

Portanto o bloqueio principal do loader de personagem foi atravessado.

---

# Bug independente já corrigido em AddJobCharacter

Havia off-by-one depois de:

```cpp
JobCharacterCount++;
```

O source acessava o índice errado.

Foi ajustado para:

```cpp
JobCharacter[JobCharacterCount - 1]
```

Esse bug era real, mas NÃO foi a causa original do primeiro crash.

---

# Estado atual — MOUSE COM OFFSET / ESCALA

Este é o problema de maior prioridade.

Sintoma descrito:

- O offset aumenta conforme o ponteiro se desloca pela tela.
- Não é possível clicar corretamente nos personagens/botões.
- Parece pré-selecionar o personagem da direita.
- Texto/informação da classe não aparece como esperado.

Isso é compatível com erro linear de escala de coordenadas.

## Hit-test usa `pCursorPos` diretamente

Exemplo no `HoLogin::GetSelectCharacter()`:

```cpp
rect.left < pCursorPos.x &&
rect.right > pCursorPos.x &&
rect.top < pCursorPos.y &&
rect.bottom > pCursorPos.y
```

Então a correção deve ser feita centralmente na origem de `pCursorPos`, não em cada botão.

O mesmo `pCursorPos` é usado por:

- seleção de personagem;
- QuitButton;
- BackButton;
- UI geral;
- seleção de players/items;
- cursor customizado.

---

# Handler real do mouse

Arquivo:

```text
SrcGame/src/Winmain.cpp
```

Trecho atual do `WM_MOUSEMOVE`:

```cpp
case WM_MOUSEMOVE:

    if (g_IsDxProjectZoomIn)
    {
        ...
    }
    else
    {
        xPs = LOWORD(lParam);
        yPs = HIWORD(lParam);

        AutoMouse_Distance += abs(pCursorPos.x - xPs);
        AutoMouse_Distance += abs(pCursorPos.y - yPs);

        g_iFzmCursorPosX = pCursorPos.x = xPs;
        g_iFzmCursorPosY = pCursorPos.y = yPs;
    }
```

Ou seja:

**não existe qualquer transformação entre coordenada recebida pelo Windows/Wine e coordenada lógica do jogo.**

---

# Estado das dimensões da janela

Config:

`game.ini`:

```ini
[Screen]
Windowed=1
AutoAdjust=1
StartMaximized=1
Ratio=43
Resolution=1
```

`Settings.ini`:

```ini
[Screen]
Windowed=1
Resolution=1
```

Apesar disso, o trace de criação da janela mostra:

```text
windowMode=0
sx=1920
sy=1080
width=1400
height=1050
```

Existe no source:

```cpp
if(1 /* TEMP_MINGW_FORCE_WINDOWMODE */ || SETTINGSHANDLER->getWindowMode())
```

Então há uma discrepância importante entre:

- config pedindo windowed;
- log dizendo `windowMode=0`;
- código forçando o ramo de janela.

Isso pode estar relacionado ao problema.

## Criação da janela

A área desejada é baseada em:

```text
SETTINGSHANDLER width  = 1400
SETTINGSHANDLER height = 1050
```

e é passada por `AdjustWindowRectEx()`.

## `WM_SIZE` observado em runtime

Trace existente:

```text
WM_SIZE_COUNT = 2
WM_SIZE wParam=0 client=1398x1048
WM_SIZE wParam=0 client=1398x1011
```

Ou seja, o client area efetivo observado mudou para:

```text
1398 x 1011
```

enquanto vários sistemas internos foram inicializados/configurados para:

```text
1400 x 1050
```

Mouse observado:

```text
MOUSE_RANGE_X = 258 .. 1258
MOUSE_RANGE_Y = 2 .. 1010
```

Isso já mostra uma diferença principalmente vertical relevante:

```text
lógico esperado: 1050
client real:      1011
```

Mas ainda NÃO aplicar fórmula definitiva sem medir os valores simultaneamente no mesmo `WM_MOUSEMOVE`.

---

# DPI

Busca no source encontrou:

```text
DPI_HITS=1
```

mas esse único hit era apenas `GetDeviceCaps()` do Direct3D, não configuração de DPI awareness.

Não foram encontradas chamadas como:

```text
SetProcessDPIAware
SetProcessDpiAwareness
SetProcessDpiAwarenessContext
GetDpiForWindow
```

Portanto o processo aparentemente não declara DPI awareness.

Isso mantém DPI virtualization como hipótese relevante, porém ainda não comprovada como causa única.

---

# `WM_SIZE`

No `Winmain.cpp`, em `WM_SIZE`, quando window mode entra nos ramos esperados, o código sincroniza:

```cpp
WinSizeX = LOWORD(lParam);
WinSizeY = HIWORD(lParam);

smScreenWidth = WinSizeX;
smScreenHeight = WinSizeY;

SETTINGSHANDLER->setWidth(WinSizeX);
SETTINGSHANDLER->setHeight(WinSizeY);

MidX = WinSizeX / 2;
MidY = WinSizeY / 2;

g_fWinSizeRatio_X = float(WinSizeX) / 800.f;
g_fWinSizeRatio_Y = float(WinSizeY) / 600.f;

d3dpp.BackBufferWidth = WinSizeX;
d3dpp.BackBufferHeight = WinSizeY;
```

Porém os ramos dependem de:

```cpp
SETTINGSHANDLER->getWindowMode()
```

e o log mostrou `windowMode=0`.

Isso é um ponto crítico a confirmar.

---

# Próximo passo recomendado

Instrumentar SOMENTE o `WM_MOUSEMOVE` para registrar, no mesmo instante:

```text
rawX/rawY
GetClientRect width/height
WinSizeX/WinSizeY
smScreenWidth/smScreenHeight
SETTINGSHANDLER width/height
d3dpp.BackBufferWidth/Height
WindowMode / getWindowMode()
```

Registrar poucos pontos/amostras, sem gerar log gigante.

Objetivo:

descobrir qual transformação é realmente necessária.

Possíveis cenários:

## Cenário A — client físico != resolução lógica

Exemplo:

```text
raw client = 1398x1011
logical    = 1400x1050
```

Correção central provável:

```cpp
pCursorPos.x = MulDiv(rawX, logicalWidth, clientWidth);
pCursorPos.y = MulDiv(rawY, logicalHeight, clientHeight);
```

## Cenário B — `WM_SIZE` deveria ter sincronizado tudo mas não sincroniza

Se `getWindowMode()==0` impedir o ramo `WM_SIZE`, o fix pode ser no estado/configuração da janela, e não no mouse.

## Cenário C — DPI virtualization

Se `GetClientRect`, `WM_MOUSEMOVE` e backbuffer estiverem em espaços diferentes por DPI, definir DPI awareness cedo no processo pode ser a solução correta.

Não escolher A/B/C sem o trace simultâneo.

---

# Texto da classe / aparente pré-seleção

Não tratar ainda como bug separado.

`JobBodySelectNum` é definido a partir de `GetSelectCharacter()`.

Se `pCursorPos` estiver errado:

- o jogo pode detectar hover/click no personagem da direita;
- `JobBodySelectNum` pode receber índice incorreto;
- texto/descrição pode não entrar no estado esperado.

Reavaliar texto somente DEPOIS do mouse ficar correto.

---

# Cursor customizado — prioridade posterior

Arquivos lógicos usados:

```text
DefaultCursor.tga
SellCursor.tga
BuyCursor.tga
RepairCursor.tga
Attack_Cursor.tga
GetItem_Cursor1.tga
GetItem_Cursor2.tga
Talk_Cursor.tga
```

Trace já comprovou:

- materiais recebem IDs;
- `DrawCursor()` é chamado;
- `CursorClass=1`;
- coordenadas ficam dentro da tela.

Mesmo assim o cursor customizado não aparece.

Não priorizar agora.

Uma correção no mouse pode também melhorar a percepção/posição do cursor, mas não explica sozinha textura invisível.

---

# Instrumentações/markers atualmente relevantes

Manter por enquanto:

```text
MINGW_MODEL_PATH_CHECKSUM_V2
MINGW_ADDJOB_LOAD_FAIL_GUARD_V2
MINGW_CHANGEMOTION_NULL_GUARD_V1
MINGW_CHAR_CREATE_TRACE_V4
MINGW_JOB_LOAD_TRACE_V1
MINGW_ORIGINAL_CURSOR_TRACE_V1
```

Depois que criação/mouse estiverem estáveis, revisar quais guards/traces podem ser removidos.

---

# Regras de trabalho importantes

- NÃO pedir upload de source; o usuário cola saída de terminal.
- Se uma cola longa virar `.txt`, tratar como saída do terminal/source.
- Responder em português.
- Preferir patches automáticos em shell/Python.
- Preferir um único bloco de comando.
- Evitar rebuilds desnecessários.
- Cliente demora vários minutos para compilar.
- Instrumentação/log é preferível a debugger/watchpoints.
- NUNCA colocar `exit` nos snippets.
- NUNCA colocar `set -e`.
- NUNCA usar `|| false`.
- NUNCA colocar `|| ...` depois do `cd`.
- Usar exatamente:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
```

- Não usar shell que possa fechar o terminal em caso de falha.
- Evitar greps recursivos gigantes.
- Restringir diagnóstico a arquivos relevantes.
- Não incluir `Dependency/DirectX` em scans amplos salvo necessidade.
- Não repetir passwords em logs.
- Não bypassar autenticação.
- Não voltar a rede/auth/DB sem nova evidência.
- Não afirmar que binários originais eram baseline funcional — eles também não funcionavam.
- Não pedir logs de uma versão original funcional.
- Não colocar constantes mágicas para resolução sem necessidade.
- Ao patchar source, evitar match que possa acertar declaração em vez de chamada.
- Validar source antes de build.
- Se patch falhar antes da gravação, declarar claramente que nada foi alterado.
- Não usar `ssh -P`; porta usa `ssh -p 2223`.

---

# Paths principais

Source:

```text
/home/gabriel/Área de Trabalho/summit/SourceCode
```

Runtime cliente:

```text
/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente
```

Runtime servidor Fedora:

```text
/home/gabriel/Área de Trabalho/summit/Servidor
```

Build cliente:

```bash
sh cliente.sh
```

Servidor:

```text
server.sh
build_server_mingw_i686_j8.sh
link_server_mingw_i686.sh
send_to_s.sh
```

SSH Windows:

```bash
ssh -p 2223 server@127.0.0.1
```

---

# Último estado funcional confirmado

- Login funciona.
- Tela de seleção funciona.
- Clan init funciona.
- Fontes/textos DirectX funcionam.
- Loader de personagens foi corrigido o suficiente para os personagens da criação aparecerem.
- O cliente não está mais bloqueado no `SetLoadPattern()` inicial.
- Problema bloqueante atual:
  **mouse com offset/escala crescente**.
- Efeito observado:
  impossível clicar corretamente, aparente pré-seleção do personagem da direita e ausência do texto esperado.
- Próximo passo:
  **instrumentar o WM_MOUSEMOVE com GetClientRect + dimensões lógicas/backbuffer simultâneas e então corrigir centralmente a transformação do mouse.**
