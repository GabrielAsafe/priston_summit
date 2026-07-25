#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$(pwd)}"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP_DIR="$ROOT/.doc-backups/$TIMESTAMP"

cd "$ROOT"
mkdir -p "$BACKUP_DIR" docs

backup_file() {
    local file="$1"
    if [[ -f "$file" ]]; then
        mkdir -p "$BACKUP_DIR/$(dirname "$file")"
        cp -a "$file" "$BACKUP_DIR/$file"
    fi
}

ensure_file() {
    local file="$1"
    local title="$2"
    if [[ ! -f "$file" ]]; then
        mkdir -p "$(dirname "$file")"
        printf '# %s\n' "$title" > "$file"
    fi
}

replace_marked_block() {
    local file="$1"
    local block_name="$2"
    local content_file="$3"
    local begin="<!-- BEGIN AUTO:${block_name} -->"
    local end="<!-- END AUTO:${block_name} -->"
    local tmp
    tmp="$(mktemp)"

    awk -v begin="$begin" -v end="$end" '
        $0 == begin { skipping=1; next }
        $0 == end   { skipping=0; next }
        !skipping   { print }
    ' "$file" > "$tmp"

    {
        cat "$tmp"
        printf '\n%s\n' "$begin"
        cat "$content_file"
        printf '%s\n' "$end"
    } > "$file"

    rm -f "$tmp"
}

write_block() {
    local file="$1"
    local block_name="$2"
    local title="$3"
    local tmp

    ensure_file "$file" "$title"
    backup_file "$file"
    tmp="$(mktemp)"
    cat > "$tmp"
    replace_marked_block "$file" "$block_name" "$tmp"
    rm -f "$tmp"
    echo "Atualizado: $file"
}

write_block "MINGW_PORT_STATUS_2026_07.md" "connection-status" "Estado do port MinGW — Julho de 2026" <<'BLOCK'
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
BLOCK

write_block "docs/milestone_servidorcompilado_funcional.md" "connection-next-step" "Marco — servidor compilado e funcional" <<'BLOCK'
## Estado confirmado

- O servidor compila com MinGW i686.
- O executável final utiliza o subsistema `Windows CUI`.
- O SQL Server conecta corretamente.
- Mapas, itens, NPCs e sistemas principais são carregados.
- `InitAll()` termina completamente.
- O servidor entra no loop principal.
- A porta TCP é aberta.
- A CLI é executada na consola.
- O processo permanece ativo.

## Correção do subsistema do executável

O executável compilado inicialmente estava configurado como:

```text
Subsystem 00000002 (Windows GUI)
```

O linker foi alterado para usar:

```text
-mconsole
```

O executável final passou a apresentar:

```text
Subsystem 00000003 (Windows CUI)
```

## Próximo objetivo

Estabelecer a conexão TCP entre o cliente e o servidor e identificar o primeiro pacote do protocolo de login.
BLOCK

write_block "investigando cliente e server não conectarem.md" "connection-investigation" "Investigação da conexão entre cliente e servidor" <<'BLOCK'
## Objetivo atual

Fazer o cliente SummitTale estabelecer conexão TCP com o servidor compilado em MinGW.

## Estado confirmado do servidor

- O servidor inicia corretamente.
- A conexão SQL funciona.
- A inicialização termina.
- O processo permanece ativo.
- O servidor entra no loop principal.
- A porta configurada é `9000/TCP`.
- O executável utiliza o subsistema Windows Console.
- Não existe crash durante a inicialização.

## Perguntas da investigação

1. Para qual IP o cliente tenta conectar?
2. Para qual porta o cliente tenta conectar?
3. O cliente chega a executar `connect()`?
4. Qual é o código de retorno de `connect()`?
5. Qual é o resultado de `WSAGetLastError()` quando ocorre falha?
6. O servidor executa `accept()`?
7. O cliente envia algum pacote?
8. O servidor recebe algum byte?
9. O opcode recebido é reconhecido?
10. O servidor ou o cliente fecha a conexão?
11. Existem diferenças de estrutura, alinhamento ou tamanho dos pacotes?

## Checklist de conectividade

| Etapa | Estado | Evidência |
|---|---|---|
| Servidor inicia completamente | Confirmado | Logs de inicialização |
| Servidor escuta na porta 9000 | Confirmado | `netstat` |
| Cliente usa o IP correto | Pendente | Instrumentação do cliente |
| Cliente usa a porta correta | Pendente | Instrumentação do cliente |
| TCP SYN chega ao servidor | Pendente | Wireshark ou `pktmon` |
| `connect()` retorna sucesso | Pendente | Log do cliente |
| `accept()` é executado | Pendente | Log do servidor |
| Primeiro pacote é enviado | Pendente | Log ou captura |
| Primeiro pacote é recebido | Pendente | Log do servidor |
| Opcode é reconhecido | Pendente | Dispatch de pacotes |
| Login chega ao SQL | Pendente | Log do servidor/SQL |

## Comandos básicos de validação

```bat
tasklist /FI "IMAGENAME eq Server.exe"
netstat -ano | findstr :9000
powershell -NoProfile -Command "Test-NetConnection 127.0.0.1 -Port 9000"
```

```bash
grep -RInE "127\\.0\\.0\\.1|localhost|9000|connect\\s*\\(|socket\\s*\\(|inet_addr|gethostbyname|getaddrinfo" SrcGame
```

```bash
grep -RInE "ServerIP|ServerPort|LoginServer|GameServer|AreaServer|WorldServer" SrcGame
```
BLOCK

