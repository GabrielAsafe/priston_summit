#!/usr/bin/env bash

ROOT="/home/gabriel/Área de Trabalho/summit"
REPO="$ROOT"
CUR="$ROOT/SourceCode"
REC="/home/gabriel/Transferências/recovery_pre_reset_20260824_0958"
REC_PATCH="$REC/working_tree_vs_HEAD.patch"
AUD034="$ROOT/bugfixes/034_recovery_vs_current"
OUT="$ROOT/bugfixes/040_recovery_chat_white_image_audit"

mkdir -p "$OUT/per_file"

echo "=== 040 — RECOVERY vs CURRENT — CHAT / WHITE IMAGE AUDIT ==="
echo

HEAD="$(git -C "$REPO" rev-parse HEAD 2>/dev/null)"
printf '%s\n' "$HEAD" > "$OUT/base_commit.txt"

{
    echo "HEAD=$HEAD"
    echo "CURRENT=$CUR"
    echo "RECOVERY=$REC"
    echo "RECOVERY_PATCH=$REC_PATCH"
    echo
    echo "IMPORTANT:"
    echo "- READ-ONLY audit"
    echo "- no source modifications"
    echo "- server monster fix state is intentionally preserved"
} | tee "$OUT/00_context.txt"

if [ ! -f "$REC_PATCH" ]; then
    echo "ERRO: recovery patch não encontrado:"
    echo "$REC_PATCH"
else
    python3 - "$REPO" "$CUR" "$REC_PATCH" "$AUD034" "$OUT" <<'PY'
from pathlib import Path
import subprocess
import sys
import re
import hashlib
import os

repo = Path(sys.argv[1])
cur = Path(sys.argv[2])
patch_path = Path(sys.argv[3])
aud034 = Path(sys.argv[4])
out = Path(sys.argv[5])
per_file = out / "per_file"

data = patch_path.read_bytes()

# ------------------------------------------------------------
# Split original recovery patch by file.
# ------------------------------------------------------------

matches = list(re.finditer(rb"(?m)^diff --git a/(.*?) b/(.*?)\n", data))

entries = []

for i, m in enumerate(matches):
    start = m.start()
    end = matches[i + 1].start() if i + 1 < len(matches) else len(data)

    a_path = m.group(1).decode("utf-8", errors="surrogateescape")
    b_path = m.group(2).decode("utf-8", errors="surrogateescape")
    block = data[start:end]

    path = b_path if b_path != "/dev/null" else a_path

    # Recovery patch was generated from repository root, where files are
    # normally stored as SourceCode/...
    repo_target = repo / path

    entries.append({
        "index": i + 1,
        "path": path,
        "block": block,
        "repo_target": repo_target,
    })

def safe_name(path):
    return (
        path.replace("/", "__")
            .replace("\\", "__")
            .replace(" ", "_")
    )

def apply_check(patch_file, reverse=False):
    cmd = ["git", "-C", str(repo), "apply", "--check", "--recount"]
    if reverse:
        cmd.append("--reverse")
    cmd.append(str(patch_file))

    p = subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
    )
    return p.returncode, p.stdout.strip()

def sha256_file(p):
    if not p.is_file():
        return "-"
    return hashlib.sha256(p.read_bytes()).hexdigest()

# Terms are intentionally broad for candidate discovery.
path_terms = [
    "chat", "message", "msg", "talk", "whisper", "clan", "party",
    "notice", "ui", "window", "box", "dialog", "interface",
    "texture", "image", "sprite", "material", "render", "draw",
    "font", "color", "alpha", "blend", "background", "bitmap",
    "graphic", "hochat", "sinmsg", "textarea", "textbox",
]

