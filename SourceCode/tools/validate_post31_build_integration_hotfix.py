#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
onserver = (root / 'Shared/server/legacy_runtime/OnSever_legacy_server.inl').read_text(errors='replace')
rank_h = (root / 'SrcServer/src/systems/progression/class_rank/Rank.h').read_text(errors='replace')
rank_cpp = (root / 'SrcServer/src/systems/progression/class_rank/Rank.cpp').read_text(errors='replace')
battle = (root / 'SrcServer/src/systems/progression/battle_pass/BattlePass.cpp').read_text(errors='replace')

legacy_overload = 'SendRankclassToGame(rsPLAYINFO* Player);'
query_overload = 'SendRankclassToGame(rsPLAYINFO* Player, const gameplay::progression::ClassRankQuery& query);'
if legacy_overload not in rank_h or query_overload not in rank_h:
    raise SystemExit('class-rank compatibility/query overload pair is incomplete')
if 'void Rank::SendRankclassToGame(rsPLAYINFO* Player)\n{' not in rank_cpp:
    raise SystemExit('class-rank UI-open compatibility bridge is missing')
if 'ranktype' in rank_h or 'rankjob_code' in rank_h:
    raise SystemExit('shared mutable class-rank request state was reintroduced')

selection = 'const authentication::v2::CharacterSelectionDecision selectionDecision ='
selection_pos = onserver.find(selection)
if selection_pos < 0:
    raise SystemExit('character selection decision integration is missing')
scope_open = onserver.rfind('{', 0, selection_pos)
next_case = onserver.find('case OpCode::OPCODE_ID_GETUSERINFO', selection_pos)
scope_close = onserver.find('\n\t\t\t}', selection_pos)
if scope_open < 0 or scope_close < 0 or next_case < 0 or scope_close > next_case:
    raise SystemExit('character selection decision is not scoped before the next switch case')

required_claim_fragments = (
    'const gameplay::progression::BattlePassClaimDecision claimDecision =',
    'BattlePassClaimPolicy::Evaluate(claimContext)',
    'claimDecision != gameplay::progression::BattlePassClaimDecision::PassNotEntitled',
    'claimDecision != gameplay::progression::BattlePassClaimDecision::LevelTooLow',
    'claimDecision != gameplay::progression::BattlePassClaimDecision::AlreadyClaimed',
)
for fragment in required_claim_fragments:
    if fragment not in battle:
        raise SystemExit(f'battle-pass claim control flow missing: {fragment}')

print('post31 build integration hotfix validator: PASS')
