#!/usr/bin/env python3
"""Validate the active server notice path against ServerDB__NoticeSystem."""

from __future__ import annotations

import sqlite3
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MIGRATIONS = ROOT / "Database" / "migrations"
NEWS = ROOT / "SrcServer" / "src" / "systems" / "community" / "news" / "News.cpp"


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        db = sqlite3.connect(Path(tmp) / "summit.sqlite3")
        for migration in sorted(MIGRATIONS.glob("[0-9][0-9][0-9]_*.sql")):
            db.executescript(migration.read_text(encoding="utf-8"))
        cols = [r[1] for r in db.execute('PRAGMA table_info("ServerDB__NoticeSystem")')]
        assert cols == ["ID", "Message", "Type", "Activated"], cols
        db.close()

    source = NEWS.read_text(encoding="latin1")
    assert 'SELECT Message, Type FROM NoticeSystem WHERE Activated=1' in source
    assert 'SELECT * FROM NoticeSystem' not in source
    assert 'db->GetData(szMessageLogin, 256, 1, PARAMTYPE_String)' in source
    assert 'db->GetData(&iMessageType, 0, 2, PARAMTYPE_Integer)' in source

    print("SQLite notice contract PASS: named Message/Type projection over proven NoticeSystem schema")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
