#include "opcodes.h"
#include <windows.h>
#include <stdio.h>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/network/transport/legacy/smwsock.h"

#include "systems/gameplay/character/core/character.h"
#include "core/gameplay/runtime/playmain.h"
#include "systems/audio/effects/legacy/effectsnd.h"
#include "core/data/legacy_loader/fileread.h"
#include "systems/network/client/netplay.h"
#include "systems/rendering/shadows/legacy/makeshadow.h"
#include "systems/rendering/effects/particle/legacy/particle.h"
#include "systems/rendering/effects/legacy_ho/core/HoMinMax.h"
#include "systems/rendering/effects/particles/ho/HoNewParticle.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleMgr.h"
#include "systems/world/field/field.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "compat/legacy/sin/sinLinkHeader.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "systems/combat/skills/client/SkillSub.h"
#include "systems/combat/damage/damage.h"
#include "systems/network/region/AreaServer.h"

#include "systems/social/party/ui/cPartyWindow.h"

static sSKILL Skill_ScoutHawk;


// Summit Tier 5 runtime visual probe. Diagnostic only: it does not alter skill routing.
static void SummitT5VisualProbe(const char *skill, const char *stage, smCHAR *pcActor, smCHAR *pcTarget,
	int beginResult, int motionResult)
{
	FILE *f = fopen("LOGS\\summit_t5_visual_probe.log", "a");
	if(!f) return;

	smMOTIONINFO *motion = pcActor ? pcActor->MotionInfo : 0;
	fprintf(f,
		"tick=%lu skill=%s stage=%s actor=%p target=%p attack=0x%08lX begin=%d motion=%d "
		"state=%ld key=%d start=%ld end=%ld ev=%ld,%ld,%ld,%ld loop=%d speed=%d\n",
		(unsigned long)GetTickCount(), skill ? skill : "?", stage ? stage : "?",
		(void *)pcActor, (void *)pcTarget,
		(unsigned long)(pcActor ? pcActor->AttackSkil : 0), beginResult, motionResult,
		(long)(motion ? motion->State : -1), motion ? (int)motion->KeyCode : -1,
		(long)(motion ? motion->StartFrame : 0), (long)(motion ? motion->EndFrame : 0),
		(long)(motion ? motion->EventFrame[0] : 0), (long)(motion ? motion->EventFrame[1] : 0),
		(long)(motion ? motion->EventFrame[2] : 0), (long)(motion ? motion->EventFrame[3] : 0),
		pcActor ? pcActor->MotionLoop : -1, pcActor ? pcActor->MotionLoopSpeed : -1);
	fclose(f);
}

// Summit Tier 5: canonical EU particle scripts are loaded by the legacy Ho particle manager.
// Visual animation routing stays separate from the Summit server-authority play code.
static void SummitT5StartParticle(const char *name, smCHAR *pcChar)
{
	if(!name || !pcChar) return;
	POINT3D pos = { pcChar->pX, pcChar->pY, pcChar->pZ };
	int particleID = g_NewParticleMgr.Start((char *)name, pos);

	FILE *f = fopen("LOGS\\summit_t5_visual_probe.log", "a");
	if(f)
	{
		fprintf(f, "tick=%lu skill=particle stage=start name=%s actor=%p particle_id=%d pos=%d,%d,%d\n",
			(unsigned long)GetTickCount(), name, (void *)pcChar, particleID, pos.x, pos.y, pos.z);
		fclose(f);
	}
}

static int SummitT5StartParticleAt(const char *name, int x, int y, int z)
{
	if(!name) return -1;
	POINT3D pos = { x, y, z };
	return g_NewParticleMgr.Start((char *)name, pos);
}

struct SummitT5PersistentParticleState
{
	smCHAR *Actor;
	int LogicalSkillCode;
	int ParticleID;
	DWORD EndTick;
};

static SummitT5PersistentParticleState gSummitT5PersistentParticles[16] = {};

static int SummitT5ContinueSkillActive(int skillCode)
{
	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag && ContinueSkill[i].CODE == skillCode)
			return TRUE;
	}
	return FALSE;
}

static void SummitT5PersistentParticleStop(SummitT5PersistentParticleState *state)
{
	if(!state) return;
	if(state->ParticleID >= 0 && g_NewParticleMgr.GetRunning(state->ParticleID))
		g_NewParticleMgr.SetFastStop(state->ParticleID);
	ZeroMemory(state, sizeof(*state));
	state->ParticleID = -1;
}

static int SummitT5AttachParticleToChar(smCHAR *actor, int logicalSkillCode, const char *name, DWORD durationMs)
{
	if(!actor || !name || !durationMs) return -1;

	SummitT5PersistentParticleState *slot = 0;
	for(int i = 0; i < 16; i++)
	{
		if(gSummitT5PersistentParticles[i].Actor == actor &&
			gSummitT5PersistentParticles[i].LogicalSkillCode == logicalSkillCode)
		{
			SummitT5PersistentParticleStop(&gSummitT5PersistentParticles[i]);
			slot = &gSummitT5PersistentParticles[i];
			break;
		}
		if(!slot && !gSummitT5PersistentParticles[i].Actor)
			slot = &gSummitT5PersistentParticles[i];
	}
	if(!slot) return -1;

	POINT3D pos = { actor->pX, actor->pY, actor->pZ };
	int particleID = g_NewParticleMgr.Start((char *)name, pos);
	if(particleID < 0) return particleID;

	slot->Actor = actor;
	slot->LogicalSkillCode = logicalSkillCode;
	slot->ParticleID = particleID;
	slot->EndTick = GetTickCount() + durationMs;
	return particleID;
}

void SummitT5PersistentParticleTick()
{
	DWORD now = GetTickCount();
	for(int i = 0; i < 16; i++)
	{
		SummitT5PersistentParticleState *state = &gSummitT5PersistentParticles[i];
		if(!state->Actor) continue;

		smCHAR *actor = state->Actor;
		if(actor->Flag == 0 || !actor->MotionInfo ||
			actor->MotionInfo->State == CHRMOTION_STATE_DEAD ||
			(actor == lpCurPlayer && !SummitT5ContinueSkillActive(state->LogicalSkillCode)) ||
			(int)(now - state->EndTick) >= 0 ||
			state->ParticleID < 0 || !g_NewParticleMgr.GetRunning(state->ParticleID))
		{
			SummitT5PersistentParticleStop(state);
			continue;
		}

		D3DXVECTOR3 attachPos;
		attachPos.x = (float)actor->pX;
		attachPos.y = (float)actor->pY;
		attachPos.z = (float)actor->pZ;
		g_NewParticleMgr.SetAttachPos(state->ParticleID, attachPos);
	}
}

struct SummitT5MotionEventState
{
	smCHAR *Actor;
	smCHAR *Target;
	int LogicalSkillCode;
	int NetworkSkillCode;
	int Level;
	int FinalEvent;
	int LastEvent;
	int Active;
};

static SummitT5MotionEventState gSummitT5MotionEventStates[16] = {};

static SummitT5MotionEventState *SummitT5FindMotionEventState(smCHAR *actor, int create)
{
	if(!actor) return 0;
	SummitT5MotionEventState *freeState = 0;
	for(int i = 0; i < 16; i++)
	{
		if(gSummitT5MotionEventStates[i].Active && gSummitT5MotionEventStates[i].Actor == actor)
			return &gSummitT5MotionEventStates[i];
		if(!gSummitT5MotionEventStates[i].Active && !freeState)
			freeState = &gSummitT5MotionEventStates[i];
	}
	return create ? freeState : 0;
}

void SummitT5MotionEventArm(smCHAR *actor, int logicalSkillCode, int networkSkillCode, int level, smCHAR *target, int finalEvent)
{
	SummitT5MotionEventState *state = SummitT5FindMotionEventState(actor, TRUE);
	if(!state) return;
	state->Actor = actor;
	state->Target = target;
	state->LogicalSkillCode = logicalSkillCode;
	state->NetworkSkillCode = networkSkillCode & 0xFF;
	state->Level = level;
	state->FinalEvent = finalEvent;
	state->LastEvent = 0;
	state->Active = TRUE;
}

void SummitT5MotionEventClear(smCHAR *actor)
{
	SummitT5MotionEventState *state = SummitT5FindMotionEventState(actor, FALSE);
	if(!state) return;
	ZeroMemory(state, sizeof(*state));
}

void SummitT5MotionEventEnd(smCHAR *actor)
{
	SummitT5MotionEventClear(actor);
}

static POINT3D SummitT5MotionImpactPosition(smCHAR *actor, smCHAR *target)
{
	POINT3D pos = { 0, 0, 0 };
	if(actor && actor->GetAttackPoint(&pos.x, &pos.y, &pos.z) == TRUE)
		return pos;
	if(target)
	{
		pos.x = target->pX;
		pos.y = target->pY + 24 * fONE;
		pos.z = target->pZ;
		return pos;
	}
	if(actor)
	{
		pos.x = actor->pX;
		pos.y = actor->pY;
		pos.z = actor->pZ;
	}
	return pos;
}

int SummitT5MotionEventDispatch(smCHAR *actor)
{
	SummitT5MotionEventState *state = SummitT5FindMotionEventState(actor, FALSE);
	if(!state || !state->Active) return FALSE;

	int eventIndex = actor->MotionEvent;
	if(eventIndex <= state->LastEvent) return TRUE;
	state->LastEvent = eventIndex;

	if(state->Level < 1 || state->Level > 10)
	{
		SummitT5MotionEventClear(actor);
		return TRUE;
	}

	int networkSkill = state->NetworkSkillCode | (state->Level << 8);
	smCHAR *target = state->Target ? state->Target : actor->chrAttackTarget;
	POINT3D impact = SummitT5MotionImpactPosition(actor, target);

	switch(state->LogicalSkillCode)
	{
	case SKILL_T5_RUPTURE:
		if(eventIndex == 1)
		{
			SkillPlaySound(0x4855, actor->pX, actor->pY, actor->pZ);
		}
		else if(eventIndex == 2 || eventIndex == 3)
		{
			if(eventIndex == 2)
				SummitT5StartParticleAt("MC5RuptureHit", impact.x, impact.y, impact.z);
			else
			{
				SetDynLight(impact.x, impact.y, impact.z, 0, 140, 0, 0, 200);
				SkillPlaySound(0x4856, actor->pX, actor->pY, actor->pZ);
				SummitT5StartParticleAt("MC5RuptureFinalHit", impact.x, impact.y, impact.z);
			}

			if(actor == lpCurPlayer && target && actor->AttackCritcal >= 0)
			{
				int baseAttackRating = actor->smCharInfo.Attack_Rating;
				int attackRatingBonus = (baseAttackRating * Grand_Smash_AttackRate[state->Level - 1]) / 100;
				if(eventIndex == 2) attackRatingBonus = (attackRatingBonus * 70) / 100;
				actor->smCharInfo.Attack_Rating = baseAttackRating + attackRatingBonus;
				int result = dm_SendTransDamage(target, 0, 0, 0, 0, networkSkill, TRUE);
				actor->smCharInfo.Attack_Rating = baseAttackRating;
				if(!result) actor->AttackCritcal = -1;
				else DamageExp(target, result);
			}
		}
		break;

	case SKILL_T5_SEISMIC_IMPACT:
		SetDynLight(impact.x, impact.y, impact.z, 200, 0, 0, 0, 400);
		if(eventIndex == 3)
		{
			PlayWaponSoundDirect(actor->pX, actor->pY, actor->pZ, 13);
		}
		else if(eventIndex == 1 || eventIndex == 2 || eventIndex == 4)
		{
			SkillPlaySound(SKILL_SOUND_SKILL_AVANGING_CRASH, actor->pX, actor->pY, actor->pZ);
			SummitT5StartParticleAt("R5SesmicImpact", impact.x, impact.y, impact.z);
			if(actor == lpCurPlayer && target)
			{
				if(eventIndex == 4)
				{
					dm_SelectRange(target->pX, target->pY, target->pZ, Fighter_T5_Seismic_Area[state->Level - 1], TRUE);
					dm_AddRangeDamage(target);
					dm_SendRangeDamage(target->pX, target->pY, target->pZ, target, 0, 0, 0, 0, networkSkill);
				}
				else
				{
					int result = dm_SendTransDamage(target, 0, 0, 0, 0, networkSkill, TRUE);
					if(!result) actor->AttackCritcal = -1;
					else DamageExp(target, result);
				}
			}
		}
		break;

	case SKILL_T5_RING_OF_SPEARS:
		if(eventIndex == 1)
			SkillPlaySound(0x4857, actor->pX, actor->pY, actor->pZ);
		break;

	case SKILL_T5_WIZARD_TRANCE:
		if(eventIndex == 1)
		{
			// EU also starts MG5WizardTrance.luac here. Summit does not yet have
			// the matching owner-aware .luac particle engine path, so only preserve
			// the verified MotionEvent sound instead of inventing a fallback visual.
			SkillPlaySound(SKILL_SOUND_SKILL_ENERGY_SHIELD, actor->pX, actor->pY, actor->pZ);
		}
		break;

	case SKILL_T5_FIERCE_WIND:
		if(eventIndex == 1)
		{
			SummitT5StartParticleAt("Ac5FierceWindProjectile", impact.x, impact.y, impact.z);
		}
		else if(eventIndex == 2 && target)
		{
			SkillPlaySound(SKILL_SOUND_SKILL_EXTREM_RAGE1, actor->pX, actor->pY, actor->pZ);
			if(actor == lpCurPlayer)
			{
				RECT rect;
				rect.left = -50;
				rect.right = 50;
				rect.top = 0;
				rect.bottom = 340;
				dm_SelectRangeBox(actor, &rect, FALSE);
				dm_SendRangeDamage(target->pX, target->pY, target->pZ, target, 0, 0, 0, 0, networkSkill);
			}
		}
		break;
	}

	if(state->FinalEvent > 0 && eventIndex >= state->FinalEvent)
		SummitT5MotionEventClear(actor);
	return TRUE;
}

