# Summit Tale — Investigação do problema de login / `cbCnt = 0`

## Contexto

Estamos investigando um problema em que o cliente consegue enviar o login ao servidor, mas o login falha/kicka logo depois. O servidor ainda **não compila**; até agora a compilação/conversão foi feita apenas no cliente. Portanto, qualquer solução que dependa de alterar e recompilar `Server.exe` ainda não é aplicável no momento.

A investigação foi feita usando:

* código-fonte disponível do servidor;
* logs do servidor Windows;
* análise de strings/exportações das DLLs;
* queries diretas no SQL Server;
* comparação de DLLs;
* validação de conexão SQL via PowerShell/ADO.

---

## 1. Fluxo de login encontrado no código

No trecho de `OnSever.cpp`, a função de entrada do login chama:

```cpp
PushLogAccount(lpPlayInfo, szID, szPassword, IP, MAC)
```

A validação real não acontece diretamente em `rsLogAccount()`. O login é processado depois na thread:

```cpp
LogAccountThreadProc
```

Dentro dessa thread, encontramos:

```cpp
int val = 0;
int cbCnt;
```

Depois, no fluxo com DLL SQL habilitada:

```cpp
val = TRUE;

val = bSql_LogOn(
    szTempID,
    LogAccount->szPassword,
    szIP,
    szGPCode,
    (char *)&DisuseDay,
    (char *)&UsePeriod,
    szGuid,
    (char *)&Bl_RNo,
    (char *)&MeterCode
);
```

Depois:

```cpp
cbCnt = val;
```

Se `cbCnt != TRUE`, o servidor envia falha ao cliente:

```cpp
smTransCommand.code = OpCode::OPCODE_FAILCONNECT;
smTransCommand.WParam = (DWORD)cbCnt;
smTransCommand.EParam = 2;
```

Conclusão:

```text
O kick/falha vem porque cbCnt recebe o retorno de bSql_LogOn().
```

Logo, o ponto crítico é:

```cpp
bSql_LogOn()
```

---

## 2. `bSql_LogOn()` não valida login diretamente

Em `SourceCode/SrcServer/src/Server/DllServer.cpp`, encontramos:

```cpp
int bSql_LogOn(
    char *lpszPID,
    char *lpszPWD,
    char *lpszIP,
    char *lpszGPCode,
    char *lpszDisuseDay,
    char *lpszUsePeriod,
    char *szGuid,
    char *lpszPCRNo,
    char *lpszGubunCode
)
{
    int result = 0;

    EnterCriticalSection(&cSqlSection);

    result = BillSql.SQLLogon(
        lpszPID,
        lpszPWD,
        lpszIP,
        lpszGPCode,
        lpszDisuseDay,
        lpszUsePeriod,
        szGuid,
        lpszPCRNo,
        lpszGubunCode
    );

    LeaveCriticalSection(&cSqlSection);

    return result;
}
```

Ou seja:

```text
OnSever.cpp
  -> bSql_LogOn()
     -> BillSql.SQLLogon()
        -> Database.dll
```

A lógica real de autenticação está em:

```text
Database.dll
```

e não no código-fonte C++ disponível do servidor.

---

## 3. DLL usada no login

No `DllServer.cpp`, a DLL é carregada assim:

```cpp
hLogSqlDLL = LoadLibrary("Database");
```

Depois:

```cpp
SQLLogon = (LPFN_SQLLogon)GetProcAddress(hLogSqlDLL, "SQLLogon");
```

Também são carregadas funções como:

```cpp
SQLLogout
SetGameUser
SetAllGameUser
SetGameUserGPCode
SetPersonalMember
```

Durante a inicialização:

```cpp
if (BillSql.SetAllGameUser)
    BillSql.SetAllGameUser();
```

Isso indicou que a DLL pode inicializar/cachear dados das tabelas de contas durante o boot do servidor.

---

## 4. Comparação das DLLs

Foram comparadas as DLLs da pasta:

```text
/home/gabriel/Área de Trabalho/summit/cliente/dlls
```

com as DLLs da pasta do servidor:

