int F_Extreme_Shield()
{
	int SkillIndex = 0;

	CheckMana = E_Shield_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}


	if(!CheckContinueSkillUse(pUseSkill))return FALSE;
	if(RequireStateCheck(CheckMana))
	{

		if(pUseSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[1].ItemIndex - 1].CODE&sinITEM_MASK2))
		{
			CheckCharForm();
			SwitchSkill(pUseSkill);
			SkillIndex = ContinueSkillUse(pUseSkill, E_Shield_UseTime[pUseSkill->Point - 1]);
			if(SkillIndex)
			{
				ContinueSkill[SkillIndex - 1].PlusState[0] = E_Shield_BlockRate[pUseSkill->Point - 1];

			}
			sinSetMana(sinGetMana() - CheckMana);
			sinSetStamina(sinGetStamina() - CheckStamina);
			cSkill.CheckSkillMasteryForm(pUseSkill);
			pUseSkill->Mastery = 0;
			cSkill.ReformSkillMasteryForm(pUseSkill);

			ReformCharForm();

			cInvenTory.SetItemToChar();
			pUseSkill->UseSKillIncreCount++;
			if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
			{
				pUseSkill->UseSKillIncreCount = 0;
				pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

			}
			SendProcessSKillToServer(SKILL_PLAY_EXTREME_SHIELD, pUseSkill->Point, 0, 0);
			return TRUE;
		}

	}
	return FALSE;
}

int F_Mechanic_Bomb()
{

	CheckMana = Mechanic_Bomb_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}



	if(RequireStateCheck(CheckMana))
	{
		CheckCharForm();
		sinSetMana(sinGetMana() - CheckMana);
		sinSetStamina(sinGetStamina() - CheckStamina);
		cSkill.CheckSkillMasteryForm(pUseSkill);
		pUseSkill->Mastery = 0;
		cSkill.ReformSkillMasteryForm(pUseSkill);

		ReformCharForm();
		pUseSkill->UseSKillIncreCount++;
		if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
		{
			pUseSkill->UseSKillIncreCount = 0;
			pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

		}

		return TRUE;

	}

	return FALSE;
}

int F_Physical_Absorption()
{
	int SkillIndex = 0;

	CheckMana = P_Absor_bUseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(!CheckContinueSkillUse(pUseSkill))return FALSE;

	if(RequireStateCheck(CheckMana))
	{
		SwitchSkill(pUseSkill);
		CheckCharForm();
		SkillIndex = ContinueSkillUse(pUseSkill, P_Absorb_UseTime[pUseSkill->Point - 1]);
		if(SkillIndex)
		{
			ContinueSkill[SkillIndex - 1].PlusState[0] = GetRandomPos(P_Absorb[pUseSkill->Point - 1][0], P_Absorb[pUseSkill->Point - 1][1]);


		}

		sinSetMana(sinGetMana() - CheckMana);
		sinSetStamina(sinGetStamina() - CheckStamina);
		cSkill.CheckSkillMasteryForm(pUseSkill);
		pUseSkill->Mastery = 0;
		cSkill.ReformSkillMasteryForm(pUseSkill);

		ReformCharForm();

		cInvenTory.SetItemToChar();
		SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_BLUE;
		pUseSkill->UseSKillIncreCount++;
		if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
		{
			pUseSkill->UseSKillIncreCount = 0;
			pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

		}

		return TRUE;

	}

	return FALSE;
}

int F_Poison_Attribute()
{


	cInvenTory.SetItemToChar();

	return TRUE;
}

