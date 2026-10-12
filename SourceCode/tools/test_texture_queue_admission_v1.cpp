#include "rendering/assets/v3/TextureQueueAdmissionPolicy.h"
#include <cassert>
#include <cstdio>

using namespace summit::rendering::assets::v3;
int main()
{
    TextureQueuePressure p = {0u, 0u, 0u};
    assert(TextureQueueCanAdmit(p, 6u, 2) == TEXTURE_QUEUE_ACCEPT);
    p = {2u, 12u, 14u};
    // Previously this was rejected (global in-flight >= 6), even when the
    // CPU workers were free. HIGH must still pass; LOW must wait for GPU.
    assert(TextureQueueCanAdmit(p, 6u, 0) == TEXTURE_QUEUE_ACCEPT);
    assert(TextureQueueCanAdmit(p, 6u, 2) == TEXTURE_QUEUE_GPU_READY_FULL);
    p = {6u, 3u, 9u};
    assert(TextureQueueCanAdmit(p, 6u, 1) == TEXTURE_QUEUE_CPU_FULL);
    assert(TextureQueueCanAdmit(p, 6u, 0) == TEXTURE_QUEUE_ACCEPT);
    p = {8u, 0u, 8u};
    assert(TextureQueueCanAdmit(p, 6u, 0) == TEXTURE_QUEUE_CPU_FULL);
    p = {1u, 14u, 15u};
    assert(TextureQueueCanAdmit(p, 6u, 0) == TEXTURE_QUEUE_GPU_READY_FULL);
    p = {1u, 5u, 22u};
    assert(TextureQueueCanAdmit(p, 6u, 0) == TEXTURE_QUEUE_TOTAL_FULL);
    p = {0u, 0u, 2u};
    assert(TextureQueueCanAdmit(p, 1u, 2) == TEXTURE_QUEUE_ACCEPT);
    p = {1u, 0u, 1u};
    assert(TextureQueueCanAdmit(p, 1u, 2) == TEXTURE_QUEUE_CPU_FULL);
    assert(TextureQueueCanAdmit(p, 1u, 0) == TEXTURE_QUEUE_ACCEPT);
    std::puts("SUMMIT_TEXTURE_QUEUE_ADMISSION=PASS");
}
