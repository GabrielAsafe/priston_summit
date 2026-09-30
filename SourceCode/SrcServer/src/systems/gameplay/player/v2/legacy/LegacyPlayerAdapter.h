#pragma once

#include "gameplay/player/v2/IPlayerQuery.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

class rsPLAYINFO;

namespace gameplay
{
namespace player
{
namespace legacy
{
class LegacyPlayerAdapter final : public IPlayerQuery, public ICharacterQuery
{
public:
    explicit LegacyPlayerAdapter(const LegacyOnlinePlayerRegistry& registry);

    bool TryGet(PlayerId player, PlayerSnapshot& out) const override;
    bool TryGet(PlayerHandle player, PlayerSnapshot& out) const override;
    bool TryGetByKey(const std::string& playerKey, PlayerSnapshot& out) const override;
    bool IsOnline(PlayerId player) const override;
    bool TryGetCharacter(PlayerId player, CharacterSnapshot& out) const override;

    // Transitional AMBER entry point for legacy callers that already hold the
    // kernel pointer. It converts immediately to a modern snapshot.
    bool TryGetLegacy(const rsPLAYINFO* player, PlayerSnapshot& out) const;

private:
    bool Translate(const rsPLAYINFO* player, PlayerSnapshot& out) const;
    bool TranslateCharacter(const rsPLAYINFO* player, CharacterSnapshot& out) const;

    const LegacyOnlinePlayerRegistry& registry_;
};

const LegacyPlayerAdapter& ServerLegacyPlayerAdapter();
}
}
}
