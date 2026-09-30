#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/events/v2/WorldEventLifecycleEvents.h').read_text()
runtime_h=(root/'SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.h').read_text()
runtime_cpp=(root/'SrcServer/src/systems/gameplay/events/v2/GameplayEventRuntime.cpp').read_text()
onserver=(root/'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text()

def body(signature, next_signature):
 start=onserver.index(signature)
 end=onserver.index(next_signature, start)
 return onserver[start:end]
item=body('int rsSoD_CatchItem_SD207(', 'int\trsSendCommandSodUserTeam(') if 'int\trsSendCommandSodUserTeam(' in onserver else body('int rsSoD_CatchItem_SD207(', 'int	rsSendCommandSodUserTeam(')
kill_start=onserver.index('int rsOpen_MonsterItemExp(rsKILL_MONSTER_INFO *lpKillMonInfo)')
kill=onserver[kill_start:kill_start+18000]
checks=[
 ('score event contract exists', 'struct EventScoreChanged' in contract and 'PlayerId player' in contract and 'int delta' in contract and 'int teamScore' in contract),
 ('runtime publish api exists', 'PublishEventScoreChanged' in runtime_h and 'PublishEventScoreChanged' in runtime_cpp),
 ('runtime validates event identity', 'event.eventKey.empty() || !event.player.Valid()' in runtime_cpp),
 ('sd207 preserves +5000 player mutation', 'EventScore += 5000' in item and 'rsSodTeamScore[team] += 5000' in item),
 ('sd207 publishes after mutation', item.index('EventScore += 5000') < item.index('PublishEventScoreChanged(')),
 ('sd207 reports bellatra identity', '"bellatra"' in item and '5000, lpPlayInfo->EventScore, team, rsSodTeamScore[team]' in item),
 ('monster score mutation preserved', 'EventScore += score' in kill and 'EventCount++' in kill and 'rsSodTeamScore[scoreTeam] += score' in kill),
 ('monster score event published after mutation', kill.index('EventScore += score') < kill.index('PublishEventScoreChanged(')),
 ('invalid team remains non-mutating', 'const bool scoreTeamValid = scoreTeam >= 0 && scoreTeam < 4' in kill and 'scoreTeamValid ? rsSodTeamScore[scoreTeam] : 0' in kill),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('bellatra score event: PASS')
