#include <cassert>
#include <string>

#include "presentation/v2/TextureService.h"

class FakeTextureService final : public presentation::ITextureService
{
public:
    presentation::TextureHandle Load(const presentation::TextureAssetId& texture) override
    {
        key = texture.key;
        return texture.Valid() ? presentation::TextureHandle(17) : presentation::TextureHandle();
    }
    std::string key;
};

int main()
{
    FakeTextureService textures;
    presentation::TextureHandle loaded = textures.Load(
        presentation::TextureAssetId("ui.teleport.window"));
    assert(loaded.Valid());
    assert(loaded.value == 17);
    assert(textures.key == "ui.teleport.window");
    assert(!presentation::TextureHandle().Valid());
    return 0;
}
