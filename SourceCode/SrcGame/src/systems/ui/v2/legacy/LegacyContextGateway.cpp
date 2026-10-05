#include "systems/ui/v2/legacy/LegacyContextGateway.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/network/client/netplay.h"
#include "systems/commerce/restoration/cRestaureWindow.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"
#include "systems/ui/v2/legacy/LegacyInventoryGateway.h"
#include "systems/ui/messages/legacy_sin/sinMsg.h"

// A few UI-state globals live in the legacy .cpp files and were never exposed
// through headers. Keep them confined to this compatibility adapter.
extern int SelectShopButtonPosi;
extern int ShowShopItemIndex;
extern int ShopArrowPosi;
extern int CraftItemButtonIndex;
extern int TradeCheckFlag;
extern int CloseWareHouseFlag;
extern int SelectMyShopItemIndex;
extern int SelectMyShopButtonPosi;
extern int MyShopItemIndex[100];
extern int uiV2ConfirmedMerchantSale;
extern int uiV2ConfirmedMerchantSaleResult;
extern int sinMoveKindInter[MAX_SIN_KIND_INTER];
extern POINT AgingLevelSheltomXY[4][3];
extern int AgingLevelSheltomIndex[4][3];

namespace ui::v2::legacy
{
namespace
{
ContextItemSnapshot MakeItem(int index, const sITEM& item, bool interactive = true,
                             bool remote = false, const char* caption = nullptr)
{
    ContextItemSnapshot out;
    out.index = index;
    out.name = item.sItemInfo.ItemName[0] ? item.sItemInfo.ItemName : item.ItemNameIndex;
    out.caption = caption ? caption : "";
    out.price = 0;
    out.interactive = interactive;
    out.remote = remote;
    if (item.lpItem)
        out.texture = TextureHandle(reinterpret_cast<std::uintptr_t>(item.lpItem));
    return out;
}

void AddAction(ContextSnapshot& s, int id, const char* label, bool enabled = true, bool selected = false)
{
    s.actions.push_back(ContextActionSnapshot{id, label ? label : "", enabled, selected});
}

bool HasItem(const sITEM& item) { return item.Flag != 0; }

bool InventoryHasRoomFor(const sITEM& item)
{
    sITEM copy = item;
    return cInvenTory.CheckInvenEmpty(&copy) != 0;
}

class OffscreenCursorGuard
{
public:
    OffscreenCursorGuard() : old_(pCursorPos)
    {
        pCursorPos.x = -10000;
        pCursorPos.y = -10000;
    }
    ~OffscreenCursorGuard() { pCursorPos = old_; }

private:
    POINT old_{};
};

bool PutMouseItemBackInInventory()
{
    if (!MouseItem.Flag)
        return true;
    if (cInvenTory.AutoSetInvenItem(&MouseItem))
    {
        cInvenTory.ReFormInvenItem();
        return true;
    }
    cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
    return false;
}

bool IsClassCompatible(const sITEM& item)
{
    if (!sinChar)
        return true;
    sITEM copy = item;
    copy.sItemInfo.NotUseFlag = 0;
    cInvenTory.CharOnlySetItem(&copy);
    return copy.sItemInfo.NotUseFlag == 0;
}

bool MeetsStatRequirements(const sITEM& item)
{
    if (!sinChar)
        return true;
    const sITEMINFO& info = item.sItemInfo;
    return info.Level <= sinChar->Level &&
           info.Strength <= sinChar->Strength &&
           info.Spirit <= sinChar->Spirit &&
           info.Talent <= sinChar->Talent &&
           info.Dexterity <= sinChar->Dexterity &&
           info.Health <= sinChar->Health;
}

std::string RequirementSummary(const sITEM& item)
{
    if (!sinChar)
        return {};
    const sITEMINFO& info = item.sItemInfo;
    std::string text;
    auto append = [&text](const std::string& part)
    {
        if (!text.empty()) text += ", ";
        text += part;
    };
    if (info.Level > sinChar->Level) append("Nivel " + std::to_string(info.Level));
    if (info.Strength > sinChar->Strength) append("Forca " + std::to_string(info.Strength));
    if (info.Spirit > sinChar->Spirit) append("Espirito " + std::to_string(info.Spirit));
    if (info.Talent > sinChar->Talent) append("Talento " + std::to_string(info.Talent));
    if (info.Dexterity > sinChar->Dexterity) append("Destreza " + std::to_string(info.Dexterity));
    if (info.Health > sinChar->Health) append("Vida " + std::to_string(info.Health));
    return text;
}

void ApplyMerchantEligibility(ContextItemSnapshot& entry, const sITEM& item)
{
    entry.classCompatible = IsClassCompatible(item);
    entry.requirementsMet = MeetsStatRequirements(item);
    if (!entry.classCompatible)
        entry.restriction = "INCOMPATIVEL com sua classe/personagem";
    else if (!entry.requirementsMet)
        entry.restriction = "Requer: " + RequirementSummary(item);
}

void ConfigureRecipeSlot(ContextItemSnapshot& entry, int index, const sITEM& item)
{
    // Preserve the spatial language of the original Craft/Aging window:
    // item on the left, a 3x4 sheltom field on the right and the optional
    // special stone between the two areas near the bottom.
    if (index == 0)
    {
        const int gw = std::clamp((item.w + 21) / 22, 1, 3);
        const int gh = std::clamp((item.h + 21) / 22, 1, 4);
        entry.gridX = (3 - gw) / 2;
        entry.gridY = (4 - gh) / 2;
        entry.gridWidth = gw;
        entry.gridHeight = gh;
        entry.caption = "Item principal";
        return;
    }
    if (index >= 1 && index <= 12)
    {
        const int n = index - 1;
        entry.gridX = 5 + (n % 3);
        entry.gridY = n / 3;
        entry.gridWidth = 1;
        entry.gridHeight = 1;
        entry.caption = "Pedra " + std::to_string(index);
        return;
    }
    if (index == 14)
    {
        entry.gridX = 3;
        entry.gridY = 3;
        entry.gridWidth = 1;
        entry.gridHeight = 1;
        entry.caption = "Pedra especial";
    }
}

void AddRecipeSlots(ContextSnapshot& out, bool aging)
{
    out.slots.clear();

    ContextSlotSnapshot main;
    main.gridX = 0;
    main.gridY = 0;
    main.gridWidth = 3;
    main.gridHeight = 4;
    main.label = "Item";
    out.slots.push_back(std::move(main));

    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            ContextSlotSnapshot stone;
            stone.gridX = 5 + col;
            stone.gridY = row;
            stone.gridWidth = 1;
            stone.gridHeight = 1;
            stone.label = "Pedra";
            if (aging)
            {
                const int hint = AgingLevelSheltomIndex[row][col];
                stone.required = hint > 0;
                if (hint > 0 && hint <= MAX_SHELTOM && cAging.lpGraySheltom[hint - 1])
                    stone.hintTexture = TextureHandle(reinterpret_cast<std::uintptr_t>(cAging.lpGraySheltom[hint - 1]));
            }
            out.slots.push_back(std::move(stone));
        }
    }

