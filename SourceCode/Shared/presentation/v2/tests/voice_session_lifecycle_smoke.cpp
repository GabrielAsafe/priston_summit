#include "Shared/presentation/v2/VoiceSessionLifecycle.h"
#include <cassert>
#include <iostream>

int main()
{
    presentation::VoiceSessionLifecycle lifecycle;
    assert(lifecycle.Stage() == presentation::VoiceSessionStage::Stopped);
    assert(!lifecycle.CanUseIpc());
    assert(!lifecycle.MarkIpcReady());
    assert(lifecycle.MarkRemoteReady());
    assert(lifecycle.RemoteAvailable());
    assert(!lifecycle.CanUseIpc());
    assert(lifecycle.MarkIpcReady());
    assert(lifecycle.CanUseIpc());
    assert(!lifecycle.MarkRemoteReady());
    lifecycle.Close();
    assert(lifecycle.Stage() == presentation::VoiceSessionStage::Stopped);
    assert(lifecycle.MarkRemoteReady());
    std::cout << "voice session lifecycle smoke: PASS\n";
    return 0;
}