// Summit Archer T5-4 / Bombardment channel state.
static int gSummitBombardmentPoint = 0;
static DWORD gSummitBombardmentEndTime = 0;
static DWORD gSummitBombardmentNextTick = 0;
static smCHAR *gSummitBombardmentTarget = 0;

static int SummitArcherBombardmentIsActive()
{
	return gSummitBombardmentPoint >= 1 && gSummitBombardmentPoint <= 10 && gSummitBombardmentEndTime != 0;
}

static void SummitArcherBombardmentStop(int notifyServer)
{
	int point = gSummitBombardmentPoint;
	gSummitBombardmentPoint = 0;
	gSummitBombardmentEndTime = 0;
	gSummitBombardmentNextTick = 0;
	gSummitBombardmentTarget = 0;
	if(notifyServer && point >= 1 && point <= 10)
		SendCancelSkillToServer(SKILL_PLAY_T54, point, 0, 0);
}

void SummitArcherBombardmentTick()
{
	if(!SummitArcherBombardmentIsActive()) return;
	if(!lpCurPlayer || !gSummitBombardmentTarget)
	{
		SummitArcherBombardmentStop(FALSE);
		return;
	}

	int point = gSummitBombardmentPoint - 1;
	DWORD now = dwPlayTime;
	if(point < 0 || point >= 10 || sinGetLife() <= 0 || now >= gSummitBombardmentEndTime ||
	   sinSkill.UseSkill[20].CODE != SKILL_T5_BOMBARDMENT || sinSkill.UseSkill[20].Point != gSummitBombardmentPoint ||
	   (lpCurPlayer->dwActionItemCode & sinITEM_MASK2) != sinWS1 ||
	   !gSummitBombardmentTarget->Flag || gSummitBombardmentTarget->smCharInfo.Life[0] <= 0 ||
	   (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_VILLAGE))
	{
		SummitArcherBombardmentStop(TRUE);
		return;
	}

	if(now < gSummitBombardmentNextTick) return;

	int manaDrain = Archer_T5_Bombard_MPDrain[point];
	int staminaDrain = Archer_T5_Bombard_SPDrain[point];
	if(manaDrain <= 0 || staminaDrain <= 0 || sinGetMana() < manaDrain || sinGetStamina() < staminaDrain)
	{
		SummitArcherBombardmentStop(TRUE);
		return;
	}

	sinSetMana((short)(sinGetMana() - manaDrain));
	sinSetStamina((short)(sinGetStamina() - staminaDrain));
	gSummitBombardmentNextTick = now + 1000;

	lpCurPlayer->AttackSkil = SKILL_PLAY_T54 | (gSummitBombardmentPoint << 8);
	int summitBombBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, gSummitBombardmentTarget, 0, 0, 0);
	int summitBombMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
	SummitT5VisualProbe("Archer/Bombardment", "tick-motion", lpCurPlayer, gSummitBombardmentTarget, summitBombBegin, summitBombMotion);
	lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
	lpCurPlayer->MotionLoop = 1;
	int result = dm_SendTransDamage(gSummitBombardmentTarget, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
	if(!result) lpCurPlayer->AttackCritcal = -1;
}

// Summit Magician T5-4 / Cataclysm channel state. EU exposes duration, damage boost and
// MP/SP drain but the legacy handler is binary-only; Summit uses a documented 1 Hz
// target-lock cadence as the least-speculative compatibility implementation.
static int gSummitCataclysmPoint = 0;
static DWORD gSummitCataclysmEndTime = 0;
static DWORD gSummitCataclysmNextTick = 0;
static smCHAR *gSummitCataclysmTarget = 0;

static int SummitMagicianCataclysmIsActive()
{
	return gSummitCataclysmPoint >= 1 && gSummitCataclysmPoint <= 10 && gSummitCataclysmEndTime != 0;
}

static void SummitMagicianCataclysmStop(int notifyServer)
{
	int point = gSummitCataclysmPoint;
	gSummitCataclysmPoint = 0;
	gSummitCataclysmEndTime = 0;
	gSummitCataclysmNextTick = 0;
	gSummitCataclysmTarget = 0;
	if(notifyServer && point >= 1 && point <= 10)
		SendCancelSkillToServer(SKILL_PLAY_T54, point, 0, 0);
}

void SummitMagicianCataclysmTick()
{
	if(!SummitMagicianCataclysmIsActive()) return;
	if(!lpCurPlayer || !gSummitCataclysmTarget)
	{
		SummitMagicianCataclysmStop(FALSE);
		return;
	}

	int point = gSummitCataclysmPoint - 1;
	DWORD now = dwPlayTime;
	if(point < 0 || point >= 10 || sinGetLife() <= 0 || now >= gSummitCataclysmEndTime ||
	   sinSkill.UseSkill[20].CODE != SKILL_T5_CATACLYSM || sinSkill.UseSkill[20].Point != gSummitCataclysmPoint ||
	   (lpCurPlayer->dwActionItemCode & sinITEM_MASK2) != sinWM1 ||
	   !gSummitCataclysmTarget->Flag || gSummitCataclysmTarget->smCharInfo.Life[0] <= 0 ||
	   (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_VILLAGE))
	{
		SummitMagicianCataclysmStop(TRUE);
		return;
	}

	if(now < gSummitCataclysmNextTick) return;

	int manaDrain = Magician_T5_Cataclysm_MPDrain[point];
	int staminaDrain = Magician_T5_Cataclysm_SPDrain[point];
	if(manaDrain <= 0 || staminaDrain <= 0 || sinGetMana() < manaDrain || sinGetStamina() < staminaDrain)
	{
		SummitMagicianCataclysmStop(TRUE);
		return;
	}

	sinSetMana((short)(sinGetMana() - manaDrain));
	sinSetStamina((short)(sinGetStamina() - staminaDrain));
	gSummitCataclysmNextTick = now + 1000;

	lpCurPlayer->AttackSkil = SKILL_PLAY_T54 | (gSummitCataclysmPoint << 8);
	int summitCatBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, gSummitCataclysmTarget, 0, 0, 0);
	int summitCatMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
	SummitT5VisualProbe("Magician/Cataclysm", "tick-motion", lpCurPlayer, gSummitCataclysmTarget, summitCatBegin, summitCatMotion);
	lpCurPlayer->MotionLoopSpeed = 80;
	lpCurPlayer->MotionLoop = 1;
	int result = dm_SendTransDamage(gSummitCataclysmTarget, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
	if(!result) lpCurPlayer->AttackCritcal = -1;
}

// Summit Fighter T5-4 / Raging Strikes (Whirlwind) channel state.
static int gSummitWhirlwindPoint = 0;
static DWORD gSummitWhirlwindEndTime = 0;
static DWORD gSummitWhirlwindNextTick = 0;

static int SummitWhirlwindIsActive()
{
	return gSummitWhirlwindPoint >= 1 && gSummitWhirlwindPoint <= 10 && gSummitWhirlwindEndTime != 0;
}

static void SummitWhirlwindStop(int notifyServer)
{
	int point = gSummitWhirlwindPoint;
	gSummitWhirlwindPoint = 0;
	gSummitWhirlwindEndTime = 0;
	gSummitWhirlwindNextTick = 0;
	if(notifyServer && point >= 1 && point <= 10)
		SendCancelSkillToServer(SKILL_PLAY_T54, point, 0, 0);
}

void SummitWhirlwindTick()
{
	if(!SummitWhirlwindIsActive()) return;
	if(!lpCurPlayer)
	{
		SummitWhirlwindStop(FALSE);
		return;
	}

	int point = gSummitWhirlwindPoint - 1;
	DWORD now = dwPlayTime;
	if(point < 0 || point >= 10 || sinGetLife() <= 0 || now >= gSummitWhirlwindEndTime ||
	   sinSkill.UseSkill[20].CODE != SKILL_T5_WHIRLWIND || sinSkill.UseSkill[20].Point != gSummitWhirlwindPoint ||
	   (lpCurPlayer->dwActionItemCode & sinITEM_MASK2) != sinWA1 ||
	   (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_VILLAGE))
	{
		SummitWhirlwindStop(TRUE);
		return;
	}

	if(now < gSummitWhirlwindNextTick) return;

	int manaDrain = Fighter_T5_Whirlwind_MPDrain[point];
	int staminaDrain = Fighter_T5_Whirlwind_SPDrain[point];
	if(manaDrain <= 0 || staminaDrain <= 0 || sinGetMana() < manaDrain || sinGetStamina() < staminaDrain)
	{
		SummitWhirlwindStop(TRUE);
		return;
	}

	sinSetMana((short)(sinGetMana() - manaDrain));
	sinSetStamina((short)(sinGetStamina() - staminaDrain));
	gSummitWhirlwindNextTick = now + 1000;

	lpCurPlayer->AttackSkil = SKILL_PLAY_T54 | (gSummitWhirlwindPoint << 8);
	int summitWhirlBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
	int summitWhirlMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
	SummitT5VisualProbe("Fighter/RagingStrikes", "tick-motion", lpCurPlayer, 0, summitWhirlBegin, summitWhirlMotion);
	lpCurPlayer->MotionLoopSpeed = 80;
	lpCurPlayer->MotionLoop = 1;

	dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,
		Cyclone_Strike_Area[point], TRUE, Cyclone_Strike_AttackNum[point]);
	dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,
		0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
}






