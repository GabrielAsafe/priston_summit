#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "content/v1/field_runtime/content/CompiledFieldRuntimeSource.h"
#include "content/v1/stage_spawn/content/CompiledStageSpawnContentSource.h"
#include "content/v1/stage_spawn/content/StageSpawnService.h"

int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    summit::content::field_runtime::CompiledFieldRuntimeSource field(argv[1]);
    assert(field.IsValid());

    summit::content::field_runtime::StageStartPointSet starts;
    std::string error;
    assert(field.LoadStartPoints("GameServer\\Field\\Village-2.ase.spp",starts,&error));
    assert(starts.points.size()==2);
    assert(starts.points[0].slot==0 && starts.points[0].x==-1000 && starts.points[0].z==-2000);
    assert(starts.points[1].slot==5 && starts.points[1].state==2);

    summit::content::field_runtime::FixedNpcSet fixed;
    assert(field.LoadFixedNpcs("GameServer/Field/Village-2.ase.spc",fixed,&error));
    assert(fixed.npcs.size()==2);
    assert(fixed.npcs[0].slot==20 && fixed.npcs[0].npcId=="npc.ji_woo");
    assert(fixed.npcs[0].definitionPath=="GameServer/NPC/ji_woo.npc");
    assert(fixed.npcs[1].slot==41 && fixed.npcs[1].npcId=="npc.enzo_o_restaure");

    std::vector<unsigned char> spawnBytes;
    assert(field.LoadStageSpawnSection(spawnBytes,&error));
    summit::content::stage_spawn::CompiledStageSpawnContentSource spawnSource(spawnBytes);
    assert(spawnSource.IsValid());
    summit::content::stage_spawn::StageSpawnService spawns(spawnSource);
    summit::content::stage_spawn::StageSpawnDefinition spawn;
    assert(spawns.Load("GameServer/Field/Village-2.ase.spm",spawn,&error));

    std::cout << "PASS field_runtime_content_smoke starts=" << starts.points.size()
              << " fixed=" << fixed.npcs.size()
              << " spawn_actors=" << spawn.actors.size() << "\n";
    return 0;
}
