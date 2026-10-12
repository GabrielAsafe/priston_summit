#pragma once
// SUMMIT 247: conservative per-slot WAV buffer reuse, independent of DirectSound.
// No allocation, disk access, telemetry or environment variables in this state.
// Caller must invalidate on every buffer replacement, release or device reset.
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace summit {
namespace audio {

struct WavStamp247 {
    uint64_t size;
    uint64_t write_time;
};

template<size_t Slots, size_t MaxPath = 260>
class WavSlotReuse247 {
public:
    WavSlotReuse247() { Clear(); }

    void Clear() {
        for (size_t i = 0; i < Slots; ++i) Forget(i);
    }

    void Forget(size_t slot) {
        if (slot >= Slots) return;
        slots_[slot].valid = false;
        slots_[slot].path[0] = '\0';
        slots_[slot].stamp.size = 0;
        slots_[slot].stamp.write_time = 0;
    }

    bool Remember(size_t slot, const char *path, const WavStamp247 &stamp) {
        if (slot >= Slots || !path) return false;
        size_t length = 0;
        while (length < MaxPath && path[length]) ++length;
        Forget(slot);
        if (!length || length >= MaxPath) return false;
        memcpy(slots_[slot].path, path, length + 1);
        slots_[slot].stamp = stamp;
        slots_[slot].valid = true;
        return true;
    }

    bool Matches(size_t slot, const char *path, const WavStamp247 &stamp) const {
        if (slot >= Slots || !path || !slots_[slot].valid) return false;
        // Exact spelling matters under Wine on case-sensitive filesystems.
        return strcmp(slots_[slot].path, path) == 0 &&
               slots_[slot].stamp.size == stamp.size &&
               slots_[slot].stamp.write_time == stamp.write_time;
    }

private:
    struct Entry {
        bool valid;
        char path[MaxPath];
        WavStamp247 stamp;
    };
    Entry slots_[Slots];
};

} // namespace audio
} // namespace summit
