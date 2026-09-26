#pragma once

#include "gameplay/covenants/v2/CovenantUIProtocol.h"
#include "systems/ui/v2/domain/ICovenantGateway.h"

namespace ui::v2
{
class LegacyCovenantGateway final : public ICovenantGateway
{
public:
    const CovenantUiSnapshot& Snapshot() const override { return snapshot_; }
    void SelectCovenant(int covenantId) override;
    void RequestRefresh() override;
    void Execute(CovenantUiAction action, int value = 0, const std::string& key = {}) override;
    void ApplyState(const gameplay::covenants::protocol::CovenantUIStatePacket& packet);
    void Tick(unsigned long nowMilliseconds);
    void Reset();

private:
    CovenantUiSnapshot snapshot_{};
    int selectedCovenantId_ = 1;
    unsigned long lastRefreshTick_ = 0;
};

LegacyCovenantGateway& GetLegacyCovenantGateway();
void ApplyLegacyCovenantStatePacket(const gameplay::covenants::protocol::CovenantUIStatePacket& packet);
}
