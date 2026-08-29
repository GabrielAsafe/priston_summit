#!/usr/bin/env bash

ROOT="/home/gabriel/Área de Trabalho/summit"
REPO="$ROOT"
CUR="$ROOT/SourceCode"

FIX037="$ROOT/bugfixes/037_decodeopenmonster_cp949_recovery"
FIX038="$ROOT/bugfixes/038_monster_crash_symbolization"
FIX039="$ROOT/bugfixes/039_reapply_findmotion_null_guard"

BUGFIX="$ROOT/bugfixes/041_monster_spawn_recovery_fix"

FILEREAD="$CUR/SrcServer/src/fileread.cpp"
CHARACTER="$CUR/SrcServer/src/character.cpp"

EXPECTED_HEAD="22f79f193f1bf32e29e495a6be3bdf44028dee8e"
EXPECTED_FILEREAD_BEFORE="a4c5d51661551bdd4bed240581b567fd11943e44dab7a0bb841038260c7cb726"
EXPECTED_FILEREAD_AFTER="3a9ca7501d29f0afb5172cf125b363120c27120aaf05d559a8a08aadb5088e34"
EXPECTED_CHARACTER_BEFORE="44a122a1ff5b14934e622a3879e0ccb40ad135184c223d4efd1e34903cdfa365"
EXPECTED_CHARACTER_AFTER="fb066ac583c24584a758a13e0e8640204944e62e195a8d2b32337a5b347aad6e"

mkdir -p "$BUGFIX/before" "$BUGFIX/after" "$BUGFIX/evidence"

echo "=== 041 — DOCUMENT + COMMIT MONSTER SPAWN FIX ==="
echo

HEAD="$(git -C "$REPO" rev-parse HEAD 2>/dev/null)"
FILE_SHA="$(sha256sum "$FILEREAD" | awk '{print $1}')"
CHAR_SHA="$(sha256sum "$CHARACTER" | awk '{print $1}')"

echo "HEAD=$HEAD"
echo "fileread.cpp=$FILE_SHA"
echo "character.cpp=$CHAR_SHA"
echo

VALID=1

if [ "$HEAD" != "$EXPECTED_HEAD" ]; then
    echo "ERRO: HEAD não é o Fix 036 esperado."
    echo "Esperado: $EXPECTED_HEAD"
    echo "Atual:    $HEAD"
    VALID=0
fi

if [ "$FILE_SHA" != "$EXPECTED_FILEREAD_AFTER" ]; then
    echo "ERRO: fileread.cpp não está no estado runtime-validado da 037."
    echo "Esperado: $EXPECTED_FILEREAD_AFTER"
    echo "Atual:    $FILE_SHA"
    VALID=0
fi

if [ "$CHAR_SHA" != "$EXPECTED_CHARACTER_AFTER" ]; then
    echo "ERRO: character.cpp não está no estado runtime-validado da 039."
    echo "Esperado: $EXPECTED_CHARACTER_AFTER"
    echo "Atual:    $CHAR_SHA"
    VALID=0
fi

if [ "$VALID" -ne 1 ]; then
    echo
    echo "Nenhum commit foi criado porque o source não corresponde ao estado testado."
else
    printf '%s\n' "$HEAD" > "$BUGFIX/base_commit.txt"

    # BEFORE = exatamente o commit 22f79f1.
    git -C "$REPO" show "$HEAD:SourceCode/SrcServer/src/fileread.cpp" \
        > "$BUGFIX/before/fileread.cpp" 2>/dev/null
    git -C "$REPO" show "$HEAD:SourceCode/SrcServer/src/character.cpp" \
        > "$BUGFIX/before/character.cpp" 2>/dev/null

    BEFORE_FILE_SHA="$(sha256sum "$BUGFIX/before/fileread.cpp" | awk '{print $1}')"
    BEFORE_CHAR_SHA="$(sha256sum "$BUGFIX/before/character.cpp" | awk '{print $1}')"

    if [ "$BEFORE_FILE_SHA" != "$EXPECTED_FILEREAD_BEFORE" ] || \
       [ "$BEFORE_CHAR_SHA" != "$EXPECTED_CHARACTER_BEFORE" ]; then
        echo "ERRO: snapshots BEFORE não correspondem à base conhecida."
        echo "fileread BEFORE=$BEFORE_FILE_SHA"
        echo "character BEFORE=$BEFORE_CHAR_SHA"
    else
        cp -a "$FILEREAD" "$BUGFIX/after/fileread.cpp"
        cp -a "$CHARACTER" "$BUGFIX/after/character.cpp"

        (
            cd "$BUGFIX/before" &&
            sha256sum fileread.cpp character.cpp
        ) > "$BUGFIX/hashes_before.sha256"

        (
            cd "$BUGFIX/after" &&
            sha256sum fileread.cpp character.cpp
        ) > "$BUGFIX/hashes_after.sha256"

        diff -u \
            "$BUGFIX/before/fileread.cpp" \
            "$BUGFIX/after/fileread.cpp" \
            > "$BUGFIX/fileread_before_after.diff" 2>/dev/null || true

        diff -u \
            "$BUGFIX/before/character.cpp" \
            "$BUGFIX/after/character.cpp" \
            > "$BUGFIX/character_before_after.diff" 2>/dev/null || true

        git -C "$REPO" diff --binary "$HEAD" -- \
            "SourceCode/SrcServer/src/fileread.cpp" \
            "SourceCode/SrcServer/src/character.cpp" \
            > "$BUGFIX/change.patch"

        git -C "$REPO" diff "$HEAD" -- \
            "SourceCode/SrcServer/src/fileread.cpp" \
            "SourceCode/SrcServer/src/character.cpp" \
            > "$BUGFIX/source_diff.txt"

        # Preserve the exact evidence that led to the fix.
        for f in \
            "$FIX037/recovery_validation.txt" \
            "$FIX037/hunk_headers.txt" \
            "$FIX037/change.patch" \
            "$FIX037/before_after.diff"
        do
            if [ -f "$f" ]; then
                cp -a "$f" "$BUGFIX/evidence/037_$(basename "$f")"
            fi
        done

        for f in \
            "$FIX038/addr2line.txt" \
            "$FIX038/nearest_symbols.txt" \
            "$FIX038/crash_disassembly_window.txt" \
            "$FIX038/server_exe.sha256" \
            "$FIX038/image_base.txt"
        do
            if [ -f "$f" ]; then
                cp -a "$f" "$BUGFIX/evidence/038_$(basename "$f")"
            fi
        done

        for f in \
            "$FIX039/crash_evidence.txt" \
            "$FIX039/change.patch" \
            "$FIX039/before_after.diff"
        do
            if [ -f "$f" ]; then
                cp -a "$f" "$BUGFIX/evidence/039_$(basename "$f")"
            fi
        done

        cat > "$BUGFIX/files_changed.txt" <<'EOF'