int OpenPlaySkill(sSKILL *lpSkill)
{
	int flag;

	int cnt;
	short wpDamage[2];


	pUseSkill = lpSkill;

	flag = 0;

	if(lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_VILLAGE)
		return flag;

	if(sinCheckSkillUseOk(lpSkill) == FALSE) return flag;
	if(lpSkill->Point > 10) return flag;

	wpDamage[0] = cInvenTory.InvenItem[sInven[0].ItemIndex - 1].sItemInfo.Damage[0];
	wpDamage[1] = cInvenTory.InvenItem[sInven[0].ItemIndex - 1].sItemInfo.Damage[1];

	lpSkill->SkillTaget_CODE = 0;

	switch(lpSkill->Skill_Info.CODE)
	{

		case SKILL_EXTREME_SHIELD:

		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->BeginSkill(SKILL_PLAY_EXTREME_SHIELD, 0, 0, 0, 0, 0);
				lpCurPlayer->AttackSkil = SKILL_PLAY_EXTREME_SHIELD;
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 100;
				lpCurPlayer->MotionLoop = 1;
				flag++;
			}
		}
		break;

		case SKILL_PHYSICAL_ABSORB:

		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_PHYSICAL_ABSORB;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_PHYSICAL_ABSORB, lpSkill->Point, 0, 0);


				SkillPlaySound(SKILL_SOUND_PHYSICAL_ABSORB3, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, TRUE);
				flag++;
			}
		}
		break;

		case SKILL_POISON_ATTRIBUTE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->BeginSkill(SKILL_PLAY_POISON_ATTRIBUTE, 0, 0, 0, 0, 0);
			flag++;
		}
		break;


		case SKILL_PIKE_WIND:


		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->BeginSkill(SKILL_PLAY_PIKEWIND, 0, 0, 0, 0, 0);
				lpCurPlayer->AttackSkil = SKILL_PLAY_PIKEWIND;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;



				dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Pike_Wind_Push_Lenght[lpSkill->Point - 1], FALSE);

				dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

				flag++;
			}
		}
		break;

		case SKILL_SCOUT_HAWK:

		if(lpCurPlayer->HoSkillCode != SKILL_SCOUT_HAWK && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			lpCurPlayer->AttackSkil = SKILL_PLAY_SCOUT_HAWK;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			memcpy(&Skill_ScoutHawk, lpSkill, sizeof(sSKILL));
			lpCurPlayer->HideWeapon = TRUE;
			flag++;
		}
		break;




		case SKILL_MAXIMIZE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE && lpSkill->Point)
		{

			lpCurPlayer->AttackSkil = SKILL_PLAY_MAXIMIZE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;

			flag++;
		}
		break;

		case SKILL_AUTOMATION:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE && lpSkill->Point)
		{

			lpCurPlayer->AttackSkil = SKILL_PLAY_AUTOMATION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;

			SkillPlaySound(SKILL_SOUND_SKILL_AUTOMATION3, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, TRUE);

			flag++;
		}
		break;

		case SKILL_GROUND_PIKE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_GROUND_PIKE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;



				dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, G_Pike_Range[lpSkill->Point - 1], FALSE);

				dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

				flag++;
			}
		}
		break;


		case SKILL_VENOM_SPEAR:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_VENOM_SPEAR;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				flag++;
			}
		}
		break;

		case SKILL_VANISH:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_VANISH;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				flag++;
			}
		}
		break;



		case SKILL_FALCON:

		if(lpCurPlayer->HoSkillCode != SKILL_FALCON && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_FALCON;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				lpCurPlayer->HideWeapon = TRUE;

				SkillFalconPoint = (lpSkill->Point - 1) ^ lpCurPlayer->dwObjectSerial ^ ((DWORD)chrOtherPlayer);

				SendProcessSKillToServer(SKILL_PLAY_FALCON, lpSkill->Point, 0, 0);
			}
		}
		break;

		case SKILL_GOLDEN_FALCON:

		if(lpCurPlayer->HoSkillCode != SKILL_GOLDEN_FALCON && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_GOLDEN_FALCON;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				lpCurPlayer->HideWeapon = TRUE;

				SkillFalconPoint = (lpSkill->Point - 1) ^ lpCurPlayer->dwObjectSerial ^ ((DWORD)chrOtherPlayer);

				cnt = cSkill.GetSkillPoint(SKILL_FALCON);
				SendProcessSKillToServer(SKILL_PLAY_GOLDEN_FALCON, lpSkill->Point, cnt, 0);
			}
		}
		break;


		case SKILL_ROAR:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_ROAR;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;





				dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Roar_Range[lpSkill->Point - 1], FALSE);

				dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

				flag++;
			}
		}
		break;

		case SKILL_CONCENTRATION:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_CONCENTRATION;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				flag++;
			}
		}
		break;

		case SKILL_SWIFT_AXE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			if(lpSkill->Point)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_SWIFT_AXE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				flag++;
			}
		}
		break;



		case SKILL_METAL_ARMOR:

		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_METAL_ARMOR;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				cnt = cSkill.GetSkillPoint(SKILL_PHYSICAL_ABSORB);

				SendProcessSKillToServer(SKILL_PLAY_METAL_ARMOR, lpSkill->Point, cnt, 0);


				flag++;
			}
		}
		break;

		case SKILL_SPARK_SHIELD:

		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_SPARK_SHIELD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				cnt = cSkill.GetSkillPoint(SKILL_SPARK);

				SendProcessSKillToServer(SKILL_PLAY_SPARK_SHIELD, lpSkill->Point, cnt, 0);


				flag++;
			}
		}
		break;








		case SKILL_HOLY_BODY:

		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_BODY;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_HOLY_BODY, lpSkill->Point, 0, 0);


				SkillPlaySound(SKILL_SOUND_PHYSICAL_ABSORB3, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, TRUE);
				flag++;
			}
		}
		break;

		case SKILL_HOLY_VALOR:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_VALOR;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
			flag++;
		}
		break;

		case SKILL_DRASTIC_SPIRIT:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_DRASTIC_SPIRIT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
			flag++;
		}
		break;

		case SKILL_DIVINE_INHALATION:

		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_INHALATION;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_DIVINE_INHALATION, lpSkill->Point, 0, 0);



				flag++;
			}
		}
		break;


		case SKILL_WINDY:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_WINDY;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			flag++;
		}
		break;

		case SKILL_TRIUMPH_OF_VALHALLA:

		if(!lpCharSelPlayer &&
		   GetAssaUseEffect(lpCurPlayer, SKILL_TRIUMPH_OF_VALHALLA) == FALSE &&
		   !cSkill.SearchContiueSkill(SKILL_HALL_OF_VALHALLA) &&
		   lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			lpCurPlayer->AttackSkil = SKILL_PLAY_TRIUMPH_OF_VALHALLA;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			SendProcessSKillToServer(SKILL_PLAY_TRIUMPH_OF_VALHALLA, lpSkill->Point, 0, lpCurPlayer->smCharInfo.Level);
			lpCurPlayer->chrAttackTarget = 0;
			cSkill.SetT_Of_Valhalla(lpSkill->Point, lpCurPlayer->smCharInfo.Level);

			flag++;
		}
		break;



		case SKILL_HEALING:

		if(lpCurPlayer->MotionInfo->State < 0x100 && !lpCharSelPlayer)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_HEALING;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				cnt = GetRandomPos(Healing_Heal[lpSkill->Point - 1][0] + wpDamage[0], Healing_Heal[lpSkill->Point - 1][1] + wpDamage[0]);

				sinSetLife(sinGetLife() + cnt);
				ResetEnergyGraph(0);



				flag++;
			}
		}
		break;

		case SKILL_DIVINE_LIGHTNING:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{


			lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_LIGHTNING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = 85;
			lpCurPlayer->MotionLoop = 1;

			flag++;
		}
		break;

		case SKILL_HOLY_REFLECTION:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_REFLECTION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			flag++;
		}
		break;
		case SKILL_GRAND_HEALING:
		if(cParty.GetMemberCount() > 0 && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_GREAT_HEALING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;

			if(AreaServerMode)
				dm_SendTransDamage(lpCurPlayer, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil, FALSE, smWsockServer);
			else
				dm_SendTransDamage(lpCurPlayer, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil, FALSE);

			flag++;
		}
		break;

		case SKILL_RESURRECTION:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_RESURRECTION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			flag++;
		}
		break;

		case SKILL_EXTINCTION:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_EXTINCTION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 160, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

			flag++;
		}
		break;

		case SKILL_VIRTUAL_LIFE: //VL GRUPO SACER
			if(!lpCharSelPlayer && GetAssaUseEffect(lpCurPlayer, SKILL_VIRTUAL_LIFE) == FALSE && lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_VIRTUAL_LIFE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

				for(int i = 0; i < InterfaceParty.PartyMemberCount; i++)
				{
					if(InterfaceParty.PartyMember[i].ChrCode != lpCurPlayer->dwObjectSerial)
					{
						int x = InterfaceParty.PartyMember[i].CharacterPos.x - lpCurPlayer->pX;
						int y = InterfaceParty.PartyMember[i].CharacterPos.z - lpCurPlayer->pZ;

						x >>= FLOATNS;
						y >>= FLOATNS;

						int dist = x * x + y * y;
						if(dist < PARTY_GETTING_DIST)
						{
							SendProcessSKillToServer(SKILL_PLAY_VIRTUAL_LIFE, lpSkill->Point, InterfaceParty.PartyMember[i].ChrCode, 0);
							AssaParticle_VirtualLife(FindChrPlayer(InterfaceParty.PartyMember[i].ChrCode), Virtual_Life_Time[lpSkill->Point - 1]);
						}
					}
				}

				SendProcessSKillToServer(SKILL_PLAY_VIRTUAL_LIFE, lpSkill->Point, 0, 0);
				cSkill.SetVirtualLife(Virtual_Life_Time[lpSkill->Point - 1], lpSkill->Point);
				lpCurPlayer->chrAttackTarget = 0;

				flag++;
			}
			break;


		case SKILL_AGONY:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_AGONY;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;
				flag++;


				ResetEnergyGraph(3);
			}
		}
		break;

		case SKILL_ZENITH:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_ZENITH;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;



				SendResistanceToServer();

				flag++;
			}
		}
		break;

		case SKILL_ENCHANT_WEAPON:
		if(lpCurPlayer->MotionInfo->State < 0x100 && !lpCharSelPlayer && !lpCurPlayer->WeaponEffect)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->chrAttackTarget = 0;

				lpCurPlayer->AttackSkil = SKILL_PLAY_ENCHANT_WEAPON;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				cnt = rand() % 3;
				lpCurPlayer->AttackSkil |= (cnt << 12);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;
				flag++;
			}
		}
		break;


		case SKILL_ENERGY_SHIELD:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_ENERGY_SHIELD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_ENERGY_SHIELD, lpSkill->Point, 0, 0);

				flag++;
			}
		}
		break;

		case SKILL_DIASTROPHISM:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_DIASTROPHISM;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 120;
				lpCurPlayer->MotionLoop = 1;
				flag++;


			}
		}
		break;

		case SKILL_SPIRIT_ELEMENTAL:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_SPIRIT_ELEMENTAL;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_SPIRIT_ELEMENTAL, lpSkill->Point, 0, 0);

				flag++;
			}
		}
		break;

		case SKILL_DANCING_SWORD:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_DANCING_SWORD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				cnt = rand() % 2;
				lpCurPlayer->AttackSkil |= (cnt << 12);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_DANCING_SWORD, lpSkill->Point, cnt, 0);

				flag++;
			}

		}
		break;




		case SKILL_COMPULSION:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_COMPULSION;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;


				dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Compulsion_Area[lpSkill->Point - 1], FALSE);

				dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

				flag++;
			}
		}
		break;

		case SKILL_MAGNETIC_SPHERE:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_MAGNETIC_SPHERE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_MAGNETIC_SPHERE, lpSkill->Point, 0, 0);

				flag++;
			}
		}
		break;

		case SKILL_METAL_GOLEM:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_METAL_GOLEM;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_METAL_GOLEM, lpSkill->Point, 0, 0);

				flag++;
			}
		}
		break;


		case SKILL_BERSERKER:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_BERSERKER;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_BERSERKER, lpSkill->Point, 0, 0);

				flag++;
			}
		}
		break;




		case SKILL_VAGUE:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_VAGUE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_VAGUE, lpSkill->Point, 0, 0);

				flag++;
			}
		}
		break;


		case SKILL_RECALL_WOLVERIN:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_RECALL_WOLVERIN;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				lpCurPlayer->HideWeapon = TRUE;
				flag++;

				SendProcessSKillToServer(SKILL_PLAY_RECALL_WOLVERIN, lpSkill->Point, 0, 0);
			}
		}
		break;

		case SKILL_FORCE_OF_NATURE:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_FORCE_OF_NATURE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_FORCE_OF_NATURE, lpSkill->Point, 0, 0);
				cSkill.ForeceOfNature(Force_Of_Nature_Time[lpSkill->Point - 1], lpSkill->Point);

				lpCurPlayer->dwForceOfNatureTime = dwPlayTime + Force_Of_Nature_Time[lpSkill->Point - 1] * 1000;

				flag++;
			}
		}
		break;


		case SKILL_GODLY_SHIELD:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_GODLY_SHIELD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_GODLY_SHIELD, lpSkill->Point, 0, 0);
				flag++;
			}
		}
		break;

		case SKILL_GOD_BLESS:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_GODS_BLESS;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_GODS_BLESS, lpSkill->Point, 0, 0);
				flag++;
			}
		}
		break;


		case SKILL_HALL_OF_VALHALLA:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(GetAssaUseEffect(lpCurPlayer, SKILL_TRIUMPH_OF_VALHALLA) == FALSE && lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_HALL_OF_VALHALLA;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				cnt = cSkill.GetSkillPoint(SKILL_TRIUMPH_OF_VALHALLA);
				SendProcessSKillToServer(SKILL_PLAY_HALL_OF_VALHALLA, lpSkill->Point, cnt, lpCurPlayer->smCharInfo.Level);

				cSkill.HellOfValhalla(Hall_Of_Valhalla_Time[lpSkill->Point - 1], lpSkill->Point, cnt, lpCurPlayer->smCharInfo.Level, 0);

				lpCurPlayer->dwHallOfValhallaTime = dwPlayTime + Hall_Of_Valhalla_Time[lpSkill->Point - 1] * 1000;

				flag++;
			}
		}
		break;

		case SKILL_FROST_JAVELIN:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_FROST_JAVELIN;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_FROST_JAVELIN, lpSkill->Point, 0, 0);
				flag++;
			}
		}
		break;


		case SKILL_REGENERATION_FIELD:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_REGENERATION_FIELD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				cSkill.Regeneration_Field(Regeneration_Field_Time[lpSkill->Point - 1], lpSkill->Point, 0);
				flag++;
			}
		}
		break;

		case SKILL_SUMMON_MUSPELL:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_SUMMON_MUSPELL;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_SUMMON_MUSPELL, lpSkill->Point, 0, 0);

				flag++;
			}
		}
		break;


		case SKILL_FIRE_ELEMENTAL:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_FIRE_ELEMENTAL;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer(SKILL_PLAY_FIRE_ELEMENTAL, lpSkill->Point, 0, 0);

				flag++;
			}
		}
		break;
		case SKILL_DISTORTION:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{

				lpCurPlayer->AttackSkil = SKILL_PLAY_DISTORTION;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;


				dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Distortion_Area[lpSkill->Point - 1], FALSE);

				dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);


				flag++;
			}

		}
		break;


	}

	if(flag) CheckPlaySkill();

	return flag;
}


