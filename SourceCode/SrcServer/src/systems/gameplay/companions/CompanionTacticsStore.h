#pragma once

#include <windows.h>

#include "gameplay/companions/CompanionProtocol.h"

namespace gameplay
{
namespace companions
{

struct CompanionTacticsConfig
{
    CompanionTacticsConfig();

    bool enabled;
    protocol::CompanionUITacticRule rules[protocol::COMPANION_UI_TACTIC_RULES];
};

// Custom tactics live in their own sidecar.  The legacy 84-byte companion
// profile ABI is intentionally left untouched so older profiles remain byte-
// compatible and rollback does not require profile migration.
bool LoadCompanionTactics(const char *account, const char *characterName, CompanionTacticsConfig *config);
bool SaveCompanionTactics(const char *account, const char *characterName, const CompanionTacticsConfig &config);

} // namespace companions
} // namespace gameplay