    ContextSlotSnapshot special;
    special.gridX = 3;
    special.gridY = 3;
    special.gridWidth = 1;
    special.gridHeight = 1;
    special.label = "Especial";
    special.special = true;
    out.slots.push_back(std::move(special));
}

const char* CoinSectionName(int section)
{
    switch (section)
    {
        case 1: return "Armas";
        case 2: return "Defesas";
        case 3: return "Trajes";
        case 4: return "Acessorios";
        case 5: return "Sheltoms";
        case 6: return "Itens premium";
        case 7: return "Servicos";
        default: return "Catalogo";
    }
}
}

ContextSnapshot LegacyContextGateway::GetSnapshot() const
{
    ContextSnapshot out;
    if (!sinChar)
        return out;

    out.money = sinChar->Money;

    if (cShop.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::Merchant;
        out.title = "Mercador";
        out.subtitle = "Compre na lista; use os modos para reparar ou vender itens da mochila.";
        out.statusText = "Vermelho: classe/personagem incompativel | Amarelo: requisitos ainda nao atendidos";
        out.items.reserve(30);
        for (int i = 0; i < 30; ++i)
        {
            if (!HasItem(cShop.ShowShopItem[i]))
                continue;
            ContextItemSnapshot entry = MakeItem(i, cShop.ShowShopItem[i]);
            entry.price = cShop.ShowShopItem[i].sItemInfo.Price;
            ApplyMerchantEligibility(entry, cShop.ShowShopItem[i]);
            out.items.push_back(std::move(entry));
        }
        AddAction(out, 10, "Reparar tudo", sinShopKind != 3);
        AddAction(out, 11, "Reparar", sinShopKind != 3, CursorClass == SIN_CURSOR_REPAIR);
        AddAction(out, 12, "Vender", true, CursorClass == SIN_CURSOR_SELL);
        AddAction(out, 13, "Comprar", true, CursorClass == SIN_CURSOR_BUY || CursorClass == 1);
        AddAction(out, 14, "Armas", sinShopKind != 3, sinShopKind == 1);
        AddAction(out, 15, "Defesa", sinShopKind != 3, sinShopKind == 2);
        return out;
    }

    if (cMyShop.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::PersonalShop;
        out.title = MyShopSendButton ? "Minha loja - vendendo" : "Minha loja";
        out.subtitle = MyShopSendButton
            ? "A loja esta publicada. Encerre as vendas para editar os itens."
            : "Botao direito na mochila adiciona um item e abre a definicao de preco.";
        bool hasItems = false;
        for (int i = 0; i < 30; ++i)
        {
            if (!HasItem(cMyShop.MyShopItem[i]))
                continue;
            hasItems = true;
            std::string price;
            if (cMyShop.MyShopItem[i].CoinPrice > 0)
                price = std::to_string(cMyShop.MyShopItem[i].CoinPrice) + " coin";
            else if (cMyShop.MyShopItem[i].sItemInfo.Money > 0)
                price = std::to_string(cMyShop.MyShopItem[i].sItemInfo.Money) + " gold";
            else
                price = "preco pendente";
            out.items.push_back(MakeItem(i, cMyShop.MyShopItem[i], !MyShopSendButton, false, price.c_str()));
        }
        if (MyShopSendButton)
            AddAction(out, 60, "Encerrar vendas");
        else
            AddAction(out, 61, "Abrir loja", hasItems);
        AddAction(out, 62, "Fechar painel");
        return out;
    }

    if (cCharShop.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::PlayerMerchant;
        out.title = "Loja de jogador";
        out.subtitle = "Selecione um item para comprar. A confirmacao e as validacoes continuam no sistema original.";
        for (int i = 0; i < 30; ++i)
        {
            if (!HasItem(cCharShop.CharShopItem[i]))
                continue;
            std::string price;
            if (sRecvMyShop.sMyShopItem[i].Price > 0)
                price = std::to_string(sRecvMyShop.sMyShopItem[i].Price) + " gold";
            if (sRecvMyShop.sMyShopItem[i].dwTemp[0] > 0)
            {
                if (!price.empty()) price += " + ";
                price += std::to_string(sRecvMyShop.sMyShopItem[i].dwTemp[0]) + " coin";
            }
            out.items.push_back(MakeItem(i, cCharShop.CharShopItem[i], true, false, price.c_str()));
        }
        AddAction(out, 16, "Fechar loja");
        return out;
    }

    if (cCoinShop.isOpen())
    {
        out.valid = true;
        out.kind = ContextKind::CoinShop;
        out.title = "Loja de Moedas";
        out.money = cCoinShop.Coin;
        out.currency = "SCoin";
        const int section = cCoinShop.GetSectionForUI();
        const int page = cCoinShop.GetPageForUI();
        const int pages = std::max(1, cCoinShop.GetPageCountForUI());
        out.subtitle = std::string(CoinSectionName(section)) + " - pagina " +
                       std::to_string(page + 1) + "/" + std::to_string(pages);
        const int count = cCoinShop.GetItemCountForUI();
        const int begin = std::max(0, page * 12);
        const int end = std::min(count, begin + 12);
        bool hasSelected = false;
        for (int i = begin; i < end; ++i)
        {
            int price = 0; bool selected = false;
            const sITEM* item = cCoinShop.GetItemForUI(i, &price, &selected);
            if (!item) continue;
            ContextItemSnapshot entry = MakeItem(i, *item, true, false);
            entry.price = price;
            entry.selected = selected;
            hasSelected = hasSelected || selected;
            out.items.push_back(std::move(entry));
        }
        AddAction(out, 70, "< Categoria", section > 1);
        AddAction(out, 71, "Categoria >", section < 7);
        AddAction(out, 72, "< Pagina", page > 0);
        AddAction(out, 73, "Pagina >", page + 1 < pages);
        AddAction(out, 74, "Comprar", hasSelected);
        AddAction(out, 75, "Fechar");
        return out;
    }

    if (cCoinShop_T.isOpen())
    {
        out.valid = true;
        out.kind = ContextKind::TimeCoinShop;
        out.title = "Loja de Moedas Temporais";
        out.money = cCoinShop_T.Coin;
        out.currency = "TCoin";
        const int section = cCoinShop_T.GetSectionForUI();
        const int page = cCoinShop_T.GetPageForUI();
        const int pages = std::max(1, cCoinShop_T.GetPageCountForUI());
        out.subtitle = std::string(CoinSectionName(section)) + " - pagina " +
                       std::to_string(page + 1) + "/" + std::to_string(pages);
        const int count = cCoinShop_T.GetItemCountForUI();
        const int begin = std::max(0, page * 12);
        const int end = std::min(count, begin + 12);
        bool hasSelected = false;
        for (int i = begin; i < end; ++i)
        {
            int price = 0; bool selected = false;
            const sITEM* item = cCoinShop_T.GetItemForUI(i, &price, &selected);
            if (!item) continue;
            ContextItemSnapshot entry = MakeItem(i, *item, true, false);
            entry.price = price;
            entry.selected = selected;
            hasSelected = hasSelected || selected;
            out.items.push_back(std::move(entry));
        }
        AddAction(out, 70, "< Categoria", section > 1);
        AddAction(out, 71, "Categoria >", section < 7);
        AddAction(out, 72, "< Pagina", page > 0);
        AddAction(out, 73, "Pagina >", page + 1 < pages);
        AddAction(out, 74, "Comprar", hasSelected);
        AddAction(out, 75, "Fechar");
        return out;
    }

    if (cRestaure.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::Restoration;
        out.title = "Restauracao";
        out.subtitle = "Itens apagados disponíveis para restaurar usando moedas.";
        out.money = cCoinShop.Coin;
        out.currency = "SCoin";
        out.feeCurrency = "SCoin";
        for (int i = 0; i < static_cast<int>(cRestaure.vItems.size()); ++i)
        {
            RESTAURE_ITEM* restore = cRestaure.vItems[i];
            if (!restore || !HasItem(restore->psItem))
                continue;
            ContextItemSnapshot entry = MakeItem(i, restore->psItem);
            entry.price = restore->CoinPrice;
            out.items.push_back(std::move(entry));
        }
        const int restoreCount = static_cast<int>(cRestaure.vItems.size());
        if (restoreCount > 0 && cRestaure.itemIndex >= 0 && cRestaure.itemIndex < restoreCount)
            out.fee = cRestaure.vItems[cRestaure.itemIndex]->CoinPrice;
        AddAction(out, 50, "Anterior", cRestaure.itemIndex > 0);
        AddAction(out, 51, "Proximo", cRestaure.itemIndex + 1 < restoreCount);
        AddAction(out, 52, "Restaurar", restoreCount > 0);
        AddAction(out, 53, "Fechar");
        return out;
    }

    if (cTrade.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::Trade;
        out.title = cTrade.szTradeCharName[0] ? std::string("Troca - ") + cTrade.szTradeCharName : "Troca";
        out.subtitle = sTrade.CheckFlag ? "Sua oferta esta confirmada." : "Botao direito na mochila adiciona itens a sua oferta.";
        for (int i = 0; i < MAX_TRADE_ITEM; ++i)
        {
            if (HasItem(sTrade.TradeItem[i]))
                out.items.push_back(MakeItem(i, sTrade.TradeItem[i], !sTrade.CheckFlag, false, "Voce"));
            if (HasItem(sTradeRecv.TradeItem[i]))
                out.items.push_back(MakeItem(1000 + i, sTradeRecv.TradeItem[i], false, true, "Outro"));
        }
        AddAction(out, 30, sTrade.CheckFlag ? "Confirmado" : "Confirmar", !sTrade.CheckFlag, sTrade.CheckFlag != 0);
        AddAction(out, 31, "Cancelar troca");
        return out;
    }

    if (cWareHouse.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::Warehouse;
        out.title = "Armazem";
        out.subtitle = "Botao direito na mochila guarda; clique aqui para devolver ao inventario.";
        out.money = sWareHouse.Money;
        out.gridColumns = 9;
        out.gridRows = 9;
        out.showEmptyGrid = true;
        bool occupied[9][9] = {};
        for (int i = 0; i < 100; ++i)
        {
            if (!HasItem(sWareHouse.WareHouseItem[i]))
                continue;
            const sITEM& item = sWareHouse.WareHouseItem[i];
            ContextItemSnapshot entry = MakeItem(i, item);
            entry.gridX = std::clamp((item.x - 21) / 22, 0, 8);
            entry.gridY = std::clamp((item.y - (136 + sinInterHeight2)) / 22, 0, 8);
            entry.gridWidth = std::clamp((item.w + 21) / 22, 1, 9 - entry.gridX);
            entry.gridHeight = std::clamp((item.h + 21) / 22, 1, 9 - entry.gridY);
            for (int y = entry.gridY; y < entry.gridY + entry.gridHeight; ++y)
                for (int x = entry.gridX; x < entry.gridX + entry.gridWidth; ++x)
                    occupied[y][x] = true;
            out.items.push_back(std::move(entry));
        }
        int occupiedCells = 0;
        for (int y = 0; y < 9; ++y)
            for (int x = 0; x < 9; ++x)
                if (occupied[y][x]) ++occupiedCells;
        const int currentWeight = std::max(0, static_cast<int>(sWareHouse.Weight[0]) - 197);
        const int maxWeight = std::max(0, static_cast<int>(sWareHouse.Weight[1]) - 196);
        out.statusText = "Espaco: " + std::to_string(occupiedCells) + "/81 celulas   Peso: " +
                         std::to_string(currentWeight) + "/" + std::to_string(maxWeight);
        AddAction(out, 20, "Fechar armazem");
        return out;
    }

    if (cCraftItem.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::Mixing;
        out.title = cCraftItem.ForceFlag ? "Força / Alquimia" : "Combinação";
        out.subtitle = "Item principal + ate 12 materiais + pedra especial. Botao direito envia itens da mochila.";
        out.fee = cCraftItem.ForceItemPrice;
        out.gridColumns = 8;
        out.gridRows = 4;
        out.showEmptyGrid = false;
        AddRecipeSlots(out, false);
        out.statusText = "Item a esquerda | 12 pedras a direita | pedra especial";
        for (int i = 0; i < 15; ++i)
        {
            if (!HasItem(sCraftItem.CraftItem[i]))
                continue;
            ContextItemSnapshot entry = MakeItem(i, sCraftItem.CraftItem[i]);
            ConfigureRecipeSlot(entry, i, sCraftItem.CraftItem[i]);
            if (entry.gridX >= 0)
                out.items.push_back(std::move(entry));
        }
        AddAction(out, 40, cCraftItem.ForceFlag ? "Criar Força" : "Misturar");
        AddAction(out, 41, "Cancelar");
        return out;
    }

    if (cAging.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::Aging;
        out.title = "Aprimoramento";
        out.subtitle = "Item principal + ate 12 sheltoms + pedra especial. As validacoes continuam no sistema original.";
        out.gridColumns = 8;
        out.gridRows = 4;
        out.showEmptyGrid = false;
        AddRecipeSlots(out, true);
        out.statusText = "Item a esquerda | sheltoms exigidos destacados | pedra especial";
        for (int i = 0; i < 15; ++i)
        {
            if (!HasItem(sAgingItem.AgingItem[i]))
                continue;
            ContextItemSnapshot entry = MakeItem(i, sAgingItem.AgingItem[i]);
            ConfigureRecipeSlot(entry, i, sAgingItem.AgingItem[i]);
            if (entry.gridX >= 0)
                out.items.push_back(std::move(entry));
        }
        AddAction(out, 40, "Aprimorar", AgingCheckSheltomFlag != 0);
        AddAction(out, 41, "Cancelar");
        return out;
    }

    if (SmeltingItem.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::Smelting;
        out.title = "Fundicao";
        out.subtitle = "Adicione os materiais com botao direito e confirme quando a receita estiver completa.";
        out.fee = SmeltingItem.m_SmeltingPrice;
        for (int i = 0; i < 5; ++i)
            if (HasItem(S_smeltingItem.SmeltingItem[i]))
                out.items.push_back(MakeItem(i, S_smeltingItem.SmeltingItem[i]));
        AddAction(out, 40, "Fundir");
        AddAction(out, 41, "Cancelar");
        return out;
    }

    if (ManufactureItem.m_OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::Manufacture;
        out.title = "Manufactura";
        out.subtitle = "Receita + runas. O resultado e o custo continuam validados pelo codigo original.";
        out.fee = ManufactureItem.m_ManufacturePrice;
        for (int i = 0; i < 4; ++i)
            if (HasItem(g_sManufactureItem.ManufactureItem[i]))
                out.items.push_back(MakeItem(i, g_sManufactureItem.ManufactureItem[i], true, false, i == 0 ? "Receita" : "Runa"));
        AddAction(out, 40, "Fabricar");
        AddAction(out, 41, "Cancelar");
        return out;
    }

    if (cMixtureReset.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::MixtureReset;
        out.title = "Limpeza de Combinação";
        out.subtitle = "Item principal + pedra de reset. As regras e o resultado continuam no codigo original.";
        for (int i = 0; i < 2; ++i)
            if (HasItem(sMixtureResetItem.MixtureResetItem[i]))
                out.items.push_back(MakeItem(i, sMixtureResetItem.MixtureResetItem[i], true, false, i == 0 ? "Item" : "Pedra"));
        AddAction(out, 40, "Remover Combinação", sMixtureResetItem.MixtureResetItem[1].Flag != 0);
        AddAction(out, 41, "Cancelar");
        return out;
    }

    if (cSkinChange.OpenFlag)
    {
        out.valid = true;
        out.kind = ContextKind::SkinChange;
        out.title = "Alteração de Visual";
        out.subtitle = "Item principal + pedra de visual compatível. As validações continuam no sistema original.";
        for (int i = 0; i < 2; ++i)
            if (HasItem(sSkinChangeItem.SkinChangeItem[i]))
                out.items.push_back(MakeItem(i, sSkinChangeItem.SkinChangeItem[i], true, false, i == 0 ? "Item" : "Visual"));
        AddAction(out, 40, "Aplicar Visual", sSkinChangeItem.SkinChangeItem[1].Flag != 0);
        AddAction(out, 41, "Cancelar");
        return out;
    }

    return out;
}


