#include "systems/ui/v2/pages/CompanionsPage.h"
#include "gameplay/companions/v2/CompanionDiagnostic315.h"
#include "gameplay/companions/v2/CompanionTradeClientFreeze.h"
#include "gameplay/companions/v2/CompanionTradePreviewRefresh351I.h"
#include "gameplay/companions/v2/CompanionTradePreviewSelection352A.h"

#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <unordered_set>
#include <string>

#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/TextLayout.h"
#ifdef _WIN32
#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/rendering/legacy_3d/smTexture.h"
#endif

namespace ui::v2
{
namespace
{
std::string ResultText(int result)
{
    switch (result)
    {
        case 0: return {};
        case 1: return "Operacao concluida.";
        case -2: return "Esse personagem nao pode ser usado agora.";
        case -3: return "Falha ao ler/gravar o DAT do personagem.";
        case -4: return "Esse personagem ja esta online.";
        case -5: return "Esse companion ja esta invocado.";
        case -6: return "Limite de companions ativos atingido.";
        case -7: return "Falha ao criar a entidade do companion.";
        case -8: return "Nenhum personagem elegivel encontrado.";
        case -9: return "Nao ha espaco para guardar o loadout desta sessao.";
        case -10: return "Acao invalida para esta versao da UI.";
        case -11: return "Falha ao persistir o perfil do companion.";
        case -12: return "Dispensa primeiro o Companion para modificar o DAT.";
        case -13: return "Equipamento rejeitado. Consulta o estado do Companion.";
        case -14: return "Selecao de equipamento invalida; nenhum item foi alterado.";
        case -15: return "DAT inconsistente ou inacessivel. Nada foi gravado.";
        case -16: return "Equipamento incompativel segundo as regras V2.";
        case -17: return "Dimensoes do item nao encontradas no catalogo V2.";
        case -18: return "Sem espaco livre na Mochila A para guardar a peca.";
        case -19: return "Itens sobrepostos ou fora da grelha na Mochila A.";
        case -20: return "Falha na gravacao/backup do DAT. Alteracao cancelada.";
        case -21: return "Movimentacao de mochila rejeitada; nada foi alterado.";
        case -100: return "Sem ligacao ao servidor. Comando nao enviado.";
        case -101: return "Falha no envio do comando ao servidor.";
        case -102: return "Resposta de estado com versao invalida.";
        case -103: return "Resposta de mochila invalida.";
        case -104: return "Pedido de Companion invalido no servidor.";
        default: return "Resultado do servidor: " + std::to_string(result);
    }
}

const char* CompanionSlotLabel(int position)
{
    switch(position)
    {
        case 0: return "Mochila";
        case 1: return "Mao direita";
        case 2: return "Mao esquerda";
        case 3: return "Armadura";
        case 4: return "Amuleto";
        case 5: return "Anel direito";
        case 6: return "Anel esquerdo";
        case 7: return "Sheltom";
        case 8: return "Bracelete";
        case 9: return "Luvas";
        case 10: return "Botas";
        case 11: case 12: case 13: return "Pocao";
        case 14: return "Traje";
        case 15: case 16: return "Brinco";
        default: return "Outro";
    }
}

// Coordinates and slot IDs are the same as InventoryPage (player inventory).
// Unlike PATCH 309, never draw item names inside or beneath the slots.
struct CompanionGearRect { int slot, x, y, w, h; const char* title; };
static const CompanionGearRect CompanionGearSlots[16] = {
    {1, 40, 122, 58, 98, "Mao D"}, {2, 194, 122, 58, 98, "Mao E"},
    {3, 117, 122, 66, 98, "Armadura"}, {4, 40, 244, 42, 42, "Amuleto"},
    {5, 88, 244, 42, 42, "Anel D"}, {6, 136, 244, 42, 42, "Anel E"},
    {7, 184, 244, 42, 42, "Sheltom"}, {8, 40, 296, 54, 54, "Bracelete"},
    {9, 102, 296, 54, 54, "Luvas"}, {10, 164, 296, 54, 54, "Botas"},
    {11, 40, 370, 42, 42, "P1"}, {12, 88, 370, 42, 42, "P2"},
    {13, 136, 370, 42, 42, "P3"}, {14, 226, 296, 66, 98, "Traje"},
    {15, 226, 244, 42, 42, "Brinco E"}, {16, 274, 244, 42, 42, "Brinco D"}
};
static constexpr int CompanionBagX = 346;
static constexpr int CompanionBagY = 92;
static constexpr int CompanionBagCell = 30;
static constexpr int CompanionBagColumns = 12;
static constexpr int CompanionBagRows = 6;
static constexpr int CompanionBagCapacity = 72;

void CompanionBorder(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x,rect.y,rect.width,1},color);
    r.DrawRect({rect.x,rect.y+rect.height-1,rect.width,1},color);
    r.DrawRect({rect.x,rect.y,1,rect.height},color);
    r.DrawRect({rect.x+rect.width-1,rect.y,1,rect.height},color);
}

// Reuse the game's catalog and texture ownership (same as sinInvenTory).
// Catalog lookups are memoized, but *not* texture pointers: legacy item loading
// may refresh an entry. Draw functions must NEVER load BMPs or create D3D resources.
int CompanionCatalogIndex(std::uint32_t code)
{
#ifdef _WIN32
    if (!code) return -1;
    struct Entry { std::uint32_t code=0; int index=-1; };
    static std::array<Entry,512> entries{};
    static std::size_t next=0;
    for(const auto& entry:entries)
        if(entry.code==code &&
           (entry.index<0 || (entry.index<MAX_ITEM && sItem[entry.index].CODE==code)))
            return entry.index;
    int index=-1;
    for(int i=0;i<MAX_ITEM;++i)
        if(sItem[i].CODE==code) { index=i; break; }
    entries[next]={code,index}; next=(next+1)%entries.size();
    return index;
#else
    (void)code; return -1;
#endif
}

TextureHandle CompanionItemIcon(std::uint32_t code, int& cellsWide, int& cellsHigh)
{
    cellsWide=1; cellsHigh=1;
#ifdef _WIN32
    const int index=CompanionCatalogIndex(code);
    if(index<0) return {};
    cellsWide=std::max(1,(sItem[index].w+21)/22);
    cellsHigh=std::max(1,(sItem[index].h+21)/22);
    return TextureHandle(reinterpret_cast<std::uintptr_t>(sItem[index].lpTempItem));
#else
    (void)code; return {};
#endif
}

// Called from Update, never OnRender. One missing icon per ~100ms while a
// Companion inventory is open limits render-thread stalls from legacy BMP IO.
bool CompanionWarmIcon(std::uint32_t code)
{
#ifdef _WIN32
    static std::unordered_set<std::uint32_t> attempted;
    if(!code || attempted.count(code)) return false;
    const int index=CompanionCatalogIndex(code);
    if(index>=0 && sItem[index].lpTempItem) return false;
    attempted.insert(code);
    if(index<0) return true; // Don't hammer disk every frame for missing assets.
    if(!sItem[index].ItemFilePath[0] || !sItem[index].LastCategory[0]) return true;
    char path[256]{};
    const int n=std::snprintf(path,sizeof(path),
        "image//Sinimage//Items//%s//it%s.bmp",
        sItem[index].ItemFilePath,sItem[index].LastCategory);
    if(n>0 && n<static_cast<int>(sizeof(path)))
        sItem[index].lpTempItem=LoadDibSurfaceOffscreen(path);
    return true;
#else
    (void)code; return false;
#endif
}

const char* TacticName(CompanionUiTactic tactic)
{
    switch (tactic)
    {
        case CompanionUiTactic::OwnerFocus: return "Foco no dono";
        case CompanionUiTactic::Defensive: return "Defensivo";
        case CompanionUiTactic::HoldPosition: return "Manter posicao";
        case CompanionUiTactic::Balanced:
        default: return "Balanceado";
    }
}

const char* CustomConditionName(int condition)
{
    switch(condition)
    {
        case 1: return "Self HP";
        case 2: return "Owner HP";
        case 3: return "Enemies";
        case 4: return "Owner target";
        case 5: return "Party threat";
        case 0:
        default: return "Always";
    }
}

const char* CustomActionName(int action)
{
    switch(action)
    {
        case 0: return "Owner target";
        case 1: return "Defend";
        case 2: return "Nearest";
        case 3: return "Retreat";
        case 4: return "Hold";
        default: return "Nearest";
    }
}

std::string ItemDisplay(const std::string& name, std::uint32_t code)
{
    if (!code)
        return "Nenhum";
    if (!name.empty())
        return name;
    char buffer[32] = {};
    std::snprintf(buffer, sizeof(buffer), "0x%08X", static_cast<unsigned>(code));
    return buffer;
}

std::string SkillDisplay(const CompanionUiSkillSlot& skill)
{
    if (!skill.useSlot || !skill.skillCode)
        return "Desligado";
    std::string text = skill.name.empty() ? ("Skill " + std::to_string(skill.useSlot)) : skill.name;
    if (skill.point > 0)
        text += " L" + std::to_string(skill.point);
    return text;
}
}

static Rect CompanionErrorBox(const Rect& root)
{
    return {root.x+20, root.y+root.height-47, root.width-40, 42};
}

static void RenderCompanionErrorBox(IUIRenderer& renderer, const Rect& root)
{
    const auto& history=summit::companion::diagnostic::Errors();
    if(!history.count) return;
    const Rect box=CompanionErrorBox(root);
    renderer.DrawRect(box,Color{77,20,27,255});
    renderer.DrawTextUtf8(FontHandle{},"ERRO COMPANION ("+std::to_string(history.count)+") - clicar para reconhecer",
        {box.x+9,box.y+2,box.width-18,18},
        TextStyle{UITheme::Text,true,TextAlign::Left});
    const auto& entry=history.entries[0];
    const std::string line="#"+std::to_string(entry.result)+" "+ResultText(entry.result)+
        (entry.repeats>1 ? " (x"+std::to_string(entry.repeats)+")" : "");
    renderer.DrawTextUtf8(FontHandle{},line,{box.x+9,box.y+20,box.width-18,18},
        TextStyle{UITheme::Text,false,TextAlign::Left});
}

CompanionsPage::CompanionsPage(ICompanionGateway& gateway, UIId id)
    : Container(id), gateway_(gateway)
{
    BuildControls();
    ForceRefresh();
}

