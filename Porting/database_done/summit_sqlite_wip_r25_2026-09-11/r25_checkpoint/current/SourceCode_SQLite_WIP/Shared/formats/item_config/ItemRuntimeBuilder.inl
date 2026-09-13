#pragma once

#include <cstring>

namespace summit
{
namespace formats
{
namespace item_config
{

struct ItemRuntimeServices
{
	int (*randomRaw)();
	int (*randomPosNormal)(int, int);
	float (*randomFloatPos)(float, float);
	float (*randomFloatPos2)(float, float);
	int (*middlePos)(int, int);
	JOB_DATA_CODE *(*getJobDataCode)(DWORD, int);
	int (*reformItem)(sITEMINFO *);
	int *notItemSpecial;
	int *randomNormalMode;
	int perfectSpecialItemFlag;
};

struct SpecialRevision
{
	DWORD dwJobCode;
	short level[2];
	short strength[2];
	short spirit[2];
	short talent[2];
	short dexterity[2];
	short health[2];
};

static const SpecialRevision kSpecialRevision[] = {
	{ JOB_CODE_MECHANICIAN, 0, 0, 5, 15, -20, -10, 0, 0, -25, -15, 0, 0 },
	{ JOB_CODE_FIGHTER, 0, 0, 10, 15, -20, -15, 0, 0, -20, -15, 0, 0 },
	{ JOB_CODE_PIKEMAN, 0, 0, 10, 15, -20, -15, 0, 0, -25, -15, 0, 0 },
	{ JOB_CODE_ARCHER, 0, 0, -25, -15, -20, -10, 0, 0, 15, 25, 0, 0 },
	{ JOB_CODE_KNIGHT, 0, 0, 5, 15, -15, -10, 5, 10, -25, -15, 0, 0 },
	{ JOB_CODE_ATALANTA, 0, 0, -20, -15, -20, -10, 0, 0, 15, 25, 0, 0 },
	{ JOB_CODE_PRIEST, 0, 0, -25, -20, 15, 20, -15, -10, -20, -15, 0, 0 },
	{ JOB_CODE_MAGICIAN, 0, 0, -25, -20, 15, 25, -15, -10, -20, -15, 0, 0 },
};

static const int kSpecialRevisionCount = sizeof(kSpecialRevision) / sizeof(kSpecialRevision[0]);

inline int CreateDefItem(sITEMINFO *lpItem, sDEF_ITEMINFO *lpDefItem, int SpJobCode, int ItemNormam, const ItemRuntimeServices &services)
{
	int is, ie;
	int cnt;
	JOB_DATA_CODE	*lpJobDataCode = 0;

	*services.randomNormalMode = ItemNormam;




	std::memcpy(lpItem, &lpDefItem->Item, sizeof(sITEMINFO));


	if(lpDefItem->sDurability[0])
	{

		if(lpDefItem->sDurability[1])
			lpItem->Durability[1] = services.randomPosNormal(lpDefItem->sDurability[0], lpDefItem->sDurability[1]);
		else
			lpItem->Durability[1] = lpDefItem->sDurability[0];

		ie = lpItem->Durability[1];
		is = ie / 2;
		lpItem->Durability[0] = services.randomPosNormal(is, ie);
	}


	for(cnt = 0; cnt < 8; cnt++)
	{
		if(lpDefItem->sResistance[cnt][1])
		{
			lpItem->Resistance[cnt] =
				services.randomPosNormal(lpDefItem->sResistance[cnt][0], lpDefItem->sResistance[cnt][1]);
		}
		else
			lpItem->Resistance[cnt] = lpDefItem->sResistance[cnt][0];
	}


	if(lpDefItem->sDamage[1] || lpDefItem->sDamage[3])
	{
		lpItem->Damage[0] = services.randomPosNormal(lpDefItem->sDamage[0], lpDefItem->sDamage[1]);
		lpItem->Damage[1] = services.randomPosNormal(lpDefItem->sDamage[2], lpDefItem->sDamage[3]);
	}


	if(lpDefItem->sAttack_Rating[1])
		lpItem->Attack_Rating = services.randomPosNormal(lpDefItem->sAttack_Rating[0], lpDefItem->sAttack_Rating[1]);
	else
		lpItem->Attack_Rating = lpDefItem->sAttack_Rating[0];


	if(lpDefItem->fAbsorb[1] != 0)
		lpItem->fAbsorb = services.randomFloatPos2(lpDefItem->fAbsorb[0], lpDefItem->fAbsorb[1]);
	else
		lpItem->fAbsorb = lpDefItem->fAbsorb[0];


	if(lpDefItem->sDefence[1])
		lpItem->Defence = services.randomPosNormal(lpDefItem->sDefence[0], lpDefItem->sDefence[1]);
	else
		lpItem->Defence = lpDefItem->sDefence[0];


	if(lpDefItem->fBlock_Rating[1] != 0)
		lpItem->fBlock_Rating = services.randomFloatPos(lpDefItem->fBlock_Rating[0], lpDefItem->fBlock_Rating[1]);
	else
		lpItem->fBlock_Rating = lpDefItem->fBlock_Rating[0];



	if(lpDefItem->fSpeed[1] != 0)
		lpItem->fSpeed = services.randomFloatPos2(lpDefItem->fSpeed[0], lpDefItem->fSpeed[1]);
	else
		lpItem->fSpeed = lpDefItem->fSpeed[0];



	if(lpDefItem->fMana_Regen[1] != 0)
		lpItem->fMana_Regen = services.randomFloatPos2(lpDefItem->fMana_Regen[0], lpDefItem->fMana_Regen[1]);
	else
		lpItem->fMana_Regen = lpDefItem->fMana_Regen[0];


	if(lpDefItem->fLife_Regen[1] != 0)
		lpItem->fLife_Regen = services.randomFloatPos2(lpDefItem->fLife_Regen[0], lpDefItem->fLife_Regen[1]);
	else
		lpItem->fLife_Regen = lpDefItem->fLife_Regen[0];


	if(lpDefItem->fStamina_Regen[1] != 0)
		lpItem->fStamina_Regen = services.randomFloatPos2(lpDefItem->fStamina_Regen[0], lpDefItem->fStamina_Regen[1]);
	else
		lpItem->fStamina_Regen = lpDefItem->fStamina_Regen[0];



	if(lpDefItem->Increase_Life[1])
		lpItem->fIncrease_Life = (float)services.randomPosNormal(lpDefItem->Increase_Life[0], lpDefItem->Increase_Life[1]);
	else
		lpItem->fIncrease_Life = (float)lpDefItem->Increase_Life[0];


	if(lpDefItem->Increase_Mana[1])
		lpItem->fIncrease_Mana = (float)services.randomPosNormal(lpDefItem->Increase_Mana[0], lpDefItem->Increase_Mana[1]);
	else
		lpItem->fIncrease_Mana = (float)lpDefItem->Increase_Mana[0];


	if(lpDefItem->Increase_Stamina[1])
		lpItem->fIncrease_Stamina = (float)services.randomPosNormal(lpDefItem->Increase_Stamina[0], lpDefItem->Increase_Stamina[1]);
	else
		lpItem->fIncrease_Stamina = (float)lpDefItem->Increase_Stamina[0];




	DWORD	dwSpecialJob;
	int		RndPos;
	int		Revision;
	int		tempJobCode = 0;


	if(SpJobCode >= 100)
	{
		tempJobCode = SpJobCode - 100;
		SpJobCode = 0;
	}

	if(SpJobCode > 0)
	{
		lpJobDataCode = services.getJobDataCode(SpJobCode, 0);

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



		cnt = services.randomRaw() % 10;


		if(*services.notItemSpecial)
		{
			cnt = 100;
			*services.notItemSpecial = 0;
		}

		if(cnt <= 3 || SpJobCode)
		{


			cnt = services.randomRaw() % 10;

			dwSpecialJob = lpItem->JobCodeMask;

			if(cnt < 3 && lpDefItem->JobBitCodeRandomCount)
			{
				lpItem->JobCodeMask = 0;
			}

			if(!lpItem->JobCodeMask && lpDefItem->JobBitCodeRandomCount)
			{
				if(tempJobCode > 0)
				{
					lpJobDataCode = services.getJobDataCode(tempJobCode, 0);
					if(lpJobDataCode)
					{
						cnt = services.randomRaw() % lpDefItem->JobBitCodeRandomCount;
						while(lpDefItem->dwJobBitCode_Random[cnt] == lpJobDataCode->JobBitCode)
						{
							cnt = services.randomRaw() % lpDefItem->JobBitCodeRandomCount;
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
					cnt = services.randomRaw() % lpDefItem->JobBitCodeRandomCount;
					dwSpecialJob = lpDefItem->dwJobBitCode_Random[cnt];
					lpItem->JobCodeMask = dwSpecialJob;
				}
			}

			if(SpJobCode > 0)
			{

				lpJobDataCode = services.getJobDataCode(SpJobCode, 0);
				if(lpJobDataCode)
				{
					dwSpecialJob = lpJobDataCode->JobBitCode;
					lpItem->JobCodeMask = dwSpecialJob;
				}
			}


			if(lpItem->JobCodeMask) lpItem->Price += (lpItem->Price * 2) / 10;




			for(cnt = 0; cnt < kSpecialRevisionCount; cnt++)
			{
				if(kSpecialRevision[cnt].dwJobCode == dwSpecialJob)
				{

					if(lpItem->Level && kSpecialRevision[cnt].level[0])
					{
						RndPos = services.randomPosNormal(kSpecialRevision[cnt].level[0], kSpecialRevision[cnt].level[1]);
						Revision = (lpItem->Level*RndPos) / 100;
						lpItem->Level += Revision;
					}

					if(lpItem->Strength && kSpecialRevision[cnt].strength[0])
					{
						RndPos = services.randomPosNormal(kSpecialRevision[cnt].strength[0], kSpecialRevision[cnt].strength[1]);
						Revision = (lpItem->Strength*RndPos) / 100;
						lpItem->Strength += Revision;
					}

					if(lpItem->Spirit && kSpecialRevision[cnt].spirit[0])
					{
						RndPos = services.randomPosNormal(kSpecialRevision[cnt].spirit[0], kSpecialRevision[cnt].spirit[1]);
						Revision = (lpItem->Spirit*RndPos) / 100;
						lpItem->Spirit += Revision;
					}

					if(lpItem->Talent && kSpecialRevision[cnt].talent[0])
					{
						RndPos = services.randomPosNormal(kSpecialRevision[cnt].talent[0], kSpecialRevision[cnt].talent[1]);
						Revision = (lpItem->Talent*RndPos) / 100;
						lpItem->Talent += Revision;
					}

					if(lpItem->Dexterity && kSpecialRevision[cnt].dexterity[0])
					{
						RndPos = services.randomPosNormal(kSpecialRevision[cnt].dexterity[0], kSpecialRevision[cnt].dexterity[1]);
						Revision = (lpItem->Dexterity*RndPos) / 100;
						lpItem->Dexterity += Revision;
					}

					if(lpItem->Health && kSpecialRevision[cnt].health[0])
					{
						RndPos = services.randomPosNormal(kSpecialRevision[cnt].health[0], kSpecialRevision[cnt].health[1]);
						Revision = (lpItem->Health*RndPos) / 100;
						lpItem->Health += Revision;
					}
				}
			}


			if(lpDefItem->fSpecial_Absorb[1] != 0)
			{
				lpItem->JobItem.Add_fAbsorb = services.randomFloatPos(lpDefItem->fSpecial_Absorb[0], lpDefItem->fSpecial_Absorb[1]);
			}
			else
			{
				lpItem->JobItem.Add_fAbsorb = lpDefItem->fSpecial_Absorb[0];
			}


			if(lpDefItem->sSpecial_Defence[1])
			{
				lpItem->JobItem.Add_Defence = services.randomPosNormal(lpDefItem->sSpecial_Defence[0], lpDefItem->sSpecial_Defence[1]);
			}
			else
			{
				lpItem->JobItem.Add_Defence = lpDefItem->sSpecial_Defence[0];
			}


			if(lpDefItem->fSpecial_fSpeed[1] != 0)
			{
				lpItem->JobItem.Add_fSpeed = services.randomFloatPos(lpDefItem->fSpecial_fSpeed[0], lpDefItem->fSpecial_fSpeed[1]);
			}
			else
			{
				lpItem->JobItem.Add_fSpeed = lpDefItem->fSpecial_fSpeed[0];
			}


			if(lpDefItem->fSpecial_Magic_Mastery[1] != 0)
			{
				lpItem->JobItem.Add_fMagic_Mastery = services.randomFloatPos(lpDefItem->fSpecial_Magic_Mastery[0], lpDefItem->fSpecial_Magic_Mastery[1]);
			}
			else
			{
				lpItem->JobItem.Add_fMagic_Mastery = lpDefItem->fSpecial_Magic_Mastery[0];
			}


			if(lpDefItem->fSpecial_Mana_Regen[1] != 0)
			{
				lpItem->JobItem.Per_Mana_Regen = services.randomFloatPos(lpDefItem->fSpecial_Mana_Regen[0], lpDefItem->fSpecial_Mana_Regen[1]);
			}
			else
			{
				lpItem->JobItem.Per_Mana_Regen = lpDefItem->fSpecial_Mana_Regen[0];
			}



			if(lpDefItem->Lev_Attack_Rating[1])
			{
				lpItem->JobItem.Lev_Attack_Rating = services.randomPosNormal(lpDefItem->Lev_Attack_Rating[0], lpDefItem->Lev_Attack_Rating[1]);
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
		std::memset(&lpItem->JobItem, 0, sizeof(sITEM_SPECIAL));
	}





	services.reformItem(lpItem);

	return	TRUE;
}


inline int CreateDefItemPerf(sITEMINFO *lpItem, sDEF_ITEMINFO *lpDefItem, int SpJobCode, const ItemRuntimeServices &services)
{
	int cnt;
	JOB_DATA_CODE	*lpJobDataCode = 0;


	std::memcpy(lpItem, &lpDefItem->Item, sizeof(sITEMINFO));

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


	DWORD	dwSpecialJob = lpItem->JobCodeMask;
	int		RndPos;
	int		Revision;

	if (SpJobCode > 0)
	{
		lpJobDataCode = services.getJobDataCode(SpJobCode, 0);

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
		lpJobDataCode = services.getJobDataCode(SpJobCode, 0);
		if (lpJobDataCode)
		{
			dwSpecialJob = lpJobDataCode->JobBitCode;
			lpItem->JobCodeMask = dwSpecialJob;
		}

		if (lpItem->JobCodeMask) lpItem->Price += (lpItem->Price * 2) / 10;

		for (cnt = 0; cnt < kSpecialRevisionCount; cnt++)
		{
			if (kSpecialRevision[cnt].dwJobCode == dwSpecialJob)
			{
				if (lpItem->Level && kSpecialRevision[cnt].level[0])
				{
					RndPos = kSpecialRevision[cnt].level[0];
					Revision = (lpItem->Level*RndPos) / 100;
					lpItem->Level += Revision;
				}

				if (lpItem->Strength && kSpecialRevision[cnt].strength[0])
				{
					RndPos = kSpecialRevision[cnt].strength[0];
					Revision = (lpItem->Strength*RndPos) / 100;
					lpItem->Strength += Revision;
				}

				if (lpItem->Spirit && kSpecialRevision[cnt].spirit[0])
				{
					RndPos = kSpecialRevision[cnt].spirit[0];
					Revision = (lpItem->Spirit*RndPos) / 100;
					lpItem->Spirit += Revision;
				}

				if (lpItem->Talent && kSpecialRevision[cnt].talent[0])
				{
					RndPos = kSpecialRevision[cnt].talent[0];
					Revision = (lpItem->Talent*RndPos) / 100;
					lpItem->Talent += Revision;
				}

				if (lpItem->Dexterity && kSpecialRevision[cnt].dexterity[0])
				{
					RndPos = kSpecialRevision[cnt].dexterity[0];
					Revision = (lpItem->Dexterity*RndPos) / 100;
					lpItem->Dexterity += Revision;
				}

				if (lpItem->Health && kSpecialRevision[cnt].health[0])
				{
					RndPos = kSpecialRevision[cnt].health[0];
					Revision = (lpItem->Health*RndPos) / 100;
					lpItem->Health += Revision;
				}
			}
		}

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

		lpItem->SpecialItemFlag[1] = services.perfectSpecialItemFlag;
	}
	else
	{
		lpItem->JobCodeMask = 0;
	}

	if (!lpItem->JobCodeMask)
	{
		std::memset(&lpItem->JobItem, 0, sizeof(sITEM_SPECIAL));
	}

	services.reformItem(lpItem);

	return	TRUE;
}

inline int SetLowerItem(sDEF_ITEMINFO *lpDefItem, const ItemRuntimeServices &services)
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
	int		RndPos;
	int		Revision;


	if(lpItem->JobCodeMask)
	{


		dwSpecialJob = lpItem->JobCodeMask;


		for(cnt = 0; cnt < kSpecialRevisionCount; cnt++)
		{
			if(kSpecialRevision[cnt].dwJobCode == dwSpecialJob)
			{

				if(lpItem->Level && kSpecialRevision[cnt].level[0])
				{
					RndPos = services.middlePos(kSpecialRevision[cnt].level[0], kSpecialRevision[cnt].level[1]);
					Revision = (lpItem->Level*RndPos) / 100;
					lpItem->Level += Revision;
				}

				if(lpItem->Strength && kSpecialRevision[cnt].strength[0])
				{
					RndPos = services.middlePos(kSpecialRevision[cnt].strength[0], kSpecialRevision[cnt].strength[1]);
					Revision = (lpItem->Strength*RndPos) / 100;
					lpItem->Strength += Revision;
				}

				if(lpItem->Spirit && kSpecialRevision[cnt].spirit[0])
				{
					RndPos = services.middlePos(kSpecialRevision[cnt].spirit[0], kSpecialRevision[cnt].spirit[1]);
					Revision = (lpItem->Spirit*RndPos) / 100;
					lpItem->Spirit += Revision;
				}

				if(lpItem->Talent && kSpecialRevision[cnt].talent[0])
				{
					RndPos = services.middlePos(kSpecialRevision[cnt].talent[0], kSpecialRevision[cnt].talent[1]);
					Revision = (lpItem->Talent*RndPos) / 100;
					lpItem->Talent += Revision;
				}

				if(lpItem->Dexterity && kSpecialRevision[cnt].dexterity[0])
				{
					RndPos = services.middlePos(kSpecialRevision[cnt].dexterity[0], kSpecialRevision[cnt].dexterity[1]);
					Revision = (lpItem->Dexterity*RndPos) / 100;
					lpItem->Dexterity += Revision;
				}

				if(lpItem->Health && kSpecialRevision[cnt].health[0])
				{
					RndPos = services.middlePos(kSpecialRevision[cnt].health[0], kSpecialRevision[cnt].health[1]);
					Revision = (lpItem->Health*RndPos) / 100;
					lpItem->Health += Revision;
				}
			}
		}


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
		std::memset(&lpItem->JobItem, 0, sizeof(sITEM_SPECIAL));
	}


	services.reformItem(lpItem);

	return	TRUE;
}



} // namespace item_config
} // namespace formats
} // namespace summit