ContextInputRequest LegacyContextGateway::ItemActivationRequest(int index) const
{
    const ContextSnapshot state = GetSnapshot();
    ContextInputRequest request;
    if (!state.valid)
        return request;

    if (state.kind == ContextKind::Merchant)
    {
        if (index >= 0 && index < 30 && cShop.ShowShopItem[index].Flag &&
            cShop.ShowShopItem[index].Class == ITEM_CLASS_POTION)
        {
            request.kind = ContextInputKind::Number;
            request.title = "Quantidade";
            request.message = std::string("Quantas unidades de ") + cShop.ShowShopItem[index].sItemInfo.ItemName + " deseja comprar?";
            request.minimum = 1;
            request.maximum = std::max(1, cShop.ShowShopItem[index].sItemInfo.PotionCount > 0 ?
                                        cShop.ShowShopItem[index].sItemInfo.PotionCount : 9999);
            request.initial = 1;
        }
        return request;
    }

    if (state.kind == ContextKind::PlayerMerchant)
    {
        if (index < 0 || index >= 30 || !cCharShop.CharShopItem[index].Flag)
            return request;
        const sITEM& item = cCharShop.CharShopItem[index];
        if (item.Class == ITEM_CLASS_POTION)
        {
            request.kind = ContextInputKind::Number;
            request.title = "Quantidade";
            request.message = std::string("Quantas unidades de ") + item.sItemInfo.ItemName + " deseja comprar?";
            request.minimum = 1;
            request.maximum = std::max(1, static_cast<int>(item.sItemInfo.PotionCount));
            if (index < 30 && sRecvMyShop.sMyShopItem[index].Price > 0 && sinChar)
                request.maximum = std::min(request.maximum, std::max(1, sinChar->Money / sRecvMyShop.sMyShopItem[index].Price));
            if (sinChar)
                request.maximum = std::min(request.maximum, std::max(1, sinChar->Weight[1] - sinChar->Weight[0]));
            request.initial = 1;
        }
        else
        {
            request.kind = ContextInputKind::Confirmation;
            request.title = "Confirmar compra";
            request.message = std::string("Comprar ") + item.sItemInfo.ItemName + "?";
        }
        return request;
    }

    if (state.kind == ContextKind::PersonalShop && !MyShopSendButton)
    {
        if (index >= 0 && index < 30 && cMyShop.MyShopItem[index].Flag)
        {
            request.kind = ContextInputKind::Confirmation;
            request.title = "Remover da loja";
            request.message = std::string("Remover ") + cMyShop.MyShopItem[index].sItemInfo.ItemName + " da venda?";
        }
    }
    return request;
}

