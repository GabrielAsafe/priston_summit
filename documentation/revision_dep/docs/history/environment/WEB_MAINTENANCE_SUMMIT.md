# Summit Tale — Manutenção Web PHP / Docker

## Estado atual

O projeto web legado foi migrado para Docker com PHP antigo, porque tentar portar diretamente para PHP 8 gerou muitos erros de compatibilidade.

## O que aprendemos

### 1. PHP 8 não é adequado para rodar o painel legado sem grande refatoração

Foram encontrados problemas como:

- `session_register()`
- `session_unregister()`
- `session_is_registered()`
- `mysql_escape_string()`
- `each()`
- `ereg()` / `eregi()`
- `&$var` em chamadas de função
- arrays sem aspas, exemplo `$_POST[username]`
- strings quebradas com `$_POST["campo"]` dentro de aspas duplas

Conclusão: portar tudo para PHP 8 seria trabalhoso demais para esta fase.

---

### 2. Ambiente web foi migrado para PHP 5.6

Foi criado um novo `Dockerfile.web` baseado em Debian Jessie com:

- Apache 2
- PHP 5.6
- php5-odbc
- FreeTDS
- unixODBC
- ODBC ativo

Validação:

```text
PHP 5.6.40
odbc
PDO_ODBC

---

## Login normal do painel `/painel/`

O painel normal usa `action=Logar`, não `acao=Logar`.

Form correto:

```text
username=<usuario>
password=<senha>
action=Logar

---

## Personagens no painel normal

O painel não lista personagens apenas pela base de dados.

Ele procura ficheiros `.dat` no servidor:

```text
/srv/summit/Servidor/DataServer/userinfo/<num>/<userid>.dat
/srv/summit/Servidor/DataServer/userdata/<num>/<charname>.dat

Para a conta alface, numDir(alface) retornou:

156

Logo o ficheiro esperado é:

/srv/summit/Servidor/DataServer/userinfo/156/alface.dat

Esse ficheiro não existe atualmente, por isso o painel mostra:

PERSONAGENS: Vazio

Conclusão:

Login do painel está funcional.
Montagem do path /srv/summit/Servidor está funcional.
A conta alface ainda não tem personagem criado no DataServer.
O próximo teste deve ser criar personagem pelo cliente do jogo.

---

## Correção da leitura de personagem `.dat`

O painel antigo lia o dono do personagem no offset errado:

```php
$charID = trim(substr($fRead,0x2c0,10),"\x00");
No ficheiro real:

Servidor/DataServer/userdata/247/Hancock.dat

foi encontrado:

0x10  -> Hancock
0x2ec -> cicin

Correção aplicada em:

docker/web/wwwroot - EPIC SIMPLE/painel/inicial.inc.php

Alterado para:

$charID = trim(substr($fRead,0x2ec,10),"\x00");

Resultado:

NICK: Hancock
CLASSE: Archer
LEVEL: 75

Conclusão:

O painel agora consegue:

listar personagens via userinfo;
abrir o .dat real em userdata;
validar corretamente o dono do personagem;
carregar nick, classe e level.