ARQUIVOS DE SOURCE ALTERADOS NESTE FIX

1. SourceCode/SrcServer/src/fileread.cpp
   Função:
     DecodeOpenMonster()

   Alterações recuperadas do working tree pré-reset:
     - token CP949 ACTOR
     - token CP949 BOSS_ACTOR
     - token CP949 MAX_ACTOR_POS
     - token CP949 DELAY
     - token CP949 MAX_ACTOR
     - instrumentação MINGW_SPM_ACTOR_MATCH_TRACE_V1

   Papel:
     - ler arquivos .spm
     - reconhecer blocos ACTOR
     - comparar o nome do ACTOR com lpCharInfoList[].szName
     - alimentar rsMonster[] / MonCnt

2. SourceCode/SrcServer/src/character.cpp
   Função:
     smCHAR::FindMotionCountFromCode(DWORD MotionCode)

   Alteração recuperada do working tree pré-reset:
     - if(!smMotionInfo)
     - trace motion_null_trace.txt
     - return 0 antes da dereferência

   Papel:
     - impedir crash quando o fluxo reativado de monstros encontra
       personagem sem smMotionInfo carregado.

Nenhum arquivo do cliente foi alterado.
Nenhum código de item/drop foi alterado.
EOF

        cat > "$BUGFIX/runtime_validation.txt" <<'EOF'
VALIDAÇÃO RUNTIME — 2026-08-24

BASE
----
Fix 036 já commitado:
22f79f1 fix(server): restore CP949 character definition tokens

Esse fix restaurou smCharDecode() e resolveu os modelos repetidos dos NPCs.

ETAPA 037 — DecodeOpenMonster
-----------------------------
Depois do Fix 036, monstros ainda não apareciam.

Foi reaplicado do recovery pré-reset o conjunto completo existente em
DecodeOpenMonster().

spm_actor_trace.txt confirmou que o parser .spm passou a reconhecer ACTOR
e que os nomes estavam encontrando correspondência válida em lpCharInfoList.

Exemplos observados:

GameServer//Field//fore-3.ase.spm
  Cogumelo Fantasma -> match=63  listCnt=374
  Duende Macabro    -> match=72  listCnt=374
  Imp               -> match=18  listCnt=374
  Planta Assassina  -> match=40  listCnt=374
  Doral             -> match=30  listCnt=374

GameServer//Field//fore-1.ase.spm
  Hopi              -> match=55  listCnt=374
  Cuepi             -> match=54  listCnt=374
  Tatu              -> match=104 listCnt=374
  Cogumelo Fantasma -> match=63  listCnt=374
  Duende Macabro    -> match=72  listCnt=374

Conclusão:
- ACTOR passou a ser reconhecido;
- lpCharInfoList estava populada;
- ACTOR -> NAME funcionava;
- o caminho de monster loading/spawn foi reativado.

Após isso o servidor crashou.

ETAPA 038 — simbolização do crash
---------------------------------
Crash:
  exception      = 0xC0000005
  operation      = READ
  invalid addr   = 0x0001598C
  crash RVA      = 0x00011DCE

Server.exe:
  preferred ImageBase = 0x00400000

Logo:
  RVA 0x00011DCE -> VA 0x00411DCE

