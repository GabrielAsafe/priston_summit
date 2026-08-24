# 041 — Monster spawn/loading recovery fix

## Status

**RESOLVIDO E VALIDADO EM RUNTIME — 2026-08-24**

Resultado observado:

**os mobs voltaram ao jogo e estão disponíveis para combate.**

---

## Base necessária

Este fix parte de:

`22f79f1 fix(server): restore CP949 character definition tokens`

O Fix 036 restaurou os tokens CP949 completos de `smCharDecode()` e
corrigiu os NPCs repetindo o mesmo modelo.

Esse fix é pré-requisito.

---

## Sintoma

Depois de corrigir `smCharDecode()`:

- NPCs ficaram corretos;
- nomes/modelos das definições voltaram;
- monstros ainda não apareciam.

---

## Arquivo 1 — `SrcServer/src/fileread.cpp`

### Função

`DecodeOpenMonster()`

### Problema

Os arquivos `.spm` usam tokens legados CP949.

O CURRENT ainda não possuía o conjunto pré-reset completo dessa função.

### Recuperado

Todos os 4 hunks de `DecodeOpenMonster()` presentes no recovery foram
reaplicados juntos.

Tokens restaurados:

- `ACTOR`
- `BOSS_ACTOR`
- `MAX_ACTOR_POS`
- `DELAY`
- `MAX_ACTOR`

Também foi restaurado:

`MINGW_SPM_ACTOR_MATCH_TRACE_V1`

### Evidência

`spm_actor_trace.txt` passou a registrar ACTORs com `match >= 0`.

Isso provou:

`.spm`
→ `DecodeOpenMonster()`
→ ACTOR
→ `lpCharInfoList[].szName`
→ correspondência válida.

Logo, o parser de spawn voltou a alcançar a lista de monstros.

---

## Crash revelado após reativar o fluxo

A restauração de `DecodeOpenMonster()` fez o servidor avançar para um
caminho que antes não estava sendo executado e revelou um crash
determinístico.

Exceção:

- `0xC0000005`
- READ
- endereço `0x0001598C`
- RVA `0x00011DCE`

A Auditoria 038 simbolizou:

`0x00411DCE`
→
`smCHAR::FindMotionCountFromCode(unsigned long)`

Disassembly relevante:

```text
411dc8: mov 0x4704(%eax), %eax
411dce: mov 0x1598c(%eax), %eax
```

No crash:

`EAX = 0`

Portanto o valor obtido de `this+0x4704` (`smMotionInfo`) era NULL.

A instrução seguinte tentou acessar:

`NULL + 0x1598C`

que corresponde exatamente ao endereço inválido registrado.

---

## Arquivo 2 — `SrcServer/src/character.cpp`

### Função

`smCHAR::FindMotionCountFromCode(DWORD MotionCode)`

### Recuperado

O recovery pré-reset já continha exatamente a proteção necessária:

```cpp
if(!smMotionInfo)
{
    ...
    return 0;
}
```

Também existe trace para:

`motion_null_trace.txt`

### Resultado

Com `DecodeOpenMonster()` restaurado **e** o guard reaplicado:

- o crash em `FindMotionCountFromCode()` deixou de interromper o fluxo;
- os monstros voltaram a aparecer;
- os mobs ficaram jogáveis.

---

## Por que as duas alterações são um único fix funcional

A sequência experimental demonstrou dependência:

### Sem `DecodeOpenMonster()` restaurado

O fluxo dos monstros não chegava corretamente à correspondência ACTOR.

### Com `DecodeOpenMonster()` e sem guard

ACTOR passou a funcionar, mas o servidor caiu em
`FindMotionCountFromCode()` por `smMotionInfo == NULL`.

### Com `DecodeOpenMonster()` + guard

O carregamento avançou e os mobs voltaram ao jogo.

Portanto não são duas hipóteses independentes empilhadas arbitrariamente.
São duas partes do estado pré-reset necessárias para restaurar o mesmo
caminho funcional de carregamento/spawn.

---

## Arquivos de source alterados

1. `SourceCode/SrcServer/src/fileread.cpp`
   - `DecodeOpenMonster()`

2. `SourceCode/SrcServer/src/character.cpp`
   - `smCHAR::FindMotionCountFromCode()`

Nenhum arquivo do cliente foi alterado.

Nenhum código de item/drop foi alterado.

---

## Fonte

As duas alterações vieram de:

`/home/gabriel/Transferências/recovery_pre_reset_20260824_0958/working_tree_vs_HEAD.patch`

Nenhum código foi portado do armazém histórico.

---

## Resultado final

### NPCs

Continuam corrigidos pelo Fix 036.

### Monstros

**RESOLVIDO.**

Mobs presentes em runtime e disponíveis para combate.

### Próxima investigação

Cliente/UI: chat com imagem/fundo branco.

Essa investigação deve ser feita separadamente e não deve alterar este
estado funcional do servidor.
