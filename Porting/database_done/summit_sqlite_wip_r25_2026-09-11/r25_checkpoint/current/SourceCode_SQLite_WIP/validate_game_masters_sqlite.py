#!/usr/bin/env python3
"""Validate the R23 GameMasters INTEGER-active SQLite compatibility fix."""
from pathlib import Path
import sqlite3,tempfile
ROOT=Path(__file__).resolve().parent
SRC=ROOT/'SrcServer/src/systems/admin/game_masters/GameMasters.cpp'
s=SRC.read_text(encoding='utf-8')
assert 'SELECT id,accountId,password,security,active FROM GameMasters WHERE active =?' in s
assert 'int active = 1;' in s
assert 'BindInputParameter(&active, 1, PARAMTYPE_Integer)' in s
assert 'BindInputParameter("True", 1, PARAMTYPE_String)' not in s
# Demonstrate the semantic mismatch this fixes in SQLite.
db=sqlite3.connect(':memory:'); db.execute('CREATE TABLE GameMasters(id INTEGER PRIMARY KEY, accountId TEXT, password TEXT, security INTEGER, active INTEGER)'); db.execute("INSERT INTO GameMasters VALUES(1,'gm','pw',4,1)")
assert db.execute('SELECT id FROM GameMasters WHERE active=?',(1,)).fetchone()==(1,)
assert db.execute('SELECT id FROM GameMasters WHERE active=?',('True',)).fetchone() is None
print('GameMasters SQLite PASS: active INTEGER bind selects enabled GMs; legacy string True bind removed')
