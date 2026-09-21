#include <cassert>

#include "gameplay/classes/v2/legacy/LegacyClassOnboardingAdapter.h"

int main()
{
    int lowLevelPresentRow = -1;
    assert(gameplay::classes::legacy::TryLowLevelPresentItemRow(1, lowLevelPresentRow));
    assert(lowLevelPresentRow == 0);
    assert(gameplay::classes::legacy::TryLowLevelPresentItemRow(8, lowLevelPresentRow));
    assert(lowLevelPresentRow == 7);
    assert(!gameplay::classes::legacy::TryLowLevelPresentItemRow(9, lowLevelPresentRow));
    assert(!gameplay::classes::legacy::TryLowLevelPresentItemRow(42, lowLevelPresentRow));

    using namespace gameplay::classes::legacy;

    assert(StarterPresentWeaponCode(3, 0x00000100u) == 0x01060100u); // Archer/bow
    assert(StarterPresentWeaponCode(5, 0x00000100u) == 0x01080100u); // Atalanta/javelin
    assert(StarterPresentWeaponCode(7, 0x00000100u) == 0x01040100u); // Magician/staff
    assert(StarterPresentWeaponCode(8, 0x00000100u) == 0x01040100u); // Priestess/staff
    assert(StarterPresentWeaponCode(1, 0x00000100u) == 0u);          // melee legacy random fallback
    assert(StarterPresentWeaponCode(9, 0x00000100u) == 0u);          // disabled class

    assert(StarterPresentArmorCode(1, 0x00000200u) == 0x02010200u);
    assert(StarterPresentArmorCode(7, 0x00000200u) == 0x02050200u);
    assert(StarterPresentArmorCode(9, 0x00000200u) == 0u);

    return 0;
}
