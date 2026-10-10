#pragma once

#include <string>
#include <cstdint>
#include "systems/ui/v2/domain/ContextUIModels.h"

namespace ui::v2
{
enum class ContextInputKind
{
    None,
    Confirmation,
    Number
};

struct ContextInputRequest
{
    ContextInputKind kind = ContextInputKind::None;
    std::string title;
    std::string message;
    int minimum = 0;
    int maximum = 0;
    int initial = 0;
    std::string primaryOption;
    std::string alternateOption;
};

class IContextGateway
{
public:
    virtual ~IContextGateway() = default;
    [[nodiscard]] virtual ContextSnapshot GetSnapshot() const = 0;
    // Optional cheap change token. Max means unsupported (keep existing polling).
    [[nodiscard]] virtual std::uint64_t ContextStateRevision() const
    {
        return static_cast<std::uint64_t>(-1);
    }
    virtual bool ActivateItem(int index) = 0;
    virtual bool AcceptInventoryItem(int inventoryIndex) = 0;
    virtual bool InvokeAction(int actionId) = 0;
    virtual bool CloseContext() = 0;

    // Optional interaction metadata lets the V2 shell collect confirmation or
    // a numeric value without moving business rules into widgets. Gateways
    // that do not need a modal keep the original methods unchanged.
    [[nodiscard]] virtual ContextInputRequest ItemActivationRequest(int index) const
    {
        (void)index;
        return {};
    }
    [[nodiscard]] virtual ContextInputRequest InventoryAcceptanceRequest(int inventoryIndex) const
    {
        (void)inventoryIndex;
        return {};
    }
    virtual bool ActivateItemResolved(int index, int value)
    {
        (void)value;
        return ActivateItem(index);
    }
    virtual bool AcceptInventoryItemResolved(int inventoryIndex, int value)
    {
        (void)value;
        return AcceptInventoryItem(inventoryIndex);
    }
    virtual bool ActivateItemResolvedOption(int index, int value, bool alternate)
    {
        (void)alternate;
        return ActivateItemResolved(index, value);
    }
    virtual bool AcceptInventoryItemResolvedOption(int inventoryIndex, int value, bool alternate)
    {
        (void)alternate;
        return AcceptInventoryItemResolved(inventoryIndex, value);
    }
};
}
