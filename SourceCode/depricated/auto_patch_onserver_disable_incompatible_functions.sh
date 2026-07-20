#!/usr/bin/env bash
set -euo pipefail

FILE="SrcServer/src/Server/OnSever.cpp"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="${FILE}.bak_disable_functions_${STAMP}"

cp -a "$FILE" "$BACKUP"
echo "Backup: $BACKUP"

python3 - <<'PY'
from pathlib import Path
import re

path = Path("SrcServer/src/Server/OnSever.cpp")
text = path.read_text(encoding="latin-1")
lines = text.splitlines()

###############################################################################
# Utilitários básicos para analisar chaves, ignorando strings e comentários.
###############################################################################

def sanitize(line):
    line = re.sub(r'"(?:\\.|[^"\\])*"', '""', line)
    line = re.sub(r"'(?:\\.|[^'\\])*'", "''", line)
    line = re.sub(r"//.*$", "", line)
    return line

def brace_delta(line):
    clean = sanitize(line)
    return clean.count("{") - clean.count("}")

###############################################################################
# Localizar funções de nível superior.
###############################################################################

functions = []
depth = 0
function_start = None
signature_start = None

control_words = {
    "if", "for", "while", "switch", "catch",
    "else", "do", "namespace", "class", "struct",
    "enum", "union"
}

for i, line in enumerate(lines):
    clean = sanitize(line).strip()

    if depth == 0:
        if clean and not clean.startswith("#"):
            if signature_start is None:
                signature_start = i

        if "{" in clean:
            signature = " ".join(
                sanitize(x).strip()
                for x in lines[signature_start:i + 1]
            )

            first_word = (
                signature.split("(", 1)[0]
                .strip()
                .split(" ", 1)[0]
                if signature
                else ""
            )

            looks_like_function = (
                "(" in signature
                and ")" in signature
                and first_word not in control_words
                and not signature.startswith("typedef")
            )

            if looks_like_function:
                function_start = signature_start

    depth += brace_delta(line)

    if depth == 0:
        if function_start is not None:
            functions.append((function_start, i))
            function_start = None

        signature_start = None

###############################################################################
# Símbolos incompatíveis ainda presentes fora dos blocos anteriores.
###############################################################################

patterns = [
    r"\bOpCode::OPCODE_BAD_PLAYER\b",
    r"\bOpCode::OPCODE_CHECK_EXPDATA\b",
    r"\bOpCode::OPCODE_PRIMEITEM_DATA\b",
    r"\bOpCode::OPCODE_PRIMEITEM_DATA2\b",
    r"\bOpCode::OPCODE_PRIMEITEM_DATA3\b",
    r"\bOpCode::OPCODE_PRIMEITEM_DATA4\b",
    r"\bOpCode::OPCODE_FORCEORB_DATA\b",
    r"\bOpCode::OPCODE_BOOSTER_DATA\b",
    r"\bOpCode::OPCODE_SKILLDELAY_DATA\b",
    r"\bOpCode::CHECK_SOD_CROWN\b",
    r"\bOpCode::PLAYER_GM\b",
    r"\bOpCode::OPCODE_IPLIST\b",
    r"\bOpCode::OPCODE_UPDATE_SERVER_PARAM\b",
    r"\.Mp\b",
    r"\.CoinPrice\b",
]

combined = re.compile("|".join(f"(?:{p})" for p in patterns))

target_lines = []

inside_disabled = False

for index, line in enumerate(lines):
    if "#if 0" in line:
        inside_disabled = True

    if not inside_disabled and combined.search(line):
        target_lines.append(index)

    if inside_disabled and "#endif" in line:
        inside_disabled = False

###############################################################################
# Mapear cada referência para sua função externa.
###############################################################################

ranges = {}

for target in target_lines:
    owner = None

    for start, end in functions:
        if start <= target <= end:
            owner = (start, end)
            break

    if owner is None:
        print(
            f"Aviso: referência incompatível fora de função "
            f"na linha {target + 1}: {lines[target].strip()}"
        )
        continue

    ranges.setdefault(owner, []).append(target)

###############################################################################
# Não desativar uma função duas vezes.
###############################################################################

selected = []

for (start, end), refs in sorted(ranges.items()):
    block = "\n".join(lines[start:end + 1])

    if "SUMMIT_DISABLED_INCOMPATIBLE_FUNCTION" in block:
        continue

    selected.append((start, end, refs))

###############################################################################
# Aplicar de baixo para cima.
###############################################################################

