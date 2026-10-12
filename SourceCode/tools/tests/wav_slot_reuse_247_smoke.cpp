#include "SrcGame/src/systems/audio/playback/legacy/SummitWavSlotReuse247.h"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
    summit::audio::WavSlotReuse247<4> cache;
    const summit::audio::WavStamp247 stampA{4096, 100};
    const summit::audio::WavStamp247 stampUpdated{4096, 101};
    const summit::audio::WavStamp247 stampResized{8192, 100};
    assert(!cache.Matches(0, "wav/a.wav", stampA));
    assert(cache.Remember(0, "wav/a.wav", stampA));
    assert(cache.Matches(0, "wav/a.wav", stampA));
    assert(!cache.Matches(0, "wav/b.wav", stampA));
    assert(!cache.Matches(0, "WAV/a.wav", stampA)); // Wine case-sensitive host
    assert(!cache.Matches(0, "wav/a.wav", stampUpdated));
    assert(!cache.Matches(0, "wav/a.wav", stampResized));
    assert(!cache.Matches(1, "wav/a.wav", stampA)); // no buffer sharing
    cache.Forget(0);
    assert(!cache.Matches(0, "wav/a.wav", stampA));
    assert(cache.Remember(1, "wav/b.wav", stampA));
    cache.Clear();
    assert(!cache.Matches(1, "wav/b.wav", stampA));
    assert(!cache.Remember(4, "wav/a.wav", stampA));
    assert(!cache.Remember(0, "", stampA));
    assert(!cache.Remember(0, nullptr, stampA));
    char longName[300]; memset(longName, 'X', sizeof(longName)); longName[299] = 0;
    assert(!cache.Remember(0, longName, stampA));
    assert(!cache.Matches(0, "wav/a.wav", stampA));
    assert(cache.Remember(2, "wav/c.wav", stampA));
    assert(cache.Remember(2, "wav/d.wav", stampA));
    assert(!cache.Matches(2, "wav/c.wav", stampA));
    assert(cache.Matches(2, "wav/d.wav", stampA));
    std::cout << "SUMMIT_AUDIO_247_SMOKE=PASS\n";
}