ContextInputRequest LegacyContextGateway::InventoryAcceptanceRequest(int inventoryIndex) const
{
    ContextInputRequest request;
    const ContextSnapshot state = GetSnapshot();
    if (inventoryIndex < 0 || inventoryIndex >= INVENTORY_MAXITEM)
        return request;
    const sITEM& item = cInvenTory.InvenItem[inventoryIndex];
    if (!item.Flag || item.ItemPosition > 0)
        return request;

    if (state.kind == ContextKind::Merchant)
    {
        // Selling to an NPC merchant is immediate in the V2 surface. The
        // resolved path below still runs every legacy salability, Gold-limit
        // and protected-item validation before calling SellItemToShop().
        return request;
    }

    if (state.kind != ContextKind::PersonalShop || MyShopSendButton)
        return request;
    request.kind = ContextInputKind::Number;
    request.title = "Preco de venda";
    request.message = std::string("Defina o preco de venda para ") + item.sItemInfo.ItemName + ".";
    request.minimum = 1;
    request.maximum = 1000000000;
    request.initial = std::max(1, item.CoinPrice > 0 ? item.CoinPrice : static_cast<int>(item.sItemInfo.Money));
    request.primaryOption = "Ouro";
    request.alternateOption = "Moedas";
    return request;
}

