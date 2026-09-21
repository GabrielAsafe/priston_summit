#include "gameplay/classes/v2/legacy/LegacyJobDataView.h"

enum LegacyClassItemRequirementRevisionMode
{
    LEGACY_CLASS_REVISION_RANDOM = 0,
    LEGACY_CLASS_REVISION_MINIMUM,
    LEGACY_CLASS_REVISION_MIDDLE
};

static int SelectLegacyClassItemRequirementRevision(
    const gameplay::classes::ClassPercentRange& range,
    LegacyClassItemRequirementRevisionMode mode)
{
    if(mode == LEGACY_CLASS_REVISION_MINIMUM)
        return range.minimum;
    if(mode == LEGACY_CLASS_REVISION_MIDDLE)
        return GetMiddlePos(range.minimum, range.maximum);
    return GetRandomPos_Normal(range.minimum, range.maximum);
}

static void ApplyLegacyClassItemRequirementRevision(
    sITEMINFO *lpItem,
    DWORD jobBitCode,
    LegacyClassItemRequirementRevisionMode mode)
{
    if(!lpItem || jobBitCode == 0)
        return;

    gameplay::classes::ClassItemRequirementRevisionProfile profile;
    if(!gameplay::classes::DefaultClassService().TryItemRequirementRevisionByJobBitCode(
           static_cast<std::uint32_t>(jobBitCode), profile))
        return;

    int selected = 0;
    int revision = 0;

#define APPLY_CLASS_REQUIREMENT_REVISION(field, rangeField) \
    do { \
        if(lpItem->field && profile.rangeField.IsActive()) { \
            selected = SelectLegacyClassItemRequirementRevision(profile.rangeField, mode); \
            revision = (lpItem->field * selected) / 100; \
            lpItem->field += revision; \
        } \
    } while(0)

    APPLY_CLASS_REQUIREMENT_REVISION(Level, level);
    APPLY_CLASS_REQUIREMENT_REVISION(Strength, strength);
    APPLY_CLASS_REQUIREMENT_REVISION(Spirit, spirit);
    APPLY_CLASS_REQUIREMENT_REVISION(Talent, talent);
    APPLY_CLASS_REQUIREMENT_REVISION(Dexterity, dexterity);
    APPLY_CLASS_REQUIREMENT_REVISION(Health, health);

#undef APPLY_CLASS_REQUIREMENT_REVISION
}

