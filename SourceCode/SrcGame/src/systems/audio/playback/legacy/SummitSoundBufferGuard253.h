#ifndef SUMMIT_SOUND_BUFFER_GUARD_253_H
#define SUMMIT_SOUND_BUFFER_GUARD_253_H
namespace summit { namespace audio {
// DirectSound slot owns exactly one COM buffer. When replacing the WAV, stop
// playback and release the old buffer before overwriting the slot pointer.
template<class Buffer>
inline void ReleaseSoundBuffer253(Buffer *&slot)
{
    if (!slot) return;
    Buffer *old=slot;
    slot=0;
    old->Stop();
    old->Release();
}
} }
#endif
