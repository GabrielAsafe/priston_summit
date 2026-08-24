# Fix 001 - Server hotuk deploy

## Estado

APROVADO.

## Problema

O cliente crashava depois do login mesmo usando executáveis conhecidos como funcionais.

O servidor estava configurado em `Configs/hotuk.ini` com:

    *GAME_SERVER   124.198.128.16  124.198.128.16  124.198.128.16

enquanto o ambiente atual utiliza:

    192.168.1.111

Além disso, `SourceCode/send_to_s.sh` enviava somente:

- Server.exe
- libgcc_s_dw2-1.dll
- libstdc++-6.dll
- libwinpthread-1.dll

O `hotuk.ini` não era enviado pelo fluxo normal de deploy.

Isso permitia que o binário compilado e a configuração efetivamente utilizada
no Windows divergissem.

## Diagnóstico

Teste isolado feito diretamente no Windows:

ANTES:

    *GAME_SERVER   124.198.128.16  124.198.128.16  124.198.128.16

Resultado:

- cliente crashava depois do login.

ALTERAÇÃO ISOLADA:

    *GAME_SERVER   192.168.1.111  192.168.1.111  192.168.1.111

Resultado observado:

- cliente entrou no jogo;
- pet presente;
- F11 não causou crash imediato;
- NPCs ainda ausentes.

Portanto o crash de login/entrada foi isolado ao GAME_SERVER incorreto.

## Alterações deste fix

### Servidor/Configs/hotuk.ini

GAME_SERVER:

    124.198.128.16
    ->
    192.168.1.111

O ficheiro passa também a ser versionado pelo Git.

### SourceCode/send_to_s.sh

Adicionado envio explícito de:

    ../Servidor/Configs/hotuk.ini

para:

    C:/SummitTale/Servidor/Configs/hotuk.ini

Assim `server.sh`, que já chama `send_to_s.sh`, passa a colocar no Windows a
mesma configuração versionada usada pelo checkpoint.

## Ficheiros funcionais alterados

- SourceCode/send_to_s.sh
- Servidor/Configs/hotuk.ini

## Evidências

- `before/` contém cópias integrais anteriores.
- `after/` contém cópias integrais posteriores.
- `hashes_before.sha256` contém SHA-256 anteriores.
- `hashes_after.sha256` contém SHA-256 posteriores.
- `change.patch` contém o patch funcional Git.

## Fora do escopo

Este fix NÃO altera:

- NPCs;
- spawn;
- items;
- drops;
- merchants;
- cliente;
- código C/C++;
- DLLs;
- outros parâmetros de hotuk.ini.

## Teste final necessário

Executar:

    cd SourceCode
    sh server.sh

Confirmar no Windows que o hotuk.ini recebido contém:

    *GAME_SERVER   192.168.1.111  192.168.1.111  192.168.1.111

Depois iniciar/reiniciar Server.exe e confirmar que o cliente entra normalmente.

Somente após esse teste o fix deve ser marcado APROVADO e commitado.


## Resultado final do teste

Data: 2026-08-24

O fluxo oficial foi executado através de:

    cd SourceCode
    sh server.sh

Resultado do build:

- 186 objetos compilados;
- 186 OK;
- 0 FAIL;
- link OK.

Resultado do deploy:

- Server.exe enviado com sucesso;
- libgcc_s_dw2-1.dll enviada com sucesso;
- libstdc++-6.dll enviada com sucesso;
- libwinpthread-1.dll enviada com sucesso;
- Configs/hotuk.ini enviado com sucesso.

Validação no Windows:

    *GAME_SERVER   192.168.1.111  192.168.1.111  192.168.1.111

SHA-256 do hotuk.ini implantado:

    87c04eb8390feb982c8be934b155f85e816b00a739ec2a567774d93335abce06

SHA-256 observado do Server.exe implantado:

    24d348278ba2c1fd1908eb69b78d2398aa5122958f4b4b9f853ebe9e828af6d8

Teste funcional:

- servidor iniciou;
- conexão SQL AccountDB OK;
- conexão SQL BillingDB OK;
- login do cliente OK;
- seleção de personagem OK;
- entrada no mapa OK;
- crash anterior não ocorreu.

Situação observada após o fix:

- pet presente;
- NPCs ainda ausentes;
- NPCs serão tratados separadamente no Fix 002.

Conclusão:

O Fix 001 foi APROVADO.
