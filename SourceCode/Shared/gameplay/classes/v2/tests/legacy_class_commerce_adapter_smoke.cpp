#include <cassert>

#include "gameplay/commerce/legacy/LegacyClassCommerceAdapter.h"

int main()
{
    using gameplay::commerce::legacy::ManufacturePreviewArmorCode;

    assert(ManufacturePreviewArmorCode(1) == 0x02014300u); // Fighter: DA1 male
    assert(ManufacturePreviewArmorCode(3) == 0x02014400u); // Archer: DA1 female
    assert(ManufacturePreviewArmorCode(7) == 0x02054300u); // Magician: DA2 male
    assert(ManufacturePreviewArmorCode(8) == 0x02054400u); // Priestess: DA2 female
    assert(ManufacturePreviewArmorCode(9) == 0u);          // disabled Assassin
    return 0;
}
