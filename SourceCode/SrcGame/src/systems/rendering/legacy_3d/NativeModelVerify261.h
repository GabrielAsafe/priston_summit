#pragma once
// Offline-only client cache verifier; called before normal WinMain initialization.
#include "rendering/legacy_3d/OfflineModelPath261.h"
#include <cstdio>
#include <cstring>

smPAT3D* smReadModel(char* path, char* nodeName);
smPAT3D* smReadModel_Bip(char* path);

namespace summit_offline_model_261 {
inline int RunFromCommandLine(const char* args)
{
    char verb[40] = {};
    char role[16] = {};
    char filename[256] = {};
    char extra[2] = {};
    const int fields = args ? std::sscanf(args, "%39s %15s %255s %1s", verb, role, filename, extra) : 0;
    if (fields != 3 || std::strcmp(verb, "--summit-verify-model") != 0) {
        std::fprintf(stderr, "MODEL_VERIFY_261=FAIL arguments\n");
        return 2;
    }
    const bool mesh = std::strcmp(role, "mesh") == 0;
    const bool bone = std::strcmp(role, "bone") == 0;
    if ((!mesh && !bone) || !SafeCachePath(filename, mesh)) {
        std::fprintf(stderr, "MODEL_VERIFY_261=FAIL invalid_role_or_path\n");
        return 2;
    }
    const DWORD attrs = GetFileAttributesA(filename);
    if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) {
        std::fprintf(stderr, "MODEL_VERIFY_261=FAIL invalid_cache_file\n");
        return 2;
    }
    initSinCos();
    g_IsReadTextures = 0; // Do not access D3D9 or load textures when checking binary structures.
    smASE_SetPhysique(nullptr);
    smPAT3D* pattern = mesh ? smReadModel(filename, nullptr) : smReadModel_Bip(filename);
    if (!pattern) {
        std::fprintf(stderr, "MODEL_VERIFY_261=FAIL client_native_readback\n");
        return 3;
    }
    delete pattern;
    std::puts("MODEL_VERIFY_261=PASS");
    // GUI-subsystem executables may have no redirected stdout under Wine.
    // Use a distinctive process status to confirm this exact code path ran.
    return 42;
}
}