```text
/home/gabriel/Área de Trabalho/summit/Servidor
```

Resultado dos hashes:

```text
cliente/dlls/Database.dll == Servidor/Database.dll
cliente/dlls/SQL.dll      == Servidor/sql.dll
```

Ambas tinham o mesmo SHA256:

```text
5d3cd36dc7d2ba5402b36ea01f2fadd0a4c840b09528c9c913debab213f1f0dc
```

Apenas `Clan.dll` era diferente:

```text
cliente/dlls/Clan.dll != Servidor/clan.dll
```

Conclusão:

```text
Copiar Database.dll ou SQL.dll do cliente para o servidor não deve resolver,
pois já são iguais.
```

A `Clan.dll` diferente pode afetar clan/tickets, mas não parece ser a causa principal da falha de senha/login.

---

## 5. Strings da `Database.dll`

Usamos `strings` e `objdump` na `Database.dll`.

A DLL exporta:

```text
SQLLogon
SQLLogout
SetGameUser
SetGameUser2
SetGameUserGPCode
SetGameUserGPCodeLog
SetGameUserMX
```

As strings revelaram que a DLL monta dinamicamente nomes de tabelas por letra:

```text
[9GameUser]
%cGameUser
[0GameUser]

[9PersonalMember]
%cPersonalMember
[0PersonalMember]
```

A query principal de login encontrada na DLL é:

```sql
SELECT userid,
       Passwd,
       GameCode,
       GPCode,
       RegistDay,
       DisuseDay,
       UsePeriod,
       Credit,
       SelectChk,
       EventChk,
       BlockChk,
       inuse,
       DelChk,
       ServerName,
       EditDay,
       RNo,
       SNo,
       Channel,
       BNum
FROM %s
WHERE userid = '%s'
```

Conclusão:

```text
A DLL não usa somente AllGameUser / ALLPersonalMember.
Ela usa tabelas por letra, como AGameUser / APersonalMember,
BGameUser / BPersonalMember, etc.
```

Para o usuário:

```text
admin
```

a DLL deve consultar:

```text
AccountDB.dbo.AGameUser
AccountDB.dbo.APersonalMember
```

---

## 6. Estado das tabelas do usuário `admin`

Foram consultadas:

```sql
SELECT *
FROM AccountDB.dbo.AGameUser
WHERE userid = 'admin';

SELECT *
FROM AccountDB.dbo.APersonalMember
WHERE userid = 'admin';
```

Resultado relevante:

```text
AGameUser:
userid    = admin
Passwd    = 1234
GPCode    = PTP-RUD001
DisuseDay = 2030-12-31
Credit    = 0
SelectChk = 0
EventChk  = 0
BlockChk  = 0
inuse     = 0
DelChk    = 0

APersonalMember:
Userid = admin
Passwd = 1234
```

Também foi verificado:

```sql
SELECT COUNT(*) FROM AccountDB.dbo.AGameUser;
SELECT COUNT(*) FROM AccountDB.dbo.APersonalMember;
```

Ambas retornaram:

```text
2
```

Conclusão:

```text
As tabelas por letra existem, não estão vazias,
e o usuário admin existe com senha 1234.
```

---

## 7. Ajuste feito em `GameCode`

A conta `admin` tinha inicialmente:

```text
GameCode =
```

ou seja, vazio.

Como a DLL lê `GameCode` na query de login, testamos preencher:

```sql
UPDATE dbo.AGameUser
SET GameCode = 'PT'
WHERE userid = 'admin';
```

Depois a conta ficou:

```text
userid    = admin
Passwd    = 1234
GameCode  = PT
GPCode    = PTP-RUD001
BlockChk  = 0
DelChk    = 0
inuse     = 0
```

O servidor foi reiniciado depois disso para garantir que qualquer cache de `SetAllGameUser()` fosse recarregado.

Estado após ajuste:

```text
GameCode=PT
Passwd=1234
GPCode=PTP-RUD001
BlockChk=0
DelChk=0
inuse=0
```

Ainda não foi confirmado que isso resolveu o login.

---

## 8. Configuração SQL do servidor Windows

