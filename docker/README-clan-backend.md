# Summit Tale --- Backend de Clãs em Docker

## Visão geral

Este documento descreve o trabalho feito para tornar o backend legado de
clãs executável no ambiente Docker/Linux do projeto Summit Tale.

O backend original de clãs foi escrito principalmente em **Classic ASP /
ASPX**, usando `ADODB.Connection` + `SQLOLEDB`, tecnologia pensada para
IIS/Windows. O ambiente atual usa **Debian Jessie + Apache + PHP 5.6 +
unixODBC/FreeTDS**, com o SQL Server num container separado.

A solução adotada foi **portar os endpoints necessários para PHP**,
mantendo as URLs legadas `.asp` e `.aspx`, os nomes dos parâmetros e o
protocolo textual esperado pelo jogo.

Resultado validado:

-   containers BPT e EPIC iniciam normalmente;
-   PHP 5.6 está ativo;
-   ODBC e PDO_ODBC estão carregados;
-   `_clan_port/lib.php` está disponível nos dois webroots;
-   URLs PHP funcionam;
-   URLs legadas `.asp` / `.aspx` portadas são executadas pelo PHP;
-   smoke tests retornam `Code=100` quando chamados sem parâmetros;
-   a suíte de alcance dos endpoints foi executada com sucesso.

------------------------------------------------------------------------

## Arquitetura

``` text
                         +----------------------+
                         |    Cliente / Jogo    |
                         +----------+-----------+
                                    |
                        HTTP legado .asp/.aspx
                                    |
                 +------------------+------------------+
                 |                                     |
                 v                                     v
        localhost:8088                         localhost:8089
        summit-web-bpt                         summit-web-epic
        Apache + PHP 5.6                       Apache + PHP 5.6
                 |                                     |
                 +------------------+------------------+
                                    |
                              ODBC / FreeTDS
                                    |
                                    v
                           summittale-mssql:1433
                              Microsoft SQL Server
                                    |
                                    v
                                  ClanDB
```

O Apache continua expondo as URLs que o cliente antigo conhece. Apenas
os endpoints de clã explicitamente portados são associados ao handler
PHP.

------------------------------------------------------------------------

# Estrutura do projeto

O projeto Docker utilizado fica em:

``` text
/home/gabriel/Área de Trabalho/summit/docker
```

A estrutura relevante é:

``` text
docker/
├── Dockerfile.web
├── docker-compose.web.yml
├── backups/
└── web/
    ├── wwwroot - BPT/
    │   ├── _clan_port/
    │   │   └── lib.php
    │   └── Clan/
    │       ├── verificarcl.asp
    │       ├── verificarcl.php
    │       ├── ClanInsert.aspx
    │       └── ClanInsert.php
    │
    └── wwwroot - EPIC SIMPLE/
        ├── _clan_port/
        │   └── lib.php
        ├── Clan/
        │   ├── verificarcl.asp
        │   ├── verificarcl.php
        │   ├── ClanInsert.aspx
        │   └── ClanInsert.php
        └── ServerMain/
            ├── ClanSubChipRelease.asp / .php
            ├── ClanSubChipUpdate.asp / .php
            ├── GetAllMyClanMember.asp / .php
            ├── clanChipChange.asp / .php
            ├── clanInsertClanWon.asp / .php
            ├── clanRemove.asp / .php
            ├── clanWonRelease.asp / .php
            ├── clanWonSelfLeave.asp / .php
            ├── isCheckClanJang.asp / .php
            ├── isCheckClanName.asp / .php
            ├── isCheckClanNum.asp / .php
            ├── isCheckClanwon.asp / .php
            ├── isClanMember.asp / .php
            ├── isKflag.asp / .php
            ├── isPFlag.asp / .php
            └── sod2info.asp / .php
```

------------------------------------------------------------------------

# Pacote de instalação

O pacote utilizado para aplicar o port fica em:

``` text
/home/gabriel/Transferências/clan-backend-port
```

Estrutura:

``` text
clan-backend-port/
├── install.sh
├── Dockerfile.web.ported
├── README.md
├── PORTING_NOTES.md
├── VERIFY_PATHS.txt
├── tests/
│   ├── test_endpoints.sh
│   └── last_local_reachability.txt
└── webroot/
    ├── _clan_port/
    │   └── lib.php
    ├── Clan/
    │   ├── verificarcl.asp
    │   ├── verificarcl.php
    │   ├── ClanInsert.aspx
    │   └── ClanInsert.php
    └── ServerMain/
        └── endpoints portados
```

