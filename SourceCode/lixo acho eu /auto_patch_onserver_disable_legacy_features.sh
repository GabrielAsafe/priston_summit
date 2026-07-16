#!/usr/bin/env bash
set -euo pipefail

FILE="SrcServer/src/Server/OnSever.cpp"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="${FILE}.bak_disable_legacy_${STAMP}"

cp -a "$FILE" "$BACKUP"
echo "Backup: $BACKUP"

python3 - <<'PY'
from pathlib import Path
import re

path = Path("SrcServer/src/Server/OnSever.cpp")
text = path.read_text(encoding="latin-1")
lines = text.splitlines()

missing_opcodes = {
    "CHECK_SOD_CROWN",
    "OPCODE_BAD_PLAYER",
    "OPCODE_BOOSTER_DATA",
    "OPCODE_CHECK_EXPDATA",
    "OPCODE_CONTINUEDATA",
    "OPCODE_EVENT_GAME",
    "OPCODE_FORCEORB_DATA",
    "OPCODE_FUNCTON_RESULT",
    "OPCODE_IPLIST",
    "OPCODE_PRIMEITEM_DATA",
    "OPCODE_PRIMEITEM_DATA2",
    "OPCODE_PRIMEITEM_DATA3",
    "OPCODE_PRIMEITEM_DATA4",
    "OPCODE_SET_OBJSERIAL",
    "OPCODE_SKILLDELAY_DATA",
    "OPCODE_UPDATE_SERVER_PARAM",
    "PLAYER_GM",
    "UPDATE_ITEM_PREMIUM",
    "WARMODE",
}

disabled = []
unhandled = []


def brace_delta(line):
    """
    Conta chaves aproximadamente, ignorando strings simples.
    É suficiente para localizar o fim dos blocos case desta fonte.
    """
    clean = re.sub(r'"(?:\\.|[^"\\])*"', '""', line)
    clean = re.sub(r"'(?:\\.|[^'\\])*'", "''", clean)
    clean = re.sub(r"//.*$", "", clean)
    return clean.count("{") - clean.count("}")


def contains_missing(line):
    found = []

    for name in missing_opcodes:
        if re.search(rf"\bOpCode::{re.escape(name)}\b", line):
            found.append(name)

    return found


def find_case_end(start):
    """
    Localiza o break do case, respeitando blocos internos.
    """
    depth = 0
    entered_block = False

    for index in range(start, len(lines)):
        line = lines[index]
        depth += brace_delta(line)

        if "{" in line:
            entered_block = True

        if (
            index > start
            and re.match(r"^\s*break\s*;\s*(?://.*)?$", line)
            and (depth <= 0 or not entered_block)
        ):
            return index

        # Alguns cases terminam imediatamente antes do próximo case/default.
        if (
            index > start
            and depth <= 0
            and re.match(r"^\s*(case\b|default\s*:)", line)
        ):
            return index - 1

    return None


# Primeiro procura cases ligados aos opcodes faltantes.
ranges = []

for index, line in enumerate(lines):
    found = contains_missing(line)

    if not found:
        continue

    if re.search(r"\bcase\s+OpCode::", line):
        end = find_case_end(index)

        if end is None:
            unhandled.append(
                f"linha {index + 1}: case sem fim localizado: {found}"
            )
            continue

        ranges.append((index, end, found))


# Remove ranges duplicados/sobrepostos.
ranges.sort()
merged = []

for start, end, names in ranges:
    if merged and start <= merged[-1][1]:
        merged[-1][1] = max(merged[-1][1], end)
        merged[-1][2].update(names)
    else:
        merged.append([start, end, set(names)])


# Aplica de baixo para cima para não alterar os índices anteriores.
for start, end, names in reversed(merged):
    indent = re.match(r"^(\s*)", lines[start]).group(1)
    label = ", ".join(sorted(names))

    block = [
        f"{indent}#if 0 /* SUMMIT_DISABLED_MISSING_OPCODE: {label} */",
        *lines[start:end + 1],
        f"{indent}#endif /* SUMMIT_DISABLED_MISSING_OPCODE */",
    ]

    lines[start:end + 1] = block
    disabled.append((start + 1, end + 1, label))


