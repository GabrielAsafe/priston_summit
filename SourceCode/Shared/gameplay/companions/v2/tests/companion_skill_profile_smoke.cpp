#include <cassert>

#include "gameplay/companions/v2/CompanionSkillProfile.h"

int main()
{
    using gameplay::companions::DefaultDamageSkillForClass;
    using gameplay::companions::HasCompanionHealingRole;

    assert(DefaultDamageSkillForClass(1) == 0x02010004u);
    assert(DefaultDamageSkillForClass(2) == 0x01020001u);
    assert(DefaultDamageSkillForClass(3) == 0x04010003u);
    assert(DefaultDamageSkillForClass(4) == 0x03010004u);
    assert(DefaultDamageSkillForClass(5) == 0x06020002u);
    assert(DefaultDamageSkillForClass(6) == 0x05010001u);
    assert(DefaultDamageSkillForClass(7) == 0x08010002u);
    assert(DefaultDamageSkillForClass(8) == 0x07010002u);
    assert(DefaultDamageSkillForClass(0) == 0u);
    assert(DefaultDamageSkillForClass(9) == 0u);

    for(int classId = 1; classId <= 7; ++classId)
        assert(!HasCompanionHealingRole(classId));
    assert(HasCompanionHealingRole(8));
    assert(!HasCompanionHealingRole(0));
    assert(!HasCompanionHealingRole(9));
    return 0;
}