void CompanionsPage::BuildControls()
{
    for (std::size_t i = 0; i < characterButtons_.size(); ++i)
    {
        auto& button = AddChild<Button>(600 + static_cast<UIId>(i), "-");
        button.SetOnClick([this, i](const UIEvent&) { SelectVisibleRow(i); });
        characterButtons_[i] = &button;
    }

    auto& summonDismiss = AddChild<Button>(620, "Invocar");
    summonDismiss.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(selected->active ? CompanionUiAction::Dismiss : CompanionUiAction::Summon);
    });
    summonDismissButton_ = &summonDismiss;

    auto& dismissAll = AddChild<Button>(621, "Dispensar todos");
    dismissAll.SetOnClick([this](const UIEvent&) { gateway_.Execute(CompanionUiAction::DismissAll); });
    dismissAllButton_ = &dismissAll;

    auto& refresh = AddChild<Button>(622, "Atualizar");
    refresh.SetOnClick([this](const UIEvent&) { ForceRefresh(); });
    refreshButton_ = &refresh;

    auto& aggressive = AddChild<Button>(630, "Agressivo");
    aggressive.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::SetMode, 0); });
    aggressiveButton_ = &aggressive;
    auto& follow = AddChild<Button>(631, "Seguir");
    follow.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::SetMode, 1); });
    followButton_ = &follow;
    auto& passive = AddChild<Button>(632, "Passivo");
    passive.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::SetMode, 2); });
    passiveButton_ = &passive;

    auto& tacticPrev = AddChild<Button>(633, "<");
    tacticPrev.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        int value = static_cast<int>(selected->tactic) - 1;
        if (value < 0) value = 3;
        ExecuteForSelected(CompanionUiAction::SetTactic, value);
    });
    tacticPrevButton_ = &tacticPrev;
    auto& tacticNext = AddChild<Button>(634, ">");
    tacticNext.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        int value = (static_cast<int>(selected->tactic) + 1) % 4;
        ExecuteForSelected(CompanionUiAction::SetTactic, value);
    });
    tacticNextButton_ = &tacticNext;

    auto& retreatPrev = AddChild<Button>(635, "-");
    retreatPrev.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(CompanionUiAction::SetRetreatHp, std::max(0, selected->retreatHpPercent - 5));
    });
    retreatPrevButton_ = &retreatPrev;
    auto& retreatNext = AddChild<Button>(636, "+");
    retreatNext.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(CompanionUiAction::SetRetreatHp, std::min(90, selected->retreatHpPercent + 5));
    });
    retreatNextButton_ = &retreatNext;

    auto& rangePrev = AddChild<Button>(637, "-");
    rangePrev.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(CompanionUiAction::SetPreferredRange, std::max(0, selected->preferredRange - 25));
    });
    rangePrevButton_ = &rangePrev;
    auto& rangeNext = AddChild<Button>(638, "+");
    rangeNext.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(CompanionUiAction::SetPreferredRange, std::min(600, selected->preferredRange + 25));
    });
    rangeNextButton_ = &rangeNext;

    auto& rightPrev = AddChild<Button>(640, "<");
    rightPrev.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::EquipRightPrev); });
    rightPrevButton_ = &rightPrev;
    auto& rightNext = AddChild<Button>(641, ">");
    rightNext.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::EquipRightNext); });
    rightNextButton_ = &rightNext;
    auto& leftPrev = AddChild<Button>(642, "<");
    leftPrev.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::EquipLeftPrev); });
    leftPrevButton_ = &leftPrev;
    auto& leftNext = AddChild<Button>(643, ">");
    leftNext.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::EquipLeftNext); });
    leftNextButton_ = &leftNext;

    auto& reset = AddChild<Button>(650, "Equipamento do DAT");
    reset.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ResetEquipment); });
    resetEquipmentButton_ = &reset;

    auto& resync = AddChild<Button>(651, "Recarregar DAT");
    resync.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ResyncData); });
    resyncButton_ = &resync;

    auto& manageEquipment = AddChild<Button>(790, "Gerir");
    manageEquipment.SetOnClick([this](const UIEvent&)
    {
        if (!SelectedEntry()) return;
        equipmentManagerOpen_ = true;
        selectedInventoryRecordIndex_ = -1;
        dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
        activeCompanionBag_=1;
        inventoryDisplayPage_=0;
        inventoryCacheCount_=0;
        inventoryExpectedPage_=0;
        inventoryLastProcessedPage_=-1;
        inventoryCacheComplete_=false;
        inventoryCacheName_=SelectedEntry()->name;
        gateway_.Execute(CompanionUiAction::InventoryPage, SelectedEntry()->name, 0);
        RefreshControls();
    });
    manageEquipmentButton_ = &manageEquipment;
    auto& openTrade = AddChild<Button>(860, "Troca");
    openTrade.SetOnClick([this](const UIEvent&) { OpenTradePreview(); });
    tradeOpenButton_ = &openTrade;
    auto& closeTrade = AddChild<Button>(861, "Fechar");
    closeTrade.SetOnClick([this](const UIEvent&) {
        tradePreviewOpen_ = false; tradePreviewName_.clear(); RefreshControls();
    });
    tradeCloseButton_ = &closeTrade;
    auto& refreshTrade = AddChild<Button>(862, "Atualizar listas");
    refreshTrade.SetOnClick([this](const UIEvent&) {
        tradeSelected_[0] = tradeSelected_[1] = -1;
        RequestTradePreview(0,tradePageRequested_[0]);
        RequestTradePreview(1,tradePageRequested_[1]);
    });
    tradeRefreshButton_ = &refreshTrade;
    auto& attemptTrade = AddChild<Button>(892, "Transferir item");
    attemptTrade.SetOnClick([this](const UIEvent&) { RequestTradeAttempt(); });
    tradeAttemptButton_ = &attemptTrade;
    for(int side=0; side<2; ++side)
    {
        auto& prev = AddChild<Button>(863+side*2, "< Anterior");
        prev.SetOnClick([this,side](const UIEvent&) {
            RequestTradePreview(side,std::max(0,tradePageRequested_[side]-1));
        });
        tradePrevButtons_[side]=&prev;
        auto& next = AddChild<Button>(864+side*2, "Seguinte >");
        next.SetOnClick([this,side](const UIEvent&) {
            const auto& page=gateway_.TradePreview(side);
            if(page.received && page.status==1 &&
               (tradePageRequested_[side]+1)*10<page.totalItems)
                RequestTradePreview(side,tradePageRequested_[side]+1);
        });
        tradeNextButtons_[side]=&next;
        for(int row=0;row<10;++row)
        {
            auto& button=AddChild<Button>(870+side*10+row, "-");
            button.SetOnClick([this,side,row](const UIEvent&) {
                const auto& page=gateway_.TradePreview(side);
                if(!tradePreviewOpen_ || !page.received ||
                   page.status != 1 || page.characterName != tradePreviewName_ ||
                   page.page != tradePageRequested_[side] || row >= page.itemCount)
                    return;
                const auto& item=page.items[static_cast<std::size_t>(row)];
                // Match rsTradeInvenItem: equipped entries are not in the
                // inventory trade window; potion stacks use quantity semantics
                // and are not supported by the exact-item Companion journal.
                if(gameplay::companions::v2::TradePreviewBlockReason352A(
                      item.position,item.code,item.key,item.checksum)!=
                   gameplay::companions::v2::TradePreviewBlock352A::None)
                    return;
                tradeSelected_[side]=item.index;
                RefreshControls();
            });
            tradeItemButtons_[side][row]=&button;
        }
    }

    auto& closeEquipment = AddChild<Button>(791, "Fechar");
    closeEquipment.SetOnClick([this](const UIEvent&)
    {
        equipmentManagerOpen_ = false;
        dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
        RefreshControls();
    });
    closeEquipmentManagerButton_ = &closeEquipment;
    auto& bagA = AddChild<Button>(850, "Mochila A");
    bagA.SetOnClick([this](const UIEvent&) { activeCompanionBag_=1; inventoryDisplayPage_=0; selectedInventoryRecordIndex_=-1; dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0; RefreshControls(); });
    inventoryBagAButton_=&bagA;
    auto& bagB = AddChild<Button>(851, "Mochila B");
    bagB.SetOnClick([this](const UIEvent&) { activeCompanionBag_=2; inventoryDisplayPage_=0; selectedInventoryRecordIndex_=-1; dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0; RefreshControls(); });
    inventoryBagBButton_=&bagB;
    auto& moveBag=AddChild<Button>(852,"Mover p/ B");
    moveBag.SetOnClick([this](const UIEvent&) {
        const int target=activeCompanionBag_==1?2:1;
        if(selectedInventoryRecordIndex_<0 || !inventoryCacheComplete_) return;
        const int index=selectedInventoryRecordIndex_;
        ExecuteForSelected(CompanionUiAction::MoveBag,(target<<16)|(index&0xffff));
        if(const auto* chosen=SelectedEntry()) {
            selectedInventoryRecordIndex_=-1;
            inventoryCacheCount_=0;inventoryExpectedPage_=0;
            inventoryLastProcessedPage_=-1;inventoryCacheComplete_=false;
            gateway_.Execute(CompanionUiAction::InventoryPage,chosen->name,0);
        }
    });
    inventoryMoveBagButton_=&moveBag;
    auto& organize=AddChild<Button>(853,"Organizar");
    organize.SetOnClick([this](const UIEvent&) {
        if(!inventoryCacheComplete_) return;
        ExecuteForSelected(CompanionUiAction::OrganizeBag,activeCompanionBag_);
        if(const auto* chosen=SelectedEntry()) {
            selectedInventoryRecordIndex_=-1;
            inventoryCacheCount_=0;inventoryExpectedPage_=0;
            inventoryLastProcessedPage_=-1;inventoryCacheComplete_=false;
            gateway_.Execute(CompanionUiAction::InventoryPage,chosen->name,0);
        }
    });
    inventoryOrganizeButton_=&organize;


    auto& manageRightPrev = AddChild<Button>(792, "<");
    manageRightPrev.SetOnClick([this](const UIEvent&)
    { ExecuteForSelected(CompanionUiAction::EquipRightPrev); });
    managerRightPrevButton_ = &manageRightPrev;
    auto& manageRightNext = AddChild<Button>(793, ">");
    manageRightNext.SetOnClick([this](const UIEvent&)
    { ExecuteForSelected(CompanionUiAction::EquipRightNext); });
    managerRightNextButton_ = &manageRightNext;
    auto& manageLeftPrev = AddChild<Button>(794, "<");
    manageLeftPrev.SetOnClick([this](const UIEvent&)
    { ExecuteForSelected(CompanionUiAction::EquipLeftPrev); });
    managerLeftPrevButton_ = &manageLeftPrev;
    auto& manageLeftNext = AddChild<Button>(795, ">");
    manageLeftNext.SetOnClick([this](const UIEvent&)
    { ExecuteForSelected(CompanionUiAction::EquipLeftNext); });
    managerLeftNextButton_ = &manageLeftNext;
    auto& manageReset = AddChild<Button>(796, "Equip. do DAT");
    manageReset.SetOnClick([this](const UIEvent&)
    { ExecuteForSelected(CompanionUiAction::ResetEquipment); });
    managerResetButton_ = &manageReset;
    auto& manageResync = AddChild<Button>(797, "Recarregar DAT");
    manageResync.SetOnClick([this](const UIEvent&)
    { ExecuteForSelected(CompanionUiAction::ResyncData); });
    managerResyncButton_ = &manageResync;

    for (std::size_t i=0; i<inventoryItemButtons_.size(); ++i)
    {
        auto& row = AddChild<Button>(800 + static_cast<UIId>(i), "-");
        row.SetOnClick([this, i](const UIEvent&)
        {
            const auto& inv = gateway_.InventorySnapshot();
            const CompanionUiEntry* selected = SelectedEntry();
            if (!selected || !inv.received || inv.characterName != selected->name ||
                i >= static_cast<std::size_t>(inv.itemCount)) return;
            selectedInventoryRecordIndex_ = inv.items[i].recordIndex;
            RefreshControls();
        });
        inventoryItemButtons_[i] = &row;
    }
    for(std::size_t i=0; i<inventorySlotButtons_.size(); ++i)
    {
        const int slot = static_cast<int>(i)+1;
        auto& button = AddChild<Button>(830+static_cast<UIId>(i), CompanionGearSlots[i].title);
        button.SetOnClick([this,slot](const UIEvent&) {
            selectedEquipSlot_ = slot;
            const auto& inv = gateway_.InventorySnapshot();
            bool allowed = false;
            for(int n=0; n<inv.itemCount; ++n)
                if(inv.items[static_cast<std::size_t>(n)].recordIndex == selectedInventoryRecordIndex_ &&
                   (inv.items[static_cast<std::size_t>(n)].equipMask & (1u<<slot)))
                    allowed = true;
            if(allowed && slot >= 3 && slot != 11 && slot != 12 && slot != 13)
                ExecuteForSelected(CompanionUiAction::SetInventorySlot,
                    (slot<<16)|(selectedInventoryRecordIndex_ & 0xFFFF));
            else if(allowed && slot == 1)
                ExecuteForSelected(CompanionUiAction::EquipInventoryRight, selectedInventoryRecordIndex_);
            else if(allowed && slot == 2)
                ExecuteForSelected(CompanionUiAction::EquipInventoryLeft, selectedInventoryRecordIndex_);
            else if(selectedInventoryRecordIndex_ < 0 && slot>=3 && slot!=11 && slot!=12 && slot!=13)
                ExecuteForSelected(CompanionUiAction::SetInventorySlot, (slot<<16)|0xFFFF);
        });
        inventorySlotButtons_[i] = &button;
    }
    auto& inventoryPrev = AddChild<Button>(810, "< Anterior");
    inventoryPrev.SetOnClick([this](const UIEvent&) {
        if(inventoryDisplayPage_>0) --inventoryDisplayPage_;
        RefreshControls();
    });
    inventoryPrevPageButton_=&inventoryPrev;
    auto& inventoryNext = AddChild<Button>(811, "Seguinte >");
    inventoryNext.SetOnClick([this](const UIEvent&) {
        ++inventoryDisplayPage_;
        RefreshControls();
    });
    inventoryNextPageButton_=&inventoryNext;
    auto& inventoryRight = AddChild<Button>(812, "Equipar direita");
    inventoryRight.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* companion = SelectedEntry();
        if (!companion) return;
        for (int i=0; i<inventoryCacheCount_; ++i)
        {
            const auto& item = inventoryCache_[i];
            if (item.recordIndex != selectedInventoryRecordIndex_) continue;
            if (item.bagNumber != 1 || item.position == -1) return;
            ExecuteForSelected(CompanionUiAction::EquipInventoryRight,
                item.code == companion->rightItemCode ? -1 : item.recordIndex);
            return;
        }
        if (selectedInventoryRecordIndex_ < 0 && companion->rightItemCode)
            ExecuteForSelected(CompanionUiAction::EquipInventoryRight, -1);
    });
    inventoryEquipRightButton_ = &inventoryRight;
    auto& inventoryLeft = AddChild<Button>(813, "Equipar esquerda");
    inventoryLeft.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* companion = SelectedEntry();
        if (!companion) return;
        for (int i=0; i<inventoryCacheCount_; ++i)
        {
            const auto& item = inventoryCache_[i];
            if (item.recordIndex != selectedInventoryRecordIndex_) continue;
            if (item.bagNumber != 1 || item.position == -1) return;
            ExecuteForSelected(CompanionUiAction::EquipInventoryLeft,
                item.code == companion->leftItemCode ? -1 : item.recordIndex);
            return;
        }
        if (selectedInventoryRecordIndex_ < 0 && companion->leftItemCode)
            ExecuteForSelected(CompanionUiAction::EquipInventoryLeft, -1);
    });
    inventoryEquipLeftButton_ = &inventoryLeft;
    auto& inventoryRefresh = AddChild<Button>(814, "Atualizar mochila");
    inventoryRefresh.SetOnClick([this](const UIEvent&)
    {
        if (const auto* selected = SelectedEntry())
        {
            selectedInventoryRecordIndex_=-1;
            dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
            inventoryCacheCount_=0;
            inventoryExpectedPage_=0;
            inventoryLastProcessedPage_=-1;
            inventoryCacheComplete_=false;
            inventoryCacheName_=selected->name;
            gateway_.Execute(CompanionUiAction::InventoryPage,selected->name,0);
        }
    });
    inventoryRefreshButton_ = &inventoryRefresh;

    auto& equipmentMode = AddChild<Button>(655, "Equipamento");
    equipmentMode.SetOnClick([this](const UIEvent&)
    {
        tacticsView_ = false;
        skillsView_ = false;
        buffSkillsView_ = false;
        RefreshControls();
    });
    equipmentModeButton_ = &equipmentMode;

    auto& detailMode = AddChild<Button>(652, "Skills");
    detailMode.SetOnClick([this](const UIEvent&)
    {
        tacticsView_ = false;
        skillsView_ = true;
        buffSkillsView_ = false;
        RefreshControls();
    });
    detailModeButton_ = &detailMode;

    auto& skillMode = AddChild<Button>(653, "Suporte");
    skillMode.SetOnClick([this](const UIEvent&)
    {
        tacticsView_ = false;
        skillsView_ = true;
        buffSkillsView_ = true;
        RefreshControls();
    });
    skillModeButton_ = &skillMode;

    auto& tacticsMode = AddChild<Button>(654, "Taticas custom");
    tacticsMode.SetOnClick([this](const UIEvent&)
    {
        tacticsView_ = !tacticsView_;
        if (tacticsView_) skillsView_ = false;
        RefreshControls();
    });
    tacticsModeButton_ = &tacticsMode;

    auto& customToggle = AddChild<Button>(719, "Custom OFF");
    customToggle.SetOnClick([this](const UIEvent&)
    {
        ExecuteForSelected(CompanionUiAction::ToggleCustomTactics);
    });
    customTacticsToggleButton_ = &customToggle;

    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        auto& activePrev = AddChild<Button>(660 + static_cast<UIId>(slot) * 2, "<");
        activePrev.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveSkill, -static_cast<int>(slot + 1));
        });
        activeSkillPrevButtons_[slot] = &activePrev;

        auto& activeNext = AddChild<Button>(661 + static_cast<UIId>(slot) * 2, ">");
        activeNext.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveSkill, static_cast<int>(slot + 1));
        });
        activeSkillNextButtons_[slot] = &activeNext;

        auto& minResource = AddChild<Button>(700 + static_cast<UIId>(slot) * 3, "R0");
        minResource.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveMinResource, static_cast<int>(slot + 1));
        });
        activeMinResourceButtons_[slot] = &minResource;

        auto& minTargets = AddChild<Button>(701 + static_cast<UIId>(slot) * 3, "N1");
        minTargets.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveMinTargets, static_cast<int>(slot + 1));
        });
        activeMinTargetsButtons_[slot] = &minTargets;

        auto& targetMode = AddChild<Button>(702 + static_cast<UIId>(slot) * 3, "TA");
        targetMode.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveTargetMode, static_cast<int>(slot + 1));
        });
        activeTargetModeButtons_[slot] = &targetMode;

        auto& buffPrev = AddChild<Button>(670 + static_cast<UIId>(slot) * 4, "<");
        buffPrev.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleBuffSkill, -static_cast<int>(slot + 1));
        });
        buffSkillPrevButtons_[slot] = &buffPrev;

        auto& buffNext = AddChild<Button>(671 + static_cast<UIId>(slot) * 4, ">");
        buffNext.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleBuffSkill, static_cast<int>(slot + 1));
        });
        buffSkillNextButtons_[slot] = &buffNext;

        auto& intervalPrev = AddChild<Button>(672 + static_cast<UIId>(slot) * 4, "-");
        intervalPrev.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::AdjustBuffInterval, -static_cast<int>(slot + 1));
        });
        buffIntervalPrevButtons_[slot] = &intervalPrev;

        auto& intervalNext = AddChild<Button>(673 + static_cast<UIId>(slot) * 4, "+");
        intervalNext.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::AdjustBuffInterval, static_cast<int>(slot + 1));
        });
        buffIntervalNextButtons_[slot] = &intervalNext;
    }

    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        const int value = static_cast<int>(slot + 1);
        auto& enabled = AddChild<Button>(720 + static_cast<UIId>(slot) * 6, "ON");
        enabled.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ToggleTacticRule, value); });
        tacticRuleEnabledButtons_[slot] = &enabled;

        auto& condition = AddChild<Button>(721 + static_cast<UIId>(slot) * 6, "Always");
        condition.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::CycleTacticCondition, value); });
        tacticRuleConditionButtons_[slot] = &condition;

        auto& action = AddChild<Button>(722 + static_cast<UIId>(slot) * 6, "Nearest");
        action.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::CycleTacticAction, value); });
        tacticRuleActionButtons_[slot] = &action;

        auto& threshold = AddChild<Button>(723 + static_cast<UIId>(slot) * 6, "-");
        threshold.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::CycleTacticThreshold, value); });
        tacticRuleThresholdButtons_[slot] = &threshold;

        auto& up = AddChild<Button>(724 + static_cast<UIId>(slot) * 6, "^");
        up.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::MoveTacticRule, -value); });
        tacticRuleUpButtons_[slot] = &up;

        auto& down = AddChild<Button>(725 + static_cast<UIId>(slot) * 6, "v");
        down.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::MoveTacticRule, value); });
        tacticRuleDownButtons_[slot] = &down;
    }

    auto& resetActive = AddChild<Button>(690, "Ativas: auto V1");
    resetActive.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ResetActiveSkills); });
    resetActiveSkillsButton_ = &resetActive;

    auto& resetBuffs = AddChild<Button>(691, "Desligar suporte");
    resetBuffs.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ResetBuffs); });
    resetBuffsButton_ = &resetBuffs;

    LayoutControls();
}

