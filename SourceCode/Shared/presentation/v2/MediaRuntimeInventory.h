#pragma once

#include <array>

namespace presentation
{
enum class MediaSubsystem
{
    AudioPlayback,
    AudioEffects,
    VoiceIpc,
    VideoPlayback
};

struct MediaRuntimeDescriptor
{
    MediaSubsystem subsystem;
    bool ownsNativeDevice;
    bool ownsExternalProcessOrIpc;
    bool ownsCodecOrGraph;
};

inline constexpr std::array<MediaRuntimeDescriptor, 4> MediaRuntimeInventory{{
    {MediaSubsystem::AudioPlayback, true, false, true},
    {MediaSubsystem::AudioEffects, true, false, false},
    {MediaSubsystem::VoiceIpc, false, true, false},
    {MediaSubsystem::VideoPlayback, true, false, true},
}};
}
