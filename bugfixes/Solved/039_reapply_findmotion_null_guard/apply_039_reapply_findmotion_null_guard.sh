#!/usr/bin/env bash

ROOT="/home/gabriel/Área de Trabalho/summit"
REPO="$ROOT"
CUR="$ROOT/SourceCode"

REC_PATCH="$ROOT/bugfixes/034_recovery_vs_current/per_file/033__SourceCode__SrcServer__src__character.cpp.patch"
BUGFIX="$ROOT/bugfixes/039_reapply_findmotion_null_guard"
CHARACTER="$CUR/SrcServer/src/character.cpp"
FILEREAD="$CUR/SrcServer/src/fileread.cpp"

mkdir -p "$BUGFIX/before" "$BUGFIX/after"

EXPECTED_CHARACTER_BEFORE="44a122a1ff5b14934e622a3879e0ccb40ad135184c223d4efd1e34903cdfa365"
EXPECTED_CHARACTER_AFTER="fb066ac583c24584a758a13e0e8640204944e62e195a8d2b32337a5b347aad6e"
EXPECTED_FILEREAD_037="3a9ca7501d29f0afb5172cf125b363120c27120aaf05d559a8a08aadb5088e34"

echo "=== 039 — REAPPLY RECOVERY FindMotionCountFromCode NULL GUARD ==="
echo

HEAD="$(git -C "$REPO" rev-parse HEAD 2>/dev/null)"
printf '%s\n' "$HEAD" > "$BUGFIX/base_commit.txt"

CHAR_SHA="$(sha256sum "$CHARACTER" | awk '{print $1}')"
FILE_SHA="$(sha256sum "$FILEREAD" | awk '{print $1}')"

echo "HEAD=$HEAD"
echo "character.cpp=$CHAR_SHA"
echo "fileread.cpp=$FILE_SHA"
echo

if [ "$CHAR_SHA" != "$EXPECTED_CHARACTER_BEFORE" ]; then
    echo "ERRO: character.cpp não está no estado esperado antes da 035."
    echo "Esperado: $EXPECTED_CHARACTER_BEFORE"
    echo "Atual:    $CHAR_SHA"
else
    if [ "$FILE_SHA" != "$EXPECTED_FILEREAD_037" ]; then
        echo "ERRO: fileread.cpp não está no estado 037 esperado."
        echo "Esperado: $EXPECTED_FILEREAD_037"
        echo "Atual:    $FILE_SHA"
    else
        if [ ! -f "$REC_PATCH" ]; then
            echo "ERRO: patch recovery da 035 não encontrado:"
            echo "$REC_PATCH"
        else
            cp -a "$CHARACTER" "$BUGFIX/before/character.cpp"

            (
                cd "$BUGFIX/before" &&
                sha256sum character.cpp
            ) > "$BUGFIX/hashes_before.sha256"

            cp -a "$REC_PATCH" "$BUGFIX/change.patch"

            cat > "$BUGFIX/crash_evidence.txt" <<'EOF'
Crash observado com Fix 036 + experimental 037 e sem a 035:

Exception:
- code = 0xC0000005
- operation = READ
- invalid address = 0x0001598C
- crash RVA = 0x00011DCE

Simbolização:
- preferred VA = 0x00411DCE
- função = smCHAR::FindMotionCountFromCode(unsigned long)

Disassembly crítico:

00411d84 <smCHAR::FindMotionCountFromCode(unsigned long)>:
...
411dc5: mov this -> eax
411dc8: mov 0x4704(eax), eax
411dce: mov 0x1598c(eax), eax

No crash:
- EAX = 0x00000000

Logo:
- this->smMotionInfo, localizado em this+0x4704, é NULL;
- a instrução seguinte tenta ler smMotionInfo+0x1598c;
- NULL + 0x1598c = 0x0001598C;
- esse valor é exatamente o endereço inválido registrado pela exceção.

Conclusão:
o guard recuperado na 035, que testa `if(!smMotionInfo)` antes do loop,
protege exatamente a instrução que causou o crash reativado pela 037.
EOF

            cat > "$BUGFIX/README.md" <<'EOF'
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
EOF

            echo "=== APPLY CHECK ==="

            if git -C "$REPO" apply --check --recount "$REC_PATCH"; then
                echo "Patch recovery da 035 aplica limpo."

                if git -C "$REPO" apply --recount "$REC_PATCH"; then
                    AFTER_SHA="$(sha256sum "$CHARACTER" | awk '{print $1}')"

                    if [ "$AFTER_SHA" != "$EXPECTED_CHARACTER_AFTER" ]; then
                        echo "ERRO: hash AFTER inesperado."
                        echo "Esperado: $EXPECTED_CHARACTER_AFTER"
                        echo "Atual:    $AFTER_SHA"
                    else
                        cp -a "$CHARACTER" "$BUGFIX/after/character.cpp"

                        (
                            cd "$BUGFIX/after" &&
                            sha256sum character.cpp
                        ) > "$BUGFIX/hashes_after.sha256"

                        diff -u \
                            "$BUGFIX/before/character.cpp" \
                            "$BUGFIX/after/character.cpp" \
                            > "$BUGFIX/before_after.diff" 2>/dev/null || true

                        git -C "$REPO" diff -- "SourceCode/SrcServer/src/character.cpp" \
                            > "$BUGFIX/source_diff.txt"

                        git -C "$REPO" diff --check -- "SourceCode/SrcServer/src/character.cpp" \
                            > "$BUGFIX/diff_check.txt" 2>&1

                        git -C "$REPO" status --short > "$BUGFIX/status_after_apply.txt"

                        echo
                        echo "=== RESULT ==="
                        cat "$BUGFIX/hashes_before.sha256"
                        cat "$BUGFIX/hashes_after.sha256"
                        echo
                        echo "039 aplicada e NÃO commitada."
                        echo "Mantida a 037 ativa."
                        echo "Compile somente o servidor e faça o teste runtime."
                    fi
                else
                    echo "ERRO: aplicação falhou."
                fi
            else
                echo "ERRO: patch não aplica limpo. Nenhuma alteração feita."
            fi
        fi
    fi
fi
