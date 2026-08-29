#!/usr/bin/env bash

ROOT="/home/gabriel/Área de Trabalho/summit"
REPO="$ROOT"
CUR="$ROOT/SourceCode"
BASE="/home/gabriel/Transferências/summit_pre_reset_restored_20260824/SourceCode"
REC="/home/gabriel/Transferências/recovery_pre_reset_20260824_0958"
REC_PATCH="$REC/working_tree_vs_HEAD.patch"
AUD040="$ROOT/bugfixes/040_recovery_chat_white_image_audit"
OUT="$ROOT/bugfixes/042_chat_white_ui_semantic_audit"

TARGET_REL="SourceCode/SrcGame/src/sinbaram/sinInterFace.cpp"
TARGET_CUR="$ROOT/$TARGET_REL"
TARGET_BASE="$BASE/SrcGame/src/sinbaram/sinInterFace.cpp"

mkdir -p "$OUT"

echo "=== 042 — CHAT WHITE UI SEMANTIC AUDIT ==="
echo

HEAD="$(git -C "$REPO" rev-parse HEAD 2>/dev/null)"
printf '%s\n' "$HEAD" > "$OUT/base_commit.txt"

{
    echo "HEAD=$HEAD"
    echo "TARGET=$TARGET_REL"
    echo "CURRENT=$TARGET_CUR"
    echo "BASE=$TARGET_BASE"
    echo "RECOVERY_PATCH=$REC_PATCH"
    echo
    echo "READ ONLY: nenhum source será alterado."
} | tee "$OUT/00_context.txt"

if [ ! -f "$TARGET_CUR" ] || [ ! -f "$TARGET_BASE" ] || [ ! -f "$REC_PATCH" ]; then
    echo "ERRO: arquivo necessário ausente."
else
    sha256sum "$TARGET_CUR" "$TARGET_BASE" > "$OUT/01_hashes.sha256"

    python3 - "$TARGET_CUR" "$TARGET_BASE" "$REC_PATCH" "$OUT" <<'PY'
from pathlib import Path
import sys, re, difflib, hashlib, unicodedata

cur_p = Path(sys.argv[1])
base_p = Path(sys.argv[2])
rec_patch = Path(sys.argv[3])
out = Path(sys.argv[4])

cur_b = cur_p.read_bytes()
base_b = base_p.read_bytes()
patch_b = rec_patch.read_bytes()

def decode_best(b):
    for enc in ("utf-8-sig", "cp949", "cp1252", "latin1"):
        try:
            return b.decode(enc), enc
        except UnicodeDecodeError:
            pass
    return b.decode("latin1", errors="replace"), "latin1-replace"

cur_t, cur_enc = decode_best(cur_b)
base_t, base_enc = decode_best(base_b)

(out / "02_encoding.txt").write_text(
    f"CURRENT_ENCODING={cur_enc}\n"
    f"BASE_ENCODING={base_enc}\n"
    f"CURRENT_EF_BF_BD={cur_b.count(bytes.fromhex('efbfbd'))}\n"
    f"BASE_EF_BF_BD={base_b.count(bytes.fromhex('efbfbd'))}\n",
    encoding="utf-8"
)

# Extract recovery block for exactly this file.
m = re.search(
    rb"(?ms)^diff --git a/SourceCode/SrcGame/src/sinbaram/sinInterFace\.cpp "
    rb"b/SourceCode/SrcGame/src/sinbaram/sinInterFace\.cpp\n"
    rb"(.*?)(?=^diff --git |\Z)",
    patch_b,
)
if not m:
    raise SystemExit("ERRO: bloco recovery de sinInterFace.cpp não encontrado")

block = (
    b"diff --git a/SourceCode/SrcGame/src/sinbaram/sinInterFace.cpp "
    b"b/SourceCode/SrcGame/src/sinbaram/sinInterFace.cpp\n" + m.group(1)
)
(out / "03_recovery_file.patch").write_bytes(block)

# Normalize lines for semantic comparison: remove CR, trim trailing whitespace,
# normalize Unicode only for analysis, never for output/source.
def norm_lines(text):
    lines = []
    for ln in text.replace("\r\n", "\n").replace("\r", "\n").splitlines():
        ln = ln.rstrip()
        ln = unicodedata.normalize("NFC", ln)
        lines.append(ln)
    return lines

cur_n = norm_lines(cur_t)
base_n = norm_lines(base_t)

sem_diff = list(difflib.unified_diff(
    base_n, cur_n,
    fromfile="BASE_normalized",
    tofile="CURRENT_normalized",
    lineterm=""
))
(out / "04_base_vs_current_normalized.diff").write_text(
    "\n".join(sem_diff) + ("\n" if sem_diff else ""),
    encoding="utf-8",
)