int	NormalAttackRange;
int	NormalAttackMode;
int	LastAttackRange;


int GetSkillDistRange(sSKILL* lpSkill, int AttackRange, int AttackMode)
{
	int cnt;

	NormalAttackRange = AttackRange;
	NormalAttackMode = AttackMode;

	switch (lpSkill->CODE)
	{
	case SKILL_MECHANIC_BOMB:
		if (lpSkill->Point)
		{

			cnt = Mechanic_Bomb_Attack_Range[lpSkill->Point - 1];
			return	(cnt * 3 * fONE);
		}
		break;

	case SKILL_SPARK:
		if (lpSkill->Point)
		{

			return	140 * fONE;
		}
		break;

	case SKILL_EXPANSION:

		if (lpSkill->Point)
		{
			cnt = AttackRange + ((AttackRange - 50 * fONE) * Expansion_Size[lpSkill->Point - 1]) / 100;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_VENGEANCE:
		if (lpSkill->Point)
		{
			cnt = AttackRange + Vengeance_Range[lpSkill->Point - 1] * fONE;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_TORNADO:

		if (lpSkill->Point)
		{
			cnt = Tornado_Range[lpSkill->Point - 1];
			if (AttackRange > cnt) break;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_SWORD_BLAST:

		if (lpSkill->Point)
		{
			cnt = Sword_Blast_ShootingRange[lpSkill->Point - 1];
			if (AttackRange > cnt) break;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_SHIELD_STRIKE:

		return	110 * fONE;

	case SKILL_HEALING:

		return	180 * fONE;
	case SKILL_HOLY_BOLT:

		return	170 * fONE;
	case SKILL_MULTISPARK:

		return	210 * fONE;
	case SKILL_HOLY_MIND:

		return	160 * fONE;

	case SKILL_DIVINE_LIGHTNING:
		return 160 * fONE;

	case SKILL_FIRE_BOLT:
		if (lpSkill->Point)
		{

			return	200 * fONE;
		}
		break;
	case SKILL_FIRE_BALL:
		if (lpSkill->Point)
		{

			return	FireBall_Range[lpSkill->Point - 1] * fONE;
		}
		break;

	case SKILL_WATORNADO:
		if (lpSkill->Point)
		{
			return Watornado_Range[lpSkill->Point - 1] * fONE;
		}
		break;

	case SKILL_ENCHANT_WEAPON:
		return	140 * fONE;

	case SKILL_DEAD_RAY:
		if (lpSkill->Point)
		{
			return	Dead_Ray_Range[lpSkill->Point - 1] * fONE;
		}
		break;

	case SKILL_HOLY_INCANTATION:
		return	120 * fONE;

	case SKILL_VIGOR_BALL:
		return	230 * fONE;

	case SKILL_TRIUMPH_OF_VALHALLA:
		return	200 * fONE;

	case SKILL_PERFORATION:
		if (lpSkill->Point)
		{
			return	AttackRange + (Perforation_Attack_Range[lpSkill->Point - 1] * fONE);
		}
		break;

	case SKILL_VIRTUAL_LIFE:
		return 450 * fONE;
	}

	return NULL;
}



int RetryPlayAttack(smCHAR *lpChar)
{

	return lpCurPlayer->PlayAttackFromPosi(lpChar->pX, lpChar->pY, lpChar->pZ,
										   NormalAttackRange, NormalAttackMode);

}



int PlaySkillSubLife(int Life)
{
	int sLife;

	CheckCharForm();

	sLife = sinGetLife();
	sLife -= Life;
	if(sLife <= 0) sLife = 1;
	sinSetLife(sLife);


	return sLife;
}



int PlaySkillAttack(sSKILL *lpSkill, smCHAR *lpChar)
{

	int cnt;
	int	temp;
	int x, y, z;
	smTRANS_COMMAND	smTransCommand;
	RECT	rect;
	smWINSOCK	*lpsmSock;
	short wpDamage[2];
	int	 skFlag;

	lpCurPlayer->AttackSkil = 0;

	if(!lpSkill) return FALSE;

	if(lpSkill->CODE == SKILL_HEALING || lpSkill->CODE == SKILL_ENCHANT_WEAPON ||
	   lpSkill->CODE == SKILL_VIRTUAL_LIFE || lpSkill->CODE == SKILL_TRIUMPH_OF_VALHALLA)
	{
		skFlag = TRUE;
	}
	else
	{
		skFlag = FALSE;
	}


	if(!PkMode && lpChar->smCharInfo.State != smCHAR_STATE_ENEMY && skFlag == FALSE)
		return FALSE;

	if(sinCheckSkillUseOk(lpSkill) == FALSE || lpSkill->Point > 10)
	{
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) return FALSE;;
		return TRUE;
	}

	CheckPlaySkill();

	pUseSkill = lpSkill;


	wpDamage[0] = cInvenTory.InvenItem[sInven[0].ItemIndex - 1].sItemInfo.Damage[0];
	wpDamage[1] = cInvenTory.InvenItem[sInven[0].ItemIndex - 1].sItemInfo.Damage[1];

	lpSkill->SkillTaget_CODE = lpChar->dwObjectSerial;

	switch(lpSkill->CODE)
	{
		case SKILL_EXTREME_SHIELD:

		return TRUE;

		case SKILL_MECHANIC_BOMB:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_MECHANIC_BOMB;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->HideWeapon = TRUE;
		}
		return TRUE;

		case SKILL_PHYSICAL_ABSORB:
		return TRUE;

		case SKILL_RAVING:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_RAVING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);


			if(lpSkill->Point)
			{
				lpCurPlayer->MotionLoop = Ravind_Speed[lpSkill->Point - 1];

				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

				if(!cnt)
				{
					lpCurPlayer->AttackCritcal = -1;
				}
				else
				{

					PlaySkillSubLife(((int)(Raving_UseLife[lpSkill->Point - 1] * 10)*lpCurPlayer->smCharInfo.Life[1]) / 1000);

					DamageExp(lpChar, cnt);
				}

			}
			else
			{
				lpCurPlayer->MotionLoop = 1;
			}

			lpCurPlayer->MotionLoopSpeed = 90 + (10 * lpCurPlayer->MotionLoop);
			if(lpCurPlayer->MotionLoopSpeed < 100) lpCurPlayer->MotionLoopSpeed = 100;





			lpCurPlayer->AttackAnger = 150;
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_IMPACT:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_IMPACT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			if(lpSkill->Point)
			{
				temp = lpCurPlayer->smCharInfo.Attack_Rating;
				lpCurPlayer->smCharInfo.Attack_Rating += Impact_Attack_Rating[lpSkill->Point - 1];

				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
				lpCurPlayer->smCharInfo.Attack_Rating = temp;

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
				{

					DamageExp(lpChar, cnt);
				}
			}
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_PIKE_WIND:
		return TRUE;

		case SKILL_CRITICAL_HIT:

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->AttackSkil = SKILL_PLAY_CRITICAL_HIT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);


			cnt = lpCurPlayer->smCharInfo.Attack_Speed - 6;
			if(cnt < 0) cnt = 0;
			if(cnt > 6) cnt = 6;
			cnt += 2;
			lpCurPlayer->MotionLoopSpeed = (80 * (fONE + 32 * cnt + 32)) >> FLOATNS;


			lpCurPlayer->MotionLoop = 2;
			break;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_JUMPING_CRASH:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{


			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_JUMPING_CRASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			if(lpSkill->Point)
			{
				temp = lpCurPlayer->smCharInfo.Attack_Rating;



				lpCurPlayer->smCharInfo.Attack_Rating +=
					(lpCurPlayer->smCharInfo.Attack_Rating * Jumping_Crash_Attack_Rating[lpSkill->Point - 1]) / 100;

				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
				lpCurPlayer->smCharInfo.Attack_Rating = temp;

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
					DamageExp(lpChar, cnt);
			}
			lpCurPlayer->MotionLoopSpeed = 80;
			lpCurPlayer->MotionLoop = 1;
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_SCOUT_HAWK:

		return TRUE;

		case SKILL_WIND_ARROW:


		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			if(lpCurPlayer->CheckShootingTest(lpChar) == TRUE)
			{

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

				lpCurPlayer->AttackSkil = SKILL_PLAY_WIND_ARROW;
				lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);


				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
					DamageExp(lpChar, cnt);




				lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, Wind_Arrow_Speed[lpSkill->Point - 1]);

				lpCurPlayer->MotionLoop = 1;
			}
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_PERFECT_AIM:



		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_PERFECT_AIM;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			temp = lpCurPlayer->smCharInfo.Attack_Rating;




			lpCurPlayer->smCharInfo.Attack_Rating +=
				(lpCurPlayer->smCharInfo.Attack_Rating * Perfect_Aim_Attack_Rate[lpSkill->Point - 1]) / 100;


			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp(lpChar, cnt);


			lpCurPlayer->smCharInfo.Attack_Rating = temp;




			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;



		case SKILL_GREAT_SMASH:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_GREAT_SMASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = 110;
			lpCurPlayer->MotionLoop = 1;


			if(lpSkill->Point)
			{
				temp = lpCurPlayer->smCharInfo.Attack_Rating;




				lpCurPlayer->smCharInfo.Attack_Rating += G_Smash_Attack_Rate[lpSkill->Point - 1];



				dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, 70, TRUE);

				dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

				lpCurPlayer->smCharInfo.Attack_Rating = temp;
			}
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_MAXIMIZE:
		case SKILL_AUTOMATION:
		return TRUE;

		case SKILL_SPARK:


		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_SPARK;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->HideWeapon = TRUE;
		}
		return TRUE;


		case SKILL_T5_RUPTURE:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			int summitRuptureNetworkSkill = SKILL_PLAY_T51 | (lpSkill->Point << 8);
			lpCurPlayer->AttackSkil = summitRuptureNetworkSkill;
			int summitRuptureBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitRuptureMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Mechanician/Rupture", "cast-motion-control", lpCurPlayer, lpChar, summitRuptureBegin, summitRuptureMotion);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, 4);
			lpCurPlayer->MotionLoop = 1;
			SummitT5MotionEventArm(lpCurPlayer, SKILL_T5_RUPTURE, SKILL_PLAY_T51, lpSkill->Point, lpChar, 3);
			lpCurPlayer->AttackSkil = summitRuptureNetworkSkill;
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_GRAND_SMASH:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_GRAND_SMASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, 4);
			lpCurPlayer->MotionLoop = 1;


			if(lpSkill->Point)
			{
				temp = lpCurPlayer->smCharInfo.Attack_Rating;


				lpCurPlayer->smCharInfo.Attack_Rating += Grand_Smash_AttackRate[lpSkill->Point - 1];

				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
				if(!cnt)
				{
					lpCurPlayer->AttackCritcal = -1;
				}

				lpCurPlayer->smCharInfo.Attack_Rating = temp;
			}

			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;



		case SKILL_T5_RING_OF_SPEARS:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			// EU Ring of Spears animates with T41, then sends the T51 skill.
			// Summit T41 maps to Pikeman Tier-4 slot 1 (Assassin Eye).
			int summitRingNetworkSkill = SKILL_PLAY_T51 | (lpSkill->Point << 8);
			lpCurPlayer->AttackSkil = SKILL_PLAY_ASSASSIN_EYE | (lpSkill->Point << 8);
			int summitRingBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitRingMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Pikeman/RingOfSpears", "cast-motion-control", lpCurPlayer, lpChar, summitRingBegin, summitRingMotion);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;
			lpCurPlayer->AttackSkil = summitRingNetworkSkill;
			SummitT5StartParticle("PS5RingOfSpears", lpChar);
			SummitT5MotionEventArm(lpCurPlayer, SKILL_T5_RING_OF_SPEARS, SKILL_PLAY_T51, lpSkill->Point, lpChar, 1);

			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt) lpCurPlayer->AttackCritcal = -1;
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_GROUND_PIKE:
		return TRUE;

		case SKILL_TORNADO:


		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_TORNADO;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			cnt = GetRandomPos(lpCurPlayer->smCharInfo.Attack_Damage[0], lpCurPlayer->smCharInfo.Attack_Damage[1]);
			cnt += (cnt*Tornado_Damage[lpSkill->Point - 1]) / 100;



			x = (lpCurPlayer->pX - lpChar->pX) >> FLOATNS;
			z = (lpCurPlayer->pZ - lpChar->pZ) >> FLOATNS;

			y = (int)sqrt(x*x + z*z);

			y += 8;





			dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, y, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);


		}
		return TRUE;

		case SKILL_CHAIN_LANCE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_CHAIN_LANCE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;


			if(lpSkill->Point)
			{




				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
				if(!cnt)
				{
					lpCurPlayer->AttackCritcal = -1;
				}


			}
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_ARROW_OF_RAGE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			lpCurPlayer->AttackSkil = SKILL_PLAY_ARROWOF_RAGE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 120;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

		case SKILL_FALCON:

		return TRUE;

		case SKILL_AVALANCHE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_AVALANCHE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->AttackCriticalCount = 0;


			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

			if(!cnt)
			{
				lpCurPlayer->AttackCritcal = -1;
			}
			if(lpCurPlayer->AttackCritcal > 0) lpCurPlayer->AttackCriticalCount++;



			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, Avalanche_Speed[lpSkill->Point - 1]);
			lpCurPlayer->MotionLoop = Avalanche_ArrowNum[lpSkill->Point - 1];

			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_ELEMENTAL_SHOT:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_ELEMENTAL_SHOT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			cnt = rand() % 2;
			lpCurPlayer->AttackSkil |= (cnt << 12);

			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->AttackCriticalCount = 0;

			if(cnt == 0)
			{
				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, FALSE);
				if(!cnt)
				{
					lpCurPlayer->AttackCritcal = -1;
				}
			}
			else
			{

				dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, 75, FALSE);

				dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
			}

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, 3);
			lpCurPlayer->MotionLoop = 2;

			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_BOMB_SHOT:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_BOMB_SHOT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->AttackCriticalCount = 0;




			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, BombShot_Area[lpSkill->Point - 1], FALSE);

			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

			lpCurPlayer->MotionLoopSpeed = (GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed) * 80) / 100;
			lpCurPlayer->MotionLoop = 2;

			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_PERFORATION:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_PERFORATION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->AttackCriticalCount = 0;




			rect.left = -40;
			rect.right = 40;
			rect.top = 0;
			rect.bottom = lpCurPlayer->smCharInfo.Shooting_Range + Perforation_Attack_Range[lpSkill->Point - 1];
			rect.bottom += 20;
			rect.bottom += 20;


			dm_SelectRangeBox(lpCurPlayer, &rect, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;

			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_TRIPLE_IMPACT:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_TRIPLE_IMPACT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			switch(T_Impact_Hit[lpSkill->Point - 1])
			{
				case 1:

				cnt = lpCurPlayer->AttackSkil;
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);
				lpCurPlayer->AttackSkil = cnt;
				break;
				case 2:

				cnt = lpCurPlayer->AttackSkil;
				lpCurPlayer->AttackSkil = SKILL_PLAY_IMPACT;
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->AttackSkil = cnt;
				break;
				case 3:

				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				break;
			}



			if(lpSkill->Point)
			{

				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
				{

					DamageExp(lpChar, cnt);
				}
			}

			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_BRUTAL_SWING:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_BRUTAL_SWING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			if(lpSkill->Point)
			{

				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
				{

					DamageExp(lpChar, cnt);
				}
			}
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_ROAR:
		return TRUE;

		case SKILL_RAGE_OF_ZECRAM:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_RAGEOF_ZECRAM;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			if(lpSkill->Point)
			{

				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
				{

					DamageExp(lpChar, cnt);
				}
			}
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_AVANGING_CRASH:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_AVANGING_CRASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			temp = lpCurPlayer->smCharInfo.Attack_Rating;


			lpCurPlayer->smCharInfo.Attack_Rating +=
				(lpCurPlayer->smCharInfo.Attack_Rating * A_Crash_AttackRate[lpSkill->Point - 1]) / 100;


			if(lpSkill->Point)
			{
				cnt = dm_SendTransDamage(lpChar, cSkill.GetSkillPoint(SKILL_BRUTAL_SWING), 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
				{

					DamageExp(lpChar, cnt);
				}
			}

			lpCurPlayer->smCharInfo.Attack_Rating = temp;

			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_BONE_CRASH:
			 
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_BONE_SMASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			if(lpSkill->Point)
			{
				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
				{

					DamageExp(lpChar, cnt);
				}
			}

			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_T5_STONE_SKIN:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				// EU Stone Skin uses the visual T51 motion. Keep Summit T52 only as the
				// logical/server skill slot so visual routing and server authority stay independent.
				lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
				int summitStoneBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitStoneMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Magician/StoneSkin", "cast-motion-eu-t51", lpCurPlayer, 0, summitStoneBegin, summitStoneMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				SummitT5StartParticle("R5StoneSkin", lpCurPlayer);
				lpCurPlayer->PlayInvincible = Magician_T5_StoneSkin_ImmortalWindow[lpSkill->Point - 1] * 70;
				lpCurPlayer->AttackSkil = SKILL_PLAY_T52 | (lpSkill->Point << 8);
				SendProcessSKillToServer(SKILL_PLAY_T52, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_CONSECRATION:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_T54 | (lpSkill->Point << 8);
				int summitConsecrationBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitConsecrationMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Priestess/Consecration", "cast-motion", lpCurPlayer, 0, summitConsecrationBegin, summitConsecrationMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				SendPartySkillToServer(SKILL_PLAY_T54, lpSkill->Point, 450, 0, 0, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_WIZARD_TRANCE:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				// EU CWizardTrance casts through Magician T33 (Spirit Elemental), then processes T51.
				lpCurPlayer->AttackSkil = SKILL_PLAY_SPIRIT_ELEMENTAL | (lpSkill->Point << 8);
				int summitTranceBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitTranceMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Magician/WizardTrance", "cast-motion-eu-t33", lpCurPlayer, 0, summitTranceBegin, summitTranceMotion);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;
				SummitT5StartParticle("MG5OrbMastery", lpCurPlayer);
				lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
				SummitT5MotionEventArm(lpCurPlayer, SKILL_T5_WIZARD_TRANCE, SKILL_PLAY_T51, lpSkill->Point, 0, 1);
				SendProcessSKillToServer(SKILL_PLAY_T51, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_ICE_ELEMENTAL:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				// EU CIceElementalHandler casts through Priestess T44 (Summon Muspell), then processes T51.
				lpCurPlayer->AttackSkil = SKILL_PLAY_SUMMON_MUSPELL | (lpSkill->Point << 8);
				int summitIceBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitIceMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Priestess/IceElemental", "cast-motion-eu-t44", lpCurPlayer, 0, summitIceBegin, summitIceMotion);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;
				// EU Ice Elemental reuses the legacy Fire Elemental sound (0x4710).
				SkillPlaySound(SKILL_SOUND_SKILL_FIRE_ELEMENTAL, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);
				lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
				SendProcessSKillToServer(SKILL_PLAY_T51, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_SUMMON_TIGER:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				// EU Summon Tiger starts with Atalanta T41 animation, then routes as T51.
				lpCurPlayer->AttackSkil = SKILL_PLAY_HALL_OF_VALHALLA | (lpSkill->Point << 8);
				int summitTigerBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitTigerMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Atalanta/NaturesCall", "cast-motion", lpCurPlayer, 0, summitTigerBegin, summitTigerMotion);
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;
				lpCurPlayer->HideWeapon = TRUE;
				lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
				SendProcessSKillToServer(SKILL_PLAY_T51, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_HOLY_CONVICTION:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				// EU Holy Conviction animates with T43, then switches to T51.
				// Summit T43 maps to Knight Tier-4 slot 3 (Gods Blessing).
				lpCurPlayer->AttackSkil = SKILL_PLAY_GODS_BLESS | (lpSkill->Point << 8);
				int summitHolyBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitHolyMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Knight/HolyConviction", "cast-motion-control", lpCurPlayer, 0, summitHolyBegin, summitHolyMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
				SummitT5AttachParticleToChar(lpCurPlayer, SKILL_T5_HOLY_CONVICTION, "KS5HolyConviction",
					(DWORD)Knight_T5_HolyConviction_Duration[lpSkill->Point - 1] * 1000);
				// EU Holy Conviction reuses the legacy Holy Valor sound (0x2510).
				SkillPlaySound(SKILL_SOUND_SKILL_HOLY_VALOR1, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);
				SendPartySkillToServer(SKILL_PLAY_T51, lpSkill->Point, 450, 0, 0, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_LETHAL_SIGHT:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
				int summitLethalBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitLethalMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Archer/LethalSight", "cast-motion-control", lpCurPlayer, 0, summitLethalBegin, summitLethalMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				SummitT5StartParticle("Ac5LethalSight", lpCurPlayer);
				SummitT5AttachParticleToChar(lpCurPlayer, SKILL_T5_LETHAL_SIGHT, "Ac5LethalSightKeep", 300000);
				// EU Lethal Sight reuses the legacy Force of Nature sound (0x4340).
				SkillPlaySound(SKILL_SOUND_SKILL_FORCE_OF_NATURE, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);
				SendProcessSKillToServer(SKILL_PLAY_T51, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_FOCUS:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_T52 | (lpSkill->Point << 8);
				int summitFocusBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitFocusMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Pikeman/DeathMantra", "cast-motion", lpCurPlayer, 0, summitFocusBegin, summitFocusMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				SendProcessSKillToServer(SKILL_PLAY_T52, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_PARASIT_SHOT:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T52 | (lpSkill->Point << 8);
			int summitParasitBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitParasitMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Mechanician/SwarmShot", "cast-motion", lpCurPlayer, lpChar, summitParasitBegin, summitParasitMotion);

			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp(lpChar, cnt);
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_T5_LIGHTNING_SURGE:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T52 | (lpSkill->Point << 8);
			int summitLightningBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitLightningMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Priestess/LightningSurge", "cast-motion", lpCurPlayer, lpChar, summitLightningBegin, summitLightningMotion);
			lpCurPlayer->MotionLoopSpeed = 80;
			lpCurPlayer->MotionLoop = 1;
			// EU exposes DamageBoost + Area + ChargedArea. ChargedArea semantics are not present in the open handler,
			// so Summit applies only the proven area component here.
			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, Priestess_T5_LightningSurge_Area[lpSkill->Point - 1], TRUE);
			dm_AddRangeDamage(lpChar);
			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_HEAVENLY_LIGHT:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T53 | (lpSkill->Point << 8);
			int summitHeavenBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitHeavenMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Priestess/HeavenlyLight", "cast-motion", lpCurPlayer, lpChar, summitHeavenBegin, summitHeavenMotion);
			lpCurPlayer->MotionLoopSpeed = 80;
			lpCurPlayer->MotionLoop = 1;
			// EU exposes attack-rate reduction, duration and charged area. Summit applies the
			// proven blind component to monsters selected inside that area; no damage is sent.
			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, Priestess_T5_HeavenlyLight_Area[lpSkill->Point - 1], TRUE);
			dm_AddRangeDamage(lpChar);
			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_DIVINE_INQUISITON:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T52 | (lpSkill->Point << 8);
			int summitInqBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitInqMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Knight/DivineInquisition", "cast-motion", lpCurPlayer, lpChar, summitInqBegin, summitInqMotion);
			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, Knight_T5_DivineInquisition_Area[lpSkill->Point - 1], TRUE);
			dm_AddRangeDamage(lpChar);
			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_FIERCE_WIND:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			// EU Fierce Wind animates through Tier-5 slot T51.
			// Preserve T52 only for Summit server-side skill discrimination.
			int summitFierceWindNetworkSkill = SKILL_PLAY_T52 | (lpSkill->Point << 8);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
			int summitFierceBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitFierceMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Archer/FierceWind", "cast-motion-control", lpCurPlayer, lpChar, summitFierceBegin, summitFierceMotion);
			lpCurPlayer->AttackSkil = summitFierceWindNetworkSkill;
			lpCurPlayer->AttackCriticalCount = 0;
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;
			SummitT5MotionEventArm(lpCurPlayer, SKILL_T5_FIERCE_WIND, SKILL_PLAY_T52, lpSkill->Point, lpChar, 2);
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_SEISMIC_IMPACT:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			// EU Seismic Impact animates through T51; T52 remains Summit network routing.
			int summitSeismicNetworkSkill = SKILL_PLAY_T52 | (lpSkill->Point << 8);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
			int summitSeismicBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitSeismicMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Fighter/SeismicImpact", "cast-motion-control", lpCurPlayer, lpChar, summitSeismicBegin, summitSeismicMotion);
			lpCurPlayer->AttackSkil = summitSeismicNetworkSkill;
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;
			SummitT5MotionEventArm(lpCurPlayer, SKILL_T5_SEISMIC_IMPACT, SKILL_PLAY_T52, lpSkill->Point, lpChar, 4);
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_RED_RAY:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			// EU CRedRayHandler animates with T52. Summit keeps T53 as the logical/network slot.
			int summitRedRayNetworkSkill = SKILL_PLAY_T53 | (lpSkill->Point << 8);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T52 | (lpSkill->Point << 8);
			int summitRayBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitRayMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Magician/RedRay", "cast-motion-eu-t52", lpCurPlayer, lpChar, summitRayBegin, summitRayMotion);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;
			lpCurPlayer->AttackSkil = summitRedRayNetworkSkill;
			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt) lpCurPlayer->AttackCritcal = -1; else DamageExp(lpChar, cnt);
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_PLAGUE_JAVELIN:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T53 | (lpSkill->Point << 8);
			int summitPlagueBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitPlagueMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Atalanta/PlagueJavelin", "cast-motion", lpCurPlayer, lpChar, summitPlagueBegin, summitPlagueMotion);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;
			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt) lpCurPlayer->AttackCritcal = -1; else DamageExp(lpChar, cnt);
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_ENTALING_ROOTS:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T53 | (lpSkill->Point << 8);
			int summitRootsBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitRootsMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Archer/EntalingRoots", "cast-motion", lpCurPlayer, lpChar, summitRootsBegin, summitRootsMotion);
			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, FALSE);
			if(!cnt) lpCurPlayer->AttackCritcal = -1;
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_T5_GLORIOUS_SHIELD:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_T53 | (lpSkill->Point << 8);
				int summitGloriousBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitGloriousMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Knight/GloriousShield", "cast-motion", lpCurPlayer, 0, summitGloriousBegin, summitGloriousMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				SendProcessSKillToServer(SKILL_PLAY_T53, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_DEATH_MASTER:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_T53 | (lpSkill->Point << 8);
				int summitDeathBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitDeathMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Pikeman/ExDeath", "cast-motion", lpCurPlayer, 0, summitDeathBegin, summitDeathMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				SendProcessSKillToServer(SKILL_PLAY_T53, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_HARDENING_SHIELD:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_T53 | (lpSkill->Point << 8);
				int summitHardenBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitHardenMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Mechanician/HardenedWard", "cast-motion", lpCurPlayer, 0, summitHardenBegin, summitHardenMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				SendProcessSKillToServer(SKILL_PLAY_T53, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_RUTHLESS_OF_ZECRAM:
		if(lpCurPlayer->MotionInfo->State < 0x100)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				lpCurPlayer->AttackSkil = SKILL_PLAY_T53 | (lpSkill->Point << 8);
				int summitRuthlessBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
				int summitRuthlessMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				SummitT5VisualProbe("Fighter/MightOfZecram", "cast-motion", lpCurPlayer, 0, summitRuthlessBegin, summitRuthlessMotion);
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				SendProcessSKillToServer(SKILL_PLAY_T53, lpSkill->Point, 0, 0);
			}
		}
		return TRUE;

		case SKILL_T5_TWISTER:
		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_T54 | (lpSkill->Point << 8);
			int summitTwisterBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
			int summitTwisterMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Pikeman/SinisterSpin", "cast-motion", lpCurPlayer, 0, summitTwisterBegin, summitTwisterMotion);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;

			dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,
				Tornado_Range[lpSkill->Point - 1], TRUE);
			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,
				0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
		}
		return TRUE;

		case SKILL_T5_MAGNETIC_DISCHARGE:
		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_T54 | (lpSkill->Point << 8);
			int summitMagneticBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
			int summitMagneticMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Mechanician/MagneticDischarge", "cast-motion", lpCurPlayer, 0, summitMagneticBegin, summitMagneticMotion);

			dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,
				Impulsion_Range[lpSkill->Point - 1], TRUE, Impulsion_LightingNum[lpSkill->Point - 1]);
			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,
				0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
		}
		return TRUE;


		case SKILL_T5_DIVINE_CROSS:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T54 | (lpSkill->Point << 8);
			int summitCrossBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitCrossMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Knight/DivineCross", "cast-motion", lpCurPlayer, lpChar, summitCrossBegin, summitCrossMotion);
			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt) lpCurPlayer->AttackCritcal = -1; else DamageExp(lpChar, cnt);
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_COMBO_JAVELIN:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			// EU SkillManager maps ComboJavelin to the Atalanta T51 animation.
			int summitComboNetworkSkill = SKILL_PLAY_T54 | (lpSkill->Point << 8);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
			int summitComboBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitComboMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Atalanta/TripleThreat", "cast-motion", lpCurPlayer, lpChar, summitComboBegin, summitComboMotion);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(6);
			lpCurPlayer->MotionLoop = 1;
			lpCurPlayer->AttackSkil = summitComboNetworkSkill;

			for(int comboHit = 0; comboHit < 3; comboHit++)
			{
				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
				if(!cnt) lpCurPlayer->AttackCritcal = -1; else DamageExp(lpChar, cnt);
			}
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_T5_CATACLYSM:
		if(SummitMagicianCataclysmIsActive())
		{
			SummitMagicianCataclysmStop(TRUE);
			return TRUE;
		}
		if(lpCurPlayer->MotionInfo->State < 0x100 && lpChar && lpSkill->Point >= 1 && lpSkill->Point <= 10)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				int point = lpSkill->Point - 1;
				gSummitCataclysmPoint = lpSkill->Point;
				gSummitCataclysmEndTime = dwPlayTime + (DWORD)Magician_T5_Cataclysm_Duration[point] * 1000;
				gSummitCataclysmNextTick = dwPlayTime;
				gSummitCataclysmTarget = lpChar;
				SendProcessSKillToServer(SKILL_PLAY_T54, lpSkill->Point, 0, 0);
				SummitT5VisualProbe("Magician/Cataclysm", "channel-start", lpCurPlayer, lpChar, 0, 0);
				SummitMagicianCataclysmTick();
			}
		}
		return TRUE;

		case SKILL_T5_BOMBARDMENT:
		if(SummitArcherBombardmentIsActive())
		{
			SummitArcherBombardmentStop(TRUE);
			return TRUE;
		}
		if(lpCurPlayer->MotionInfo->State < 0x100 && lpChar && lpSkill->Point >= 1 && lpSkill->Point <= 10)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				int point = lpSkill->Point - 1;
				gSummitBombardmentPoint = lpSkill->Point;
				gSummitBombardmentEndTime = dwPlayTime + (DWORD)Archer_T5_Bombard_Duration[point] * 1000;
				gSummitBombardmentNextTick = dwPlayTime;
				gSummitBombardmentTarget = lpChar;
				SendProcessSKillToServer(SKILL_PLAY_T54, lpSkill->Point, 0, 0);
				SummitT5VisualProbe("Archer/Bombardment", "channel-start", lpCurPlayer, lpChar, 0, 0);
				SummitArcherBombardmentTick();
			}
		}
		return TRUE;

		case SKILL_T5_WHIRLWIND:
		if(SummitWhirlwindIsActive())
		{
			SummitWhirlwindStop(TRUE);
			return TRUE;
		}
		if(lpCurPlayer->MotionInfo->State < 0x100 && lpSkill->Point >= 1 && lpSkill->Point <= 10)
		{
			if(lpSkill->Skill_Info.FuncPointer() == TRUE)
			{
				int point = lpSkill->Point - 1;
				gSummitWhirlwindPoint = lpSkill->Point;
				gSummitWhirlwindEndTime = dwPlayTime + (DWORD)Fighter_T5_Whirlwind_Duration[point] * 1000;
				gSummitWhirlwindNextTick = dwPlayTime;
				SendProcessSKillToServer(SKILL_PLAY_T54, lpSkill->Point, 0, 0);
				SummitT5VisualProbe("Fighter/RagingStrikes", "channel-start", lpCurPlayer, 0, 0, 0);
				SummitWhirlwindTick();
			}
		}
		return TRUE;


		case SKILL_EXPANSION:

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_EXPANSION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);


			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom = LastAttackRange >> FLOATNS;
			rect.bottom += 20;


			dm_SelectRangeBox(lpCurPlayer, &rect, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);




			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill)
		{
			if(RetryPlayAttack(lpChar)) break;
			AttCancel = TRUE;
		}
		return TRUE;




		case SKILL_SWORD_BLAST:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_SWORD_BLAST;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			rect.left = -40;
			rect.right = 40;
			rect.top = 0;
			rect.bottom = Sword_Blast_ShootingRange[lpSkill->Point - 1] + 50;





			dm_SelectRangeBox(lpCurPlayer, &rect, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_DOUBLE_CRASH:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_DOUBLE_CRASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			if(lpSkill->Point)
			{

				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
				{

					DamageExp(lpChar, cnt);
				}
			}


			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_BRANDISH:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_BRANDISH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);



			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, Brandish_Range[lpSkill->Point - 1], FALSE);

			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_PIERCING:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_PIERCING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, 0, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);



			temp = lpCurPlayer->smCharInfo.Attack_Rating;
			lpCurPlayer->smCharInfo.Attack_Rating += Piercing_Attack_Rating[lpSkill->Point - 1];



			rect.left = -40;
			rect.right = 40;
			rect.top = 0;
			rect.bottom = 110 + 30;




			dm_SelectRangeBox(lpCurPlayer, &rect, TRUE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);


			lpCurPlayer->smCharInfo.Attack_Rating = temp;
			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_HOLY_INCANTATION:

		if(lpChar->smCharInfo.Brood == smCHAR_MONSTER_UNDEAD && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_INCANTATION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_GRAND_CROSS:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_GRAND_CROSS;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, 1);
			lpCurPlayer->MotionLoop = 1;


			temp = lpCurPlayer->smCharInfo.Attack_Rating;

			lpCurPlayer->smCharInfo.Attack_Rating += (lpCurPlayer->smCharInfo.Attack_Rating*Grand_Cross_AttackRate[lpSkill->Point - 1]) / 100;

			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp(lpChar, cnt);


			lpCurPlayer->smCharInfo.Attack_Rating = temp;

			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;




		case SKILL_SHIELD_STRIKE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_SHIELD_STRIKE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 90;
			lpCurPlayer->MotionLoop = 1;


			rect.left = -50;
			rect.right = 50;
			rect.top = 0;
			rect.bottom = 160;





			dm_SelectRangeBox(lpCurPlayer, &rect, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);


			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_FARINA:

		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_FARINA;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			temp = lpCurPlayer->smCharInfo.Attack_Rating;



			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, Farina_Speed[lpSkill->Point - 1]);
			lpCurPlayer->MotionLoop = 1;



			lpCurPlayer->smCharInfo.Attack_Rating +=
				Farina_AttackRate[lpSkill->Point - 1] + lpCurPlayer->smCharInfo.Level;


			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp(lpChar, cnt);


			lpCurPlayer->smCharInfo.Attack_Rating = temp;


			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_VIGOR_SPEAR:

		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_VIGOR_SPEAR;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);



			cnt = dm_SendTransDamage(lpChar, wpDamage[0], 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp(lpChar, cnt);


			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_TWIST_JAVELIN:

		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_TWIST_JAVELIN;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			temp = lpCurPlayer->smCharInfo.Attack_Rating;
			lpCurPlayer->smCharInfo.Attack_Rating += (lpCurPlayer->smCharInfo.Attack_Rating*Twist_Javelin_Attack_Rating[lpSkill->Point - 1]) / 100;


			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp(lpChar, cnt);

			lpCurPlayer->smCharInfo.Attack_Rating = temp;

			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_SOUL_SUCKER:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_SOUL_SUCKER;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 90;
			lpCurPlayer->MotionLoop = 1;

			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom = 200;





			dm_SelectRangeBox(lpCurPlayer, &rect, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_FIRE_JAVELIN:

		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_ATTACK);

			lpCurPlayer->AttackSkil = SKILL_PLAY_FIRE_JAVELIN;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);






			dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, FALSE);

			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_SPLIT_JAVELIN:

		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->AttackSkil = SKILL_PLAY_SPLIT_JAVELIN;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);


			temp = lpCurPlayer->smCharInfo.Attack_Rating;
			lpCurPlayer->smCharInfo.Attack_Rating += Split_Javelin_AttackRate[lpSkill->Point - 1];
			z = Split_Javelin_AttackNum[lpSkill->Point - 1];
			x = 0;
			y = sinGetAccuracy(lpChar->smCharInfo.Level, lpChar->smCharInfo.Defence);

			for(cnt = 0; cnt < z; cnt++)
			{
				if((rand() % 100) < y)
				{
					x++;
				}
			}

			if(x > 0)
			{
				dm_SendTransDamage(lpChar, x, 0, 0, 0, lpCurPlayer->AttackSkil, FALSE);
			}
			else
			{
				lpCurPlayer->AttackCritcal = -1;
			}

			lpCurPlayer->AttackSkil |= (x << 12);
			lpCurPlayer->smCharInfo.Attack_Rating = temp;

			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, 1);
			lpCurPlayer->MotionLoopSpeed -= (lpCurPlayer->MotionLoopSpeed * 20) / 100;
			lpCurPlayer->MotionLoop = 1;



			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_TRIUMPH_OF_VALHALLA:
		if(lpChar->smCharInfo.State == smCHAR_STATE_USER &&
		   GetAssaUseEffect(lpChar, SKILL_TRIUMPH_OF_VALHALLA) == FALSE &&
		   GetAssaUseEffect(lpChar, SKILL_HALL_OF_VALHALLA) == FALSE &&
		   lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_TRIUMPH_OF_VALHALLA;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			return TRUE;
		}
		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_LIGHTNING_JAVELIN:

		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}
		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);

			lpCurPlayer->AttackSkil = SKILL_PLAY_LIGHTNING_JAVELIN;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, 100, FALSE);

			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);


			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;




		case SKILL_STORM_JAVELIN:

		if(lpCurPlayer->CheckShootingTest(lpChar) == FALSE)
		{
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			return TRUE;
		}

		if(lpSkill->Point && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);



			lpCurPlayer->AttackSkil = SKILL_PLAY_STORM_JAVELIN;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);


			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom = 300;
			rect.bottom += 20;
			rect.bottom += 20;


			dm_SelectRangeBox(lpCurPlayer, &rect, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);


			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 2;



			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;



		case SKILL_HEALING:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_HEALING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;

			dm_SendTransDamage(lpChar, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil, FALSE);




		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_HOLY_BOLT:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_BOLT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(8);
			lpCurPlayer->MotionLoop = 1;



			cnt = dm_SendTransDamage(lpChar, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil, FALSE);

			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp(lpChar, cnt);

			return TRUE;

		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_MULTISPARK:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			cnt = M_Spark_Num[lpSkill->Point - 1];

			cnt = GetRandomPos((cnt / 2) + 1, cnt);

			lpCurPlayer->AttackSkil = SKILL_PLAY_MULTI_SPARK;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->AttackSkil |= (cnt << 12);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);




			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(8);
			lpCurPlayer->MotionLoop = 1;

			cnt = dm_SendTransDamage(lpChar, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil, FALSE);
			if(cnt)
				DamageExp(lpChar, cnt);
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_HOLY_MIND:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_MIND;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;

			cnt = HolyMind_DecDamage[lpSkill->Point - 1];

			smTransCommand.size = sizeof(smTRANS_COMMAND);
			smTransCommand.code = OpCode::OPCODE_HOLYMIND;
			smTransCommand.WParam = HolyMind_DecDamage[lpSkill->Point - 1];
			smTransCommand.LParam = 15;
			smTransCommand.SParam = lpChar->dwObjectSerial;
			smTransCommand.EParam = lpCurPlayer->dwObjectSerial;

			lpsmSock = GetAreaServerSock();
			if(lpsmSock)
				lpsmSock->Send((char *)&smTransCommand, smTransCommand.size, TRUE);

		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_DIVINE_LIGHTNING:
		case SKILL_HOLY_REFLECTION:
		case SKILL_GRAND_HEALING:

		case SKILL_RESURRECTION:
		case SKILL_EXTINCTION:
		return TRUE;

		case SKILL_VIRTUAL_LIFE:


		if(lpChar->smCharInfo.State == smCHAR_STATE_USER && GetAssaUseEffect(lpChar, SKILL_VIRTUAL_LIFE) == FALSE && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			lpCurPlayer->AttackSkil = SKILL_PLAY_VIRTUAL_LIFE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			SendProcessSKillToServer(SKILL_PLAY_VIRTUAL_LIFE, lpSkill->Point, lpChar->dwObjectSerial, 0);
			return TRUE;
		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;


		case SKILL_VIGOR_BALL:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_VIGOR_BALL;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(7);
			lpCurPlayer->MotionLoop = 1;

			DamageExp(lpChar, 1);

			return TRUE;

		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;



		case SKILL_FIRE_BOLT:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_FIRE_BOLT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 100;
			lpCurPlayer->MotionLoop = 1;


			cnt = dm_SendTransDamage(lpChar, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil, FALSE);
			if(cnt)
				DamageExp(lpChar, cnt);

		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_FIRE_BALL:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_FIRE_BALL;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 100;
			lpCurPlayer->MotionLoop = 1;



			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, FireBall_Area[lpSkill->Point - 1], FALSE);

			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil);


		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_WATORNADO:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_COLUMN_OF_WATER;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 90;
			lpCurPlayer->MotionLoop = 1;



			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, Watornado_Area[lpSkill->Point - 1], FALSE);

			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil);


		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_ENCHANT_WEAPON:
		if(!lpChar->WeaponEffect && lpChar->PartyFlag && lpChar->smCharInfo.State == smCHAR_STATE_USER &&
		   lpSkill->Skill_Info.FuncPointer() == TRUE)
		{

			lpCurPlayer->AttackSkil = SKILL_PLAY_ENCHANT_WEAPON;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			cnt = rand() % 3;
			lpCurPlayer->AttackSkil |= (cnt << 12);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 90;
			lpCurPlayer->MotionLoop = 1;
			return TRUE;

		}
		lpCurPlayer->chrAttackTarget = 0;

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;

		case SKILL_DEAD_RAY:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_DEAD_RAY;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			lpCurPlayer->MotionLoopSpeed = 100;
			lpCurPlayer->MotionLoop = 1;

			cnt = dm_SendTransDamage(lpChar, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil, FALSE);

			if(cnt)
				DamageExp(lpChar, cnt);

		}

		if(lpSkill == sinSkill.pLeftSkill && RetryPlayAttack(lpChar)) break;
		return TRUE;




		case SKILL_HOLY_BODY:
		case SKILL_DRASTIC_SPIRIT:
		case SKILL_HOLY_VALOR:
		case SKILL_DIVINE_INHALATION:


		case SKILL_WINDY:

		case SKILL_AGONY:
		case SKILL_ZENITH:

		case SKILL_ENERGY_SHIELD:
		case SKILL_DIASTROPHISM:
		case SKILL_SPIRIT_ELEMENTAL:
		case SKILL_DANCING_SWORD:


		return TRUE;




		case SKILL_IMPULSION:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_IMPULSION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, Impulsion_Range[lpSkill->Point - 1], TRUE, Impulsion_LightingNum[lpSkill->Point - 1]);

			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

		}
		return TRUE;


		case SKILL_DETORYER:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_DESTROYER;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;


			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, 1);
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

		case SKILL_CYCLONE_STRIKE:

		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_CYCLONE_STRIKE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			dm_SelectRange(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Cyclone_Strike_Area[lpSkill->Point - 1], TRUE, Cyclone_Strike_AttackNum[lpSkill->Point - 1]);
			dm_AddRangeDamage(lpChar);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
		}
		return TRUE;

		case SKILL_ASSASSIN_EYE:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_ASSASSIN_EYE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, FALSE);
		}
		return TRUE;

		case SKILL_CHARGING_STRIKE:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_CHARGING_STRIKE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			SkillChargingFlag = 1;

			lpCurPlayer->MotionLoopSpeed = 60 + (Charging_Strike_Time[lpSkill->Point - 1] * 2);
			lpCurPlayer->MotionLoop = 1;

			SkillPlaySound(SKILL_SOUND_SKILL_CHARGING, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);
		}
		return TRUE;
		case SKILL_SHADOW_MASTER:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_SHADOW_MASTER;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);



			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(7);
			lpCurPlayer->MotionLoop = 1;

		}
		return TRUE;

		case SKILL_PHOENIX_SHOT:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_PHOENIX_SHOT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			
            //Carregamento da Skill
			//SkillChargingFlag = 1;
			lpCurPlayer->MotionLoopSpeed = 60 + (Phoenix_Shot_Time[lpSkill->Point - 1] * 2);

			lpCurPlayer->MotionLoop = 1;

			SkillPlaySound(SKILL_SOUND_SKILL_CHARGING, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);
		}
		return TRUE;

		case SKILL_DIVINE_PIERCING:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_PIERCING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);

			if(Divine_Piercing_AttackNum[lpSkill->Point - 1] >= 7)
			{
				cnt = lpCurPlayer->AttackSkil;
				lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_PIERCING3;
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->AttackSkil = cnt;
			}
			else if(Divine_Piercing_AttackNum[lpSkill->Point - 1] >= 5)
			{
				cnt = lpCurPlayer->AttackSkil;
				lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_PIERCING2;
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
				lpCurPlayer->AttackSkil = cnt;
			}
			else
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);




			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(7);
			lpCurPlayer->MotionLoop = 1;

		}
		return TRUE;
		case SKILL_SWORD_OF_JUSTICE:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_SWORD_OF_JUSTICE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

			rect.left = -70;
			rect.right = 70;
			rect.top = 0;
			rect.bottom = Sword_Of_Justice_Area[lpSkill->Point - 1];
			rect.bottom += 20;
			rect.bottom += 20;


			dm_SelectRangeBox(lpCurPlayer, &rect, TRUE);
			dm_AddRangeDamage(lpChar);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
		}
		return TRUE;

		case SKILL_X_RAGE:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_X_RAGE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);




			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, X_Rage_Area[lpSkill->Point - 1], FALSE);

			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);

		}
		return TRUE;
		case SKILL_VENGEANCE:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_VENGEANCE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(6);
			lpCurPlayer->MotionLoop = 1;

			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt)
				lpCurPlayer->AttackCritcal = -1;
		}


		return TRUE;


		case SKILL_GLACIAL_SPIKE:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_GLACIAL_SPIKE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


		}
		return TRUE;

		case SKILL_CHAIN_LIGHTNING:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_CHAIN_LIGHTNING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

		}
		return TRUE;


		case SKILL_FLAME_WAVE:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_FLAME_WAVE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);


			rect.left = -60;
			rect.right = 60;
			rect.top = 0;
			rect.bottom = 300;
			rect.bottom += 20;
			rect.bottom += 20;


			dm_SelectRangeBox(lpCurPlayer, &rect, FALSE);

			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil);

		}
		return TRUE;
		case SKILL_M_METEO:
		if(lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			lpCurPlayer->AttackSkil = SKILL_PLAY_METEO;
			lpCurPlayer->AttackSkil |= (lpSkill->Point << 8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);

		}
		return TRUE;


		case SKILL_COMPULSION:
		case SKILL_MAGNETIC_SPHERE:
		case SKILL_METAL_GOLEM:
		case SKILL_BERSERKER:
		case SKILL_VAGUE:
		case SKILL_RECALL_WOLVERIN:
		case SKILL_FORCE_OF_NATURE:
		case SKILL_GODLY_SHIELD:
		case SKILL_GOD_BLESS:
		case SKILL_HALL_OF_VALHALLA:
		case SKILL_FROST_JAVELIN:
		case SKILL_REGENERATION_FIELD:
		case SKILL_SUMMON_MUSPELL:
		case SKILL_FIRE_ELEMENTAL:
		case SKILL_DISTORTION:


		return TRUE;


	}

	if(lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_ATTACK && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_SKILL)
	{
		RetryPlayAttack(lpChar);
	}

	return FALSE;
}