Na pasta do servidor Windows:

```text
C:\SummitTale\Servidor
```

existem arquivos:

```text
sql.reg
sql.reg.bak_docker_sql
SQL.dll Reg.reg
clan.reg
```

O `sql.reg` ativo aponta para:

```text
AccountDbIP   = 10.0.2.2,1433
AccountDbID   = sa
AccountDbPwd  = SummitTale@12345
AccountDbName = AccountDB
```

Também foram verificadas as chaves reais no registro:

```cmd
reg query HKLM /f AccountDb /s
```

Resultado:

```text
HKEY_LOCAL_MACHINE\SOFTWARE\PristonTale\GameServer
    AccountDbIP    REG_SZ    10.0.2.2,1433
    AccountDbID    REG_SZ    sa
    AccountDbPwd   REG_SZ    SummitTale@12345
    AccountDbName  REG_SZ    AccountDB

HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\PristonTale\GameServer
    AccountDbIP    REG_SZ    10.0.2.2,1433
    AccountDbID    REG_SZ    sa
    AccountDbPwd   REG_SZ    SummitTale@12345
    AccountDbName  REG_SZ    AccountDB
```

Conclusão:

```text
O servidor está configurado para acessar o SQL Docker pelo host 10.0.2.2:1433.
```

---

## 9. Configuração PHP comparada

O PHP atual usa:

```php
$host = 'summittale-mssql';
$user = 'sa';
$pass = 'SummitTale@12345';

$connection_string = 'SummitSQL';
```

Teste PHP:

```php
$conn = odbc_connect($connection_string, $user, $pass);
```

retorna conexão OK.

Foi levantada a hipótese de a senha do SQL do servidor Windows estar diferente da senha usada no PHP. Isso foi testado diretamente pelo Windows.

---

## 10. Teste direto de conexão SQL pelo Windows

Como `sqlcmd` não está instalado no Windows:

```text
'sqlcmd' is not recognized
```

foi usado PowerShell com ADO:

```cmd
powershell -NoProfile -Command "$cs='Provider=SQLOLEDB;Data Source=10.0.2.2,1433;Initial Catalog=AccountDB;User ID=sa;Password=SummitTale@12345;'; $c=New-Object -ComObject ADODB.Connection; $c.Open($cs); $rs=$c.Execute(\"SELECT COUNT(*) FROM dbo.AGameUser WHERE userid='admin'\"); $rs.Fields.Item(0).Value; $c.Close()"
```

Resultado:

```text
1
```

Conclusão:

```text
Windows -> SQL Docker -> AccountDB funciona.
Senha SummitTale@12345 está correta.
A Database.dll deve conseguir conectar ao mesmo SQL se usar os mesmos valores do registro.
```

---

## 11. Porta do servidor

Após reiniciar o `Server.exe`, foi verificado:

```cmd
netstat -ano | findstr LISTENING
```

Resultado relevante:

```text
TCP 0.0.0.0:1997 0.0.0.0:0 LISTENING 2960
```

E:

```cmd
tasklist /FI "PID eq 2960"
```

Resultado:

```text
Server.exe 2960 Services 0 997,580 K
```

Conclusão:

```text
O Server.exe está rodando e ouvindo na porta 1997.
```

---

## 12. Logs de login

Arquivos recentes:

```text
LogFile\7-6.log
LogFile\(MAC_PC)7-6.log
```

O log geral `7-6.log` contém apenas inicializações:

```text
18:4:40 >**** Starting Service **** - ( 0 / 0 )
10:17:25 >**** Starting Service **** - ( 0 / 0 )
10:20:44 >**** Starting Service **** - ( 0 / 0 )
20:7:37 >**** Starting Service **** - ( 0 / 0 )
13:6:57 >**** Starting Service **** - ( 0 / 0 )
13:10:52 >**** Starting Service **** - ( 0 / 0 )
```

O log `(MAC_PC)7-6.log` mostra tentativas reais de login:

