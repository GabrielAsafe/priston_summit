#pragma once

#include "gameplay/core/v2/GameplayIds.h"
#include "presentation/v2/PresentationIds.h"

namespace presentation
{
class IAudioService
{
public:
    virtual ~IAudioService() {}
    virtual bool PlayOneShot(const SoundId& sound, int volume = 400) = 0;
    virtual bool PlayForEntity(gameplay::EntityId entity, const SoundId& sound) = 0;
};
}