int PlaySkillLoop(int SkillCode, smCHAR *lpChar)
{
	int point, param;
	int	cnt;
	smCHAR	*chrAttackTarget;
	RECT	rect;

	point = (SkillCode >> 8) & 0xF;
	param = (SkillCode >> 12) & 0xF;

	switch(SkillCode & 0xFF)
	{

		case SKILL_PLAY_CRITICAL_HIT:

		if(lpChar->chrAttackTarget && point)
		{
			if(!lpChar->chrAttackTarget->smCharInfo.Life[0])
			{
				lpChar->MotionLoop = 0;
				break;
			}

			cnt = dm_SendTransDamage(lpChar->chrAttackTarget, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt)
				lpChar->AttackCritcal = -1;
			else
			{
				lpChar->AttackCritcal = 0;
				DamageExp(lpChar->chrAttackTarget, cnt);
			}



		}
		break;

		case SKILL_PLAY_RAVING:
		if(lpChar->chrAttackTarget && point)
		{

			if(!lpChar->chrAttackTarget->smCharInfo.Life[0])
			{
				lpChar->MotionLoop = 0;
				break;
			}

			cnt = dm_SendTransDamage(lpChar->chrAttackTarget, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);

			if(!cnt) lpChar->AttackCritcal = -1;
			else
			{

				lpChar->AttackCritcal = 0;
				PlaySkillSubLife(((int)(Raving_UseLife[point - 1] * 10)*lpChar->smCharInfo.Life[1]) / 1000);
				DamageExp(lpChar->chrAttackTarget, cnt);
			}

			lpChar->AttackAnger = 150;
			lpChar->Angle.y = GetRadian2D(lpChar->pX, lpChar->pZ, lpChar->chrAttackTarget->pX, lpChar->chrAttackTarget->pZ);

		}
		break;

		case SKILL_PLAY_AVALANCHE:

		if(lpChar->chrAttackTarget && point)
		{

			if(!lpChar->chrAttackTarget->smCharInfo.Life[0])
			{
				lpChar->MotionLoop = 0;
				break;
			}


			cnt = dm_SendTransDamage(lpChar->chrAttackTarget, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt)
				lpChar->AttackCritcal = -1;
			else
			{
				lpChar->AttackCritcal = 0;
				DamageExp(lpChar->chrAttackTarget, cnt);
			}


			lpChar->Angle.y = GetRadian2D(lpChar->pX, lpChar->pZ, lpChar->chrAttackTarget->pX, lpChar->chrAttackTarget->pZ);

		}
		break;


		case SKILL_PLAY_ELEMENTAL_SHOT:
		if(lpChar->chrAttackTarget && point)
		{

			if(!lpChar->chrAttackTarget->smCharInfo.Life[0])
			{
				lpChar->MotionLoop = 0;
				break;
			}

			chrAttackTarget = lpChar->chrAttackTarget;

			if(param == 0)
			{
				cnt = dm_SendTransDamage(lpChar->chrAttackTarget, 0, 0, 0, 0, lpChar->AttackSkil, FALSE);
				if(!cnt)
				{
					lpChar->AttackCritcal = -1;
				}
				else
				{
					lpChar->AttackCritcal = 0;
				}
			}
			else
			{

				dm_SelectRange(chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 75, FALSE);

				dm_SendRangeDamage(chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, chrAttackTarget, 0, 0, 0, 0, lpChar->AttackSkil);
			}
		}
		break;

		case SKILL_PLAY_STORM_JAVELIN:
		if(lpChar->chrAttackTarget && point)
		{

			chrAttackTarget = lpChar->chrAttackTarget;

			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom = 300;
			rect.bottom += 20;
			rect.bottom += 20;


			dm_SelectRangeBox(lpChar, &rect, FALSE);

			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, 0, 0, 0, 0, 0, lpChar->AttackSkil);
		}
		break;

		case SKILL_PLAY_BOMB_SHOT:
		if(lpChar->chrAttackTarget && point)
		{

			chrAttackTarget = lpChar->chrAttackTarget;


			dm_SelectRange(chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, BombShot_Area[point - 1], FALSE);

			dm_SendRangeDamage(chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, chrAttackTarget, 0, 0, 0, 0, lpChar->AttackSkil);
		}
		break;

	}

	return TRUE;
}



