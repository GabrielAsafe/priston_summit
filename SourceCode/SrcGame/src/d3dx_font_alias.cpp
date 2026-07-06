#include <windows.h>

extern "C" HRESULT D3DXCreateFontIndirectA(void*, const void*, void**);

extern "C" HRESULT D3DXCreateFontIndirect(
    void* a,
    const void* b,
    void** c
) {
    return D3DXCreateFontIndirectA(a, b, c);
}
