#!/usr/bin/env python3
"""Static contract checks for the SQLite transaction primitive used by DB migrations/callers."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent
public_h = (ROOT / 'Shared/persistence/database/Database_sqlite_server.h').read_text()
public_inl = (ROOT / 'Shared/persistence/database/Database_sqlite_server.inl').read_text()
sqlite_h = (ROOT / 'SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.h').read_text()
sqlite_cpp = (ROOT / 'SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp').read_text()

for name in ('BeginTransaction', 'CommitTransaction', 'RollbackTransaction'):
    assert f'BOOL {name}();' in public_h
    assert f'BOOL {name}();' in sqlite_h
    assert f'SQLConnection::{name}()' in public_inl
    assert f'SQLiteConnection::{name}()' in sqlite_cpp

assert 'BEGIN IMMEDIATE;' in sqlite_cpp
assert 'COMMIT;' in sqlite_cpp
assert 'ROLLBACK;' in sqlite_cpp
assert 'transactionActive' in sqlite_cpp
assert 'BeginTransaction requer Open()' in sqlite_cpp
assert 'transacao aberta no Close()' in sqlite_cpp

print('SQLite transaction API PASS: begin/commit/rollback=present, close rollback guard=present')
