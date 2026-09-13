#!/usr/bin/env python3
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent

ACTIVE_ROOTS = [
    ROOT / "SrcServer",
    ROOT / "Shared" / "server",
]
EXTRA_FILES = [
    ROOT / "Shared" / "security" / "integrity" / "isaocheck_legacy_server.inl",
]

EXTS = {".c", ".cc", ".cpp", ".h", ".hpp", ".inl"}

STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"', re.S)
SQL_VERB_RE = re.compile(r"\b(?:SELECT|INSERT|UPDATE|DELETE|REPLACE|EXEC|MERGE)\b", re.I)

UNSUPPORTED = [
    (re.compile(r"\bIF\s+NOT\s+EXISTS\s*\(", re.I), "T-SQL IF NOT EXISTS"),
    (re.compile(r"\bIF\s+EXISTS\s*\(", re.I), "T-SQL IF EXISTS"),
    (re.compile(r"\bDATEDIFF\s*\(", re.I), "DATEDIFF"),
    (re.compile(r"\bDATEPART\s*\(", re.I), "DATEPART"),
    (re.compile(r"\bCONVERT\s*\(", re.I), "CONVERT"),
    (re.compile(r"\bSCOPE_IDENTITY\s*\(", re.I), "SCOPE_IDENTITY"),
    (re.compile(r"@@IDENTITY\b", re.I), "@@IDENTITY"),
    (re.compile(r"@@ROWCOUNT\b", re.I), "@@ROWCOUNT"),
    (re.compile(r"\bMERGE\b", re.I), "MERGE"),
    (re.compile(r"\bWITH\s*\(\s*NOLOCK\s*\)", re.I), "WITH(NOLOCK)"),
    (re.compile(r"\bNEWID\s*\(", re.I), "NEWID"),
    (re.compile(r"\bEXEC(?:UTE)?\s+", re.I), "EXEC/EXECUTE"),
]

SUPPORTED_LEGACY = [
    (re.compile(r"\bGETDATE\s*\(", re.I), "GETDATE"),
    (re.compile(r"\bDATEADD\s*\(", re.I), "DATEADD"),
    (re.compile(r"\bSELECT\s+TOP\s+\d+", re.I), "SELECT TOP"),
    (re.compile(r"\bISNULL\s*\(", re.I), "ISNULL->COALESCE"),
    (re.compile(r"\bDAY\s*\(", re.I), "DAY"),
    (re.compile(r"\bMONTH\s*\(", re.I), "MONTH"),
]

def iter_files():
    seen = set()
    for base in ACTIVE_ROOTS:
        if not base.exists():
            continue
        for p in base.rglob("*"):
            if not p.is_file() or p.suffix.lower() not in EXTS:
                continue
            if "legacy_oracle" in p.parts or ".before_" in p.name:
                continue
            if p.name == "SqliteQueryCompat.inl":
                continue
            if p not in seen:
                seen.add(p)
                yield p
    for p in EXTRA_FILES:
        if p.exists() and p not in seen:
            yield p

def sql_literals(path: Path):
    text = path.read_text(encoding="latin1", errors="ignore")
    for m in STRING_RE.finditer(text):
        literal = m.group(0)[1:-1]
        if not SQL_VERB_RE.search(literal):
            continue
        line = text.count("\n", 0, m.start()) + 1
        yield line, literal

def main() -> int:
    failures = []
    counts = {label: 0 for _, label in SUPPORTED_LEGACY}
    literal_count = 0
    prepare_count = 0
    source_files = list(iter_files())

    for p in source_files:
        text = p.read_text(encoding="latin1", errors="ignore")
        prepare_count += len(re.findall(r"\bPrepare\s*\(", text))
        rel = p.relative_to(ROOT).as_posix()

        for line, literal in sql_literals(p):
            literal_count += 1
            for rx, label in UNSUPPORTED:
                if rx.search(literal):
                    failures.append((rel, line, label, literal))
            for rx, label in SUPPORTED_LEGACY:
                if rx.search(literal):
                    counts[label] += 1

    print(f"active_files={len(source_files)}")
    print(f"prepare_calls={prepare_count}")
    print(f"sql_string_literals={literal_count}")
    for label in sorted(counts):
        print(f"supported_legacy[{label}]={counts[label]}")

    if failures:
        print()
        print("UNSUPPORTED ACTIVE SQL:")
        for rel, line, label, literal in failures:
            print(f"  {rel}:{line}: {label}: {literal}")
        print(f"unsupported_count={len(failures)}")
        print("RESULT=FAIL")
        return 1

    print("unsupported_count=0")
    print("RESULT=PASS")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
