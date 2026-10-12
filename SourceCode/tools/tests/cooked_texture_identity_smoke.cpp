#include <cassert>
#include <iostream>
#include <string>
#include "Shared/rendering/legacy_3d/material/SummitCookedTextureIdentity.h"

int main()
{
    using summit::rendering::cooked::BuildRelativePath;
    using summit::rendering::cooked::Dib;
    using summit::rendering::cooked::Material;
    assert(BuildRelativePath("image/Sinimage/Items/WA/itwa101.bmp", Dib) ==
        "dib\\image\\Sinimage\\Items\\WA\\itwa101.bmp.dds");
    assert(BuildRelativePath("image\\Sinimage\\Items\\DropItem\\a.bmp", Material) ==
        "material\\image\\Sinimage\\Items\\DropItem\\a.bmp.dds");
    assert(BuildRelativePath("image\\Sinimage\\Items\\DropItem\\a.tga", Material) ==
        "material\\image\\Sinimage\\Items\\DropItem\\a.tga.dds");
    assert(BuildRelativePath("./Field//a.BMP", Material) == "material\\Field\\a.BMP.dds");
    assert(BuildRelativePath("../Field/a.bmp", Material).empty());
    assert(BuildRelativePath("C:\\Field\\a.bmp", Material).empty());
    assert(BuildRelativePath("/Field/a.bmp", Material).empty());
    assert(BuildRelativePath("Field/../../../a.bmp", Material).empty());
    assert(BuildRelativePath("Field/a.dds", Material).empty());
    assert(BuildRelativePath("Field/a.exe", Material).empty());
    assert(BuildRelativePath("Field/a.bmp", Material) != BuildRelativePath("Field/a.tga", Material));
    assert(BuildRelativePath("Field/a.bmp", Material) != BuildRelativePath("Field/a.bmp", Dib));
    std::cout << "SUMMIT_COOKED_IDENTITY_CPP=PASS\n";
}