for start, end, refs in reversed(selected):
    signature = ""

    for line in lines[start:min(end + 1, start + 8)]:
        signature += " " + line.strip()

        if "{" in line:
            break

    signature = re.sub(r"\s+", " ", signature).strip()
    indent = re.match(r"^(\s*)", lines[start]).group(1)

    referenced = sorted({
        lines[ref].strip()
        for ref in refs
    })

    marker = (
        f"{indent}#if 0 "
        f"/* SUMMIT_DISABLED_INCOMPATIBLE_FUNCTION\n"
        f"{indent}Função: {signature}\n"
        f"{indent}Motivo: depende de protocolo/estrutura ausente nesta revisão.\n"
        f"{indent}*/"
    )

    replacement = [
        marker,
        *lines[start:end + 1],
        f"{indent}#endif "
        f"/* SUMMIT_DISABLED_INCOMPATIBLE_FUNCTION */",
    ]

    lines[start:end + 1] = replacement

    print()
    print(f"Função desativada: linhas {start + 1}-{end + 1}")
    print(f"  {signature[:180]}")

    for ref in referenced[:8]:
        print(f"    referência: {ref}")

###############################################################################
# Corrigir goto quit atravessando char accountId[32].
#
# Localiza a declaração ainda ativa, encontra sua função e move a declaração
# para imediatamente após a abertura da função.
###############################################################################

text = "\n".join(lines) + "\n"
lines = text.splitlines()

account_line = None
inside_disabled = False

for index, line in enumerate(lines):
    if "#if 0" in line:
        inside_disabled = True

    if (
        not inside_disabled
        and re.search(
            r"\bchar\s+accountId\s*\[\s*32\s*\]\s*;",
            line
        )
    ):
        account_line = index
        break

    if inside_disabled and "#endif" in line:
        inside_disabled = False

if account_line is not None:
    # Refaz o mapa de funções depois das alterações.
    functions2 = []
    depth = 0
    start = None
    signature_start = None

    for i, line in enumerate(lines):
        clean = sanitize(line).strip()

        if depth == 0:
            if clean and not clean.startswith("#"):
                if signature_start is None:
                    signature_start = i

            if "{" in clean:
                signature = " ".join(
                    sanitize(x).strip()
                    for x in lines[signature_start:i + 1]
                )

                if "(" in signature and ")" in signature:
                    start = signature_start

        depth += brace_delta(line)

        if depth == 0:
            if start is not None:
                functions2.append((start, i))
                start = None

            signature_start = None

    owner = None

    for start, end in functions2:
        if start <= account_line <= end:
            owner = (start, end)
            break

    if owner:
        start, end = owner

        opening = None

        for i in range(start, min(end + 1, start + 30)):
            if "{" in sanitize(lines[i]):
                opening = i
                break

        if opening is not None and account_line > opening:
            original = lines[account_line]
            indent = original[:len(original) - len(original.lstrip())]

            del lines[account_line]
            lines.insert(
                opening + 1,
                f"{indent}char accountId[32] = {{0}};"
            )

            print()
            print(
                "accountId movido para o início da função "
                "para evitar salto sobre inicialização"
            )

###############################################################################
# Gravar.
###############################################################################

path.write_text(
    "\n".join(lines) + "\n",
    encoding="latin-1"
)

print()
print("Total de funções desativadas:", len(selected))
PY

echo
echo "Referências incompatíveis ainda ativas:"
grep -nE \
'OpCode::(OPCODE_BAD_PLAYER|OPCODE_CHECK_EXPDATA|OPCODE_PRIMEITEM_DATA2?|OPCODE_PRIMEITEM_DATA3|OPCODE_PRIMEITEM_DATA4|OPCODE_FORCEORB_DATA|OPCODE_BOOSTER_DATA|OPCODE_SKILLDELAY_DATA|CHECK_SOD_CROWN|PLAYER_GM|OPCODE_IPLIST|OPCODE_UPDATE_SERVER_PARAM)|\.Mp\b|\.CoinPrice\b' \
"$FILE" |
grep -v \
'SUMMIT_DISABLED_INCOMPATIBLE_FUNCTION' |
head -120 || true

echo
echo "Compilando OnSever.cpp..."

rm -f /tmp/OnSever.o /tmp/OnSever.err

i686-w64-mingw32-g++ \
    -m32 \
    -std=gnu++14 \
    -fpermissive \
    -w \
    -fmax-errors=400 \
    -DWIN32 \
    -D_WINDOWS \
    -D_M_IX86 \
    -Di386 \
    -D__i386__ \
    -D_SERVER \
    -I. \
    -ISrcServer/src \
    -ISrcServer/src/sinbaram \
    -ISrcServer/src/Skills \
    -ISrcGame/src \
    -IShared \
    -IShared/Lua \
    -IShared2 \
    -Imodules \
    -Imodules/File \
    "-IDependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include" \
    -c "$FILE" \
    -o /tmp/OnSever.o \
    2>/tmp/OnSever.err || true

echo

if [ -f /tmp/OnSever.o ]; then
    echo "ONSERVER OK"
else
    echo "OnSever ainda possui erros:"
    echo

    grep -E 'error:|fatal error:' /tmp/OnSever.err |
        sed -E 's/^.*error: //' |
        sort -u |
        head -160

    echo
    echo "Erros com linhas:"
    grep -E \
        'OnSever\.cpp:[0-9]+:|error:|fatal error:' \
        /tmp/OnSever.err |
        head -200
fi
