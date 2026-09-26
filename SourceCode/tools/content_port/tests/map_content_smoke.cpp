#include <cassert>
#include <iostream>
#include <string>

#include "content/v1/map/content/CompiledMapContentSource.h"

using namespace summit::content::map;

int main(int argc,char **argv)
{
    if(argc!=3) return 2;
    CompiledMapContentSource source(argv[1]);
    if(!source.IsValid()) { std::cerr << source.LastError() << "\n"; return 3; }
    MapCatalogDefinition catalog;
    std::string error;
    assert(source.LoadCatalog(catalog,&error));
    assert(catalog.maps.size()==53);
    assert(catalog.dynamicFieldCode==53);
    assert(catalog.staticFieldCount==53);
    assert(catalog.profile==MapProfileClient || catalog.profile==MapProfileServer);

    MapDefinition map;
    assert(source.LoadField(0,map,&error));
    assert(map.id=="map.fore_3");
    assert(map.stagePath=="forest//fore-3.ase");
    assert(map.stageObjects.size()==14);
    assert(map.warpGates.size()==1);
    assert(map.warpGates[0].outGates.size()==1);

    const int requested=std::atoi(argv[2]);
    assert(source.LoadField(requested,map,&error));
    std::cout << "PASS map_content_smoke profile=" << static_cast<int>(catalog.profile)
              << " maps=" << catalog.maps.size() << " field=" << requested
              << " id=" << map.id << "\n";
    return 0;
}
