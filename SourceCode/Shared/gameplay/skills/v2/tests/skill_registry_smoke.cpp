#include <cassert>
#include <iostream>

#include "gameplay/skills/v2/content/LegacySkillCatalog.h"
#include "gameplay/skills/v2/services/SkillService.h"
#include "gameplay/skills/v2/legacy/LegacySkillAdapter.h"

int main()
{
    const gameplay::skills::SkillRegistry& registry = gameplay::skills::DefaultSkillRegistry();
    const gameplay::skills::SkillService& service = gameplay::skills::DefaultSkillService();

    assert(registry.AllSkills().size() == 160u);

    int active = 0;
    int passive = 0;
    int tiers[5] = {0, 0, 0, 0, 0};
    for(std::vector<gameplay::skills::SkillDefinition>::const_iterator it = registry.AllSkills().begin();
        it != registry.AllSkills().end(); ++it)
    {
        assert(it->tier >= 1 && it->tier <= 5);
        ++tiers[it->tier - 1];
        if(it->IsPassive()) ++passive;
        else if(it->IsExecutable()) ++active;
        assert(service.ResolveSlot(it->ownerClassId, it->useSlot) == &*it);
    }

    assert(active == 142);
    assert(passive == 18);
    for(int i = 0; i < 5; ++i) assert(tiers[i] == 32);

    // Magician T5 Stone Skin: logical id stays Rank5 while presentation reuses
    // Energy Shield visual play code (0x89), network remains T52 (0xE2).
    const gameplay::skills::SkillDefinition* stoneSkin = service.ResolveSlot(7, 18);
    assert(stoneSkin);
    assert(stoneSkin->id == 0x08100002u);
    assert(stoneSkin->legacy.playCode == 0xE2u);
    assert(stoneSkin->legacy.presentationPlayCode == 0x89u);
    assert(service.IsBuff(stoneSkin->id));

    // Pikeman T5 Sinister Spin keeps T54 (0xE4) on the wire/gameplay side,
    // but reuses Tornado (0x16) strictly as its legacy presentation alias.
    const gameplay::skills::SkillDefinition* sinisterSpin = service.ResolveSlot(4, 20);
    assert(sinisterSpin);
    assert(sinisterSpin->id == 0x03100004u);
    assert(sinisterSpin->legacy.playCode == 0xE4u);
    assert(sinisterSpin->legacy.presentationPlayCode == 0x16u);

    const gameplay::skills::LegacySkillAdapter& legacy = gameplay::skills::DefaultLegacySkillAdapter();
    assert(legacy.IsBuff(0x01040004u)); // Spark Shield is reactive but legacy timed-effect/buff.
    assert(legacy.IsBuff(0x04020002u)); // Falcon is a Summon in domain, buff path in legacy.
    assert(service.Find(0x04020002u)->behavior == gameplay::skills::SkillBehavior::Summon);

    std::cout << "skill_registry_smoke: PASS\n";
    return 0;
}
