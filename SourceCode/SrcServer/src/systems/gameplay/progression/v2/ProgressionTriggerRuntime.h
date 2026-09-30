#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace progression
{
// Processes the current online player's Story/Covenant conditions using the
// authoritative level/job snapshot already held by the server. This is the
// future character/session integration point; no character lifecycle hook is
// installed while that subsystem remains frozen.
bool ProcessServerProgressionOnlinePlayer(PlayerId player);

// Best-effort notification used by legacy gameplay systems after they persist a
// state change that can affect progression conditions. Before Progression V2
// reaches Ready this is intentionally a no-op.
void NotifyServerProgressionPlayerStateChanged(PlayerId player);

// Re-evaluates all currently online players after a persisted global Story flag
// changes. Nested world changes are coalesced to avoid cross-player recursion.
void NotifyServerProgressionWorldStateChanged();
void RegisterServerProgressionEventTriggers();
}
}
