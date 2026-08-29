# 039 — Reaplicar guard de `smMotionInfo` após reativar monster loading

## Status

**EXPERIMENTAL — AGUARDA BUILD/RUNTIME — NÃO COMMITAR**

## Base funcional conhecida

Fix 036:

`22f79f1 fix(server): restore CP949 character definition tokens`

Resultado runtime:
- NPCs corrigidos;
- modelos repetidos resolvidos.

## Estado experimental anterior

A 037 reintroduziu todos os hunks recovery de `DecodeOpenMonster()`:

- ACTOR;
- BOSS_ACTOR;
- MAX_ACTOR_POS;
- DELAY;
- MAX_ACTOR;
- trace de ACTOR.

Resultado:

`spm_actor_trace.txt` mostrou ACTOR sendo reconhecido com `match >= 0`
e `listCnt=374`.

Isso provou que o parser `.spm` voltou a preencher/processar a cadeia de monstros.

Em seguida o servidor crashou.

## Diagnóstico do crash — Auditoria 038

O crash foi determinístico em:

`RVA 0x00011DCE`

Com ImageBase preferencial `0x00400000`:

`VA 0x00411DCE`

Símbolo:

`smCHAR::FindMotionCountFromCode(unsigned long)`

A instrução fatal:

`mov 0x1598c(%eax), %eax`

Imediatamente antes:

`mov 0x4704(%this), %eax`

No dump do crash:

`EAX=0x00000000`

O acesso inválido registrado:

`0x0001598C`

Portanto a falha é exatamente:

`smMotionInfo == NULL`

seguido de:

`smMotionInfo->MotionCount`

ou campo equivalente no offset `0x1598c`.

## Relação com a antiga 035

O recovery pré-reset já continha uma alteração exatamente nessa função:

```cpp
if(!smMotionInfo)
{
    ...
    return 0;
}
```

Ela também grava:

`motion_null_trace.txt`

A 035 havia sido testada antes de o caminho de monstros estar plenamente reativado e,
naquele momento, foi considerada inconclusiva.

Antes da 037 ela foi corretamente removida para manter o experimento isolado.

A 038 agora fornece evidência direta de que a 035 é uma dependência real do caminho
reativado pela 037.

## Hipótese da 039

Manter a 037 e reaplicar exatamente o guard pré-reset da 035 deve impedir a
dereferência nula em `FindMotionCountFromCode()` e permitir que o carregamento
de monstros prossiga além do ponto atual.

## Escopo

Somente:

`SourceCode/SrcServer/src/character.cpp`

Nenhuma nova lógica inventada.

A alteração é reaplicada do patch recovery pré-reset já identificado na 035.

`fileread.cpp` permanece exatamente no estado experimental 037.

## Validação necessária

1. Compilar apenas o servidor.
2. No runtime Windows, apagar/renomear o `motion_null_trace.txt` antigo antes do teste.
3. Iniciar o servidor.
4. Verificar:
   - se ainda crasha;
   - se `motion_null_trace.txt` é recriado;
   - quais name/model aparecem no novo trace;
   - se os monstros aparecem;
   - se NPCs continuam corretos.
5. Não commitar antes da validação runtime.
