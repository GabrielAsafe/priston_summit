# Fix 014 - Disable client startup debug MessageBoxes

## Estado

APROVADO.

## Problema

Instrumentações adicionadas durante o port MinGW exibiam MessageBox
durante a inicialização normal do cliente.

Também permanecia ativa a janela:

    Error loading imagem

que não deve mais interromper a inicialização.

## Alteração

Foram comentadas somente oito chamadas:

### Winmain.cpp

- Error loading imagem
- ENTROU NO WINMAIN
- AFTER ANTICHEAT
- AFTER READCONFIG
- AFTER DIRECTSOUND
- AFTER REGISTERCLASS
- BEFORE CREATEWINDOWEX

### mingw_debug_entry.cpp

- ENTROU NO ENTRYPOINT DEBUG

As linhas foram preservadas no source como comentários marcados com:

    BUGFIX_014_STARTUP_DEBUG_BOX_DISABLED

## Fora do escopo

Não foram alteradas outras MessageBox.

Em particular permanecem ativas para investigação futura:

- D3D CREATEDEVICE DEBUG
- CreateWindow/CreateWindowEx failed
- erros de socket
- erros Lua
- demais mensagens funcionais do cliente

## Evidências

- before/
- after/
- hashes_before.sha256
- hashes_after.sha256
- change.patch
- winmain_before_after.diff
- entry_before_after.diff

## Teste

Executar:

    cd SourceCode
    sh cliente.sh

Confirmar:

1. cliente abre;
2. as sete janelas DEBUG MINGW acima não aparecem;
3. Error loading imagem não aparece;
4. login funciona;
5. seleção funciona;
6. personagem entra no mapa;
7. NPCs continuam presentes.

## Resultado

APROVADO.

Teste funcional realizado em 2026-08-24.

Resultado:

- cliente iniciou normalmente;
- ENTROU NO WINMAIN não apareceu;
- AFTER ANTICHEAT não apareceu;
- AFTER READCONFIG não apareceu;
- AFTER DIRECTSOUND não apareceu;
- AFTER REGISTERCLASS não apareceu;
- BEFORE CREATEWINDOWEX não apareceu;
- ENTROU NO ENTRYPOINT DEBUG não apareceu;
- Error loading imagem não apareceu;
- login continuou funcionando;
- seleção de personagem continuou funcionando;
- entrada no mapa continuou funcionando;
- nenhuma regressão funcional foi observada.

Conclusão:

Fix 014 aprovado.

