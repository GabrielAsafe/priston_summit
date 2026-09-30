#pragma once

#include "presentation/v2/AudioService.h"

namespace presentation
{
namespace legacy
{
class LegacyAudioAdapter final : public IAudioService
{
public:
    bool PlayOneShot(const SoundId& sound, int volume = 400) override;
    bool PlayForEntity(gameplay::EntityId entity, const SoundId& sound) override;
};

LegacyAudioAdapter& ClientLegacyAudioAdapter();
}
}
