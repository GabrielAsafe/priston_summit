#!/usr/bin/env bash
set -euo pipefail

SERVER="SrcServer/src/opcodes.h"
GAME="SrcGame/src/opcodes.h"

STAMP="$(date +%Y%m%d_%H%M%S)"

cp -a "$SERVER" "${SERVER}.bak_add_missing_${STAMP}"
cp -a "$GAME"   "${GAME}.bak_add_missing_${STAMP}"

echo "Backups:"
echo "  ${SERVER}.bak_add_missing_${STAMP}"
echo "  ${GAME}.bak_add_missing_${STAMP}"

python3 - <<'PY'
from pathlib import Path
import re
import sys

files = [
    Path("SrcServer/src/opcodes.h"),
    Path("SrcGame/src/opcodes.h"),
]

missing_names = [
    "OPCODE_BAD_PLAYER",
    "OPCODE_CHECK_EXPDATA",
    "OPCODE_PRIMEITEM_DATA",
    "OPCODE_PRIMEITEM_DATA2",
    "OPCODE_PRIMEITEM_DATA3",
    "OPCODE_PRIMEITEM_DATA4",
    "OPCODE_FORCEORB_DATA",
    "OPCODE_BOOSTER_DATA",
    "OPCODE_SKILLDELAY_DATA",
    "CHECK_SOD_CROWN",
    "PLAYER_GM",
    "OPCODE_IPLIST",
    "OPCODE_UPDATE_SERVER_PARAM",
]

definition_pattern = re.compile(
    r"""
    \b
    (?P<name>[A-Za-z_][A-Za-z0-9_]*)
    \s*=\s*
    (?P<base>0x[0-9A-Fa-f]+|\d+)
    (?:\s*\+\s*(?P<offset>0x[0-9A-Fa-f]+|\d+))?
    \s*;
    """,
    re.VERBOSE,
)

BASE = 0x70000000

def parse_definitions(text):
    definitions = {}

    for match in definition_pattern.finditer(text):
        name = match.group("name")
        base = int(match.group("base"), 0)
        offset_raw = match.group("offset")
        value = base

        if offset_raw is not None:
            value += int(offset_raw, 0)

        definitions[name] = value

    return definitions

def find_namespace_end(text):
    namespace_match = re.search(r"\bnamespace\s+OpCode\s*\{", text)

    if not namespace_match:
        raise RuntimeError("namespace OpCode não encontrado")

    opening = text.find("{", namespace_match.start())

    depth = 0
    in_string = False
    in_char = False
    escaped = False
    in_line_comment = False
    in_block_comment = False

    for i in range(opening, len(text)):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if in_line_comment:
            if char == "\n":
                in_line_comment = False
            continue

        if in_block_comment:
            if char == "*" and nxt == "/":
                in_block_comment = False
            continue

        if in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            continue

        if in_char:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == "'":
                in_char = False
            continue

        if char == "/" and nxt == "/":
            in_line_comment = True
            continue

        if char == "/" and nxt == "*":
            in_block_comment = True
            continue

        if char == '"':
            in_string = True
            continue

        if char == "'":
            in_char = True
            continue

        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1

            if depth == 0:
                return i

    raise RuntimeError("fim do namespace OpCode não encontrado")

texts = {
    path: path.read_text(encoding="latin-1")
    for path in files
}

definitions_by_file = {
    path: parse_definitions(text)
    for path, text in texts.items()
}

# Detectar valores 0x70000000 realmente ocupados nos dois headers.
used_values = set()

for definitions in definitions_by_file.values():
    for value in definitions.values():
        if BASE <= value < BASE + 0x10000:
            used_values.add(value)

if not used_values:
    raise RuntimeError("nenhum opcode da família 0x70000000 encontrado")

max_used = max(used_values)
next_value = max_used + 1

# Preservar valores já existentes caso algum símbolo esteja definido apenas
# num dos dois headers.
assigned = {}

for name in missing_names:
    existing_values = {
        definitions[name]
        for definitions in definitions_by_file.values()
        if name in definitions
    }

    if len(existing_values) > 1:
        formatted = ", ".join(
            f"0x{value:08X}"
            for value in sorted(existing_values)
        )
        raise RuntimeError(
            f"{name} possui valores divergentes: {formatted}"
        )

    if existing_values:
        value = next(iter(existing_values))
        assigned[name] = value
        used_values.add(value)
        continue

    while next_value in used_values:
        next_value += 1

    assigned[name] = next_value
    used_values.add(next_value)
    next_value += 1

