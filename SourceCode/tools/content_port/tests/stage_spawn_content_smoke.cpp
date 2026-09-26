#include <cassert>
#include <iostream>
#include <string>

#include "content/v1/stage_spawn/content/CompiledStageSpawnContentSource.h"
#include "content/v1/stage_spawn/content/LegacyStageSpawnContentSource.h"
#include "content/v1/stage_spawn/content/StageSpawnService.h"

using namespace summit::content::stage_spawn;

int main(int argc,char **argv)
{
    if(argc!=4) return 2;
    const char *pack=argv[1];
    const char *logical=argv[2];
    const char *legacyPath=argv[3];

    CompiledStageSpawnContentSource compiled(pack);
    assert(compiled.IsValid());
    StageSpawnService compiledService(compiled);
    StageSpawnDefinition a;
    std::string error;
    assert(compiledService.Load(logical,a,&error));

    LegacyStageSpawnContentSource legacy;
    StageSpawnService legacyService(legacy);
    StageSpawnDefinition b;
    assert(legacyService.Load(legacyPath,b,&error));

    assert(a.limitMax==b.limitMax);
    assert(a.openInterval==b.openInterval);
    assert(a.intervalMilliseconds==b.intervalMilliseconds);
    assert(a.openLimit==b.openLimit);
    assert(a.actors.size()==b.actors.size());
    assert(a.bosses.size()==b.bosses.size());
    for(std::size_t i=0;i<a.actors.size();++i)
    {
        assert(a.actors[i].legacyNameBytes==b.actors[i].legacyNameBytes);
        assert(a.actors[i].weight==b.actors[i].weight);
    }
    std::cout << "PASS stage_spawn_content_smoke actors=" << a.actors.size()
              << " bosses=" << a.bosses.size() << "\n";
    return 0;
}
