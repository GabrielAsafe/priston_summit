#!/usr/bin/env bash

ROOT="/home/gabriel/Área de Trabalho/summit"
REPO="$ROOT"
CUR="$ROOT/SourceCode"
OUT="$ROOT/bugfixes/038_monster_crash_symbolization"

mkdir -p "$OUT"

echo "=== 038 — SYMBOLIZE MONSTER CRASH RVA 0x11DCE ==="
echo

# Prefer the exact MinGW server artifact from the build tree.
EXE="$CUR/dist_mingw_server_i686/Server.exe"

if [ ! -f "$EXE" ]; then
    EXE="$(find "$CUR" -type f -iname 'Server.exe' -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -n 1 | cut -d' ' -f2-)"
fi

echo "EXE=$EXE"

if [ -z "$EXE" ] || [ ! -f "$EXE" ]; then
    echo "ERRO: Server.exe não encontrado no SourceCode."
else
    sha256sum "$EXE" | tee "$OUT/server_exe.sha256"

    ADDR2LINE=""
    OBJDUMP=""
    NM=""

    for c in i686-w64-mingw32-addr2line addr2line llvm-addr2line; do
        if command -v "$c" >/dev/null 2>&1; then
            ADDR2LINE="$c"
            break
        fi
    done

    for c in i686-w64-mingw32-objdump objdump llvm-objdump; do
        if command -v "$c" >/dev/null 2>&1; then
            OBJDUMP="$c"
            break
        fi
    done

    for c in i686-w64-mingw32-nm nm llvm-nm; do
        if command -v "$c" >/dev/null 2>&1; then
            NM="$c"
            break
        fi
    done

    {
        echo "ADDR2LINE=$ADDR2LINE"
        echo "OBJDUMP=$OBJDUMP"
        echo "NM=$NM"
    } | tee "$OUT/tools.txt"

    IMAGE_BASE=""

    if [ -n "$OBJDUMP" ]; then
        "$OBJDUMP" -p "$EXE" > "$OUT/objdump_headers.txt" 2>&1
        IMAGE_BASE="$("$OBJDUMP" -p "$EXE" 2>/dev/null | awk '
            tolower($1)=="imagebase" {
                gsub(/^0x/,"",$2);
                print $2;
                found=1;
                exit
            }
            END {
                if (!found) {}
            }
        ')"
    fi

    echo "IMAGE_BASE_HEX=$IMAGE_BASE" | tee "$OUT/image_base.txt"

    python3 - "$IMAGE_BASE" "$OUT/addresses.txt" <<'PY'
from pathlib import Path
import sys

base_s = sys.argv[1].strip()
out = Path(sys.argv[2])

# Crash RVA plus return-address-looking RVAs from the captured stack.
rvas = [
    0x00011DCE,  # exception
    0x00707F1D,
    0x0017AD92,
    0x0044C8E9,
    0x0017E370,
    0x0057DE38,
    0x0017F09A,
    0x0044C921,
    0x0044CD70,
    0x00186B0D,
    0x003652FC,
    0x0044C500,
    0x0016DBA1,
    0x0044C521,
    0x001B820D,
    0x0044C461,
    0x001B8B6B,
    0x0044FD80,
    0x0044FD5D,
    0x00074934,
    0x0070B180,
]

base = None
if base_s:
    try:
        base = int(base_s, 16)
    except ValueError:
        base = None

lines = []
for rva in rvas:
    if base is None:
        lines.append(f"RVA 0x{rva:08X}")
    else:
        lines.append(
            f"RVA 0x{rva:08X} PREFERRED_VA 0x{base+rva:08X}"
        )

out.write_text("\n".join(lines) + "\n", encoding="utf-8")
print("\n".join(lines))
PY

    if [ -n "$ADDR2LINE" ]; then
        echo
        echo "=== ADDR2LINE ==="

        python3 - "$IMAGE_BASE" "$EXE" "$ADDR2LINE" "$OUT/addr2line.txt" <<'PY'
import subprocess
from pathlib import Path
import sys

base_s, exe, tool, out_s = sys.argv[1:5]
out = Path(out_s)

rvas = [
    0x00011DCE,
    0x00707F1D,
    0x0017AD92,
    0x0044C8E9,
    0x0017E370,
    0x0057DE38,
    0x0017F09A,
    0x0044C921,
    0x0044CD70,
    0x00186B0D,
    0x003652FC,
    0x0044C500,
    0x0016DBA1,
    0x0044C521,
    0x001B820D,
    0x0044C461,
    0x001B8B6B,
    0x0044FD80,
    0x0044FD5D,
    0x00074934,
    0x0070B180,
]

base = None
if base_s:
    try:
        base = int(base_s, 16)
    except ValueError:
        pass

report = []

