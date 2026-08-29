#!/usr/bin/env bash
set -euo pipefail

ONSERVER="SrcServer/src/Server/OnSever.cpp"
SERVER_OPCODES="SrcServer/src/opcodes.h"
SHARED_OPCODES="Shared/opcodes.h"
COMPAT="SrcServer/src/server_compat_packets.h"

STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="backup_onserver_remaining_${STAMP}"

mkdir -p "$BACKUP/SrcServer/src/Server"
mkdir -p "$BACKUP/SrcServer/src"

cp -a "$ONSERVER" \
    "$BACKUP/SrcServer/src/Server/OnSever.cpp"

cp -a "$SERVER_OPCODES" \
    "$BACKUP/SrcServer/src/opcodes.h"

cp -a "$COMPAT" \
    "$BACKUP/SrcServer/src/server_compat_packets.h"

echo "Backup criado em: $BACKUP"

###############################################################################
# 1. Corrigir SEC_CONSOLE
#
# SEC_CONSOLE não existe no enum atual. O nível administrativo confirmado
# disponível no projeto é SEC_MODERATOR.
###############################################################################

sed -i \
    's/AccountTypes::SEC_CONSOLE/AccountTypes::SEC_MODERATOR/g' \
    "$ONSERVER"

###############################################################################
# 2. Corrigir temporariamente PRIME_ITEM_TIME_3H
#
# A constante de 3 horas não existe. Usa 6H, que está confirmada no projeto.
# Isso mantém a funcionalidade ativa, porém com duração de 6 horas.
###############################################################################

sed -i \
    's/\bPRIME_ITEM_TIME_3H\b/PRIME_ITEM_TIME_6H/g' \
    "$ONSERVER"

###############################################################################
# 3. Copiar opcodes conhecidos do Shared para o header local do servidor
#
# Não cria valores. Apenas copia definições existentes em Shared/opcodes.h.
###############################################################################

python3 - <<'PY'
from pathlib import Path
import re

server_path = Path("SrcServer/src/opcodes.h")
shared_path = Path("Shared/opcodes.h")

server = server_path.read_text(encoding="latin-1")
shared = shared_path.read_text(encoding="latin-1")

wanted = [
    "PLAYER_TOP_LEVEL",
    "UPDATE_BATTLE_POINTS",
    "SEND_ALERT_PVP",
    "DELETE_ITEM_PREMIUM",
    "SEARCH_GAME_CHEAT",
    "INIT_CHEAT_PROTECTION",
    "SET_ITEM_PREMIUM",
    "SEND_ALERT",
    "BATTLE_POINTS",
    "BOSS_TIME",
    "FIND_CHEAT",
    "OPCODE_GET_RESTAURE",
    "OPCODE_ID_GETUSERINFO",
    "UPDATE_ITEM_PREMIUM",
]

definitions = {}

pattern = re.compile(
    r"static\s+const\s+int\s+"
    r"([A-Za-z_][A-Za-z0-9_]*)"
    r"\s*=\s*([^;]+);"
)

for match in pattern.finditer(shared):
    definitions.setdefault(
        match.group(1),
        match.group(2).strip()
    )

missing_definitions = []

to_insert = []

for name in wanted:
    already_present = re.search(
        rf"\bstatic\s+const\s+int\s+{re.escape(name)}\b",
        server
    )

    if already_present:
        continue

    value = definitions.get(name)

    if value is None:
        missing_definitions.append(name)
        continue

    to_insert.append(
        f"    static const int {name} = {value};"
    )

if to_insert:
    # O header local termina com o fechamento do namespace OpCode.
    close_pos = server.rfind("}")

    if close_pos < 0:
        raise SystemExit(
            "Não encontrei o fechamento do namespace em "
            "SrcServer/src/opcodes.h"
        )

    block = (
        "\n\n"
        "    // Compatibilidade recuperada de Shared/opcodes.h\n"
        + "\n".join(to_insert)
        + "\n"
    )

    server = server[:close_pos] + block + server[close_pos:]

server_path.write_text(server, encoding="latin-1")

print("OpCodes copiados:", len(to_insert))

for line in to_insert:
    print("  " + line.strip())

if missing_definitions:
    print("\nNão encontrados no Shared:")
    for name in missing_definitions:
        print("  " + name)
PY

###############################################################################
# 4. Criar aliases globais apenas para símbolos usados sem OpCode::
###############################################################################

python3 - <<'PY'
from pathlib import Path
import re

path = Path("SrcServer/src/Server/OnSever.cpp")
text = path.read_text(encoding="latin-1")

aliases = [
    "UPDATE_BATTLE_POINTS",
    "DELETE_ITEM_PREMIUM",
]

marker = "/* SUMMIT_ONSERVER_GLOBAL_OPCODE_ALIASES */"

