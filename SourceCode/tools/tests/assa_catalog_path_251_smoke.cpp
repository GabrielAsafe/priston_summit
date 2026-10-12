#include <cassert>
#include <cstring>
#include "rendering/effects/legacy_sin/SummitAssaCatalog251.h"
int main() {
    char small[16] = {};
    assert(summit::effects::JoinCatalogPath251(small,"a/","b/","c.bmp"));
    assert(std::strcmp(small,"a/b/c.bmp")==0);
    assert(!summit::effects::JoinCatalogPath251(small,"very/long/","prefix/","path.bmp","/*.*"));
    assert(small[0]==0);
    std::strcpy(small,"root");
    assert(summit::effects::AppendCatalogPath251(small,"/file"));
    assert(std::strcmp(small,"root/file")==0);
    assert(!summit::effects::AppendCatalogPath251(small,"/123456789012"));
}