int F_Great_Smash()
{

	CheckMana = G_Smash_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		for(int i = 0; i < 8; i++)
		{
			if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
			{
				CheckCharForm();
				sinSetMana(sinGetMana() - CheckMana);
				sinSetStamina(sinGetStamina() - CheckStamina);
				cSkill.CheckSkillMasteryForm(pUseSkill);
				pUseSkill->Mastery = 0;
				cSkill.ReformSkillMasteryForm(pUseSkill);

				ReformCharForm();
				pUseSkill->UseSKillIncreCount++;
				if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
				{
					pUseSkill->UseSKillIncreCount = 0;
					pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

				}
				return TRUE;
			}
		}
	}

	return FALSE;

}
int F_Maximize()
{
	int SkillIndex = 0;
	CheckMana = Maximize_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(!CheckContinueSkillUse(pUseSkill))return FALSE;
	if(RequireStateCheck(CheckMana))
	{
		SwitchSkill(pUseSkill);
		CheckCharForm();
		SkillIndex = ContinueSkillUse(pUseSkill, Maximize_Time[pUseSkill->Point - 1]);

		if(SkillIndex)
		{
			ContinueSkill[SkillIndex - 1].PlusState[0] = Maximize_Damage[pUseSkill->Point - 1];

		}

		sinSetMana(sinGetMana() - CheckMana);
		sinSetStamina(sinGetStamina() - CheckStamina);
		cSkill.CheckSkillMasteryForm(pUseSkill);
		pUseSkill->Mastery = 0;
		cSkill.ReformSkillMasteryForm(pUseSkill);

		ReformCharForm();

		cInvenTory.SetItemToChar();
		SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_YELLOW;
		pUseSkill->UseSKillIncreCount++;
		if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
		{
			pUseSkill->UseSKillIncreCount = 0;
			pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

		}

		return TRUE;

	}

	return FALSE;

}

int F_Automation()
{

	int SkillIndex = 0;
	CheckMana = Automation_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(!CheckContinueSkillUse(pUseSkill))return FALSE;
	if(RequireStateCheck(CheckMana))
	{
		for(int i = 0; i < 8; i++)
		{
			if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
			{

				SwitchSkill(pUseSkill);
				CheckCharForm();
				SkillIndex = ContinueSkillUse(pUseSkill, Automation_Time[pUseSkill->Point - 1]);
				if(SkillIndex)
				{
					ContinueSkill[SkillIndex - 1].PlusState[0] = Automation_Speed[pUseSkill->Point - 1];

				}
				sinSetMana(sinGetMana() - CheckMana);
				sinSetStamina(sinGetStamina() - CheckStamina);
				cSkill.CheckSkillMasteryForm(pUseSkill);
				pUseSkill->Mastery = 0;
				cSkill.ReformSkillMasteryForm(pUseSkill);

				ReformCharForm();

				cInvenTory.SetItemToChar();

				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_YELLOW;
				pUseSkill->UseSKillIncreCount++;
				if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
				{
					pUseSkill->UseSKillIncreCount = 0;
					pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

				}
				return TRUE;
			}
		}
	}
	return FALSE;
}


int F_Spark()
{
	CheckMana = Spark_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}


	if(RequireStateCheck(CheckMana))
	{
		CheckCharForm();
		sinSetMana(sinGetMana() - CheckMana);
		sinSetStamina(sinGetStamina() - CheckStamina);
		cSkill.CheckSkillMasteryForm(pUseSkill);
		pUseSkill->Mastery = 0;
		cSkill.ReformSkillMasteryForm(pUseSkill);

		ReformCharForm();
		pUseSkill->UseSKillIncreCount++;
		if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
		{
			pUseSkill->UseSKillIncreCount = 0;
			pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

		}

		return TRUE;

	}

	return FALSE;

}


int F_Melee_Mastery()
{

	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Fire_Attribute()
{


	cInvenTory.SetItemToChar();

	return TRUE;
}
int F_Raving()
{

	float CheckLife = 0;

	CheckMana = Raving_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	CheckLife = (float)((Raving_UseLife[pUseSkill->Point - 1] / 10) / sinChar->Life[1]) * 100;

	if(RequireStateCheck(CheckMana))
	{
		if((short)CheckLife <= sinGetLife())
		{
			for(int i = 0; i < 8; i++)
			{
				if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
				{
					CheckCharForm();
					sinSetMana(sinGetMana() - CheckMana);
					sinSetStamina(sinGetStamina() - CheckStamina);
					sinSetLife(sinGetLife() - (short)CheckLife);
					cSkill.CheckSkillMasteryForm(pUseSkill);
					pUseSkill->Mastery = 0;
					cSkill.ReformSkillMasteryForm(pUseSkill);
					ReformCharForm();
					pUseSkill->UseSKillIncreCount++;
					if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
					{
						pUseSkill->UseSKillIncreCount = 0;
						pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

					}

					return TRUE;

				}
			}
		}
	}

	return FALSE;
}
int F_Impact()
{

	CheckMana = Impact_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		for(int i = 0; i < 8; i++)
		{
			if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
			{
				CheckCharForm();
				sinSetMana(sinGetMana() - CheckMana);
				sinSetStamina(sinGetStamina() - CheckStamina);
				cSkill.CheckSkillMasteryForm(pUseSkill);
				pUseSkill->Mastery = 0;
				cSkill.ReformSkillMasteryForm(pUseSkill);
				ReformCharForm();
				pUseSkill->UseSKillIncreCount++;
				if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
				{
					pUseSkill->UseSKillIncreCount = 0;
					pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

				}

				return TRUE;

			}
		}
	}

	return FALSE;
}

