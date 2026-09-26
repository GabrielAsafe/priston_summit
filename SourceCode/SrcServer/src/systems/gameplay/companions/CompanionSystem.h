#pragma once

#include <windows.h>

#include "gameplay/companions/CompanionProtocol.h"

class rsPLAYINFO;
class smCHAR;
struct TRANS_RECORD_DATA;

namespace gameplay
{
namespace companions
{

enum CompanionMode
{
    COMPANION_MODE_AGGRESSIVE = 0,
    COMPANION_MODE_FOLLOW = 1,
    COMPANION_MODE_PASSIVE = 2
};

static const int COMPANION_MAX_ACTIVE = 4;

// Core lifecycle.
int Summon(rsPLAYINFO *owner, const char *characterName);
int SummonSelected(rsPLAYINFO *owner);
int Select(rsPLAYINFO *owner, const char *characterName);
int Dismiss(rsPLAYINFO *owner, const char *characterName);
int DismissAll(rsPLAYINFO *owner);
void OnOwnerDisconnect(rsPLAYINFO *owner);

// Query/state used by the EXP and AI integrations.
int GetActiveCount(rsPLAYINFO *owner);
int GetPartyActiveCount(rsPLAYINFO *partyMaster, int area);
bool IsManagedCompanion(smCHAR *character);
// Local scalable passive layer.
int GetPassiveEvadePercent(smCHAR *character);
int GetReactiveDamage(smCHAR *character, int receivedDamage);
// V14+ defensive/offensive runtime hooks.
bool CanMonsterTargetCompanion(smCHAR *monster, smCHAR *target);
void ApplyIncomingDamage(smCHAR *target, smCHAR *attacker, int *damage);
void ApplyOutgoingBasicDamage(smCHAR *attacker, smCHAR *target, int *damage);
void ApplyOutgoingBasicPostAbsorption(smCHAR *attacker, smCHAR *target, int *damage);
void ApplyAI(smCHAR *character);
// Publishes the client-only visual runtime packet after the authoritative
// server character tick has finalized movement/motion for this frame.
void PublishVisualState(smCHAR *character);
// Returns the companion-specific formation point in fixed-point world units.
// False means the caller should use the owner position unchanged.
bool GetFollowPosition(smCHAR *character, int *x, int *z);

// Companion EXP uses the exact monster-level penalty and ExpManager level
// multiplier, then applies the same party-share rate as a normal party member.
INT64 AwardExperience(rsPLAYINFO *owner, smCHAR *killedMonster, float partyShareRate);

// Runtime tuning / bridge for the configurable crystal item.
DWORD GetConfiguredCrystalItemCode();
int HandleCrystalUse(rsPLAYINFO *owner, DWORD itemCode);

int SetMode(rsPLAYINFO *owner, CompanionMode mode);
CompanionMode GetMode(rsPLAYINFO *owner);
const char *GetSelectedName(rsPLAYINFO *owner);

// Structured client UI protocol. These actions intentionally bypass chat so
// companion management can evolve without adding more slash commands.
void SendUIState(rsPLAYINFO *owner, int lastResult = 0);
void HandleUIAction(rsPLAYINFO *owner, const protocol::CompanionUIActionPacket *packet);

} // namespace companions
} // namespace gameplay
