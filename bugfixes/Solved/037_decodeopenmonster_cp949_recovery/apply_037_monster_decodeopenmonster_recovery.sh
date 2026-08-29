#!/usr/bin/env bash

ROOT="/home/gabriel/Área de Trabalho/summit"
REPO="$ROOT"
CUR="$ROOT/SourceCode"
REC_PATCH="$ROOT/bugfixes/034_recovery_vs_current/per_file/034__SourceCode__SrcServer__src__fileread.cpp.patch"

FIX035="$ROOT/bugfixes/035_reintroduce_server_character_recovery"
FIX036="$ROOT/bugfixes/036_smchardecode_cp949_npc_fix"
BUGFIX="$ROOT/bugfixes/037_decodeopenmonster_cp949_recovery"

CHARACTER="$CUR/SrcServer/src/character.cpp"
FILEREAD="$CUR/SrcServer/src/fileread.cpp"
PATCH="$BUGFIX/change.patch"

mkdir -p "$BUGFIX/before" "$BUGFIX/after"

echo "=== 037 — CLEAN 035 + RESTORE DecodeOpenMonster FROM RECOVERY ==="
echo

HEAD="$(git -C "$REPO" rev-parse HEAD 2>/dev/null)"
echo "HEAD=$HEAD"

if [ "$HEAD" != "22f79f1"* ]; then
    echo "AVISO: HEAD não começa com 22f79f1."
    echo "Continuando somente se os arquivos abaixo passarem nas validações."
fi

# -------------------------------------------------------------------
# A. Limpar somente o experimento 035 de character.cpp
# -------------------------------------------------------------------

echo
echo "=== A. CLEAN EXPERIMENTAL 035 ==="

EXPECTED_BEFORE="44a122a1ff5b14934e622a3879e0ccb40ad135184c223d4efd1e34903cdfa365"
EXPECTED_AFTER="fb066ac583c24584a758a13e0e8640204944e62e195a8d2b32337a5b347aad6e"
CURRENT_CHAR_SHA="$(sha256sum "$CHARACTER" | awk '{print $1}')"

echo "character.cpp current: $CURRENT_CHAR_SHA"

if [ "$CURRENT_CHAR_SHA" = "$EXPECTED_AFTER" ]; then
    SNAP="$FIX035/character.cpp.BEFORE"

    if [ ! -f "$SNAP" ]; then
        echo "ERRO: snapshot BEFORE da 035 não encontrado:"
        echo "$SNAP"
        exit 1
    fi

    SNAP_SHA="$(sha256sum "$SNAP" | awk '{print $1}')"

    if [ "$SNAP_SHA" != "$EXPECTED_BEFORE" ]; then
        echo "ERRO: snapshot 035 BEFORE tem hash inesperado:"
        echo "$SNAP_SHA"
        exit 1
    fi

    cp -a "$SNAP" "$CHARACTER"
    echo "035 revertida com segurança."

elif [ "$CURRENT_CHAR_SHA" = "$EXPECTED_BEFORE" ]; then
    echo "character.cpp já está sem a 035."
else
    echo "ERRO: character.cpp não corresponde nem ao BEFORE nem ao AFTER conhecido da 035."
    echo "Nenhuma sobrescrita será feita."
    exit 1
fi

FINAL_CHAR_SHA="$(sha256sum "$CHARACTER" | awk '{print $1}')"

if [ "$FINAL_CHAR_SHA" != "$EXPECTED_BEFORE" ]; then
    echo "ERRO: limpeza da 035 não produziu o hash esperado."
    exit 1
fi

echo "character.cpp final: $FINAL_CHAR_SHA"

# -------------------------------------------------------------------
# B. Remover apenas metadados pós-commit 036 que ficaram untracked.
# -------------------------------------------------------------------

echo
echo "=== B. CLEAN POST-COMMIT 036 UNTRACKED METADATA ==="

for f in \
    "$FIX036/commit.txt" \
    "$FIX036/commit_hash.txt" \
    "$FIX036/status_after_commit.txt"
do
    REL="${f#$ROOT/}"

    if [ -f "$f" ]; then
        if git -C "$REPO" ls-files --error-unmatch "$REL" >/dev/null 2>&1; then
            echo "Mantido (tracked): $REL"
        else
            rm -f "$f"
            echo "Removido (untracked pós-commit): $REL"
        fi
    fi
done

# -------------------------------------------------------------------
# C. Snapshot base da 037
# -------------------------------------------------------------------

echo
echo "=== C. SNAPSHOT 037 BEFORE ==="

BASE_COMMIT="$(git -C "$REPO" rev-parse HEAD)"
printf '%s\n' "$BASE_COMMIT" > "$BUGFIX/base_commit.txt"

cp -a "$FILEREAD" "$BUGFIX/before/fileread.cpp"
(
    cd "$BUGFIX/before" &&
    sha256sum fileread.cpp
) > "$BUGFIX/hashes_before.sha256"

git -C "$REPO" status --short > "$BUGFIX/status_before.txt"

# -------------------------------------------------------------------
# D. Extrair exatamente todos os hunks recovery de DecodeOpenMonster
# -------------------------------------------------------------------

echo
echo "=== D. EXTRACT + VALIDATE DecodeOpenMonster RECOVERY ==="