int F_Triple_Impact()
{


	CheckMana = T_Impact_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		for(int i = 0; i < 8; i++)
		{
			if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
			{
				CheckCharForm();
				sinSetMana(sinGetMana() - CheckMana);
				sinSetStamina(sinGetStamina() - CheckStamina);
				cSkill.CheckSkillMasteryForm(pUseSkill);
				pUseSkill->Mastery = 0;
				cSkill.ReformSkillMasteryForm(pUseSkill);
				ReformCharForm();
				pUseSkill->UseSKillIncreCount++;
				if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
				{
					pUseSkill->UseSKillIncreCount = 0;
					pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

				}

				return TRUE;

			}
		}
	}

	return FALSE;
}

int F_Brutal_Swing()
{


	CheckMana = B_Swing_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		for(int i = 0; i < 8; i++)
		{
			if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
			{
				CheckCharForm();
				sinSetMana(sinGetMana() - CheckMana);
				sinSetStamina(sinGetStamina() - CheckStamina);
				cSkill.CheckSkillMasteryForm(pUseSkill);
				pUseSkill->Mastery = 0;
				cSkill.ReformSkillMasteryForm(pUseSkill);
				ReformCharForm();
				pUseSkill->UseSKillIncreCount++;
				if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
				{
					pUseSkill->UseSKillIncreCount = 0;
					pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

				}

				return TRUE;

			}
		}
	}

	return FALSE;
}

int F_Roar()
{

	CheckMana = Roar_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{


		CheckCharForm();
		sinSetMana(sinGetMana() - CheckMana);
		sinSetStamina(sinGetStamina() - CheckStamina);
		cSkill.CheckSkillMasteryForm(pUseSkill);
		pUseSkill->Mastery = 0;
		cSkill.ReformSkillMasteryForm(pUseSkill);
		ReformCharForm();
		pUseSkill->UseSKillIncreCount++;
		if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
		{
			pUseSkill->UseSKillIncreCount = 0;
			pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

		}

		return TRUE;


	}

	return FALSE;
}

int F_RageOfZecram()
{

	CheckMana = R_Zecram_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		for(int i = 0; i < 8; i++)
		{
			if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
			{
				CheckCharForm();
				sinSetMana(sinGetMana() - CheckMana);
				sinSetStamina(sinGetStamina() - CheckStamina);
				cSkill.CheckSkillMasteryForm(pUseSkill);
				pUseSkill->Mastery = 0;
				cSkill.ReformSkillMasteryForm(pUseSkill);
				ReformCharForm();
				pUseSkill->UseSKillIncreCount++;
				if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
				{
					pUseSkill->UseSKillIncreCount = 0;
					pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

				}

				return TRUE;

			}
		}
	}

	return FALSE;

}