if marker not in text:
    lines = [
        "",
        marker,
    ]

    for name in aliases:
        # Só adiciona alias se o código realmente usa o nome global.
        global_use = re.search(
            rf"(?<!OpCode::)\b{re.escape(name)}\b",
            text
        )

        if global_use:
            lines.append(
                f"static const int {name} = OpCode::{name};"
            )

    lines.append("")

    block = "\n".join(lines)

    include_pattern = re.compile(
        r'(#include\s+"server_build_compat\.h"\s*\n)'
    )

    text, count = include_pattern.subn(
        r"\1" + block,
        text,
        count=1
    )

    if count != 1:
        raise SystemExit(
            "Não encontrei include de server_build_compat.h"
        )

path.write_text(text, encoding="latin-1")
PY

###############################################################################
# 5. Desativar o bloco OPCODE_PLAYERKILLING
#
# Esse bloco depende de rsPLAYINFO::KillCount e KillTime, que não existem.
###############################################################################

python3 - <<'PY'
from pathlib import Path
import re

path = Path("SrcServer/src/Server/OnSever.cpp")
text = path.read_text(encoding="latin-1")

if "SUMMIT_DISABLED_PLAYERKILLING_BEGIN" not in text:
    pattern = re.compile(
        r"(?P<indent>^[ \t]*)"
        r"case\s+OpCode::OPCODE_PLAYERKILLING\s*:"
        r"(?P<body>.*?)"
        r"(?P=indent)break\s*;",
        flags=re.M | re.S
    )

    match = pattern.search(text)

    if match:
        original = match.group(0)
        indent = match.group("indent")

        replacement = (
            f"{indent}#if 0 "
            f"/* SUMMIT_DISABLED_PLAYERKILLING_BEGIN\n"
            f"{original}\n"
            f"{indent}SUMMIT_DISABLED_PLAYERKILLING_END */\n"
            f"{indent}#endif"
        )

        text = (
            text[:match.start()]
            + replacement
            + text[match.end():]
        )

        print("Bloco OPCODE_PLAYERKILLING desativado")
    else:
        print(
            "Aviso: bloco OPCODE_PLAYERKILLING "
            "não foi localizado automaticamente"
        )
else:
    print("Bloco OPCODE_PLAYERKILLING já estava desativado")

path.write_text(text, encoding="latin-1")
PY

###############################################################################
# 6. Garantir PACKET_KILL e PACKET_BUFFS a partir do cliente
###############################################################################

python3 - <<'PY'
from pathlib import Path
import re

client_path = Path("SrcGame/src/netplay.cpp")
compat_path = Path("SrcServer/src/server_compat_packets.h")

client = client_path.read_text(encoding="latin-1")
compat = compat_path.read_text(encoding="latin-1")

blocks = []

if "struct PACKET_BUFFS" not in compat:
    match = re.search(
        r"struct\s+PACKET_BUFFS\s*\{.*?\};",
        client,
        flags=re.S
    )

    if match:
        blocks.append(
            "\n#ifndef SUMMIT_PACKET_BUFFS_DEFINED\n"
            "#define SUMMIT_PACKET_BUFFS_DEFINED\n"
            f"{match.group(0)}\n"
            "#endif\n"
        )
    else:
        print("Aviso: PACKET_BUFFS não encontrada no cliente")

if not re.search(
    r"#define\s+PACKET_KILL\b",
    compat
):
    match = re.search(
        r"#define\s+PACKET_KILL\s+([^\r\n]+)",
        client
    )

    if match:
        blocks.append(
            "\n#ifndef PACKET_KILL\n"
            f"#define PACKET_KILL {match.group(1).strip()}\n"
            "#endif\n"
        )
    else:
        print("Aviso: PACKET_KILL não encontrado no cliente")

if blocks:
    marker = "\n#endif"

    pos = compat.rfind(marker)

    if pos < 0:
        raise SystemExit(
            "Não encontrei o #endif final em "
            "server_compat_packets.h"
        )

    compat = (
        compat[:pos]
        + "\n".join(blocks)
        + compat[pos:]
    )

    compat_path.write_text(compat, encoding="latin-1")
    print("Compatibilidade PACKET_BUFFS/PACKET_KILL adicionada")
else:
    print("PACKET_BUFFS/PACKET_KILL já estavam disponíveis")
PY

###############################################################################
# 7. Verificações
###############################################################################

echo
echo "Verificações:"
echo

grep -n \
    'SEC_CONSOLE\|PRIME_ITEM_TIME_3H\|AccountTypesMap' \
    "$ONSERVER" || true

grep -n \
    'SUMMIT_DISABLED_PLAYERKILLING' \
    "$ONSERVER" || true

grep -nE \
    'PLAYER_TOP_LEVEL|SEND_ALERT|BATTLE_POINTS|BOSS_TIME|OPCODE_GET_RESTAURE' \
    "$SERVER_OPCODES" | tail -30 || true

###############################################################################
# 8. Compilar OnSever.cpp
###############################################################################

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
    -c "$ONSERVER" \
    -o /tmp/OnSever.o \
    2>/tmp/OnSever.err || true

echo

if [ -f /tmp/OnSever.o ]
then
    echo "ONSERVER OK"
else
    echo "OnSever ainda possui erros:"
    echo

    grep -E 'error:|fatal error:' /tmp/OnSever.err |
        sed -E 's/^.*error: //' |
        sort -u |
        head -160
fi
