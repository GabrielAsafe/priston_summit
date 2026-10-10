#pragma once

#include "systems/ui/v2/domain/IContextGateway.h"

namespace ui::v2::legacy
{
class LegacyContextGateway final : public IContextGateway
{
public:
    [[nodiscard]] ContextSnapshot GetSnapshot() const override;
    [[nodiscard]] std::uint64_t ContextStateRevision() const override;
    bool ActivateItem(int index) override;
    bool AcceptInventoryItem(int inventoryIndex) override;
    [[nodiscard]] ContextInputRequest ItemActivationRequest(int index) const override;
    [[nodiscard]] ContextInputRequest InventoryAcceptanceRequest(int inventoryIndex) const override;
    bool ActivateItemResolved(int index, int value) override;
    bool AcceptInventoryItemResolved(int inventoryIndex, int value) override;
    bool AcceptInventoryItemResolvedOption(int inventoryIndex, int value, bool alternate) override;
    bool InvokeAction(int actionId) override;
    bool CloseContext() override;

private:
    bool ReturnContextItem(ContextKind kind, int index);
};
}
