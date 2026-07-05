#!/usr/bin/env python3
from pathlib import Path
import re
import subprocess

ROOT = Path.cwd()

SOURCE_DIRS = [
    "Shared",
    "Shared2",
    "SrcGame/src",
    "SrcServer/src",
    "modules",
    "Dependency/DirectX9/Include",
]

INCLUDE_DIRS = [
    ".",
    "Shared",
    "Shared2",
    "SrcGame/src",
    "SrcGame/src/smLib3d",
    "SrcGame/src/sinbaram",
    "SrcGame/src/Language",
    "SrcGame/src/Resources",
    "SrcGame/src/Sound",
    "SrcGame/src/HoBaram",
    "SrcGame/src/HoBaram/NewEffect",
    "SrcGame/src/HoBaram/NewEffect/math",
    "SrcGame/src/HoBaram/NewEffect/Script",
    "SrcGame/src/Library/LuaSrc",
    "SrcGame/src/Library/LuaSrc/Lua",
    "SrcGame/src/TJBOY/clanmenu",
    "SrcGame/src/TJBOY/isaocheck",
    "SrcGame/src/TJBOY/park",
    "SrcGame/src/TJBOY/Ygy",
    "SrcGame/src/TJBOY/Ygy/memmap",
    "SrcGame/src/Server",
    "SrcGame/src/WinInt",
    "SrcGame/src/Security",
    "SrcGame/src/Settings",
    "modules",
    "modules/File",
    "modules/MD5",
    "modules/Helper",
    "Dependency/DirectX9/Include",
]

EXTS = {".h", ".hpp", ".c", ".cpp", ".cc", ".cxx"}

include_re = re.compile(r'(^\s*#\s*include\s*)([<"])([^">]+)([">])', re.MULTILINE)

def mingw_include_dirs():
    try:
        p = subprocess.run(
            ["x86_64-w64-mingw32-g++", "-xc++", "-E", "-v", "/dev/null"],
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True,
        )
        out = p.stderr
        inside = False
        dirs = []
        for line in out.splitlines():
            if "#include <...> search starts here:" in line:
                inside = True
                continue
            if "End of search list." in line:
                inside = False
            elif inside:
                d = line.strip()
                if d:
                    dirs.append(Path(d))
        return dirs
    except Exception:
        return []

def source_files():
    for d in SOURCE_DIRS:
        p = ROOT / d
        if not p.exists():
            continue
        for f in p.rglob("*"):
            if f.is_file() and f.suffix.lower() in EXTS:
                yield f

def safe_read(path):
    try:
        return path.read_text(errors="ignore")
    except Exception:
        return ""

def safe_write(path, text):
    path.write_text(text, errors="ignore")

def normalize_inc(s):
    return s.replace("\\", "/").replace("//", "/")

def exists_exact(include_name, including_file):
    inc = normalize_inc(include_name)

    candidates = []

    if include_name.startswith(".") or "/" in inc:
        candidates.append((including_file.parent / inc).resolve())

    for d in INCLUDE_DIRS:
        candidates.append((ROOT / d / inc).resolve())

    for c in candidates:
        if c.exists() and c.is_file():
            return c

    for d in mingw_dirs:
        c = d / inc
        if c.exists() and c.is_file():
            return c

    return None

def build_file_index():
    idx = {}
    roots = [ROOT / d for d in SOURCE_DIRS if (ROOT / d).exists()]
    for base in roots:
        for f in base.rglob("*"):
            if f.is_file():
                idx.setdefault(f.name.lower(), []).append(f.resolve())

    for d in mingw_dirs:
        if d.exists():
            for f in d.rglob("*"):
                if f.is_file():
                    idx.setdefault(f.name.lower(), []).append(f.resolve())

    return idx

def best_include_path(real_path):
    real = real_path.resolve()

    # Prefer path relative to SrcGame/src if possible
    preferred_roots = [
        ROOT / "SrcGame/src",
        ROOT / "SrcServer/src",
        ROOT,
    ]

    for r in preferred_roots:
        try:
            rel = real.relative_to(r.resolve())
            return str(rel).replace("\\", "/")
        except ValueError:
            pass

    return real.name

def patch_file(path, mapping):
    text = safe_read(path)
    if not text:
        return 0

    changed = 0

    def repl(m):
        nonlocal changed
        prefix, opench, inc, closech = m.groups()
        inc_norm = normalize_inc(inc)

        if inc in mapping:
            new_inc = mapping[inc]
        elif inc_norm in mapping:
            new_inc = mapping[inc_norm]
        else:
            return m.group(0)

        if new_inc != inc:
            changed += 1
            return f"{prefix}{opench}{new_inc}{closech}"

        return m.group(0)

    new_text = include_re.sub(repl, text)

    if changed:
        safe_write(path, new_text)

    return changed

mingw_dirs = mingw_include_dirs()
file_index = build_file_index()

all_includes = []
missing = []
case_fixed = {}
ambiguous = []

for f in source_files():
    text = safe_read(f)
    for m in include_re.finditer(text):
        inc = m.group(3)
        inc_norm = normalize_inc(inc)
        all_includes.append((f, inc, inc_norm))

        if exists_exact(inc_norm, f):
            continue

        base = Path(inc_norm).name.lower()
        matches = file_index.get(base, [])

        if len(matches) == 1:
            fixed = best_include_path(matches[0])
            case_fixed[inc] = fixed
            case_fixed[inc_norm] = fixed
        elif len(matches) > 1:
            ambiguous.append((inc, matches[:10]))
        else:
            missing.append(inc)

changed_files = 0
changed_includes = 0

for f in source_files():
    n = patch_file(f, case_fixed)
    if n:
        changed_files += 1
        changed_includes += n

out = ROOT / "include_auto_fix_report"
out.mkdir(exist_ok=True)

(out / "all_includes.txt").write_text(
    "\n".join(sorted(set(i[1] for i in all_includes))) + "\n",
    errors="ignore"
)

(out / "auto_fixed_mapping.txt").write_text(
    "\n".join(f"{k} -> {v}" for k, v in sorted(case_fixed.items())) + "\n",
    errors="ignore"
)

(out / "missing.txt").write_text(
    "\n".join(sorted(set(missing))) + "\n",
    errors="ignore"
)

(out / "ambiguous.txt").write_text(
    "\n".join(
        f"{inc}\n" + "\n".join(f"  {m}" for m in matches)
        for inc, matches in ambiguous
    ) + "\n",
    errors="ignore"
)

print("Done.")
print(f"Unique includes:      {len(set(i[1] for i in all_includes))}")
print(f"Auto-fixed mappings:  {len(case_fixed)}")
print(f"Changed files:        {changed_files}")
print(f"Changed includes:     {changed_includes}")
print(f"Still missing:        {len(set(missing))}")
print(f"Ambiguous:            {len(ambiguous)}")
print()
print("Reports:")
print("  include_auto_fix_report/all_includes.txt")
print("  include_auto_fix_report/auto_fixed_mapping.txt")
print("  include_auto_fix_report/missing.txt")
print("  include_auto_fix_report/ambiguous.txt")
