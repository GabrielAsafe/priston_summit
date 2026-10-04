#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
h = (root / 'SrcServer/src/systems/progression/class_rank/Rank.h').read_text(errors='replace')
cpp = (root / 'SrcServer/src/systems/progression/class_rank/Rank.cpp').read_text(errors='replace')
repo = (root / 'SrcServer/src/systems/progression/class_rank/v2/legacy/LegacyClassRankRepository.h').read_text(errors='replace')
query_h = (root / 'Shared/gameplay/progression/v2/ClassRankQuery.h').read_text(errors='replace')

for forbidden in ('int\tranktype;', 'int\trankjob_code;', 'ranktype = infos->type', 'rankjob_code = infos->job_code'):
    if forbidden in h or forbidden in cpp:
        raise SystemExit(f'shared class-rank request state remains: {forbidden}')

for required in ('ClassRankQueryService::TryFromLegacy', 'SendRankclassToGame(Player, query)'):
    if required not in cpp:
        raise SystemExit(f'missing request-local class-rank flow: {required}')
if 'RankingMetric::Kills' not in query_h or 'RankingMetric::Kills' not in repo:
    raise SystemExit('kills ranking metric is no longer represented end-to-end')

if 'ClassRankReadService::LoadTop' not in cpp:
    raise SystemExit('Rank handler is not delegating projection/load to ClassRankReadService')
if 'bool IsGlobal() const noexcept' not in query_h:
    raise SystemExit('ClassRankQuery lost explicit global/class-scoped identity')
if 'query.IsGlobal()' not in repo:
    raise SystemExit('repository no longer honors request-local global/class filter')

print('class rank request context validator: PASS')