## `install.sh`

É o instalador do port.

Ele:

1.  valida os caminhos esperados;
2.  cria backup do backend atual;
3.  copia `_clan_port` para BPT e EPIC;
4.  copia os endpoints `Clan/` para BPT e EPIC;
5.  copia `ServerMain/` para EPIC;
6.  substitui `Dockerfile.web` pela versão preparada para o port;
7.  valida os ficheiros no host;
8.  reconstrói os containers;
9.  valida os ficheiros dentro dos containers;
10. executa lint PHP;
11. executa smoke tests HTTP.

Os backups são criados em:

``` text
/home/gabriel/Área de Trabalho/summit/docker/backups/clan-port-YYYYMMDD-HHMMSS/
```

------------------------------------------------------------------------

# Webroots

## BPT --- porta 8088

Container:

``` text
summit-web-bpt
```

URL base:

``` text
http://localhost:8088
```

O BPT recebe principalmente o fluxo de criação em `Clan/`.

Estrutura adicionada:

``` text
wwwroot - BPT/
├── _clan_port/
│   └── lib.php
└── Clan/
    ├── verificarcl.asp
    ├── verificarcl.php
    ├── ClanInsert.aspx
    └── ClanInsert.php
```

## EPIC SIMPLE --- porta 8089

Container:

``` text
summit-web-epic
```

URL base:

``` text
http://localhost:8089
```

É onde está a API completa de manutenção de clãs:

``` text
wwwroot - EPIC SIMPLE/
├── _clan_port/
├── Clan/
└── ServerMain/
```

------------------------------------------------------------------------

# `_clan_port/lib.php`

Este é o helper compartilhado pelos endpoints portados.

Localização dentro de cada container:

``` text
/var/www/html/_clan_port/lib.php
```

Ele é responsável por:

-   resposta `text/plain`;
-   protocolo usando `CR` (`\r`) como separador;
-   leitura case-insensitive dos parâmetros, semelhante ao `Request()`
    do ASP;
-   validação de parâmetros obrigatórios;
-   leitura da configuração SQL já existente no site;
-   conexão via ODBC;
-   seleção da base `ClanDB`;
-   escaping dos parâmetros SQL;
-   execução de queries;
-   leitura de Recordsets;
-   transações;
-   commit e rollback;
-   retorno padronizado de `Code=100` em falhas.

A configuração de conexão é procurada em:

``` text
/var/www/html/padm/incluir/configura.php
```

ou:

``` text
/var/www/html/painel/incluir/configura.php
```

Depois da conexão ODBC é executado:

``` sql
USE ClanDB
```

------------------------------------------------------------------------

# Dockerfile.web

O web container utiliza:

``` text
Debian Jessie
Apache 2.4
PHP 5.6
php5-odbc
unixODBC
FreeTDS
```

O FreeTDS aponta para:

``` text
summittale-mssql:1433
```

DSN:

``` text
SummitSQL
```

O Apache **não executa qualquer `.asp` existente no site como PHP**.

Foi criada uma configuração `LocationMatch` que permite PHP apenas para
a lista conhecida de endpoints portados.

Isso mantém compatibilidade com chamadas antigas como:

``` text
/ServerMain/clanRemove.asp
```

mesmo que o conteúdo atual desse ficheiro seja PHP.

Os ficheiros `.php` equivalentes também ficam disponíveis para
diagnóstico e testes.

------------------------------------------------------------------------

# Base de dados de clãs

Os endpoints trabalham principalmente com a base:

``` text
ClanDB
```

As tabelas identificadas como principais são:

## `CL`

Tabela principal dos clãs.

Campos utilizados incluem:

``` text
IDX
ClanName
UserID
ClanZang
MemCnt
Note
MIconCnt
RegiDate
LimitDate
DelActive
PFlag
KFlag
Cpoint
ClanMoney
CNFlag
```

Interpretação:

-   `IDX`: identificador do clã;
-   `ClanName`: nome;
-   `UserID`: conta associada ao líder;
-   `ClanZang`: personagem líder;
-   `MemCnt`: número de membros;
-   `MIconCnt`: identificador do ícone;
-   `RegiDate`: criação;
-   `LimitDate`: limite/data legada;
-   `Cpoint`: pontuação/ranking;
-   `ClanMoney`: dinheiro do clã.

## `UL`

Tabela de membros dos clãs.

Campos usados:

``` text
IDX
userid
ChName
ClanName
ChType
ChLv
Permi
JoinDate
MIconCnt
```

A relação lógica é:

``` text
CL.IDX
   |
   +------ UL.IDX
```

`Permi` é utilizado para permissões:

``` text
0 = membro normal
2 = sublíder
```

A liderança principal é identificada através de `CL.ClanZang`.

## `LI`

Mantém o contador utilizado para gerar `MIconCnt`.

O port mantém o comportamento legado:

``` text
LI.ID = 1
LI.IMG = contador atual
```

Na criação de um clã, `IMG` é incrementado e utilizado como `MIconCnt`.

`LI` não contém a imagem propriamente dita.

## `CT`

Tabela utilizada como gate/contexto de autorização em determinadas
operações.

Possui informação como:

``` text
ServerName
MIDX
ClanName
ClanJang
ClanImage
UserID
ChName
GPCode
LogonTime
IP
Flag
```

`verificarcl.asp` e `clanRemove.asp` exigem exatamente um registo em
`CT` para o `userid`.

Isto preserva o comportamento legado que tentava distinguir uma operação
iniciada pelo fluxo do jogo de uma chamada direta pelo navegador.

------------------------------------------------------------------------

# APIs expostas

As URLs abaixo são relativas ao host.

No EPIC:

``` text
http://localhost:8089
```

No BPT, para os endpoints `Clan/`:

``` text
http://localhost:8088
```

Os parâmetros são aceites através de `$_REQUEST` nos ports
compartilhados, mantendo comportamento próximo do `Request()` do ASP.
`isClanMember` preserva o fluxo legado via query string.

------------------------------------------------------------------------

## Criação de clã

### `GET/POST /Clan/verificarcl.asp`

Equivalente PHP:

``` text
/Clan/verificarcl.php
```

Parâmetros obrigatórios:

``` text
userid
gserver
chname
clName
chtype
lv
```

Função:

-   valida parâmetros;
-   conecta à `ClanDB`;
-   exige `CT.userid` com exatamente um registo;
-   verifica se o personagem já pertence a um clã;
-   verifica se `clName` já existe;
-   incrementa `LI.IMG`;
-   cria o registo em `CL`;
-   cria o líder em `UL`.

Respostas relevantes:

``` text
Code=1
CMoney=500000
```

Sucesso.

``` text
Code=2
CMoney=0
```

Personagem já associado a um clã.

``` text
Code=3
CMoney=0
```

Nome do clã já existe.

``` text
Code=100
```

Parâmetros inválidos ou erro.

Se a validação `CT` falhar, preserva a mensagem legada informando que a
criação está sendo tentada pelo navegador.

### `GET/POST /Clan/ClanInsert.aspx`

Equivalente:

``` text
/Clan/ClanInsert.php
```

Parâmetros:

``` text
userid
gserver
chname
clName
chtype
lv
```

Executa praticamente o mesmo fluxo de criação, porém preserva a
diferença do legado: **não exige o gate da tabela `CT`**.

------------------------------------------------------------------------

# Consulta e estado do clã

## `/ServerMain/isClanMember.asp`

Equivalente:

``` text
/ServerMain/isClanMember.php
```

Parâmetros:

``` text
userid
gserver
chname
```

Retorna o estado do personagem e informações do clã.

Códigos principais:

``` text
Code=0   sem clã
Code=1   membro
Code=2   líder
Code=5   sublíder
Code=100 erro
```

O protocolo pode retornar campos como:

``` text
CName
CNote
CZang
CSubChip
CStats
CMCnt
CIMG
CSec
CRegiD
CLimitD
CDelActive
CPFlag
CKFlag
CMoney
CNFlag
```

O separador legado `CR` (`0x0D`) foi mantido.

## `/ServerMain/GetAllMyClanMember.asp`

Parâmetros:

``` text
userid
gserver
chname
```

Função:

-   descobre o clã através de `UL`;
-   obtém o líder em `CL`;
-   lista os membros em `UL`.

