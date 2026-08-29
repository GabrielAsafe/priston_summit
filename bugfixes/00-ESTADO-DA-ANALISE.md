# Estado da análise estática

## Escopo

- Arquivos C/C++/headers no ZIP: **1204**
- Inventário CSV: **11.383 entradas**
- Pares candidatos `new[]`/`delete`: **75**
- Ocorrências de `'/0'` no CLIENT: **47**
- Ocorrências de `'/0'` no SERVER: **0**

## Situação

Esta pasta contém a primeira bateria de achados confirmados/fortemente confirmados.

A análise é estática. O projeto é Windows/DirectX legado e não foi executado integralmente neste ambiente. Por isso, cada relatório separa evidência de código de hipóteses de exploração/runtime.

## Priorização sugerida

1. BUG-001 e BUG-002 — fronteira de rede/código dinâmico.
2. BUG-003 a BUG-006 — `CurseFilter`.
3. BUG-011 — `sinSTRING`.
4. BUG-008 e BUG-009 — corrupção de memória no loader de animação.
5. BUG-010 — saneamento sistêmico de ownership.
6. Demais erros de parsing/undefined behavior.