void CompanionsPage::LayoutControls()
{
    for (std::size_t i = 0; i < characterButtons_.size(); ++i)
        characterButtons_[i]->SetBounds({22, 74 + static_cast<int>(i) * 48, 236, 38});

    summonDismissButton_->SetBounds({282, 74, 150, 34});
    dismissAllButton_->SetBounds({440, 74, 150, 34});
    refreshButton_->SetBounds({598, 74, 126, 34});

    aggressiveButton_->SetBounds({282, 132, 136, 32});
    followButton_->SetBounds({426, 132, 136, 32});
    passiveButton_->SetBounds({570, 132, 136, 32});

    tacticPrevButton_->SetBounds({282, 172, 36, 30});
    tacticNextButton_->SetBounds({688, 172, 36, 30});
    retreatPrevButton_->SetBounds({282, 208, 36, 28});
    retreatNextButton_->SetBounds({446, 208, 36, 28});
    rangePrevButton_->SetBounds({496, 208, 36, 28});
    rangeNextButton_->SetBounds({688, 208, 36, 28});

    rightPrevButton_->SetBounds({282, 326, 36, 30});
    rightNextButton_->SetBounds({688, 326, 36, 30});
    leftPrevButton_->SetBounds({282, 360, 36, 30});
    leftNextButton_->SetBounds({688, 360, 36, 30});

    resetEquipmentButton_->SetBounds({282, 404, 145, 32});
    resyncButton_->SetBounds({434, 404, 130, 32});
    tradeOpenButton_->SetBounds({571, 404, 82, 32});
    tradeCloseButton_->SetBounds({620, 14, 94, 28});
    tradeRefreshButton_->SetBounds({410, 14, 194, 28});
    tradeAttemptButton_->SetBounds({22, 444, 194, 27});
    for(int side=0;side<2;++side)
    {
        const int x=side==0?22:394;
        tradePrevButtons_[side]->SetBounds({x,408,140,30});
        tradeNextButtons_[side]->SetBounds({x+148,408,140,30});
        // A visual 5x2 inventory page per character; the authoritative
        // record index and revision remain unchanged by the presentation.
        for(int row=0;row<10;++row)
            tradeItemButtons_[side][row]->SetBounds(
                {x+4+(row%5)*64,124+(row/5)*102,58,82});
    }
    manageEquipmentButton_->SetBounds({660, 404, 64, 32});
    closeEquipmentManagerButton_->SetBounds({620, 14, 94, 28});
    for (std::size_t i=0; i<inventoryItemButtons_.size(); ++i)
    {
        const int col = static_cast<int>(i)%12, row = static_cast<int>(i)/12;
        inventoryItemButtons_[i]->SetBounds({348 + col*30, 140 + row*30, 28, 28});
    }
    for(std::size_t i=0; i<inventorySlotButtons_.size(); ++i)
    {
        const auto& s = CompanionGearSlots[i];
        inventorySlotButtons_[i]->SetBounds({s.x, s.y, s.w, s.h});
    }
    inventoryBagAButton_->SetBounds({346,52,92,30});
    inventoryBagBButton_->SetBounds({444,52,92,30});
    inventoryMoveBagButton_->SetBounds({541,52,91,30});
    inventoryOrganizeButton_->SetBounds({638,52,82,30});
    inventoryPrevPageButton_->SetBounds({346, 418, 110, 27});
    inventoryNextPageButton_->SetBounds({464, 418, 110, 27});
    inventoryRefreshButton_->SetBounds({582, 418, 125, 27});
    inventoryEquipRightButton_->SetBounds({24, 418, 145, 28});
    inventoryEquipLeftButton_->SetBounds({180, 418, 145, 28});
    managerRightPrevButton_->SetBounds({292, 332, 36, 30});
    managerRightNextButton_->SetBounds({678, 332, 36, 30});
    managerLeftPrevButton_->SetBounds({292, 370, 36, 30});
    managerLeftNextButton_->SetBounds({678, 370, 36, 30});
    managerResetButton_->SetBounds({292, 409, 178, 29});
    managerResyncButton_->SetBounds({480, 409, 180, 29});
    equipmentModeButton_->SetBounds({292, 246, 100, 28});
    tacticsModeButton_->SetBounds({398, 246, 100, 28});
    skillModeButton_->SetBounds({504, 246, 100, 28});
    detailModeButton_->SetBounds({610, 246, 114, 28});
    customTacticsToggleButton_->SetBounds({292, 282, 120, 26});

    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        const int y = 292 + static_cast<int>(slot) * 29;
        // Active and buff controls share the same wide row geometry, but only
        // one group is visible at a time. This gives long skill names room.
        activeSkillPrevButtons_[slot]->SetBounds({292, y, 30, 24});
        activeMinResourceButtons_[slot]->SetBounds({558, y, 38, 24});
        activeMinTargetsButtons_[slot]->SetBounds({600, y, 38, 24});
        activeTargetModeButtons_[slot]->SetBounds({642, y, 38, 24});
        activeSkillNextButtons_[slot]->SetBounds({684, y, 30, 24});

        buffSkillPrevButtons_[slot]->SetBounds({292, y, 30, 24});
        buffSkillNextButtons_[slot]->SetBounds({566, y, 30, 24});
        buffIntervalPrevButtons_[slot]->SetBounds({650, y, 28, 24});
        buffIntervalNextButtons_[slot]->SetBounds({686, y, 28, 24});
    }
    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        const int y = 316 + static_cast<int>(slot) * 29;
        tacticRuleEnabledButtons_[slot]->SetBounds({292, y, 38, 24});
        tacticRuleConditionButtons_[slot]->SetBounds({334, y, 112, 24});
        tacticRuleActionButtons_[slot]->SetBounds({450, y, 100, 24});
        tacticRuleThresholdButtons_[slot]->SetBounds({554, y, 44, 24});
        tacticRuleUpButtons_[slot]->SetBounds({602, y, 30, 24});
        tacticRuleDownButtons_[slot]->SetBounds({636, y, 30, 24});
    }
    resetActiveSkillsButton_->SetBounds({292, 410, 220, 28});
    resetBuffsButton_->SetBounds({292, 410, 220, 28});
}

