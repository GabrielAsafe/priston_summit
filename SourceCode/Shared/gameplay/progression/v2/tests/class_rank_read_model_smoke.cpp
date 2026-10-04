#include <cassert>
#include <vector>
#include "gameplay/progression/v2/ClassRankReadModel.h"

class FakeClassRankRepository final : public gameplay::progression::IClassRankRepository
{
public:
    bool ok = true;
    std::vector<gameplay::progression::ClassRankEntry> source;
    bool Load(const gameplay::progression::ClassRankQuery&, std::vector<gameplay::progression::ClassRankEntry>& entries) override
    {
        if(!ok) return false;
        entries = source;
        return true;
    }
};

int main()
{
    using namespace gameplay::progression;
    FakeClassRankRepository repository;
    for(int i = 0; i < 60; ++i)
        repository.source.push_back(ClassRankEntry{"P", i % 10, 1000 + i, i});

    std::vector<ClassRankEntry> entries;
    assert(ClassRankReadService::LoadTop(repository, ClassRankQuery{}, entries));
    assert(entries.size() == ClassRankReadService::LegacyResultLimit);
    assert(entries.front().level == 1000);
    assert(entries.back().level == 1050);

    repository.ok = false;
    assert(!ClassRankReadService::LoadTop(repository, ClassRankQuery{}, entries));
    assert(entries.empty());
    return 0;
}
