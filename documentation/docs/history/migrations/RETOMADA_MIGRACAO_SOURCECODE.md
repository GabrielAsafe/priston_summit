# Retomada da migração da SourceCode

**Estado consolidado em 31/08/2026**

## 1. Objetivo
Organizar a SourceCode radioativa por sistemas, migrando uma pasta de cada vez, sem alterar comportamento. O cliente será concluído antes de começar o servidor.

## 2. Fonte de verdade e diretórios
- Source de referência: `out(8).zip`.
- Git root: `/home/gabriel/Área de Trabalho/summit`.
- Source ativa: `/home/gabriel/Área de Trabalho/summit/SourceCode`.
- Pacotes de migração: `/home/gabriel/Área de Trabalho/summit/migration`.
- Build/runtime do cliente é a validação primária durante a fase de cliente.

## 3. Regras obrigatórias
1. Uma pasta por migração.
2. Cliente inteiro primeiro; servidor depois.
3. Auditar e atualizar os scripts de compilação no mesmo lote sempre que houver `-I`, filtros ou paths hardcoded para o diretório antigo.
4. Build/link somente do cliente nas migrações de cliente, salvo mudança realmente compartilhada.
5. Depois do build bem-sucedido, executar `send_to_client.sh` e testar runtime.
6. Só depois do runtime aprovado criar o commit.
7. O commit deve conter **somente os arquivos alterados pela migração**. Nunca usar `git commit` genérico com stage externo.
8. `stage.sh`/`commit.sh` devem trabalhar por pathspec; o padrão seguro é `git commit --only -- <paths da migração>`.
9. Auditorias como `git diff --check` devem rodar somente nos paths da migração. Problemas externos não podem bloquear o lote.
10. Se falhar, rollback somente do lote; não limpar trabalhos externos.
11. Não inferir estrutura de outras versões; usar somente o que existe na source atual e nos scripts atuais.

## 4. O que já foi feito

### Migration 001 — client Dump -> diagnostics/crash
- `SrcGame/src/Dump/WinDump.cpp` -> `SrcGame/src/diagnostics/crash/WinDump.cpp`
- `SrcGame/src/Dump/WinDump.h` -> `SrcGame/src/diagnostics/crash/WinDump.h`
- Include em `SrcGame/src/Winmain.cpp` atualizado.
- Build/link e runtime aprovados.
- A 002 observou `2559b728fe4a03516aba1c9765e859fd727002a2` como HEAD/base após a 001; tratar esse hash como checkpoint observado da 001.

### Migration 002 — client Login -> systems/authentication
- `SrcGame/src/Login/Login.cpp` -> `SrcGame/src/systems/authentication/Login.cpp`
- `SrcGame/src/Login/Login.h` -> `SrcGame/src/systems/authentication/Login.h`
- Includes corrigidos em `HoBaram/HoOpening.cpp`, `HoBaram/HoOpeningOFF.cpp` e `Server/OnSever.cpp`.
- Build/runtime aprovados antes do fechamento do commit.
- Houve um commit incorreto (`26c1e565...`) que carregou milhares de arquivos externos; ele não deve ser usado como checkpoint de migração.
- O histórico foi corrigido. Commit válido da 002: `cce19441` (`refactor(client): move login into authentication system`), contendo exatamente 5 arquivos.

### Migration 003 — client Security -> systems/security/client_integrity
Destino:
- `SrcGame/src/Security/Security.cpp` -> `SrcGame/src/systems/security/client_integrity/Security.cpp`
- `SrcGame/src/Security/Security.h` -> `SrcGame/src/systems/security/client_integrity/Security.h`
- `systems/authentication/Login.cpp` passa a incluir `systems/security/client_integrity/Security.h`.
- O `-ISrcGame/src/Security` legado deve ser removido dos scripts de compilação ativos; `-ISrcGame/src` é suficiente para o include canônico.

**Evidência funcional já obtida:** em uma execução anterior, a 003 compilou/linkou o cliente, enviou os binários e o jogo funcionou.

**Estado atual:** depois de corrigir o histórico da 002, a 003 foi reaplicada e abortou por causa de um `git diff --check` global encontrar whitespace em `HoBaram/HoSkyOld.cpp`, arquivo que não pertence ao lote. Portanto a 003 está aplicada/parcialmente aplicada na working tree, mas ainda não é checkpoint Git atual. O pacote corrigido é resumível e limita auditorias aos paths da migração.

## 5. Ponto exato para retomar
1. Sobrescrever/extrair o pacote corrigido da 003 na pasta de migrations.
2. Executar novamente o `install.sh`; ele deve detectar o estado já aplicado e retomar audit/build em vez de reaplicar arquivos.
3. Confirmar build/link do cliente.
4. Confirmar `send_to_client.sh`.
5. Testar o jogo.
6. Se aprovado, fechar **somente** a 003 usando o `commit.sh` do pacote.
7. Verificar o commit com `git show --stat --oneline HEAD`.
8. Só então criar a Migration 004.

Comando de retomada:
```bash
cd "/home/gabriel/Área de Trabalho/summit/migration" && \
unzip -o migration_003_client_security_to_client_integrity.zip && \
./migration_003_client_security_to_client_integrity/install.sh \
  "/home/gabriel/Área de Trabalho/summit/SourceCode"
```

Depois do teste runtime:
```bash
cd "/home/gabriel/Área de Trabalho/summit/migration" && \
./migration_003_client_security_to_client_integrity/commit.sh \
  "/home/gabriel/Área de Trabalho/summit/SourceCode"
```

## 6. Armadilhas já identificadas
- O repositório Git fica em `summit`, não em `SourceCode`.
- Working tree suja é possível e não é motivo por si só para bloquear uma migração.
- Stage externo é perigoso: foi a causa do commit gigante da 002.
- Caminhos absolutos do ambiente de geração (`/mnt/data/...`) não podem aparecer nos pacotes; scripts devem usar `SCRIPT_DIR`.
- Não exigir hash do arquivo inteiro quando a migração só precisa alterar um include em um arquivo que pode conter bugfixes paralelos; validar invariantes/hunks do lote.
- Não executar `git diff --check` global: isso já bloqueou a 003 por whitespace preexistente em `HoSkyOld.cpp`.
- Migração de cliente não deve compilar servidor automaticamente.
- A pasta antiga deve desaparecer quando ficar vazia.

## 7. Critério para considerar uma migração concluída
Uma migração só termina quando: estrutura nova existe; caminho antigo desapareceu; includes antigos do sistema não restam; build script foi auditado/atualizado; cliente compilou/linkou; envio ocorreu; runtime passou; commit contém somente os arquivos do lote.

## 8. Próximo passo após a 003
Não iniciar a 004 antes de a 003 estar revalidada e commitada isoladamente. Depois disso, escolher a próxima pasta pequena do cliente por acoplamento real, auditar consumidores e `-I`, e repetir o mesmo protocolo.