const CompanionUiEntry* CompanionsPage::SelectedEntry() const
{
    const auto& snapshot = gateway_.Snapshot();
    if (!snapshot.valid)
        return nullptr;
    for (int i = 0; i < snapshot.entryCount; ++i)
    {
        const auto& entry = snapshot.entries[static_cast<std::size_t>(i)];
        if (entry.selected && !entry.currentCharacter)
            return &entry;
    }
    return nullptr;
}

void CompanionsPage::SelectVisibleRow(std::size_t row)
{
    const auto& snapshot = gateway_.Snapshot();
    if (!snapshot.valid || row >= static_cast<std::size_t>(snapshot.entryCount))
        return;
    const auto& entry = snapshot.entries[row];
    if (entry.name.empty() || entry.currentCharacter)
        return;
    equipmentManagerOpen_ = false;
    tradePreviewOpen_ = false;
    dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
    selectedInventoryRecordIndex_ = -1;
    gateway_.Execute(CompanionUiAction::Select, entry.name);
}

void CompanionsPage::RequestTradePreview(int side, int page)
{
    if(!tradePreviewOpen_ || tradePreviewName_.empty() || side<0 || side>1 ||
       page<0 || page>1000) return;
    tradePageRequested_[side] = page;
    tradeSelected_[side]=-1;
    gateway_.Execute(CompanionUiAction::TradePage, tradePreviewName_,
                     (side<<16)|page);
}

void CompanionsPage::OpenTradePreview()
{
    const auto* selected=SelectedEntry();
    if(!selected || selected->active) return;
    equipmentManagerOpen_=false;
    tradePreviewOpen_=true;
    tradePreviewName_=selected->name;
    tradePageRequested_[0]=tradePageRequested_[1]=0;
    tradeSelected_[0]=tradeSelected_[1]=-1;
    RequestTradePreview(0,0);
    RequestTradePreview(1,0);
    RefreshControls();
}

void CompanionsPage::RequestTradeAttempt()
{
    if(!tradePreviewOpen_ || tradePreviewName_.empty()) return;
    const int side = tradeSelected_[0] >= 0 ? 0 : (tradeSelected_[1] >= 0 ? 1 : -1);
    if(side < 0 || (tradeSelected_[0] >= 0 && tradeSelected_[1] >= 0)) return;
    const auto& src = gateway_.TradePreview(side);
    const auto& owner = gateway_.TradePreview(0);
    const auto& companion = gateway_.TradePreview(1);
    if(!src.received || !owner.received || !companion.received ||
       src.status != 1 || owner.status != 1 || companion.status != 1 ||
       src.characterName != tradePreviewName_ ||
       owner.characterName != tradePreviewName_ ||
       companion.characterName != tradePreviewName_ ||
       !owner.revision || !companion.revision) return;
    for(int i=0; i<src.itemCount; ++i)
    {
        const auto& selected=src.items[static_cast<std::size_t>(i)];
        if(selected.index==tradeSelected_[side])
        {
            if(gameplay::companions::v2::TradePreviewBlockReason352A(
                   selected.position,selected.code,selected.key,selected.checksum)!=
               gameplay::companions::v2::TradePreviewBlock352A::None) return;
            gateway_.RequestTradeAttempt(tradePreviewName_,side,
                selected.index,selected.code,selected.key,selected.checksum,
                owner.revision,companion.revision);
            return;
        }
    }
}

