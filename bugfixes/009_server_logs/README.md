# Fix 009 - Server logs directory

## Estado

APROVADO.

## Base

f9cba455b975f73d71ab585b3b4ea24cd61c2254

## Problema

O servidor já possui a pasta LOGS, mas os ficheiros de log/debug
continuavam sendo escritos diretamente na raiz do runtime.

A auditoria encontrou:

    35 writes na raiz
    0 writes para LOGS

## Alteração

Somente o caminho dos writers foi alterado.

Exemplo:

    fopen("mingw_auth_direct.txt", ...)

para:

    fopen("LOGS\\mingw_auth_direct.txt", ...)

Nenhuma pasta nova é criada.

Nenhuma lógica de autenticação, gameplay, SQL, rede ou logging foi
alterada.

## Escopo

Foram redirecionados os writers auditados dos seguintes ficheiros:

- game.log
- BackupData.log
- mingw_server_transport.txt
- SQL.txt
- abc.txt
- mingw_logsql_worker.txt
- mingw_real_server_login.txt
- mingw_auth_direct.txt
- mingw_processinfo.txt
- mingw_getrecord_trace.txt
- mingw_checkusermodel.txt
- mingw_insrecord_probe.txt
- mingw_disconnect_caller.txt
- ClanDebug.txt
- DDD2.txt
- game_debug.txt

## Evidências

- before/
- after/
- hashes_before.sha256
- hashes_after.sha256
- replacements.txt
- change.patch

## Teste

Com Server.exe fechado:

    cd SourceCode
    sh server.sh

Depois:

1. iniciar Server.exe;
2. confirmar AccountDB;
3. confirmar BillingDB;
4. fazer login pelo cliente;
5. entrar no mapa;
6. confirmar NPCs;
7. confirmar novos logs em LOGS;
8. confirmar que os mesmos logs não recebem novas escritas na raiz.

## Fora do escopo

Não altera as MessageBox DEBUG do cliente.

Essas serão tratadas separadamente no Fix 014.

## Resultado

APROVADO.

Teste realizado em 2026-08-24.

Validação no runtime Windows:

    C:\SummitTale\Servidor

A pasta já existente:

    C:\SummitTale\Servidor\LOGS

recebeu durante a execução:

    mingw_server_transport.txt
    mingw_logsql_worker.txt

Os ficheiros auditados não foram criados/atualizados na raiz do servidor.

Os demais logs da lista podem não existir em uma execução específica
quando os respectivos caminhos de código não são acionados.

Validação de source antes do teste:

    ROOT_WRITERS = 0
    LOGS_WRITERS = 35
    AUDIT009_RC = 0

Conclusão:

O redirecionamento dos logs do servidor para a pasta LOGS foi aprovado.