content_patterns = {
    "CHAT": [
        rb"\bchat\b", rb"Chat", rb"CHAT", rb"whisper", rb"Whisper",
        rb"message", rb"Message", rb"talk", rb"Talk",
    ],
    "UI": [
        rb"\bUI[A-Za-z_]", rb"Window", rb"window", rb"Box", rb"box",
        rb"Interface", rb"interface", rb"Dialog", rb"dialog",
    ],
    "IMAGE_TEXTURE": [
        rb"CreateTexture", rb"Texture", rb"texture", rb"Image", rb"image",
        rb"Sprite", rb"sprite", rb"Material", rb"material",
        rb"\.png", rb"\.PNG", rb"\.bmp", rb"\.BMP", rb"\.tga", rb"\.TGA",
    ],
    "RENDER_BLEND": [
        rb"Render", rb"render", rb"Draw", rb"draw", rb"Alpha", rb"alpha",
        rb"Blend", rb"blend", rb"SMMAT_", rb"SetRenderState",
        rb"D3DRS_", rb"D3DTSS_", rb"Color", rb"color",
    ],
    "FONT_TEXT": [
        rb"Font", rb"font", rb"Text", rb"text", rb"String", rb"string",
    ],
}

results = []

for e in entries:
    pfile = per_file / f"{e['index']:03d}__{safe_name(e['path'])}.patch"
    pfile.write_bytes(e["block"])

    rc_apply, out_apply = apply_check(pfile, False)
    rc_rev, out_rev = apply_check(pfile, True)

    if rc_rev == 0 and rc_apply != 0:
        state = "ALREADY_PRESENT"
    elif rc_apply == 0 and rc_rev != 0:
        state = "APPLY_CLEAN"
    elif rc_apply == 0 and rc_rev == 0:
        state = "AMBIGUOUS_BOTH_APPLY"
    else:
        state = "CONFLICT_OR_DIVERGED"

    lower_path = e["path"].lower()

    if "/srcgame/" in lower_path or lower_path.startswith("sourcecode/srcgame/"):
        side = "CLIENT"
    elif "/srcserver/" in lower_path or lower_path.startswith("sourcecode/srcserver/"):
        side = "SERVER"
    elif "shared" in lower_path:
        side = "SHARED"
    else:
        side = "OTHER"

    block = e["block"]
    labels = []

    for label, pats in content_patterns.items():
        if any(re.search(pat, block, flags=re.IGNORECASE) for pat in pats):
            labels.append(label)

    path_hit = any(term in lower_path for term in path_terms)

    # Stronger ranking: client + still absent/diverged + chat/UI/image evidence.
    score = 0

    if side == "CLIENT":
        score += 100
    elif side == "SHARED":
        score += 40

    if state == "APPLY_CLEAN":
        score += 60
    elif state == "CONFLICT_OR_DIVERGED":
        score += 30
    elif state == "ALREADY_PRESENT":
        score -= 25

    if path_hit:
        score += 40

    if "CHAT" in labels:
        score += 50
    if "IMAGE_TEXTURE" in labels:
        score += 35
    if "RENDER_BLEND" in labels:
        score += 30
    if "UI" in labels:
        score += 25
    if "FONT_TEXT" in labels:
        score += 10

    adds = sum(
        1 for x in block.splitlines()
        if x.startswith(b"+") and not x.startswith(b"+++")
    )
    dels = sum(
        1 for x in block.splitlines()
        if x.startswith(b"-") and not x.startswith(b"---")
    )
    hunks = sum(1 for x in block.splitlines() if x.startswith(b"@@ "))

    results.append({
        **e,
        "patch_file": pfile,
        "state": state,
        "side": side,
        "labels": labels,
        "path_hit": path_hit,
        "score": score,
        "adds": adds,
        "dels": dels,
        "hunks": hunks,
        "current_sha": sha256_file(e["repo_target"]),
        "apply_output": out_apply,
        "reverse_output": out_rev,
    })

# ------------------------------------------------------------
# Full current classification.
# ------------------------------------------------------------

table_lines = [
    "index\tscore\tstate\tside\thunks\tadds\tdels\tlabels\tpath"
]

