#pragma once

#include <string>

namespace presentation
{
struct EffectId
{
    std::string key;

    EffectId() {}
    explicit EffectId(const std::string& keyValue) : key(keyValue) {}

    bool Valid() const { return !key.empty(); }
};

struct TextureAssetId
{
    std::string key;

    TextureAssetId() {}
    explicit TextureAssetId(const std::string& keyValue) : key(keyValue) {}

    bool Valid() const { return !key.empty(); }
};

struct AnimationId
{
    std::string key;

    AnimationId() {}
    explicit AnimationId(const std::string& keyValue) : key(keyValue) {}

    bool Valid() const { return !key.empty(); }
};

struct LightId
{
    std::string key;

    LightId() {}
    explicit LightId(const std::string& keyValue) : key(keyValue) {}

    bool Valid() const { return !key.empty(); }
};

struct SoundId
{
    std::string key;

    SoundId() {}
    explicit SoundId(const std::string& keyValue) : key(keyValue) {}

    bool Valid() const { return !key.empty(); }
};
}
