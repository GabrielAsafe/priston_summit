// Client-only Summit Tier 5 SkillSub runtime imported from bugfix stack 091-158.
// Included only by the client SkillSub wrapper; server routing remains server-specific.
#include <stdio.h>
#include "systems/rendering/effects/legacy_ho/core/HoMinMax.h"
#include "systems/rendering/effects/particles/ho/HoNewParticle.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleMgr.h"

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

static int SummitHandleTier5PlaySkillAttack(sSKILL *lpSkill, smCHAR *lpChar, int *handled)
{
	int cnt;
	RECT rect;
	*handled = TRUE;

	switch(lpSkill->CODE)
	{
		case SKILL_T5_RUPTURE:
		if(lpSkill->Point && lpChar && lpSkill->Skill_Info.FuncPointer() == TRUE)
		{
			RetryPlayAttack(lpChar);
			lpCurPlayer->AttackSkil = SKILL_PLAY_T51 | (lpSkill->Point << 8);
			int summitRuptureBegin = lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil, 0, lpChar, 0, 0, 0);
			int summitRuptureMotion = lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_SKILL);
			SummitT5VisualProbe("Mechanician/Rupture", "cast-motion-control", lpCurPlayer, lpChar, summitRuptureBegin, summitRuptureMotion);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed, 4);
			lpCurPlayer->MotionLoop = 1;

			// EU Rupture has two damaging events: first with 70% of Grand Smash's
			// attack-rating bonus, then the full bonus on the final impact.
			int ruptureBaseAttackRating = lpCurPlayer->smCharInfo.Attack_Rating;
			int ruptureAttackBonus = Grand_Smash_AttackRate[lpSkill->Point - 1];
			for(int ruptureHit = 0; ruptureHit < 2; ruptureHit++)
			{
				lpCurPlayer->smCharInfo.Attack_Rating = ruptureBaseAttackRating +
					(ruptureHit == 0 ? (ruptureAttackBonus * 70) / 100 : ruptureAttackBonus);
				SummitT5StartParticle(ruptureHit == 0 ? "MC5RuptureHit" : "MC5RuptureFinalHit", lpChar);
				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
					DamageExp(lpChar, cnt);
			}
			lpCurPlayer->smCharInfo.Attack_Rating = ruptureBaseAttackRating;
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

			cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
			if(!cnt) lpCurPlayer->AttackCritcal = -1;
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
				SummitT5StartParticle("KS5HolyConviction", lpCurPlayer);
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
				SummitT5StartParticle("Ac5LethalSightKeep", lpCurPlayer);
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
			SummitT5StartParticle("Ac5FierceWind", lpCurPlayer);
			SummitT5StartParticle("Ac5FierceWindProjectile", lpChar);

			rect.left = -40;
			rect.right = 40;
			rect.top = 0;
			rect.bottom = lpCurPlayer->smCharInfo.Shooting_Range + 120;
			dm_SelectRangeBox(lpCurPlayer, &rect, FALSE);
			dm_SendRangeDamage(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame(lpCurPlayer->smCharInfo.Attack_Speed);
			lpCurPlayer->MotionLoop = 1;
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
			SummitT5StartParticle("R5SesmicImpact", lpChar);

			// EU behavior has two direct damaging events and a final area impact.
			for(int seismicHit = 0; seismicHit < 2; seismicHit++)
			{
				cnt = dm_SendTransDamage(lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil, TRUE);
				if(!cnt)
					lpCurPlayer->AttackCritcal = -1;
				else
					DamageExp(lpChar, cnt);
			}

			dm_SelectRange(lpChar->pX, lpChar->pY, lpChar->pZ, Fighter_T5_Seismic_Area[lpSkill->Point - 1], TRUE);
			dm_AddRangeDamage(lpChar);
			dm_SendRangeDamage(lpChar->pX, lpChar->pY, lpChar->pZ, lpChar, 0, 0, 0, 0, lpCurPlayer->AttackSkil);
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

		default:
			*handled = FALSE;
			return FALSE;
	}

	// Preserve the legacy PlaySkillAttack tail reached by T5 cases that use break.
	if(lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_ATTACK && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_SKILL)
		RetryPlayAttack(lpChar);

	return FALSE;
}
