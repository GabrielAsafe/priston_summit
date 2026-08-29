Sim. Revendo a source atual com foco especificamente em comportamento de segurança e privacidade, **não encontrei evidência clara de malware clássico** — nada que pareça roubar dados do navegador, instalar persistência no Windows, criptografar arquivos, baixar e executar payload escondido, keylogger ativo ou injetar código em outros processos para controle remoto.

Mas encontrei **algumas práticas muito invasivas e, em dois casos, problemas de segurança mais graves que o próprio MAC address**.

### Sobre o MAC: há uma finalidade real, mas você não precisa dela

A coleta acontece em:

`SrcGame/src/Login/Login.cpp:16-30`

```cpp
PIP_ADAPTER_INFO AdapterInfo;
...
GetAdaptersInfo(AdapterInfo, &dwBufLen);

sprintf_s(
    packet.mac,
    sizeof(packet.mac),
    "%02X-%02X-%02X-%02X-%02X-%02X",
    info->Address[0],
    info->Address[1],
    info->Address[2],
    info->Address[3],
    info->Address[4],
    info->Address[5]
);
```

E esse MAC é colocado no pacote de login:

`SrcGame/src/Login/Login.h:9-18`

```cpp
struct LoginPacket
{
    int size;
    int code;
    char account[ACCOUNT_SIZE];
    char password[PASSWORD_SIZE];
    char hash[33];
    char mac[30];
    char szPath[MAX_PATH];
    char szPCName[MAX_COMPUTERNAME_LENGTH + 1];
};
```

O cliente ainda coleta **caminho completo do Game.exe e nome do computador**:

`SrcGame/src/Login/Login.cpp:42-47`

```cpp
GetModuleFileName(NULL, packet.szPath, MAX_PATH);

DWORD size = sizeof(packet.szPCName) / sizeof(packet.szPCName[0]);
GetComputerName(packet.szPCName, &size);
```

E envia tudo ao servidor no login:

```cpp
smWsockDataServer->Send((char *)&packet, packet.size, TRUE);
```

Encontrei exatamente para que o MAC é usado no servidor:

`SrcServer/src/Server/OnSever.cpp:33412-33433`

```cpp
//Verificar se Mac ta banido
std::shared_ptr<SQLConnection> db =
    SQLConnection::GetConnection(DATABASEID_SecurityDB);

if (db->Prepare(
    "SELECT * FROM BannedMac WHERE MacAddress=?"))
{
    db->BindInputParameter(szMac, 1, PARAMTYPE_String);

    ...
    if (iHas > 0)
        return FALSE;
}
```

Então a justificativa histórica é:

```text
MAC
 ↓
identificação aproximada do computador
 ↓
BannedMac
 ↓
hardware/device ban
```

Isso é uma finalidade legítima em um MMO antigo. **Não é, por si só, evidência de malware.**

Mas eu não usaria isso hoje.

O MAC não é necessário para gameplay, autenticação ou funcionamento da rede. É apenas mecanismo de moderação/ban. Além disso, MAC é facilmente alterável/spoofável, pode mudar de adaptador, VM ou interface e não constitui uma identidade confiável da máquina.

No seu caso, rodando localmente, **não existe razão prática para coletá-lo**.

---

## O que me preocupa mais: o servidor grava sua senha em texto puro

Isto eu removeria imediatamente.

`SrcServer/src/Server/OnSever.cpp:33388-33409`

contém:

```cpp
wsprintf(
    szBuff,
    "%d:%d:%d - ID( %s ) - Pw( %s ) - Nick ( %s ) "
    "- IP( %s ) - MAC_PC( %s ) - PcName(%s ) /r/n",
    st.wHour,
    st.wMinute,
    st.wSecond,
    szID,
    szPassword,
    lpPlayInfo->getName(),
    lpPlayInfo->lpsmSock->szIPAddr,
    szMac,
    szPCName
);
```

E grava isso em:

```cpp
"LogFile//(MAC_PC)%d-%d.log"
```

Ou seja, um login produz algo equivalente a:

```text
ID: jogador123
Pw: minhasenha
Nick: Personagem
IP: 192.168.1.20
MAC: AA-BB-CC-DD-EE-FF
PcName: DESKTOP-ABC
```

**Isso não é justificável.**

Não vejo razão operacional legítima para registrar a senha do usuário em log.

Essa seria minha prioridade número 1 para remoção.

O próprio binário atual do servidor confirma que esse código chegou ao `Server.exe`; encontrei nele a string:

```text
ID( %s ) - Pw( %s ) - Nick ( %s ) - IP( %s )
- MAC_PC( %s ) - PcName(%s )
```

Portanto não é apenas código abandonado.

---

## E a senha também trafega sem proteção evidente

O `LoginPacket` contém:

```cpp
char password[PASSWORD_SIZE];
```