bool LegacyContextGateway::ActivateItemResolved(int index, int value)
{
    const ContextSnapshot state = GetSnapshot();
    if (!state.valid)
        return false;

    if (state.kind == ContextKind::Merchant)
    {
        if (index < 0 || index >= 30 || !cShop.ShowShopItem[index].Flag || value <= 0)
            return false;
        if (!IsClassCompatible(cShop.ShowShopItem[index]) ||
            !MeetsStatRequirements(cShop.ShowShopItem[index]))
        {
            cMessageBox.ShowMessage(MESSAGE_NO_USE_ITEM);
            return false;
        }
        return cShop.SendBuyItemToServer(&cShop.ShowShopItem[index], value) != 0;
    }

    if (state.kind == ContextKind::PlayerMerchant)
    {
        if (index < 0 || index >= 30 || !cCharShop.CharShopItem[index].Flag)
            return false;
        SelectMyShopItemIndex = index + 1;
        const bool prepared = cCharShop.BuyItem(index) != 0;
        SelectMyShopItemIndex = 0;
        if (!prepared)
            return false;

        if (cCharShop.CharShopItem[index].Class == ITEM_CLASS_POTION)
        {
            if (!sMessageBox2[MESSAGE_MYSHOP_POTION].Flag)
                return false;
            cMessageBox.CloseMessage();
            const int count = std::max(1, value);
            sMyShop_Server.sTime = count;
            sMyShop_Server.Price *= count;
            Send_PersonalShopItem(sRecvMyShop.CHAR_CODE, &sMyShop_Server);
            return true;
        }

        if (!sMessageBox3[MESSAGE_MYSHOP_ITEM_BUY].Flag)
            return false;
        sMessageBox3[MESSAGE_MYSHOP_ITEM_BUY].Flag = 0;
        sinMessageBoxShowFlag = 0;
        Send_PersonalShopItem(sRecvMyShop.CHAR_CODE, &sMyShop_Server);
        return true;
    }

    if (state.kind == ContextKind::PersonalShop)
    {
        if (MyShopSendButton || index < 0 || index >= 30 || !cMyShop.MyShopItem[index].Flag)
            return false;
        const int inventoryIndex = cMyShop.SearchShopItemToInven(&cMyShop.MyShopItem[index]);
        if (inventoryIndex <= 0 || inventoryIndex > INVENTORY_MAXITEM)
            return false;
        MyShopItemIndex[inventoryIndex - 1] = 0;
        cMyShop.MyShopItem[index].Flag = 0;
        std::memset(&cMyShop.MyShopItem[index], 0, sizeof(sITEM));
        return true;
    }

    return ActivateItem(index);
}

bool LegacyContextGateway::AcceptInventoryItemResolved(int inventoryIndex, int value)
{
    return AcceptInventoryItemResolvedOption(inventoryIndex, value, false);
}

bool LegacyContextGateway::AcceptInventoryItemResolvedOption(int inventoryIndex, int value, bool alternate)
{
    const ContextSnapshot state = GetSnapshot();

    if (state.kind == ContextKind::Merchant)
    {
        if (inventoryIndex < 0 || inventoryIndex >= INVENTORY_MAXITEM)
            return false;

        // The V2 surface requested an immediate sale. Ask the legacy inventory path
        // to perform its complete salability/money/donate-item checks and the
        // normal MouseItem staging, but bypass creation of the old message box.
        uiV2ConfirmedMerchantSaleResult = 0;
        uiV2ConfirmedMerchantSale = 1;
        LegacyInventoryGateway inventory;
        const bool activated = inventory.ActivateItem(inventoryIndex);

        // If validation returned before the merchant-sale branch, the one-shot
        // bypass was not consumed. Never let it leak into a later right click.
        if (uiV2ConfirmedMerchantSale)
            uiV2ConfirmedMerchantSale = 0;

        // Defensive cleanup for stale state from older builds/sessions. The V2
        // path above does not call ShowMessage3(), so this should normally be 0.
        sMessageBox3[MESSAGE_SELL_HIGHRANK_ITEM].Flag = 0;
        return activated && uiV2ConfirmedMerchantSaleResult != 0;
    }

    if (state.kind != ContextKind::PersonalShop)
        return AcceptInventoryItem(inventoryIndex);
    if (MyShopSendButton || inventoryIndex < 0 || inventoryIndex >= INVENTORY_MAXITEM || value <= 0)
        return false;

    LegacyInventoryGateway inventory;
    if (!inventory.ActivateItem(inventoryIndex))
        return false;

    // Keep every legacy item restriction and the 9x9 placement rules. A valid
    // item reaches MESSAGE_MYSHOP_ITEM; close that numeric popup before it can
    // render and commit the value collected by the V2 modal instead.
    if (!sMessageBox2[MESSAGE_MYSHOP_ITEM].Flag || !MyShopMouseItem.Flag)
        return false;
    cMessageBox.CloseMessage();

    if (alternate)
    {
        // Coin listings never participated in the legacy Gold carry-limit
        // calculation. Preserve that behavior and store only CoinPrice.
        MyShopMouseItem.sItemInfo.Money = 0;
        MyShopMouseItem.CoinPrice = value;
    }
    else
    {
        const int stackCount = MyShopMouseItem.Class == ITEM_CLASS_POTION
                                   ? std::max(1, static_cast<int>(MyShopMouseItem.sItemInfo.PotionCount))
                                   : 1;
        const long long total = static_cast<long long>(cMyShop.GetTotalMyShopItemMoney()) +
                                static_cast<long long>(value) * stackCount;
        if (total > 1000000000LL ||
            !cInvenTory.CheckMoneyLimit(static_cast<int>(std::min<long long>(total, 1000000000LL))))
        {
            cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
            return false;
        }
        MyShopMouseItem.CoinPrice = 0;
        MyShopMouseItem.sItemInfo.Money = value;
    }

    return cMyShop.LastSetMyShopItem(&MyShopMouseItem) != 0;
}