for rva in rvas:
    candidates = [("RVA", rva)]
    if base is not None:
        candidates.append(("PREFERRED_VA", base + rva))

    report.append(f"===== CRASH_RVA 0x{rva:08X} =====")

    for label, addr in candidates:
        p = subprocess.run(
            [tool, "-f", "-C", "-i", "-e", exe, f"0x{addr:X}"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
        )
        report.append(f"[{label} 0x{addr:08X}]")
        report.append(p.stdout.strip())

text = "\n".join(report) + "\n"
out.write_text(text, encoding="utf-8")
print(text)
PY
    else
        echo "AVISO: addr2line não encontrado."
    fi

    if [ -n "$NM" ]; then
        "$NM" -n -C "$EXE" > "$OUT/nm_sorted.txt" 2>&1

        python3 - "$IMAGE_BASE" "$OUT/nm_sorted.txt" "$OUT/nearest_symbols.txt" <<'PY'
from pathlib import Path
import sys, re

base_s = sys.argv[1].strip()
nm_path = Path(sys.argv[2])
out = Path(sys.argv[3])

targets_rva = [
    0x00011DCE,
    0x0017AD92,
    0x0017E370,
    0x0017F09A,
    0x00186B0D,
    0x0016DBA1,
    0x001B820D,
    0x001B8B6B,
    0x00074934,
]

base = None
if base_s:
    try:
        base = int(base_s, 16)
    except ValueError:
        pass

symbols = []
for line in nm_path.read_text(errors="replace").splitlines():
    m = re.match(r"^\s*([0-9A-Fa-f]+)\s+\S\s+(.*)$", line)
    if m:
        symbols.append((int(m.group(1),16), m.group(2)))

symbols.sort()

def nearest(addr):
    prev = None
    nxt = None
    for s in symbols:
        if s[0] <= addr:
            prev = s
        else:
            nxt = s
            break
    return prev, nxt

report = []
for rva in targets_rva:
    report.append(f"===== RVA 0x{rva:08X} =====")
    candidates = [("RVA", rva)]
    if base is not None:
        candidates.append(("PREFERRED_VA", base+rva))
    for label, addr in candidates:
        prev, nxt = nearest(addr)
        report.append(f"[{label} 0x{addr:08X}]")
        if prev:
            report.append(
                f"PREV 0x{prev[0]:08X} delta=0x{addr-prev[0]:X} {prev[1]}"
            )
        if nxt:
            report.append(
                f"NEXT 0x{nxt[0]:08X} delta=0x{nxt[0]-addr:X} {nxt[1]}"
            )

out.write_text("\n".join(report) + "\n", encoding="utf-8")
print("\n".join(report))
PY
    else
        echo "AVISO: nm não encontrado."
    fi

    if [ -n "$OBJDUMP" ]; then
        # Full disassembly is saved, then a focused window around the crash
        # address is extracted using preferred VA when ImageBase is known.
        "$OBJDUMP" -d -C "$EXE" > "$OUT/disassembly.txt" 2>&1

        python3 - "$IMAGE_BASE" "$OUT/disassembly.txt" "$OUT/crash_disassembly_window.txt" <<'PY'
from pathlib import Path
import sys, re

base_s = sys.argv[1].strip()
src = Path(sys.argv[2])
out = Path(sys.argv[3])

rva = 0x11DCE
base = None
if base_s:
    try:
        base = int(base_s,16)
    except ValueError:
        pass

targets = {rva}
if base is not None:
    targets.add(base+rva)

lines = src.read_text(errors="replace").splitlines()

best = None
for i,line in enumerate(lines):
    m = re.match(r"^\s*([0-9A-Fa-f]+):", line)
    if not m:
        continue
    addr = int(m.group(1),16)
    for target in targets:
        delta = abs(addr-target)
        if best is None or delta < best[0]:
            best = (delta, i, addr, target)

if best is None:
    text = "Nenhuma linha de disassembly encontrada.\n"
else:
    _, i, addr, target = best
    lo = max(0, i-30)
    hi = min(len(lines), i+31)
    text = (
        f"nearest_instruction=0x{addr:08X}\n"
        f"target=0x{target:08X}\n"
        + "\n".join(lines[lo:hi])
        + "\n"
    )

out.write_text(text, encoding="utf-8")
print(text)
PY
    fi

    {
        echo "=== SOURCE SEARCH — downstream monster path ==="
        grep -RIn \
            --include='*.cpp' --include='*.h' \
            -E 'DecodeOpenMonster|rsMonster|MonCnt|LoadOftenMeshPattern|InitMonster|OpenMonster|CreateMonster' \
            "$CUR/SrcServer/src" 2>/dev/null | head -n 500
    } > "$OUT/monster_path_refs.txt"

    echo
    echo "=== OUTPUT SUMMARY ==="
    echo "$OUT/addr2line.txt"
    echo "$OUT/nearest_symbols.txt"
    echo "$OUT/crash_disassembly_window.txt"
    echo "$OUT/monster_path_refs.txt"
    echo
    echo "Nenhum source foi alterado."
fi
