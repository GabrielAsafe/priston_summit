#pragma once
// Runtime guard for verified 95,268-byte MODERN_172 client model caches.
// Validates only the offsets established on the supplied Summit INX assets.
// Unknown cache variants must fail closed and be supported explicitly later.
#include <cctype>
#include <cstdio>
#include <cstring>

namespace summit { namespace inventory { namespace visual_v2 {
namespace detail {
inline char LowerAscii(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}
inline bool EqualAsciiIgnoreCase(const char *a, const char *b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (LowerAscii(*a) != LowerAscii(*b)) return false;
        ++a; ++b;
    }
    return !*a && !*b;
}
inline bool EqualAssetPath(const char *actual, const char *expected) {
    if (!actual || !expected) return false;
    while (*actual && *expected) {
        char a = *actual == '\\' ? '/' : LowerAscii(*actual);
        char b = *expected == '\\' ? '/' : LowerAscii(*expected);
        if (a != b) return false;
        ++actual; ++expected;
    }
    return *actual == 0 && *expected == 0;
}
inline const char *BaseName(const char *value) {
    const char *base = value;
    for (const char *p = value; *p; ++p)
        if (*p == '/' || *p == '\\') base = p + 1;
    return base;
}
inline bool ReadAt(std::FILE *f, long offset, char *dst, std::size_t length) {
    return std::fseek(f, offset, SEEK_SET) == 0 &&
           std::fread(dst, 1, length, f) == length;
}
}
inline bool VerifyNativeInx(const char *inxPath, const char *smdPath, unsigned expectedFamily, const char *meshGroupOverride = nullptr) {
    if (!inxPath || !smdPath || expectedFamily < 1 || expectedFamily > 7) return false;
    const char *base = detail::BaseName(smdPath);
    const std::size_t len = std::strlen(base);
    if (len < 5 || len > 19 || !detail::EqualAsciiIgnoreCase(base + len - 4, ".smd")) return false;
    const std::size_t stemLen = len - 4;
    if (stemLen > 15) return false; // MODERN_172 group name is 16 bytes including NUL.
    // A compiled INX stores the SMD/ASE path and its internal geometry-group
    // independently.  Models such as StylishUniform have group fig_pa_01body,
    // NOT a group named StylishUniform.  The expected group is explicitly
    // provided by the Content binding; existing models default to SMD stem.
    const char *expectedGroup = meshGroupOverride && meshGroupOverride[0]
        ? meshGroupOverride : nullptr;
    char expected[32] = {0};
    if (expectedGroup) {
        const std::size_t groupLen = std::strlen(expectedGroup);
        if (groupLen == 0 || groupLen > 15) return false;
        for (std::size_t i = 0; i < groupLen; ++i) {
            const unsigned char ch = static_cast<unsigned char>(expectedGroup[i]);
            if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                  (ch >= '0' && ch <= '9') || ch == '_')) return false;
        }
        std::memcpy(expected, expectedGroup, groupLen);
    } else {
        std::memcpy(expected, base, stemLen);
    }
    std::FILE *file = std::fopen(inxPath, "rb");
    if (!file) return false;
    bool valid = false;
    do {
        if (std::fseek(file, 0, SEEK_END) != 0 || std::ftell(file) != 95268L) break;
        char modelPath[64] = {0};
        char group[20] = {0};
        char motion[64] = {0};
        if (!detail::ReadAt(file, 0, modelPath, 63) ||
            !detail::ReadAt(file, 192, group, 20) ||
            !detail::ReadAt(file, 88472, motion, 63)) break;
        // Reject unterminated or malformed paths: never read past an INX field.
        if (!std::memchr(modelPath, 0, 63) || !std::memchr(motion, 0, 63)) break;
        if (static_cast<unsigned char>(group[0]) != 1 || group[1] || group[2] || group[3] ||
            !std::memchr(group + 4, 0, 16)) break;
        const char *modelBase = detail::BaseName(modelPath);
        const std::size_t modelLen = std::strlen(modelBase);
        if (modelLen != stemLen + 4 ||
            !detail::EqualAsciiIgnoreCase(modelBase + modelLen - 4, ".ase") ||
            !detail::EqualAsciiIgnoreCase(group + 4, expected)) break;
        char expectedAse[32] = {0};
        // The ASE filename follows the SMD stem, not the geometry-group name.
        std::memcpy(expectedAse, base, stemLen);
        std::memcpy(expectedAse + stemLen, ".ASE", 5);
        if (!detail::EqualAsciiIgnoreCase(modelBase, expectedAse)) break;
        // A same-named mesh in another folder is a different asset.
        const std::size_t smdLength = std::strlen(smdPath);
        if (smdLength > 180) break;
        char expectedFullAse[192] = {0};
        std::memcpy(expectedFullAse, smdPath, smdLength - 4);
        std::memcpy(expectedFullAse + smdLength - 4, ".ase", 5);
        if (!detail::EqualAssetPath(modelPath, expectedFullAse)) break;
        const char *motionBase = detail::BaseName(motion);
        if (detail::LowerAscii(motionBase[0]) != 'm' ||
            motionBase[1] != static_cast<char>('0' + expectedFamily) ||
            !detail::EqualAsciiIgnoreCase(motionBase + 2, "bip.in")) break;
        char expectedMotion[] = "char/tmABCD/M1bip.in";
        expectedMotion[13] = static_cast<char>('0' + expectedFamily);
        if (!detail::EqualAssetPath(motion, expectedMotion)) break;
        valid = true;
    } while (false);
    std::fclose(file);
    return valid;
}
}}} // summit::inventory::visual_v2
