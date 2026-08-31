# Migration 001 — Client Dump -> diagnostics/crash

## Objetivo

Migrar somente a pasta de crash dump do cliente:

```text
SrcGame/src/Dump/
->
SrcGame/src/diagnostics/crash/
```

Nenhuma lógica funcional de `WinDump.cpp` ou `WinDump.h` é alterada neste lote.

## Escopo

Arquivos afetados:

```text
R SrcGame/src/Dump/WinDump.cpp -> SrcGame/src/diagnostics/crash/WinDump.cpp
R SrcGame/src/Dump/WinDump.h   -> SrcGame/src/diagnostics/crash/WinDump.h
M SrcGame/src/Winmain.cpp
M SrcGame/src/priston.vcxproj
```

Mudanças textuais:

```text
#include "Dump/WinDump.h"
->
#include "diagnostics/crash/WinDump.h"
```

E atualização das duas entradas equivalentes em `priston.vcxproj`.

## Commit-base

`346137322c2635ce94b70aa849f23832511ba5ba`

## Evidência / análise

Antes da migração, as únicas referências ativas do cliente encontradas para `Dump/WinDump` eram:

```text
SrcGame/src/Winmain.cpp
SrcGame/src/priston.vcxproj (ClCompile)
SrcGame/src/priston.vcxproj (ClInclude)
```

Arquivos de backup não fazem parte do lote.

Depois da migração, a auditoria da árvore ativa preparada encontrou zero referências ativas a `Dump/WinDump`.

## Integridade

`WinDump.cpp` e `WinDump.h` foram movidos byte por byte, sem alterações internas.

Ver:

```text
hashes_before.sha256
hashes_after.sha256
```

Os hashes do conteúdo dos arquivos movidos são idênticos antes/depois.

O patch foi aplicado em uma árvore Git temporária construída a partir dos snapshots BEFORE e produziu exatamente os snapshots AFTER.

Validação local do patch:

```text
git apply --check: OK
git diff --cached --check (CRLF-aware): OK
rename WinDump.cpp: 100%
rename WinDump.h: 100%
```

## Como aplicar

Extraia sempre dentro de:

```text
/home/gabriel/Área de Trabalho/summit/migration/
```

Fluxo padrão deste e dos próximos lotes:

```bash
cd "/home/gabriel/Área de Trabalho/summit/migration" && \
unzip -o migration_001_client_dump_to_diagnostics_crash.zip && \
./migration_001_client_dump_to_diagnostics_crash/install.sh \
  "/home/gabriel/Área de Trabalho/summit/SourceCode"
```

O `install.sh` valida commit-base, working tree limpa, SHA-256 do patch, `git apply --check`, aplica o lote, audita o caminho legado e executa build/link de cliente e servidor.

Depois do teste de runtime:

```bash
cd "/home/gabriel/Área de Trabalho/summit/migration" && \
./migration_001_client_dump_to_diagnostics_crash/audit.sh \
  "/home/gabriel/Área de Trabalho/summit/SourceCode"
```

Em caso de falha ou reprovação no runtime:

```bash
cd "/home/gabriel/Área de Trabalho/summit/migration" && \
./migration_001_client_dump_to_diagnostics_crash/rollback.sh \
  "/home/gabriel/Área de Trabalho/summit/SourceCode"
```

## Build obrigatório

Cliente:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
sh cliente.sh
```

Servidor:

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
sh server.sh
```

## Teste no jogo

Mínimo recomendado para este lote:

- cliente inicia normalmente;
- login e entrada no mapa funcionam;
- servidor inicia normalmente;
- não há regressão observável de startup;
- não alterar/remover a instrumentação de crash existente.

Não é necessário provocar um crash proposital apenas para aprovar a reorganização do diretório.

## Critério de aprovação

Somente marcar `APROVADO` quando:

1. `git diff --check` estiver aceitável para a configuração do repositório;
2. cliente compilar e linkar sem erros;
3. servidor compilar e linkar sem erros;
4. teste básico no jogo passar;
5. diff continuar limitado a este lote.

## Estado

`PENDENTE DE BUILD/TESTE NO REPOSITÓRIO ATIVO`

## Working tree suja (regra operacional)

Esta migração foi preparada para coexistir com bugfixes não commitados.

- `install.sh` **não exige working tree limpa**;
- valida SHA-256 somente dos quatro ficheiros tocados pelo lote antes de aplicar;
- ignora deliberadamente `ccache/`, binários de `dist_*` e outros fontes modificados;
- falha se algum ficheiro deste lote não corresponder exatamente ao BEFORE conhecido;
- falha se já houver alterações staged nos ficheiros do lote;
- `stage.sh` aplica somente `change.patch` ao index, preservando alterações anteriores de `Winmain.cpp` como unstaged;
- `rollback.sh` reverte somente o delta desta migração.

Isso é necessário porque `Winmain.cpp` já contém trabalho de bugfix anterior ao lote 001.
