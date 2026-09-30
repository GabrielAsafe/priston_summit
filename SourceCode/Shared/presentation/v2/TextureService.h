#pragma once

#include "presentation/v2/PresentationIds.h"

namespace presentation
{
struct TextureHandle
{
    int value;

    explicit TextureHandle(int valueValue = -1) : value(valueValue) {}
    bool Valid() const { return value >= 0; }
};

class ITextureService
{
public:
    virtual ~ITextureService() {}
    virtual TextureHandle Load(const TextureAssetId& texture) = 0;
};
}