Símbolo:
  smCHAR::FindMotionCountFromCode(unsigned long)

Disassembly:

00411d84 <smCHAR::FindMotionCountFromCode(unsigned long)>:
...
411dc5: mov this -> eax
411dc8: mov 0x4704(eax), eax
411dce: mov 0x1598c(eax), eax

No crash:
  EAX = 0x00000000

Interpretação:
  this->smMotionInfo era NULL;
  a instrução 0x411DCE tentou ler smMotionInfo + 0x1598C;
  NULL + 0x1598C = 0x0001598C;
  esse era exatamente o endereço inválido informado pela exceção.

ETAPA 039 — guard de smMotionInfo
---------------------------------
O recovery pré-reset continha exatamente um guard em:

smCHAR::FindMotionCountFromCode()

Comportamento:
  if(!smMotionInfo)
  {
      ...trace...
      return 0;
  }

O guard foi reaplicado mantendo a 037 ativa.

RESULTADO FINAL
---------------
- servidor deixou de cair nesse ponto;
- monstros voltaram aos mapas;
- mobs estão disponíveis para combate;
- NPCs continuam corretos;
- bug de monster spawn/loading considerado RESOLVIDO.

Conclusão técnica:
o estado funcional dependia de MAIS DE UMA alteração pré-reset:

1. Fix 036:
   smCharDecode() com tokens CP949 corretos.

2. Fix 041 / parte fileread:
   DecodeOpenMonster() com tokens CP949 corretos para os .spm.

3. Fix 041 / parte character:
   proteção de smMotionInfo NULL no caminho reativado de motions.

Aplicar apenas uma parte produzia estados híbridos:
- sem DecodeOpenMonster: mobs não entravam no fluxo;
- com DecodeOpenMonster mas sem guard: fluxo era reativado e crashava;
- com ambos: mobs voltaram e ficaram jogáveis.
EOF

        cat > "$BUGFIX/recovery_source.txt" <<'EOF'
FONTE DAS ALTERAÇÕES

Recovery utilizado:
  /home/gabriel/Transferências/recovery_pre_reset_20260824_0958/working_tree_vs_HEAD.patch

O recovery representa alterações existentes no working tree imediatamente
antes do reset que causou a regressão.

CURRENT continuou sendo a base de edição.

O armazém histórico NÃO foi usado como fonte de código deste fix.

Alterações reaplicadas do recovery:
- DecodeOpenMonster() em SrcServer/src/fileread.cpp
- FindMotionCountFromCode() em SrcServer/src/character.cpp
EOF

        cat > "$BUGFIX/README.md" <<'EOF'
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
EOF

        git -C "$REPO" status --short > "$BUGFIX/status_before_commit.txt"

        echo "=== SOURCE DIFF CHECK ==="
        git -C "$REPO" diff --check -- \
            "SourceCode/SrcServer/src/fileread.cpp" \
            "SourceCode/SrcServer/src/character.cpp" \
            | tee "$BUGFIX/diff_check.txt"

        echo
        echo "=== BUGFIX STRUCTURE ==="
        find "$BUGFIX" -maxdepth 2 -type f -printf '%P\n' | sort

        echo
        echo "=== SOURCE DIFF STAT ==="
        git -C "$REPO" diff --stat "$HEAD" -- \
            "SourceCode/SrcServer/src/fileread.cpp" \
            "SourceCode/SrcServer/src/character.cpp"

        REL_BUGFIX="$(realpath --relative-to="$REPO" "$BUGFIX")"

        # Stage exactly the two validated server source files + this bugfix record.
        git -C "$REPO" add -- \
            "SourceCode/SrcServer/src/fileread.cpp" \
            "SourceCode/SrcServer/src/character.cpp" \
            "$REL_BUGFIX"

        echo
        echo "=== STAGED ==="
        git -C "$REPO" diff --cached --stat

        # Commit only these paths so unrelated working-tree noise is excluded.
        echo
        echo "=== COMMIT ==="
        if git -C "$REPO" commit --only \
            -m "fix(server): restore monster spawn loading path" \
            -- \
            "SourceCode/SrcServer/src/fileread.cpp" \
            "SourceCode/SrcServer/src/character.cpp" \
            "$REL_BUGFIX"
        then
            echo
            echo "=== COMMIT CREATED ==="
            git -C "$REPO" log -1 --oneline
            git -C "$REPO" rev-parse HEAD > "$BUGFIX/commit_hash.txt"
            git -C "$REPO" log -1 --oneline > "$BUGFIX/commit.txt"

            echo
            echo "=== FINAL STATUS (SOURCE RELEVANT) ==="
            git -C "$REPO" status --short | grep -E \
                'SrcServer/src/(fileread|character)\.cpp|SrcGame/src/' || true

            echo
            echo "Fix 041 commitado."
            echo "Metadados commit.txt/commit_hash.txt foram gerados depois do commit"
            echo "e ficam como evidência local, sem criar um commit circular."
        else
            echo "ERRO: commit falhou. Working tree preservado."
        fi
    fi
fi
