#pragma once
// Offline-only native ASE cooker input validation; no Win32 dependencies.
#include <cctype>
#include <cstring>

namespace summit_model_cook_259 {
inline bool SafeRelativeASE(const char* input)
{
    if (!input) return false;
    const size_t n = std::strlen(input);
    // The existing server ChangeFileExt() has a 64-byte static buffer.
    // Keep sufficient room for changing the extension and a terminating NUL.
    if (n < 5 || n > 63) return false;
    size_t component = 0;
    for (size_t i = 0; i < n; ++i) {
        const unsigned char c = static_cast<unsigned char>(input[i]);
        if (c == '/') {
            if (component == 0) return false;
            component = 0;
        } else {
            // No absolute paths, spaces, Win32 devices, traversal, or shell characters.
            if (!(std::isalnum(c) || c == '_' || c == '-' || c == '.')) return false;
            ++component;
        }
    }
    if (!component) return false;
    // No dot or dot-dot path components.
    const char* current = input;
    while (*current) {
        const char* end = std::strchr(current, '/');
        const size_t len = end ? static_cast<size_t>(end-current) : std::strlen(current);
        if ((len == 1 && current[0] == '.') ||
            (len == 2 && current[0] == '.' && current[1] == '.')) return false;
        current = end ? end+1 : current+len;
    }
    const char* last = input + n - 4;
    return last[0] == '.' &&
        (last[1] == 'a' || last[1] == 'A') &&
        (last[2] == 's' || last[2] == 'S') &&
        (last[3] == 'e' || last[3] == 'E');
}
}