for r in sorted(results, key=lambda x: (-x["score"], x["index"])):
    table_lines.append(
        f"{r['index']}\t{r['score']}\t{r['state']}\t{r['side']}\t"
        f"{r['hunks']}\t{r['adds']}\t{r['dels']}\t"
        f"{','.join(r['labels'])}\t{r['path']}"
    )

(out / "01_full_classification.tsv").write_text(
    "\n".join(table_lines) + "\n",
    encoding="utf-8",
)

# ------------------------------------------------------------
# Chat / white-image focused candidates.
# ------------------------------------------------------------

focus = [
    r for r in results
    if (
        r["side"] in {"CLIENT", "SHARED"}
        and (
            r["path_hit"]
            or "CHAT" in r["labels"]
            or "IMAGE_TEXTURE" in r["labels"]
            or "RENDER_BLEND" in r["labels"]
            or "UI" in r["labels"]
        )
    )
]

focus_sorted = sorted(focus, key=lambda x: (-x["score"], x["index"]))

report = [
    "=== RECOVERY CANDIDATES — CHAT / WHITE IMAGE ===",
    "",
    "Priority meaning:",
    "- CLIENT first",
    "- APPLY_CLEAN first (recovery change still missing)",
    "- then CHAT / IMAGE_TEXTURE / RENDER_BLEND / UI evidence",
    "",
]

for r in focus_sorted:
    report.append(
        f"[{r['index']:03d}] score={r['score']:3d} "
        f"{r['state']:22s} {r['side']:7s} "
        f"hunks={r['hunks']:3d} +{r['adds']}/-{r['dels']} "
        f"labels={','.join(r['labels']) or '-'} "
        f"{r['path']}"
    )

(out / "02_chat_white_candidates.txt").write_text(
    "\n".join(report) + "\n",
    encoding="utf-8",
)

# Only genuinely missing or diverged candidates.
missing = [
    r for r in focus_sorted
    if r["state"] in {"APPLY_CLEAN", "CONFLICT_OR_DIVERGED"}
]

missing_report = [
    "=== STILL MISSING / DIVERGED FROM PRE-RESET RECOVERY ===",
    ""
]

for r in missing:
    missing_report.append(
        f"[{r['index']:03d}] score={r['score']:3d} "
        f"{r['state']:22s} "
        f"hunks={r['hunks']:3d} +{r['adds']}/-{r['dels']} "
        f"{r['path']}"
    )

(out / "03_missing_chat_white_candidates.txt").write_text(
    "\n".join(missing_report) + "\n",
    encoding="utf-8",
)

# ------------------------------------------------------------
# Generate combined focused diff, but cap it to relevant candidates.
# ------------------------------------------------------------

combined = []

for r in missing[:25]:
    combined.append(
        f"\n\n================ [{r['index']:03d}] "
        f"{r['state']} score={r['score']} "
        f"{r['path']} ================\n"
    )
    combined.append(r["block"].decode("latin1"))

(out / "04_missing_chat_white_focus.diff").write_text(
    "".join(combined),
    encoding="latin1",
)

# ------------------------------------------------------------
# Extract suspicious added/removed lines involving actual rendering/chat.
# ------------------------------------------------------------

interesting_line_re = re.compile(
    rb"(chat|message|whisper|talk|texture|image|sprite|material|render|draw|"
    rb"alpha|blend|background|font|color|window|textbox|textarea|\.png|\.bmp|\.tga)",
    re.IGNORECASE,
)

line_report = []

for r in missing:
    hits = []
    for line in r["block"].splitlines():
        if (
            (line.startswith(b"+") or line.startswith(b"-"))
            and not line.startswith(b"+++")
            and not line.startswith(b"---")
            and interesting_line_re.search(line)
        ):
            hits.append(line.decode("latin1", errors="replace"))

    if hits:
        line_report.append(
            f"================ [{r['index']:03d}] "
            f"{r['state']} {r['path']} ================"
        )
        line_report.extend(hits[:120])
        line_report.append("")

