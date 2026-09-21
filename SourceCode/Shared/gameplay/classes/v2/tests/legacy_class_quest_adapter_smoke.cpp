#include <cassert>
#include <cstdint>

#include "gameplay/quests/legacy/LegacyClassQuestAdapter.h"

int main()
{
    using namespace gameplay::quests::legacy;

    assert(SupportsLegacyChangeJobQuestContent(1));
    assert(SupportsLegacyChangeJobQuestContent(8));
    assert(!SupportsLegacyChangeJobQuestContent(9));
    assert(SupportsLegacyChangeJobQuestStep(1, 0));
    assert(SupportsLegacyChangeJobQuestStep(1, 1));
    assert(SupportsLegacyChangeJobQuestStep(1, 2));
    assert(!SupportsLegacyChangeJobQuestStep(1, 3));
    assert(!SupportsLegacyChangeJobQuestStep(9, 0));

    // Starter weapon families are class content, not quest switch statements.
    assert(DefaultWeaponRewardCode(2, 0x00000002u) == 0x01020002u); // Mechanician / claw
    assert(DefaultWeaponRewardCode(1, 0x00000002u) == 0x01010002u); // Fighter / axe
    assert(DefaultWeaponRewardCode(3, 0x00000002u) == 0x01060002u); // Archer / bow
    assert(DefaultWeaponRewardCode(6, 0x00000002u) == 0x01070002u); // Knight / sword
    assert(DefaultWeaponRewardCode(8, 0x00000001u) == 0x01040001u); // Priestess / magic

    assert(QuestBoardPresentationGroup(1) == 5);  // melee
    assert(QuestBoardPresentationGroup(3) == 8);  // projectile
    assert(QuestBoardPresentationGroup(8) == 11); // caster
    assert(QuestBoardPresentationGroup(9) == 0);  // reserved/disabled Assassin

    assert(!UsesCasterStarterDefense(1));
    assert(UsesCasterStarterDefense(7));
    assert(UsesCasterStarterDefense(8));

    assert(ChangeJob3WeaponRewardCode(1) == 0x01010800u);
    assert(ChangeJob3WeaponRewardCode(2) == 0x01020800u);
    assert(ChangeJob3WeaponRewardCode(3) == 0x01060A00u);
    assert(ChangeJob3WeaponRewardCode(4) == 0x01050900u);
    assert(ChangeJob3WeaponRewardCode(5) == 0x01080900u);
    assert(ChangeJob3WeaponRewardCode(6) == 0x01070A00u);
    assert(ChangeJob3WeaponRewardCode(7) == 0x01040900u);
    assert(ChangeJob3WeaponRewardCode(8) == 0x01040900u);
    assert(ChangeJob3WeaponRewardCode(9) == 0u);

    assert(ChangeJob3OrbRewardCode(1) == 0x03050200u);
    assert(ChangeJob3OrbRewardCode(3) == 0x03050300u);
    assert(ChangeJob3OrbRewardCode(5) == 0x03050300u);
    assert(ChangeJob3OrbRewardCode(7) == 0x03050400u);
    assert(ChangeJob3OrbRewardCode(8) == 0x03050400u);
    assert(ChangeJob3OrbRewardCode(9) == 0u);

    assert(ChangeJob3ItemNameIndex(1) == 0);
    assert(ChangeJob3ItemNameIndex(2) == 1);
    assert(ChangeJob3ItemNameIndex(3) == 2);
    assert(ChangeJob3ItemNameIndex(4) == 3);
    assert(ChangeJob3ItemNameIndex(5) == 4);
    assert(ChangeJob3ItemNameIndex(6) == 5);
    assert(ChangeJob3ItemNameIndex(7) == 6);
    assert(ChangeJob3ItemNameIndex(8) == 6);
    assert(ChangeJob3ItemNameIndex(9) == -1);
    assert(!UsesFieldBasedChangeJob3Progress(1));
    assert(UsesFieldBasedChangeJob3Progress(7));
    assert(UsesFieldBasedChangeJob3Progress(8));
    return 0;
}
