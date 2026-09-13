#!/usr/bin/env python3
"""Static coverage checks for active server SQL during the SQLite migration.

This deliberately validates only facts that can be proven statically:
- every SQL Server three-part table reference in active server string literals
  resolves to a physical SQLite table created by the migrations;
- removed external persistence namespaces/tables do not reappear;
- source-only contracts no longer depend on the ambiguous positional SQL forms
  that forced schema invention.

Bare table references are routed by SQLConnection/SqliteQueryCompat and are
validated separately by the migration contract tests.
"""

from __future__ import annotations

import re
import sqlite3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MIGRATION_DIR = ROOT / "Database" / "migrations"

# C/C++ ordinary string literals. SQL in this codebase is ASCII-compatible;
# decoding escapes is unnecessary for extracting identifier text.
STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"', re.S)
SQL_VERB_RE = re.compile(r"\b(?:SELECT|INSERT|UPDATE|DELETE)\b", re.I)
QUALIFIED_PATTERNS = (
    re.compile(r"\[([A-Za-z_][A-Za-z0-9_]*)\]\.\[dbo\]\.\[([A-Za-z_][A-Za-z0-9_]*)\]", re.I),
    re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\.dbo\.([A-Za-z_][A-Za-z0-9_]*)\b", re.I),
    re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\.\.([A-Za-z_][A-Za-z0-9_]*)\b", re.I),
)

FORBIDDEN_SQL = (
    (re.compile(r"\bPAINEL_NET\b", re.I), "external PAINEL_NET persistence"),
    (re.compile(r"\[CoinShopDB\]\.\[dbo\]\.\[Usuario\]", re.I), "legacy CoinShopDB.Usuario table"),
    (re.compile(r"SELECT\s+\*\s+FROM\s+(?:\[?ExtraItems\]?|\[?BannedIP\]?|\[?BannedMac\]?)", re.I), "ambiguous source-only SELECT *"),
    (re.compile(r"INSERT\s+INTO\s+(?:\[?BannedIP\]?|\[?AgingRecord\]?)\s+VALUES", re.I), "ambiguous source-only positional INSERT"),
)


def active_source_files() -> list[Path]:
    roots = [ROOT / "SrcServer", ROOT / "Shared" / "server"]
    result: list[Path] = []
    for base in roots:
        for path in base.rglob("*"):
            if path.is_file() and path.suffix.lower() in {".c", ".cc", ".cpp", ".h", ".hpp", ".inl"}:
                result.append(path)

    # This server-specific legacy bridge owns the remaining clan/web DB calls
    # but lives under Shared/security, outside Shared/server. Include it
    # explicitly so SQLite coverage cannot silently miss its SQL literals.
    result.append(ROOT / "Shared" / "security" / "integrity" / "isaocheck_legacy_server.inl")
    return result


def sql_literals(path: Path):
    text = path.read_text(encoding="latin1")
    for match in STRING_RE.finditer(text):
        literal = match.group(0)[1:-1]
        if not SQL_VERB_RE.search(literal):
            continue
        line = text.count("\n", 0, match.start()) + 1
        yield line, literal


def migrated_tables() -> set[str]:
    db = sqlite3.connect(":memory:")
    try:
        for path in sorted(MIGRATION_DIR.glob("[0-9][0-9][0-9]_*.sql")):
            db.executescript(path.read_text(encoding="utf-8"))
        return {
            row[0].lower()
            for row in db.execute(
                "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'"
            )
        }
    finally:
        db.close()


def main() -> int:
    tables = migrated_tables()
    qualified: dict[str, list[tuple[str, int, str]]] = {}
    forbidden: list[tuple[str, int, str, str]] = []

    for path in active_source_files():
        rel = path.relative_to(ROOT).as_posix()
        for line, literal in sql_literals(path):
            for pattern, label in FORBIDDEN_SQL:
                if pattern.search(literal):
                    forbidden.append((rel, line, label, literal))
            for pattern in QUALIFIED_PATTERNS:
                for match in pattern.finditer(literal):
                    physical = f"{match.group(1)}__{match.group(2)}".lower()
                    qualified.setdefault(physical, []).append((rel, line, match.group(0)))

    missing = {name: refs for name, refs in qualified.items() if name not in tables}

    if forbidden:
        print("Forbidden/ambiguous active SQL remains:")
        for rel, line, label, literal in forbidden:
            print(f"  {rel}:{line}: {label}: {literal}")
    if missing:
        print("Qualified SQL references without migrated SQLite tables:")
        for name, refs in sorted(missing.items()):
            print(f"  {name}")
            for rel, line, token in refs:
                print(f"    {rel}:{line}: {token}")

    if forbidden or missing:
        print("RESULT: FAIL")
        return 1

    print(
        f"SQLite query coverage PASS: qualified_tables={len(qualified)}, "
        f"missing=0, forbidden_external_or_positional=0"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
