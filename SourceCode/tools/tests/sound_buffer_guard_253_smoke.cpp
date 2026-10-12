#include <cassert>
#include "systems/audio/playback/legacy/SummitSoundBufferGuard253.h"
struct FakeBuffer {
    int stop=0, release=0;
    void Stop(){++stop;}
    void Release(){++release;}
};
int main() {
    FakeBuffer old;
    FakeBuffer *slot=&old;
    summit::audio::ReleaseSoundBuffer253(slot);
    assert(slot==nullptr && old.stop==1 && old.release==1);
    summit::audio::ReleaseSoundBuffer253(slot);
    assert(old.stop==1 && old.release==1); // double release prevented
}
