#!/usr/bin/env python3
"""Validate the locally migrated clan membership contract.

This validator intentionally joins three project facts:
- the server-side replacement no longer calls isClanMember.asp;
- the client parser still maps response codes 1/2/5 to member/leader/sub-chief;
- the migrated ClanDB CL/UL schema supports those roles and the legacy MIconCnt side effect.
"""

from __future__ import annotations

import sqlite3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SERVER_ISAO = ROOT / "Shared" / "security" / "integrity" / "isaocheck_legacy_server.inl"
CLIENT_CLAN = ROOT / "Shared" / "social" / "clan" / "ClanEntityLegacyCommon.inl"
MIGRATIONS = ROOT / "Database" / "migrations"


def function_body(text: str, signature: str, next_signature: str) -> str:
    start = text.index(signature)
    end = text.index(next_signature, start)
    return text[start:end]


def apply_schema(db: sqlite3.Connection) -> None:
    for path in sorted(MIGRATIONS.glob("[0-9][0-9][0-9]_*.sql")):
        db.executescript(path.read_text(encoding="utf-8"))


def seed_clan(db: sqlite3.Connection) -> None:
    db.execute(
        "INSERT INTO ClanDB__CL(ClanName,Note,NoteCnt,UserID,ClanZang,Flag,MemCnt,MIconCnt,"
        "RegiDate,LimitDate,DelActive,PFlag,KFlag,Cpoint,CWin,CFail,ClanMoney,CNFlag,SiegeMoney,LIDER_BC) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
        (
            "Alpha", "Alpha note", 0, "leader-account", "Leader", 0, 3, 321,
            "2026-09-01 10:00:00", "2026-10-01 10:00:00", "0", 7, 8,
            100, 1, 0, 7654321, 2, 0, 0,
        ),
    )
    for user_id, name, permi in (
        ("leader-account", "Leader", "0"),
        ("sub-account", "SubChief", "2"),
        ("member-account", "Member", "0"),
    ):
        db.execute(
            "INSERT INTO ClanDB__UL(userid,ChName,ChType,ChLv,ClanName,Permi,JoinDate,DelActive,PFlag,KFlag,MIconCnt) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?)",
            (user_id, name, 1, 100, "Alpha", permi, "2026-09-01 10:00:00", "0", 0, 0, 0),
        )


def resolve_role(db: sqlite3.Connection, character: str) -> tuple[int, int, int, int]:
    row = db.execute(
        "SELECT ClanName FROM ClanDB__UL WHERE ChName=? LIMIT 1", (character,)
    ).fetchone()
    if not row:
        return 0, 0, 0, 0

    clan_name = row[0]
    clan = db.execute(
        "SELECT ClanZang,MemCnt,MIconCnt,PFlag,KFlag FROM ClanDB__CL WHERE ClanName=? LIMIT 1",
        (clan_name,),
    ).fetchone()
    if not clan:
        db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (character,))
        return 0, 0, 0, 0

    leader, _members, icon, pflag, kflag = clan
    sub = db.execute(
        "SELECT ChName FROM ClanDB__UL WHERE ClanName=? AND Permi='2' LIMIT 1",
        (clan_name,),
    ).fetchone()
    sub_name = sub[0] if sub else ""

    if leader.casefold() == character.casefold():
        code = 2
    elif sub_name and sub_name.casefold() == character.casefold():
        code = 5
    else:
        code = 1

    db.execute("UPDATE ClanDB__UL SET MIconCnt=? WHERE ChName=?", (icon, character))
    if code == 1:
        return code, 0, 0, icon
    return code, pflag, kflag, icon