# Exact source references relevant to classic chat UI and white rectangles.
terms = [
    "chat", "Chat", "CHAT",
    "whisper", "Whisper",
    "message", "Message",
    "talk", "Talk",
    "Draw", "draw", "Render", "render",
    "Texture", "texture",
    "Image", "image",
    "Material", "material",
    "Alpha", "alpha",
    "Blend", "blend",
    "Color", "color",
    "Back", "back",
    "Font", "font",
    "CreateTextureMaterial",
    "dsDrawTexImage",
    "smMaterial",
]

term_re = re.compile("|".join(re.escape(x) for x in terms))

def collect_context(lines, radius=5):
    hit_idx = set()
    for i, ln in enumerate(lines):
        if term_re.search(ln):
            for j in range(max(0, i-radius), min(len(lines), i+radius+1)):
                hit_idx.add(j)

    result = []
    prev = None
    for i in sorted(hit_idx):
        if prev is None or i != prev + 1:
            result.append(f"\n===== around line {i+1} =====")
        result.append(f"{i+1:5d}: {lines[i]}")
        prev = i
    return result

(out / "05_current_chat_render_context.txt").write_text(
    "\n".join(collect_context(cur_t.replace("\r\n","\n").splitlines())) + "\n",
    encoding="utf-8",
    errors="replace"
)
(out / "06_base_chat_render_context.txt").write_text(
    "\n".join(collect_context(base_t.replace("\r\n","\n").splitlines())) + "\n",
    encoding="utf-8",
    errors="replace"
)

# Parse normalized diff into hunks and score for chat/render relevance.
hunks = []
current = []
for ln in sem_diff:
    if ln.startswith("@@ "):
        if current:
            hunks.append(current)
        current = [ln]
    elif current:
        current.append(ln)
if current:
    hunks.append(current)

def hunk_score(h):
    text = "\n".join(h)
    score = 0
    reasons = []
    groups = {
        "CHAT": r"chat|whisper|message|talk",
        "TEXTURE": r"texture|image|material|\.png|\.bmp|\.tga|CreateTextureMaterial",
        "RENDER": r"render|draw|dsDrawTexImage|alpha|blend|color",
        "FONT": r"font|text|string",
    }
    for name, pat in groups.items():
        if re.search(pat, text, re.I):
            reasons.append(name)
            score += {"CHAT":50,"TEXTURE":40,"RENDER":40,"FONT":15}[name]
    changed = sum(1 for x in h if x.startswith("+") or x.startswith("-"))
    score += min(changed, 20)
    return score, reasons

ranked = []
for idx, h in enumerate(hunks, 1):
    score, reasons = hunk_score(h)
    ranked.append((score, idx, reasons, h))

ranked.sort(reverse=True)

report = []
for score, idx, reasons, h in ranked:
    if score <= 0:
        continue
    report.append(
        f"================ HUNK {idx} score={score} "
        f"reasons={','.join(reasons) or '-'} ================"
    )
    report.extend(h)
    report.append("")

(out / "07_ranked_semantic_hunks.txt").write_text(
    "\n".join(report) + "\n",
    encoding="utf-8",
)

# Search recovery patch lines themselves for chat/render terms.
patch_lines = block.decode("latin1", errors="replace").splitlines()
recovery_hits = []
for i, ln in enumerate(patch_lines):
    if (ln.startswith("+") or ln.startswith("-")) and term_re.search(ln):
        recovery_hits.append((i, ln))

ctx_idx = set()
for i,_ in recovery_hits:
    for j in range(max(0,i-4), min(len(patch_lines),i+5)):
        ctx_idx.add(j)

rec_context = []
prev=None
for i in sorted(ctx_idx):
    if prev is None or i != prev+1:
        rec_context.append(f"\n===== recovery patch line {i+1} =====")
    rec_context.append(f"{i+1:5d}: {patch_lines[i]}")
    prev=i

(out / "08_recovery_chat_render_changed_context.txt").write_text(
    "\n".join(rec_context) + "\n",
    encoding="utf-8",
)

# Detect image paths in current/base and differences.
path_re = re.compile(
    r'["\']([^"\']+\.(?:png|bmp|tga|jpg|jpeg|dds))["\']',
    re.I
)
cur_paths = sorted(set(path_re.findall(cur_t)))
base_paths = sorted(set(path_re.findall(base_t)))

only_cur = sorted(set(cur_paths)-set(base_paths))
only_base = sorted(set(base_paths)-set(cur_paths))

