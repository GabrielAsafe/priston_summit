#!/usr/bin/env python3
from pathlib import Path
import sqlite3
import sys
import tempfile
root=Path(__file__).resolve().parents[1]
coin=(root/'SrcServer/src/systems/commerce/coin_shop/CoinShop.cpp').read_text(errors='replace')
time=(root/'SrcServer/src/systems/commerce/coin_shop/CoinShop_T.cpp').read_text(errors='replace')
adapter=(root/'SrcServer/src/systems/commerce/v2/legacy/SQLitePremiumPurchaseReplayStore.h').read_text(errors='replace')
conn=(root/'SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp').read_text(errors='replace')
migration=(root/'Database/migrations/047_premium_purchase_receipts.sql').read_text()
contract=(root/'Shared/gameplay/commerce/v2/PremiumPurchaseReplayGuard.h').read_text(errors='replace')
checks={
 'replay store contract exists': 'class IPremiumPurchaseReplayStore' in contract,
 'memory guard still implements contract': 'public IPremiumPurchaseReplayStore' in contract,
 'coin shop uses durable store': 'SQLitePremiumPurchaseReplayStore store' in coin,
 'time shop uses durable store': 'SQLitePremiumPurchaseReplayStore store' in time,
 'adapter serializes reserve with transaction': 'BeginTransaction()' in adapter and 'SELECT ItemCode,State FROM PremiumPurchaseReceipt' in adapter,
 'adapter persists commit': 'UPDATE PremiumPurchaseReceipt SET State=?' in adapter,
 'migration registered': '{ 47, "047_premium_purchase_receipts.sql" }' in conn,
}
failed=[name for name,value in checks.items() if not value]
if failed:
    for name in failed: print('FAIL:',name,file=sys.stderr)
    raise SystemExit(1)

with tempfile.NamedTemporaryFile(suffix='.sqlite3') as tmp:
    db=sqlite3.connect(tmp.name)
    db.executescript(migration)
    db.execute('INSERT INTO CoinShopDB__PremiumPurchaseReceipt VALUES(?,?,?,?,?,?,?)',
               ('alice',0,7,'BI101',0,'now','now'))
    try:
        db.execute('INSERT INTO CoinShopDB__PremiumPurchaseReceipt VALUES(?,?,?,?,?,?,?)',
                   ('alice',0,7,'BI102',0,'now','now'))
        raise SystemExit('duplicate purchase receipt identity was accepted')
    except sqlite3.IntegrityError:
        pass
    db.execute('UPDATE CoinShopDB__PremiumPurchaseReceipt SET State=1 WHERE PurchaserKey=? AND Scope=? AND RequestId=?',
               ('alice',0,7))
    db.commit(); db.close()
    reopened=sqlite3.connect(tmp.name)
    row=reopened.execute('SELECT ItemCode,State FROM CoinShopDB__PremiumPurchaseReceipt WHERE PurchaserKey=? AND Scope=? AND RequestId=?',
                         ('alice',0,7)).fetchone()
    assert row == ('BI101',1), row
    reopened.execute('INSERT INTO CoinShopDB__PremiumPurchaseReceipt VALUES(?,?,?,?,?,?,?)',
                     ('alice',1,7,'BI201',0,'now','now'))
    reopened.commit(); reopened.close()
print('premium purchase replay persistence v2: PASS')