Resposta inclui:

``` text
Code
CClanName
CClanZang
CMem
```

`CMem` é repetido para cada membro.

## `/ServerMain/isCheckClanJang.asp`

Parâmetros:

``` text
userid
gserver
```

Verifica se existe um clã cujo `CL.UserID` corresponde ao utilizador.

Retorno principal:

``` text
Code=0
Code=1
Code=100
```

## `/ServerMain/isCheckClanName.asp`

Parâmetros:

``` text
ClName
gserver
```

Verifica se o nome de clã existe em `CL`.

## `/ServerMain/isCheckClanNum.asp`

Parâmetro:

``` text
num
```

Procura o clã através de `MIconCnt`.

Quando encontrado, retorna:

``` text
Code=1
CName=<nome>
CNote=<nota>
```

## `/ServerMain/isCheckClanwon.asp`

Parâmetros:

``` text
clwon
gserver
```

O endpoint legado/port atual funciona como um check simples e retorna
`Code=1` quando os parâmetros obrigatórios estão presentes.

## `/ServerMain/isKflag.asp`

Parâmetro utilizado:

``` text
chname
```

Verifica se o personagem possui registo de associação em `UL`.

## `/ServerMain/isPFlag.asp`

Parâmetro utilizado:

``` text
chname
```

Assim como `isKflag`, verifica a associação do personagem em `UL`.

------------------------------------------------------------------------

# Gestão de membros

## `/ServerMain/clanInsertClanWon.asp`

Parâmetros:

``` text
userid
gserver
chname
clName
clwon
clwonUserid
lv
chtype
chlv
```

Função:

-   carrega o clã;
-   verifica líder/sublíder;
-   verifica se o alvo já pertence a um clã;
-   atualiza `CL.MemCnt`;
-   adiciona o novo membro em `UL`.

O port utiliza transação para manter `CL` e `UL` sincronizadas.

## `/ServerMain/clanWonRelease.asp`

Parâmetros:

``` text
userid
gserver
chname
clName
clwon1
ticket
```

Função:

-   localiza o membro;
-   confirma o clã;
-   impede remover o próprio líder através desse fluxo;
-   decrementa `CL.MemCnt`;
-   remove o membro de `UL`.

## `/ServerMain/clanWonSelfLeave.asp`

Parâmetros:

``` text
userid
gserver
chname
clName
```

Função:

-   permite que um membro abandone o clã;
-   impede o líder de sair por este fluxo;
-   decrementa `MemCnt`;
-   remove o personagem de `UL`.

------------------------------------------------------------------------

# Liderança e subliderança

## `/ServerMain/clanChipChange.asp`

Parâmetros:

``` text
chname
gserver
clName
```

Apesar do nome, este endpoint **não altera a imagem do clã**.

Ele transfere/define a liderança:

``` sql
UPDATE CL
SET ClanZang = <chname>,
    UserID   = <userid>
WHERE ClanName = <clName>
```

O `userid` é obtido através de `UL`.

## `/ServerMain/ClanSubChipUpdate.asp`

Parâmetros:

``` text
chname
gserver
```

Define o personagem como sublíder.

O fluxo:

``` text
todos os Permi do clã -> 0
personagem escolhido  -> Permi=2
```

## `/ServerMain/ClanSubChipRelease.asp`

Parâmetros:

``` text
chname
gserver
```

Remove a permissão de sublíder:

``` text
Permi=0
```

------------------------------------------------------------------------

# Remoção de clã

## `/ServerMain/clanRemove.asp`

Parâmetros:

``` text
userid
gserver
chname
clName
```

Função:

1.  exige exatamente um registo `CT` para o `userid`;
2.  confirma que `chname` é `CL.ClanZang`;
3.  remove o clã de `CL`;
4.  remove os membros correspondentes de `UL`.

As duas remoções são executadas em transação.

Sucesso:

``` text
Code=1
```

Falha de validação:

``` text
Code=0
```

Erro/parâmetro ausente:

``` text
Code=100
```

------------------------------------------------------------------------

# SOD / ranking

## `/ServerMain/sod2info.asp`

Parâmetros obrigatórios:

``` text
userid
gserver
chname
index
```

O `index` define a operação.

### `index=1`

Obtém informação do clã com maior `Cpoint`, incluindo dados como:

``` text
CClanMoney
CTax
CName
CNote
CZang
CIMG
TotalEMoney
TotalMoney
```

Também determina a relação do personagem com o clã vencedor.

### `index=3`

Retorna até nove clãs com `Cpoint > 0`, ordenados por pontuação.

Campos:

``` text
CIMG
CName
CPoint
CRegistDay
```

Outros índices retornam:

``` text
Code=104
```

------------------------------------------------------------------------

# Compatibilidade das URLs legadas

O cliente pode continuar chamando:

``` text
/Clan/verificarcl.asp
/Clan/ClanInsert.aspx
/ServerMain/clanRemove.asp
...
```

O Apache possui uma configuração específica para executar esses caminhos
com mod_php.

Os equivalentes `.php` existem principalmente para:

-   diagnóstico;
-   teste manual;
-   manutenção;
-   facilitar futuras migrações.

Exemplo:

``` bash
curl -i "http://localhost:8089/ServerMain/clanRemove.asp"
```

Sem parâmetros:

``` text
Code=100
```

Isto é esperado e comprova que a URL chegou ao código PHP e que a
validação inicial foi executada.

------------------------------------------------------------------------

# Testes

A suíte fica em:

``` text
/home/gabriel/Transferências/clan-backend-port/tests/test_endpoints.sh
```

Executar:

``` bash
/home/gabriel/Transferências/clan-backend-port/tests/test_endpoints.sh \
  http://localhost:8089
```

Ela valida 18 endpoints.

Para endpoints que modificam a base, a suíte propositalmente **não envia
os parâmetros obrigatórios**. Dessa forma, o endpoint deve retornar:

``` text
Code=100
```

antes de realizar qualquer escrita.

Para `isKflag` e `isPFlag`, utiliza um personagem fictício:

``` text
__reachability_probe__
```

Assim, a suíte é principalmente um teste de:

``` text
HTTP
  -> Apache
  -> handler .asp/.aspx
  -> PHP
  -> carregamento do endpoint
  -> validação básica
```

Ela **não substitui testes funcionais completos com personagens/clãs
reais**.

------------------------------------------------------------------------

# Smoke tests úteis

### EPIC

``` bash
curl -i "http://localhost:8089/ServerMain/clanRemove.php"
curl -i "http://localhost:8089/ServerMain/clanRemove.asp"
curl -i "http://localhost:8089/Clan/verificarcl.asp"
```

Sem parâmetros, esperado:

``` text
Code=100
```

### BPT

``` bash
curl -i "http://localhost:8088/Clan/verificarcl.asp"
```

Esperado:

``` text
Code=100
```

------------------------------------------------------------------------

# Diagnóstico

Ver logs:

``` bash
docker logs --tail 100 summit-web-epic
docker logs --tail 100 summit-web-bpt
```

Validar PHP:

``` bash
docker exec summit-web-epic php -l /var/www/html/_clan_port/lib.php
docker exec summit-web-epic php -l /var/www/html/ServerMain/clanRemove.php
```

Validar módulos ODBC:

``` bash
docker exec summit-web-epic php -m | grep -i odbc
```

Esperado:

``` text
odbc
PDO_ODBC
```

Validar helper:

``` bash
docker exec summit-web-epic \
  test -f /var/www/html/_clan_port/lib.php
```

------------------------------------------------------------------------

# Containers

Os containers relevantes são:

``` text
summit-web-bpt
summit-web-epic
summittale-mssql
summittale-dbadmin
```

Portas:

``` text
8088 -> BPT
8089 -> EPIC SIMPLE
1433 -> SQL Server
8978 -> CloudBeaver
```

O aviso do Docker Compose sobre containers órfãos é esperado quando os
serviços web são geridos por um compose separado do SQL Server.

**Não usar `--remove-orphans` indiscriminadamente**, pois
`summittale-mssql` pode aparecer nessa lista apesar de ser necessário
para o backend.

------------------------------------------------------------------------

# Rebuild manual

Caso seja necessário reconstruir os serviços web:

``` bash
cd "/home/gabriel/Área de Trabalho/summit/docker"

docker compose -f docker-compose.web.yml down
docker compose -f docker-compose.web.yml build --no-cache
docker compose -f docker-compose.web.yml up -d
```

Depois:

``` bash
docker ps
```