int F_Pike_Wind()
{

	CheckMana = Pike_Wind_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		if(pUseSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
		{
			CheckCharForm();
			sinSetMana(sinGetMana() - CheckMana);
			sinSetStamina(sinGetStamina() - CheckStamina);
			cSkill.CheckSkillMasteryForm(pUseSkill);
			pUseSkill->Mastery = 0;
			cSkill.ReformSkillMasteryForm(pUseSkill);
			ReformCharForm();
			pUseSkill->UseSKillIncreCount++;
			if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
			{
				pUseSkill->UseSKillIncreCount = 0;
				pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

			}

			return TRUE;

		}
	}

	return FALSE;

}
int F_Ice_Attribute()
{


	cInvenTory.SetItemToChar();

	return TRUE;
}
int F_Critical_Hit()
{

	CheckMana = Critical_Hit_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		CheckCharForm();
		sinSetMana(sinGetMana() - CheckMana);
		sinSetStamina(sinGetStamina() - CheckStamina);
		cSkill.CheckSkillMasteryForm(pUseSkill);
		pUseSkill->Mastery = 0;
		cSkill.ReformSkillMasteryForm(pUseSkill);

		ReformCharForm();
		pUseSkill->UseSKillIncreCount++;
		if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
		{
			pUseSkill->UseSKillIncreCount = 0;
			pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

		}

		return TRUE;

	}
	return FALSE;

}
int F_Jumping_Crash()
{

	CheckMana = Jumping_Crash_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		for(int i = 0; i < 8; i++)
		{
			if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
			{
				CheckCharForm();
				sinSetMana(sinGetMana() - CheckMana);
				sinSetStamina(sinGetStamina() - CheckStamina);
				cSkill.CheckSkillMasteryForm(pUseSkill);
				pUseSkill->Mastery = 0;
				cSkill.ReformSkillMasteryForm(pUseSkill);

				ReformCharForm();
				pUseSkill->UseSKillIncreCount++;
				if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
				{
					pUseSkill->UseSKillIncreCount = 0;
					pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

				}

				return TRUE;

			}
		}
	}

	return FALSE;

}

int F_Ground_Pike()
{
	CheckMana = G_Pike_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		if(pUseSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
		{
			CheckCharForm();
			sinSetMana(sinGetMana() - CheckMana);
			sinSetStamina(sinGetStamina() - CheckStamina);
			cSkill.CheckSkillMasteryForm(pUseSkill);
			pUseSkill->Mastery = 0;
			cSkill.ReformSkillMasteryForm(pUseSkill);

			ReformCharForm();
			pUseSkill->UseSKillIncreCount++;
			if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
			{
				pUseSkill->UseSKillIncreCount = 0;
				pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

			}

			return TRUE;

		}
	}

	return FALSE;

}

int F_Tornado_Pike()
{

	CheckMana = Tornado_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		if(pUseSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
		{
			CheckCharForm();
			sinSetMana(sinGetMana() - CheckMana);
			sinSetStamina(sinGetStamina() - CheckStamina);
			cSkill.CheckSkillMasteryForm(pUseSkill);
			pUseSkill->Mastery = 0;
			cSkill.ReformSkillMasteryForm(pUseSkill);
			ReformCharForm();
			pUseSkill->UseSKillIncreCount++;
			if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
			{
				pUseSkill->UseSKillIncreCount = 0;
				pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

			}

			return TRUE;

		}
	}

	return FALSE;

}

int F_Weapone_Defence_Mastery()
{

	cInvenTory.SetItemToChar();

	return TRUE;
}

int F_Expansion()
{

	CheckMana = Expansion_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		for(int i = 0; i < 8; i++)
		{
			if(pUseSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
			{

				CheckCharForm();
				sinSetMana(sinGetMana() - CheckMana);
				sinSetStamina(sinGetStamina() - CheckStamina);
				cSkill.CheckSkillMasteryForm(pUseSkill);
				pUseSkill->Mastery = 0;
				cSkill.ReformSkillMasteryForm(pUseSkill);

				ReformCharForm();
				pUseSkill->UseSKillIncreCount++;
				if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
				{
					pUseSkill->UseSKillIncreCount = 0;
					pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

				}

				return TRUE;
			}
		}
	}

	return FALSE;

}