int CreateDefItem(sITEMINFO *lpItem, sDEF_ITEMINFO *lpDefItem, int SpJobCode, int ItemNormam)
{
	int is, ie;
	int cnt;
	JOB_DATA_CODE	*lpJobDataCode = 0;

	Random_NormalMode = ItemNormam;




	memcpy(lpItem, &lpDefItem->Item, sizeof(sITEMINFO));


	if(lpDefItem->sDurability[0])
	{

		if(lpDefItem->sDurability[1])
			lpItem->Durability[1] = GetRandomPos_Normal(lpDefItem->sDurability[0], lpDefItem->sDurability[1]);
		else
			lpItem->Durability[1] = lpDefItem->sDurability[0];

		ie = lpItem->Durability[1];
		is = ie / 2;
		lpItem->Durability[0] = GetRandomPos_Normal(is, ie);
	}


	for(cnt = 0; cnt < 8; cnt++)
	{
		if(lpDefItem->sResistance[cnt][1])
		{
			lpItem->Resistance[cnt] =
				GetRandomPos_Normal(lpDefItem->sResistance[cnt][0], lpDefItem->sResistance[cnt][1]);
		}
		else
			lpItem->Resistance[cnt] = lpDefItem->sResistance[cnt][0];
	}


	if(lpDefItem->sDamage[1] || lpDefItem->sDamage[3])
	{
		lpItem->Damage[0] = GetRandomPos_Normal(lpDefItem->sDamage[0], lpDefItem->sDamage[1]);
		lpItem->Damage[1] = GetRandomPos_Normal(lpDefItem->sDamage[2], lpDefItem->sDamage[3]);
	}


	if(lpDefItem->sAttack_Rating[1])
		lpItem->Attack_Rating = GetRandomPos_Normal(lpDefItem->sAttack_Rating[0], lpDefItem->sAttack_Rating[1]);
	else
		lpItem->Attack_Rating = lpDefItem->sAttack_Rating[0];


	if(lpDefItem->fAbsorb[1] != 0)
		lpItem->fAbsorb = GetRandomFloatPos2(lpDefItem->fAbsorb[0], lpDefItem->fAbsorb[1]);
	else
		lpItem->fAbsorb = lpDefItem->fAbsorb[0];


	if(lpDefItem->sDefence[1])
		lpItem->Defence = GetRandomPos_Normal(lpDefItem->sDefence[0], lpDefItem->sDefence[1]);
	else
		lpItem->Defence = lpDefItem->sDefence[0];


	if(lpDefItem->fBlock_Rating[1] != 0)
		lpItem->fBlock_Rating = GetRandomFloatPos(lpDefItem->fBlock_Rating[0], lpDefItem->fBlock_Rating[1]);
	else
		lpItem->fBlock_Rating = lpDefItem->fBlock_Rating[0];



	if(lpDefItem->fSpeed[1] != 0)
		lpItem->fSpeed = GetRandomFloatPos2(lpDefItem->fSpeed[0], lpDefItem->fSpeed[1]);
	else
		lpItem->fSpeed = lpDefItem->fSpeed[0];



	if(lpDefItem->fMana_Regen[1] != 0)
		lpItem->fMana_Regen = GetRandomFloatPos2(lpDefItem->fMana_Regen[0], lpDefItem->fMana_Regen[1]);
	else
		lpItem->fMana_Regen = lpDefItem->fMana_Regen[0];


	if(lpDefItem->fLife_Regen[1] != 0)
		lpItem->fLife_Regen = GetRandomFloatPos2(lpDefItem->fLife_Regen[0], lpDefItem->fLife_Regen[1]);
	else
		lpItem->fLife_Regen = lpDefItem->fLife_Regen[0];


	if(lpDefItem->fStamina_Regen[1] != 0)
		lpItem->fStamina_Regen = GetRandomFloatPos2(lpDefItem->fStamina_Regen[0], lpDefItem->fStamina_Regen[1]);
	else
		lpItem->fStamina_Regen = lpDefItem->fStamina_Regen[0];



	if(lpDefItem->Increase_Life[1])
		lpItem->fIncrease_Life = (float)GetRandomPos_Normal(lpDefItem->Increase_Life[0], lpDefItem->Increase_Life[1]);
	else
		lpItem->fIncrease_Life = (float)lpDefItem->Increase_Life[0];


	if(lpDefItem->Increase_Mana[1])
		lpItem->fIncrease_Mana = (float)GetRandomPos_Normal(lpDefItem->Increase_Mana[0], lpDefItem->Increase_Mana[1]);
	else
		lpItem->fIncrease_Mana = (float)lpDefItem->Increase_Mana[0];


	if(lpDefItem->Increase_Stamina[1])
		lpItem->fIncrease_Stamina = (float)GetRandomPos_Normal(lpDefItem->Increase_Stamina[0], lpDefItem->Increase_Stamina[1]);
	else
		lpItem->fIncrease_Stamina = (float)lpDefItem->Increase_Stamina[0];




	DWORD	dwSpecialJob;
	int		tempJobCode = 0;


	if(SpJobCode >= 100)
	{
		tempJobCode = SpJobCode - 100;
		SpJobCode = 0;
	}

	if(SpJobCode > 0)
	{
		lpJobDataCode = GetJobDataCode(SpJobCode, 0);

		if(lpJobDataCode)
		{
			dwSpecialJob = lpJobDataCode->JobBitCode;

			for(cnt = 0; cnt < lpDefItem->JobBitCodeRandomCount; cnt++)
			{
				if(lpDefItem->dwJobBitCode_Random[cnt] == dwSpecialJob) break;
			}
			if(cnt >= lpDefItem->JobBitCodeRandomCount)
			{
				if(dwSpecialJob != lpItem->JobCodeMask) SpJobCode = 0;
			}
		}
	}


	if(lpItem->JobCodeMask || lpDefItem->JobBitCodeRandomCount || SpJobCode)
	{



		cnt = rand() % 10;


		if(NotItemSpecial)
		{
			cnt = 100;
			NotItemSpecial = 0;
		}

		if(cnt <= 3 || SpJobCode)
		{


			cnt = rand() % 10;

			dwSpecialJob = lpItem->JobCodeMask;

			if(cnt < 3 && lpDefItem->JobBitCodeRandomCount)
			{
				lpItem->JobCodeMask = 0;
			}

			if(!lpItem->JobCodeMask && lpDefItem->JobBitCodeRandomCount)
			{
				if(tempJobCode > 0)
				{
					lpJobDataCode = GetJobDataCode(tempJobCode, 0);
					if(lpJobDataCode)
					{
						cnt = rand() % lpDefItem->JobBitCodeRandomCount;
						while(lpDefItem->dwJobBitCode_Random[cnt] == lpJobDataCode->JobBitCode)
						{
							cnt = rand() % lpDefItem->JobBitCodeRandomCount;
							if(lpDefItem->dwJobBitCode_Random[cnt] != lpJobDataCode->JobBitCode)
							{
								dwSpecialJob = lpDefItem->dwJobBitCode_Random[cnt];
								lpItem->JobCodeMask = dwSpecialJob;
								break;
							}
						}
					}
				}
				else
				{
					cnt = rand() % lpDefItem->JobBitCodeRandomCount;
					dwSpecialJob = lpDefItem->dwJobBitCode_Random[cnt];
					lpItem->JobCodeMask = dwSpecialJob;
				}
			}

			if(SpJobCode > 0)
			{

				lpJobDataCode = GetJobDataCode(SpJobCode, 0);
				if(lpJobDataCode)
				{
					dwSpecialJob = lpJobDataCode->JobBitCode;
					lpItem->JobCodeMask = dwSpecialJob;
				}
			}


			if(lpItem->JobCodeMask) lpItem->Price += (lpItem->Price * 2) / 10;




			ApplyLegacyClassItemRequirementRevision(lpItem, dwSpecialJob, LEGACY_CLASS_REVISION_RANDOM);


			if(lpDefItem->fSpecial_Absorb[1] != 0)
			{
				lpItem->JobItem.Add_fAbsorb = GetRandomFloatPos(lpDefItem->fSpecial_Absorb[0], lpDefItem->fSpecial_Absorb[1]);
			}
			else
			{
				lpItem->JobItem.Add_fAbsorb = lpDefItem->fSpecial_Absorb[0];
			}


			if(lpDefItem->sSpecial_Defence[1])
			{
				lpItem->JobItem.Add_Defence = GetRandomPos_Normal(lpDefItem->sSpecial_Defence[0], lpDefItem->sSpecial_Defence[1]);
			}
			else
			{
				lpItem->JobItem.Add_Defence = lpDefItem->sSpecial_Defence[0];
			}


			if(lpDefItem->fSpecial_fSpeed[1] != 0)
			{
				lpItem->JobItem.Add_fSpeed = GetRandomFloatPos(lpDefItem->fSpecial_fSpeed[0], lpDefItem->fSpecial_fSpeed[1]);
			}
			else
			{
				lpItem->JobItem.Add_fSpeed = lpDefItem->fSpecial_fSpeed[0];
			}


			if(lpDefItem->fSpecial_Magic_Mastery[1] != 0)
			{
				lpItem->JobItem.Add_fMagic_Mastery = GetRandomFloatPos(lpDefItem->fSpecial_Magic_Mastery[0], lpDefItem->fSpecial_Magic_Mastery[1]);
			}
			else
			{
				lpItem->JobItem.Add_fMagic_Mastery = lpDefItem->fSpecial_Magic_Mastery[0];
			}


			if(lpDefItem->fSpecial_Mana_Regen[1] != 0)
			{
				lpItem->JobItem.Per_Mana_Regen = GetRandomFloatPos(lpDefItem->fSpecial_Mana_Regen[0], lpDefItem->fSpecial_Mana_Regen[1]);
			}
			else
			{
				lpItem->JobItem.Per_Mana_Regen = lpDefItem->fSpecial_Mana_Regen[0];
			}



			if(lpDefItem->Lev_Attack_Rating[1])
			{
				lpItem->JobItem.Lev_Attack_Rating = GetRandomPos_Normal(lpDefItem->Lev_Attack_Rating[0], lpDefItem->Lev_Attack_Rating[1]);
			}
			else
			{
				lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[0];
			}


		}
		else
		{

			lpItem->JobCodeMask = 0;
		}
	}

	if(!lpItem->JobCodeMask)
	{
		ZeroMemory(&lpItem->JobItem, sizeof(sITEM_SPECIAL));
	}





	ReformItem(lpItem);

	return	TRUE;
}


