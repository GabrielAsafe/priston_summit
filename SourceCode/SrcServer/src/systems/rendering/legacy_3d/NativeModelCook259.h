#pragma once
// Explicitly invoked offline converter. Never reached in normal server startup.
// Uses the engine's own ASE -> SMD/SMB serialization and verifies reloading.
#include "systems/rendering/legacy_3d/ModelCookPath259.h"
#include <cstdio>
#include <cstring>

// Legacy readers are implemented in smRead3d_legacy_server.inl but not exported in its header.
smPAT3D* smReadModel(char* file, char* szModelName);
smPAT3D* smReadModel_Bip(char* file);
void smASE_EnableOfflineBoneHierarchy273(bool enabled);
void smASE_EnableOfflineEmptyMaterials274(bool enabled);
// Only queried on an explicitly requested offline conversion failure.
#ifdef SUMMIT_NATIVE_ASE_REASON_265
const char* smASEFailureReason265();
#endif

namespace summit_model_cook_259 {
inline int RunOne(int count, char** args)
{
    if (count != 4 || !args[2] || !args[3]) {
        std::fprintf(stderr, "MODEL_COOK_259=FAIL arguments\n");
        return 2;
    }
    const bool mesh = std::strcmp(args[2], "mesh") == 0;
    const bool bone = std::strcmp(args[2], "bone") == 0;
    if ((!mesh && !bone) || !SafeRelativeASE(args[3])) {
        std::fprintf(stderr, "MODEL_COOK_259=FAIL invalid_role_or_path\n");
        return 2;
    }
    const DWORD attrs = GetFileAttributesA(args[3]);
    if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) {
        std::fprintf(stderr, "MODEL_COOK_259=FAIL invalid_source\n");
        return 2;
    }
    // The Python orchestrator gives each conversion an empty scratch directory.
    // Refuse to overwrite *any* existing cache, even with a recent timestamp.
    char cache[64] = {0};
    std::strcpy(cache, args[3]);
    char* dot = std::strrchr(cache, '.');
    if (!dot) return 2;
    std::strcpy(dot, mesh ? ".smd" : ".smb");
    if (GetFileAttributesA(cache) != INVALID_FILE_ATTRIBUTES) {
        std::fprintf(stderr, "MODEL_COOK_259=FAIL cache_exists\n");
        return 2;
    }
    initSinCos();
    // Model serialization stores texture names, not uploaded D3D resources.
    g_IsReadTextures = 0;
    // PATCH 262: the normal server startup calls InitMaterial() *before*
    // InitMonster()/smASE_Read(). The offline entry point skips that startup,
    // leaving smMaterial and the shared material group NULL. Bone ASE reading
    // directly uses smMaterial; geometry imports may also need this table.
    // Initialize only the CPU-side material registry; do not initialize D3D.
    struct OfflineMaterialScope {
        bool ready;
        OfflineMaterialScope() : ready(InitMaterial() != FALSE && smMaterialGroup && smMaterial) {}
        ~OfflineMaterialScope() {
            smASE_SetPhysique(nullptr);
            if (smMaterialGroup) CloseMaterial();
            smMaterial = nullptr;
        }
    } materials;
    if (!materials.ready) {
        std::fprintf(stderr, "MODEL_COOK_259=FAIL material_init\n");
        return 3;
    }
    // Only the explicit offline command enables the bounded bone-link guard.
    // The normal server/NPC startup is unchanged.
    struct OfflineBoneScope273 {
        OfflineBoneScope273() { smASE_EnableOfflineBoneHierarchy273(true); }
        ~OfflineBoneScope273() { smASE_EnableOfflineBoneHierarchy273(false); }
    } boneGuard;
    // PATCH 274: only the explicit offline converter may normalize
    // non-zero face slot IDs under an explicit zero-count MATERIAL_LIST.
    struct OfflineEmptyMaterialsScope274 {
        OfflineEmptyMaterialsScope274() { smASE_EnableOfflineEmptyMaterials274(true); }
        ~OfflineEmptyMaterialsScope274() { smASE_EnableOfflineEmptyMaterials274(false); }
    } emptyMaterialsGuard;
    smASE_SetPhysique(nullptr);
    smPAT3D* created = mesh ? smASE_Read(args[3]) : smASE_ReadBone(args[3]);
    if (!created) {
#ifdef SUMMIT_NATIVE_ASE_REASON_265
        std::fprintf(stderr, "MODEL_COOK_259=FAIL ase_parse reason=%s\n", smASEFailureReason265());
#else
        std::fprintf(stderr, "MODEL_COOK_259=FAIL ase_parse\n");
#endif
        return 3;
    }
    smASE_SetPhysique(nullptr);
    delete created;
    const DWORD outputAttrs = GetFileAttributesA(cache);
    if (outputAttrs == INVALID_FILE_ATTRIBUTES || (outputAttrs & FILE_ATTRIBUTE_DIRECTORY)) {
        std::fprintf(stderr, "MODEL_COOK_259=FAIL cache_not_created\n");
        return 3;
    }
    // An independent read-back of the native engine format is mandatory.
    smPAT3D* verified = mesh ? smReadModel(cache, nullptr) : smReadModel_Bip(cache);
    if (!verified) {
        std::fprintf(stderr, "MODEL_COOK_259=FAIL native_readback\n");
        return 4;
    }
    delete verified;
    std::puts("MODEL_COOK_259=PASS");
    return 0;
}
}
