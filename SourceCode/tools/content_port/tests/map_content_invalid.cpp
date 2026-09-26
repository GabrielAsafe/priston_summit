#include <cassert>
#include <iostream>
#include "content/v1/map/content/CompiledMapContentSource.h"
using namespace summit::content::map;
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    CompiledMapContentSource source(argv[1]);
    assert(!source.IsValid());
    assert(!source.LastError().empty());
    std::cout << "PASS invalid_smap error=" << source.LastError() << "\n";
    return 0;
}