int CreateDefItem(sITEMINFO *lpItem, sDEF_ITEMINFO *lpDefItem, int SpJobCode)
{
	return CreateDefItem(lpItem, lpDefItem, SpJobCode, 0);
}

int CreateDefItemPerf(sITEMINFO *lpItem, sDEF_ITEMINFO *lpDefItem, int SpJobCode)
{
	int cnt;
	JOB_DATA_CODE	*lpJobDataCode = 0;


	memcpy(lpItem, &lpDefItem->Item, sizeof(sITEMINFO));

	if (lpDefItem->sDurability[0])
	{
		if (lpDefItem->sDurability[1])
		{
			lpItem->Durability[0] = lpDefItem->sDurability[1];
			lpItem->Durability[1] = lpDefItem->sDurability[1];
		}
		else
		{
			lpItem->Durability[0] = lpDefItem->sDurability[0];
			lpItem->Durability[1] = lpDefItem->sDurability[0];
		}
	}

	for (cnt = 0; cnt < 8; cnt++)
	{
		if (lpDefItem->sResistance[cnt][1])
			lpItem->Resistance[cnt] = lpDefItem->sResistance[cnt][1];
		else
			lpItem->Resistance[cnt] = lpDefItem->sResistance[cnt][0];
	}

	if (lpDefItem->sDamage[1] || lpDefItem->sDamage[3])
	{
		lpItem->Damage[0] = lpDefItem->sDamage[1];
		lpItem->Damage[1] = lpDefItem->sDamage[3];
	}

	if (lpDefItem->sAttack_Rating[1])
		lpItem->Attack_Rating = lpDefItem->sAttack_Rating[1];
	else
		lpItem->Attack_Rating = lpDefItem->sAttack_Rating[0];

	if (lpDefItem->fAbsorb[1] != 0)
		lpItem->fAbsorb = lpDefItem->fAbsorb[1];
	else
		lpItem->fAbsorb = lpDefItem->fAbsorb[0];

	if (lpDefItem->sDefence[1])
		lpItem->Defence = lpDefItem->sDefence[1];
	else
		lpItem->Defence = lpDefItem->sDefence[0];

	if (lpDefItem->fBlock_Rating[1] != 0)
		lpItem->fBlock_Rating = lpDefItem->fBlock_Rating[1];
	else
		lpItem->fBlock_Rating = lpDefItem->fBlock_Rating[0];

	if (lpDefItem->fSpeed[1] != 0)
		lpItem->fSpeed = lpDefItem->fSpeed[1];
	else
		lpItem->fSpeed = lpDefItem->fSpeed[0];


	if (lpDefItem->fMana_Regen[1] != 0)
		lpItem->fMana_Regen = lpDefItem->fMana_Regen[1];
	else
		lpItem->fMana_Regen = lpDefItem->fMana_Regen[0];

	if (lpDefItem->fLife_Regen[1] != 0)
		lpItem->fLife_Regen = lpDefItem->fLife_Regen[1];
	else
		lpItem->fLife_Regen = lpDefItem->fLife_Regen[0];

	if (lpDefItem->fStamina_Regen[1] != 0)
		lpItem->fStamina_Regen = lpDefItem->fStamina_Regen[1];
	else
		lpItem->fStamina_Regen = lpDefItem->fStamina_Regen[0];


	if (lpDefItem->Increase_Life[1])
		lpItem->fIncrease_Life = (float)lpDefItem->Increase_Life[1];
	else
		lpItem->fIncrease_Life = (float)lpDefItem->Increase_Life[0];

	if (lpDefItem->Increase_Mana[1])
		lpItem->fIncrease_Mana = (float)lpDefItem->Increase_Mana[1];
	else
		lpItem->fIncrease_Mana = (float)lpDefItem->Increase_Mana[0];

	if (lpDefItem->Increase_Stamina[1])
		lpItem->fIncrease_Stamina = (float)lpDefItem->Increase_Stamina[1];
	else
		lpItem->fIncrease_Stamina = (float)lpDefItem->Increase_Stamina[0];


	DWORD	dwSpecialJob;

	if (SpJobCode > 0)
	{
		lpJobDataCode = GetJobDataCode(SpJobCode, 0);

		if (lpJobDataCode)
		{
			dwSpecialJob = lpJobDataCode->JobBitCode;

			for (cnt = 0; cnt < lpDefItem->JobBitCodeRandomCount; cnt++)
			{
				if (lpDefItem->dwJobBitCode_Random[cnt] == dwSpecialJob) break;
			}
			if (cnt >= lpDefItem->JobBitCodeRandomCount)
			{
				if (dwSpecialJob != lpItem->JobCodeMask) SpJobCode = 0;
			}
		}
	}

	if (SpJobCode > 0)
	{
		lpJobDataCode = GetJobDataCode(SpJobCode, 0);
		if (lpJobDataCode)
		{
			dwSpecialJob = lpJobDataCode->JobBitCode;
			lpItem->JobCodeMask = dwSpecialJob;
		}

		if (lpItem->JobCodeMask) lpItem->Price += (lpItem->Price * 2) / 10;

		ApplyLegacyClassItemRequirementRevision(lpItem, dwSpecialJob, LEGACY_CLASS_REVISION_MINIMUM);

		if (lpDefItem->fSpecial_Absorb[1] != 0)
			lpItem->JobItem.Add_fAbsorb = lpDefItem->fSpecial_Absorb[1];
		else
			lpItem->JobItem.Add_fAbsorb = lpDefItem->fSpecial_Absorb[0];

		if (lpDefItem->sSpecial_Defence[1])
			lpItem->JobItem.Add_Defence = lpDefItem->sSpecial_Defence[1];
		else
			lpItem->JobItem.Add_Defence = lpDefItem->sSpecial_Defence[0];

		if (lpDefItem->fSpecial_fSpeed[1] != 0)
			lpItem->JobItem.Add_fSpeed = lpDefItem->fSpecial_fSpeed[1];
		else
			lpItem->JobItem.Add_fSpeed = lpDefItem->fSpecial_fSpeed[0];

		if (lpDefItem->fSpecial_Magic_Mastery[1] != 0)
			lpItem->JobItem.Add_fMagic_Mastery = lpDefItem->fSpecial_Magic_Mastery[1];
		else
			lpItem->JobItem.Add_fMagic_Mastery = lpDefItem->fSpecial_Magic_Mastery[0];

		if (lpDefItem->fSpecial_Mana_Regen[1] != 0)
			lpItem->JobItem.Per_Mana_Regen = lpDefItem->fSpecial_Mana_Regen[1];
		else
			lpItem->JobItem.Per_Mana_Regen = lpDefItem->fSpecial_Mana_Regen[0];

		/*if (lpDefItem->Lev_Attack_Rating[1])
			lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[1];
		else
			lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[0];*/

		if (!lpDefItem->Lev_Attack_Rating[0] && lpDefItem->Lev_Attack_Rating[1])
			lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[1];
		else if (lpDefItem->Lev_Attack_Rating[0])
			lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[0];

		lpItem->SpecialItemFlag[1] = ITEM_KIND_NORMAL;
	}
	else
	{
		lpItem->JobCodeMask = 0;
	}

	if (!lpItem->JobCodeMask)
	{
		ZeroMemory(&lpItem->JobItem, sizeof(sITEM_SPECIAL));
	}

	ReformItem(lpItem);

	return	TRUE;
}

