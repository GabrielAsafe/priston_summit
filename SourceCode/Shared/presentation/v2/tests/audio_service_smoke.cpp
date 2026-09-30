#include <cassert>
#include <string>

#include "presentation/v2/AudioService.h"

class FakeAudioService final : public presentation::IAudioService
{
public:
    FakeAudioService() : calls(0), volumeSeen(0), entityCalls(0), entitySeen() {}
    bool PlayOneShot(const presentation::SoundId& sound, int volume) override
    {
        ++calls;
        key = sound.key;
        volumeSeen = volume;
        return sound.Valid();
    }

    bool PlayForEntity(gameplay::EntityId entity, const presentation::SoundId& sound) override
    {
        ++entityCalls;
        entitySeen = entity;
        key = sound.key;
        return entity.Valid() && sound.Valid();
    }
    int calls;
    int volumeSeen;
    int entityCalls;
    gameplay::EntityId entitySeen;
    std::string key;
};

int main()
{
    FakeAudioService audio;
    presentation::IAudioService& service = audio;
    assert(service.PlayOneShot(presentation::SoundId("ui.stat_reset.consume")));
    assert(audio.calls == 1);
    assert(audio.key == "ui.stat_reset.consume");
    assert(audio.volumeSeen == 400);
    assert(service.PlayForEntity(gameplay::EntityId(77u), presentation::SoundId("character.resurrection_restart")));
    assert(audio.entityCalls == 1);
    assert(audio.entitySeen == gameplay::EntityId(77u));
    assert(audio.key == "character.resurrection_restart");
    assert(!presentation::SoundId().Valid());
    return 0;
}