def clan_member_response(db: sqlite3.Connection, character: str) -> tuple[int, str]:
    row = db.execute(
        "SELECT ClanName FROM ClanDB__UL WHERE ChName=? LIMIT 1", (character,)
    ).fetchone()
    if not row:
        return 0, "Code=0\r"
    clan_name = row[0] or ""
    if not clan_name:
        db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (character,))
        return 0, "Code=0\r"
    if not db.execute(
        "SELECT 1 FROM ClanDB__CL WHERE ClanName=? LIMIT 1", (clan_name,)
    ).fetchone():
        db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (character,))
        return 0, "Code=0\r"

    names = [r[0] for r in db.execute(
        "SELECT ChName FROM ClanDB__UL WHERE ClanName=? ORDER BY MIDX", (clan_name,)
    )]
    if not names:
        return 0, "Code=0\r"
    response = "Code=1\r" + "".join(f"CMem={name}\r" for name in names)
    if len(response.encode("latin1")) >= 2048:
        return 100, response
    return 1, response


def parse_legacy_clan_members(response: str, own_character: str) -> list[str]:
    # Equivalent to CLANEntity::ParsigClanWon: first CMem=, then every CR
    # consumes the six-byte separator "\rCMem=".  A trailing CR counts the
    # final entry; the local implementation deliberately preserves it.
    pos = response.find("CMem")
    if pos < 0:
        return []
    k = pos + 5
    raw: list[str] = []
    current = []
    while k < len(response):
        if response[k] == "\r":
            raw.append("".join(current))
            current = []
            k += 6
        else:
            current.append(response[k])
            k += 1
    return [name for name in raw if name != own_character]


def appoint_subchief(db: sqlite3.Connection, target: str) -> int:
    db.execute("BEGIN IMMEDIATE")
    try:
        row = db.execute(
            "SELECT ClanName FROM ClanDB__UL WHERE ChName=? LIMIT 1", (target,)
        ).fetchone()
        if not row or not row[0]:
            db.rollback()
            return 0
        clan_name = row[0]
        db.execute("UPDATE ClanDB__UL SET Permi='0' WHERE ClanName=?", (clan_name,))
        db.execute(
            "UPDATE ClanDB__UL SET Permi='2' WHERE ChName=? AND ClanName=?",
            (target, clan_name),
        )
        db.commit()
        return 1
    except Exception:
        db.rollback()
        raise


def release_subchief(db: sqlite3.Connection, target: str) -> int:
    db.execute("BEGIN IMMEDIATE")
    try:
        if not db.execute(
            "SELECT ClanName FROM ClanDB__UL WHERE ChName=? LIMIT 1", (target,)
        ).fetchone():
            db.rollback()
            return 0
        db.execute("UPDATE ClanDB__UL SET Permi='0' WHERE ChName=?", (target,))
        db.commit()
        return 1
    except Exception:
        db.rollback()
        raise

def join_member(
    db: sqlite3.Connection,
    requester: str,
    clan_name: str,
    target: str,
    target_user: str,
    ch_type: int,
    ch_level: int,
) -> int:
    db.execute("BEGIN IMMEDIATE")
    try:
        clan = db.execute(
            "SELECT IDX,ClanZang,MemCnt FROM ClanDB__CL WHERE ClanName=? LIMIT 1",
            (clan_name,),
        ).fetchone()
        if not clan:
            db.rollback()
            return 0
        idx, leader, members = clan
        if members + 1 > 100:
            db.rollback()
            return 2

        row = db.execute(
            "SELECT ChName FROM ClanDB__UL WHERE Permi='2' AND ClanName=? LIMIT 1",
            (clan_name,),
        ).fetchone()
        subchief = row[0] if row else ""
        if requester != leader and requester != subchief:
            db.rollback()
            return 0

        row = db.execute(
            "SELECT ClanName FROM ClanDB__UL WHERE ChName=? LIMIT 1", (target,)
        ).fetchone()
        if row and row[0]:
            db.rollback()
            return 0
        if row and not row[0]:
            db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (target,))

        db.execute(
            "UPDATE ClanDB__CL SET MemCnt=? WHERE ClanName=?",
            (members + 1, clan_name),
        )
        db.execute(
            "INSERT INTO ClanDB__UL"
            "(IDX,userid,ChName,ClanName,ChType,ChLv,Permi,JoinDate,DelActive,PFlag,KFlag,MIconCnt) "
            "VALUES (?,?,?,?,?,?,'0',datetime('now','localtime'),'0',0,0,0)",
            (idx, target_user, target, clan_name, ch_type, ch_level),
        )
        db.commit()
        return 1
    except Exception:
        db.rollback()
        raise


