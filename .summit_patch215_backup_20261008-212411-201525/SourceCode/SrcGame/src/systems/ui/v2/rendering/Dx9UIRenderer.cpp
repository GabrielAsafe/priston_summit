#include "systems/ui/v2/rendering/Dx9UIRenderer.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>
#include <d3d9.h>

#include "systems/rendering/2d/legacy/drawsub.h"
#include "systems/rendering/legacy_3d/smDsx.h"

namespace ui::v2
{
namespace
{
struct ColorVertex
{
    float x;
    float y;
    float z;
    float rhw;
    D3DCOLOR color;
};

constexpr DWORD ColorVertexFvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

void ApplyUiPrimitiveState()
{
    if (!d3ddev)
        return;
    d3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
    d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
    d3ddev->SetRenderState(D3DRS_FOGENABLE, FALSE);
    d3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

COLORREF ToColorRef(Color color)
{
    return RGB(color.r, color.g, color.b);
}

ID3DXFont* LegacyFont(bool bold)
{
    return bold ? FontBold : Font;
}

// The original client owns the D3DX font objects and already forwards
// lost/reset-device notifications to them. Keep V2 on those same objects
// instead of introducing another unmanaged D3DX resource lifetime.
ID3DXFont* EnsureLegacyFont(bool bold)
{
    ID3DXFont* font = LegacyFont(bold);
    if (font || !d3ddev)
        return font;

    const int savedOpacity = FontOpacity;
    const D3DCOLOR savedColor = FontColor;

    // DrawFontText lazily creates the requested font when needed. The first
    // invocation after creation does not draw, so prime it off-screen.
    FontOpacity = 0;
    char prime[] = " ";
    DrawFontText(-10000, -10000, prime, RGB(255, 255, 255), bold ? 1 : 0);

    FontOpacity = savedOpacity;
    FontColor = savedColor;
    return LegacyFont(bold);
}

void LogTextFailureOnce(const char* reason)
{
    static bool logged = false;
    if (logged)
        return;
    logged = true;

    FILE* file = std::fopen("ui_v2_text.log", "wt");
    if (!file)
        return;
    std::fprintf(file, "UI V2 text backend failure: %s\n", reason ? reason : "unknown");
    std::fprintf(file, "d3ddev=%p Font=%p FontBold=%p FontOpacity=%d\n",
                 static_cast<void*>(d3ddev), static_cast<void*>(Font),
                 static_cast<void*>(FontBold), FontOpacity);
    std::fclose(file);
}
}

struct Dx9UIRenderer::Impl
{
    IDirect3DStateBlock9* stateBlock = nullptr;
    std::vector<RECT> clips;
    bool active = false;
};

Dx9UIRenderer::Dx9UIRenderer() : impl_(std::make_unique<Impl>()) {}

Dx9UIRenderer::~Dx9UIRenderer()
{
    if (impl_->stateBlock)
        impl_->stateBlock->Release();
}

void Dx9UIRenderer::Begin()
{
    impl_->active = false;
    if (!d3ddev)
        return;
    if (impl_->stateBlock)
    {
        impl_->stateBlock->Release();
        impl_->stateBlock = nullptr;
    }
    if (FAILED(d3ddev->CreateStateBlock(D3DSBT_ALL, &impl_->stateBlock)) || !impl_->stateBlock)
        return;
    impl_->stateBlock->Capture();
    impl_->active = true;

    ApplyUiPrimitiveState();
    d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
}

void Dx9UIRenderer::End()
{
    if (!d3ddev || !impl_->active)
        return;
    impl_->clips.clear();
    d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    if (impl_->stateBlock)
    {
        impl_->stateBlock->Apply();
        impl_->stateBlock->Release();
        impl_->stateBlock = nullptr;
    }
    impl_->active = false;
}

void Dx9UIRenderer::DrawImage(TextureHandle texture, const Rect& destination, const Rect* source)
{
    if (!impl_->active || !texture.IsValid() || destination.IsEmpty())
        return;
    auto* nativeTexture = reinterpret_cast<LPDIRECT3DTEXTURE9>(texture.Value());
    if (!nativeTexture)
        return;

    RECT src{};
    if (source)
    {
        src = {source->x, source->y, source->x + source->width, source->y + source->height};
    }
    else
    {
        D3DSURFACE_DESC desc{};
        if (FAILED(nativeTexture->GetLevelDesc(0, &desc)))
            return;
        src = {0, 0, static_cast<LONG>(desc.Width), static_cast<LONG>(desc.Height)};
    }

    RECT dst{destination.x, destination.y, destination.x + destination.width, destination.y + destination.height};
    DrawTexture2D(nativeTexture, dst, src, TRUE);
}

void Dx9UIRenderer::DrawRect(const Rect& rect, Color color)
{
    if (!impl_->active || !d3ddev || rect.IsEmpty())
        return;

    // DrawTexture2D() is legacy code and leaves Z-write, alpha-test and
    // culling enabled. Reassert the V2 state on every solid primitive so a
    // textured skill/item cannot make later panels, tabs or buttons vanish.
    ApplyUiPrimitiveState();

    const float left = static_cast<float>(rect.x) - 0.5f;
    const float top = static_cast<float>(rect.y) - 0.5f;
    const float right = static_cast<float>(rect.x + rect.width) - 0.5f;
    const float bottom = static_cast<float>(rect.y + rect.height) - 0.5f;
    const D3DCOLOR nativeColor = static_cast<D3DCOLOR>(color.ToARGB());
    const ColorVertex vertices[4] = {
        {left, top, 0.0f, 1.0f, nativeColor},
        {right, top, 0.0f, 1.0f, nativeColor},
        {left, bottom, 0.0f, 1.0f, nativeColor},
        {right, bottom, 0.0f, 1.0f, nativeColor}
    };

    d3ddev->SetTexture(0, nullptr);
    d3ddev->SetFVF(ColorVertexFvf);
    d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(ColorVertex));
}

void Dx9UIRenderer::DrawTextUtf8(FontHandle, std::string_view utf8, const Rect& rect, const TextStyle& style)
{
    if (!impl_->active || !d3ddev || rect.IsEmpty() || utf8.empty())
        return;

    // In v1.0.1 the V2 backend called ID3DXFont::DrawTextA directly. On the
    // real legacy client that path produced no visible glyphs even though the
    // same font object worked through DrawFontText(). Use the proven legacy
    // text entry point at this compatibility boundary and keep alignment local
    // to the V2 renderer.
    ID3DXFont* activeFont = EnsureLegacyFont(style.bold);
    if (!activeFont)
    {
        LogTextFailureOnce("legacy D3DX font could not be created");
        return;
    }

    std::string text(utf8);
    RECT measure{0, 0, std::max(1, rect.width), std::max(1, rect.height)};
    const D3DCOLOR measureColor = D3DCOLOR_ARGB(255, 255, 255, 255);
    activeFont->DrawTextA(nullptr, text.c_str(), static_cast<int>(text.size()), &measure,
                          DT_CALCRECT | DT_SINGLELINE, measureColor);

    const int textWidth = std::max(0, static_cast<int>(measure.right - measure.left));
    const int textHeight = std::max(1, static_cast<int>(measure.bottom - measure.top));

    int x = rect.x;
    if (style.align == TextAlign::Center)
        x = rect.x + std::max(0, (rect.width - textWidth) / 2);
    else if (style.align == TextAlign::Right)
        x = rect.x + std::max(0, rect.width - textWidth);

    const int y = rect.y + std::max(0, (rect.height - textHeight) / 2);

    const int savedOpacity = FontOpacity;
    const D3DCOLOR savedColor = FontColor;
    FontOpacity = style.color.a;

    // The legacy texture helper mutates several device states. Reassert all
    // V2 2D states before glyph rendering; End() restores the game state.
    ApplyUiPrimitiveState();
    const int result = DrawFontText(x, y, text.c_str(), ToColorRef(style.color), style.bold ? 1 : 0);

    FontOpacity = savedOpacity;
    FontColor = savedColor;

    if (result <= 0)
        LogTextFailureOnce("DrawFontText returned no glyph height");
}

void Dx9UIRenderer::PushClip(const Rect& rect)
{
    if (!impl_->active || !d3ddev)
        return;
    Rect clipped = rect;
    if (!impl_->clips.empty())
    {
        const RECT& previous = impl_->clips.back();
        clipped = clipped.Intersect({previous.left, previous.top, previous.right - previous.left, previous.bottom - previous.top});
    }
    RECT native{clipped.x, clipped.y, clipped.x + clipped.width, clipped.y + clipped.height};
    impl_->clips.push_back(native);
    d3ddev->SetScissorRect(&native);
    d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
}

void Dx9UIRenderer::PopClip()
{
    if (!impl_->active || !d3ddev || impl_->clips.empty())
        return;
    impl_->clips.pop_back();
    if (impl_->clips.empty())
    {
        d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        return;
    }
    d3ddev->SetScissorRect(&impl_->clips.back());
}
}
