#!/usr/bin/env python3
from __future__ import annotations

import sqlite3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / 'Shared/security/integrity/isaocheck_legacy_server.inl'


def build_schema(db: sqlite3.Connection) -> None:
    db.executescript('''
    CREATE TABLE ClanDB__CL (
        IDX INTEGER PRIMARY KEY AUTOINCREMENT,
        ClanName TEXT,
        Note TEXT,
        NoteCnt INTEGER,
        UserID TEXT NOT NULL,
        ClanZang TEXT,
        Flag INTEGER,
        MemCnt INTEGER,
        MIconCnt INTEGER,
        RegiDate TEXT,
        LimitDate TEXT,
        DelActive TEXT,
        PFlag INTEGER,
        KFlag INTEGER,
        Cpoint INTEGER NOT NULL,
        CWin INTEGER NOT NULL,
        CFail INTEGER NOT NULL,
        ClanMoney INTEGER NOT NULL,
        CNFlag INTEGER NOT NULL,
        SiegeMoney INTEGER NOT NULL,
        LIDER_BC INTEGER NOT NULL
    );
    CREATE TABLE ClanDB__UL (
        IDX INTEGER,
        MIDX INTEGER PRIMARY KEY AUTOINCREMENT,
        userid TEXT,
        ChName TEXT,
        ChType INTEGER NOT NULL,
        ChLv INTEGER NOT NULL,
        ClanName TEXT NOT NULL,
        Permi TEXT,
        JoinDate TEXT,
        DelActive TEXT,
        PFlag INTEGER,
        KFlag INTEGER,
        MIconCnt INTEGER
    );
    ''')


def add_clan(db, name, leader, points, icon, money, date, note='note'):
    db.execute('''INSERT INTO ClanDB__CL
        (ClanName,Note,UserID,ClanZang,MemCnt,MIconCnt,RegiDate,Cpoint,CWin,CFail,ClanMoney,CNFlag,SiegeMoney,LIDER_BC)
        VALUES (?,?,?,?,?,?,?,?,0,0,?,0,0,0)''',
        (name, note, leader.lower(), leader, 1, icon, date, points, money))


def add_member(db, clan, char, perm='0'):
    db.execute('''INSERT INTO ClanDB__UL
        (IDX,userid,ChName,ChType,ChLv,ClanName,Permi,MIconCnt)
        VALUES (1,?,?,1,100,?,?,0)''',
        (char.lower(), char, clan, perm))


def main_page(db: sqlite3.Connection, char: str) -> str:
    row = db.execute('''SELECT Cpoint,Note,ClanName,MIconCnt,ClanZang,ClanMoney
                        FROM ClanDB__CL ORDER BY Cpoint DESC LIMIT 1''').fetchone()
    if not row:
        return 'Code=0'
    _, note, top_name, icon, top_leader, money = row
    member = db.execute('SELECT ClanName,Permi FROM ClanDB__UL WHERE ChName=? LIMIT 1', (char,)).fetchone()
    base = lambda code, cmoney=0, totals=False: (
        f'Code={code}|CClanMoney={cmoney}|CTax=0|CName={top_name}|CNote={note or ""}|'
        f'CZang={top_leader or ""}|CIMG={icon or 0}|'
        + (f'TotalEMoney={money}|TotalMoney={money}|' if totals else '')
    )
    if not member:
        return base(0)
    member_clan, perm = member
    if (member_clan or '').lower() == (top_name or '').lower():
        if (top_leader or '').lower() == char.lower():
            return base(1, money, True)
        if str(perm) == '2':
            return base(2, 0, True)
        return base(3, 0, True)
    own_leader = db.execute('SELECT ClanZang FROM ClanDB__CL WHERE ClanName=? LIMIT 1', (member_clan,)).fetchone()
    if own_leader and (own_leader[0] or '').lower() == char.lower():
        return base(4)
    if str(perm) == '2':
        return base(5)
    return base(6)


def rank_page(db: sqlite3.Connection) -> str:
    rows = db.execute('''SELECT MIconCnt,ClanName,Cpoint,RegiDate
                         FROM ClanDB__CL ORDER BY Cpoint DESC LIMIT 9''').fetchall()
    if not rows:
        return 'Code=0'
    out = 'Code=1\r'
    for icon, name, point, regdate in rows:
        name = name or ''
        if point > 0 and name and name not in out:
            date = (regdate or '').split(' ', 1)[0]
            out += f'CIMG={icon or 0}\rCName={name}\rCPoint={point}\rCRegistDay={date}\r'
    return out


