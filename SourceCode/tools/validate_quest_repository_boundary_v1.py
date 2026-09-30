#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
contract=(root/'Shared/gameplay/quests/v2/IQuestRepository.h').read_text(errors='replace')
repo=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestRepository.cpp').read_text(errors='replace')
state=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestStateQuery.cpp').read_text(errors='replace')
handler=(root/'SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp').read_text(errors='replace')
start_handler=handler[handler.index('void cQUESTHANDLER::StartQuest'):handler.index('void cQUESTHANDLER::CancelQuest')]
cancel_handler=handler[handler.index('void cQUESTHANDLER::CancelQuest'):handler.index('bool cQUESTHANDLER::CancelQuestProgressSilent')]
verify_handler=handler[handler.index('void cQUESTHANDLER::VerifyLevel'):handler.index('bool cQUESTHANDLER::KillMonster')]
kill_handler=handler[handler.index('bool cQUESTHANDLER::KillMonster'):handler.index('bool cQUESTHANDLER::updateKilledMonsterOfPartyMember')]
reward_handler=handler[handler.index('void cQUESTHANDLER::GetReward'):handler.index('bool cQUESTHANDLER::inQuest')]
inquest_handler=handler[handler.index('bool cQUESTHANDLER::inQuest'):handler.index('void cQUESTHANDLER::VerifyLevel')]
debug_handler=handler[handler.index('void cQUESTHANDLER::DebugQuest'):]
arena_helper=handler[handler.index('static bool EnsureTier5ArenaObjective'):handler.index('void cQUESTHANDLER::StartQuest')]
objective=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestObjectiveService.cpp').read_text(errors='replace')
lifecycle=(root/'SrcServer/src/systems/gameplay/quests/v2/legacy/LegacyQuestLifecycleService.cpp').read_text(errors='replace')
checks=[
 ('repository contract', 'class IQuestRepository' in contract and 'TryIsCompleted' in contract and 'TryIsInProgress' in contract),
 ('completion SQL preserved', 'SELECT COUNT(*) FROM Complete WHERE QuestID=? AND PlayerName=?' in repo),
 ('progress SQL preserved', 'SELECT COUNT(*) FROM Progress WHERE QuestID=? AND PlayerName=?' in repo),
 ('quest state query composes repository', 'repository_.TryIsCompleted' in state and 'repository_.TryIsInProgress' in state),
 ('state query no cQuestHandler dependency', 'cQuestHandler' not in state),
 ('legacy completion delegates repository', 'ServerLegacyQuestRepository().TryIsCompleted' in handler),
 ('legacy progress delegates repository', 'ServerLegacyQuestRepository().TryIsInProgress' in handler),
 ('repository exact completion read', 'TryIsCompletedVariant' in contract and 'LegacyQuestRepository::TryIsCompletedVariant' in repo),
 ('handler completed reads delegate repository', 'TryIsCompletedVariant' in handler and handler.count('ServerLegacyQuestRepository().TryIsCompleted(') >= 2),
 ('exact completion SQL centralized', 'SELECT QuestID FROM Complete WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?' not in handler),
 ('repository pending objective read', 'TryHasPendingObjectives' in contract and 'TryIsObjectivePending' in contract),
 ('repository objective writes', 'CompleteObjective' in contract and 'CompleteObjectives' in contract and 'CancelProgress' in contract),
 ('objective service composes repository', 'ServerLegacyQuestRepository()' in objective and 'cQuestHandler' not in objective),
 ('handler delegates pending read', 'ServerLegacyQuestRepository().TryHasPendingObjectives' in handler),
 ('handler delegates objective pending read', 'ServerLegacyQuestRepository().TryIsObjectivePending' in handler),
 ('handler delegates objective completion', 'ServerLegacyQuestRepository().CompleteObjective' in handler and 'ServerLegacyQuestRepository().CompleteObjectives' in handler),
 ('handler delegates silent cancel', 'ServerLegacyQuestRepository().CancelProgress' in handler),
 ('repository start progress write', 'StartProgress' in contract and 'LegacyQuestRepository::StartProgress' in repo),
 ('start progress SQL centralized', all(sql in repo and sql not in start_handler for sql in [
    "INSERT INTO Progress SELECT ?2, ?3, ?4, ?5, datetime('now','localtime') WHERE EXISTS(SELECT 1 FROM RepeatedQuest WHERE QuestID=?1)",
    "INSERT INTO Progress SELECT ?2, ?3, ?4, ?5, datetime('now','localtime') WHERE EXISTS(SELECT 1 FROM DailyQuest WHERE QuestID=?1)",
    "INSERT INTO Progress SELECT ?5, ?6, ?7, ?8, datetime('now','localtime') WHERE EXISTS(SELECT 1 FROM SingleQuest WHERE QuestID=?1) AND NOT EXISTS(SELECT 1 FROM Progress WHERE QuestID=?2 AND QuestKind=?3 AND PlayerName=?4)",
    "SELECT * FROM QuestMonsters WHERE QuestID=? AND QuestKind=? AND QuestType=?",
    "INSERT INTO MonsterKilled VALUES (?,0,?,?,?,?,?,?,?)"])),
 ('handler delegates start progress', 'ServerLegacyQuestRepository().StartProgress' in handler),
 ('repository exact cancel write', 'CancelProgressVariant' in contract and 'LegacyQuestRepository::CancelProgressVariant' in repo),
 ('handler delegates exact cancel', 'ServerLegacyQuestRepository().CancelProgressVariant' in cancel_handler),
 ('exact cancel SQL centralized', all(sql in repo and sql not in cancel_handler for sql in [
    'DELETE FROM Progress WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?',
    'DELETE FROM MonsterKilled WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?'])),
 ('repository level validation read', 'ListInvalidForLevel' in contract and 'LegacyQuestRepository::ListInvalidForLevel' in repo),
 ('lifecycle composes level repository', 'ServerLegacyQuestRepository().ListInvalidForLevel' in lifecycle and 'cQuestHandler' not in lifecycle),
 ('level validation SQL centralized', all(sql in repo and sql not in verify_handler for sql in [
    'SELECT * FROM Progress WHERE PlayerName=?',
    'SELECT * FROM RepeatedQuest WHERE QuestID=? AND QuestType=? AND (LevelMin > ? OR LevelMax < ?)',
    'SELECT * FROM DailyQuest WHERE QuestID=? AND QuestType=? AND (LevelMin > ? OR LevelMax < ?)',
    'SELECT * FROM SingleQuest WHERE QuestID=? AND QuestType=? AND (LevelMin > ? OR LevelMax < ?)'])),
 ('repository monster kill persistence', 'RecordMonsterKill' in contract and 'LegacyQuestRepository::RecordMonsterKill' in repo),
 ('handler delegates monster kill persistence', 'ServerLegacyQuestRepository().RecordMonsterKill' in kill_handler),
 ('monster kill SQL centralized', all(sql in repo and sql not in kill_handler for sql in [
    'UPDATE MonsterKilled SET Killed=Killed+1 ',
    'SELECT changes()',
    'SELECT QuestID,QuestKind,QuestType,Killed,\\"Count\\" '])),
 ('monster kill exclusions preserved', 'QuestID NOT IN (10010,10011)' in repo and 'QuestKind != 8' in repo and 'PartyFlag=1' in repo),
 ('repository reward claim persistence', 'ClaimReward' in contract and 'LegacyQuestRepository::ClaimReward' in repo),
 ('handler delegates reward claim', 'ServerLegacyQuestRepository().ClaimReward' in reward_handler),
 ('reward SQL centralized', all(sql in repo and sql not in reward_handler for sql in [
    'SELECT * FROM Progress WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?',
    'SELECT * FROM MonsterKilled WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?',
    'INSERT OR REPLACE INTO Complete (QuestID, QuestKind, QuestType, PlayerName, EndDate)',
    'SELECT * FROM Rewards WHERE QuestID=? AND QuestKind=? AND QuestType=?',
    'SELECT * FROM ItemRewards WHERE QuestID=? AND QuestKind=? AND QuestType=?'])),
 ('reward side effects stay handler-side', all(token in reward_handler for token in [
    'OPCODE_ADDEXP', 'SendPostBox', 'cCoinShop.AddPlayerCoin', 'Itens enviados ao Distribuidor.'])),
 ('reward payload exposes legacy fields', all(token in contract for token in [
    'QuestRewardPayload', 'experience', 'gold', 'coin', 'tCoin', 'QuestItemRewardRecord'])),
 ('repository party objective target query', 'HasPartyObjectiveTarget' in contract and 'LegacyQuestRepository::HasPartyObjectiveTarget' in repo),
 ('handler delegates party objective target query', 'ServerLegacyQuestRepository().HasPartyObjectiveTarget' in inquest_handler),
 ('party objective SQL centralized', r'PartyFlag=1 AND Killed < \"Count\" LIMIT 1' in repo and 'db->Prepare' not in inquest_handler),
 ('debug reset delegates repository', 'ServerLegacyQuestRepository().CancelProgress' in debug_handler),
 ('debug reset has no direct SQL', 'db->Prepare' not in debug_handler and 'SQLConnection' not in debug_handler),
 ('repository objective ensure write', 'EnsureObjective' in contract and 'LegacyQuestRepository::EnsureObjective' in repo),
 ('tier5 arena objective delegates repository', 'ServerLegacyQuestRepository().EnsureObjective' in arena_helper),
 ('tier5 arena objective constants preserved', all(token in arena_helper for token in [
    'QUESTID_T5_QUEST_ARENA', '"Crystal Tower"', '\t9,', '\t1,', '\t0,', '\t32'])),
 ('tier5 arena objective has no direct SQL', 'db->Prepare' not in arena_helper and 'SQLConnection' not in arena_helper),
 ('repository admin completion write', 'MarkSingleCompletedForTesting' in contract and 'LegacyQuestRepository::MarkSingleCompletedForTesting' in repo),
 ('admin completion SQL centralized', all(sql in repo and sql not in handler for sql in [
    'SELECT COALESCE(MAX(QuestType),0), COUNT(*) FROM SingleQuest WHERE QuestID=?',
    'INSERT OR REPLACE INTO Complete (QuestID, QuestKind, QuestType, PlayerName, EndDate)'])),
 ('objective SQL centralized in repository', all(sql not in handler for sql in [
    'SELECT QuestID FROM MonsterKilled WHERE PlayerName=? AND QuestID=? AND Killed < Count',
    'SELECT QuestID FROM MonsterKilled WHERE PlayerName=? AND QuestID=? AND MonsterName=? AND Killed < Count',
    'UPDATE MonsterKilled SET Killed=Count WHERE PlayerName=? AND QuestID=? AND MonsterName=? AND Killed < Count',
    'UPDATE MonsterKilled SET Killed=Count WHERE PlayerName=? AND QuestID=? AND Killed < Count'])),
]
failed=[name for name,ok in checks if not ok]
for name,ok in checks: print(('PASS: ' if ok else 'FAIL: ')+name)
if failed: sys.exit(1)
print('quest repository boundary: PASS')