def release_member(db: sqlite3.Connection, clan_name: str, encoded_target: str) -> int:
    # Preserve clanWonRelease.asp: only Request("clwon1") was consumed even
    # when the client appended clwon2/clwon3...
    target = encoded_target
    if target.startswith("clwon1="):
        target = target[7:]
    target = target.split("&", 1)[0]
    if not target:
        return 100

    db.execute("BEGIN IMMEDIATE")
    try:
        row = db.execute(
            "SELECT ClanName FROM ClanDB__UL WHERE ChName=? LIMIT 1", (target,)
        ).fetchone()
        if not row:
            db.rollback()
            return 0
        if not row[0]:
            db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (target,))
            db.commit()
            return 0
        if row[0] != clan_name:
            db.rollback()
            return 0

        clan = db.execute(
            "SELECT ClanZang,MemCnt FROM ClanDB__CL WHERE ClanName=? LIMIT 1",
            (clan_name,),
        ).fetchone()
        if not clan:
            db.rollback()
            return 100
        leader, members = clan
        if target == leader:
            db.rollback()
            return 4

        db.execute(
            "UPDATE ClanDB__CL SET MemCnt=? WHERE ClanName=?",
            (members - 1, clan_name),
        )
        db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (target,))
        db.commit()
        return 1
    except Exception:
        db.rollback()
        raise


def self_leave(db: sqlite3.Connection, character: str, clan_name: str) -> int:
    db.execute("BEGIN IMMEDIATE")
    try:
        row = db.execute(
            "SELECT ClanName,Permi FROM ClanDB__UL WHERE ChName=? LIMIT 1",
            (character,),
        ).fetchone()
        if not row:
            db.rollback()
            return 0
        if not row[0]:
            db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (character,))
            db.commit()
            return 0
        if row[0] != clan_name:
            db.rollback()
            return 0

        clan = db.execute(
            "SELECT ClanZang,MemCnt FROM ClanDB__CL WHERE ClanName=? LIMIT 1",
            (clan_name,),
        ).fetchone()
        if not clan:
            db.rollback()
            return 100
        leader, members = clan
        if character == leader:
            db.rollback()
            return 4

        db.execute(
            "UPDATE ClanDB__CL SET MemCnt=? WHERE ClanName=?",
            (members - 1, clan_name),
        )
        db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (character,))
        db.commit()
        return 1
    except Exception:
        db.rollback()
        raise



def create_clan(
    db: sqlite3.Connection,
    user_id: str,
    character: str,
    clan_name: str,
    ch_type: int,
    level: int,
) -> tuple[int, str, int | None]:
    """Model the proven clanInsert.aspx contract used by the R16 C++ path."""
    if not user_id or not character or not clan_name:
        return 100, "", None

    db.execute("BEGIN IMMEDIATE")
    try:
        row = db.execute(
            "SELECT ClanName FROM ClanDB__UL WHERE ChName=? LIMIT 1", (character,)
        ).fetchone()
        if row:
            if row[0]:
                db.rollback()
                return 2, "", None
            db.execute("DELETE FROM ClanDB__UL WHERE ChName=?", (character,))

        if db.execute(
            "SELECT ClanZang FROM ClanDB__CL WHERE ClanName=? LIMIT 1", (clan_name,)
        ).fetchone():
            db.rollback()
            return 3, "", None

        row = db.execute("SELECT IMG FROM ClanDB__LI WHERE ID=1 LIMIT 1").fetchone()
        icon_default = 1000000000
        current = row[0] if row else icon_default
        image_id = current + 1
        if row:
            db.execute("UPDATE ClanDB__LI SET IMG=? WHERE ID=1", (image_id,))
        else:
            # Historical first-use INSERT stores Icon_Default + 1.
            db.execute("INSERT INTO ClanDB__LI(IMG,ID) VALUES (?,1)", (image_id,))

        db.execute(
            "INSERT INTO ClanDB__CL"
            "(ClanName,UserID,ClanZang,MemCnt,Note,MIconCnt,RegiDate,LimitDate,"
            "DelActive,PFlag,KFlag,Flag,NoteCnt,Cpoint,CWin,CFail,ClanMoney,CNFlag,SiegeMoney,LIDER_BC) "
            "VALUES (?,?,?,?,?,?,datetime('now','localtime'),datetime('now','localtime','+3600 days'),"
            "'0',0,0,0,1,0,0,0,0,0,0,0)",
            (clan_name, user_id, character, 1, "New clan", image_id),
        )
        clan_idx = db.execute(
            "SELECT IDX FROM ClanDB__CL WHERE ClanName=? LIMIT 1", (clan_name,)
        ).fetchone()[0]
        db.execute(
            "INSERT INTO ClanDB__UL"
            "(IDX,userid,ChName,ClanName,ChType,ChLv,Permi,JoinDate,DelActive,PFlag,KFlag,MIconCnt) "
            "VALUES (?,?,?,?,?,?,'0',datetime('now','localtime'),'0',0,0,?)",
            (clan_idx, user_id, character, clan_name, ch_type, level, image_id),
        )
        db.commit()
        return 1, "Code=1\rCMoney=500000\r", image_id
    except Exception:
        db.rollback()
        raise

