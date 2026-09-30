#pragma once

#include "presentation/v2/TextureService.h"

namespace presentation
{
namespace legacy
{
class LegacyTextureAdapter final : public ITextureService
{
public:
    TextureHandle Load(const TextureAssetId& texture) override;
};

LegacyTextureAdapter& ClientLegacyTextureAdapter();
}
}