bool LegacyContextGateway::ActivateItem(int index)
{
    const ContextSnapshot state = GetSnapshot();
    if (!state.valid || MouseItem.Flag)
        return false;

    if (state.kind == ContextKind::Merchant)
    {
        if (index < 0 || index >= 30 || !cShop.ShowShopItem[index].Flag)
            return false;
        if (!IsClassCompatible(cShop.ShowShopItem[index]) ||
            !MeetsStatRequirements(cShop.ShowShopItem[index]))
        {
            cMessageBox.ShowMessage(MESSAGE_NO_USE_ITEM);
            return false;
        }
        ShowShopItemIndex = index + 1;
        cShop.RButtonDown(0, 0);
        ShowShopItemIndex = 0;
        return true;
    }

    if (state.kind == ContextKind::PlayerMerchant)
    {
        if (index < 0 || index >= 30 || !cCharShop.CharShopItem[index].Flag)
            return false;
        SelectMyShopItemIndex = index + 1;
        const bool result = cCharShop.BuyItem(index) != 0;
        SelectMyShopItemIndex = 0;
        return result;
    }

    if (state.kind == ContextKind::PersonalShop)
    {
        if (MyShopSendButton || index < 0 || index >= 30 || !cMyShop.MyShopItem[index].Flag)
            return false;
        const int inventoryIndex = cMyShop.SearchShopItemToInven(&cMyShop.MyShopItem[index]);
        if (inventoryIndex <= 0 || inventoryIndex > INVENTORY_MAXITEM)
            return false;
        MyShopItemCancelIndex[0] = inventoryIndex;
        MyShopItemCancelIndex[1] = index + 1;
        cMessageBox.ShowMessage3(MESSAGE_MYSHOP_ITEM_CANCEL,
                                 cInvenTory.InvenItem[inventoryIndex - 1].sItemInfo.ItemName);
        return true;
    }

    if (state.kind == ContextKind::CoinShop)
        return cCoinShop.SelectItemForUI(index);
    if (state.kind == ContextKind::TimeCoinShop)
        return cCoinShop_T.SelectItemForUI(index);

    if (state.kind == ContextKind::Restoration)
    {
        if (index < 0 || index >= static_cast<int>(cRestaure.vItems.size()))
            return false;
        cRestaure.itemIndex = index;
        return true;
    }

    return ReturnContextItem(state.kind, index);
}

bool LegacyContextGateway::AcceptInventoryItem(int inventoryIndex)
{
    if (inventoryIndex < 0 || inventoryIndex >= INVENTORY_MAXITEM || MouseItem.Flag)
        return false;

    sITEM& source = cInvenTory.InvenItem[inventoryIndex];
    if (!source.Flag || source.ItemPosition > 0)
        return false;

    const ContextSnapshot state = GetSnapshot();
    if (state.kind == ContextKind::Merchant)
        return AcceptInventoryItemResolvedOption(inventoryIndex, 1, false);

    if (state.kind == ContextKind::CoinShop || state.kind == ContextKind::TimeCoinShop ||
        state.kind == ContextKind::Restoration || state.kind == ContextKind::PlayerMerchant)
        return false;

    // Most migrated contexts already have battle-tested right-click routing in
    // cINVENTORY::RButtonDown. Reuse that routing through the logical index;
    // only reset/skin need explicit adapters because their old right-click
    // branches are missing/incomplete.
    if (!cMixtureReset.OpenFlag && !cSkinChange.OpenFlag)
    {
        if (cMyShop.OpenFlag && MyShopSendButton)
            return false;
        LegacyInventoryGateway inventory;
        return inventory.ActivateItem(inventoryIndex);
    }

    LegacyInventoryGateway inventory;
    const sITEM original = source;
    const POINT previousCursor = pCursorPos;
    if (!inventory.BeginDrag(inventoryIndex) || !MouseItem.Flag)
        return false;

    bool placed = false;
    if (cMixtureReset.OpenFlag)
    {
        if (cMixtureReset.isItemOkay(&original) && cMixtureReset.isEmpty(0))
        {
            const int startX = 58;
            const int startY = 193 + sinInterHeight2;
            const int endX = startX + 3 * 22;
            const int endY = startY + 4 * 22;
            pCursorPos.x = startX + (endX - startX) / 2;
            pCursorPos.y = startY + (endY - startY) / 2;
            MouseItem.x = MouseItem.SetX = startX + ((endX - startX) - MouseItem.w) / 2;
            MouseItem.y = MouseItem.SetY = startY + ((endY - startY) - MouseItem.h) / 2;
            if (cMixtureReset.SetMixtureItemResetAreaCheck(&MouseItem))
                placed = cMixtureReset.LastSetMixtureResetItem(&MouseItem, 2) != 0 && !MouseItem.Flag;
        }
        else if (cMixtureReset.isStoneOkay(&original) && cMixtureReset.isEmpty(1) && !cMixtureReset.isEmpty(0))
        {
            // The stone area is the same one used by the legacy reset window.
            const int startX = AgingLevelSheltomXY[0][0].x + 23;
            const int startY = AgingLevelSheltomXY[0][0].y + 31;
            MouseItem.x = MouseItem.SetX = startX;
            MouseItem.y = MouseItem.SetY = startY;
            pCursorPos.x = startX + 11;
            pCursorPos.y = startY + 11;
            if (cMixtureReset.SetMixtureItemResetAreaCheck(&MouseItem))
            {
                placed = cMixtureReset.LastSetMixtureResetItem(&MouseItem, 1) != 0 && !MouseItem.Flag;
                if (placed)
                    MixtureResetCheckFlag = 1;
            }
        }
    }
    else if (cSkinChange.OpenFlag)
    {
        const DWORD family = original.sItemInfo.CODE & sinITEM_MASK2;
        const bool skinStone = family == sinSK1 || family == sinSK2 || family == sinSK3 || family == sinSK4;
        if (skinStone && sSkinChangeItem.SkinChangeItem[0].Flag && !sSkinChangeItem.SkinChangeItem[1].Flag)
        {
            const int startX = 219;
            const int startY = 193 + sinInterHeight2;
            MouseItem.x = MouseItem.SetX = startX + (3 * 22 - MouseItem.w) / 2;
            MouseItem.y = MouseItem.SetY = startY + (4 * 22 - MouseItem.h) / 2;
            pCursorPos.x = MouseItem.x + std::max(1, MouseItem.w / 2);
            pCursorPos.y = MouseItem.y + std::max(1, MouseItem.h / 2);
            if (cSkinChange.SetSkinChangeAreaCheck(&MouseItem))
                placed = cSkinChange.LastSetSkinChangeItem(&MouseItem, 1) != 0 && !MouseItem.Flag;
        }
        else if (!skinStone && !sSkinChangeItem.SkinChangeItem[0].Flag)
        {
            const int startX = 58;
            const int startY = 193 + sinInterHeight2;
            pCursorPos.x = startX + 33;
            pCursorPos.y = startY + 44;
            MouseItem.x = MouseItem.SetX = startX + (3 * 22 - MouseItem.w) / 2;
            MouseItem.y = MouseItem.SetY = startY + (4 * 22 - MouseItem.h) / 2;
            if (cSkinChange.SetSkinChangeAreaCheck(&MouseItem))
                placed = cSkinChange.LastSetSkinChangeItem(&MouseItem, 2) != 0 && !MouseItem.Flag;
        }
    }

    if (!placed && MouseItem.Flag)
        inventory.CancelDrag();
    pCursorPos = previousCursor;
    return placed;
}