int F_Scout_Hawk()
{

	int SkillIndex = 0;

	CheckMana = Scout_Hawk_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}


	if(RequireStateCheck(CheckMana))
	{
		CheckCharForm();
		pUseSkill->UseTime = Scout_Hawk_UseTime[pUseSkill->Point - 1];
		sinContinueSkillSet(pUseSkill);
		SwitchSkill(pUseSkill);
		sinSetMana(sinGetMana() - CheckMana);
		sinSetStamina(sinGetStamina() - CheckStamina);
		cSkill.CheckSkillMasteryForm(pUseSkill);
		pUseSkill->Mastery = 0;
		cSkill.ReformSkillMasteryForm(pUseSkill);

		ReformCharForm();

		pUseSkill->UseSKillIncreCount++;
		if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
		{
			pUseSkill->UseSKillIncreCount = 0;
			pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

		}

		return TRUE;

	}
	return FALSE;

}
int F_Shooting_Mastery()
{

	cInvenTory.SetItemToChar();
	return TRUE;

}
int F_Wind_Arrow()
{

	CheckMana = Wind_Arrow_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		if(pUseSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
		{
			CheckCharForm();
			sinSetMana(sinGetMana() - CheckMana);
			sinSetStamina(sinGetStamina() - CheckStamina);
			cSkill.CheckSkillMasteryForm(pUseSkill);
			pUseSkill->Mastery = 0;
			cSkill.ReformSkillMasteryForm(pUseSkill);

			ReformCharForm();
			pUseSkill->UseSKillIncreCount++;
			if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
			{
				pUseSkill->UseSKillIncreCount = 0;
				pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

			}

			return TRUE;


		}
	}
	return FALSE;
}
int F_Perfect_Aim()
{

	CheckMana = Perfect_Aim_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		if(pUseSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
		{
			CheckCharForm();
			sinSetMana(sinGetMana() - CheckMana);
			sinSetStamina(sinGetStamina() - CheckStamina);
			cSkill.CheckSkillMasteryForm(pUseSkill);
			pUseSkill->Mastery = 0;
			cSkill.ReformSkillMasteryForm(pUseSkill);

			ReformCharForm();
			pUseSkill->UseSKillIncreCount++;
			if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
			{
				pUseSkill->UseSKillIncreCount = 0;
				pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

			}

			return TRUE;

		}
	}
	return FALSE;

}

int F_Dions_Eye()
{
	cInvenTory.SetItemToChar();
	return TRUE;

}

int F_Falcon()
{

	CheckMana = Falcon_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{

		CheckCharForm();
		SwitchSkill(pUseSkill);
		sinSetMana(sinGetMana() - CheckMana);
		sinSetStamina(sinGetStamina() - CheckStamina);
		cSkill.CheckSkillMasteryForm(pUseSkill);
		pUseSkill->Mastery = 0;
		cSkill.ReformSkillMasteryForm(pUseSkill);

		pUseSkill->UseTime = Falcon_Time[pUseSkill->Point - 1];
		sinContinueSkillSet(pUseSkill);

		ReformCharForm();
		pUseSkill->UseSKillIncreCount++;
		if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
		{
			pUseSkill->UseSKillIncreCount = 0;
			pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

		}
		return TRUE;


	}
	return FALSE;
}

int F_Arrow_of_Rage()
{
	CheckMana = A_Rage_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		if(pUseSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
		{
			CheckCharForm();
			sinSetMana(sinGetMana() - CheckMana);
			sinSetStamina(sinGetStamina() - CheckStamina);
			cSkill.CheckSkillMasteryForm(pUseSkill);
			pUseSkill->Mastery = 0;
			cSkill.ReformSkillMasteryForm(pUseSkill);

			ReformCharForm();
			pUseSkill->UseSKillIncreCount++;
			if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
			{
				pUseSkill->UseSKillIncreCount = 0;
				pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

			}

			return TRUE;

		}
	}
	return FALSE;

}

int F_Avalanche()
{
	CheckMana = Avalanche_UseMana[pUseSkill->Point - 1];




	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - ((CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100) + (CheckMana * 20 / 100));
	}

	else if(chaPremiumitem.m_ManaReducePotiontime > 0)
	{
		CheckMana = CheckMana - (CheckMana * chaPremiumitem.m_ManaReducePotionValue / 100);
	}

	else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		CheckMana = CheckMana - (CheckMana * 20 / 100);
	}

	if(RequireStateCheck(CheckMana))
	{
		if(pUseSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE&sinITEM_MASK2))
		{
			CheckCharForm();
			sinSetMana(sinGetMana() - CheckMana);
			sinSetStamina(sinGetStamina() - CheckStamina);
			cSkill.CheckSkillMasteryForm(pUseSkill);
			pUseSkill->Mastery = 0;
			cSkill.ReformSkillMasteryForm(pUseSkill);

			ReformCharForm();
			pUseSkill->UseSKillIncreCount++;
			if(pUseSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[pUseSkill->Skill_Info.SkillNum] + ((pUseSkill->Point - 1) / 3))
			{
				pUseSkill->UseSKillIncreCount = 0;
				pUseSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

			}

			return TRUE;

		}
	}
	return FALSE;

}



