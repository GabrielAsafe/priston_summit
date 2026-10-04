#pragma once

namespace presentation
{
enum class VoiceSessionStage
{
    Stopped = 0,
    RemoteReady,
    IpcReady
};

class VoiceSessionLifecycle
{
public:
    VoiceSessionLifecycle() : stage_(VoiceSessionStage::Stopped) {}

    VoiceSessionStage Stage() const noexcept { return stage_; }
    bool RemoteAvailable() const noexcept { return stage_ != VoiceSessionStage::Stopped; }
    bool CanUseIpc() const noexcept { return stage_ == VoiceSessionStage::IpcReady; }

    bool MarkRemoteReady() noexcept
    {
        if (stage_ != VoiceSessionStage::Stopped)
            return false;
        stage_ = VoiceSessionStage::RemoteReady;
        return true;
    }

    bool MarkIpcReady() noexcept
    {
        if (stage_ != VoiceSessionStage::RemoteReady)
            return false;
        stage_ = VoiceSessionStage::IpcReady;
        return true;
    }

    void Close() noexcept { stage_ = VoiceSessionStage::Stopped; }

private:
    VoiceSessionStage stage_;
};
}
