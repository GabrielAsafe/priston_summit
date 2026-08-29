# 043 — Chat background asset extension fix

## Status

**EXPERIMENTAL — aguarda build/runtime — NÃO COMMITADO**

## Sintoma

O fundo da janela de chat aparecia branco.

Em algumas situações, ao abrir outro menu, a janela ganhava cor/aparência
temporariamente, indicando que a UI estava sendo desenhada mas o asset
principal do fundo não estava sendo carregado corretamente.

## Arquivo alterado

`SourceCode/SrcGame/src/sinbaram/sinInterFace.cpp`

## Causa

A Auditoria 042 comparou CURRENT contra a BASE pré-regressão depois de
normalizar encoding e finais de linha.

Apesar do diff bruto do recovery parecer enorme (`+3433/-3433`), após
normalização restou somente um hunk semântico relevante.

Diferença encontrada:

CURRENT:

`image//Sinimage//Inter//Chat//ChatDefault-1.png`

BASE:

`image//Sinimage//Inter//Chat//ChatDefault-1.tga`

O asset real esperado pelo código/base usa formato TGA.

A referência `.png` fazia o cliente tentar carregar o arquivo/formato errado,
resultando no fundo branco do chat.

## Fix

Foi alterada somente uma string:

`ChatDefault-1.png`

para:

`ChatDefault-1.tga`

Nenhuma outra linha de `sinInterFace.cpp` foi restaurada da BASE/recovery.

## Motivo para não aplicar o diff inteiro

O diff bruto de milhares de linhas era majoritariamente ruído de representação,
encoding e/ou line endings.

A comparação normalizada provou que a diferença funcional relevante para esse
bug é apenas a extensão do asset.

## Validação necessária

1. Compilar somente o cliente.
2. Executar o cliente.
3. Abrir o jogo sem abrir menus auxiliares.
4. Verificar se o fundo do chat aparece corretamente desde o início.
5. Abrir/fechar outros menus para confirmar que o estado do chat permanece estável.
6. Não commitar antes da validação runtime.
