#include <cassert>
#include <cstring>
#include "rendering/effects/legacy_sin/SummitAssaLookup252.h"
struct Entry { char resName[64]; };
int main() {
    Entry entries[4]={{"FIRE.bmp"},{"fire.BMP"},{"b_muspel.ASE"},{"MoDel.ASE"}};
    summit::effects::AssaLookup252 cache;
    assert(cache.Find("fire.bmp") == -1);
    cache.Rebuild(entries,4);
    assert(cache.Count()==4);
    assert(cache.Find("fire.bmp")==0); // first match identical to linear scan
    assert(cache.Find("B_MUSPEL.ase")==2);
    assert(cache.Find("unknown.bmp")==-1);
    cache.Invalidate();assert(cache.Count()==-1);
    cache.Rebuild(entries,2);
    assert(cache.Find("model.ase")==-1);
}