void CompanionsPage::RenderTradePreview(UIRenderContext& context)
{
    const Rect root=GlobalBounds();
    auto& render=context.Renderer();
    render.DrawRect(root,Color{16,20,29,255});
    render.DrawTextUtf8(FontHandle{},"TROCA - "+tradePreviewName_,
        {root.x+24,root.y+16,380,25},TextStyle{UITheme::Accent,true,TextAlign::Left});
    render.DrawTextUtf8(FontHandle{},"TROCA DE ITENS - ATUALIZACAO EM DIRETO",
        {root.x+24,root.y+50,680,25},TextStyle{UITheme::TextMuted,true,TextAlign::Center});
    for(int side=0;side<2;++side)
    {
        const int x=side==0?22:394;
        render.DrawTextUtf8(FontHandle{},side==0?"JOGADOR (ONLINE)":"COMPANION (DAT)",
            {root.x+x,root.y+77,320,21},TextStyle{UITheme::Accent,true,TextAlign::Left});
        const auto& page=gateway_.TradePreview(side);
        if(!page.received || page.characterName!=tradePreviewName_ ||
           page.side!=side || page.page!=tradePageRequested_[side])
            render.DrawTextUtf8(FontHandle{},"A aguardar servidor...",
                {root.x+x,root.y+384,315,20},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
        else if(page.status != 1)
            render.DrawTextUtf8(FontHandle{},"Indisponivel ("+std::to_string(page.status)+")",
                {root.x+x,root.y+384,315,20},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
        else
        {
            render.DrawTextUtf8(FontHandle{},"Itens "+std::to_string(page.totalItems)+
                " | Pagina "+std::to_string(page.page+1),
                {root.x+x,root.y+384,315,20},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
            std::string choice="Clica num item para selecionar";
            for(int i=0;i<page.itemCount;++i)
            {
                const auto& item=page.items[static_cast<std::size_t>(i)];
                if(item.index!=tradeSelected_[side]) continue;
                choice="Item selecionado para "+std::string(side==0?"enviar":"receber");
                break;
            }
            render.DrawTextUtf8(FontHandle{},choice,
                {root.x+x,root.y+326,320,20},TextStyle{UITheme::Text,false,TextAlign::Left});
            render.DrawTextUtf8(FontHandle{},"Cinza: EQUIP / SEM ID / POCAO / POS?",
                {root.x+x,root.y+354,320,18},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
        }
    }
    const auto& attempt = gateway_.TradeAttemptResult();
    const std::string status = attempt.received && attempt.characterName==tradePreviewName_
        ? (attempt.status==2 ? "Gravada. Se nao sincronizar, volta a entrar."
          : (attempt.status==3 ? "Recuperada. Se nao sincronizar, volta a entrar."
          : (attempt.status==-74 || attempt.status==-75 ? "Resultado incerto: fecha o jogo; verifica os DATs."
          : (attempt.status==-55 ? "Save sem resposta: confirmacao NAO enviada."
          : (attempt.status==1 ? (gameplay::companions::v2::TradeClientFrozen()
             ? "Inventario aplicado: a confirmar save..."
             : "Troca concluida. Listas atualizadas.")
          : "Troca recusada: "+std::to_string(attempt.status)+" (sem transferencia)")))))
        : "Seleciona um item e confirma a troca.";
    render.DrawTextUtf8(FontHandle{},status,
        {root.x+235,root.y+450,495,21},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
}

void CompanionsPage::ExecuteForSelected(CompanionUiAction action, int value)
{
    const CompanionUiEntry* selected = SelectedEntry();
    if (!selected)
    {
        summit::companion::diagnostic::Trace("CLIENT","ui-no-selection",
            static_cast<int>(action),value,-8);
        return;
    }
    summit::companion::diagnostic::Trace("CLIENT","ui-action",
        static_cast<int>(action),value,0);
    gateway_.Execute(action, selected->name, value);
}

void CompanionsPage::ForceRefresh()
{
    refreshAccumulator_ = 0.0f;
    gateway_.RequestRefresh();
}

void CompanionsPage::RefreshControls()
{
    const auto& snapshot = gateway_.Snapshot();
    for (std::size_t row = 0; row < characterButtons_.size(); ++row)
    {
        Button* button = characterButtons_[row];
        if (!snapshot.valid || row >= static_cast<std::size_t>(snapshot.entryCount))
        {
            button->SetText("-");
            button->SetEnabled(false);
            button->SetSelected(false);
            continue;
        }

        const auto& entry = snapshot.entries[row];
        std::string text = entry.name.empty() ? "-" : entry.name;
        if (entry.currentCharacter)
            text += "  [em uso]";
        else if (entry.active)
        {
            text += entry.dead ? "  [morto]" : "  [ativo]";
            if (entry.maxLife > 0)
                text += " HP " + std::to_string(entry.life) + "/" + std::to_string(entry.maxLife);
        }
        if (entry.level > 0)
            text += "  Lv " + std::to_string(entry.level);
        button->SetText(text);
        button->SetEnabled(!entry.currentCharacter && !entry.name.empty());
        button->SetSelected(entry.selected);
    }

    const CompanionUiEntry* selected = SelectedEntry();
    const bool hasSelected = selected != nullptr;
    summonDismissButton_->SetEnabled(hasSelected);
    summonDismissButton_->SetText(hasSelected && selected->active ? "Dispensar" : "Invocar");
    dismissAllButton_->SetEnabled(snapshot.valid && snapshot.activeCount > 0);
    if (!hasSelected)
        equipmentManagerOpen_ = false;
    const bool equipmentManager = equipmentManagerOpen_ && hasSelected;
    const bool equipmentView = !skillsView_ && !tacticsView_;
    equipmentModeButton_->SetVisible(true);
    detailModeButton_->SetVisible(true);
    tacticsModeButton_->SetVisible(true);
    equipmentModeButton_->SetEnabled(hasSelected);
    equipmentModeButton_->SetSelected(hasSelected && equipmentView);
    equipmentModeButton_->SetText("Equipamento");
    detailModeButton_->SetEnabled(hasSelected);
    detailModeButton_->SetSelected(hasSelected && skillsView_ && !tacticsView_ && !buffSkillsView_);
    detailModeButton_->SetText("Skills");
    // Keep all top-level tabs stable. Selecting Tactics must not make the
    // Support tab disappear; clicking Support already leaves tacticsView_.
    skillModeButton_->SetVisible(true);
    skillModeButton_->SetEnabled(hasSelected);
    skillModeButton_->SetSelected(hasSelected && skillsView_ && !tacticsView_ && buffSkillsView_);
    skillModeButton_->SetText("Suporte");
    tacticsModeButton_->SetEnabled(hasSelected);
    tacticsModeButton_->SetSelected(tacticsView_);
    tacticsModeButton_->SetText("Taticas custom");

    rightPrevButton_->SetVisible(equipmentView);
    rightNextButton_->SetVisible(equipmentView);
    leftPrevButton_->SetVisible(equipmentView);
    leftNextButton_->SetVisible(equipmentView);
    resetEquipmentButton_->SetVisible(equipmentView);
    resyncButton_->SetVisible(equipmentView);
    rightPrevButton_->SetEnabled(hasSelected && equipmentView);
    rightNextButton_->SetEnabled(hasSelected && equipmentView);
    leftPrevButton_->SetEnabled(hasSelected && equipmentView);
    leftNextButton_->SetEnabled(hasSelected && equipmentView);
    resetEquipmentButton_->SetEnabled(hasSelected && equipmentView);
    resyncButton_->SetEnabled(hasSelected && equipmentView);
    manageEquipmentButton_->SetVisible(equipmentView && !equipmentManager);
    manageEquipmentButton_->SetEnabled(hasSelected && equipmentView && !equipmentManager);
    closeEquipmentManagerButton_->SetVisible(equipmentManager);
    closeEquipmentManagerButton_->SetEnabled(equipmentManager);
    Button* managerControls[] = {
        managerRightPrevButton_, managerRightNextButton_,
        managerLeftPrevButton_, managerLeftNextButton_,
        managerResetButton_, managerResyncButton_
    };
    for (Button* control : managerControls)
    {
        control->SetVisible(false); // Retire 307's redundant two-hand overlay controls.
        control->SetEnabled(false);
    }
    // Text buttons from PATCH 309 must never cover item icons.
    for (Button* b : inventoryItemButtons_) { b->SetVisible(false); b->SetEnabled(false); }
    for (Button* b : inventorySlotButtons_) { b->SetVisible(false); b->SetEnabled(false); }
    inventoryBagAButton_->SetVisible(equipmentManager);
    inventoryBagBButton_->SetVisible(equipmentManager);
    inventoryMoveBagButton_->SetVisible(equipmentManager);
    inventoryOrganizeButton_->SetVisible(equipmentManager);
    bool selectedBagItem=false;
    for(int i=0;i<inventoryCacheCount_;++i)
        if(inventoryCache_[i].recordIndex==selectedInventoryRecordIndex_ &&
           inventoryCache_[i].position==0 &&
           inventoryCache_[i].bagNumber==activeCompanionBag_ &&
           inventoryCache_[i].gridX>=0 && inventoryCache_[i].gridY>=0)
            selectedBagItem=true;
    inventoryMoveBagButton_->SetText(activeCompanionBag_==1?"Mover p/ B":"Mover p/ A");
    inventoryMoveBagButton_->SetEnabled(equipmentManager && inventoryCacheComplete_ &&
        selectedBagItem && selected && !selected->active);
    inventoryOrganizeButton_->SetEnabled(equipmentManager && inventoryCacheComplete_ &&
        selected && !selected->active);

    inventoryBagAButton_->SetSelected(activeCompanionBag_==1);
    inventoryBagBButton_->SetSelected(activeCompanionBag_==2);
    inventoryPrevPageButton_->SetVisible(equipmentManager);
    inventoryNextPageButton_->SetVisible(equipmentManager);
    inventoryRefreshButton_->SetVisible(equipmentManager);
    inventoryEquipRightButton_->SetVisible(equipmentManager);
    inventoryEquipLeftButton_->SetVisible(equipmentManager);
    int visibleCount=0;
    for (int i=0;i<inventoryCacheCount_;++i)
        if(inventoryCache_[i].position==0 && inventoryCache_[i].bagNumber==activeCompanionBag_)
            ++visibleCount;
    inventoryPrevPageButton_->SetEnabled(equipmentManager && inventoryDisplayPage_>0);
    inventoryNextPageButton_->SetEnabled(equipmentManager &&
        (inventoryDisplayPage_+1)*CompanionBagCapacity<visibleCount);
    inventoryRefreshButton_->SetEnabled(equipmentManager);
    bool allowRight=false,allowLeft=false;
    bool removeRight=false,removeLeft=false;
    for(int i=0;i<inventoryCacheCount_;++i)
        if(inventoryCache_[i].recordIndex==selectedInventoryRecordIndex_) {
            const auto& item=inventoryCache_[i];
            // A stale display mask must never prevent the authoritative V2
            // policy from assessing an item. The server validates the slot,
            // class, requirements, ownership and record index on every request.
            allowRight=item.bagNumber==1 && item.position!=-1;
            allowLeft=item.bagNumber==1 && item.position!=-1;
            removeRight=allowRight && selected && item.code==selected->rightItemCode;
            removeLeft=allowLeft && selected && item.code==selected->leftItemCode;
        }
    if(selectedInventoryRecordIndex_<0 && selected) {
        removeRight=selected->rightItemCode!=0;
        removeLeft=selected->leftItemCode!=0;
    }
    inventoryEquipRightButton_->SetText(removeRight?"Retirar direita":"Equipar direita");
    inventoryEquipLeftButton_->SetText(removeLeft?"Retirar esquerda":"Equipar esquerda");
    inventoryEquipRightButton_->SetEnabled(equipmentManager && (allowRight || removeRight));
    inventoryEquipLeftButton_->SetEnabled(equipmentManager && (allowLeft || removeLeft));
    if (equipmentManager)
    {
        // Prevent hidden underlying controls from receiving pointer events.
        Button* coveredControls[] = {
            equipmentModeButton_, detailModeButton_, skillModeButton_, tacticsModeButton_,
            rightPrevButton_, rightNextButton_, leftPrevButton_, leftNextButton_,
            resetEquipmentButton_, resyncButton_
        };
        for (Button* control : coveredControls)
            control->SetVisible(false);
        // A real separate management surface must not leave active, clickable
        // controls from the underlying character page beneath the inventory.
        for (Button* control : characterButtons_) control->SetVisible(false);
        Button* general[] = {summonDismissButton_, dismissAllButton_, refreshButton_,
            aggressiveButton_, followButton_, passiveButton_, tacticPrevButton_,
            tacticNextButton_, retreatPrevButton_, retreatNextButton_, rangePrevButton_,
            rangeNextButton_, customTacticsToggleButton_, resetActiveSkillsButton_,
            resetBuffsButton_};
        for (Button* control : general) control->SetVisible(false);
    }
    else
    {
        for (Button* control : characterButtons_) control->SetVisible(true);
        summonDismissButton_->SetVisible(true);
        dismissAllButton_->SetVisible(true);
        refreshButton_->SetVisible(true);
        aggressiveButton_->SetVisible(true);
        followButton_->SetVisible(true);
        passiveButton_->SetVisible(true);
        tacticPrevButton_->SetVisible(true);
        tacticNextButton_->SetVisible(true);
        retreatPrevButton_->SetVisible(true);
        retreatNextButton_->SetVisible(true);
        rangePrevButton_->SetVisible(true);
        rangeNextButton_->SetVisible(true);
    }

    customTacticsToggleButton_->SetVisible(tacticsView_ && !equipmentManager);
    customTacticsToggleButton_->SetEnabled(hasSelected && tacticsView_);
    customTacticsToggleButton_->SetSelected(snapshot.customTacticsEnabled);
    customTacticsToggleButton_->SetText(snapshot.customTacticsEnabled ? "Custom ON" : "Custom OFF");
    for(std::size_t ruleIndex = 0; ruleIndex < 4; ++ruleIndex)
    {
        const auto& rule = snapshot.tacticRules[ruleIndex];
        Button* controls[] = { tacticRuleEnabledButtons_[ruleIndex], tacticRuleConditionButtons_[ruleIndex],
            tacticRuleActionButtons_[ruleIndex], tacticRuleThresholdButtons_[ruleIndex],
            tacticRuleUpButtons_[ruleIndex], tacticRuleDownButtons_[ruleIndex] };
        for(Button* control : controls)
        {
            control->SetVisible(tacticsView_ && !equipmentManager);
            control->SetEnabled(hasSelected && tacticsView_);
        }
        tacticRuleEnabledButtons_[ruleIndex]->SetSelected(rule.enabled);
        tacticRuleEnabledButtons_[ruleIndex]->SetText(rule.enabled ? "ON" : "OFF");
        tacticRuleConditionButtons_[ruleIndex]->SetText(CustomConditionName(rule.condition));
        tacticRuleActionButtons_[ruleIndex]->SetText(CustomActionName(rule.action));
        tacticRuleThresholdButtons_[ruleIndex]->SetText(rule.threshold > 0 ? std::to_string(rule.threshold) : "--");
        tacticRuleUpButtons_[ruleIndex]->SetEnabled(hasSelected && tacticsView_ && ruleIndex > 0);
        tacticRuleDownButtons_[ruleIndex]->SetEnabled(hasSelected && tacticsView_ && ruleIndex + 1 < 4);
    }

    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        const bool buffConfigured = snapshot.valid && snapshot.buffSkills[slot].useSlot != 0;
        const bool activeView = skillsView_ && !tacticsView_ && !buffSkillsView_ && !equipmentManager;
        const bool buffView = skillsView_ && !tacticsView_ && buffSkillsView_ && !equipmentManager;
        activeSkillPrevButtons_[slot]->SetVisible(activeView);
        activeSkillNextButtons_[slot]->SetVisible(activeView);
        activeMinResourceButtons_[slot]->SetVisible(activeView);
        activeMinTargetsButtons_[slot]->SetVisible(activeView);
        activeTargetModeButtons_[slot]->SetVisible(activeView);
        buffSkillPrevButtons_[slot]->SetVisible(buffView);
        buffSkillNextButtons_[slot]->SetVisible(buffView);
        buffIntervalPrevButtons_[slot]->SetVisible(buffView);
        buffIntervalNextButtons_[slot]->SetVisible(buffView);
        activeSkillPrevButtons_[slot]->SetEnabled(hasSelected && activeView);
        activeSkillNextButtons_[slot]->SetEnabled(hasSelected && activeView);
        const bool activeConfigured = snapshot.valid && snapshot.activeSkills[slot].useSlot != 0;
        activeMinResourceButtons_[slot]->SetEnabled(hasSelected && activeView && activeConfigured);
        activeMinTargetsButtons_[slot]->SetEnabled(hasSelected && activeView && activeConfigured);
        activeTargetModeButtons_[slot]->SetEnabled(hasSelected && activeView && activeConfigured);
        if(snapshot.valid)
        {
            const auto& policy = snapshot.activeSkills[slot];
            activeMinResourceButtons_[slot]->SetText("R" + std::to_string(policy.minResourcePercent));
            activeMinTargetsButtons_[slot]->SetText("N" + std::to_string(policy.minimumTargets));
            activeTargetModeButtons_[slot]->SetText(policy.targetMode == 1 ? "TO" : (policy.targetMode == 2 ? "TN" : "TA"));
        }
        buffSkillPrevButtons_[slot]->SetEnabled(hasSelected && buffView);
        buffSkillNextButtons_[slot]->SetEnabled(hasSelected && buffView);
        buffIntervalPrevButtons_[slot]->SetEnabled(hasSelected && buffView && buffConfigured);
        buffIntervalNextButtons_[slot]->SetEnabled(hasSelected && buffView && buffConfigured);
    }
    resetActiveSkillsButton_->SetVisible(skillsView_ && !tacticsView_ && !buffSkillsView_ && !equipmentManager);
    resetBuffsButton_->SetVisible(skillsView_ && !tacticsView_ && buffSkillsView_ && !equipmentManager);
    resetActiveSkillsButton_->SetEnabled(hasSelected && skillsView_ && !tacticsView_ && !buffSkillsView_);
    resetBuffsButton_->SetEnabled(hasSelected && skillsView_ && !tacticsView_ && buffSkillsView_);

    aggressiveButton_->SetEnabled(hasSelected);
    followButton_->SetEnabled(hasSelected);
    passiveButton_->SetEnabled(hasSelected);
    tacticPrevButton_->SetEnabled(hasSelected);
    tacticNextButton_->SetEnabled(hasSelected);
    retreatPrevButton_->SetEnabled(hasSelected);
    retreatNextButton_->SetEnabled(hasSelected);
    rangePrevButton_->SetEnabled(hasSelected);
    rangeNextButton_->SetEnabled(hasSelected);

    aggressiveButton_->SetSelected(hasSelected && selected->mode == CompanionUiMode::Aggressive);
    followButton_->SetSelected(hasSelected && selected->mode == CompanionUiMode::Follow);
    passiveButton_->SetSelected(hasSelected && selected->mode == CompanionUiMode::Passive);

    // Modal exclusivity: no hidden equipment interaction through trade preview.
    tradeOpenButton_->SetVisible(hasSelected && equipmentView && !equipmentManager && !tradePreviewOpen_);
    tradeOpenButton_->SetEnabled(hasSelected && !selected->active);
    tradeCloseButton_->SetVisible(tradePreviewOpen_);
    tradeRefreshButton_->SetVisible(tradePreviewOpen_);
    const bool oneSelected=(tradeSelected_[0] >= 0) != (tradeSelected_[1] >= 0);
    tradeAttemptButton_->SetVisible(tradePreviewOpen_);
    tradeAttemptButton_->SetEnabled(tradePreviewOpen_ && oneSelected);
    for(int side=0;side<2;++side)
    {
        tradePrevButtons_[side]->SetVisible(tradePreviewOpen_);
        tradeNextButtons_[side]->SetVisible(tradePreviewOpen_);
        const auto& page=gateway_.TradePreview(side);
        const bool valid=tradePreviewOpen_ && page.received && page.status==1 &&
            page.characterName==tradePreviewName_ && page.side==side &&
            page.page==tradePageRequested_[side];
        tradePrevButtons_[side]->SetEnabled(valid && tradePageRequested_[side]>0);
        tradeNextButtons_[side]->SetEnabled(valid &&
            (tradePageRequested_[side]+1)*10<page.totalItems);
        for(int row=0;row<10;++row)
        {
            Button* button=tradeItemButtons_[side][row];
            button->SetVisible(tradePreviewOpen_ && valid && row<page.itemCount);
            if(!valid || row>=page.itemCount) { button->SetIcon({}); button->SetText(""); }
            const auto reason = (valid && row<page.itemCount) ?
                gameplay::companions::v2::TradePreviewBlockReason352A(
                    page.items[static_cast<std::size_t>(row)].position,
                    page.items[static_cast<std::size_t>(row)].code,
                    page.items[static_cast<std::size_t>(row)].key,
                    page.items[static_cast<std::size_t>(row)].checksum) :
                gameplay::companions::v2::TradePreviewBlock352A::UnknownPosition;
            button->SetEnabled(valid && row<page.itemCount &&
                reason==gameplay::companions::v2::TradePreviewBlock352A::None);
            if(valid && row<page.itemCount)
            {
                const auto& item=page.items[static_cast<std::size_t>(row)];
                // Keep passive records visible rather than hiding the
                // entire page; explain why a row cannot be transferred.
                int iconW=1,iconH=1;
                const auto icon=CompanionItemIcon(item.code,iconW,iconH);
                button->SetIcon(icon);
                // A missing icon is distinguishable without exposing raw
                // item codes. Disabled entries still show their art.
                // The preview keeps forbidden records visible but now
                // explains each disabled cell, without ever enabling it.
                button->SetText(reason!=gameplay::companions::v2::TradePreviewBlock352A::None ?
                    gameplay::companions::v2::TradePreviewBlockLabel352A(reason) :
                    (icon.IsValid()?"":"..."));
                button->SetSelected(item.index==tradeSelected_[side]);
            }
        }
    }
    if(tradePreviewOpen_)
        for(const auto& child : Children())
            if(child->Id()<860)
                child->SetVisible(false);
}

void CompanionsPage::Update(float deltaSeconds)
{
    // SUMMIT350V: the always-on client bridge pumps trade exactly once.
    iconWarmAccumulator_ += std::min(std::max(deltaSeconds,0.0f),0.25f);
    if(iconWarmAccumulator_>=0.10f && (tradePreviewOpen_ || equipmentManagerOpen_))
    {
        iconWarmAccumulator_=0.0f;
        // At most ONE synchronous catalog BMP load per ~100ms. Never load
        // assets from OnRender or RefreshControls.
        bool warmed=false;
        if(tradePreviewOpen_)
            for(int side=0;side<2 && !warmed;++side)
            {
                const auto& page=gateway_.TradePreview(side);
                if(!page.received || page.status!=1 ||
                   page.characterName!=tradePreviewName_ ||
                   page.page!=tradePageRequested_[side]) continue;
                for(int i=0;i<page.itemCount && !warmed;++i)
                    warmed=CompanionWarmIcon(page.items[static_cast<std::size_t>(i)].code);
            }
        else
            for(int i=0;i<inventoryCacheCount_ && !warmed;++i)
                if(inventoryCache_[i].position!=0 ||
                   (inventoryCache_[i].bagNumber==activeCompanionBag_ &&
                    inventoryItemPages_[i]==inventoryDisplayPage_))
                    warmed=CompanionWarmIcon(inventoryCache_[i].code);
    }
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 30.0f)
    {
        refreshAccumulator_ = 0.0f;
        gateway_.RequestRefresh();
    }
    // TradePages from before the journal commit are stale. The gateway's
    // first successful delivery (status=1) is NOT the durability receipt;
    // refresh only after the authenticated post-save ACK has thawed the UI.
    const auto& tradeResult351I=gateway_.TradeAttemptResult();
    if(gameplay::companions::v2::TradePreviewRefreshDue351I(
          tradePreviewOpen_ && !tradePreviewName_.empty() &&
          tradeResult351I.characterName==tradePreviewName_,
          tradeResult351I.received,tradeResult351I.status,
          tradeResult351I.requestId,tradeRefreshedRequest351I_,
          gameplay::companions::v2::TradeClientFrozen()))
    {
        tradeRefreshedRequest351I_=tradeResult351I.requestId;
        tradeSelected_[0]=tradeSelected_[1]=-1;
        // After removing the last item the former final page may not exist.
        // Use page zero on both sides and request CURRENT server revisions.
        RequestTradePreview(0,0);
        RequestTradePreview(1,0);
        summit::companion::diagnostic::Trace("CLIENT","trade-preview-refreshed",
            0,static_cast<int>(tradeRefreshedRequest351I_),1);
    }
    if(tradePreviewOpen_ && (!SelectedEntry() ||
       SelectedEntry()->name!=tradePreviewName_ || SelectedEntry()->active))
    { tradePreviewOpen_=false; tradePreviewName_.clear(); }
    if(equipmentManagerOpen_ && SelectedEntry() &&
       SelectedEntry()->name!=inventoryCacheName_)
    {
        // No cross-character inventory from a stale selection.
        equipmentManagerOpen_=false;
        dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
        inventoryCacheCount_=0;
        inventoryCacheComplete_=false;
        selectedInventoryRecordIndex_=-1;
    }
    if (equipmentManagerOpen_ && !inventoryCacheComplete_)
    {
        const auto& page = gateway_.InventorySnapshot();
        if(page.received && page.characterName==inventoryCacheName_ &&
            page.page==inventoryExpectedPage_ && page.page!=inventoryLastProcessedPage_)
        {
            if(page.status!=1) { inventoryCacheComplete_=true; }
            else
            {
                for(int i=0;i<page.itemCount && i<static_cast<int>(page.items.size()) &&
                    inventoryCacheCount_<static_cast<int>(inventoryCache_.size());++i)
                {
                    const auto& item=page.items[static_cast<std::size_t>(i)];
                    bool exists=false;
                    for(int j=0;j<inventoryCacheCount_;++j)
                        if(inventoryCache_[j].recordIndex==item.recordIndex) {exists=true;break;}
                    if(!exists) inventoryCache_[inventoryCacheCount_++]=item;
                }
                RebuildInventoryRects();
                inventoryLastProcessedPage_=page.page;
                if((page.page+1)*static_cast<int>(page.items.size())>=page.totalItems ||
                    inventoryCacheCount_==static_cast<int>(inventoryCache_.size()))
                    inventoryCacheComplete_=true;
                else
                {
                    ++inventoryExpectedPage_;
                    gateway_.Execute(CompanionUiAction::InventoryPage,inventoryCacheName_,inventoryExpectedPage_);
                }
            }
        }
    }
    RefreshControls();
    Container::Update(deltaSeconds);
}

void CompanionsPage::RebuildInventoryRects()
{
    // Reconstruct the EXACT 12x6 multi-cell geometry of InventoryPage.
    // A corrupt/duplicate DAT grid coordinate is relocated, never overdrawn.
    for(auto& rect:inventoryItemRects_) rect={};
    for(auto& page:inventoryItemPages_) page=0;
    std::array<std::array<std::array<bool,CompanionBagCapacity>,4>,2> occupied{};
    int bagOrdinal[2]={0,0};
    for(int i=0;i<inventoryCacheCount_;++i)
    {
        const auto& item=inventoryCache_[i];
        if(item.position>0 && item.position<=16)
        {
            const auto& slot=CompanionGearSlots[item.position-1];
            inventoryItemRects_[i]={slot.x+4,slot.y+4,slot.w-8,slot.h-8};
            continue;
        }
        if(item.position!=0 || item.bagNumber<1 || item.bagNumber>2) continue;
        const int bag=item.bagNumber-1;
        const int ordinal=bagOrdinal[bag]++;
        const int page=ordinal/CompanionBagCapacity;
        if(page>=4) continue;
        inventoryItemPages_[i]=page;
        int width=1,height=1;
        // Also returns the native icon's 22px-cell dimensions. Loading only
        // happens for uncached BMPs, never for unknown DAT object IDs.
        (void)CompanionItemIcon(item.code,width,height);
        width=std::clamp(width,1,CompanionBagColumns);
        height=std::clamp(height,1,CompanionBagRows);
        auto fits=[&](int x,int y)->bool {
            if(x<0 || y<0 || x+width>CompanionBagColumns || y+height>CompanionBagRows)
                return false;
            for(int yy=y;yy<y+height;++yy)
                for(int xx=x;xx<x+width;++xx)
                    if(occupied[bag][page][yy*CompanionBagColumns+xx]) return false;
            return true;
        };
        int x=item.gridX,y=item.gridY;
        if(!fits(x,y))
        {
            x=-1;
            for(int yy=0;yy+height<=CompanionBagRows && x<0;++yy)
                for(int xx=0;xx+width<=CompanionBagColumns;++xx)
                    if(fits(xx,yy)) {x=xx;y=yy;break;}
        }
        if(x<0) continue; // no overlap, no impossible rectangle
        for(int yy=y;yy<y+height;++yy)
            for(int xx=x;xx<x+width;++xx)
                occupied[bag][page][yy*CompanionBagColumns+xx]=true;
        inventoryItemRects_[i]={CompanionBagX+x*CompanionBagCell+1,
                                CompanionBagY+y*CompanionBagCell+1,
                                width*CompanionBagCell-2,height*CompanionBagCell-2};
    }
}

void CompanionsPage::RenderInventoryManager(UIRenderContext& context)
{
    const Rect root=GlobalBounds();
    auto& r=context.Renderer();
    const auto* companion=SelectedEntry();
    if(!companion) return;
    // Opaque backing ensures character page text cannot bleed through.
    r.DrawRect(root,Color{16,20,29,255});
    r.DrawTextUtf8(FontHandle{},"INVENTARIO - "+companion->name,
                   {root.x+24,root.y+16,450,24},TextStyle{UITheme::Accent,true,TextAlign::Left});
    r.DrawTextUtf8(FontHandle{},"EQUIPAMENTO",
                   {root.x+24,root.y+96,220,20},TextStyle{UITheme::TextMuted,true,TextAlign::Left});
    r.DrawTextUtf8(FontHandle{},"MOCHILA",
                   {root.x+CompanionBagX,root.y+68,220,20},
                   TextStyle{UITheme::TextMuted,true,TextAlign::Left});
    const Rect bag{root.x+CompanionBagX,root.y+CompanionBagY,
                   CompanionBagColumns*CompanionBagCell,CompanionBagRows*CompanionBagCell};
    r.DrawRect(bag,UITheme::Slot);
    for(int x=0;x<=CompanionBagColumns;++x)
        r.DrawRect({bag.x+x*CompanionBagCell,bag.y,1,bag.height},UITheme::BorderSoft);
    for(int y=0;y<=CompanionBagRows;++y)
        r.DrawRect({bag.x,bag.y+y*CompanionBagCell,bag.width,1},UITheme::BorderSoft);
    // Draw the 16 equipment slots at exactly the player InventoryPage coordinates.
    for(const auto& slot:CompanionGearSlots)
    {
        const Rect rect{root.x+slot.x,root.y+slot.y,slot.w,slot.h};
        r.DrawRect(rect,UITheme::Slot);
        CompanionBorder(r,rect,selectedEquipSlot_==slot.slot?UITheme::Accent:UITheme::BorderSoft);
        bool occupied=slot.slot==1 ? companion->rightItemCode!=0 :
                      slot.slot==2 ? companion->leftItemCode!=0 : false;
        // Physical DAT items remain equipped even when the combat-loadout
        // preference is cleared. Never draw an empty-slot label over one.
        for(int i=0;i<inventoryCacheCount_;++i)
            if(inventoryCache_[i].position==slot.slot) { occupied=true;break; }
        if(!occupied)
            r.DrawTextUtf8(FontHandle{},slot.title,
                {rect.x+2,rect.y+rect.height-18,rect.width-4,16},
                TextStyle{UITheme::TextMuted,false,TextAlign::Center});
    }
    // Server pages (10 records) are assembled into a complete local view.
    // The user's bag tabs are NOT transport pages.
    const auto& received=gateway_.InventorySnapshot();
    if(!inventoryCacheComplete_)
        r.DrawTextUtf8(FontHandle{},"A carregar itens do DAT...",
            {bag.x+8,bag.y+9,bag.width-16,24},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
    else if(received.status!=1 || inventoryCacheCount_==0)
        r.DrawTextUtf8(FontHandle{},received.status!=1 ? "Falha ao ler DAT do Companion." : "Inventario vazio.",
            {bag.x+8,bag.y+9,bag.width-16,24},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
    for(int i=0;i<inventoryCacheCount_;++i)
    {
        const auto& item=inventoryCache_[i];
        // Hand slot is a VIEW of the active combat loadout. When it differs
        // from the physical DAT position, show the DAT item as a labelled
        // thumbnail below, not as the currently equipped combat weapon.
        if((item.position==1 && item.code!=companion->rightItemCode) ||
           (item.position==2 && item.code!=companion->leftItemCode)) continue;
        if(item.position==0 && (item.bagNumber!=activeCompanionBag_ ||
                inventoryItemPages_[i]!=inventoryDisplayPage_)) continue;
        const Rect localRect=inventoryItemRects_[i];
        const Rect rect{root.x+localRect.x,root.y+localRect.y,localRect.width,localRect.height};
        int iconW=1,iconH=1;
        const TextureHandle texture=CompanionItemIcon(item.code,iconW,iconH);
        if(rect.IsEmpty()) continue;
        if(texture.IsValid()) r.DrawImage(texture,rect,nullptr);
        else
        {
            // Deliberately no letter-placeholder on top of grid cells.
            r.DrawRect(rect,UITheme::SurfacePressed);
            CompanionBorder(r,rect,UITheme::BorderSoft);
        }
        if(item.recordIndex==selectedInventoryRecordIndex_)
            CompanionBorder(r,rect,UITheme::Accent);
    }
    // Draw the EFFECTIVE server-authoritative combat loadout in the main hand
    // slots. The DAT remains unchanged by hand selection. If the physical DAT
    // item is different, keep it visible as a separate, labelled thumbnail.
    const std::uint32_t chosenCodes[2]={companion->rightItemCode,companion->leftItemCode};
    for(int hand=0;hand<2;++hand)
    {
        std::uint32_t physical=0;
        for(int i=0;i<inventoryCacheCount_;++i)
            if(inventoryCache_[i].position==hand+1) {
                physical=inventoryCache_[i].code; break;
            }
        if(chosenCodes[hand] == physical) continue;
        const auto& slot=CompanionGearSlots[hand];
        const Rect activeRect{root.x+slot.x+4,root.y+slot.y+4,slot.w-8,slot.h-8};
        if(chosenCodes[hand])
        {
            int w=1,h=1;
            const TextureHandle active=CompanionItemIcon(chosenCodes[hand],w,h);
            if(active.IsValid()) r.DrawImage(active,activeRect,nullptr);
            else r.DrawTextUtf8(FontHandle{},"Icone nao encontrado",
                {activeRect.x,activeRect.y+8,activeRect.width,45},
                TextStyle{UITheme::TextMuted,false,TextAlign::Center});
        }
        else
            r.DrawTextUtf8(FontHandle{},"Vazio (loadout)",
                {activeRect.x,activeRect.y+14,activeRect.width,36},
                TextStyle{UITheme::TextMuted,false,TextAlign::Center});
        if(physical)
        {
            int w=1,h=1;
            const TextureHandle saved=CompanionItemIcon(physical,w,h);
            const Rect thumbnail{root.x+slot.x+2,root.y+slot.y+slot.h-29,22,22};
            r.DrawRect(thumbnail,UITheme::Surface);
            if(saved.IsValid()) r.DrawImage(saved,thumbnail,nullptr);
            CompanionBorder(r,thumbnail,UITheme::BorderSoft);
            r.DrawTextUtf8(FontHandle{},"DAT",
                {thumbnail.x+23,thumbnail.y+3,28,15},
                TextStyle{UITheme::TextMuted,true,TextAlign::Left});
        }
    }
    const Rect details{root.x+CompanionBagX,root.y+292,360,117};
    r.DrawRect(details,UITheme::Surface);
    CompanionBorder(r,details,UITheme::BorderSoft);
    r.DrawTextUtf8(FontHandle{},"DETALHES DO ITEM",
                   {details.x+12,details.y+8,details.width-24,22},
                   TextStyle{UITheme::TextMuted,true,TextAlign::Left});
    bool selectedItem=false;
    for(int i=0;i<inventoryCacheCount_;++i)
    {
        const auto& item=inventoryCache_[i];
        if(item.recordIndex!=selectedInventoryRecordIndex_) continue;
        selectedItem=true;
        r.DrawTextUtf8(FontHandle{},item.name,
            {details.x+12,details.y+34,details.width-24,22},TextStyle{UITheme::Accent,true,TextAlign::Left});
        r.DrawTextUtf8(FontHandle{},std::string(CompanionSlotLabel(item.position))+
            "  |  Registo: "+std::to_string(item.recordIndex),
            {details.x+12,details.y+59,details.width-24,18},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
        break;
    }
    if(!selectedItem)
        r.DrawTextUtf8(FontHandle{},"Seleciona um item para ver o nome e equipar.",
            {details.x+12,details.y+41,details.width-24,25},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
    r.DrawTextUtf8(FontHandle{},"Maos: imagem grande = combate; mini DAT = item fisico. Retirar nao move o DAT.",
        {details.x+12,details.y+88,details.width-24,16},
        TextStyle{UITheme::TextMuted,false,TextAlign::Left});
    r.DrawTextUtf8(FontHandle{},companion->active ? "Dispensa o Companion antes de gravar equipamento no DAT."
            : "Clica num item e no slot. Clica num slot sem selecao para desequipar.",
            {root.x+24,root.y+452,690,17},TextStyle{UITheme::TextMuted,false,TextAlign::Left});
    if(dragActive_ && dragCandidateIndex_ >= 0)
    {
        for(int i=0;i<inventoryCacheCount_;++i)
        {
            const auto& moving=inventoryCache_[i];
            if(moving.recordIndex!=dragCandidateIndex_) continue;
            int w=1,h=1;
            const TextureHandle icon=CompanionItemIcon(moving.code,w,h);
            const Rect ghost{dragPointer_.x-14,dragPointer_.y-14,28,28};
            if(icon.IsValid()) r.DrawImage(icon,ghost,nullptr);
            else CompanionBorder(r,ghost,UITheme::Accent);
            break;
        }
        r.DrawTextUtf8(FontHandle{},"Solta na mochila ou num slot (Esc cancela).",
            {root.x+24,root.y+452,520,17},TextStyle{UITheme::Accent,false,TextAlign::Left});
    }
    // Independent from snapshot.lastResult: subsequent refreshes cannot erase errors.
    RenderCompanionErrorBox(r,root);
}

UIEventResult CompanionsPage::OnEvent(const UIEvent& event)
{
    if(event.type==UIEventType::PointerDown && event.button==MouseButton::Left &&
       summit::companion::diagnostic::Errors().count &&
       CompanionErrorBox(GlobalBounds()).Contains(event.pointer))
    {
        summit::companion::diagnostic::Trace("CLIENT","errors-acknowledged",0,0,0);
        summit::companion::diagnostic::Errors().DismissLatest();
        return UIEventResult::Handled;
    }
    // Drag from the companion bag onto an equipment slot. No optimistic DAT
    // mutation: send exactly the same validated command as click-to-equip.
    if(equipmentManagerOpen_ && dragCandidateIndex_!=-1 &&
       event.type==UIEventType::KeyDown && event.key==27)
    {
        dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
        return UIEventResult::Handled;
    }
    if(equipmentManagerOpen_ && dragCandidateIndex_!=-1 &&
       event.type==UIEventType::PointerMove)
    {
        dragPointer_=event.pointer;
        const int dx=event.pointer.x-dragStart_.x;
        const int dy=event.pointer.y-dragStart_.y;
        if(dx*dx+dy*dy>=36) dragActive_=true;
        return UIEventResult::Handled;
    }
    if(equipmentManagerOpen_ && dragCandidateIndex_!=-1 &&
       event.type==UIEventType::PointerUp && event.button==MouseButton::Left)
    {
        const int movingIndex=dragCandidateIndex_;
        const bool wasDragging=dragActive_;
        const int originEquipSlot=dragEquipmentSlot_;
        dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
        if(originEquipSlot && SelectedEntry() &&
           SelectedEntry()->name==inventoryCacheName_ && inventoryCacheComplete_)
        {
            const Rect root=GlobalBounds();
            const Point local{event.pointer.x-root.x,event.pointer.y-root.y};
            const Rect bag{CompanionBagX,CompanionBagY,
                CompanionBagColumns*CompanionBagCell,CompanionBagRows*CompanionBagCell};
            if(!wasDragging || bag.Contains(local))
            {
                if(originEquipSlot==1 || originEquipSlot==2)
                    ExecuteForSelected(originEquipSlot==1 ? CompanionUiAction::EquipInventoryRight :
                        CompanionUiAction::EquipInventoryLeft, -1);
                else if(!SelectedEntry()->active)
                {
                    ExecuteForSelected(CompanionUiAction::SetInventorySlot,
                        (originEquipSlot<<16)|0xffff);
                    inventoryCacheCount_=0; inventoryExpectedPage_=0;
                    inventoryLastProcessedPage_=-1; inventoryCacheComplete_=false;
                    selectedInventoryRecordIndex_=-1;
                    gateway_.Execute(CompanionUiAction::InventoryPage,SelectedEntry()->name,0);
                }
            }
        }
        else if(wasDragging && SelectedEntry() &&
                SelectedEntry()->name==inventoryCacheName_ && inventoryCacheComplete_)
        {
            // The visual Equip buttons are valid drop targets, too. This
            // matches the player's inventory affordance without copying items.
            for(int i=0;i<inventoryCacheCount_;++i)
            {
                const auto& item=inventoryCache_[i];
                if(item.recordIndex!=movingIndex) continue;
                if(inventoryEquipRightButton_->HitTest(event.pointer) &&
                   item.bagNumber==1 && item.position!=-1)
                {
                    ExecuteForSelected(CompanionUiAction::EquipInventoryRight,
                        item.code==SelectedEntry()->rightItemCode?-1:movingIndex);
                    return UIEventResult::Handled;
                }
                if(inventoryEquipLeftButton_->HitTest(event.pointer) &&
                   item.bagNumber==1 && item.position!=-1)
                {
                    ExecuteForSelected(CompanionUiAction::EquipInventoryLeft,
                        item.code==SelectedEntry()->leftItemCode?-1:movingIndex);
                    return UIEventResult::Handled;
                }
                break;
            }
            const Rect root=GlobalBounds();
            const Point local{event.pointer.x-root.x,event.pointer.y-root.y};
            for(const auto& slot:CompanionGearSlots)
            {
                if(!Rect{slot.x,slot.y,slot.w,slot.h}.Contains(local)) continue;
                for(int i=0;i<inventoryCacheCount_;++i)
                {
                    const auto& item=inventoryCache_[i];
                    if(item.recordIndex!=movingIndex || item.position!=0 ||
                       item.bagNumber!=1) continue;
                    // The server owns eligibility. Cached equipMask may be
                    // incomplete for custom content; never suppress a valid
                    // physical equipment request before it reaches V2.
                    // No client-side bypass for active Companions or bad slots.
                    if(slot.slot>=3 && slot.slot!=11 && slot.slot!=12 && slot.slot!=13)
                    {
                        if(SelectedEntry()->active) break;
                        ExecuteForSelected(CompanionUiAction::SetInventorySlot,
                            (slot.slot<<16)|(movingIndex&0xffff));
                        inventoryCacheCount_=0;
                        inventoryExpectedPage_=0;
                        inventoryLastProcessedPage_=-1;
                        inventoryCacheComplete_=false;
                        selectedInventoryRecordIndex_=-1;
                        gateway_.Execute(CompanionUiAction::InventoryPage,SelectedEntry()->name,0);
                    }
                    else if(slot.slot==1 || slot.slot==2)
                        ExecuteForSelected(slot.slot==1?CompanionUiAction::EquipInventoryRight:
                            CompanionUiAction::EquipInventoryLeft,movingIndex);
                    break;
                }
                break;
            }
            // Within-bag drag: server validates footprint, boundaries and all
            // collisions before changing any DAT. No optimistic UI movement.
            const Rect bagGrid{CompanionBagX,CompanionBagY,
                CompanionBagColumns*CompanionBagCell,CompanionBagRows*CompanionBagCell};
            if(bagGrid.Contains(local) && inventoryDisplayPage_==0 &&
               movingIndex>=0 && movingIndex<256 && !SelectedEntry()->active)
            {
                for(int i=0;i<inventoryCacheCount_;++i)
                {
                    const auto& item=inventoryCache_[i];
                    if(item.recordIndex!=movingIndex || item.position!=0 ||
                       item.bagNumber!=activeCompanionBag_) continue;
                    const int x=(local.x-CompanionBagX)/CompanionBagCell;
                    const int y=(local.y-CompanionBagY)/CompanionBagCell;
                    const int packed=movingIndex | (x<<8) | (y<<12) |
                        (activeCompanionBag_<<15);
                    ExecuteForSelected(CompanionUiAction::MoveBagCell,packed);
                    inventoryCacheCount_=0;
                    inventoryExpectedPage_=0;
                    inventoryLastProcessedPage_=-1;
                    inventoryCacheComplete_=false;
                    selectedInventoryRecordIndex_=-1;
                    gateway_.Execute(CompanionUiAction::InventoryPage,SelectedEntry()->name,0);
                    break;
                }
            }
        }
        return UIEventResult::Handled;
    }
    // Standard clickable controls (tabs, Refresh, Close) first.
    const UIEventResult child=Container::OnEvent(event);
    if(child!=UIEventResult::Ignored) return child;
    if(tradePreviewOpen_) return GlobalBounds().Contains(event.pointer)?UIEventResult::Handled:UIEventResult::Ignored;
    if(!equipmentManagerOpen_ || event.type!=UIEventType::PointerDown ||
        event.button!=MouseButton::Left) return child;
    dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
    const auto* companion=SelectedEntry();
    if(!companion) return child;
    const Rect root=GlobalBounds();
    const Point pointer{event.pointer.x-root.x,event.pointer.y-root.y};
    for(const auto& slot:CompanionGearSlots)
    {
        const Rect slotRect{slot.x,slot.y,slot.w,slot.h};
        if(!slotRect.Contains(pointer)) continue;
        selectedEquipSlot_=slot.slot;
        if(selectedInventoryRecordIndex_<0 && inventoryCacheComplete_ &&
           ((slot.slot==1 && companion->rightItemCode) ||
            (slot.slot==2 && companion->leftItemCode)))
        {
            dragCandidateIndex_= -2; // sentinel: the effective hand loadout
            dragEquipmentSlot_=slot.slot;
            dragStart_=event.pointer; dragPointer_=event.pointer;
            return UIEventResult::Handled;
        }
        if(selectedInventoryRecordIndex_<0 && !companion->active &&
           slot.slot>=3 && slot.slot!=11 && slot.slot!=12 && slot.slot!=13 &&
           inventoryCacheComplete_)
        {
            for(int i=0;i<inventoryCacheCount_;++i)
            {
                if(inventoryCache_[i].position!=slot.slot) continue;
                dragCandidateIndex_=inventoryCache_[i].recordIndex;
                dragEquipmentSlot_=slot.slot;
                dragActive_=false;
                dragStart_=event.pointer;dragPointer_=event.pointer;
                return UIEventResult::Handled;
            }
        }
        bool allowed=false;
        for(int i=0;i<inventoryCacheCount_;++i)
            if(inventoryCache_[i].recordIndex==selectedInventoryRecordIndex_ &&
               inventoryCache_[i].position==0 && inventoryCache_[i].bagNumber==1 &&
               ((slot.slot>=3 && slot.slot<=16 &&
                 slot.slot!=11 && slot.slot!=12 && slot.slot!=13) ||
                ((slot.slot==1 || slot.slot==2) &&
                 inventoryCache_[i].position!=-1)))
                allowed=true;
        bool editingDat=false;
        if(allowed && slot.slot>=3 && slot.slot!=11 && slot.slot!=12 && slot.slot!=13)
        {
            ExecuteForSelected(CompanionUiAction::SetInventorySlot,
                (slot.slot<<16)|(selectedInventoryRecordIndex_&0xffff));
            editingDat=true;
        }
        else if(allowed && (slot.slot==1 || slot.slot==2))
        {
            bool sameAsEquipped=false;
            for(int i=0;i<inventoryCacheCount_;++i)
            {
                if(inventoryCache_[i].recordIndex!=selectedInventoryRecordIndex_) continue;
                const auto& item=inventoryCache_[i];
                sameAsEquipped = item.code ==
                    (slot.slot==1 ? companion->rightItemCode : companion->leftItemCode);
                break;
            }
            ExecuteForSelected(slot.slot==1 ? CompanionUiAction::EquipInventoryRight :
                CompanionUiAction::EquipInventoryLeft,
                sameAsEquipped ? -1 : selectedInventoryRecordIndex_);
        }
        else if(selectedInventoryRecordIndex_<0 && slot.slot>=3 &&
                slot.slot!=11 && slot.slot!=12 && slot.slot!=13)
        {
            ExecuteForSelected(CompanionUiAction::SetInventorySlot,(slot.slot<<16)|0xffff);
            editingDat=true;
        }
        if(editingDat)
        {
            // Next authoritative response repopulates the view after mutation.
            dragCandidateIndex_=-1; dragActive_=false; dragEquipmentSlot_=0;
            inventoryCacheCount_=0;
            inventoryExpectedPage_=0;
            inventoryLastProcessedPage_=-1;
            inventoryCacheComplete_=false;
            selectedInventoryRecordIndex_=-1;
            gateway_.Execute(CompanionUiAction::InventoryPage,companion->name,0);
        }
        return UIEventResult::Handled;
    }
    const Rect bag{CompanionBagX,CompanionBagY,
        CompanionBagColumns*CompanionBagCell,CompanionBagRows*CompanionBagCell};
    if(bag.Contains(pointer))
    {
        for(int i=0;i<inventoryCacheCount_;++i)
        {
            const auto& item=inventoryCache_[i];
            if(item.position!=0 || item.bagNumber!=activeCompanionBag_ ||
                inventoryItemPages_[i]!=inventoryDisplayPage_) continue;
            const Rect& itemRect=inventoryItemRects_[i];
            if(itemRect.Contains(pointer))
            {
                selectedInventoryRecordIndex_=item.recordIndex;
                dragCandidateIndex_=item.recordIndex;
                dragEquipmentSlot_=0;
                dragActive_=false;
                dragStart_=event.pointer;
                dragPointer_=event.pointer;
                RefreshControls();
                return UIEventResult::Handled;
            }
        }
        // Empty bag cell deselects; the hand buttons then offer "Retirar".
        selectedInventoryRecordIndex_=-1;
        RefreshControls();
        return UIEventResult::Handled;
    }
    return root.Contains(event.pointer)?UIEventResult::Handled:UIEventResult::Ignored;
}

void CompanionsPage::OnRender(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    r.DrawRect(root, UITheme::WindowBg);

    const auto& snapshot = gateway_.Snapshot();
    // Modal replaces underlying page instead of translucent overpainting it.
    if(tradePreviewOpen_ && SelectedEntry()) { RenderTradePreview(context); return; }
    if(equipmentManagerOpen_ && SelectedEntry()) { RenderInventoryManager(context); return; }

    r.DrawTextUtf8(FontHandle{}, "COMPANIONS DA CONTA", {root.x + 22, root.y + 20, 300, 24},
                   TextStyle{UITheme::Text, true, TextAlign::Left});

    if (!snapshot.valid)
    {
        r.DrawTextUtf8(FontHandle{}, "Aguardando estado do servidor...", {root.x + 22, root.y + 48, 360, 22},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        return;
    }

    r.DrawTextUtf8(FontHandle{}, "Ativos: " + std::to_string(snapshot.activeCount) + "/" + std::to_string(snapshot.maxActive),
                   {root.x + root.width - 220, root.y + 20, 190, 24},
                   TextStyle{UITheme::Accent, true, TextAlign::Right});

    r.DrawTextUtf8(FontHandle{}, "Personagens", {root.x + 22, root.y + 50, 236, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, "Acao", {root.x + 282, root.y + 50, 180, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, "Comportamento", {root.x + 282, root.y + 112, 260, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});

    const CompanionUiEntry* selected = SelectedEntry();
    // PATCH 213: the custom-tactics grid owns the detail area. Do not leave
    // the behavior summary visually underneath its controls.
    if (selected && !tacticsView_)
    {
        r.DrawTextUtf8(FontHandle{}, std::string("Tatica: ") + TacticName(selected->tactic),
                       {root.x + 330, root.y + 176, 350, 22},
                       TextStyle{UITheme::Text, false, TextAlign::Left});
        r.DrawTextUtf8(FontHandle{}, "Recuar HP: " + std::to_string(selected->retreatHpPercent) + "%",
                       {root.x + 326, root.y + 212, 120, 20},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        const std::string rangeText = selected->preferredRange > 0 ?
            std::to_string(selected->preferredRange) : std::string("auto");
        r.DrawTextUtf8(FontHandle{}, "Range: " + rangeText,
                       {root.x + 540, root.y + 212, 144, 20},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    }

    const int detailY = root.y + 246;
    r.DrawRect({root.x + 274, detailY, root.width - 298, 192}, UITheme::Surface);

    if (tacticsView_)
    {
        // The mode buttons and Custom ON/OFF already communicate the view.
        // Do not render implementation guidance in the control row: it looked
        // like stray text under the tabs and collided with localized labels.
        if(!selected)
        {
            r.DrawTextUtf8(FontHandle{}, "Selecione outro personagem da conta.",
                           {root.x + 292, detailY + 72, 390, 24},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        }
    }
    else if (skillsView_)
    {
        if (!buffSkillsView_)
        {
            r.DrawTextUtf8(FontHandle{}, "SKILLS ATIVAS", {root.x + 292, detailY + 8, 220, 20},
                           TextStyle{UITheme::TextMuted, true, TextAlign::Left});
            const std::string activeMode = snapshot.skillLoadoutActive ? "CUSTOM" : "AUTO V1";
            r.DrawTextUtf8(FontHandle{}, activeMode, {root.x + 600, detailY + 8, 104, 20},
                           TextStyle{snapshot.skillLoadoutActive ? UITheme::Accent : UITheme::TextMuted, true, TextAlign::Right});

            if (selected)
            {
                for (std::size_t slot = 0; slot < 4; ++slot)
                {
                    const int y = detailY + 46 + static_cast<int>(slot) * 29;
                    const auto& active = snapshot.activeSkills[slot];
                    r.DrawTextUtf8(FontHandle{},
                                   std::to_string(slot + 1) + ". " + SkillDisplay(active),
                                   {root.x + 330, y, 218, 22},
                                   TextStyle{UITheme::Text, false, TextAlign::Left});
                }
            }
            else
            {
                r.DrawTextUtf8(FontHandle{}, "Selecione outro personagem da conta.",
                               {root.x + 292, detailY + 72, 390, 24},
                               TextStyle{UITheme::TextMuted, false, TextAlign::Left});
            }
        }
        else
        {
            r.DrawTextUtf8(FontHandle{}, "SUPORTE / RECAST", {root.x + 292, detailY + 8, 240, 20},
                           TextStyle{UITheme::TextMuted, true, TextAlign::Left});
            const std::string buffMode = snapshot.buffLoadoutActive ? "ON" : "OFF";
            r.DrawTextUtf8(FontHandle{}, buffMode, {root.x + 600, detailY + 8, 104, 20},
                           TextStyle{snapshot.buffLoadoutActive ? UITheme::Accent : UITheme::TextMuted, true, TextAlign::Right});

            if (selected)
            {
                for (std::size_t slot = 0; slot < 4; ++slot)
                {
                    const int y = detailY + 46 + static_cast<int>(slot) * 29;
                    const auto& buff = snapshot.buffSkills[slot];
                    r.DrawTextUtf8(FontHandle{},
                                   std::to_string(slot + 1) + ". " + SkillDisplay(buff),
                                   {root.x + 330, y, 226, 22},
                                   TextStyle{UITheme::Text, false, TextAlign::Left});

                    const std::string interval = (buff.useSlot && buff.intervalSeconds > 0)
                        ? std::to_string(buff.intervalSeconds) + "s"
                        : std::string("--");
                    r.DrawTextUtf8(FontHandle{}, interval,
                                   {root.x + 602, y, 44, 22},
                                   TextStyle{UITheme::Accent, false, TextAlign::Right});
                }
            }
            else
            {
                r.DrawTextUtf8(FontHandle{}, "Selecione outro personagem da conta.",
                               {root.x + 292, detailY + 72, 390, 24},
                               TextStyle{UITheme::TextMuted, false, TextAlign::Left});
            }
        }
    }
    else
    {
        r.DrawTextUtf8(FontHandle{}, "EQUIPAMENTO DO COMPANION", {root.x + 292, detailY + 12, 300, 20},
                       TextStyle{UITheme::TextMuted, true, TextAlign::Left});
        if (selected)
        {
            r.DrawTextUtf8(FontHandle{}, selected->name + " - Lv " + std::to_string(selected->level),
                           {root.x + 292, detailY + 34, 280, 22},
                           TextStyle{UITheme::Text, true, TextAlign::Left});

            std::string status = selected->active ? (selected->dead ? "Morto" : "Ativo") : "Offline";
            if (selected->maxLife > 0)
                status += "  HP " + std::to_string(selected->life) + "/" + std::to_string(selected->maxLife);
            if (selected->maxMana > 0)
                status += "  MP " + std::to_string(selected->mana) + "/" + std::to_string(selected->maxMana);
            r.DrawTextUtf8(FontHandle{}, status, {root.x + 292, detailY + 56, 390, 20},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});

            if (selected->active && !selected->targetName.empty())
                r.DrawTextUtf8(FontHandle{}, "Alvo: " + selected->targetName, {root.x + 292, detailY + 76, 390, 20},
                               TextStyle{UITheme::Accent, false, TextAlign::Left});

            r.DrawTextUtf8(FontHandle{}, "Mao direita: " + ItemDisplay(selected->rightItemName, selected->rightItemCode),
                           {root.x + 330, detailY + 80, 350, 24},
                           TextStyle{UITheme::Text, false, TextAlign::Left});
            r.DrawTextUtf8(FontHandle{}, "Mao esquerda: " + ItemDisplay(selected->leftItemName, selected->leftItemCode),
                           {root.x + 330, detailY + 112, 350, 24},
                           TextStyle{UITheme::Text, false, TextAlign::Left});
        }
        else
        {
            r.DrawTextUtf8(FontHandle{}, "Selecione outro personagem da conta.",
                           {root.x + 292, detailY + 52, 390, 24},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        }
    }


    RenderCompanionErrorBox(r,root);
}
}