def parse_main_like_client(s: str) -> int:
    fields = {}
    for part in s.split('|'):
        if '=' in part:
            k, v = part.split('=', 1)
            fields[k] = v
    return int(fields.get('Code', '999'))


def parse_rank_like_client(s: str):
    vals = []
    for token in s.split():
        if '=' not in token:
            continue
        key, value = token.split('=', 1)
        if key == 'Code':
            if int(value) == 2:
                return []
            continue
        vals.append((key, value))
    return vals


def assert_source_contract() -> None:
    text = SOURCE.read_text(encoding='latin1')
    start = text.index('int ISAOCHECK::read_sod2(')
    end = text.index('\n\nbool UTF8ToWeb', start)
    fn = text[start:end]
    required = [
        'SQLConnection::GetConnection(DATABASEID_ClanDB)',
        'SELECT TOP 1 Cpoint,Note,ClanName,MIconCnt,ClanZang,ClanMoney',
        'SELECT TOP 1 ClanName,Permi FROM UL WHERE ChName=?',
        'SELECT TOP 9 MIconCnt,ClanName,Cpoint,RegiDate FROM CL ORDER BY Cpoint DESC',
        'Code=%d|CClanMoney=%lld|CTax=0|CName=%s|CNote=%s|CZang=%s|CIMG=%d|',
        'TotalEMoney=%lld|TotalMoney=%lld|',
        'CIMG=%d\\rCName=%s\\rCPoint=%d\\rCRegistDay=%s\\r',
    ]
    missing = [x for x in required if x not in fn]
    assert not missing, f'missing source contract fragments: {missing}'
    assert 'sod2info.asp?' not in fn, 'server read_sod2 still performs HTTP'
    assert 'GetHttpRequest(' not in fn, 'server read_sod2 still calls HTTP helper'


def main() -> int:
    assert_source_contract()
    db = sqlite3.connect(':memory:')
    build_schema(db)
    assert main_page(db, 'Nobody') == 'Code=0'

    add_clan(db, 'TopClan', 'TopLead', 900, 101, 500000, '2026-09-06 10:11:12', 'top note')
    add_clan(db, 'OtherClan', 'OtherLead', 500, 202, 123, '2026-09-05 09:00:00', 'other note')
    add_member(db, 'TopClan', 'TopLead', '0')
    add_member(db, 'TopClan', 'TopSub', '2')
    add_member(db, 'TopClan', 'TopMember', '0')
    add_member(db, 'OtherClan', 'OtherLead', '0')
    add_member(db, 'OtherClan', 'OtherSub', '2')
    add_member(db, 'OtherClan', 'OtherMember', '0')

    expected_codes = {
        'Outsider': 0,
        'TopLead': 1,
        'TopSub': 2,
        'TopMember': 3,
        'OtherLead': 4,
        'OtherSub': 5,
        'OtherMember': 6,
    }
    for char, code in expected_codes.items():
        payload = main_page(db, char)
        assert parse_main_like_client(payload) == code, (char, payload)

    leader_payload = main_page(db, 'TopLead')
    assert 'CClanMoney=500000|' in leader_payload
    assert 'TotalEMoney=500000|TotalMoney=500000|' in leader_payload
    assert 'CClanMoney=0|' in main_page(db, 'TopSub')
    assert 'TotalEMoney=500000|TotalMoney=500000|' in main_page(db, 'TopSub')
    assert 'TotalEMoney=' not in main_page(db, 'OtherLead')

    # Add more rows to prove the historical first-nine-row cap and positive-point filter.
    for i in range(3, 13):
        add_clan(db, f'Clan{i}', f'Lead{i}', 500 - i, 200 + i, 0, f'2026-08-{i:02d} 12:00:00')
    payload = rank_page(db)
    vals = parse_rank_like_client(payload)
    assert vals[:4] == [
        ('CIMG', '101'), ('CName', 'TopClan'), ('CPoint', '900'), ('CRegistDay', '2026-09-06')
    ], vals[:4]
    assert len(vals) <= 9 * 4
    assert all(k in {'CIMG', 'CName', 'CPoint', 'CRegistDay'} for k, _ in vals)

    db2 = sqlite3.connect(':memory:')
    build_schema(db2)
    assert rank_page(db2) == 'Code=0'

    print('SoD2 SQLite contract PASS: index1 codes=0..6, index3 protocol/ranking=PASS, HTTP removed')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
