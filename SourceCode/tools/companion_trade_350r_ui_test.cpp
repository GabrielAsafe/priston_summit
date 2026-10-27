#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include <cassert>
#include <string>

namespace ui { namespace v2 {
struct MockRenderer final: IUIRenderer {
    int images=0, text=0, rects=0;
    TextureHandle last{};
    Rect target{};
    void Begin() override {} void End() override {}
    void DrawImage(TextureHandle t,const Rect& r,const Rect*) override
    { ++images;last=t;target=r; }
    void DrawRect(const Rect&,Color) override {++rects;}
    void DrawTextUtf8(FontHandle,std::string_view,const Rect&,const TextStyle&) override {++text;}
    void PushClip(const Rect&) override {} void PopClip() override {}
};
}}
int main(){
    using namespace ui::v2;
    MockRenderer renderer; UIRenderContext context(renderer);
    Button slot(1,"Item 00FFFF #4");slot.SetBounds({10,20,58,82});
    slot.SetIcon(TextureHandle{0x1234});
    slot.Render(context);
    assert(renderer.images==1 && renderer.text==0);
    assert(renderer.last.Value()==0x1234 && renderer.target.width==50);
    assert(renderer.target.x==14 && renderer.target.y==36);
    slot.SetEnabled(false);
    slot.Render(context);
    assert(renderer.images==2); // Passive/equipped items remain visible.
    slot.SetIcon({});slot.SetText("...");slot.Render(context);
    assert(renderer.images==2 && renderer.text==1);
}