int SetLowerItem(sDEF_ITEMINFO *lpDefItem)
{
	int ie;
	int cnt;

	sITEMINFO *lpItem;
	lpItem = &lpDefItem->Item;


	if(lpDefItem->sDurability[0])
	{
		ie = lpDefItem->sDurability[0];
		lpItem->Durability[0] = ie;
		lpItem->Durability[1] = ie;
	}


	for(cnt = 0; cnt < 8; cnt++)
	{
		lpItem->Resistance[cnt] = lpDefItem->sResistance[cnt][0];
	}


	if(lpDefItem->sDamage[1] || lpDefItem->sDamage[3])
	{
		lpItem->Damage[0] = lpDefItem->sDamage[0];
		lpItem->Damage[1] = lpDefItem->sDamage[2];
	}

	lpItem->Attack_Rating = lpDefItem->sAttack_Rating[0];
	lpItem->fAbsorb = lpDefItem->fAbsorb[0];
	lpItem->Defence = lpDefItem->sDefence[0];
	lpItem->fBlock_Rating = lpDefItem->fBlock_Rating[0];
	lpItem->fSpeed = lpDefItem->fSpeed[0];

	lpItem->fMana_Regen = lpDefItem->fMana_Regen[0];
	lpItem->fLife_Regen = lpDefItem->fLife_Regen[0];
	lpItem->fStamina_Regen = lpDefItem->fStamina_Regen[0];

	lpItem->fIncrease_Life = (float)lpDefItem->Increase_Life[0];
	lpItem->fIncrease_Mana = (float)lpDefItem->Increase_Mana[0];
	lpItem->fIncrease_Stamina = (float)lpDefItem->Increase_Stamina[0];



	DWORD	dwSpecialJob;


	if(lpItem->JobCodeMask)
	{


		dwSpecialJob = lpItem->JobCodeMask;


		ApplyLegacyClassItemRequirementRevision(lpItem, dwSpecialJob, LEGACY_CLASS_REVISION_MIDDLE);


		lpItem->JobItem.Add_fAbsorb = lpDefItem->fSpecial_Absorb[0];


		lpItem->JobItem.Add_Defence = lpDefItem->sSpecial_Defence[0];


		lpItem->JobItem.Add_fSpeed = lpDefItem->fSpecial_fSpeed[0];


		lpItem->JobItem.Add_fMagic_Mastery = lpDefItem->fSpecial_Magic_Mastery[0];


		lpItem->JobItem.Per_Mana_Regen = lpDefItem->fSpecial_Mana_Regen[0];


		if(lpDefItem->Lev_Attack_Rating[1])
			lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[0] + (lpDefItem->Lev_Attack_Rating[1] - lpDefItem->Lev_Attack_Rating[0]) / 2;
		else
			lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[0];

	}
	else
	{
		ZeroMemory(&lpItem->JobItem, sizeof(sITEM_SPECIAL));
	}


	ReformItem(lpItem);

	return	TRUE;
}



namespace
{
struct V2SyntheticJobDataCode
{
    char name[128];
    char displayName[128];
    JOB_DATA_CODE value;

