#include <cstdio>
#include <cstddef>

#include "SrcGame/src/smLib3d/smType.h"
#include "SrcGame/src/smLib3d/smObj3d.h"

#define SIZEOF_T(T) \
    std::printf("sizeof(%s)=%u\n", #T, (unsigned)sizeof(T))

#define OFFSETOF_T(T, M) \
    std::printf("offsetof(%s,%s)=%u\n", #T, #M, (unsigned)offsetof(T, M))

int main()
{
    std::puts("PT_SMD_ABI_PROBE_V2");
    std::printf("sizeof(void*)=%u\n", (unsigned)sizeof(void*));

    SIZEOF_T(smFRAME_POS);
    SIZEOF_T(smDFILE_HEADER);
    SIZEOF_T(smDFILE_OBJINFO);

    SIZEOF_T(smMATRIX);
    SIZEOF_T(smFMATRIX);
    SIZEOF_T(smTEXLINK);
    SIZEOF_T(smVERTEX);
    SIZEOF_T(smFACE);
    SIZEOF_T(smTM_ROT);
    SIZEOF_T(smTM_POS);
    SIZEOF_T(smTM_SCALE);
    SIZEOF_T(smOBJ3D);

    std::puts("-- smDFILE_HEADER --");
    OFFSETOF_T(smDFILE_HEADER, szHeader);
    OFFSETOF_T(smDFILE_HEADER, ObjCounter);
    OFFSETOF_T(smDFILE_HEADER, MatCounter);
    OFFSETOF_T(smDFILE_HEADER, MatFilePoint);
    OFFSETOF_T(smDFILE_HEADER, First_ObjInfoPoint);
    OFFSETOF_T(smDFILE_HEADER, TmFrameCounter);
    OFFSETOF_T(smDFILE_HEADER, TmFrame);

    std::puts("-- smDFILE_OBJINFO --");
    OFFSETOF_T(smDFILE_OBJINFO, szNodeName);
    OFFSETOF_T(smDFILE_OBJINFO, Length);
    OFFSETOF_T(smDFILE_OBJINFO, ObjFilePoint);

    std::puts("-- smTEXLINK --");
    OFFSETOF_T(smTEXLINK, u);
    OFFSETOF_T(smTEXLINK, v);
    OFFSETOF_T(smTEXLINK, hTexture);
    OFFSETOF_T(smTEXLINK, NextTex);

    std::puts("-- smVERTEX --");
    OFFSETOF_T(smVERTEX, x);
    OFFSETOF_T(smVERTEX, nx);

    std::puts("-- smFACE --");
    OFFSETOF_T(smFACE, v);
    OFFSETOF_T(smFACE, t);
    OFFSETOF_T(smFACE, lpTexLink);

    std::puts("-- animation structs --");
    OFFSETOF_T(smTM_ROT, frame);
    OFFSETOF_T(smTM_ROT, x);
    OFFSETOF_T(smTM_POS, frame);
    OFFSETOF_T(smTM_POS, x);
    OFFSETOF_T(smTM_SCALE, frame);
    OFFSETOF_T(smTM_SCALE, x);

    std::puts("-- smOBJ3D --");
    OFFSETOF_T(smOBJ3D, Vertex);
    OFFSETOF_T(smOBJ3D, Face);
    OFFSETOF_T(smOBJ3D, TexLink);
    OFFSETOF_T(smOBJ3D, Physique);
    OFFSETOF_T(smOBJ3D, ZeroVertex);
    OFFSETOF_T(smOBJ3D, maxZ);
    OFFSETOF_T(smOBJ3D, MaxVertex);
    OFFSETOF_T(smOBJ3D, nVertex);
    OFFSETOF_T(smOBJ3D, nFace);
    OFFSETOF_T(smOBJ3D, nTexLink);
    OFFSETOF_T(smOBJ3D, ColorEffect);
    OFFSETOF_T(smOBJ3D, ClipStates);
    OFFSETOF_T(smOBJ3D, Posi);
    OFFSETOF_T(smOBJ3D, NodeName);
    OFFSETOF_T(smOBJ3D, NodeParent);
    OFFSETOF_T(smOBJ3D, pParent);
    OFFSETOF_T(smOBJ3D, Tm);
    OFFSETOF_T(smOBJ3D, TmInvert);
    OFFSETOF_T(smOBJ3D, TmResult);
    OFFSETOF_T(smOBJ3D, TmRotate);
    OFFSETOF_T(smOBJ3D, mWorld);
    OFFSETOF_T(smOBJ3D, mLocal);
    OFFSETOF_T(smOBJ3D, lFrame);
    OFFSETOF_T(smOBJ3D, qx);
    OFFSETOF_T(smOBJ3D, sx);
    OFFSETOF_T(smOBJ3D, px);
    OFFSETOF_T(smOBJ3D, TmRot);
    OFFSETOF_T(smOBJ3D, TmPos);
    OFFSETOF_T(smOBJ3D, TmScale);
    OFFSETOF_T(smOBJ3D, TmPrevRot);
    OFFSETOF_T(smOBJ3D, TmRotCnt);
    OFFSETOF_T(smOBJ3D, TmPosCnt);
    OFFSETOF_T(smOBJ3D, TmScaleCnt);
    OFFSETOF_T(smOBJ3D, TmRotFrame);
    OFFSETOF_T(smOBJ3D, TmPosFrame);
    OFFSETOF_T(smOBJ3D, TmScaleFrame);
    OFFSETOF_T(smOBJ3D, TmFrameCnt);

    return 0;
}
