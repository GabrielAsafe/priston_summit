#pragma once

class smCHAR;

namespace gameplay
{
namespace companions
{
int GetPassiveEvadePercent(smCHAR* character);
int GetReactiveDamage(smCHAR* character, int receivedDamage);
bool CanMonsterTargetCompanion(smCHAR* monster, smCHAR* target);
void ApplyIncomingDamage(smCHAR* target, smCHAR* attacker, int* damage);
void ApplyOutgoingBasicDamage(smCHAR* attacker, smCHAR* target, int* damage);
void ApplyOutgoingBasicPostAbsorption(smCHAR* attacker, smCHAR* target, int* damage);
void ApplyAI(smCHAR* character);
void PublishVisualState(smCHAR* character);
bool GetFollowPosition(smCHAR* character, int* x, int* z);
}
}
