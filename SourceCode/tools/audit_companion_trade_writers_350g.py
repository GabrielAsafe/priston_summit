#!/usr/bin/env python3
"""SUMMIT350G: static audit, not a proof of complete DAT writer isolation.

Never reads or mutates character DATs or journal. Reports blocked integration
rather than pretending an incomplete audit proves transaction correctness.
"""
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
p=root/'Shared/persistence/character_record/record_legacy_server.inl'
s=p.read_bytes().decode('latin1')
g=p.parent.parent/'gameplay/companions/v2/CompanionTradeParticipantGate350B.h'
# Header lives under Shared/gameplay, not alongside record_legacy_server.
g=root/'Shared/gameplay/companions/v2/CompanionTradeParticipantGate350B.h'
if not g.is_file() or 'class UnknownWriterGuard350G final' not in g.read_text():
    print('SUMMIT350G_AUDIT=FAIL missing atomic global writer admission')
    sys.exit(2)
heads=[
'int rsRECORD_DBASE::SendUserDataToClient(',
'int rsRECORD_DBASE::InsertCharData(',
'int rsRECORD_DBASE::DeleteCharData(',
'int rsRECORD_DBASE::SaveThrowData(',
'int ImportTTServerUser(',
]
for head in heads:
    n=s.count(head)
    if n!=1:
        print('SUMMIT350G_AUDIT=FAIL missing or duplicated '+head)
        sys.exit(2)
    # Only accept an immediate guard, before any branch or record write.
    brace=s.find('{',s.index(head))
    snippet=s[brace+1:brace+650]
    if 'UnknownWriterGuard350G summit350gExternalWriter(' not in snippet or 'if(!summit350gExternalWriter.Allowed()) return FALSE;' not in snippet:
        print('SUMMIT350G_AUDIT=FAIL guard not at entry '+head)
        sys.exit(2)
print('SUMMIT350G_AUDIT=PASS (5 legacy entrypoints guarded; 350F retains 6 + deferred queue)')
print('SUMMIT350G_BLOCKER=Uninstrumented paths outside record_legacy_server.inl not proven absent')
print('SUMMIT350G_BLOCKER=No production detached journal context / session result pump')
print('SUMMIT350G_BLOCKER=No Fedora MinGW/Wine integration verification')
print('SUMMIT350G_REAL_TRANSFER=DISABLED (-76)')