if [ ! -f "$REC_PATCH" ]; then
    echo "ERRO: recovery patch de fileread.cpp não encontrado:"
    echo "$REC_PATCH"
    exit 1
fi

python3 - "$REC_PATCH" "$PATCH" "$BUGFIX" <<'PY'
from pathlib import Path
import sys, re, hashlib

source = Path(sys.argv[1])
dest = Path(sys.argv[2])
bugfix = Path(sys.argv[3])

data = source.read_bytes()
lines = data.splitlines(keepends=True)

try:
    first_hunk = next(i for i, x in enumerate(lines) if x.startswith(b"@@ "))
except StopIteration:
    raise SystemExit("ERRO: patch recovery não contém hunks")

header = lines[:first_hunk]
selected = []
headers = []

i = first_hunk
while i < len(lines):
    if not lines[i].startswith(b"@@ "):
        i += 1
        continue

    j = i + 1
    while (
        j < len(lines)
        and not lines[j].startswith(b"@@ ")
        and not lines[j].startswith(b"diff --git ")
    ):
        j += 1

    if b"DecodeOpenMonster" in lines[i]:
        headers.append(lines[i])
        selected.extend(lines[i:j])

    i = j

if len(headers) != 4:
    raise SystemExit(
        f"ERRO: esperado 4 hunks de DecodeOpenMonster; encontrado {len(headers)}"
    )

patch = b"".join(header + selected)

tokens = {
    "ACTOR": rb"\x2A\xC3\xE2\xBF\xAC\xC0\xDA",
    "BOSS_ACTOR": rb"\x2A\xC3\xE2\xBF\xAC\xC0\xDA\xB5\xCE\xB8\xF1",
    "MAX_ACTOR_POS": rb"\x2A\xC3\xD6\xB4\xEB\xB5\xBF\xBD\xC3\xC3\xE2\xC7\xF6\xBC\xF6",
    "DELAY": rb"\x2A\xC3\xE2\xC7\xF6\xB0\xA3\xB0\xDD",
    "MAX_ACTOR": rb"\x2A\xC3\xE2\xC7\xF6\xBC\xF6",
}

missing = [name for name, token in tokens.items() if token not in patch]
if missing:
    raise SystemExit(
        "ERRO: tokens recovery ausentes: " + ", ".join(missing)
    )

trace_present = b"MINGW_SPM_ACTOR_MATCH_TRACE_V1" in patch

adds = [
    x for x in patch.splitlines()
    if x.startswith(b"+") and not x.startswith(b"+++")
]
dels = [
    x for x in patch.splitlines()
    if x.startswith(b"-") and not x.startswith(b"---")
]

dest.write_bytes(patch)

report = [
    "VALIDATED_RECOVERY_SET=YES",
    "function=DecodeOpenMonster",
    f"hunks={len(headers)}",
    f"additions={len(adds)}",
    f"deletions={len(dels)}",
    "ACTOR=present",
    "BOSS_ACTOR=present",
    "MAX_ACTOR_POS=present",
    "DELAY=present",
    "MAX_ACTOR=present",
    f"MINGW_SPM_ACTOR_MATCH_TRACE_V1={'present' if trace_present else 'absent'}",
    f"patch_sha256={hashlib.sha256(patch).hexdigest()}",
]

(bugfix / "recovery_validation.txt").write_text(
    "\n".join(report) + "\n",
    encoding="utf-8"
)

(bugfix / "hunk_headers.txt").write_bytes(b"".join(headers))

print("\n".join(report))
PY

cat "$BUGFIX/recovery_validation.txt"

# -------------------------------------------------------------------
# E. Aplicar sem commit
# -------------------------------------------------------------------

echo
echo "=== E. APPLY CHECK ==="

if ! git -C "$REPO" apply --check --recount "$PATCH"; then
    echo "ERRO: patch 037 não aplica limpo."
    echo "Nenhuma alteração de fileread.cpp foi aplicada."
    exit 1
fi

echo "Patch aplica limpo."

if ! git -C "$REPO" apply --recount "$PATCH"; then
    echo "ERRO: git apply falhou."
    exit 1
fi

cp -a "$FILEREAD" "$BUGFIX/after/fileread.cpp"
(
    cd "$BUGFIX/after" &&
    sha256sum fileread.cpp
) > "$BUGFIX/hashes_after.sha256"

diff -u \
    "$BUGFIX/before/fileread.cpp" \
    "$BUGFIX/after/fileread.cpp" \
    > "$BUGFIX/before_after.diff" 2>/dev/null || true

git -C "$REPO" diff -- "SourceCode/SrcServer/src/fileread.cpp" \
    > "$BUGFIX/source_diff.txt"

git -C "$REPO" diff --check -- "SourceCode/SrcServer/src/fileread.cpp" \
    > "$BUGFIX/diff_check.txt" 2>&1

git -C "$REPO" status --short > "$BUGFIX/status_after_apply.txt"

# -------------------------------------------------------------------
# F. Documentação detalhada
# -------------------------------------------------------------------

cat > "$BUGFIX/README.md" <<'EOF'
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
EOF

echo
echo "=== RESULT ==="
cat "$BUGFIX/hashes_before.sha256"
cat "$BUGFIX/hashes_after.sha256"
echo
echo "Bugfix criado em:"
echo "$BUGFIX"
echo
echo "037 aplicada e NÃO commitada."
echo "Compile somente o servidor e teste o runtime."
