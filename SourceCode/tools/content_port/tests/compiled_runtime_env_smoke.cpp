#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include "content/v1/text_resource/content/TextResourceRuntime.h"
#include "content/v1/definition/content/DefinitionRuntime.h"
#include "content/v1/localization/content/LocalizationRuntime.h"
#include "content/v1/gameplay_config/content/GameplayConfigRuntime.h"
static int fail(const char*m){std::cerr<<"FAIL: "<<m<<"\n";return 1;}
int main(int argc,char**argv){
 if(argc!=5) return fail("usage SRES SCFG SLOC SGCF");
 setenv("SUMMIT_TEXT_RESOURCE_PACK",argv[1],1); setenv("SUMMIT_TEXT_RESOURCE_STRICT","1",1);
 setenv("SUMMIT_DEFINITION_PACK",argv[2],1); setenv("SUMMIT_DEFINITION_STRICT","1",1);
 setenv("SUMMIT_LOCALIZATION_PACK",argv[3],1); setenv("SUMMIT_LOCALIZATION_STRICT","1",1); setenv("SUMMIT_LOCALE","pt-BR",1);
 setenv("SUMMIT_GAMEPLAY_CONFIG_PACK",argv[4],1); setenv("SUMMIT_GAMEPLAY_CONFIG_STRICT","1",1);
 if(!summit::content::text_resource::CompiledTextResourcesReady()) return fail("SRES runtime not ready");
 std::string subj; if(!summit::content::text_resource::GetTextResourceProfileString("image/clanImage/Help/AutoHelp/LevelSubj.ini","Level_1","Subj",subj)||subj!="Welcome to Priston Tale.") return fail("AutoHelp INI lookup from pack");
 if(summit::content::text_resource::OpenTextResourceFile("definitely/missing/legacy.txt","rb")) return fail("strict SRES unexpectedly fell back to filesystem");
 FILE*fp=summit::content::text_resource::OpenTextResourceFile("image/Sinimage/PetData/PetMessage.txt","rb"); if(!fp)return fail("PetMessage strict pack stream"); char line[64]={0}; bool read=std::fgets(line,sizeof(line),fp)!=0;std::fclose(fp);if(!read)return fail("PetMessage strict stream empty");
 std::vector<summit::content::definition::DefinitionEntry> defs; if(!summit::content::definition::ListCompiledDefinitions("GameServer/Monster",".inf",true,defs,summit::content::definition::DefinitionKindMonster)||defs.size()!=376u)return fail("SCFG runtime list");
 summit::content::definition::CompiledDefinition d; if(!summit::content::definition::LoadCompiledDefinition(defs[0].logicalPath.c_str(),d)||d.directives.empty())return fail("SCFG runtime load");
 std::string loc; if(!summit::content::localization::GetLocalizationValue("monster.100_arqueira_das_cinzas.name",loc)||loc!="Arqueira das Cinzas")return fail("SLOC runtime locale");
 std::string cfg; if(!summit::content::gameplay_config::GetGameplayConfigValue("Configs/Companion.ini","Companion","LifetimeMinutes",cfg)||cfg!="720")return fail("SGCF runtime lookup");
 std::cout<<"PASS strict compiled runtime resources without legacy filesystem reads\n";return 0;
}
