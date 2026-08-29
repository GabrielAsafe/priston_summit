#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-.}"
cd "$ROOT"

STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP_DIR="backup_auto_patch_onsever_${STAMP}"

mkdir -p "$BACKUP_DIR"

backup_file() {
    local f="$1"
    if [[ -f "$f" ]]; then
        mkdir -p "$BACKUP_DIR/$(dirname "$f")"
        cp -a "$f" "$BACKUP_DIR/$f"
    fi
}

require_file() {
    local f="$1"
    if [[ ! -f "$f" ]]; then
        echo "ERRO: ficheiro não encontrado: $f" >&2
        exit 1
    fi
}

ONSEVER="SrcServer/src/Server/OnSever.cpp"
OPCODES="Shared/opcodes.h"
COMPAT_PACKETS="SrcServer/src/server_compat_packets.h"
SM_PACKET="Shared/smPacket.h"

require_file "$ONSEVER"
require_file "$OPCODES"
require_file "$COMPAT_PACKETS"
require_file "$SM_PACKET"

backup_file "$ONSEVER"
backup_file "$OPCODES"
backup_file "$COMPAT_PACKETS"
backup_file "$SM_PACKET"

python3 - <<'PY'
from pathlib import Path
import re

onserver = Path("SrcServer/src/Server/OnSever.cpp")
opcodes = Path("Shared/opcodes.h")
compat = Path("SrcServer/src/server_compat_packets.h")
sm_packet = Path("Shared/smPacket.h")

def read(path):
    return path.read_text(encoding="latin-1")

def write(path, text):
    path.write_text(text, encoding="latin-1")

# 1. Includes e aliases confirmados.
text = read(onserver)

includes = [
    '#include "server_build_compat.h"',
    '#include "server_compat_packets.h"',
    '#include "Helper/Utils/utils_logging.h"',
]

for inc in reversed(includes):
    if inc not in text:
        text = inc + "\n" + text

if "#define LOG_SERVER 1" not in text:
    marker = '#include "Helper/Utils/utils_logging.h"\n'
    text = text.replace(
        marker,
        marker + "\n#ifndef LOG_SERVER\n#define LOG_SERVER 1\n#endif\n",
        1
    )

text = re.sub(r'(?<!OpCode::)\bSEND_ALERT_PVP\b', 'OpCode::SEND_ALERT_PVP', text)
text = re.sub(r'(?<!OpCode::)\bSEND_ALERT\b', 'OpCode::SEND_ALERT', text)
text = re.sub(r'(?<!OpCode::)\bOPCODE_AGING_FREE\b', 'OpCode::OPCODE_AGING_FREE', text)
text = re.sub(r'(?<!OpCode::)\bOPCODE_WHISPERMESSAGE\b', 'OpCode::OPCODE_WHISPERMESSAGE', text)
text = text.replace("OpCode::OpCode::", "OpCode::")

text = re.sub(r'\bsmTRANS_QCOMMAND_QWORD\b', 'smTRANS_COMMAND_QWORD', text)
text = re.sub(r'\bsmTRANS_QCOMMAND\b', 'smTRANS_COMMAND', text)

text = re.sub(
    r'([A-Za-z0-9_\.\[\]\->]+\.getAccount\(\))\.length\(\)',
    r'lstrlenA(\1)',
    text
)
text = re.sub(
    r'([A-Za-z0-9_\.\[\]\->]+\.getName\(\))\.length\(\)',
    r'lstrlenA(\1)',
    text
)
text = re.sub(
    r'([A-Za-z0-9_\.\[\]\->]+->getAccount\(\))\.length\(\)',
    r'lstrlenA(\1)',
    text
)
text = re.sub(
    r'([A-Za-z0-9_\.\[\]\->]+->getName\(\))\.length\(\)',
    r'lstrlenA(\1)',
    text
)

text = re.sub(
    r'(\b[A-Za-z_][A-Za-z0-9_]*\s*->\s*)setSecurity\(([^;]+)\);',
    r'\1AdminMode = static_cast<int>(\2);',
    text
)

write(onserver, text)

# 2. Opcode confirmado no cliente.
text = read(opcodes)

if "PLAYER_TOP_LEVEL" not in text:
    ns_match = re.search(r'namespace\s+OpCode\s*\{', text)
    if not ns_match:
        raise SystemExit("ERRO: namespace OpCode não encontrado em Shared/opcodes.h")

    insert_pos = ns_match.end()
    text = (
        text[:insert_pos]
        + "\n    static const int PLAYER_TOP_LEVEL = 0x4847900C;\n"
        + text[insert_pos:]
    )

write(opcodes, text)

# 3. Packets copiados do cliente atual.
text = read(compat)