int SetSkillMotionBlurColor(int dwSkillCode)
{
	switch(dwSkillCode & 0xFF)
	{
		case SKILL_PLAY_CRITICAL_HIT:

		smRender.Color_R += 256;
		smRender.Color_G += -64;
		smRender.Color_B += 256;
		return TRUE;

		case SKILL_PLAY_RAVING:

		smRender.Color_R += 256;
		smRender.Color_G += -64;
		smRender.Color_B += -64;
		return TRUE;

		case SKILL_PLAY_IMPACT:

		smRender.Color_R += 256;
		smRender.Color_G += 256;
		smRender.Color_B += -64;
		return TRUE;

		case SKILL_PLAY_TRIPLE_IMPACT:

		smRender.Color_R += 256;
		smRender.Color_G += -64;
		smRender.Color_B += 256;
		return TRUE;

		case SKILL_PLAY_BRUTAL_SWING:

		smRender.Color_R += -64;
		smRender.Color_G += 256;
		smRender.Color_B += 128;
		return TRUE;

		case SKILL_PLAY_CHAIN_LANCE:

		smRender.Color_R += 256;
		smRender.Color_G -= 64;
		smRender.Color_B -= 64;
		break;

	}

	return FALSE;
}



int GetSkillPower(smCHAR *lpChar, smCHAR *lpChar2, int dx, int dy, int dz)
{
	int x, y, z, dist;

	switch(lpChar->HoSkillCode)
	{
		case SKILL_SCOUT_HAWK:
		if(Skill_ScoutHawk.Point)
		{
			x = (lpChar->HoSkillPos.x - lpChar2->pX) >> FLOATNS;
			y = (lpChar->HoSkillPos.y - lpChar2->pY) >> FLOATNS;
			z = (lpChar->HoSkillPos.z - lpChar2->pZ) >> FLOATNS;
			dist = x*x + y*y + z*z;

			if(dist < (512 * 512))
				return Scout_Hawk_Attack_Rate[Skill_ScoutHawk.Point - 1];
		}
		break;
	}

	return NULL;
}