```text
13:7:50 - ID( admin ) - Pw( 1234 ) - Nick (  ) - IP( 192.168.1.93 ) - MAC_PC( 98-29-A6-E4-68-1B ) - PcName(DESKTOP-1ORHEPL )
13:8:16 - ID( admin ) - Pw( gm ) - Nick (  ) - IP( 192.168.1.93 ) - MAC_PC( 98-29-A6-E4-68-1B ) - PcName(DESKTOP-1ORHEPL )
```

Conclusão:

```text
O cliente envia o login.
O servidor recebe exatamente ID=admin e PW=1234.
A falha ocorre depois do recebimento do pacote de login.
```

Isso elimina:

```text
- cliente apontando para IP errado;
- porta inacessível;
- senha chegando truncada;
- usuário chegando errado;
- falha antes de rsLogAccount().
```

---

## 13. PriTaTor_Errors.log

Foi analisado o final de:

```text
PriTaTor_Errors.log
```

Apenas aparecem avisos antigos de agosto de 2025 sobre arquivos `.zhoon` ausentes, por exemplo:

```text
No .zhoon file found!
Itemfile: QT118.txt
```

Não há erro recente relacionado a:

```text
SQL
SQLLogon
AccountDB
admin
login
password
FAILCONNECT
```

Conclusão:

```text
O log atual não mostra a causa da falha de autenticação.
```

---

## 14. Hipóteses eliminadas

Até agora foram eliminadas ou ficaram muito improváveis:

### 14.1 DLL errada

`Database.dll` e `SQL.dll` são iguais entre cliente/dlls e Servidor.

### 14.2 SQL inacessível do Windows

ADO/PowerShell conecta ao SQL e consulta `AGameUser`.

### 14.3 Senha SQL errada

A senha `SummitTale@12345` funciona do Windows.

### 14.4 AccountDB errada

A consulta pelo Windows acessa `AccountDB` e encontra `admin`.

### 14.5 Usuário inexistente

`admin` existe em `AGameUser` e `APersonalMember`.

### 14.6 Senha da conta errada

A base tem:

```text
Passwd=1234
```

e o log mostra o cliente enviando:

```text
Pw( 1234 )
```

### 14.7 Bloqueio simples por campo

Campos principais estão limpos:

```text
BlockChk=0
DelChk=0
inuse=0
```

### 14.8 Porta do servidor fechada

A porta 1997 está em LISTENING pelo `Server.exe`.

---

## 15. Hipóteses ainda abertas

### 15.1 `SQLLogon()` retorna um código diferente de TRUE

Ainda não sabemos o valor real retornado por:

```cpp
BillSql.SQLLogon(...)
```

Esse valor vira:

```cpp
cbCnt
```

e é enviado ao cliente no `OPCODE_FAILCONNECT`.

Sem recompilar o servidor, ainda não conseguimos logar diretamente:

```cpp
result
```

### 15.2 Alguma regra interna da `Database.dll`

A DLL pode rejeitar por campos como:

```text
GameCode
GPCode
Credit
SelectChk
EventChk
UsePeriod
DisuseDay
Channel
BNum
RNo
SNo
```

O `GameCode` vazio já foi corrigido para `PT`, mas ainda não sabemos se era a causa.

### 15.3 Tabelas adicionais

A `Database.dll` também possui strings relacionadas a:

```text
BillingDB
RegIPAddress
IPList
UserDetail
PCRoom
PCRoomLog
BillingLogDB
```

Mesmo com `PCCheck=0`, é possível que alguma validação adicional ainda ocorra.

### 15.4 Falha depois de autenticar

É possível que `SQLLogon()` até retorne sucesso, mas o cliente seja desconectado depois por outro motivo. Porém, pelo fluxo analisado, o ponto mais provável continua sendo `cbCnt != TRUE`.

---

## 16. Próximo passo ideal

Como o servidor ainda não compila, não podemos simplesmente alterar `DllServer.cpp` para logar:

```cpp
result = BillSql.SQLLogon(...)
```

O plano correto depois que o servidor compilar será adicionar log temporário em `bSql_LogOn()`:

