#pragma once
// SUMMIT 378: data-driven V2 recipe selector, server still authorizes operations.
// The UI never modifies SQLite/DAT or trusts client-supplied item parameters.
#include <algorithm>
#include <array>
#include <functional>
#include <string>
#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/Label.h"
#include "systems/ui/v2/theme/UITheme.h"

namespace ui { namespace v2 {
class ProfessionCraftWindow378 : public Window {
public:
    typedef std::function<bool(const std::string&)> Command;
    struct Row { const char* key; int level; bool physical; };
    explicit ProfessionCraftWindow378(UIId id):Window(id,"Profissoes V2 - progresso") {
        SetBounds({70,55,730,515});SetTitleBarHeight(33);
        SetMovable(true);SetClosable(true);
        auto& close=AddChild<Button>(7801,"X");close.SetBounds({684,4,30,24});
        close.SetOnClick([this](const UIEvent&){if(onClose_)onClose_();});
        auto& prev=AddChild<Button>(7802,"< Anterior");prev.SetBounds({18,422,145,29});
        prev.SetOnClick([this](const UIEvent&){if(page_)--page_;Refresh();});
        auto& next=AddChild<Button>(7803,"Seguinte >");next.SetBounds({174,422,145,29});
        next.SetOnClick([this](const UIEvent&){if((page_+1)*8<Count())++page_;Refresh();});
        auto& xp=AddChild<Button>(7804,"Ver niveis e XP");xp.SetBounds({330,422,175,29});
        xp.SetOnClick([this](const UIEvent&){Dispatch("/professions");});
        auto& inventory=AddChild<Button>(7805,"Estado JSON");inventory.SetBounds({515,422,190,29});
        inventory.SetOnClick([this](const UIEvent&){Dispatch("/craft status");});
        for(int i=0;i<8;++i){
            auto& button=AddChild<Button>(7810+i,"");button.SetBounds({18,53+i*44,690,37});
            button.SetOnClick([this,i](const UIEvent&){
                const size_t index=page_*8+i;
                if(index>=Count())return;
                const Row& recipe=Rows()[index];
                status_->SetText(std::string(recipe.key)+": a consultar XP e niveis no servidor.");
                Dispatch("/professions");
            });buttons_[i]=&button;
        }
        auto& status=AddChild<Label>(7899,"");status.SetBounds({18,465,685,35});
        status.SetStyle(TextStyle{UITheme::TextMuted,false,TextAlign::Left});status_=&status;
        Refresh();
    }
    void SetCommand(Command command){command_=std::move(command);}
    void SetOnClose(std::function<void()> handler){onClose_=std::move(handler);}
    void SetViewport(int w,int h){Rect b=Bounds();b.x=std::max(0,(w-b.width)/2);b.y=std::max(0,(h-b.height)/2);SetBounds(b);}
private:
    // SUMMIT439A: seven profession dashboard, no legacy SQL recipe buttons.
    static const Row* Rows(){static const Row rows[]={
        {"Arcanist",0,true}, {"Blacksmith",0,true},
        {"Lumberjack",0,true}, {"Miner",0,true},
        {"Artisan",0,true}, {"Fisher",0,true},
        {"Hunter",0,true}
    };return rows;}
    static size_t Count(){return 7;}
    void Dispatch(const std::string& text) {
        if(command_ && command_(text))status_->SetText("Pedido enviado. Resultado no chat do jogo.");
        else status_->SetText("Falha no envio: nenhum item foi alterado.");
    }
    void Refresh(){
        for(int i=0;i<8;++i){const size_t idx=page_*8+i;
            if(idx<Count()) {
                const Row& recipe=Rows()[idx];
                buttons_[i]->SetText(std::string(recipe.key)+" | XP / niveis (consultar servidor)");
                buttons_[i]->SetHitTestVisible(true);
            } else {buttons_[i]->SetText("");buttons_[i]->SetHitTestVisible(false);}
        }
        status_->SetText("7 profissoes | CTRL+F9 | XP e niveis no chat, sem fabricar SQL.");
    }
    size_t page_=0;
    std::array<Button*,8> buttons_{};
    Label* status_=nullptr;
    Command command_;
    std::function<void()> onClose_;
};
}}