print(f"Maior valor anterior: 0x{max_used:08X}")
print()
print("Valores atribuídos:")

for name, value in assigned.items():
    print(
        f"  {name:<32} "
        f"0x{value:08X} "
        f"(0x70000000 + {value - BASE})"
    )

marker_begin = (
    "// SUMMIT_BEGIN_RESTORED_COMPATIBILITY_OPCODES"
)
marker_end = (
    "// SUMMIT_END_RESTORED_COMPATIBILITY_OPCODES"
)

for path in files:
    text = texts[path]
    definitions = definitions_by_file[path]

    if marker_begin in text:
        print(f"\n{path}: bloco já existe; nenhuma alteração aplicada.")
        continue

    lines = [
        "",
        "    // -----------------------------------------------------------------",
        "    // Compatibility opcodes restored for the current source revision.",
        "    // Keep this block synchronized between client and server.",
        f"    {marker_begin}",
    ]

    for name in missing_names:
        if name in definitions:
            continue

        value = assigned[name]
        offset = value - BASE

        lines.append(
            f"    static const int {name:<34} "
            f"= 0x70000000 + {offset};"
        )

    lines.extend([
        f"    {marker_end}",
        "    // -----------------------------------------------------------------",
        "",
    ])

    if len(lines) == 8:
        print(f"\n{path}: todos os símbolos já estavam definidos.")
        continue

    insert_at = find_namespace_end(text)

    updated = (
        text[:insert_at]
        + "\n".join(lines)
        + text[insert_at:]
    )

    path.write_text(updated, encoding="latin-1")
    print(f"\n{path}: atualizado.")

# Verificação cruzada final.
server_defs = parse_definitions(
    files[0].read_text(encoding="latin-1")
)
game_defs = parse_definitions(
    files[1].read_text(encoding="latin-1")
)

errors = []

for name in missing_names:
    server_value = server_defs.get(name)
    game_value = game_defs.get(name)

    if server_value is None:
        errors.append(f"{name}: ausente no servidor")
        continue

    if game_value is None:
        errors.append(f"{name}: ausente no cliente")
        continue

    if server_value != game_value:
        errors.append(
            f"{name}: servidor=0x{server_value:08X}, "
            f"cliente=0x{game_value:08X}"
        )

if errors:
    print("\nFalha na verificação:")
    for error in errors:
        print("  " + error)
    sys.exit(1)

print("\nSincronização cliente/servidor: OK")
PY

echo
echo "Definições adicionadas no servidor:"
grep -nE \
'OPCODE_BAD_PLAYER|OPCODE_CHECK_EXPDATA|OPCODE_PRIMEITEM_DATA[234]?|OPCODE_FORCEORB_DATA|OPCODE_BOOSTER_DATA|OPCODE_SKILLDELAY_DATA|CHECK_SOD_CROWN|PLAYER_GM|OPCODE_IPLIST|OPCODE_UPDATE_SERVER_PARAM' \
"$SERVER"

echo
echo "Definições adicionadas no cliente:"
grep -nE \
'OPCODE_BAD_PLAYER|OPCODE_CHECK_EXPDATA|OPCODE_PRIMEITEM_DATA[234]?|OPCODE_FORCEORB_DATA|OPCODE_BOOSTER_DATA|OPCODE_SKILLDELAY_DATA|CHECK_SOD_CROWN|PLAYER_GM|OPCODE_IPLIST|OPCODE_UPDATE_SERVER_PARAM' \
"$GAME"

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
    -c SrcServer/src/Server/OnSever.cpp \
    -o /tmp/OnSever.o \
    2>/tmp/OnSever.err || true

echo

if [ -f /tmp/OnSever.o ]; then
    echo "ONSERVER OK"
else
    echo "OnSever ainda possui erros:"
    echo

    grep -E 'error:|fatal error:' /tmp/OnSever.err \
        | sed -E 's/^.*error: //' \
        | sort -u \
        | head -160

    echo
    echo "Erros com linhas:"

    grep -E \
        'OnSever\.cpp:[0-9]+:|error:|fatal error:' \
        /tmp/OnSever.err \
        | head -240
fi