```cpp
FILE *fp = fopen("LogFile\\SQLLogon_debug.log", "a+");
if (fp)
{
    fprintf(fp,
        "ID=%s PWD=%s IP=%s result=%d GPCode=%s DisuseDay=%d UsePeriod=%d PCRNo=%d Gubun=%d\n",
        lpszPID,
        lpszPWD,
        lpszIP,
        result,
        lpszGPCode,
        *(int*)lpszDisuseDay,
        *(int*)lpszUsePeriod,
        *(int*)lpszPCRNo,
        *(int*)lpszGubunCode
    );
    fclose(fp);
}
```

Arquivo esperado:

```text
C:\SummitTale\Servidor\LogFile\SQLLogon_debug.log
```

Isso revelaria se `SQLLogon()` retorna:

```text
0
1
-1
-2
-3
-4
-5
```

ou algum outro valor.

---

## 17. Alternativas sem recompilar servidor

Como o servidor ainda não compila, as próximas alternativas são:

### 17.1 Capturar o pacote de falha no cliente

O servidor envia:

```cpp
OPCODE_FAILCONNECT
WParam = cbCnt
EParam = 2
```

Se o cliente já compila, podemos instrumentar o cliente para logar o `WParam` recebido quando chega `OPCODE_FAILCONNECT`.

Isso pode revelar o `cbCnt` sem recompilar o servidor.

### 17.2 Procurar no cliente onde trata `OPCODE_FAILCONNECT`

Buscar no código do cliente:

```bash
grep -Rni "OPCODE_FAILCONNECT\|FAILCONNECT\|EParam\|WParam" SourceCode/SrcGame SourceCode/Shared
```

Depois adicionar log no cliente para imprimir:

```text
WParam
LParam
EParam
```

quando receber `OPCODE_FAILCONNECT`.

### 17.3 Testar outra conta real antiga

Como há logs antigos com contas como:

```text
brener
b10santos
denny
exsouls
```

pode ser útil verificar se alguma dessas contas antigas existe na base e copiar o padrão dos campos dela para `admin`.

Exemplo de query:

```sql
SELECT userid, Passwd, GameCode, GPCode, RegistDay, DisuseDay, UsePeriod,
       Credit, SelectChk, EventChk, BlockChk, inuse, DelChk, ServerName,
       EditDay, RNo, SNo, Channel, BNum
FROM AccountDB.dbo.BGameUser
WHERE userid IN ('brener', 'b10santos');
```

Comparar com:

```sql
SELECT userid, Passwd, GameCode, GPCode, RegistDay, DisuseDay, UsePeriod,
       Credit, SelectChk, EventChk, BlockChk, inuse, DelChk, ServerName,
       EditDay, RNo, SNo, Channel, BNum
FROM AccountDB.dbo.AGameUser
WHERE userid = 'admin';
```

### 17.4 Verificar se `SetAllGameUser()` atualiza/cacheia algo

Como `SetAllGameUser()` roda na inicialização da DLL, pode existir algum estado derivado/cacheado. Sempre que mexer em conta na DB, reiniciar o servidor antes de testar login.

---

## 18. Estado atual resumido

O estado atual mais provável é:

```text
O login chega corretamente ao servidor.
O servidor chama SQLLogon() dentro de Database.dll.
A Database.dll consulta AGameUser/APersonalMember.
A conta admin existe e a senha bate.
Mesmo assim, a autenticação falha ou algo após a autenticação derruba o cliente.
Ainda falta descobrir o valor real de cbCnt/WParam.
```

O próximo passo mais eficiente, considerando que o servidor ainda não compila, é:

```text
Instrumentar o cliente para logar WParam do OPCODE_FAILCONNECT.
```

Isso deve revelar o retorno de `SQLLogon()` sem precisar recompilar `Server.exe`.

---

## 19. Comandos úteis já usados

### Verificar porta e processo

```cmd
netstat -ano | findstr LISTENING
tasklist /FI "PID eq 2960"
```

### Testar SQL do Windows

```cmd
powershell -NoProfile -Command "$cs='Provider=SQLOLEDB;Data Source=10.0.2.2,1433;Initial Catalog=AccountDB;User ID=sa;Password=SummitTale@12345;'; $c=New-Object -ComObject ADODB.Connection; $c.Open($cs); $rs=$c.Execute(\"SELECT COUNT(*) FROM dbo.AGameUser WHERE userid='admin'\"); $rs.Fields.Item(0).Value; $c.Close()"
```