int F_Metal_Armor()
{

	pUseSkill->UseTime = Metal_Armor_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Grand_Smash()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_M_W_Mastery()
{
	return TRUE;
}
int F_Spark_Shield()
{
	pUseSkill->UseTime = Spark_Shield_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}


int F_Concentration()
{
	pUseSkill->UseTime = Concentration_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar();

	return TRUE;
}
int F_Avanging_Crash()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Swift_Axe()
{
	pUseSkill->UseTime = Swift_Axe_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar();

	return TRUE;
}
int F_Bone_Crash()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Venom_Spear()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Vanish()
{
	pUseSkill->UseTime = Vanish_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);

	return TRUE;
}
int F_Critical_Mastery()
{
	return TRUE;
}
int F_Chain_Lance()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_E_Shot()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_G_Falcon()
{
	pUseSkill->UseTime = Golden_Falcon_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_B_Shot()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Perforation()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Impulsion()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Compulsion()
{
	pUseSkill->UseTime = Compulsion_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Magnetic_Sphere()
{
	pUseSkill->UseTime = Magnetic_Sphere_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Metal_Golem()
{
	pUseSkill->UseTime = 360;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	SummonSkillCodeIndex[0] = 1;
	return TRUE;
}


int F_Destoryer()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Berserker()
{
	pUseSkill->UseTime = Berserker_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Cyclone_Strike()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Boost_Health()
{
	return TRUE;
}


int F_Assassin_Eye()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Charging_Strike()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Vague()
{
	pUseSkill->UseTime = Vague_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Shadow_Master()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Recall_Wolverin()
{
	pUseSkill->UseTime = 6 * 60;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	SummonSkillCodeIndex[2] = 1;
	return TRUE;
}
int F_Evasion_Mastery()
{
	return TRUE;
}
int F_Phoenix_Shot()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Force_Of_Nature()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


// Summit Tier 5 bugfix stack 091-158

int F_Rupture()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Parasit_Shot()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Hardening_Shield()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	pUseSkill->UseTime = Hardening_Shield_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Magnetic_Discharge()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Survival_Instinct()
{
	// Passive T5: no execution packet or resource consumption.
	return TRUE;
}

int F_Seismic_Impact()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;

	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Ruthless_Of_Zecram()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;

	// Recast toggles the stance off without charging mana again.
	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag && ContinueSkill[i].CODE == SKILL_T5_RUTHLESS_OF_ZECRAM)
		{
			memset(&ContinueSkill[i], 0, sizeof(sSKILL));
			cInvenTory.SetItemToChar();
			SendCancelSkillToServer(SKILL_PLAY_T53, pUseSkill->Point, 0, 0);
			return FALSE;
		}
	}

	// Persistent stance. CheckContinueSkill keeps this entry alive until death/village/cancel.
	pUseSkill->UseTime = 3600;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Whirlwind()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10)
		return FALSE;

	int point = pUseSkill->Point - 1;
	if(Fighter_T5_Whirlwind_Duration[point] <= 0 || Fighter_T5_Whirlwind_MPDrain[point] <= 0 ||
	   Fighter_T5_Whirlwind_SPDrain[point] <= 0)
		return FALSE;

	// The EU metadata carries an initial stamina requirement (93 + 2/level).
	// Continuous MP/SP consumption is handled separately by the Whirlwind channel ticks.
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Ring_Of_Spears()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Pike_T5_Focus()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	// EU exposes no duration table for T52; keep it as a bounded Summit continue buff.
	pUseSkill->UseTime = 300;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Pike_T5_Death_Master()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	// EU exposes no duration table for T53; use the same bounded test duration as T52.
	pUseSkill->UseTime = 300;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Pike_T5_Twister()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Archer_T5_Lethal_Sight()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	pUseSkill->UseTime = 300;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Archer_T5_Fierce_Wind()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Archer_T5_Entaling_Roots()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Archer_T5_Bombardment()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana)
		return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
