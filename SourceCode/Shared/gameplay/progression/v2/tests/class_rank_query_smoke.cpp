#include "Shared/gameplay/progression/v2/ClassRankQuery.h"

#include <cassert>
#include <iostream>

int main()
{
    using namespace gameplay::progression;

    ClassRankQuery query{};
    assert(ClassRankQueryService::TryFromLegacy(1, 0, query));
    assert(query.metric == RankingMetric::Level);
    assert(query.IsGlobal());

    assert(ClassRankQueryService::TryFromLegacy(2, 7, query));
    assert(query.metric == RankingMetric::Kills);
    assert(query.classCode == 7);
    assert(!query.IsGlobal());

    const ClassRankQuery before = query;
    assert(!ClassRankQueryService::TryFromLegacy(99, 3, query));
    assert(query.metric == before.metric);
    assert(query.classCode == before.classCode);

    // Two simultaneous requests are represented as values, not shared singleton state.
    ClassRankQuery a{};
    ClassRankQuery b{};
    assert(ClassRankQueryService::TryFromLegacy(1, 1, a));
    assert(ClassRankQueryService::TryFromLegacy(2, 9, b));
    assert(a.metric == RankingMetric::Level && a.classCode == 1);
    assert(b.metric == RankingMetric::Kills && b.classCode == 9);

    std::cout << "class rank query smoke: PASS\n";
    return 0;
}