def dissolve_clan(db: sqlite3.Connection, user_id: str, leader_name: str, clan_name: str) -> int:
    db.execute("BEGIN IMMEDIATE")
    try:
        row = db.execute(
            "SELECT ClanZang FROM ClanDB__CL WHERE ClanName=? LIMIT 1", (clan_name,)
        ).fetchone()
        if not row or row[0] != leader_name:
            db.rollback()
            return 0

        db.execute(
            "INSERT INTO ClanDB__ClanMainDeleted "
            "(IDX,MIDX,ClanName,Userid,ClanZang,Flag,MemCnt,MIConCnt,RegiDate,LimitDate,DelDate,DelActive,PFlag,KFlag,DelState,DelCase,Server) "
            "SELECT A.IDX,B.MIDX,A.ClanName,A.UserID,A.ClanZang,A.Flag,A.MemCnt,A.MIconCnt,A.RegiDate,A.LimitDate,datetime('now','localtime'),A.DelActive,A.PFlag,A.KFlag,'1','1',1 "
            "FROM ClanDB__CL A JOIN ClanDB__UL B ON B.userid=? WHERE A.ClanName=?",
            (user_id, clan_name),
        )
        db.execute("DELETE FROM ClanDB__CL WHERE ClanName=?", (clan_name,))
        db.execute(
            "INSERT INTO ClanDB__ClanSubDeleted "
            "(IDX,MIDX,userid,ChName,ChType,ChLv,ClanName,Permi,JoinDate,DelActive,PFlag,KFlag,DelDate,DelState,DelCase,Server) "
            "SELECT IDX,MIDX,userid,ChName,ChType,ChLv,ClanName,Permi,JoinDate,DelActive,PFlag,KFlag,datetime('now','localtime'),'1','1',1 "
            "FROM ClanDB__UL WHERE ClanName=?",
            (clan_name,),
        )
        db.execute("DELETE FROM ClanDB__UL WHERE ClanName=?", (clan_name,))
        db.commit()
        return 1
    except Exception:
        db.rollback()
        raise