------------------------------------------------------------------------

# Reinstalar o port

A forma recomendada é usar o instalador:

``` bash
cd /home/gabriel/Transferências/clan-backend-port

chmod +x install.sh
./install.sh
```

Não é necessário apagar `Clan/` ou `ServerMain/`.

O instalador faz **overlay** dos endpoints portados e mantém os outros
ficheiros existentes no site.

------------------------------------------------------------------------

# Decisões de compatibilidade

## Protocolo textual

O backend continua retornando respostas no formato esperado pelo cliente
legado:

``` text
Code=...
Campo=...
Campo=...
```

Quando aplicável, o separador original `CR` (`0x0D`) foi mantido.

## Nomes de parâmetros

Foram mantidos os nomes do ASP original, inclusive diferenças como:

``` text
clName
ClName
clwon
clwon1
```

## `GETDATE()+3600`

O fluxo de criação preserva:

``` sql
GETDATE()+3600
```

No SQL Server isto representa **3600 dias**, não uma hora.

Foi mantido intencionalmente para não alterar silenciosamente a
semântica dos dados legados.

## SQL

O código original concatenava diretamente os parâmetros recebidos nas
queries.

O helper PHP faz escaping dos valores antes da execução, reduzindo o
risco da concatenação direta existente no ASP.

## Transações

Operações que precisam alterar mais de uma tabela, como:

``` text
criação de clã
remoção de clã
entrada de membro
saída/expulsão de membro
```

foram agrupadas em transações quando apropriado para evitar estados
parciais entre `CL` e `UL`.

------------------------------------------------------------------------

# Estado atual

A infraestrutura do port está funcional:

``` text
[OK] Apache
[OK] PHP 5.6
[OK] unixODBC / FreeTDS
[OK] acesso ao helper compartilhado
[OK] URLs .php
[OK] URLs legadas .asp/.aspx portadas
[OK] BPT :8088
[OK] EPIC :8089
[OK] smoke tests
[OK] suíte de reachability dos endpoints
```

Isso valida a **infraestrutura e o alcance das APIs**.

Testes funcionais com dados reais ainda devem ser feitos de forma
controlada para confirmar cada operação contra o estado real da
`ClanDB`, especialmente:

``` text
criar clã
adicionar membro
definir/remover sublíder
transferir liderança
membro sair
expulsar membro
remover clã
ranking/SOD
```

------------------------------------------------------------------------

# Resumo das APIs

  Endpoint legado                        Função
  -------------------------------------- ----------------------------------------
  `/Clan/verificarcl.asp`                Criar clã com gate `CT`
  `/Clan/ClanInsert.aspx`                Criar clã sem gate `CT`
  `/ServerMain/isClanMember.asp`         Estado completo do personagem/clã
  `/ServerMain/GetAllMyClanMember.asp`   Listar membros
  `/ServerMain/isCheckClanJang.asp`      Verificar liderança por userid
  `/ServerMain/isCheckClanName.asp`      Verificar existência do nome
  `/ServerMain/isCheckClanNum.asp`       Resolver MIconCnt para clã
  `/ServerMain/isCheckClanwon.asp`       Check legado de membro
  `/ServerMain/isKflag.asp`              Check legado relacionado ao personagem
  `/ServerMain/isPFlag.asp`              Check legado relacionado ao personagem
  `/ServerMain/clanInsertClanWon.asp`    Adicionar membro
  `/ServerMain/clanWonRelease.asp`       Expulsar membro
  `/ServerMain/clanWonSelfLeave.asp`     Sair do clã
  `/ServerMain/clanChipChange.asp`       Transferir liderança
  `/ServerMain/ClanSubChipUpdate.asp`    Definir sublíder
  `/ServerMain/ClanSubChipRelease.asp`   Remover sublíder
  `/ServerMain/clanRemove.asp`           Remover clã
  `/ServerMain/sod2info.asp`             Informação SOD/ranking

------------------------------------------------------------------------

## Nota final

O objetivo deste port não foi redesenhar o protocolo do jogo. Foi manter
o comportamento externo do backend legado enquanto se substitui a
dependência de IIS/Classic ASP por uma implementação que funciona no
ambiente Docker/Linux atual.

Por isso, as URLs, parâmetros e respostas legadas foram preservados
sempre que possível.
