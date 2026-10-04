#include "Shared/presentation/v2/MediaRuntimeInventory.h"
#include <cassert>
#include <iostream>

int main()
{
    using namespace presentation;
    assert(MediaRuntimeInventory.size() == 4u);
    assert(MediaRuntimeInventory[0].subsystem == MediaSubsystem::AudioPlayback);
    assert(MediaRuntimeInventory[2].subsystem == MediaSubsystem::VoiceIpc);
    assert(MediaRuntimeInventory[2].ownsExternalProcessOrIpc);
    assert(MediaRuntimeInventory[3].subsystem == MediaSubsystem::VideoPlayback);
    assert(MediaRuntimeInventory[3].ownsCodecOrGraph);
    std::cout << "media runtime inventory smoke: PASS\n";
    return 0;
}
