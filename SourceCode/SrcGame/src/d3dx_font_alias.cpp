#include <windows.h>
#include <d3dx9.h>

/*
 * A import library MinGW fornece:
 *     _D3DXCreateFontIndirectA@12
 *
 * Os headers antigos do projeto declaram e os objetos requerem:
 *     _D3DXCreateFontIndirect@12
 */

extern "C" HRESULT WINAPI D3DXCreateFontIndirectA(
    LPDIRECT3DDEVICE9 pDevice,
    const LOGFONTA* pLogFont,
    LPD3DXFONT* ppFont
);

extern "C" HRESULT WINAPI D3DXCreateFontIndirect(
    LPDIRECT3DDEVICE9 pDevice,
    const LOGFONT* pLogFont,
    LPD3DXFONT* ppFont
)
{
    return D3DXCreateFontIndirectA(
        pDevice,
        reinterpret_cast<const LOGFONTA*>(pLogFont),
        ppFont
    );
}