bool LegacyContextGateway::ReturnContextItem(ContextKind kind, int index)
{
    sITEM* item = nullptr;
    int pickKind = 0;
    switch (kind)
    {
        case ContextKind::Trade:
            if (index >= 1000 || index < 0 || index >= MAX_TRADE_ITEM || !sTrade.TradeItem[index].Flag || sTrade.CheckFlag)
                return false;
            item = &sTrade.TradeItem[index];
            break;
        case ContextKind::Warehouse:
            if (index < 0 || index >= 100 || !sWareHouse.WareHouseItem[index].Flag)
                return false;
            item = &sWareHouse.WareHouseItem[index];
            break;
        case ContextKind::Mixing:
            if (index < 0 || index >= 15 || !sCraftItem.CraftItem[index].Flag)
                return false;
            item = &sCraftItem.CraftItem[index];
            break;
        case ContextKind::Aging:
            if (index < 0 || index >= 15 || !sAgingItem.AgingItem[index].Flag)
                return false;
            item = &sAgingItem.AgingItem[index];
            pickKind = 1;
            break;
        case ContextKind::Smelting:
            if (index < 0 || index >= 5 || !S_smeltingItem.SmeltingItem[index].Flag)
                return false;
            item = &S_smeltingItem.SmeltingItem[index];
            break;
        case ContextKind::Manufacture:
            if (index < 0 || index >= 4 || !g_sManufactureItem.ManufactureItem[index].Flag)
                return false;
            item = &g_sManufactureItem.ManufactureItem[index];
            break;
        case ContextKind::MixtureReset:
            if (index < 0 || index >= 2 || !sMixtureResetItem.MixtureResetItem[index].Flag)
                return false;
            item = &sMixtureResetItem.MixtureResetItem[index];
            break;
        case ContextKind::SkinChange:
            if (index < 0 || index >= 2 || !sSkinChangeItem.SkinChangeItem[index].Flag)
                return false;
            item = &sSkinChangeItem.SkinChangeItem[index];
            break;
        default:
            return false;
    }

    if (!item || !InventoryHasRoomFor(*item))
    {
        cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
        return false;
    }

    const int x = item->x + std::max(1, item->w / 2);
    const int y = item->y + std::max(1, item->h / 2);
    bool picked = false;
    switch (kind)
    {
        case ContextKind::Trade: picked = cTrade.PickUpTradeItem(x, y, 1) != 0; break;
        case ContextKind::Warehouse: picked = cWareHouse.PickUpWareHouseItem(x, y, 1) != 0; break;
        case ContextKind::Mixing: picked = cCraftItem.PickUpCraftItem(x, y, 1, pickKind) != 0; break;
        case ContextKind::Aging: picked = cCraftItem.PickUpCraftItem(x, y, 1, pickKind) != 0; break;
        case ContextKind::Smelting: picked = SmeltingItem.PickUpSmeltingItem(x, y, 1) != 0; break;
        case ContextKind::Manufacture: picked = ManufactureItem.PickUpManufactureItem(x, y, 1) != 0; break;
        case ContextKind::MixtureReset: picked = cMixtureReset.PickUpMixtureResetItem(x, y, 1) != 0; break;
        case ContextKind::SkinChange: picked = cSkinChange.PickUpSkinChangeItem(x, y, 1) != 0; break;
        default: break;
    }
    return picked && PutMouseItemBackInInventory();
}