(out / "05_suspicious_changed_lines.txt").write_text(
    "\n".join(line_report) + "\n",
    encoding="utf-8",
)

# ------------------------------------------------------------
# Scan CURRENT source for chat UI/image implementation and asset paths.
# This helps correlate recovery candidates with the live white-image symptom.
# ------------------------------------------------------------

client_src = cur / "SrcGame" / "src"

source_hits = []

source_exts = {".cpp", ".c", ".h", ".hpp"}

if client_src.is_dir():
    for p in client_src.rglob("*"):
        if not p.is_file() or p.suffix.lower() not in source_exts:
            continue

        try:
            blob = p.read_bytes()
        except OSError:
            continue

        # File-level relevance: chat + visual rendering/material terms.
        has_chat = re.search(
            rb"(chat|whisper|talk|message)",
            blob,
            flags=re.IGNORECASE,
        )
        has_visual = re.search(
            rb"(CreateTexture|texture|image|sprite|material|render|draw|alpha|blend|background)",
            blob,
            flags=re.IGNORECASE,
        )

        if has_chat and has_visual:
            rel = p.relative_to(repo).as_posix()
            source_hits.append(rel)

(out / "06_current_chat_visual_files.txt").write_text(
    "\n".join(source_hits) + ("\n" if source_hits else ""),
    encoding="utf-8",
)

# Cross-reference focused recovery paths with current chat/visual files.
cross = []

current_visual_set = set(source_hits)

for r in missing:
    if r["path"] in current_visual_set:
        cross.append(
            f"[{r['index']:03d}] DIRECT_CROSSREF {r['state']} {r['path']}"
        )

(out / "07_direct_crossref.txt").write_text(
    "\n".join(cross) + ("\n" if cross else ""),
    encoding="utf-8",
)

# ------------------------------------------------------------
# Summary counts.
# ------------------------------------------------------------

summary = [
    "=== SUMMARY ===",
    f"recovery_files_total={len(results)}",
    f"client_files={sum(r['side']=='CLIENT' for r in results)}",
    f"chat_white_focus_files={len(focus)}",
    f"focus_apply_clean={sum(r['state']=='APPLY_CLEAN' for r in focus)}",
    f"focus_conflict_or_diverged={sum(r['state']=='CONFLICT_OR_DIVERGED' for r in focus)}",
    f"focus_already_present={sum(r['state']=='ALREADY_PRESENT' for r in focus)}",
    f"current_chat_visual_files={len(source_hits)}",
    f"direct_crossrefs={len(cross)}",
]

(out / "08_summary.txt").write_text(
    "\n".join(summary) + "\n",
    encoding="utf-8",
)

print("\n".join(summary))
print()
print("=== TOP MISSING CHAT / WHITE IMAGE CANDIDATES ===")
for r in missing[:30]:
    print(
        f"[{r['index']:03d}] score={r['score']:3d} "
        f"{r['state']:22s} "
        f"hunks={r['hunks']:3d} +{r['adds']}/-{r['dels']} "
        f"labels={','.join(r['labels']) or '-'} "
        f"{r['path']}"
    )

print()
print("=== DIRECT CURRENT CHAT+VISUAL CROSSREF ===")
if cross:
    print("\n".join(cross))
else:
    print("(none)")

print()
print("Output:")
for name in [
    "00_context.txt",
    "01_full_classification.tsv",
    "02_chat_white_candidates.txt",
    "03_missing_chat_white_candidates.txt",
    "04_missing_chat_white_focus.diff",
    "05_suspicious_changed_lines.txt",
    "06_current_chat_visual_files.txt",
    "07_direct_crossref.txt",
    "08_summary.txt",
]:
    print(out / name)
PY

    echo
    echo "=== CURRENT WORKTREE — IMPORTANT SERVER STATE ==="
    git -C "$REPO" status --short | grep -E \
        'SrcServer/src/(fileread|character)\.cpp|SrcGame/src/' || true

    echo
    echo "=== NO SOURCE MODIFICATIONS MADE ==="
fi
