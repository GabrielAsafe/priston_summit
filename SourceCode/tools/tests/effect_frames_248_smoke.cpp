#include <cassert>
#include <string>
#include <vector>
#include "rendering/effects/legacy_new/SummitEffectFrames248.h"
int main() {
    using summit::effects::BuildEffectFrameNames248;
    std::vector<std::string> out;
    assert(BuildEffectFrameNames248("Effect/NewEffect/fire.bmp", 3, out));
    assert(out.size() == 3 && out[0] == "Effect/NewEffect/fire1.bmp" && out[2] == "Effect/NewEffect/fire3.bmp");
    assert(BuildEffectFrameNames248("Effect\\fire.TGA", 2, out));
    assert(out[1] == "Effect\\fire2.tga");
    assert(!BuildEffectFrameNames248("no-extension", 1, out) && out.empty());
    assert(!BuildEffectFrameNames248("a.bmp", 513, out));
    assert(!BuildEffectFrameNames248(std::string(125, 'a')+".bmp", 20, out) && out.empty());
    assert(!BuildEffectFrameNames248("a.bmp", -1, out));
    assert(BuildEffectFrameNames248("folder.name/base.png", 1, out));
    assert(out[0] == "folder.name/base1.tga");
    assert(!BuildEffectFrameNames248("folder.bmp/name", 1, out));
    return 0;
}