### Ver campos do admin

```cmd
powershell -NoProfile -Command "$cs='Provider=SQLOLEDB;Data Source=10.0.2.2,1433;Initial Catalog=AccountDB;User ID=sa;Password=SummitTale@12345;'; $c=New-Object -ComObject ADODB.Connection; $c.Open($cs); $q=\"SELECT userid, Passwd, GameCode, GPCode, RegistDay, DisuseDay, UsePeriod, Credit, SelectChk, EventChk, BlockChk, inuse, DelChk, ServerName, EditDay, RNo, SNo, Channel, BNum FROM dbo.AGameUser WHERE userid='admin'\"; $rs=$c.Execute($q); for($i=0;$i -lt $rs.Fields.Count;$i++){ Write-Host ($rs.Fields.Item($i).Name + '=' + $rs.Fields.Item($i).Value) }; $c.Close()"
```

### Corrigir `GameCode`

```cmd
powershell -NoProfile -Command "$cs='Provider=SQLOLEDB;Data Source=10.0.2.2,1433;Initial Catalog=AccountDB;User ID=sa;Password=SummitTale@12345;'; $c=New-Object -ComObject ADODB.Connection; $c.Open($cs); $c.Execute(\"UPDATE dbo.AGameUser SET GameCode='PT' WHERE userid='admin'\"); $rs=$c.Execute(\"SELECT userid, Passwd, GameCode, GPCode, BlockChk, DelChk, inuse FROM dbo.AGameUser WHERE userid='admin'\"); for($i=0;$i -lt $rs.Fields.Count;$i++){ Write-Host ($rs.Fields.Item($i).Name + '=' + $rs.Fields.Item($i).Value) }; $c.Close()"
```

### Reiniciar servidor

```cmd
cd /d C:\SummitTale\Servidor

taskkill /F /IM Server.exe

start "" Server.exe

timeout /t 5

tasklist | findstr /i Server.exe

netstat -ano | findstr :1997
```

### Ver logs

```cmd
cd /d C:\SummitTale\Servidor

dir LogFile /o-d

findstr /S /N /I "admin" LogFile\*.log

type LogFile\7-6.log

type "LogFile\(MAC_PC)7-6.log"

powershell -NoProfile -Command "Get-Content 'PriTaTor_Errors.log' -Tail 500"
```

### Comparar DLLs no Linux

```bash
cd "/home/gabriel/Área de Trabalho/summit"

find . -iname "Database.dll" -o -iname "SQL.dll" -o -iname "Clan.dll"

find . -iname "Database.dll" -o -iname "SQL.dll" -o -iname "Clan.dll" \
| while read f; do sha256sum "$f"; done
```

### Strings da DLL

```bash
cd "/home/gabriel/Área de Trabalho/summit"

strings "cliente/dlls/Database.dll" | grep -Ei "SELECT|FROM|WHERE|AllGameUser|GameUser|PersonalMember|Passwd|userid|BlockChk|DelChk|inuse|AccountDB|BillingDb|GPCode|SQLLogon|SetAllGameUser" | head -300
```

### Exportações da DLL

```bash
cd "/home/gabriel/Área de Trabalho/summit"

objdump -p "cliente/dlls/Database.dll" 2>/dev/null | grep -Ei "SQLLogon|SetAllGameUser|SetGameUser|SQLLogout|Export|DLL Name"
```

---

## 20. Observação importante

Não devemos afirmar que o problema é definitivamente senha, DLL ou tabela vazia. Essas hipóteses foram testadas e ficaram improváveis.

A afirmação mais correta no estado atual é:

```text
O problema está após o recebimento do login e provavelmente no resultado de SQLLogon() ou no tratamento imediatamente posterior ao login.
```

Como o servidor ainda não compila, a melhor forma prática de avançar é pelo cliente:

```text
logar o WParam do OPCODE_FAILCONNECT.
```