def main() -> int:
    server_text = SERVER_ISAO.read_text(encoding="latin1")
    body = function_body(
        server_text,
        "int ISAOCHECK::isClanMember(char *ClanZang_id, char* gserver, char* ClanZang_ChaName, char* ret_Buf)",
        "int ISAOCHECK::joinClanMember",
    )

    assert "http://" not in body
    assert "connect_WEBdb" not in body
    for sql in (
        "SELECT TOP 1 ClanName FROM UL WHERE ChName=?",
        "SELECT TOP 1 ClanZang, MemCnt, Note, MIconCnt, RegiDate, LimitDate, PFlag, KFlag, ClanMoney, CNFlag FROM CL WHERE ClanName=?",
        "SELECT TOP 1 ChName FROM UL WHERE ClanName=? AND Permi='2'",
        "UPDATE UL SET MIconCnt=? WHERE ChName=?",
    ):
        assert sql in body, sql

    for token in (
        "Code=%d", "CName=%s", "CNote=%s", "CSubChip=%s", "CMCnt=%d",
        "CZang=%s", "CStats=1", "CSec=60", "CRegiD=%s", "CLimitD=%s",
        "CDelActive=0", "CIMG=%d", "CPFlag=%d", "CKFlag=%d", "CMoney=%lld", "CNFlag=%d",
    ):
        assert token in body, token

    read_body = function_body(
        server_text,
        "int ISAOCHECK::read_ClanData(char *ClanZang_ID, char* gserver, char* ClanZang_ChaName, char* ret_Buf)",
        "int ISAOCHECK::BreakUPClan",
    )
    assert "http://" not in read_body
    assert "connect_WEBdb" not in read_body
    for token in (
        "SELECT TOP 1 ClanName FROM UL WHERE ChName=?",
        "SELECT TOP 1 1 FROM CL WHERE ClanName=?",
        "SELECT ChName FROM UL WHERE ClanName=? ORDER BY MIDX",
        '"Code=1\\r"',
        '"CMem=%s\\r"',
    ):
        assert token in read_body, token

    dissolve_body = function_body(
        server_text,
        "int ISAOCHECK::BreakUPClan(char *ClanZang_ID, char *gserver, char *ClanZang_ChaName, char *ClanName)",
        "int ISAOCHECK::connect_WEBdb_NOTICE",
    )
    assert "http://" not in dissolve_body
    assert "connect_WEBdb" not in dissolve_body
    for token in (
        "BeginTransaction()",
        "SELECT TOP 1 ClanZang FROM CL WHERE ClanName=?",
        "INSERT INTO ClanMainDeleted",
        "INSERT INTO ClanSubDeleted",
        "DELETE FROM CL WHERE ClanName=?",
        "DELETE FROM UL WHERE ClanName=?",
        "CommitTransaction()",
        "RollbackTransaction()",
    ):
        assert token in dissolve_body, token

    appoint_body = function_body(
        server_text,
        "int ISAOCHECK::AppointSubChip(char *gserver, char* clanwon)",
        "int ISAOCHECK::ReleaseSubChip",
    )
    release_sub_body = function_body(
        server_text,
        "int ISAOCHECK::ReleaseSubChip(char *gserver, char* clanwon)",
        "int ISAOCHECK::LeaveClan",
    )
    for name, sub_body in (("AppointSubChip", appoint_body), ("ReleaseSubChip", release_sub_body)):
        assert "http://" not in sub_body, name
        assert "connect_WEBdb" not in sub_body, name
        assert "BeginTransaction()" in sub_body, name
        assert "CommitTransaction()" in sub_body, name
        assert "RollbackTransaction()" in sub_body, name
    assert "UPDATE UL SET Permi='0' WHERE ClanName=?" in appoint_body
    assert "UPDATE UL SET Permi='2' WHERE ChName=? AND ClanName=?" in appoint_body
    assert "UPDATE UL SET Permi='0' WHERE ChName=?" in release_sub_body

    join_body = function_body(
        server_text,
        "int ISAOCHECK::joinClanMember(char *ClanZang_ID, char *gserver, char *ClanZang_ChaName, char* ClanName, char* ClanWon, char* ClanWon_ID, int level, DWORD chtype, int joinlevel, char *chipflag)",
        "int ISAOCHECK::makeClan",
    )
    make_body = function_body(
        server_text,
        "int ISAOCHECK::makeClan(char *ClanZang_ID, char* gserver, char* ClanZang_ChaName, char* ClanName, char *explanation, DWORD chtype, int level, char* data)",
        "int ISAOCHECK::read_ClanName",
    )
    assert "http://" not in make_body
    assert "connect_WEBdb" not in make_body
    for token in (
        "BeginTransaction()",
        "SELECT TOP 1 ClanName FROM UL WHERE ChName=?",
        "SELECT TOP 1 ClanZang FROM CL WHERE ClanName=?",
        "SELECT TOP 1 IMG FROM LI WHERE ID=1",
        "INSERT INTO LI (IMG, ID) VALUES (?, 1)",
        "UPDATE LI SET IMG=? WHERE ID=1",
        'char noteDefault[] = "New clan"',
        "datetime('now','localtime','+3600 days')",
        "INSERT INTO CL ",
        "SiegeMoney,LIDER_BC)",
        "INSERT INTO UL ",
        "CommitTransaction()",
        "RollbackTransaction()",
        r'Database\\clan_marks\\%d.bmp',
        "GetFileAttributesA(defaultMark)",
        "CopyFileA(defaultMark, clanMark, FALSE)",
        'Code=1\\rCMoney=500000\\r',
    ):
        assert token in make_body, token
    banish_body = function_body(
        server_text,
        "int ISAOCHECK::Banishiment(char *ClanZang_ID, char *gserver, char *ClanZang_ChaName, char *ClanName, char *ClanWon)",
        "int ISAOCHECK::AppointSubChip",
    )
    leave_body = function_body(
        server_text,
        "int ISAOCHECK::Secession(char* clanzang, char* gserver, char* SecessionID, char* clanname)",
        "int ISAOCHECK::isKPFlag",
    )

    for name, mutation_body in (
        ("joinClanMember", join_body),
        ("Banishiment", banish_body),
        ("Secession", leave_body),
    ):
        assert "http://" not in mutation_body, name
        assert "connect_WEBdb" not in mutation_body, name
        assert "BeginTransaction()" in mutation_body, name
        assert "CommitTransaction()" in mutation_body, name
        assert "RollbackTransaction()" in mutation_body, name

    for sql in (
        "SELECT TOP 1 IDX, ClanZang, MemCnt FROM CL WHERE ClanName=?",
        "SELECT TOP 1 ChName FROM UL WHERE Permi='2' AND ClanName=?",
        "UPDATE CL SET MemCnt=? WHERE ClanName=?",
        "INSERT INTO UL (IDX, userid, ChName, ClanName, ChType, ChLv, Permi, JoinDate, DelActive, PFlag, KFlag, MIconCnt)",
    ):
        assert sql in join_body, sql
    assert "GETDATE()" in join_body

    assert 'strncmp(targetStart, "clwon1=", 7)' in banish_body
    assert "strchr(targetStart, '&')" in banish_body
    assert "SELECT TOP 1 ClanName FROM UL WHERE ChName=?" in banish_body
    assert "SELECT TOP 1 ClanZang, MemCnt FROM CL WHERE ClanName=?" in banish_body

    assert "SELECT TOP 1 ClanName, Permi FROM UL WHERE ChName=?" in leave_body
    assert "SELECT TOP 1 ClanZang, MemCnt FROM CL WHERE ClanName=?" in leave_body

    client_text = CLIENT_CLAN.read_text(encoding="latin1")
    parser = function_body(
        client_text,
        "void ParsingIsClanMember(char* data, int myposition)",
        "void CLANEntity::ParsingMakeClanMoney",
    )
    assert "if(myposition == 1)   cldata.myPosition = CLANUSER;" in parser
    assert "cldata.myPosition = CLAN_SUBCHIP;" in parser
    assert "cldata.myPosition = CLANCHIP;" in parser

    member_parser = function_body(
        client_text,
        "void CLANEntity::ParsigClanWon(char* data)",
        "void CLANEntity::ParsingClanWonStrLen",
    )
    assert 'pBuf = strstr(szTemp, "CMem")' in member_parser
    assert "k = nPos + 5;" in member_parser
    assert "if(szTemp[k] == '\\r')" in member_parser
    assert "k += 6;" in member_parser

    db = sqlite3.connect(":memory:")
    try:
        apply_schema(db)

        # clanInsert.aspx creation contract. The default mark copy is deliberately
        # best-effort in C++; database success must not depend on the BMP existing.
        code, response, image_id = create_clan(
            db, "create-account", "Creator", "CreatedClan", 4, 80
        )
        assert (code, response, image_id) == (1, "Code=1\rCMoney=500000\r", 1000000001)
        assert db.execute("SELECT IMG,ID FROM ClanDB__LI WHERE ID=1").fetchone() == (1000000001, 1)
        assert db.execute(
            "SELECT UserID,ClanZang,MemCnt,Note,MIconCnt,DelActive,PFlag,KFlag,Flag,NoteCnt,"
            "Cpoint,CWin,CFail,ClanMoney,CNFlag,SiegeMoney FROM ClanDB__CL WHERE ClanName='CreatedClan'"
        ).fetchone() == (
            "create-account", "Creator", 1, "New clan", 1000000001, "0", 0, 0, 0, 1,
            0, 0, 0, 0, 0, 0,
        )
        assert db.execute(
            "SELECT userid,ChName,ClanName,ChType,ChLv,Permi,DelActive,PFlag,KFlag,MIconCnt "
            "FROM ClanDB__UL WHERE ChName='Creator'"
        ).fetchone() == (
            "create-account", "Creator", "CreatedClan", 4, 80, "0", "0", 0, 0, 1000000001
        )
        # Already-in-clan and duplicate clan-name cases must not advance LI.
        assert create_clan(db, "create-account", "Creator", "OtherClan", 4, 80)[0] == 2
        assert create_clan(db, "other-account", "OtherChar", "CreatedClan", 1, 1)[0] == 3
        assert db.execute("SELECT IMG FROM ClanDB__LI WHERE ID=1").fetchone() == (1000000001,)
        # A stale empty UL row is deleted, matching the ASP, and allocation advances.
        db.execute(
            "INSERT INTO ClanDB__UL(userid,ChName,ChType,ChLv,ClanName,Permi,PFlag,KFlag,MIconCnt) "
            "VALUES ('stale','StaleCreator',0,0,'','0',0,0,0)"
        )
        db.commit()
        assert create_clan(db, "stale", "StaleCreator", "SecondClan", 2, 40) == (
            1, "Code=1\rCMoney=500000\r", 1000000002
        )
        assert db.execute("SELECT IMG FROM ClanDB__LI WHERE ID=1").fetchone() == (1000000002,)

        seed_clan(db)
        assert resolve_role(db, "Leader") == (2, 7, 8, 321)
        assert resolve_role(db, "SubChief") == (5, 7, 8, 321)
        assert resolve_role(db, "Member") == (1, 0, 0, 321)
        assert resolve_role(db, "Missing") == (0, 0, 0, 0)
        assert db.execute(
            "SELECT MIconCnt FROM ClanDB__UL WHERE ChName='Member'"
        ).fetchone() == (321,)

        db.execute(
            "INSERT INTO ClanDB__UL(userid,ChName,ChType,ChLv,ClanName,Permi,MIconCnt) "
            "VALUES ('orphan-account','Orphan',1,1,'MissingClan','0',0)"
        )
        assert resolve_role(db, "Orphan") == (0, 0, 0, 0)
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ChName='Orphan'"
        ).fetchone() == (0,)
        db.commit()

        # GetAllMyClanMember contract: CR-only CMem records and the legacy
        # parser remove the current character from the visible list.
        code, response = clan_member_response(db, "Leader")
        assert code == 1
        assert response.startswith("Code=1\rCMem=Leader\r")
        assert response.endswith("CMem=Member\r")
        assert parse_legacy_clan_members(response, "Leader") == ["SubChief", "Member"]
        assert clan_member_response(db, "Missing")[0] == 0

        # Sub-chief update/release preserves the Sandurr Permi contract.
        assert appoint_subchief(db, "Member") == 1
        assert db.execute(
            "SELECT ChName FROM ClanDB__UL WHERE ClanName='Alpha' AND Permi='2'"
        ).fetchall() == [("Member",)]
        assert release_subchief(db, "Member") == 1
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ClanName='Alpha' AND Permi='2'"
        ).fetchone() == (0,)
        assert appoint_subchief(db, "Missing") == 0
        # Restore the original sub-chief for the invite contract below.
        db.execute("UPDATE ClanDB__UL SET Permi='2' WHERE ChName='SubChief'")
        db.commit()

        # Mutation contract: leader and Permi=2 may invite; normal members may not.
        assert join_member(db, "Leader", "Alpha", "NewOne", "new-account", 3, 95) == 1
        assert db.execute(
            "SELECT ClanName,Permi,ChLv FROM ClanDB__UL WHERE ChName='NewOne'"
        ).fetchone() == ("Alpha", "0", 95)
        assert db.execute(
            "SELECT MemCnt FROM ClanDB__CL WHERE ClanName='Alpha'"
        ).fetchone() == (4,)

        assert join_member(db, "Member", "Alpha", "Denied", "denied-account", 1, 10) == 0
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ChName='Denied'"
        ).fetchone() == (0,)

        assert join_member(db, "SubChief", "Alpha", "SubInvite", "subinvite-account", 2, 80) == 1
        assert db.execute(
            "SELECT MemCnt FROM ClanDB__CL WHERE ClanName='Alpha'"
        ).fetchone() == (5,)

        # The historical ASP consumes only clwon1 even if more values are appended.
        assert release_member(db, "Alpha", "clwon1=NewOne&clwon2=SubInvite") == 1
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ChName='NewOne'"
        ).fetchone() == (0,)
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ChName='SubInvite'"
        ).fetchone() == (1,)
        assert release_member(db, "Alpha", "clwon1=Leader") == 4

        assert self_leave(db, "Leader", "Alpha") == 4
        assert self_leave(db, "SubInvite", "Alpha") == 1
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ChName='SubInvite'"
        ).fetchone() == (0,)

        # Capacity code remains 2 and must roll back completely.
        db.execute("UPDATE ClanDB__CL SET MemCnt=100 WHERE ClanName='Alpha'")
        db.commit()
        before = db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ClanName='Alpha'"
        ).fetchone()[0]
        assert join_member(db, "Leader", "Alpha", "TooMany", "full-account", 1, 1) == 2
        after = db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ClanName='Alpha'"
        ).fetchone()[0]
        assert before == after

        # Clan dissolution follows clanRemove.asp: only the leader can remove
        # the clan, and CL/UL are copied to the proven archive tables first.
        db.execute("UPDATE ClanDB__CL SET MemCnt=3 WHERE ClanName='Alpha'")
        db.commit()
        assert dissolve_clan(db, "member-account", "Member", "Alpha") == 0
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__CL WHERE ClanName='Alpha'"
        ).fetchone() == (1,)
        assert dissolve_clan(db, "leader-account", "Leader", "Alpha") == 1
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__CL WHERE ClanName='Alpha'"
        ).fetchone() == (0,)
        assert db.execute(
            "SELECT COUNT(*) FROM ClanDB__UL WHERE ClanName='Alpha'"
        ).fetchone() == (0,)
        assert db.execute(
            "SELECT ClanName,Userid,ClanZang,DelState,DelCase,Server "
            "FROM ClanDB__ClanMainDeleted WHERE ClanName='Alpha'"
        ).fetchone() == ("Alpha", "leader-account", "Leader", "1", "1", 1)
        archived_members = db.execute(
            "SELECT ChName,DelState,DelCase,Server FROM ClanDB__ClanSubDeleted "
            "WHERE ClanName='Alpha' ORDER BY MIDX"
        ).fetchall()
        assert archived_members == [
            ("Leader", "1", "1", 1),
            ("SubChief", "1", "1", 1),
            ("Member", "1", "1", 1),
        ]
    finally:
        db.close()

    print(
        "Clan SQLite contract PASS: create/mark-allocation=PASS, roles=0/1/2/5, "
        "member-list CR/CMem=PASS, sub-chief appoint/release=PASS, "
        "join/kick/leave transactions=PASS, dissolution/archive=PASS, "
        "first-clwon legacy behavior=PASS"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
