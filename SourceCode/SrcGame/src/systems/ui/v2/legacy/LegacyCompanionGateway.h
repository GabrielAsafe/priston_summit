#pragma once

#include "systems/ui/v2/domain/ICompanionGateway.h"
#include "gameplay/companions/CompanionProtocol.h"

namespace ui::v2
{
class LegacyCompanionGateway final : public ICompanionGateway
{
public:
    const CompanionUiSnapshot& Snapshot() const override { return snapshot_; }
    const CompanionRuntimeSnapshot& RuntimeSnapshot() const override { return runtimeSnapshot_; }
    void RequestRefresh() override;
    void Execute(CompanionUiAction action, const std::string& characterName = {}, int value = 0) override;

    void ApplyState(const gameplay::companions::protocol::CompanionUIStatePacket& packet);
    void ApplyRuntimeState(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet);
    void Reset();

private:
    CompanionUiSnapshot snapshot_{};
    CompanionRuntimeSnapshot runtimeSnapshot_{};
};

LegacyCompanionGateway& GetLegacyCompanionGateway();
void ApplyLegacyCompanionStatePacket(const gameplay::companions::protocol::CompanionUIStatePacket& packet);
void ApplyLegacyCompanionRuntimePacket(const gameplay::companions::protocol::CompanionRuntimeStatePacket& packet);
}