write_block "docs/04_SERVER.md" "server-network" "Servidor" <<'BLOCK'
## Rede do servidor

### Porta de escuta

- Protocolo: TCP
- Porta: `9000`
- Bind esperado: `0.0.0.0:9000`

```bat
netstat -ano | findstr :9000
```

Resultado esperado:

```text
0.0.0.0:9000 LISTENING
```

### Erro Winsock 10048

O erro `10048` indica que o endereço ou a porta já está em uso.

```bat
tasklist /FI "IMAGENAME eq Server.exe"
netstat -ano | findstr :9000
taskkill /IM Server.exe /F
```

### Instrumentação recomendada

Adicionar logs em `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, dispatch do opcode, `send()` e `closesocket()`.
BLOCK

write_block "docs/05_CLIENT.md" "client-network" "Cliente" <<'BLOCK'
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
BLOCK

write_block "docs/08_TROUBLESHOOTING.md" "client-server-troubleshooting" "Troubleshooting" <<'BLOCK'
## Cliente não conecta ao servidor

```bat
tasklist /FI "IMAGENAME eq Server.exe"
netstat -ano | findstr :9000
netsh advfirewall firewall show rule name=all | findstr /I 9000
```

```powershell
Test-NetConnection 127.0.0.1 -Port 9000
```

Verificar:

- IP e porta usados pelo cliente;
- retorno de `connect()` e `WSAGetLastError()`;
- execução de `accept()` no servidor;
- primeiro pacote enviado e recebido;
- opcode e tamanho;
- diferenças de `#pragma pack` e tamanhos de estruturas;
- encerramento prematuro do socket.
BLOCK

write_block "opcodes.md" "connection-opcodes" "Opcodes" <<'BLOCK'
## Handshake inicial

| Direção | Opcode decimal | Opcode hexadecimal | Estrutura | Tamanho | Estado |
|---|---:|---:|---|---:|---|
| Cliente → Servidor | Desconhecido | Desconhecido | Primeiro pacote | ? | Pendente |
| Servidor → Cliente | Desconhecido | Desconhecido | Resposta inicial | ? | Pendente |

## Login

| Direção | Opcode decimal | Opcode hexadecimal | Estrutura | Tamanho | Estado |
|---|---:|---:|---|---:|---|
| Cliente → Servidor | | | Conta/senha | | Pendente |
| Servidor → Cliente | | | Resultado do login | | Pendente |

## Informações obrigatórias por pacote

- valor decimal e hexadecimal;
- direção;
- tamanho;
- estrutura C/C++;
- alinhamento;
- função de envio;
- função de receção;
- resposta esperada.
BLOCK

backup_file "docs/09_CLIENT_SERVER_CONNECTION.md"
cat > "docs/09_CLIENT_SERVER_CONNECTION.md" <<'BLOCK'
# Conexão cliente-servidor

## Objetivo

Obter a primeira conexão TCP funcional entre o cliente SummitTale e o servidor compilado em MinGW.

## Ambiente

### Servidor

- Sistema operativo: Windows
- Executável: `Server.exe`
- Compilador: MinGW i686
- Subsistema: Windows Console
- IP: a confirmar
- Porta: `9000/TCP`
- SQL Server: conectado

### Cliente

- Sistema operativo: Windows
- Executável: a confirmar
- Compilador: a confirmar
- IP configurado: a confirmar
- Porta configurada: a confirmar

## Fluxo esperado

1. O cliente inicia.
2. O cliente resolve o endereço.
3. O cliente cria um socket.
4. O cliente executa `connect()`.
5. O servidor executa `accept()`.
6. O cliente envia o pacote inicial.
7. O servidor interpreta o opcode.
8. O servidor envia uma resposta.
9. O cliente prossegue para o login.

## Instrumentação do cliente

- `socket()`;
- resolução de endereço;
- `connect()`;
- `send()`;
- `recv()`;
- `shutdown()`;
- `closesocket()`.

## Instrumentação do servidor

- `bind()`;
- `listen()`;
- `accept()`;
- `recv()`;
- dispatch de pacotes;
- opcode desconhecido;
- `send()`;
- encerramento do socket.

## Critério de conclusão

- IP e porta usados pelo cliente;
- sucesso de `connect()`;
- execução de `accept()`;
- IP remoto aceite;
- primeiro pacote recebido;
- tamanho e opcode do pacote;
- resposta enviada pelo servidor.

## Registo de testes

### Teste 1

- Data:
- Build do cliente:
- Build do servidor:
- IP:
- Porta:
- Resultado no cliente:
- Resultado no servidor:
- Captura de rede:
- Conclusão:
BLOCK

echo "Criado/atualizado: docs/09_CLIENT_SERVER_CONNECTION.md"

write_block "README.md" "current-status-connection" "SummitTale" <<'BLOCK'
## Estado atual

O servidor SummitTale já:

- compila em MinGW i686;
- conecta ao SQL Server;
- carrega os sistemas principais;
- abre a porta TCP;
- permanece em execução;
- executa como aplicação de consola.

O trabalho atual está concentrado na conexão entre o cliente e o servidor.

Documentos principais:

- `MINGW_PORT_STATUS_2026_07.md`
- `docs/milestone_servidorcompilado_funcional.md`
- `docs/09_CLIENT_SERVER_CONNECTION.md`
- `investigando cliente e server não conectarem.md`
- `docs/04_SERVER.md`
- `docs/05_CLIENT.md`
- `opcodes.md`
BLOCK

echo
echo "Documentação atualizada com sucesso."
echo "Backups: $BACKUP_DIR"
echo
echo "Rever alterações com:"
echo "  git diff -- ."