packet_block = '''
#ifndef SUMMIT_SERVER_NETPLAY_COMPAT_TYPES
#define SUMMIT_SERVER_NETPLAY_COMPAT_TYPES

struct smRANKING
{
    int size, code;
    int type;
    int contador;
    char rankName[100][32];
    char rankNameClan[100][32];
    char rankLiderClan[100][32];
    int rankLevel[100];
    int rankClasse[100];
    int rankKills[100];
    int rankDeads[100];
    int rankPointsSod[100];
    int rankmembros[100];
};

struct TRANS_PREMIUM_TIME
{
    int size, code;
    DWORD ThirdEye;
    DWORD ExpUp;
    DWORD VampCuspid;
    DWORD ManaRecharg;
    DWORD StaminaRecharg;
    DWORD ManaReduce;
    DWORD MightofAwell;
    DWORD MightofAwell2;
    DWORD PhenixPet;
    DWORD HelpPet;
    DWORD VampCuspidEx;
    DWORD StaminaReduce;
    DWORD Caravan;
    DWORD GoldLooter;
    DWORD HPBooster;
    DWORD MPBooster;
    DWORD RESBooster;
};

#ifndef smTRANSCODE_OPEN_NPCinfo
#define smTRANSCODE_OPEN_NPCinfo 0x49470022
#endif

#endif
'''

if "SUMMIT_SERVER_NETPLAY_COMPAT_TYPES" not in text:
    text += "\n" + packet_block

write(compat, text)

# 4. Campos comprovadamente usados por OnSever.cpp.
lines = read(sm_packet).splitlines()

def has_field(name):
    return any(re.search(rf'\b{re.escape(name)}\b', line) for line in lines)

field_after_badplayer = [
    ("topLevelFlag", "bool    topLevelFlag;"),
    ("lastMessageTick", "DWORD   lastMessageTick;"),
]

for name, decl in field_after_badplayer:
    if not has_field(name):
        for i, line in enumerate(lines):
            if "BadPlayer" in line and "int" in line:
                indent = line[:len(line) - len(line.lstrip())]
                lines.insert(i + 1, indent + decl)
                break
        else:
            raise SystemExit(f"ERRO: não encontrei BadPlayer para inserir {name}")

premium_fields = [
    ("dwPrimeItem_NextSetTime_T", "DWORD   dwPrimeItem_NextSetTime_T;"),
    ("dwTime_BigHead1", "DWORD   dwTime_BigHead1;"),
    ("dwTime_BigHead2", "DWORD   dwTime_BigHead2;"),
    ("dwTime_BigHead3", "DWORD   dwTime_BigHead3;"),
    ("dwTime_BigHead4", "DWORD   dwTime_BigHead4;"),
]

anchor_index = None
for i, line in enumerate(lines):
    if "dwPrimeItem_NextSetTime" in line and "DWORD" in line:
        anchor_index = i
        break

if anchor_index is None:
    raise SystemExit("ERRO: não encontrei dwPrimeItem_NextSetTime em Shared/smPacket.h")

offset = 1
for name, decl in premium_fields:
    if not has_field(name):
        indent = lines[anchor_index][:len(lines[anchor_index]) - len(lines[anchor_index].lstrip())]
        lines.insert(anchor_index + offset, indent + decl)
        offset += 1

write(sm_packet, "\n".join(lines) + "\n")

print("Patch automático aplicado.")
PY

echo
echo "Backup criado em: $BACKUP_DIR"
echo
echo "Verificações:"
grep -n "PLAYER_TOP_LEVEL" "$OPCODES" || true
grep -nE 'topLevelFlag|lastMessageTick|dwPrimeItem_NextSetTime_T|dwTime_BigHead[1-4]' "$SM_PACKET" || true
grep -nE 'struct smRANKING|struct TRANS_PREMIUM_TIME|smTRANSCODE_OPEN_NPCinfo' "$COMPAT_PACKETS" || true

echo
echo "Compilando OnSever.cpp..."
rm -f /tmp/OnSever.o /tmp/OnSever.err

i686-w64-mingw32-g++     -m32 -std=gnu++14 -fpermissive -w     -fmax-errors=300     -DWIN32 -D_WINDOWS -D_M_IX86 -Di386 -D__i386__ -D_SERVER     -I.     -ISrcServer/src     -ISrcServer/src/sinbaram     -ISrcServer/src/Skills     -ISrcGame/src     -IShared -IShared/Lua -IShared2     -Imodules -Imodules/File     "-IDependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include"     -c "$ONSEVER"     -o /tmp/OnSever.o     2>/tmp/OnSever.err || true

if [[ -f /tmp/OnSever.o ]]; then
    echo "ONSEVER OK"
else
    echo
    echo "OnSever ainda possui erros. Resumo único:"
    grep -E 'error:|fatal error:' /tmp/OnSever.err         | sed -E 's/^.*error: //'         | sort -u         | head -120
fi
