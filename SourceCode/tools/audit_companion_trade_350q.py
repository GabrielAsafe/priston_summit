#!/usr/bin/env python3
"""Read-only structural acceptance checks. Does NOT replace MinGW/Wine."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'SrcServer/src/systems/gameplay/companions/CompanionSystem.cpp').read_text()
c=(r/'SrcGame/src/systems/ui/v2/legacy/LegacyCompanionGateway.cpp').read_text()
f=(r/'Shared/persistence/character_record/record_legacy_server.inl').read_text(encoding='latin-1')
router=(r/'SrcServer/src/systems/network/v2/legacy/LegacyGameplayPacketRouter.cpp').read_text()
checks={
    'worker read after journal': 'LiveImages350Q().Put(job.tradeNonce' in s,
    'delivery opt-in to protect real characters': 'SUMMIT_ENABLE_TRADE_LIVE_350Q' in s,
    'committed image uses existing transport': 'SendCommittedImage(pending.ownerAfter' in s,
    'response still requires authenticated session': 'live.connected && live.staleSaveWritesSealed' in s,
    'nonce and generation checked on apply ack':'ack->txnId!=job.tradeNonce' in s and 'owner->dwConnectCount!=job.session.generation' in s,
    'session cache/ledger rebased before unseal':s.index('RebaseLivePlayer350Q(owner,*it)')<s.index('ReleaseAfterVerifiedLiveApply(key,job.leaseToken)'),
    'postapply save guard armed before unseal':s.index('ServerTradeLiveFirstSave350Q().Arm(key')<s.index('ReleaseAfterVerifiedLiveApply(key,job.leaseToken)'),
    'first replay rejected before record mutation':f.index('CheckOrReject(')<f.index('TransRecordData.GameSaveInfo.SaveTime = (int)tServerTime;'),
    'only accepted save clears replay gate':'MarkAccepted(summit350eSession)' in f,
    'ack via registered gameplay route':'router.Register(liveAck)' in router,
    'client apply while frozen then ack1':'tradeLiveAwaitingServer350Q_=true' in c,
    'client defers next save until server approval':'tradeLivePostSaveScheduled350Q_=true' in c,
    'client unlocks only after postapply save receipt':'trade::SetTradeClientFrozen(false);\n        summit::companion::diagnostic::Trace("CLIENT","trade-live-unlocked"' in c,
}
for key,ok in checks.items():
    print(f'SUMMIT350Q_AUDIT {"PASS" if ok else "FAIL"} {key}')
if not all(checks.values()): raise SystemExit(1)
print('SUMMIT350Q_AUDIT=PASS (structural; MinGW/Wine still required)')
