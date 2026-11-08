#pragma once
// SUMMIT 387A: one-click diagnostic commands, sent through the same authenticated
// server chat-command channel as the legacy input. No client-side rewards/items.
#include <algorithm>
#include <array>
#include <functional>
#include <string>
#include <utility>
#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/Label.h"
#include "systems/ui/v2/theme/UITheme.h"

namespace ui { namespace v2 {
class ProfessionTestPanel387 final : public Window {
public:
    using Command = std::function<std::string(const std::string&)>; // SUMMIT_390A_DIAG_API
    struct Action { const char* label; const char* command; };

    explicit ProfessionTestPanel387(UIId id) : Window(id, "SUMMIT - Testes de Profissoes") {
        SetBounds({90, 65, 770, 560});
        SetTitleBarHeight(32);
        SetMovable(true);
        SetClosable(true);
        auto& close = AddChild<Button>(38701, "X  Fechar");
        close.SetBounds({635, 4, 116, 25});
        close.SetOnClick([this](const UIEvent&) { if (onClose_) onClose_(); });

        auto& help = AddChild<Label>(38702, "Comando encaminhado nao e recibo de craft | ESC fecha a janela");
        help.SetBounds({18, 37, 730, 24});
        help.SetStyle(TextStyle{UITheme::TextMuted, false, TextAlign::Left});

        for (std::size_t i = 0; i < ActionCount(); ++i) {
            const Action& action = Actions()[i];
            auto& b = AddChild<Button>(static_cast<UIId>(38710 + i), action.label);
            const int column = static_cast<int>(i % 2);
            const int line = static_cast<int>(i / 2);
            b.SetBounds({18 + column * 368, 72 + line * 43, 355, 38}); // SUMMIT_390A_17_BUTTONS
            b.SetOnClick([this, i](const UIEvent&) {
                Dispatch(Actions()[i].command);
            });
        }
        auto& status = AddChild<Label>(38799, "Pronto. Envia os comandos pelo servidor; nao altera itens localmente.");
        status.SetBounds({18, 475, 731, 65});
        status.SetStyle(TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        status_ = &status;
    }

    void SetCommand(Command c) { command_ = std::move(c); }
    void SetOnClose(std::function<void()> c) { onClose_ = std::move(c); }
    void SetViewport(int w, int h) {
        Rect b = Bounds();
        b.x = std::max(0, (w - b.width) / 2);
        b.y = std::max(0, (h - b.height) / 2);
        SetBounds(b);
    }

private:
    static const Action* Actions() {
        static const Action actions[] = {
            {"Craft: estado SQLite", "/craft status"},
            {"Craft: receitas Artisan", "/craft list artisan."},
            {"Recursos (SQLite)", "/resources"},
            {"Niveis e XP", "/professions"},
            {"Miner: listar nodes", "/mine list"},
            {"Lumberjack: listar nodes", "/gather list"},
            {"Miner: recolher proximo", "/mine"},
            {"Lumberjack: recolher proximo", "/gather"},
            {"Fisher: listar nodes", "/fish list"},
            {"Fisher: cast", "/fish cast"},
            {"Fisher: hook", "/fish hook"},
            {"Fisher: reel", "/fish reel"},
            {"Fisher: collect", "/fish collect"},
            {"Artisan: nucleo cristalino", "/craft make artisan.refine_core_crystalline"},
            {"Artisan: fazer tabuas (2 troncos)", "/craft make artisan.make_planks"}, // SUMMIT_390A_PLANKS
            {"Receitas Blacksmith", "/craft list blacksmith."},
            {"Receitas Arcanist", "/craft list arcanist."},
        };
        return actions;
    }
    static constexpr std::size_t ActionCount() { return 17; } // SUMMIT_390A_17_ACTIONS

    void Dispatch(const std::string& command) {
        if (!status_) return;
        // SUMMIT_390A_PANEL_RESULT: caller differentiates login, socket, throttle and handoff.
        status_->SetText(command_ ? command_(command) : "NAO ENVIADO: rota de comandos indisponivel.");
    }

    Command command_;
    std::function<void()> onClose_;
    Label* status_ = nullptr;
};
}} // namespace ui::v2
