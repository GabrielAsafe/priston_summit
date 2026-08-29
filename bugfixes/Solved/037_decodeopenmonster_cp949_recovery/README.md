# 037 — DecodeOpenMonster CP949 recovery

## Status

**EXPERIMENTAL — NÃO COMMITADO**

Aguarda build do servidor e validação runtime.

## Base

O Fix 036 foi validado em runtime e commitado:

`22f79f1 fix(server): restore CP949 character definition tokens`

O Fix 036 resolveu o problema dos NPCs repetindo o mesmo modelo.

Monstros continuam sem aparecer.

## Objetivo

Reintroduzir exatamente as alterações pré-reset existentes em:

`DecodeOpenMonster()`

Essa função faz parte do carregamento dos arquivos de spawn `.spm`.

## Fonte

Patch recuperado do working tree pré-reset:

`recovery_pre_reset_20260824_0958/working_tree_vs_HEAD.patch`

O armazém histórico não é fonte deste patch.

## Por que DecodeOpenMonster é o próximo alvo

O fluxo reconstruído é:

`char/*.ini / *.inf`
→ `smCharDecode()`
→ `lpCharInfoList / chrMonsterList`
→ arquivo `.spm`
→ `DecodeOpenMonster()`
→ ACTOR `"nome"`
→ comparação com `lpCharInfoList[].szName`
→ `rsMonster[]`
→ `MonCnt`
→ spawn.

Após o Fix 036, NAME e os demais campos de `smCharDecode()` voltaram a ser reconhecidos corretamente.

Se `DecodeOpenMonster()` continuar com tokens coreanos corrompidos, o `.spm` pode não reconhecer ACTOR e os demais comandos, impedindo a criação da lista de spawn mesmo com `smCharDecode()` corrigido.

## Conjunto recovery reaplicado

Todos os 4 hunks de `DecodeOpenMonster()` presentes no recovery são aplicados juntos.

Tokens esperados e validados:

- ACTOR
- BOSS_ACTOR
- MAX_ACTOR_POS
- DELAY
- MAX_ACTOR

Também é preservada, caso exista no recovery, a instrumentação pré-reset:

`MINGW_SPM_ACTOR_MATCH_TRACE_V1`

Essa instrumentação pode gerar:

`spm_actor_trace.txt`

Ela ajuda a distinguir:

1. ACTOR não reconhecido;
2. ACTOR reconhecido mas nome sem correspondência;
3. ACTOR reconhecido e encontrado corretamente.

## Experimento 035

Antes deste teste, `character.cpp` é restaurado ao estado anterior ao experimento 035.

A 035 não resolveu o problema dos NPCs e não apresentou evidência de `smMotionInfo == NULL`.

Isso deixa o teste 037 com apenas duas mudanças funcionais relevantes em relação à base commitada:

- Fix 036, já validado e commitado;
- DecodeOpenMonster recovery, ainda experimental.

## O que observar no runtime

1. Tempo de inicialização do servidor.
2. Se a fase de carregamento de personagens/monstros volta a demorar como no servidor funcional anterior.
3. Se os monstros aparecem nos mapas.
4. Se os NPCs continuam corretos.
5. Se aparece `spm_actor_trace.txt`.
6. Em caso de crash, preservar os arquivos de crash e o trace.

## Regra

**Não commitar a 037 antes do teste runtime.**

Se resolver os monstros, será documentada e commitada separadamente.
Se causar regressão, deve ser revertida usando `before/fileread.cpp`.
