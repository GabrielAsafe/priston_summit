#!/usr/bin/env python3
"""Validate BillingDB migration 018 against the catalog recovered from BillingDb202405120913.bak."""

from __future__ import annotations

import sqlite3
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MIGRATION_DIR = ROOT / "Database" / "migrations"
EXPECTED_COLUMNS = {'0CHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], '9CHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'ACHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'BCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'BillingPointCheck20060329': ['IDX', 'UserID', 'EndDay'], 'CCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'DCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'ECHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'Excite': ['NUM', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'Excite_Amount': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'Excite_Bak': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'Excite_Error': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'Excite_Free': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'Excite_NoUse': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'FCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'GCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'HCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'ICHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'JCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'KCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'LCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'MCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'NCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'NetMarble': ['NUM', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'NetMarble_Amount': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'NetMarble_Bak': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'NetMarble_Error': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'NetMarble_Free': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'NetMarble_NoUse': ['NUM', 'IDX', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'NTTBilling': ['NUM', 'EXTERNAL_ID', 'PRISTONTALE_ID', 'GOODS_TYPE', 'CHARGE_TYPE', 'CHARGE_FORM', 'AMOUNT', 'START_DAY', 'END_DAY', 'LEFT_TIME', 'CHARGE_DAY', 'UPDATE_CHECK', 'NOTE'], 'OCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'PCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'QCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'RCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'SCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'Table1': ['day', 'amount', 'count'], 'TCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'UCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'VCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'WCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'XCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'YCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE'], 'ZCHANNEL': ['PRISTONTALE_ID', 'TABLE_NAME', 'INSERT_DATE']}
EXPECTED_IDENTITY = {'0CHANNEL': [], '9CHANNEL': [], 'ACHANNEL': [], 'BCHANNEL': [], 'BillingPointCheck20060329': ['IDX'], 'CCHANNEL': [], 'DCHANNEL': [], 'ECHANNEL': [], 'Excite': ['NUM'], 'Excite_Amount': ['NUM'], 'Excite_Bak': ['NUM'], 'Excite_Error': ['NUM'], 'Excite_Free': ['NUM'], 'Excite_NoUse': ['NUM'], 'FCHANNEL': [], 'GCHANNEL': [], 'HCHANNEL': [], 'ICHANNEL': [], 'JCHANNEL': [], 'KCHANNEL': [], 'LCHANNEL': [], 'MCHANNEL': [], 'NCHANNEL': [], 'NetMarble': ['NUM'], 'NetMarble_Amount': ['NUM'], 'NetMarble_Bak': ['NUM'], 'NetMarble_Error': ['NUM'], 'NetMarble_Free': ['NUM'], 'NetMarble_NoUse': ['NUM'], 'NTTBilling': [], 'OCHANNEL': [], 'PCHANNEL': [], 'QCHANNEL': [], 'RCHANNEL': [], 'SCHANNEL': [], 'Table1': [], 'TCHANNEL': [], 'UCHANNEL': [], 'VCHANNEL': [], 'WCHANNEL': [], 'XCHANNEL': [], 'YCHANNEL': [], 'ZCHANNEL': []}


def apply_all(db: sqlite3.Connection) -> None:
    for path in sorted(MIGRATION_DIR.glob("[0-9][0-9][0-9]_*.sql")):
        db.executescript(path.read_text(encoding="utf-8"))


def columns(db: sqlite3.Connection, table: str) -> list[str]:
    return [row[1] for row in db.execute(f'PRAGMA table_info("{table}")')]


def main() -> None:
    with tempfile.TemporaryDirectory() as td:
        db = sqlite3.connect(Path(td) / "billing.sqlite3")
        apply_all(db)
        physical = {
            row[0] for row in db.execute(
                "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE 'BillingDB__%'"
            )
        }
        expected_physical = {f"BillingDB__{name}" for name in EXPECTED_COLUMNS}
        assert physical == expected_physical, (sorted(expected_physical - physical), sorted(physical - expected_physical))
        for logical, expected in EXPECTED_COLUMNS.items():
            table = f"BillingDB__{logical}"
            actual = columns(db, table)
            assert actual == expected, f"{table}: {actual!r} != {expected!r}"
            info = db.execute(f'PRAGMA table_info("{table}")').fetchall()
            pk_cols = [row[1] for row in info if row[5]]
            expected_identity = EXPECTED_IDENTITY[logical]
            if expected_identity:
                assert pk_cols == expected_identity, f"{table}: identity/PK {pk_cols!r} != {expected_identity!r}"
        db.close()
    print(f"BillingDB SQLite contract PASS: tables={len(EXPECTED_COLUMNS)}, exact column order=PASS, identity translation=PASS")


if __name__ == "__main__":
    main()
