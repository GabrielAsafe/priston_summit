#include <cassert>
#include <map>
#include "rendering/effects/legacy_new/SummitEffectMeshKey249.h"
int main(){
    using namespace summit::effects;
    std::map<EffectMeshKey249, int> cache;
    cache[MakeEffectMeshKey249("Mesh.ase", "A.ase")]=11;
    cache[MakeEffectMeshKey249("Mesh.ase", "B.ase")]=22;
    cache[MakeEffectMeshKey249("Mesh.ase", "")]=33;
    assert(cache.size()==3);
    assert(cache[MakeEffectMeshKey249("Mesh.ase", "A.ase")]==11);
    assert(cache[MakeEffectMeshKey249("Mesh.ase", "B.ase")]==22);
    assert(cache[MakeEffectMeshKey249("Mesh.ase", "")]==33);
}
