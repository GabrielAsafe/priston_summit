# 3. Contas, autenticação, GM e personagens

## 3.1 Conta e personagem são coisas diferentes

Uma **conta** é criada/gerida na SQLite.

Um **personagem** completo continua armazenado no `DataServer`.

Não tentar “criar personagem” apenas com SQL.

## 3.2 Distribuição AccountDB por inicial

As contas legadas vivem em tabelas por primeira letra:

```text
AccountDB__AGameUser
AccountDB__BGameUser
...
AccountDB__ZGameUser
```

O helper Python escolhe automaticamente:

```text
account "teste" -> AccountDB__TGameUser
```

A conta deve começar por `A-Z`.

## 3.3 Criar conta pelo helper Python

Ferramenta:

```text
SourceCode/tools/sqlite_local_admin.py
```

Exemplo:

```bash
python3 tools/sqlite_local_admin.py \
  --db "/home/gabriel/Área de Trabalho/summit/Servidor_R25_Wine/Database/summit.sqlite3" \
  create-account teste \
  --auth local
```

Sem `--password`, a senha é pedida duas vezes via `getpass`.

O helper:

- insere a conta na tabela `<Inicial>GameUser`;
- preenche campos obrigatórios;
- define `GPCode=""`;
- define `inuse="0"`;
- cria `LocalAuth__AccountPolicy` quando `--auth local` ou `legacy`.

## 3.4 Auth modes

`LocalAuth__AccountPolicy` suporta:

```text
local
legacy
```

A ausência da row equivale a modo `implicit`.

Comandos:

```bash
... set-auth teste local
... set-auth teste legacy
... set-auth teste implicit
```

### Interpretação

- `local`: SQLite é autoridade explícita da conta.
- `legacy`: provider antigo é autoridade.
- `implicit`: decisão derivada dos campos históricos.

Para novas contas do ambiente local, usar explicitamente:

```text
local
```

## 3.5 Sessão: inuse

A sessão local usa os campos históricos da conta:

```text
inuse
ServerName
SNo
StartDay
LastDay
EditDay
```

Login faz claim atômico.

Logout deve liberar o claim.

Foi corrigido um bug onde o logout só era enfileirado se:

```cpp
lpPlayInfo->szGPCode[0]
```

fosse verdadeiro.

Isso era incompatível com contas locais, pois novas contas usam `GPCode=""`.

A correção tornou `bSql_Logout(...)` incondicional no fluxo normal, e o worker decide local-first.

Sintoma clássico desse bug:

```text
primeiro login: OK
logout: aparente OK
segundo login: bloqueado
TGameUser.inuse = 1
```

## 3.6 GM

Tabela:

```text
AccountDB__GameMasters
```

Colunas:

```text
id INTEGER PRIMARY KEY AUTOINCREMENT
accountId TEXT NOT NULL
password TEXT NOT NULL
security INTEGER NOT NULL
active INTEGER NOT NULL
```

Níveis usados pelo helper:

```text
1 VIP
2 moderator
3 gamemaster
4 administrator
```

Criar/alterar GM:

```bash
python3 tools/sqlite_local_admin.py \
  --db ".../summit.sqlite3" \
  set-gm teste \
  --security 4
```

Remover:

```bash
... remove-gm teste
```

Consultar:

```bash
... show teste
```

### Nota de segurança

O sistema legado compara credenciais GM como texto simples. Não tratar esta tabela como armazenamento moderno de segredo seguro.

## 3.7 `/ativar`

O comando `/ativar <senha>` usa a credencial de `GameMasters` para colocar a sessão em modo administrativo.

Ter `security=4` na tabela não significa necessariamente que todos os comandos ficam ativos sem passar pelo fluxo de autenticação esperado pelo parser/client.

## 3.8 Criar personagem corretamente

Fluxo correto:

1. criar conta SQLite;
2. iniciar servidor;
3. fazer login no cliente;
4. criar personagem pelo cliente;
5. servidor atualiza `DataServer/userinfo`;
6. servidor cria inicialmente um placeholder `userdata`;
7. cliente envia o record completo;
8. servidor valida e persiste o save real.

### Ficheiros

Account slots:

```text
DataServer/userinfo/<hash>/<account>.dat
```

Personagem:

```text
DataServer/userdata/<hash>/<char>.dat
```

Backup:

```text
DataServer/userdata_backup/<hash>/<char>.dat
```

O `<hash>` é produzido por `GetUserCode()` no código legado.

## 3.9 Placeholder de personagem

`InsertCharData()`:

- adiciona o nome ao `userinfo`;
- cria `TRANS_RECORD_DATA` zerado;
- escreve `srRECORD_MEMORY_SIZE` como placeholder.

Esse placeholder **não é um save válido completo**.

Posteriormente `RecvRecordDataFromClient()` deve receber `OPCODE_RECORDDATA`, remontar o record e substituir/cachear o estado válido.

Não “corrigir” o sistema removendo a validação que apaga slot quebrado. O correto é garantir que o record completo seja enviado e salvo.

## 3.10 Não criar personagem via Python SQLite

`sqlite_local_admin.py create-account` cria **conta**, não personagem.

Se for necessário automatizar personagem no futuro, há duas opções seguras:

- automatizar o protocolo/fluxo de criação do cliente;
- criar uma ferramenta específica que gere `userinfo` + `userdata` no formato binário correto, com testes contra o record C++.

Não inserir uma row em alguma tabela SQLite esperando que o personagem apareça.
