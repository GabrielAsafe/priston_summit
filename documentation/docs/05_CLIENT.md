# Summit Tale — Cliente

## Estado

O cliente ainda não foi testado.

## Código fonte

```text
cliente/Source Summit Tale/SrcGame
```

Solução:

```text
cliente/Source Summit Tale/SrcGame/Game.sln
```

Projeto:

```text
cliente/Source Summit Tale/SrcGame/src/priston.vcxproj
```

## Módulos importantes

```text
Login/
Security/
Server/
Language/
HoBaram/
sinbaram/
BattleEvents/
PvP/
CoinShop.cpp
BattlePass.cpp
Rank.cpp
Ranking.cpp
netplay.cpp
smwsock.cpp
SQL.cpp
```

## Antes de testar

- Procurar IPs hardcoded no cliente.
- Identificar porta/host do servidor de login.
- Confirmar compatibilidade com `Server.exe`.
- Verificar se launcher altera configs.

<!-- BEGIN AUTO:client-network -->
## Configuração de conexão

Identificar onde o cliente define o IP, hostname, porta, servidor de login, servidor de jogo, servidor de área e endereços alternativos.

Locais possíveis:

- código-fonte;
- ficheiros `.ini` e `.dat`;
- recursos compilados;
- strings hardcoded;
- launcher;
- serviço web;
- registo do Windows.

## Pesquisa inicial

```bash
grep -RInE "127\\.0\\.0\\.1|localhost|9000|connect\\s*\\(|socket\\s*\\(|inet_addr|gethostbyname|getaddrinfo" SrcGame
```

```bash
grep -RInE "ServerIP|ServerPort|LoginServer|GameServer|AreaServer|WorldServer" SrcGame
```

## Instrumentação recomendada

Adicionar logs antes e depois de `socket()`, resolução DNS, `connect()`, `send()`, `recv()`, `shutdown()` e `closesocket()`.

```text
[CLIENT-NET] destino=127.0.0.1:9000
[CLIENT-NET] socket=123
[CLIENT-NET] connect_result=0
[CLIENT-NET] wsa_error=0
```
<!-- END AUTO:client-network -->
