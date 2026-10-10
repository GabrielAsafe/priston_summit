#!/usr/bin/env bash
set -u

ROOT="${1:-.}"
OUT="${2:-/tmp/summit_eragon_scfg_recovery.txt}"
RECOVER_DIR="${3:-/tmp/summit_eragon_scfg_recovered_defs}"

cd "$ROOT" || exit 1
mkdir -p "$RECOVER_DIR"
: > "$OUT"

python3 - "$PWD" "$OUT" "$RECOVER_DIR" <<'PY'
from __future__ import annotations

import hashlib
import os
import re
import struct
import sys
from pathlib import Path

ROOT = Path(sys.argv[1]).resolve()
OUT = Path(sys.argv[2]).resolve()
RECOVER = Path(sys.argv[3]).resolve()
RECOVER.mkdir(parents=True, exist_ok=True)

H = struct.Struct("<4sHH10I32s")
R = struct.Struct("<5I")
D = struct.Struct("<3I")

TARGETS = (
    "eragon",
    "lavagolem",
    "lava golem",
    "lava_golem",
    "magma",
    "rptlavagolem",
    r"char\monster\eragon",
    r"char\monster\lavagolem",
)

def dec(b: bytes) -> str:
    for enc in ("utf-8", "cp1252", "cp949", "latin1"):
        try:
            return b.decode(enc)
        except UnicodeDecodeError:
            pass
    return b.decode("latin1", errors="replace")

def norm_match_text(parts) -> str:
    return "\n".join(str(x) for x in parts).casefold().replace("/", "\\")

def safe_name(s: str) -> str:
    s = re.sub(r"[^A-Za-z0-9._-]+", "_", s).strip("._")
    return s[:120] or "definition"

def candidate_packs(root: Path):
    seen = set()
    for p in root.rglob("*.scfg"):
        name = p.name.casefold()
        if name not in ("definitions_server.scfg", "definitions_client.scfg") and "definition" not in name:
            continue
        try:
            key = (p.stat().st_dev, p.stat().st_ino)
        except OSError:
            continue
        if key in seen:
            continue
        seen.add(key)
        yield p

lines = []
def emit(s=""):
    print(s)
    lines.append(s)

packs = list(candidate_packs(ROOT))
emit("=== SUMMIT SCFG ERAGON / DEATH CAVE RECOVERY ===")
emit(f"ROOT={ROOT}")
emit(f"PACKS_FOUND={len(packs)}")
emit()

valid = 0
matches = 0

for pack in packs:
    try:
        data = pack.read_bytes()
        if len(data) < H.size:
            continue
        h = H.unpack_from(data, 0)
        magic, ver, flags, rc, ro, rs, dc, do, ds, so, ss, r0, r1, digest = h
        if magic != b"SCFG" or ver != 1:
            continue
        if ro != H.size or rs != R.size or ds != D.size:
            continue
        if do != ro + rc * rs or so != do + dc * ds or so + ss != len(data):
            continue
        if hashlib.sha256(data[H.size:]).digest() != digest:
            continue

        valid += 1

        def s(off: int) -> bytes:
            if off >= ss:
                raise ValueError("string offset out of range")
            b = so + off
            e = data.find(b"\0", b, so + ss)
            if e < 0:
                raise ValueError("unterminated string")
            return data[b:e]

        records = []
        for i in range(rc):
            po, io, kind, st, count = R.unpack_from(data, ro + i * rs)
            if st > dc or count > dc - st:
                raise ValueError("directive range invalid")
            path_b = s(po)
            id_b = s(io)
            dirs = []
            for j in range(st, st + count):
                co, vo, fl = D.unpack_from(data, do + j * ds)
                dirs.append((s(co), s(vo), fl))
            records.append((i, kind, path_b, id_b, dirs))

        for i, kind, path_b, id_b, dirs in records:
            path = dec(path_b)
            cid = dec(id_b)
            decoded_dirs = [(dec(c), dec(v), f, c, v) for c, v, f in dirs]
            hay = norm_match_text(
                [path, cid] + [x for d in decoded_dirs for x in d[:2]]
            )
            if not any(t.casefold().replace("/", "\\") in hay for t in TARGETS):
                continue

            matches += 1
            phash = hashlib.sha256(data).hexdigest()[:12]
            rel = pack.relative_to(ROOT)
            emit("=" * 78)
            emit(f"MATCH={matches}")
            emit(f"PACK={rel}")
            emit(f"PACK_SHA256={hashlib.sha256(data).hexdigest()}")
            emit(f"RECORD_INDEX={i}")
            emit(f"KIND={kind}")
            emit(f"PATH={path}")
            emit(f"ID={cid}")
            emit(f"DIRECTIVE_COUNT={len(dirs)}")

            # SCFG retains the original directive token bytes + rendered values.
            # Re-materialize a legacy-style document for forensic recovery.
            raw_lines = []
            for n, (cmd_txt, val_txt, fl, cmd_b, val_b) in enumerate(decoded_dirs):
                emit(f"DIRECTIVE[{n}] CMD={cmd_txt!r} VALUES={val_txt!r} FLAGS={fl}")
                emit(f"DIRECTIVE[{n}] CMD_HEX={cmd_b.hex()} VALUES_HEX={val_b.hex()}")
                raw_lines.append(cmd_b + (b" " + val_b if val_b else b""))

            outfile = RECOVER / f"{phash}_{i:05d}_{safe_name(cid)}.inf.recovered"
            outfile.write_bytes(b"\r\n".join(raw_lines) + b"\r\n")
            emit(f"RECOVERED_LEGACY_DOC={outfile}")
            emit()

    except Exception as e:
        emit(f"[WARN] PACK_PARSE_FAIL={pack.relative_to(ROOT)} ERROR={e}")

emit("=" * 78)
emit(f"VALID_SCFG_PACKS={valid}")
emit(f"MATCHED_DEFINITIONS={matches}")
emit(f"RECOVER_DIR={RECOVER}")
if matches == 0:
    emit("RESULT=NO_MATCH")
else:
    emit("RESULT=FOUND")

OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
PY

echo
echo "[INFO] report: $OUT"
echo "[INFO] recovered definitions: $RECOVER_DIR"