int	LearnSkillEffect(smCHAR *lpChar)
{
	if(lpChar)
	{
		StartSkill(lpChar->pX, lpChar->pY, lpChar->pZ, 0, 0, 0, SKILL_UP1);
		SkillPlaySound(SKILL_SOUND_LEARN, lpChar->pX, lpChar->pY, lpChar->pZ);

	}
	else
	{
		StartSkill(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, SKILL_UP1);
		SkillPlaySound(SKILL_SOUND_LEARN, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);

	}

	return TRUE;
}

int LastSkillMastery = (-10000 * SIN_MAX_USE_SKILL) ^ ((DWORD)LearnSkillEffect);


int CheckPlaySkill()
{
	int cnt;
	int point;
	int OverPoint;
	int SkillCount = 0;
	int	LastSkillCount;
	int SkillError = 0;
	int	SubMastery;
	smTRANS_COMMAND	smTransCommand;

	point = sinSkill.SkillPoint;

	for(cnt = 1; cnt < 13; cnt++)
	{
		point += sinSkill.UseSkill[cnt].Point;
		SkillCount += sinSkill.UseSkill[cnt].UseSkillCount;
		if(sinSkill.UseSkill[cnt].Point > 10) point += 16384;
	}

	LastSkillCount = LastSkillMastery ^ ((DWORD)LearnSkillEffect);
	LastSkillCount = -LastSkillCount;

	SubMastery = SkillCount - LastSkillCount;
	if((SubMastery) > 300)
	{

		if(cInvenTory.InvenItem[SelectInvenItemIndex - 1].sItemInfo.CODE == (sinBI1 | sin39) ||
		   cInvenTory.InvenItem[SelectInvenItemIndex - 1].sItemInfo.CODE == (sinBI1 | sin40) ||
		   cInvenTory.InvenItem[SelectInvenItemIndex - 1].sItemInfo.CODE == (sinBI1 | sin41))
		{
			SkillError = 0;
		}
		else
			SkillError = 2;

		smTransCommand.LParam = SubMastery;
	}

	LastSkillMastery = (-SkillCount) ^ ((DWORD)LearnSkillEffect);

	if(lpCurPlayer->smCharInfo.Level >= 10)
	{
		OverPoint = ((lpCurPlayer->smCharInfo.Level - 8) / 2) + 1;
		OverPoint += GetSkillPoint_LevelQuest(lpCurPlayer->smCharInfo.Level, sinQuest_levelLog);

		if(OverPoint < point)
		{

			SkillError = 1;
			smTransCommand.LParam = point;
		}
	}
	else
	{
		if(point > 0)
		{
			SkillError = 1;
			smTransCommand.LParam = point;
		}
	}

	if(SkillError)
	{
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.code = OpCode::OPCODE_WARNING_SKILL;
		smTransCommand.WParam = SkillError;

		smTransCommand.SParam = lpCurPlayer->smCharInfo.Level;
		smTransCommand.EParam = 0;
		smTransCommand.EParam = dm_GetCommandChkSum(&smTransCommand, lpCurPlayer->dwObjectSerial);

	#ifdef _USE_DYNAMIC_ENCODE
		if(fnEncodeDamagePacket)
		{
			fnEncodeDamagePacket(&smTransCommand);

			if(smWsockDataServer)
				smWsockDataServer->Send((char *)&smTransCommand, smTransCommand.size, TRUE);
		}
	#else
		dm_EncodePacket(&smTransCommand);

		if(smWsockDataServer)
			smWsockDataServer->Send((char *)&smTransCommand, smTransCommand.size, TRUE);
	#endif
	}


	return TRUE;
}

