path_report = [
    "=== ONLY CURRENT IMAGE PATHS ===",
    *only_cur,
    "",
    "=== ONLY BASE IMAGE PATHS ===",
    *only_base,
]
(out / "09_image_path_differences.txt").write_text(
    "\n".join(path_report) + "\n",
    encoding="utf-8",
)

# Functions whose normalized body differs and that mention chat/render vocabulary.
func_re = re.compile(
    r'(?m)^[\w:<>&*\s~]+\([^;{}]*\)\s*\{'
)

def extract_functions(text):
    funcs = []
    for m in func_re.finditer(text):
        start = m.start()
        brace = text.find("{", m.start(), m.end()+1)
        if brace < 0:
            continue
        depth=0
        i=brace
        in_str=None
        esc=False
        while i < len(text):
            ch=text[i]
            if in_str:
                if esc:
                    esc=False
                elif ch=="\\":
                    esc=True
                elif ch==in_str:
                    in_str=None
            else:
                if ch in ("'", '"'):
                    in_str=ch
                elif ch=="{":
                    depth+=1
                elif ch=="}":
                    depth-=1
                    if depth==0:
                        header=text[start:brace].strip().replace("\n"," ")
                        funcs.append((header, text[start:i+1]))
                        break
            i+=1
    return funcs

cur_funcs = {h:b for h,b in extract_functions(cur_t)}
base_funcs = {h:b for h,b in extract_functions(base_t)}

f_report=[]
for header in sorted(set(cur_funcs) & set(base_funcs)):
    cb=cur_funcs[header]
    bb=base_funcs[header]
    if norm_lines(cb) == norm_lines(bb):
        continue
    combined = cb + "\n" + bb
    if re.search(r"chat|whisper|message|talk|texture|image|material|render|draw|alpha|blend|font|color", combined, re.I):
        f_report.append(f"===== {header} =====")
        fd=list(difflib.unified_diff(
            norm_lines(bb), norm_lines(cb),
            fromfile="BASE", tofile="CURRENT", lineterm=""
        ))
        f_report.extend(fd)
        f_report.append("")

(out / "10_changed_chat_render_functions.diff").write_text(
    "\n".join(f_report) + "\n",
    encoding="utf-8"
)

summary = [
    "=== SUMMARY ===",
    f"CURRENT_ENCODING={cur_enc}",
    f"BASE_ENCODING={base_enc}",
    f"CURRENT_BYTES={len(cur_b)}",
    f"BASE_BYTES={len(base_b)}",
    f"CURRENT_SHA256={hashlib.sha256(cur_b).hexdigest()}",
    f"BASE_SHA256={hashlib.sha256(base_b).hexdigest()}",
    f"NORMALIZED_DIFF_LINES={len(sem_diff)}",
    f"NORMALIZED_HUNKS={len(hunks)}",
    f"RECOVERY_PATCH_CHANGED_CHAT_RENDER_LINES={len(recovery_hits)}",
    f"ONLY_CURRENT_IMAGE_PATHS={len(only_cur)}",
    f"ONLY_BASE_IMAGE_PATHS={len(only_base)}",
]
(out / "11_summary.txt").write_text(
    "\n".join(summary) + "\n",
    encoding="utf-8"
)
print("\n".join(summary))

print()
print("=== TOP SEMANTIC HUNKS ===")
for score, idx, reasons, h in ranked[:15]:
    if score <= 0:
        continue
    print(
        f"HUNK {idx}: score={score} reasons={','.join(reasons) or '-'} "
        f"{h[0] if h else ''}"
    )

print()
print("=== IMAGE PATH DIFFERENCES ===")
print(f"only_current={len(only_cur)} only_base={len(only_base)}")
for p in only_base[:20]:
    print("BASE_ONLY:", p)
for p in only_cur[:20]:
    print("CURRENT_ONLY:", p)

print()
print("Outputs:")
for n in [
    "02_encoding.txt",
    "03_recovery_file.patch",
    "04_base_vs_current_normalized.diff",
    "05_current_chat_render_context.txt",
    "06_base_chat_render_context.txt",
    "07_ranked_semantic_hunks.txt",
    "08_recovery_chat_render_changed_context.txt",
    "09_image_path_differences.txt",
    "10_changed_chat_render_functions.diff",
    "11_summary.txt",
]:
    print(out / n)
PY

    echo
    echo "=== SECONDARY CANDIDATES FROM AUDIT 040 ==="

    if [ -f "$AUD040/03_missing_chat_white_candidates.txt" ]; then
        sed -n '1,80p' "$AUD040/03_missing_chat_white_candidates.txt"
    else
        echo "Arquivo 040 não encontrado."
    fi

    echo
    echo "=== NO SOURCE MODIFICATIONS MADE ==="
fi
