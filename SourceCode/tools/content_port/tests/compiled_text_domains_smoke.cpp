#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "content/v1/text_resource/content/CompiledTextResourceSource.h"
#include "content/v1/definition/content/CompiledDefinitionSource.h"
#include "content/v1/localization/content/CompiledLocalizationSource.h"
#include "content/v1/gameplay_config/content/CompiledGameplayConfigSource.h"
#include "content/v1/stage_spawn/content/CompiledStageSpawnContentSource.h"

static int Fail(const char *message)
{
    std::cerr << "FAIL: " << message << "\n";
    return 1;
}

int main(int argc, char **argv)
{
    if(argc != 6)
        return Fail("expected SRES SCFG SLOC SGCF SSPB paths");

    using namespace summit::content;

    text_resource::CompiledTextResourceSource sres(argv[1]);
    if(!sres.IsValid()) return Fail(sres.LastError().c_str());
    std::vector<unsigned char> bytes;
    std::string contentId;
    if(!sres.Load("Configs/bellatra.ini", bytes, &contentId)) return Fail("server Bellatra alias missing");
    const std::size_t bellatraSize = bytes.size();
    std::vector<unsigned char> clientBellatra;
    if(!sres.Load("Bellatra/bellatra.ini", clientBellatra)) return Fail("client Bellatra alias missing");
    if(clientBellatra != bytes) return Fail("Bellatra client/server aliases differ");
    std::vector<unsigned char> petMessages;
    if(!sres.Load("image/Sinimage/PetData/PetMessage.txt", petMessages)) return Fail("PetMessage compiled UI text missing");
    std::vector<unsigned char> autoHelp;
    if(!sres.Load("image/clanImage/Help/AutoHelp/LevelSubj.ini", autoHelp)) return Fail("AutoHelp level subject config missing");

    definition::CompiledDefinitionSource scfg(argv[2]);
    if(!scfg.IsValid()) return Fail(scfg.LastError().c_str());
    std::vector<definition::DefinitionEntry> monsters, npcs, items;
    if(!scfg.List("GameServer/Monster", ".inf", true, monsters, definition::DefinitionKindMonster)) return Fail("monster list failed");
    if(!scfg.List("GameServer/NPC", ".npc", true, npcs, definition::DefinitionKindNpc)) return Fail("npc list failed");
    if(!scfg.List("GameServer/OpenItem", ".txt", true, items, definition::DefinitionKindOpenItem)) return Fail("item list failed");
    if(monsters.size()!=376u) return Fail("monster direct enumeration parity mismatch");
    if(npcs.size()!=174u) return Fail("npc direct enumeration parity mismatch");
    if(items.size()!=1826u) return Fail("OpenItem direct enumeration parity mismatch");
    definition::CompiledDefinition monster;
    if(monsters.empty() || !scfg.Load(monsters[0].logicalPath.c_str(), monster) || monster.directives.empty()) return Fail("monster semantic load failed");

    localization::CompiledLocalizationSource sloc(argv[3]);
    if(!sloc.IsValid()) return Fail(sloc.LastError().c_str());
    std::string localizedName;
    std::uint32_t localizationFlags=0;
    if(!sloc.Get("pt-BR", "monster.100_arqueira_das_cinzas.name", localizedName, &localizationFlags)) return Fail("SLOC stable monster name lookup failed");
    if(localizedName!="Arqueira das Cinzas") return Fail("SLOC pt-BR monster name mismatch");
    std::string taiwanName;
    if(!sloc.Get("zh-TW", "text.bockdol2.name", taiwanName)) return Fail("SLOC Big5 migration lookup failed");
    if(taiwanName!="福童") return Fail("SLOC Big5 migration decode mismatch");
    std::string englishFallback;
    if(!sloc.Get("fr-FR", "text.bockdol2.name", englishFallback)) return Fail("SLOC English fallback lookup failed");
    if(englishFallback!="福童") return Fail("SLOC English fallback value mismatch");
    std::vector<localization::LocalizationEntry> localeEntries;
    if(!sloc.Entries("en", localeEntries) || localeEntries.empty()) return Fail("SLOC English catalog enumeration failed");

    gameplay_config::CompiledGameplayConfigSource sgcf(argv[4]);
    if(!sgcf.IsValid()) return Fail(sgcf.LastError().c_str());
    std::string lifetime;
    if(!sgcf.Get("Configs/Companion.ini", "Companion", "LifetimeMinutes", lifetime)) return Fail("Companion config lookup failed");
    if(lifetime!="720") return Fail("Companion LifetimeMinutes mismatch");
    std::string skill;
    if(!sgcf.Get("Configs/Skill/Mech.ini", "", "E_Shield_BlockRate", skill)) return Fail("skill config lookup failed");

    stage_spawn::CompiledStageSpawnContentSource sspb(argv[5]);
    if(!sspb.IsValid()) return Fail(sspb.LastError().c_str());
    stage_spawn::StageSpawnDefinition spawn;
    std::string err;
    if(!sspb.Load("GameServer/Field/dun-3.ase.spm", spawn, &err)) return Fail(err.c_str());
    bool stableId=false;
    for(std::size_t i=0;i<spawn.actors.size();++i)
        if(spawn.actors[i].legacyNameBytes=="Cavaleiro Fantasma" && spawn.actors[i].monsterId=="monster.60_cavaleiro_fantasma") stableId=true;
    if(!stableId) return Fail("explicit Cavaleiro Fantasma stable ID override missing");

    std::cout << "PASS"
              << " sres_bellatra_bytes=" << bellatraSize
              << " monsters=" << monsters.size()
              << " npcs=" << npcs.size()
              << " items=" << items.size()
              << " sloc_en_entries=" << localeEntries.size()
              << " spawn_actors=" << spawn.actors.size()
              << "\n";
    return 0;
}
