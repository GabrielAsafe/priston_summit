#include "systems/presentation/v2/legacy/LegacyAudioAdapter.h"

// Keep the ABI dependency narrow. Pulling the historical audio/inventory headers
// here would also import Win32/D3D and character types into this adapter.
extern int sinPlaySound(int Num, int Volume);
class smCHAR;
extern smCHAR* FindChrPlayer(unsigned long dwObjectSerial);
extern int CharPlaySound(smCHAR* character);

namespace presentation
{
namespace legacy
{
namespace
{
const int kEatPotion2SoundIndex = 25; // SIN_SOUND_EAT_POTION2
}

bool LegacyAudioAdapter::PlayOneShot(const SoundId& sound, int volume)
{
    if (!sound.Valid())
        return false;

    int soundIndex = -1;
    if (sound.key == "ui.stat_reset.consume")
        soundIndex = kEatPotion2SoundIndex;
    else
        return false;

    return sinPlaySound(soundIndex, volume) != 0;
}

bool LegacyAudioAdapter::PlayForEntity(gameplay::EntityId entity, const SoundId& sound)
{
    if (!entity.Valid() || !sound.Valid())
        return false;

    if (sound.key != "character.resurrection_restart")
        return false;

    smCHAR* character = FindChrPlayer(static_cast<unsigned long>(entity.value));
    return character && CharPlaySound(character) != 0;
}

LegacyAudioAdapter& ClientLegacyAudioAdapter()
{
    static LegacyAudioAdapter adapter;
    return adapter;
}
}
}