bool LegacyContextGateway::InvokeAction(int actionId)
{
    const ContextSnapshot state = GetSnapshot();
    if (!state.valid || MouseItem.Flag)
        return false;

    if (state.kind == ContextKind::Merchant)
    {
        int legacyButton = 0;
        switch (actionId)
        {
            case 10: legacyButton = SINALLREPAIR; break;
            case 11: legacyButton = SINREPAIR; break;
            case 12: legacyButton = SINSELL; break;
            case 13: legacyButton = SINBUY; break;
            case 14: legacyButton = 8; break;
            case 15: legacyButton = 9; break;
            default: return false;
        }
        ShowShopItemIndex = 0;
        SelectShopButtonPosi = legacyButton;
        { OffscreenCursorGuard cursor; cShop.LButtonDown(0, 0); }
        SelectShopButtonPosi = 0;
        return true;
    }

    if (state.kind == ContextKind::PlayerMerchant && actionId == 16)
    {
        cCharShop.OpenFlag = 0;
        sinMoveKindInter[SIN_CHARSHOP] = 0;
        std::memset(cCharShop.CharShopItem, 0, sizeof(cCharShop.CharShopItem));
        return true;
    }

    if (state.kind == ContextKind::PersonalShop)
    {
        if (actionId == 60)
        {
            SelectMyShopButtonPosi = 1;
            { OffscreenCursorGuard cursor; cShop.LButtonDown(0, 0); }
            SelectMyShopButtonPosi = 0;
            return MyShopSendButton == 0;
        }
        if (actionId == 61)
        {
            SelectMyShopButtonPosi = 2;
            { OffscreenCursorGuard cursor; cShop.LButtonDown(0, 0); }
            SelectMyShopButtonPosi = 0;
            // The old description editor remains the compatibility modal for
            // now. The bridge temporarily releases ownership while it is open.
            return MyShopExpBox != 0 || MyShopSendButton != 0;
        }
        if (actionId == 62)
            return CloseContext();
        return false;
    }

    if (state.kind == ContextKind::CoinShop || state.kind == ContextKind::TimeCoinShop)
    {
        const bool time = state.kind == ContextKind::TimeCoinShop;
        auto section = [&]() { return time ? cCoinShop_T.GetSectionForUI() : cCoinShop.GetSectionForUI(); };
        switch (actionId)
        {
            case 70: return time ? cCoinShop_T.SelectSectionForUI(std::max(1, section() - 1))
                                 : cCoinShop.SelectSectionForUI(std::max(1, section() - 1));
            case 71: return time ? cCoinShop_T.SelectSectionForUI(std::min(7, section() + 1))
                                 : cCoinShop.SelectSectionForUI(std::min(7, section() + 1));
            case 72: return time ? cCoinShop_T.ChangePageForUI(-1) : cCoinShop.ChangePageForUI(-1);
            case 73: return time ? cCoinShop_T.ChangePageForUI(1) : cCoinShop.ChangePageForUI(1);
            case 74: return time ? cCoinShop_T.BuySelectedForUI() : cCoinShop.BuySelectedForUI();
            case 75: if (time) cCoinShop_T.Close(); else cCoinShop.Close(); return true;
            default: return false;
        }
    }

    if (state.kind == ContextKind::Restoration)
    {
        switch (actionId)
        {
            case 50:
                if (cRestaure.itemIndex > 0) --cRestaure.itemIndex;
                return true;
            case 51:
                if (cRestaure.itemIndex + 1 < static_cast<int>(cRestaure.vItems.size())) ++cRestaure.itemIndex;
                return true;
            case 52: cRestaure.Restaure(); return true;
            case 53: cRestaure.Close(); return true;
            default: return false;
        }
    }

    if (state.kind == ContextKind::Warehouse && actionId == 20)
    {
        const bool closed = cWareHouse.CloseWareHouse() != 0;
        sinMoveKindInter[SIN_WAREHOUSE] = 0;
        return closed;
    }

    if (state.kind == ContextKind::Trade)
    {
        if (actionId == 30)
        {
            if (sTrade.CheckFlag)
                return true;
            TradeCheckFlag = T_CHECK_POSI;
            { OffscreenCursorGuard cursor; cTrade.LButtonDown(0, 0); }
            TradeCheckFlag = 0;
            return sTrade.CheckFlag != 0;
        }
        if (actionId == 31)
        {
            SendRequestTrade(cTrade.TradeCharCode, 3);
            cTrade.OpenFlag = 0;
            cInvenTory.OpenFlag = 0;
            cTrade.CancelTradeItem();
            sinMoveKindInter[SIN_TRADE] = 0;
            return true;
        }
    }

    if (actionId == 40 || actionId == 41)
    {
        CraftItemButtonIndex = actionId == 40 ? 1 : 2;
        { OffscreenCursorGuard cursor; cTrade.LButtonDown(0, 0); }
        CraftItemButtonIndex = 0;
        return true;
    }

    return false;
}

bool LegacyContextGateway::CloseContext()
{
    const ContextSnapshot state = GetSnapshot();
    if (!state.valid)
        return true;

    switch (state.kind)
    {
        case ContextKind::Merchant:
            cShop.OpenFlag = SIN_CLOSE;
            // UI V2 owns the visual lifecycle. Snap the hidden legacy panel to
            // fully closed so cShop.Draw() cannot replay its slide-out animation.
            sinMoveKindInter[SIN_SHOP] = 0;
            CursorClass = 1;
            return true;
        case ContextKind::PlayerMerchant:
            cCharShop.OpenFlag = SIN_CLOSE;
            sinMoveKindInter[SIN_CHARSHOP] = 0;
            std::memset(cCharShop.CharShopItem, 0, sizeof(cCharShop.CharShopItem));
            return true;
        case ContextKind::PersonalShop:
            cMyShop.OpenFlag = SIN_CLOSE;
            sinMoveKindInter[SIN_MYSHOP] = 0;
            if (!MyShopSendButton)
            {
                std::memset(cMyShop.MyShopItem, 0, sizeof(cMyShop.MyShopItem));
                std::memset(MyShopItemIndex, 0, sizeof(int) * 100);
            }
            return true;
        case ContextKind::CoinShop:
            cCoinShop.Close();
            return true;
        case ContextKind::TimeCoinShop:
            cCoinShop_T.Close();
            return true;
        case ContextKind::Restoration:
            cRestaure.Close();
            return true;
        case ContextKind::Trade:
            SendRequestTrade(cTrade.TradeCharCode, 3);
            cTrade.OpenFlag = SIN_CLOSE;
            cTrade.CancelTradeItem();
            sinMoveKindInter[SIN_TRADE] = 0;
            return true;
        case ContextKind::Warehouse:
        {
            const bool result = cWareHouse.CloseWareHouse() != 0;
            sinMoveKindInter[SIN_WAREHOUSE] = 0;
            return result;
        }
        case ContextKind::Mixing:
        {
            const bool result = cCraftItem.CancelMixItem() != 0;
            sinMoveKindInter[SIN_CRAFTITEM] = 0;
            return result;
        }
        case ContextKind::Aging:
        {
            const bool result = cAging.CancelAgingItem() != 0;
            sinMoveKindInter[SIN_AGING] = 0;
            return result;
        }
        case ContextKind::Smelting:
        {
            const bool result = SmeltingItem.CancelSmeltingItem() != 0;
            sinMoveKindInter[SIN_SMELTING] = 0;
            return result;
        }
        case ContextKind::Manufacture:
        {
            const bool result = ManufactureItem.CancelManufactureItem() != 0;
            sinMoveKindInter[SIN_MANUFACTURE] = 0;
            return result;
        }
        case ContextKind::MixtureReset:
        {
            const bool result = cMixtureReset.CancelMixtureItemReset() != 0;
            sinMoveKindInter[SIN_MIXTURE_RESET] = 0;
            return result;
        }
        case ContextKind::SkinChange:
        {
            const bool result = cSkinChange.CancelSkinChange() != 0;
            sinMoveKindInter[SIN_SKIN_CHANGE] = 0;
            return result;
        }
        default:
            return false;
    }
}
}