    V2SyntheticJobDataCode()
    {
        name[0] = 0;
        displayName[0] = 0;
        ZeroMemory(&value, sizeof(value));
    }
};

JOB_DATA_CODE *GetV2JobDataCodeFallback(int legacyJobCode, int legacyRankIndex)
{
    typedef std::map<unsigned int, V2SyntheticJobDataCode> Cache;
    static Cache cache;

    const unsigned int cacheKey =
        (static_cast<unsigned int>(legacyJobCode) << 8) | static_cast<unsigned int>(legacyRankIndex & 0xFF);
    Cache::iterator existing = cache.find(cacheKey);
    if(existing != cache.end())
        return &existing->second.value;

    gameplay::classes::legacy::LegacyJobDataView view;
    if(!gameplay::classes::legacy::BuildLegacyJobDataView(
           gameplay::classes::DefaultClassService(), legacyJobCode, legacyRankIndex, view))
        return NULL;

    V2SyntheticJobDataCode &entry = cache[cacheKey];
    std::snprintf(entry.name, sizeof(entry.name), "%s", view.name.c_str());
    std::snprintf(entry.displayName, sizeof(entry.displayName), "%s", view.displayName.c_str());
    entry.value.szName = entry.name;
    entry.value.szName2 = entry.displayName;
    entry.value.JobBitCode = static_cast<DWORD>(view.jobBitCode);
    entry.value.JobCode = static_cast<DWORD>(view.jobCode);
    entry.value.Brood = static_cast<DWORD>(view.brood);
    entry.value.Sight = view.formula.sight;
    entry.value.LifeFunction = view.formula.lifeFunction;
    entry.value.ManaFunction = view.formula.manaFunction;
    entry.value.StaminaFunction = view.formula.staminaFunction;
    entry.value.DamageFunction[0] = static_cast<short>(view.formula.damageFunction[0]);
    entry.value.DamageFunction[1] = static_cast<short>(view.formula.damageFunction[1]);
    entry.value.DamageFunction[2] = static_cast<short>(view.formula.damageFunction[2]);
    return &entry.value;
}
}

JOB_DATA_CODE *GetJobDataCode(DWORD BaseJob, int JobCnt)
{
	int	JobCount = 0;
	int	JobCount2 = JobCnt;
	int	bJob = BaseJob & 0xFFFF;

	if(!JobCnt)
	{
		JobCount2 = BaseJob >> 24;
	}

	// ChangeJob validity belongs to the class rank chain. Legacy tables below
	// only provide localized rows for their historical ranks; registry-only
	// ranks/classes are synthesized by the V2 compatibility fallback.
	if(JobCount2 < 0 ||
		!gameplay::classes::DefaultClassService().IsValidLegacyChangeJob(bJob, JobCount2))
		return NULL;

	if(JobCount2 == CHARACTER_RANK5)
	{
		for(int rank5Index = 0; JobDataRank5[rank5Index].szName[0]; rank5Index++)
		{
			if(bJob == JobDataRank5[rank5Index].JobCode)
				return &JobDataRank5[rank5Index];
		}
		return GetV2JobDataCodeFallback(bJob, JobCount2);
	}

	int cnt = 0;
	while(1)
	{
		if(!JobDataBase[cnt].szName[0])
			break;
		if(bJob == JobDataBase[cnt].JobCode)
		{
			if(JobCount2 == JobCount)
				return &JobDataBase[cnt];
			JobCount++;
		}
		cnt++;
	}

	return GetV2JobDataCodeFallback(bJob, JobCount2);
}


#ifdef _LANGUAGE_BRAZIL
#include "systems/localization/legacy_catalog/Brazil/b_JobCode.h"
#endif



