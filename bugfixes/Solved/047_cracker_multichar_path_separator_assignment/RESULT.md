# BUGFIX 047 — Cracker multichar path separator assignment

## Causa

O código usava o literal multicaractere:

    '//'

em uma atribuição para `char`:

    szBuff[len++] = '//';

Em C/C++, `'//'` é uma constante multicaractere com valor maior que um `char`.
No compilador testado, o valor era truncado para `'/'`, funcionando apenas por
conversão implícita e gerando warning.

## Correção

As duas ocorrências, cliente e servidor, foram substituídas por um separador
Windows explícito:

    szBuff[len++] = '\\';

## Arquivos

- SourceCode/SrcGame/src/cracker.cpp
- SourceCode/SrcServer/src/cracker.cpp

## Validação

- duas ocorrências antes da correção;
- nenhuma ocorrência remanescente de `'//'` após os bugfixes relacionados;
- build concluído com sucesso;
- cliente/servidor testados;
- nenhuma regressão observada no teste realizado.

## Escopo

Este bugfix altera apenas a construção do caminho para `wsock32.dll`.

A lógica de detecção de módulos e envio de aviso permanece inalterada.
