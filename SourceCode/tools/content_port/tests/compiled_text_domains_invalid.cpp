#include <iostream>
#include "content/v1/text_resource/content/CompiledTextResourceSource.h"
#include "content/v1/definition/content/CompiledDefinitionSource.h"
#include "content/v1/localization/content/CompiledLocalizationSource.h"
#include "content/v1/gameplay_config/content/CompiledGameplayConfigSource.h"
#include "content/v1/stage_spawn/content/CompiledStageSpawnContentSource.h"
int main(int argc,char **argv){
 if(argc!=6) return 2;
 summit::content::text_resource::CompiledTextResourceSource a(argv[1]);
 summit::content::definition::CompiledDefinitionSource b(argv[2]);
 summit::content::localization::CompiledLocalizationSource c(argv[3]);
 summit::content::gameplay_config::CompiledGameplayConfigSource d(argv[4]);
 summit::content::stage_spawn::CompiledStageSpawnContentSource e(argv[5]);
 if(a.IsValid()||b.IsValid()||c.IsValid()||d.IsValid()||e.IsValid()) { std::cerr<<"FAIL corrupted pack accepted\n"; return 1; }
 if(a.LastError().empty()||b.LastError().empty()||c.LastError().empty()||d.LastError().empty()||e.LastError().empty()) { std::cerr<<"FAIL missing corruption diagnostic\n"; return 1; }
 std::cout<<"PASS corrupted domain packs rejected\n"; return 0;
}
