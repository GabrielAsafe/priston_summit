#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/domain/IMixGuideGateway.h"

namespace ui::v2
{
class Button;

class MixGuideWindow final : public Window
{
public:
    using CloseHandler = std::function<void()>;

    explicit MixGuideWindow(IMixGuideGateway& gateway, UIId id = InvalidUIId);
    void SetViewport(int width, int height);
    void SetOnClose(CloseHandler handler) { onClose_ = std::move(handler); }
    void Reload();
    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    enum class Category { All, Weapon, Armor, Shield, Orbital };
    struct VisibleRow { std::size_t recipeIndex = 0; Rect rect{}; };

    void BuildChrome();
    void Layout();
    void SetCategory(Category category);
    bool Visible(const MixRecipeSnapshot& recipe) const;
    void BuildVisibleRows();
    int MaxListScroll() const noexcept;
    int MaxDetailScroll() const noexcept;
    const MixRecipeSnapshot* Selected() const noexcept;
    void DrawList(UIRenderContext& context);
    void DrawDetails(UIRenderContext& context);

    IMixGuideGateway& gateway_;
    MixGuideSnapshot snapshot_{};
    Category category_ = Category::All;
    std::size_t selected_ = 0;
    int listScroll_ = 0;
    int detailScroll_ = 0;
    std::vector<VisibleRow> visibleRows_;
    Button* allButton_ = nullptr;
    Button* weaponButton_ = nullptr;
    Button* armorButton_ = nullptr;
    Button* shieldButton_ = nullptr;
    Button* orbitalButton_ = nullptr;
    Button* closeButton_ = nullptr;
    CloseHandler onClose_;
};
}
