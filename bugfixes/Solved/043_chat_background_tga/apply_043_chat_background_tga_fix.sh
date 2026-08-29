#!/usr/bin/env bash

ROOT="/home/gabriel/Área de Trabalho/summit"
REPO="$ROOT"
CUR="$ROOT/SourceCode"
BASE="/home/gabriel/Transferências/summit_pre_reset_restored_20260824/SourceCode"

BUGFIX="$ROOT/bugfixes/043_chat_background_tga"
TARGET_REL="SourceCode/SrcGame/src/sinbaram/sinInterFace.cpp"
TARGET="$ROOT/$TARGET_REL"
BASE_TARGET="$BASE/SrcGame/src/sinbaram/sinInterFace.cpp"

OLD='image//Sinimage//Inter//Chat//ChatDefault-1.png'
NEW='image//Sinimage//Inter//Chat//ChatDefault-1.tga'

mkdir -p "$BUGFIX/before" "$BUGFIX/after"

echo "=== 043 — CHAT BACKGROUND PNG -> TGA ==="
echo

HEAD="$(git -C "$REPO" rev-parse HEAD 2>/dev/null)"
printf '%s\n' "$HEAD" > "$BUGFIX/base_commit.txt"

if [ ! -f "$TARGET" ] || [ ! -f "$BASE_TARGET" ]; then
    echo "ERRO: source CURRENT ou BASE não encontrado."
else
    cp -a "$TARGET" "$BUGFIX/before/sinInterFace.cpp"

    (
        cd "$BUGFIX/before" &&
        sha256sum sinInterFace.cpp
    ) > "$BUGFIX/hashes_before.sha256"

    python3 - "$TARGET" "$BASE_TARGET" "$OLD" "$NEW" "$BUGFIX" <<'PY'
from pathlib import Path
import sys, hashlib

target = Path(sys.argv[1])
base = Path(sys.argv[2])
old = sys.argv[3]
new = sys.argv[4]
bugfix = Path(sys.argv[5])

cur = target.read_text(encoding="utf-8-sig")
bas = base.read_text(encoding="utf-8-sig")

cur_old = cur.count(old)
cur_new = cur.count(new)
base_old = bas.count(old)
base_new = bas.count(new)

report = [
    f"CURRENT_OLD_PNG_COUNT={cur_old}",
    f"CURRENT_NEW_TGA_COUNT={cur_new}",
    f"BASE_OLD_PNG_COUNT={base_old}",
    f"BASE_NEW_TGA_COUNT={base_new}",
]

if cur_old != 1:
    raise SystemExit(
        "ERRO: CURRENT não contém exatamente uma referência PNG esperada."
    )

if cur_new != 0:
    raise SystemExit(
        "ERRO: CURRENT já contém a referência TGA; estado inesperado."
    )

if base_new != 1:
    raise SystemExit(
        "ERRO: BASE não contém exatamente uma referência TGA esperada."
    )

fixed = cur.replace(old, new, 1)

if fixed.count(old) != 0 or fixed.count(new) != 1:
    raise SystemExit("ERRO: substituição não produziu o estado esperado.")

target.write_text(fixed, encoding="utf-8-sig", newline="")

report.extend([
    "VALIDATED_FIX=YES",
    "CHANGE=ChatDefault-1.png -> ChatDefault-1.tga",
    f"after_sha256={hashlib.sha256(target.read_bytes()).hexdigest()}",
])

(bugfix / "validation.txt").write_text(
    "\n".join(report) + "\n",
    encoding="utf-8",
)

print("\n".join(report))
PY

    cp -a "$TARGET" "$BUGFIX/after/sinInterFace.cpp"

    (
        cd "$BUGFIX/after" &&
        sha256sum sinInterFace.cpp
    ) > "$BUGFIX/hashes_after.sha256"

    diff -u \
        "$BUGFIX/before/sinInterFace.cpp" \
        "$BUGFIX/after/sinInterFace.cpp" \
        > "$BUGFIX/before_after.diff" 2>/dev/null || true

    git -C "$REPO" diff -- "$TARGET_REL" > "$BUGFIX/source_diff.txt"
    git -C "$REPO" diff --binary -- "$TARGET_REL" > "$BUGFIX/change.patch"
    git -C "$REPO" diff --check -- "$TARGET_REL" > "$BUGFIX/diff_check.txt" 2>&1

    cat > "$BUGFIX/README.md" <<'EOF'
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
EOF

    echo
    echo "=== VALIDATION ==="
    cat "$BUGFIX/validation.txt"

    echo
    echo "=== DIFF ==="
    git -C "$REPO" diff -- "$TARGET_REL"

    echo
    echo "=== RESULT ==="
    echo "Fix 043 aplicado e NÃO commitado."
    echo "Compile somente o cliente e teste."
fi