e `Login.cpp` faz:

```cpp
strcpy_s(
    packet.password,
    sizeof(packet.password),
    password.c_str()
);
```

Depois manda diretamente pelo socket.

Fui até `smWINSOCK::Send()` para verificar se havia criptografia transparente. O caminho atual é basicamente:

`SrcGame/src/smwsock.cpp:1644+`

```cpp
memcpy(smSendState.Buff, SendData, len);
```

e a thread termina em:

```cpp
send(SockInfo->Sock, szBuff, BuffLen, 0);
```

Não encontrei TLS nem criptografia sendo aplicada especificamente ao pacote de login nesse caminho.

Então, para um servidor remoto:

> **eu consideraria usuário/senha interceptáveis por alguém capaz de observar o tráfego.**

No localhost isso tem impacto muito menor, obviamente.

---

## Tem mais um lugar ruim envolvendo senhas

`SrcServer/src/GameMasters/GameMasters.cpp:104-115`:

```cpp
void GameMasters::print() const
{
    ...

    cout << "id: [" << users[i]->id;
    cout << "] - AccountId: [" << users[i]->accountId;
    cout << "] - Password: [" << users[i]->password;
    cout << "] - security: ["
         << (int)users[i]->security << "]" << endl;
}
```

Então um comando de CLI pode literalmente imprimir senhas de GM.

E elas são carregadas do banco em:

```cpp
db->GetData(
    gamemaster->password,
    30,
    3,
    PARAMTYPE_String
);
```

e comparadas diretamente:

```cpp
if (password.compare(users[i]->password) == 0)
```

Isso indica armazenamento/comparação de credencial em formato recuperável, provavelmente plaintext.

É código antigo, mas merece limpeza.

---

# Sobre o anti-cheat

Aqui existe código que **parece malware para um antivírus**, mas cuja finalidade é claramente anti-cheat.

Por exemplo:

`SrcGame/src/AntiCheat/AntiCheat.cpp`

tem:

```cpp
WriteProcessMemory(...)
```

Enumeração de módulos:

```cpp
CreateToolhelp32Snapshot(...)
Module32First(...)
Module32Next(...)
```

Enumeração de threads:

```cpp
Thread32First(...)
Thread32Next(...)
```

Consulta interna do Windows:

```cpp
NtQueryInformationThread(...)
```

Debugger:

```cpp
IsDebuggerPresent()
```

Enumeração de títulos de janelas:

```cpp
FindWindowA(...)
GetWindowTextA(...)
```

e até:

```cpp
FreeLibraryAndExitThread(...)
```

para tentar remover DLLs que não considera pertencentes ao jogo.

Esse conjunto de técnicas é **exatamente o tipo de coisa que heurísticas do Defender podem considerar suspeita**.

Mas a intenção observável é anti-cheat/self-protection, não roubo de informação.

### Porém: seu anti-cheat ainda está habilitado

Tem uma pegadinha aqui.

`AntiCheat.h:5-7`:

```cpp
#ifndef _DEBUG
#define ANTI_CHEAT_ENABLE
#endif
```

Seu build MinGW usa:

```text
-DNDEBUG
-D_CLIENT
```

mas não define `_DEBUG`.

Portanto:

```text
_DEBUG não existe
      ↓
ANTI_CHEAT_ENABLE é definido
      ↓
anti-cheat de release é compilado
```

E `Winmain.cpp:552-554` efetivamente faz:

```cpp
pcAntiCheat = new AntiCheat();

pcAntiCheat->AddNewThreadException(
    GetCurrentThreadId()
);

pcAntiCheat->CheckThread();
```

Além disso, o servidor manda:

```text
SEARCH_GAME_CHEAT
INIT_CHEAT_PROTECTION
```

e o cliente trata esses pacotes em:

`SrcGame/src/netplay.cpp:2340-2348`.

Então **`antidebug1()` estar desabilitado não significa que o anti-cheat inteiro esteja desabilitado**.

O Game.exe atual inclusive importa:

```text
WriteProcessMemory
CreateToolhelp32Snapshot
FreeLibraryAndExitThread
IsDebuggerPresent
NtQueryInformationThread
```

Eu confirmei isso no executável que você enviou.

---

### Algumas partes agressivas do anti-cheat parecem mortas

Há boas notícias.

Existe:

```cpp
void AntiInject()
{
    ...
    BlockAPI(
        hProc,
        "NTDLL.DLL",
        "LdrLoadDll"
    );
}
```

que tenta modificar `LdrLoadDll`.

Mas dentro da thread fizeram isto:

```cpp
void AntiInject();
```

em vez de:

```cpp
AntiInject();
```

Ou seja, ali estão apenas redeclarando a função. **Ela não é chamada.**

Também não encontrei chamada ativa para:

```cpp
FindHackDllThread()
```

E o checksum atualmente começa com:

```cpp
BOOL AntiCheat::ChecksumCheckMismatch(...)
{
    return TRUE;

    // resto ficou inalcançável
}
```

Portanto algumas das partes mais invasivas estão efetivamente neutralizadas.

---

# Existe um sistema web legado que eu também removeria no seu servidor local

Essa foi outra descoberta relevante.

`TJBOY/isaocheck/isaocheck.cpp` contém bastante comunicação HTTP legada.

Inclusive:

```cpp
sprintf(
    szWriteBuffer,
    "http://%s/login.php?UserID=%s&PassWord=%s",
    hostName,
    id,
    pass
);
```

e:

```cpp
"http://%s/InsertUser.php?UserID=%s&PassWord=%s&UserName=%s"
```

Ou seja: existem rotinas antigas capazes de colocar **usuário e senha na URL HTTP**.

Encontrei também endereços legados:

```text
211.39.144.157
211.39.144.158
74.122.192.107
```

e:

```text
http://211.39.144.158/NPCSound/
```

Essas strings também estão no **Game.exe atual**, portanto esse código foi compilado.

Porém há uma distinção importante: **não encontrei chamada atual para o `ISAOCHECK::IsaoCheck()` que transmite a senha dessa maneira**.

Então eu classificaria assim:

```text
código perigoso legado: SIM
exfiltração ativa de senha por esse caminho: NÃO CONFIRMADA
```

Parte dessa infraestrutura ainda é usada por recursos antigos de:

```text
clan
notices
clan images
SoD ranking
NPC sounds
```

Para um servidor puramente local, eu removeria ou colocaria toda essa camada atrás de uma opção desabilitada por padrão.

---

## Não encontrei sinais típicos de stealer/RAT

Procurei especificamente por coisas que mudariam completamente minha avaliação.

Não encontrei código tentando acessar:

```text
Chrome/Edge/Firefox credentials
Windows Credential Manager
cookies do navegador
DPAPI / CryptUnprotectData
clipboard
AppData de browsers
documentos pessoais
carteiras crypto
SSH keys
```

Também não encontrei:

```text
CreateRemoteThread
VirtualAllocEx
```

usados para injetar payload em outro programa.

Não encontrei instalação em:

```text
HKCU\...\Run
Startup
Scheduled Tasks
Services
```

para persistência.

E não apareceu um fluxo do tipo:

```text
download
 ↓
escrever EXE/DLL
 ↓
executar silenciosamente
```

que seria uma bandeira vermelha grande.

Há um antigo keyboard hook, mas está literalmente dentro de:

```cpp
#if 0
...
InstallKeyboardHook(...)
...
#endif
```

portanto não é compilado.

---

# Minha classificação atual

Eu colocaria assim:

| Item                            | Minha avaliação                                              |
| ------------------------------- | ------------------------------------------------------------ |
| Malware/stealer/RAT intencional | **Não encontrei evidência**                                  |
| MAC address                     | **Legado de hardware-ban; dispensável**                      |
| Nome do PC                      | **Coleta desnecessária**                                     |
| Caminho do Game.exe             | **Coleta desnecessária; nem é usado pelo server atual**      |
| Game hash                       | **Uso legítimo de integridade**                              |
| Senha em log                    | **Grave; remover**                                           |
| Senha raw no protocolo          | **Grave se for jogar pela Internet**                         |
| Senhas GM em plaintext          | **Ruim; corrigir**                                           |
| Anti-cheat                      | **Intrusivo, mas finalidade identificável**                  |
| Anti-cheat realmente desativado | **Não — ainda está compilado/ativo parcialmente**            |
| HTTP legado                     | **Risco; remover/desabilitar no setup local**                |
| IP/MAC bans                     | **Legítimos em MMO público, inúteis para seu cenário local** |

### O MAC, especificamente

Então a resposta curta é:

**Sim, eu identifiquei por que eles queriam o MAC. Era para `BannedMac`, isto é, banir a máquina e não apenas a conta/IP.**

Não parece que estejam usando o MAC para algo escondido.

Mas:

**não é necessário para o funcionamento do jogo; não é um identificador confiável; e para o seu servidor local eu removeria completamente.**

Eu removeria também `szPCName` e `szPath` do pacote. O servidor não usa `szPath` para nada, e `szPCName` só termina no log.

E eu trataria **a senha em plaintext nos logs como muito mais urgente que o MAC**.

Se você quiser, a próxima etapa que faz mais sentido é eu fazer uma **auditoria de segurança completa e gerar patches concretos** para deixar uma variante “local/privacy-clean”:

```text
NO ANTICHEAT
NO MAC
NO PC NAME
NO EXE PATH
NO EXTERNAL HTTP
NO EXTERNAL IP
NO PLAINTEXT PASSWORD LOGS
```

preservando apenas o que o jogo realmente precisa para funcionar.

