#include <cstdio>
#include <cstddef>

#include "SrcGame/src/smLib3d/smType.h"
#include "SrcGame/src/smLib3d/smTexture.h"

#define SIZEOF_T(T) \
    std::printf("sizeof(%s)=%u\n", #T, (unsigned)sizeof(T))

#define OFFSETOF_T(T, M) \
    std::printf("offsetof(%s,%s)=%u\n", #T, #M, (unsigned)offsetof(T, M))

int main()
{
    std::puts("PT_SMD_MATERIAL_ABI_PROBE_V1");
    std::printf("sizeof(void*)=%u\n", (unsigned)sizeof(void*));

    SIZEOF_T(smFCOLOR);
    SIZEOF_T(smTEXTUREHANDLE);
    SIZEOF_T(smMATERIAL);
    SIZEOF_T(smMATERIAL_GROUP);

    std::puts("-- smMATERIAL_GROUP --");
    OFFSETOF_T(smMATERIAL_GROUP, smMaterial);
    OFFSETOF_T(smMATERIAL_GROUP, MaterialCount);
    OFFSETOF_T(smMATERIAL_GROUP, ReformTexture);
    OFFSETOF_T(smMATERIAL_GROUP, MaxMaterial);
    OFFSETOF_T(smMATERIAL_GROUP, LastSearchMaterial);
    OFFSETOF_T(smMATERIAL_GROUP, szLastSearchName);

    std::puts("-- smMATERIAL --");
    OFFSETOF_T(smMATERIAL, InUse);
    OFFSETOF_T(smMATERIAL, TextureCounter);
    OFFSETOF_T(smMATERIAL, smTexture);
    OFFSETOF_T(smMATERIAL, TextureStageState);
    OFFSETOF_T(smMATERIAL, TextureFormState);
    OFFSETOF_T(smMATERIAL, ReformTexture);
    OFFSETOF_T(smMATERIAL, MapOpacity);
    OFFSETOF_T(smMATERIAL, TextureType);
    OFFSETOF_T(smMATERIAL, BlendType);
    OFFSETOF_T(smMATERIAL, Shade);
    OFFSETOF_T(smMATERIAL, TwoSide);
    OFFSETOF_T(smMATERIAL, SerialNum);
    OFFSETOF_T(smMATERIAL, Diffuse);
    OFFSETOF_T(smMATERIAL, Transparency);
    OFFSETOF_T(smMATERIAL, SelfIllum);
    OFFSETOF_T(smMATERIAL, TextureSwap);
    OFFSETOF_T(smMATERIAL, MatFrame);
    OFFSETOF_T(smMATERIAL, TextureClip);
    OFFSETOF_T(smMATERIAL, UseState);
    OFFSETOF_T(smMATERIAL, MeshState);
    OFFSETOF_T(smMATERIAL, WindMeshBottom);
    OFFSETOF_T(smMATERIAL, smAnimTexture);
    OFFSETOF_T(smMATERIAL, AnimTexCounter);
    OFFSETOF_T(smMATERIAL, FrameMask);
    OFFSETOF_T(smMATERIAL, Shift_FrameSpeed);
    OFFSETOF_T(smMATERIAL, AnimationFrame);

    return 0;
}