SKILL_DATA_CODE	SkillDataCode[] = {
{ "xxxxxxxxxxxxx"			,	0 },

	//MECHANICIAN
{ "ExtremeShield"			,	SKILL_PLAY_EXTREME_SHIELD },
{ "MechanicBomb"			,	SKILL_PLAY_MECHANIC_BOMB },
{ "PhysicalAbsorption"		,	SKILL_PLAY_PHYSICAL_ABSORB },
{ "PoisonAttribute"			,	0 },

{ "roundimpact"				,	SKILL_PLAY_GREAT_SMASH },
{ "Maximize"				,	SKILL_PLAY_MAXIMIZE },
{ "Automation"				,	SKILL_PLAY_AUTOMATION },
{ "Spark"					,	SKILL_PLAY_SPARK },

{ "MetalArmor"				,	SKILL_PLAY_METAL_ARMOR },
{ "GrandSmash"				,	SKILL_PLAY_GRAND_SMASH },
//{ "MechanicMastery"		,	0 },
{ "SparkShield"				,	SKILL_PLAY_SPARK_SHIELD },

{ "IMPULSION"				,	SKILL_PLAY_IMPULSION },
{ "COMPULSION"				,	SKILL_PLAY_COMPULSION },
{ "MAGNETICSPHERE"			,	SKILL_PLAY_MAGNETIC_SPHERE },
{ "METALGOLEM"				,	SKILL_PLAY_METAL_GOLEM },

// Summit Tier 5 motion registry. The legacy motion loader resolves animation
// SkillCodeList names through this table; keep both the old animation aliases
// and the current EU skill names mapped to the shared T51..T54 play slots.
{ "LandMining"			,	SKILL_PLAY_T51 },
{ "Rupture"				,	SKILL_PLAY_T51 },
{ "HyperSonic"			,	SKILL_PLAY_T52 },
{ "ParasitShot"			,	SKILL_PLAY_T52 },
{ "RollingSmash"			,	SKILL_PLAY_T53 },
{ "HardeningShield"		,	SKILL_PLAY_T53 },
{ "PowerEnhance"			,	SKILL_PLAY_T54 },
{ "MagneticDischarge"	,	SKILL_PLAY_T54 },

//FIGHTER
{ "MeleeMastery"			,	0 },
{ "FireAttribute"			,	0 },
{ "Raving"					,	SKILL_PLAY_RAVING },
{ "Impact"					,	SKILL_PLAY_IMPACT },

{ "TripleImpact"			,	SKILL_PLAY_TRIPLE_IMPACT },
{ "BrutalSwing"				,	SKILL_PLAY_BRUTAL_SWING },
{ "Roar"					,	SKILL_PLAY_ROAR },
{ "RageofZecram"			,	SKILL_PLAY_RAGEOF_ZECRAM },

{ "Concentration"			,	SKILL_PLAY_CONCENTRATION },
{ "AvangingCrash"			,	SKILL_PLAY_AVANGING_CRASH },
{ "SwiftAxe"				,	SKILL_PLAY_SWIFT_AXE },
{ "BoneSmash"				,	SKILL_PLAY_BONE_SMASH },

{ "DESTROYER"				,	SKILL_PLAY_DESTROYER },
{ "BERSERKER"				,	SKILL_PLAY_BERSERKER },
{ "CYCLONESTRIKE"			,	SKILL_PLAY_CYCLONE_STRIKE },
//{ "BoostHealth"			,	0 },

{ "DOWNHIT"				,	SKILL_PLAY_T51 },
{ "SesmicImpact"			,	SKILL_PLAY_T51 },
{ "SeismicImpact"			,	SKILL_PLAY_T51 },
{ "POWERDASH"				,	SKILL_PLAY_T52 },
{ "MOTALBLOW"				,	SKILL_PLAY_T53 },
{ "RuthlessofZecram"		,	SKILL_PLAY_T53 },
{ "BLOODYBERSERKER"		,	SKILL_PLAY_T54 },
{ "Whirlwind"				,	SKILL_PLAY_T54 },

//PIKEMAN
{ "PikeWind"				,	SKILL_PLAY_PIKEWIND },
{ "IceAttribute"			,	0 },
{ "CriticalHit"				,	SKILL_PLAY_CRITICAL_HIT },
{ "JumpingCrash"			,	SKILL_PLAY_JUMPING_CRASH },

{ "GroundPike"				,	SKILL_PLAY_GROUND_PIKE },
{ "Tornado"					,	SKILL_PLAY_TORNADO },
{ "WeaponDefenceMastery"	,	0 },
{ "Expansion"				,	SKILL_PLAY_EXPANSION },

{ "VenomSpear"				,	SKILL_PLAY_VENOM_SPEAR },
//{ "Vanish"				,	SKILL_PLAY_VANISH },
{ "BladeOfBlaze"			,	SKILL_PLAY_BLADE_OF_BLAZE },//{ "CriticalMastery"		,	0 },
{ "ChainLance"				,	SKILL_PLAY_CHAIN_LANCE },

{ "ASSASSINEYE"				,	SKILL_PLAY_ASSASSIN_EYE },
{ "CHARGINGSTRIKE"			,	SKILL_PLAY_CHARGING_STRIKE },
{ "VAGUE"					,	SKILL_PLAY_VAGUE },
{ "SHADOWMASTER"			,	SKILL_PLAY_SHADOW_MASTER },

{ "DANCINGREAPER"			,	SKILL_PLAY_T51 },
{ "RingofSpears"			,	SKILL_PLAY_T51 },
{ "RingOfSpears"			,	SKILL_PLAY_T51 },
{ "FINALSPEAR"			,	SKILL_PLAY_T52 },
{ "Focus"					,	SKILL_PLAY_T52 },
{ "DeathMantra"			,	SKILL_PLAY_T52 },
{ "AMPLIFIED"				,	SKILL_PLAY_T53 },
{ "DeathMaster"			,	SKILL_PLAY_T53 },
{ "ExDeath"				,	SKILL_PLAY_T53 },
{ "SIDESPINATTACK"		,	SKILL_PLAY_T54 },
{ "Twister"				,	SKILL_PLAY_T54 },
{ "SinisterSpin"			,	SKILL_PLAY_T54 },

//ARCHER
{ "ScoutHawk"				,	SKILL_PLAY_SCOUT_HAWK },
{ "ShootingMastery"			,	0 },
{ "WindArrow"				,	SKILL_PLAY_WIND_ARROW },
{ "PerfectAim"				,	SKILL_PLAY_PERFECT_AIM },

{ "DionsEye"				,	0 },
{ "Falcon"					,	SKILL_PLAY_FALCON },
{ "ArrowofRage"				,	SKILL_PLAY_ARROWOF_RAGE },
{ "Avalanche"				,	SKILL_PLAY_AVALANCHE },

{ "ElementalShot"			,	SKILL_PLAY_ELEMENTAL_SHOT },
{ "GoldenFalcon"			,	SKILL_PLAY_GOLDEN_FALCON },
{ "BombShot"				,	SKILL_PLAY_BOMB_SHOT },
{ "Perforation"				,	SKILL_PLAY_PERFORATION },

{ "RECALLWOLVERIN"			,	SKILL_PLAY_RECALL_WOLVERIN },
//{ "EvasionMastery"		,	0 },
{ "PHOENIXSHOT"				,	SKILL_PLAY_PHOENIX_SHOT },
{ "FORCEOFNATURE"			,	SKILL_PLAY_FORCE_OF_NATURE },

{ "EVADESHOT"				,	SKILL_PLAY_T51 },
{ "LethalSight"			,	SKILL_PLAY_T51 },
{ "THUNDERROOP"			,	SKILL_PLAY_T52 },
{ "FierceWind"				,	SKILL_PLAY_T52 },
{ "NATURESPLASH"			,	SKILL_PLAY_T53 },
{ "EntalingRoots"			,	SKILL_PLAY_T53 },
{ "EntanglingRoots"		,	SKILL_PLAY_T53 },
{ "CIRCLETRAP"			,	SKILL_PLAY_T54 },
{ "Bombardment"			,	SKILL_PLAY_T54 },

//KNIGHT
{ "SwordBlast"				,	SKILL_PLAY_SWORD_BLAST },
{ "HolyBody"				,	SKILL_PLAY_HOLY_BODY },
{ "PhysicalTraning"			,	0 },
{ "DoubleCrash"				,	SKILL_PLAY_DOUBLE_CRASH },

{ "holyteam"				,	SKILL_PLAY_HOLY_VALOR },
{ "Brandish"				,	SKILL_PLAY_BRANDISH },
{ "Piercing"				,	SKILL_PLAY_PIERCING },
{ "DrasticSpirit"			,	SKILL_PLAY_DRASTIC_SPIRIT },

{ "FlameBrandish"			,	SKILL_PLAY_FLAME_BRANDISH },//{ "SwordMastery"			,	0 },
{ "Divineinhalation"		,	SKILL_PLAY_DIVINE_INHALATION },
{ "HolyIncantation"			,	SKILL_PLAY_HOLY_INCANTATION },
{ "GrandCross"				,	SKILL_PLAY_GRAND_CROSS },

{ "DIVINEPIERCING"			,	SKILL_PLAY_DIVINE_PIERCING },
{ "DIVINEPIERCING2"			,	SKILL_PLAY_DIVINE_PIERCING2 },
{ "DIVINEPIERCING3"			,	SKILL_PLAY_DIVINE_PIERCING3 },
{ "GODLYSHIELD"				,	SKILL_PLAY_GODLY_SHIELD },
{ "GODSBLESS"				,	SKILL_PLAY_GODS_BLESS },
{ "SWORDOFJUSTICE"			,	SKILL_PLAY_SWORD_OF_JUSTICE },

{ "SOULBREAKER"			,	SKILL_PLAY_T51 },
{ "HolyConviction"			,	SKILL_PLAY_T51 },
{ "CRESCENTMOON"			,	SKILL_PLAY_T52 },
{ "DivineInquisition"		,	SKILL_PLAY_T52 },
{ "SAINTBLADE"			,	SKILL_PLAY_T53 },
{ "GloriousShield"			,	SKILL_PLAY_T53 },
{ "HOLYBENEDIC"			,	SKILL_PLAY_T54 },
{ "DivineCross"				,	SKILL_PLAY_T54 },

//ATALANTA
{ "ShieldStrike"			,	SKILL_PLAY_SHIELD_STRIKE },
{ "Farina"					,	SKILL_PLAY_FARINA },
{ "ThrowingMastery"			,	0 },
{ "VigorSpear"				,	SKILL_PLAY_VIGOR_SPEAR },

{ "Windy"					,	SKILL_PLAY_WINDY },
{ "TwistJavelin"			,	SKILL_PLAY_TWIST_JAVELIN },
{ "SoulSucker"				,	SKILL_PLAY_SOUL_SUCKER },
{ "FireJavelin"				,	SKILL_PLAY_FIRE_JAVELIN },

{ "SplitJavelin"			,	SKILL_PLAY_SPLIT_JAVELIN },
{ "TriumphOfValhalla"		,	SKILL_PLAY_TRIUMPH_OF_VALHALLA },
{ "LightningJavelin"		,	SKILL_PLAY_LIGHTNING_JAVELIN },
{ "StormJavelin"			,	SKILL_PLAY_STORM_JAVELIN },

{ "HALLOFVALHALLA"			,	SKILL_PLAY_HALL_OF_VALHALLA },
{ "XRAGE"					,	SKILL_PLAY_X_RAGE },
{ "FROSTJAVELIN"			,	SKILL_PLAY_FROST_JAVELIN },
{ "VENGEANCE"				,	SKILL_PLAY_VENGEANCE },

{ "TALARIA"				,	SKILL_PLAY_T51 },
{ "ComboJavelin"			,	SKILL_PLAY_T51 },
{ "SummonTiger"			,	SKILL_PLAY_T51 },
{ "GALAXYCOUP"			,	SKILL_PLAY_T52 },
{ "GoldenApple"			,	SKILL_PLAY_T52 },
{ "SUMMONARCUDA"			,	SKILL_PLAY_T53 },
{ "PlagueJavelin"			,	SKILL_PLAY_T53 },
{ "SNIPPYFEAR"			,	SKILL_PLAY_T54 },
{ "TripleThreat"			,	SKILL_PLAY_T54 },

//PRIESTESS
{ "Healing"					,	SKILL_PLAY_HEALING },
{ "HolyBolt"				,	SKILL_PLAY_HOLY_BOLT },
{ "MultiSpark"				,	SKILL_PLAY_MULTI_SPARK },
{ "HolyMind"				,	SKILL_PLAY_HOLY_MIND },

{ "Meditation"				,	0 },
{ "DivineLightning"			,	SKILL_PLAY_DIVINE_LIGHTNING },
{ "HolyReflection"			,	SKILL_PLAY_HOLY_REFLECTION },
{ "GrandHealing"			,	SKILL_PLAY_GREAT_HEALING },

{ "VigorBall"				,	SKILL_PLAY_VIGOR_BALL },
{ "Resurrection"			,	SKILL_PLAY_RESURRECTION },
{ "Extinction"				,	SKILL_PLAY_EXTINCTION },
{ "VirtualLife"				,	SKILL_PLAY_VIRTUAL_LIFE },

{ "GLACIALSPIKE"			,	SKILL_PLAY_GLACIAL_SPIKE },
{ "REGENERATIONFIELD"		,	SKILL_PLAY_REGENERATION_FIELD },
{ "CHAINLIGHTNING"			,	SKILL_PLAY_CHAIN_LIGHTNING },
{ "SUMMONMUSPELL"			,	SKILL_PLAY_SUMMON_MUSPELL },

{ "SpiritImpact"			,	SKILL_PLAY_T51 },
{ "IceElemental"			,	SKILL_PLAY_T51 },
{ "PiercingIce"			,	SKILL_PLAY_T52 },
{ "LightningSurge"			,	SKILL_PLAY_T52 },
{ "SummonRamiel"			,	SKILL_PLAY_T53 },
{ "HeavenlyLight"			,	SKILL_PLAY_T53 },
{ "BlessingKrishna"		,	SKILL_PLAY_T54 },
{ "Consecration"			,	SKILL_PLAY_T54 },

//MAGICIAN
{ "Agony"					,	SKILL_PLAY_AGONY },
{ "FireBolt"				,	SKILL_PLAY_FIRE_BOLT },
{ "Zenith"					,	SKILL_PLAY_ZENITH },
{ "FireBall"				,	SKILL_PLAY_FIRE_BALL },

{ "MentalMastery"			,	0 },
{ "ColumnOfWater"			,	SKILL_PLAY_COLUMN_OF_WATER },
{ "EnchantWeapon"			,	SKILL_PLAY_ENCHANT_WEAPON },
{ "DeadRay"					,	SKILL_PLAY_DEAD_RAY },

{ "EnergyShield"			,	SKILL_PLAY_ENERGY_SHIELD },
{ "Diastrophism"			,	SKILL_PLAY_DIASTROPHISM },
{ "SpiritElemental"			,	SKILL_PLAY_SPIRIT_ELEMENTAL },
{ "DancingSword"			,	SKILL_PLAY_DANCING_SWORD },

{ "FIREELEMENTAL"			,	SKILL_PLAY_FIRE_ELEMENTAL },
{ "FLAMEWAVE"				,	SKILL_PLAY_FLAME_WAVE },
{ "DISTORTION"				,	SKILL_PLAY_DISTORTION },
{ "METEO"					,	SKILL_PLAY_METEO },

{ "Silraphim"				,	SKILL_PLAY_T51 },
{ "StoneSkin"				,	SKILL_PLAY_T51 },
{ "Vistenus"				,	SKILL_PLAY_T52 },
{ "RedRay"					,	SKILL_PLAY_T52 },
{ "Primaignis"			,	SKILL_PLAY_T53 },
{ "Thirdanima"			,	SKILL_PLAY_T54 },
{ "Cataclysm"				,	SKILL_PLAY_T54 },

//
////ASSASSIN
//{ "Stingger"				,	SKILL_PLAY_STINGGER },
//{ "RunningHit"			,	SKILL_PLAY_RUNNING_HIT },
//{ "DualSwordMastery"		,	0 },
//{ "Wisp"					,	SKILL_PLAY_WISP },
//
//{ "VenomThrone"			,	SKILL_PLAY_VENOM_THRONE },
//{ "Alas"					,	SKILL_PLAY_ALAS },
//{ "SoulShock"				,	SKILL_PLAY_SOUL_SHOCK },
//{ "AttackMastery"			,	0 },
//
//{ "SoreSword"				,	SKILL_PLAY_SORE_SWORD },
//{ "BeatUp"				,	SKILL_PLAY_BEAT_UP },
//{ "Inpes"					,	SKILL_PLAY_INPES },
//{ "Blind"					,	SKILL_PLAY_BLIND },
//
//{ "FrostWind"				,	SKILL_PLAY_FROST_WIND },
//{ "CriticalMastery"		,	0 },
//{ "Polluted"				,	SKILL_PLAY_POLLUTED },
//{ "PastingShadow"			,	SKILL_PLAY_PASTING_SHADOW },
//
//{ "ShadowBomb"			,	SKILL_PLAY_SHADOW_BOMB },
//{ "RisingSlash"			,	SKILL_PLAY_RISING_SLASH },
//{ "ViolenceStab"			,	SKILL_PLAY_VIOLENCE_STAB },
//{ "Storm"					,	SKILL_PLAY_STORM },
//
////SHAMAN
//{ "DarkBolt"				,	SKILL_PLAY_DARK_BOLT },
//{ "DarkWave"				,	SKILL_PLAY_DARK_WAVE },
//{ "CurseLazy"				,	SKILL_PLAY_CURSE_LAZY },
//{ "InnerPeace"			,	0 },
//
//{ "SpiritualFlare"		,	SKILL_PLAY_SPIRITUAL_FLARE },
//{ "SoulManacle"			,	SKILL_PLAY_SOUL_MANACLE },
//{ "ChasingHunt"			,	SKILL_PLAY_CHASING_HUNT },
//{ "AdventMigal"			,	SKILL_PLAY_ADVENT_MIGAL },
//
//{ "RainMaker"				,	SKILL_PLAY_RAIN_MAKER },
//{ "LandOfGhost"			,	SKILL_PLAY_LAND_OF_GHOST },
//{ "Haunt"					,	SKILL_PLAY_HAUNT },
//{ "Scratch"				,	SKILL_PLAY_SCRATCH },
//
//{ "RecallBloodyKnight"	,	SKILL_PLAY_RECALL_BLOODY_KNIGHT },
//{ "Judgement"				,	SKILL_PLAY_JUDGEMENT },
//{ "AdventMidranda"		,	SKILL_PLAY_ADVENT_MIDRANDA },
//{ "MourningOfPray"		,	SKILL_PLAY_MOURNING_OF_PRAY },
//
//{ "Creed"					,	SKILL_PLAY_CREED },
//{ "PressOfDeity"			,	SKILL_PLAY_PRESS_OF_DEITY },
//{ "GhostyNail"			,	SKILL_PLAY_GHOST_NAIL },
//{ "HighRegeneration"		,	SKILL_HIGH_REGENERATION },
//
////
//{ "JUMPATTACK"			,	SKILL_PLAY_JUMPATTACK },
//{ "CONTATTACK1"			,	SKILL_PLAY_CONTATTACK1 },
//{ "CONTATTACK2"			,	SKILL_PLAY_CONTATTACK2 },
//{ "CONTATTACK3"			,	SKILL_PLAY_CONTATTACK3 },
//{ "CONTATTACK4"			,	SKILL_PLAY_CONTATTACK4 },

{ ""						,	0 }
};



