#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "content/v1/stage_spawn/content/CompiledStageSpawnContentSource.h"
#include "content/v1/stage_spawn/content/LegacyStageSpawnContentSource.h"

namespace fs = std::filesystem;
using namespace summit::content::stage_spawn;

static bool Same(const StageSpawnDefinition& a,const StageSpawnDefinition& b,std::string& why)
{
    if(a.limitMax!=b.limitMax) { why="limitMax"; return false; }
    if(a.openInterval!=b.openInterval) { why="openInterval"; return false; }
    if(a.intervalMilliseconds!=b.intervalMilliseconds) { why="intervalMilliseconds"; return false; }
    if(a.openLimit!=b.openLimit) { why="openLimit"; return false; }
    if(a.actors.size()!=b.actors.size()) { why="actor count"; return false; }
    if(a.bosses.size()!=b.bosses.size()) { why="boss count"; return false; }
    for(std::size_t i=0;i<a.actors.size();++i)
    {
        if(a.actors[i].legacyNameBytes!=b.actors[i].legacyNameBytes) { why="actor name"; return false; }
        if(a.actors[i].weight!=b.actors[i].weight) { why="actor weight"; return false; }
    }
    for(std::size_t i=0;i<a.bosses.size();++i)
    {
        const auto& x=a.bosses[i]; const auto& y=b.bosses[i];
        if(x.masterLegacyNameBytes!=y.masterLegacyNameBytes || x.slaveLegacyNameBytes!=y.slaveLegacyNameBytes || x.slaveCount!=y.slaveCount || x.openHours!=y.openHours)
        { why="boss payload"; return false; }
    }
    return true;
}

int main(int argc,char **argv)
{
    if(argc!=3) return 2;
    CompiledStageSpawnContentSource compiled(argv[1]);
    if(!compiled.IsValid()) { std::cerr << compiled.LastError() << "\n"; return 3; }
    LegacyStageSpawnContentSource legacy;
    fs::path root=argv[2];
    std::vector<fs::path> files;
    for(const auto& entry:fs::directory_iterator(root))
        if(entry.is_regular_file() && entry.path().extension()==".spm") files.push_back(entry.path());
    std::sort(files.begin(),files.end());
    int failed=0;
    for(const fs::path& p:files)
    {
        std::string logical="gameserver/field/"+p.filename().string();
        StageSpawnDefinition a,b; std::string e1,e2,why;
        if(!compiled.Load(logical.c_str(),a,&e1)) { std::cerr << "COMPILED_FAIL "<<p.filename()<<" "<<e1<<"\n"; ++failed; continue; }
        if(!legacy.Load(p.string().c_str(),b,&e2)) { std::cerr << "LEGACY_FAIL "<<p.filename()<<" "<<e2<<"\n"; ++failed; continue; }
        if(!Same(a,b,why)) { std::cerr << "MISMATCH "<<p.filename()<<" "<<why<<"\n"; ++failed; }
    }
    std::cout << "RESULT=" << (failed?"FAIL":"PASS") << " files=" << files.size() << " failures=" << failed << "\n";
    return failed?1:0;
}