text = "\n".join(lines) + "\n"

# Corrige getName().compare(...), pois getName() retorna const char*.
text, compare_count = re.subn(
    r'lpPlayInfo->getName\(\)\.compare\(([^;\n]+)\)',
    r'strcmp(lpPlayInfo->getName(), \1)',
    text
)

text, compare_count2 = re.subn(
    r'lpPlayInfo2->getName\(\)\.compare\(([^;\n]+)\)',
    r'strcmp(lpPlayInfo2->getName(), \1)',
    text
)

compare_count += compare_count2

# SIN_ADD_NUM está confirmado nos headers de idioma, mas nem sempre é incluído.
if "SUMMIT_COMPAT_SIN_ADD_NUM" not in text:
    marker = "#include <cstring>\n"

    block = """
#ifndef SIN_ADD_NUM
#define SIN_ADD_NUM 1
#endif
#define SUMMIT_COMPAT_SIN_ADD_NUM
"""

    if marker in text:
        text = text.replace(marker, marker + block, 1)
    else:
        text = block + text

# smTRANS_PLAYDATA atual não tem Mp.
# Comenta apenas instruções completas, terminadas em ponto e vírgula.
new_lines = []
mp_count = 0
coin_count = 0

for line in text.splitlines():
    stripped = line.strip()

    if (
        re.search(r"\b(?:lp)?[A-Za-z0-9_>\.\[\]-]+\.Mp\b", line)
        and stripped.endswith(";")
        and not stripped.startswith("//")
    ):
        new_lines.append(
            line[:len(line) - len(line.lstrip())]
            + "// SUMMIT_DISABLED_MISSING_PLAYDATA_MP: "
            + line.lstrip()
        )
        mp_count += 1
        continue

    if (
        ".CoinPrice" in line
        and stripped.endswith(";")
        and not stripped.startswith("//")
    ):
        new_lines.append(
            line[:len(line) - len(line.lstrip())]
            + "// SUMMIT_DISABLED_MISSING_COINPRICE: "
            + line.lstrip()
        )
        coin_count += 1
        continue

    new_lines.append(line)

text = "\n".join(new_lines) + "\n"
path.write_text(text, encoding="latin-1")

print("Cases desativados:", len(disabled))

for start, end, names in reversed(disabled):
    print(f"  linhas originais {start}-{end}: {names}")

print("getName().compare corrigidos:", compare_count)
print("Linhas com smTRANS_PLAYDATA::Mp comentadas:", mp_count)
print("Linhas com sMYSHOP_SELL_INFO::CoinPrice comentadas:", coin_count)

if unhandled:
    print("\nReferências não tratadas automaticamente:")

    for item in unhandled:
        print("  " + item)
PY

echo
echo "Referências restantes aos símbolos incompatíveis:"
grep -nE \
'OpCode::(CHECK_SOD_CROWN|OPCODE_BAD_PLAYER|OPCODE_BOOSTER_DATA|OPCODE_CHECK_EXPDATA|OPCODE_CONTINUEDATA|OPCODE_EVENT_GAME|OPCODE_FORCEORB_DATA|OPCODE_FUNCTON_RESULT|OPCODE_IPLIST|OPCODE_PRIMEITEM_DATA2?|OPCODE_PRIMEITEM_DATA3|OPCODE_PRIMEITEM_DATA4|OPCODE_SET_OBJSERIAL|OPCODE_SKILLDELAY_DATA|OPCODE_UPDATE_SERVER_PARAM|PLAYER_GM|UPDATE_ITEM_PREMIUM|WARMODE)' \
"$FILE" |
grep -v 'SUMMIT_DISABLED' |
head -100 || true

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
    echo "Primeiros erros com linhas:"
    grep -E 'OnSever\.cpp:[0-9]+:|error:|fatal error:' \
        /tmp/OnSever.err |
        head -160
fi
