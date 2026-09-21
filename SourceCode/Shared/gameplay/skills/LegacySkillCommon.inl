// Shared byte-equivalent cSKILL methods extracted from client/server legacy implementations.
// Target-specific skill behavior remains in each sinSkill.cpp.

#include <map>

void cSKILL::Release()
{
	int i = 0;
	if(lpSkillGage[0])
	{
		lpSkillGage[0]->Release();
		lpSkillGage[0] = 0;
	}
	if(lpSkillGage[1])
	{
		lpSkillGage[1]->Release();
		lpSkillGage[1] = 0;


	}
	if(lpSelectSkill)
	{
		lpSelectSkill->Release();
		lpSelectSkill = 0;

	}
	for(i = 0; i < 4; i++)
	{
		if(lpSkillTitle[i])
		{
			lpSkillTitle[i]->Release();
			lpSkillTitle[i] = 0;

		}
	}
	for(i = 0; i < 4; i++)
	{
		if(lpSkillTitle[i])
		{
			lpSkillTitle[i]->Release();
			lpSkillTitle[i] = 0;

		}
	}
	for(i = 0; i < SIN_MAX_USE_SKILL; i++)
	{
		if(lpSkillButton[i])
		{
			lpSkillButton[i]->Release();
			lpSkillButton[i] = 0;


		}
		if(lpSkillButton_Gray[i])
		{
			lpSkillButton_Gray[i]->Release();
			lpSkillButton_Gray[i] = 0;

		}
	}
	for(i = 0; i < 8; i++)
	{
		if(lpShortKey[i])
		{
			lpShortKey[i]->Release();
			lpShortKey[i] = 0;

		}
	}
	for(i = 0; i < 10; i++)
	{
		if(lpWeaponIcon[i])
		{
			lpWeaponIcon[i]->Release();
			lpWeaponIcon[i] = 0;
		}
	}

}

void cSKILL::Draw()
{
	int i = 0;














	if(SkillMain2OpenFlag)
	{
		DrawSprite(0, 600 - sinMoveKindInter[SIN_SKILL] + 56, lpSkillMain2, 0, 0, 800, 200);

		if(LegacySinSkillPolicy::ShowTier5BackButton(SkillMain2ButtonFlag[0]))
		{
			DrawSprite(13, 600 + 83 - sinMoveKindInter[SIN_SKILL] + 56, lpSkillMainLeftB, 0, 0, 20, 20);
		}
	}

	if(!SkillMain2OpenFlag)
	{
		DrawSprite(0, 600 - sinMoveKindInter[SIN_SKILL] + 56, lpSkillMain, 0, 0, 800, 200);

		if(LegacySinSkillPolicy::ShowTier5ForwardButton(SkillMain2ButtonFlag[1]))
		{
			DrawSprite(766, 600 + 83 - sinMoveKindInter[SIN_SKILL] + 56, lpSkillMainRightB, 0, 0, 20, 20);
		}
	}

	if(OpenFlag)
	{
		if(!SkillMain2OpenFlag)
		{
			for(i = 0; i < (int)sinChar->ChangeJob + 1 && i < 4; i++)
			{
				DrawSprite(SkillTitle[i][0], SkillTitle[i][1] + (256 - sinMoveKindInter[SIN_SKILL]), lpSkillTitle[i], 0, 0, 100, 20);
			}
		}

		int SkillPageStart = SkillMain2OpenFlag ? SIN_TIER5_SKILL_START : 1;
		int SkillPageEnd = SkillMain2OpenFlag ? ChangeJobSkillPlus :
			((ChangeJobSkillPlus < SIN_TIER5_SKILL_START) ? ChangeJobSkillPlus : SIN_TIER5_SKILL_START);

		for(i = SkillPageStart; i < SkillPageEnd; i++)
		{
			if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Use && SkillMasterFlag)
			{
				if(lpSkillButton[i])
				{
					DrawSprite(sinSkill.SkillBox[i].BoxRect.left, sinSkill.SkillBox[i].BoxRect.top + (256 - sinMoveKindInter[SIN_SKILL]),
							   lpSkillButton[i], 0, 0, 49, 46);
				}
			}
			if(sinSkill.UseSkill[i].Flag && !sinSkill.UseSkill[i].Use  && SkillMasterFlag)
			{
				if(lpSkillButton_Gray[i])
				{
					DrawSprite(sinSkill.SkillBox[i].BoxRect.left, sinSkill.SkillBox[i].BoxRect.top + (256 - sinMoveKindInter[SIN_SKILL]),
							   lpSkillButton_Gray[i], 0, 0, 49, 46);
				}
			}
			if(sinSkill.UseSkill[i].Flag &&  SkillMasterFlag && sinSkill.UseSkill[i].Point >= 10)
			{
				if(lpSkillButton_Gray[i])
				{
					DrawSprite(sinSkill.SkillBox[i].BoxRect.left, sinSkill.SkillBox[i].BoxRect.top + (256 - sinMoveKindInter[SIN_SKILL]),
							   lpSkillButton_Gray[i], 0, 0, 49, 46);
				}
			}
			if(!SkillMasterFlag)
			{
				if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Point)
				{
					if(lpSkillButton[i])
					{
						DrawSprite(sinSkill.SkillBox[i].BoxRect.left, sinSkill.SkillBox[i].BoxRect.top + (256 - sinMoveKindInter[SIN_SKILL]),
								   lpSkillButton[i], 0, 0, 49, 46);

					}
				}
				if(sinSkill.UseSkill[i].Flag && !sinSkill.UseSkill[i].Point)
				{
					if(lpSkillButton_Gray[i])
					{
						DrawSprite(sinSkill.SkillBox[i].BoxRect.left, sinSkill.SkillBox[i].BoxRect.top + (256 - sinMoveKindInter[SIN_SKILL]),
								   lpSkillButton_Gray[i], 0, 0, 49, 46);
					}
				}
			}

			if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Skill_Info.USECODE == SIN_SKILL_USE_NOT)
			{
				DrawSprite(sinSkill.SkillBox[i].GageRect.left, sinSkill.SkillBox[i].GageRect.top + (256 - sinMoveKindInter[SIN_SKILL]), lpSkillGage4, 0, 0, 5, 35);
			}

			if(sinSkill.UseSkill[i].Flag && (sinSkill.UseSkill[i].Skill_Info.USECODE != SIN_SKILL_USE_NOT || sinSkill.UseSkill[i].Skill_Info.Element[0]))
			{
				DrawSprite(sinSkill.SkillBox[i].GageRect.left, sinSkill.SkillBox[i].GageRect.top + (256 - sinMoveKindInter[SIN_SKILL]), lpSkillGage3, 0, 0, 5, 35);
				DrawSprite(sinSkill.SkillBox[i].GageRect.left, sinSkill.SkillBox[i].GageRect.top + (35 - sinSkill.UseSkill[i].UseSkillMasteryGage) + (256 - sinMoveKindInter[SIN_SKILL]), lpSkillGage2[0], 0, 35 - sinSkill.UseSkill[i].UseSkillMasteryGage, 5, 35);
				if(sinSkill.UseSkill[i].UseSkillMasteryGage >= 3)
					DrawSprite(sinSkill.SkillBox[i].GageRect.left, sinSkill.SkillBox[i].GageRect.top + (35 - sinSkill.UseSkill[i].UseSkillMasteryGage) + (256 - sinMoveKindInter[SIN_SKILL]), lpSkillGage2[1], 0, 0, 5, 3);

			}
			if(sinSkill.UseSkill[i].Flag)
			{
				DrawSprite(sinSkill.SkillBox[i].GageRect.right + 1, sinSkill.SkillBox[i].GageRect.top + (35 - sinSkill.UseSkill[i].GageLength) + (256 - sinMoveKindInter[SIN_SKILL]), lpSkillGage[0], 0, 35 - sinSkill.UseSkill[i].GageLength, 5, 35);
				if(sinSkill.UseSkill[i].GageLength >= 3)
					DrawSprite(sinSkill.SkillBox[i].GageRect.right + 1, sinSkill.SkillBox[i].GageRect.top + (35 - sinSkill.UseSkill[i].GageLength) + (256 - sinMoveKindInter[SIN_SKILL]), lpSkillGage[0], 0, 0, 5, 3);

			}

			if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Skill_Info.Element[0])
			{
				DrawSprite(sinSkill.SkillBox[i].GageRect.left, sinSkill.SkillBox[i].GageRect.top + (256 - sinMoveKindInter[SIN_SKILL]), lpSkillGage5, 0, 0, 5, 35);
			}


		}
		if(DownButtonIndex)
		{
			DrawSprite(sinSkill.SkillBox[DownButtonIndex - 1].BoxRect.left + 1, sinSkill.SkillBox[DownButtonIndex - 1].BoxRect.top + 1 + (256 - sinMoveKindInter[SIN_SKILL]),
					   lpSkillButton[DownButtonIndex - 1], 0, 0, 49, 46);

		}
		if(SkillButtonIndex)
		{
			if(sinSkill.UseSkill[SkillButtonIndex - 1].Flag)
			{
				if(DownButtonIndex)
				{
					DrawSprite(sinSkill.SkillBox[SkillButtonIndex - 1].BoxRect.left + 1,
							   sinSkill.SkillBox[SkillButtonIndex - 1].BoxRect.top + 1 + (256 - sinMoveKindInter[SIN_SKILL]),
							   lpSelectSkill, 0, 0, 49, 46);

				}
				else
				{
					DrawSprite(sinSkill.SkillBox[SkillButtonIndex - 1].BoxRect.left,
							   sinSkill.SkillBox[SkillButtonIndex - 1].BoxRect.top + (256 - sinMoveKindInter[SIN_SKILL]),
							   lpSelectSkill, 0, 0, 49, 46);
				}
			}

		}


		int SkillShortcutStart = SkillMain2OpenFlag ? SIN_TIER5_SKILL_START : 0;
		for(i = SkillShortcutStart; i < SkillPageEnd; i++)
		{
			if(sinSkill.UseSkill[i].Flag)
			{
				if(sinSkill.UseSkill[i].ShortKey)
				{
					DrawSprite(sinSkill.SkillBox[i].BoxRect.left + 9, sinSkill.SkillBox[i].BoxRect.top - 6 + (256 - sinMoveKindInter[SIN_SKILL]),
							   lpShortKey[sinSkill.UseSkill[i].ShortKey - 1], 0, 0, 16, 16);

				}
			}
		}
	}

	if(SkillMasteryShowFlag)
	{
		DrawSprite(UseSkillMasteryPosi.x, UseSkillMasteryPosi.y + (256 - sinMoveKindInter[SIN_SKILL]), lpMasteryInfo, 0, 0, 39, 27);
	}


	if(sinSkill.pLeftSkill)
	{
		if(sinSkill.pLeftSkill->UseSkillFlag)
			DrawSprite(sLeftRightSkill[0].BoxRect.left, sLeftRightSkill[0].BoxRect.top,
					   lpSkillButton[sinSkill.pLeftSkill->Position - 1], 0, 0, 49, 46);
		else
			DrawSprite(sLeftRightSkill[0].BoxRect.left, sLeftRightSkill[0].BoxRect.top,
					   lpSkillButton_Gray[sinSkill.pLeftSkill->Position - 1], 0, 0, 49, 46);


	}
	else
	{
		DrawSprite(sLeftRightSkill[0].BoxRect.left, sLeftRightSkill[0].BoxRect.top,
				   lpSkillBoxBack, 0, 0, 49, 46);

	}
	if(sinSkill.pRightSkill)
	{
		if(sinSkill.pRightSkill->UseSkillFlag)
			DrawSprite(sLeftRightSkill[1].BoxRect.left, sLeftRightSkill[1].BoxRect.top,
					   lpSkillButton[sinSkill.pRightSkill->Position - 1], 0, 0, 49, 46);
		else
			DrawSprite(sLeftRightSkill[1].BoxRect.left, sLeftRightSkill[1].BoxRect.top,
					   lpSkillButton_Gray[sinSkill.pRightSkill->Position - 1], 0, 0, 49, 46);


	}
	else
	{
		DrawSprite(sLeftRightSkill[1].BoxRect.left, sLeftRightSkill[1].BoxRect.top,
				   lpSkillBoxBack, 0, 0, 49, 46);

	}


}

void cSKILL::RButtonDown(int x, int y)
{
	for(int i = 0; i < 2; i++)
	{
		if(sLeftRightSkill[i].BoxRect.left < pCursorPos.x && sLeftRightSkill[i].BoxRect.right > pCursorPos.x &&
		   sLeftRightSkill[i].BoxRect.top < pCursorPos.y && sLeftRightSkill[i].BoxRect.bottom > pCursorPos.y)
		{
			if(i == 0 && sinSkill.pLeftSkill)
			{
				cHelpPet.PetMessage("*LeftSkill", 2);
			}
			if(i == 1 && sinSkill.pRightSkill)
			{
				cHelpPet.PetMessage("*RightSkill", 2);
			}

		}
	}

	DownButtonUseShorKey = 0;
	if(SkillButtonIndex)
	{
		if(sinSkill.UseSkill[SkillButtonIndex - 1].Flag)
		{
			RDownButtonIndex = SkillButtonIndex;
		}
	}
}

void cSKILL::RButtonUp(int x, int y)
{
	RDownButtonIndex = 0;
	LDownButtonIndex = 0;
	if(SkillButtonIndex && !DownButtonUseShorKey)
	{
		if((sinSkill.UseSkill[SkillButtonIndex - 1].Skill_Info.USECODE == SIN_SKILL_USE_RIGHT ||
			sinSkill.UseSkill[SkillButtonIndex - 1].Skill_Info.USECODE == SIN_SKILL_USE_ALL) && !SkillMasterFlag &&
		   sinSkill.UseSkill[SkillButtonIndex - 1].Point)
			sinSkill.pRightSkill = &sinSkill.UseSkill[SkillButtonIndex - 1];
	}

}

void cSKILL::KeyDown()
{



	int i = 0;
	char szKeyBuff[8] = { VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6,VK_F7,VK_F8 };
	for(i = 0; i < 8; i++)
	{
		if(sinGetKeyClick(szKeyBuff[i]))
		{
			if(SkillButtonIndex)
			{

				if(LDownButtonIndex)
				{
					if(sinSkill.UseSkill[SkillButtonIndex - 1].Skill_Info.USECODE == SIN_SKILL_USE_LEFT ||
					   sinSkill.UseSkill[SkillButtonIndex - 1].Skill_Info.USECODE == SIN_SKILL_USE_ALL &&
					   sinSkill.UseSkill[SkillButtonIndex - 1].Point && !SkillMasterFlag)
					{
						sinSkill.UseSkill[SkillButtonIndex - 1].ShortKey = i + 1;
						sinSkill.UseSkill[SkillButtonIndex - 1].MousePosi = SIN_MOUSE_POSI_LEFT;
						DownButtonUseShorKey = 1;
					}
				}
				else
				{
					if(RDownButtonIndex)
					{
						if(sinSkill.UseSkill[SkillButtonIndex - 1].Skill_Info.USECODE == SIN_SKILL_USE_RIGHT ||
						   sinSkill.UseSkill[SkillButtonIndex - 1].Skill_Info.USECODE == SIN_SKILL_USE_ALL &&
						   sinSkill.UseSkill[SkillButtonIndex - 1].Point && !SkillMasterFlag)
						{
							sinSkill.UseSkill[SkillButtonIndex - 1].ShortKey = i + 1;
							sinSkill.UseSkill[SkillButtonIndex - 1].MousePosi = SIN_MOUSE_POSI_RIGHT;
							DownButtonUseShorKey = 1;
						}
					}

				}

				for(int j = 0; j < SIN_MAX_USE_SKILL; j++)
				{
					if(j == SkillButtonIndex - 1)continue;
					if(sinSkill.UseSkill[SkillButtonIndex - 1].ShortKey == sinSkill.UseSkill[j].ShortKey)
					{
						sinSkill.UseSkill[j].ShortKey = 0;
					}
				}
			}
			else
			{
				for(int j = 0; j < SIN_MAX_USE_SKILL; j++)
				{
					if(i == sinSkill.UseSkill[j].ShortKey - 1)
					{
						if(sinSkill.UseSkill[j].Point)
						{
							if(sinSkill.UseSkill[j].MousePosi == SIN_MOUSE_POSI_LEFT)
							{
								sinSkill.pLeftSkill = &sinSkill.UseSkill[j];

							}
							if(sinSkill.UseSkill[j].MousePosi == SIN_MOUSE_POSI_RIGHT)
							{
								sinSkill.pRightSkill = &sinSkill.UseSkill[j];
							}
						}
					}
				}
			}
		}
	}
	if(sinGetKeyClick('S'))
	{
		cHelpPet.PetMessage("*Skill", 3);
		if(cSkill.OpenFlag)cSkill.OpenFlag = SIN_CLOSE;
		else cSkill.OpenFlag = SIN_OPEN;
		cInterFace.CheckAllBox(SIN_SKILL);
		if(cSkill.OpenFlag)
		{
			if(!sinFireShow)
			{
				StartMenuFlame(0, 350);
				sinPlaySound(0);
			}
			else
				sinPlaySound(SIN_SOUND_SHOW_INTER);
			sinFireShow = 1;
		}
	}

}

// Summit Tier 5 metadata imported from bugfix stack 091-158.
static void InitSummitTier5MechanicianSkillMetadata()
{
	if(sinChar->JOB_CODE != JOBCODE_MECHANICIAN) return;

	sSKILL *skill = &sinSkill.UseSkill[17];
	if(skill->CODE != SKILL_T5_RUPTURE)
		ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Rupture");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TM100 Rupture" : "TM43 Grand_Smash");
	skill->CODE = SKILL_T5_RUPTURE;
	skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 100;
	skill->Skill_Info.UseStamina[0] = 71;
	skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 180;
	skill->Skill_Info.RequireMastery[1] = 4;
	skill->Skill_Info.Element[0] = 2;
	skill->Skill_Info.Element[1] = 0;
	skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWA1;
	skill->Skill_Info.UseWeaponCode[1] = sinWC1;
	skill->Skill_Info.UseWeaponCode[2] = sinWH1;
	skill->Skill_Info.UseWeaponCode[3] = sinWP1;
	skill->Skill_Info.UseWeaponCode[4] = sinWS2;
	for(int i = 5; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Rupture");
	strcpy_s(skill->Skill_Info.SkillDoc, "Strikes the enemy twice with a powerful mechanical blow.");
	skill->Skill_Info.FuncPointer = F_Rupture;
	skill->Skill_Info.CODE = SKILL_T5_RUPTURE;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_ALL;
	skill->Skill_Info.UseMana = Rupture_UseMana;
	skill->Skill_Info.SkillNum = 16;

	skill = &sinSkill.UseSkill[18];
	if(skill->CODE != SKILL_T5_PARASIT_SHOT)
		ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Swarm Shot");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TM103 Parasit_Shot" : "TM60 Impulsion");
	skill->CODE = SKILL_T5_PARASIT_SHOT;
	skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 103;
	skill->Skill_Info.UseStamina[0] = 65;
	skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 93;
	skill->Skill_Info.RequireMastery[1] = 3;
	skill->Skill_Info.Element[0] = 2;
	skill->Skill_Info.Element[1] = 0;
	skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWA1;
	skill->Skill_Info.UseWeaponCode[1] = sinWC1;
	skill->Skill_Info.UseWeaponCode[2] = sinWH1;
	skill->Skill_Info.UseWeaponCode[3] = sinWP1;
	skill->Skill_Info.UseWeaponCode[4] = sinWS2;
	for(int i = 5; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Swarm Shot");
	strcpy_s(skill->Skill_Info.SkillDoc, "Launches a parasite-filled shot that poisons the target.");
	skill->Skill_Info.FuncPointer = F_Parasit_Shot;
	skill->Skill_Info.CODE = SKILL_T5_PARASIT_SHOT;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT;
	skill->Skill_Info.UseMana = Parasit_Shot_UseMana;
	skill->Skill_Info.SkillNum = 17;

	skill = &sinSkill.UseSkill[19];
	if(skill->CODE != SKILL_T5_HARDENING_SHIELD)
		ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Hardened Ward");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TM106 Hardening_Shield" : "TM50 S_Shield");
	skill->CODE = SKILL_T5_HARDENING_SHIELD;
	skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 103;
	skill->Skill_Info.UseStamina[0] = 90;
	skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 203;
	skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2;
	skill->Skill_Info.Element[1] = 0;
	skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWA1;
	skill->Skill_Info.UseWeaponCode[1] = sinWC1;
	skill->Skill_Info.UseWeaponCode[2] = sinWH1;
	skill->Skill_Info.UseWeaponCode[3] = sinWP1;
	skill->Skill_Info.UseWeaponCode[4] = sinWS2;
	skill->Skill_Info.UseWeaponCode[5] = sinWS1;
	skill->Skill_Info.UseWeaponCode[6] = sinWT1;
	skill->Skill_Info.UseWeaponCode[7] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Hardened Ward");
	strcpy_s(skill->Skill_Info.SkillDoc, "Reduces incoming damage while lowering your attack rating.");
	skill->Skill_Info.FuncPointer = F_Hardening_Shield;
	skill->Skill_Info.CODE = SKILL_T5_HARDENING_SHIELD;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT;
	skill->Skill_Info.UseMana = Hardening_Shield_UseMana;
	skill->Skill_Info.SkillNum = 18;

	skill = &sinSkill.UseSkill[20];
	if(skill->CODE != SKILL_T5_MAGNETIC_DISCHARGE)
		ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Magnetic Discharge");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TM110 Magnetic_Discharge" : "TM66 M_Sphere");
	skill->CODE = SKILL_T5_MAGNETIC_DISCHARGE;
	skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 110;
	skill->Skill_Info.UseStamina[0] = 95;
	skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 110;
	skill->Skill_Info.RequireMastery[1] = 5;
	skill->Skill_Info.Element[0] = 2;
	skill->Skill_Info.Element[1] = 0;
	skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWA1;
	skill->Skill_Info.UseWeaponCode[1] = sinWC1;
	skill->Skill_Info.UseWeaponCode[2] = sinWH1;
	skill->Skill_Info.UseWeaponCode[3] = sinWP1;
	skill->Skill_Info.UseWeaponCode[4] = sinWS2;
	skill->Skill_Info.UseWeaponCode[5] = sinWS1;
	skill->Skill_Info.UseWeaponCode[6] = sinWT1;
	skill->Skill_Info.UseWeaponCode[7] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Magnetic Discharge");
	strcpy_s(skill->Skill_Info.SkillDoc, "Releases magnetic spheres that damage nearby enemies.");
	skill->Skill_Info.FuncPointer = F_Magnetic_Discharge;
	skill->Skill_Info.CODE = SKILL_T5_MAGNETIC_DISCHARGE;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT;
	skill->Skill_Info.UseMana = Magnetic_Discharge_UseMana;
	skill->Skill_Info.SkillNum = 19;
}

static void InitSummitTier5FighterSkillMetadata()
{
	if(sinChar->JOB_CODE != JOBCODE_FIGHTER) return;

	sSKILL *skill = &sinSkill.UseSkill[17];
	if(skill->CODE != SKILL_T5_SURVIVAL_INSTINCT)
		ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Survival Instinct");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TF100 Survival_Instinct" : "TF70 B_Health");
	skill->CODE = SKILL_T5_SURVIVAL_INSTINCT;
	skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 103;
	strcpy_s(skill->Skill_Info.SkillName, "Survival Instinct");
	strcpy_s(skill->Skill_Info.SkillDoc, "Permanently raises maximum health and life regeneration.");
	skill->Skill_Info.FuncPointer = F_Survival_Instinct;
	skill->Skill_Info.CODE = SKILL_T5_SURVIVAL_INSTINCT;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_NOT;
	skill->Skill_Info.UseMana = 0;
	skill->Skill_Info.SkillNum = 16;

	skill = &sinSkill.UseSkill[18];
	if(skill->CODE != SKILL_T5_SEISMIC_IMPACT)
		ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Seismic Impact");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TF103 Sesmic_Impact" : "TF66 C_Strike");
	skill->CODE = SKILL_T5_SEISMIC_IMPACT;
	skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 100;
	skill->Skill_Info.UseStamina[0] = 68;
	skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 180;
	skill->Skill_Info.RequireMastery[1] = 4;
	skill->Skill_Info.Element[0] = 2;
	skill->Skill_Info.Element[1] = 0;
	skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWA1;
	skill->Skill_Info.UseWeaponCode[1] = sinWH1;
	skill->Skill_Info.UseWeaponCode[2] = sinWP1;
	skill->Skill_Info.UseWeaponCode[3] = sinWS2;
	skill->Skill_Info.UseWeaponCode[4] = 0;
	skill->Skill_Info.UseWeaponCode[5] = 0;
	skill->Skill_Info.UseWeaponCode[6] = 0;
	skill->Skill_Info.UseWeaponCode[7] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Seismic Impact");
	strcpy_s(skill->Skill_Info.SkillDoc, "Strikes the target twice, then releases a final area impact.");
	skill->Skill_Info.FuncPointer = F_Seismic_Impact;
	skill->Skill_Info.CODE = SKILL_T5_SEISMIC_IMPACT;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT;
	skill->Skill_Info.UseMana = Fighter_T5_Seismic_UseMana;
	skill->Skill_Info.SkillNum = 17;

	skill = &sinSkill.UseSkill[19];
	if(skill->CODE != SKILL_T5_RUTHLESS_OF_ZECRAM)
		ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Might of Zecram");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TF106 Ruthless_Zecram" : "TF30 R_Zecram");
	skill->CODE = SKILL_T5_RUTHLESS_OF_ZECRAM;
	skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 106;
	skill->Skill_Info.UseStamina[0] = 70;
	skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 203;
	skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2;
	skill->Skill_Info.Element[1] = 0;
	skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWA1;
	skill->Skill_Info.UseWeaponCode[1] = sinWH1;
	for(int i = 2; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Might of Zecram");
	strcpy_s(skill->Skill_Info.SkillDoc, "Fire stance: loses HP on each hit and periodically empowers an attack.");
	skill->Skill_Info.FuncPointer = F_Ruthless_Of_Zecram;
	skill->Skill_Info.CODE = SKILL_T5_RUTHLESS_OF_ZECRAM;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT;
	skill->Skill_Info.UseMana = Fighter_T5_Ruthless_UseMana;
	skill->Skill_Info.SkillNum = 18;
	skill = &sinSkill.UseSkill[20];
	if(skill->CODE != SKILL_T5_WHIRLWIND)
		ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Raging Strikes");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TF110 Whirlwind" : "TF60 C_Stryke");
	skill->CODE = SKILL_T5_WHIRLWIND;
	skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 110;
	skill->Skill_Info.UseStamina[0] = 93;
	skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 200;
	skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2;
	skill->Skill_Info.Element[1] = 0;
	skill->Skill_Info.Element[2] = 0;
	// The reference metadata omits a weapon restriction, but its description explicitly requires an axe.
	skill->Skill_Info.UseWeaponCode[0] = sinWA1;
	for(int i = 1; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Raging Strikes");
	strcpy_s(skill->Skill_Info.SkillDoc, "Channel a furious axe spin that damages nearby enemies each second.");
	skill->Skill_Info.FuncPointer = F_Whirlwind;
	skill->Skill_Info.CODE = SKILL_T5_WHIRLWIND;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT;
	// MP is drained per channel tick; this zero table only keeps the legacy activation path null-safe.
	skill->Skill_Info.UseMana = Fighter_T5_Whirlwind_ActivationMana;
	skill->Skill_Info.SkillNum = 19;

}

static void InitSummitTier5PikemanSkillMetadata()
{
	if(sinChar->JOB_CODE != JOBCODE_PIKEMAN) return;

	sSKILL *skill = &sinSkill.UseSkill[17];
	if(skill->CODE != SKILL_T5_RING_OF_SPEARS) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Ring of Spears");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TP100 Ring_Spears" : "TP20 G_Pike");
	skill->CODE = SKILL_T5_RING_OF_SPEARS; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 100;
	skill->Skill_Info.UseStamina[0] = 75; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 136; skill->Skill_Info.RequireMastery[1] = 4;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWC1; skill->Skill_Info.UseWeaponCode[1] = sinWP1; skill->Skill_Info.UseWeaponCode[2] = sinWS2;
	for(int i = 3; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Ring of Spears");
	strcpy_s(skill->Skill_Info.SkillDoc, "Surrounds the target with spears, stunning it with fear.");
	skill->Skill_Info.FuncPointer = F_Ring_Of_Spears; skill->Skill_Info.CODE = SKILL_T5_RING_OF_SPEARS;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Pike_T5_Ring_UseMana; skill->Skill_Info.SkillNum = 16;

	skill = &sinSkill.UseSkill[18];
	if(skill->CODE != SKILL_T5_FOCUS) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Death Mantra");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TP103 Focus" : "TP66 Vague");
	skill->CODE = SKILL_T5_FOCUS; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 103;
	skill->Skill_Info.UseStamina[0] = 70; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 203; skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWP1; for(int i = 1; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Death Mantra");
	strcpy_s(skill->Skill_Info.SkillDoc, "Raises maximum attack power by sacrificing minimum attack power.");
	skill->Skill_Info.FuncPointer = F_Pike_T5_Focus; skill->Skill_Info.CODE = SKILL_T5_FOCUS;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Pike_T5_Focus_UseMana; skill->Skill_Info.SkillNum = 17;

	skill = &sinSkill.UseSkill[19];
	if(skill->CODE != SKILL_T5_DEATH_MASTER) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Ex-Death");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TP106 Death_Master" : "TP70 S_Master");
	skill->CODE = SKILL_T5_DEATH_MASTER; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 106;
	skill->Skill_Info.UseStamina[0] = 93; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 180; skill->Skill_Info.RequireMastery[1] = 4;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWP1; skill->Skill_Info.UseWeaponCode[1] = sinWS2; for(int i = 2; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Ex-Death");
	strcpy_s(skill->Skill_Info.SkillDoc, "Envelops the weapon in shadow, increasing damage and attack rating.");
	skill->Skill_Info.FuncPointer = F_Pike_T5_Death_Master; skill->Skill_Info.CODE = SKILL_T5_DEATH_MASTER;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Pike_T5_Death_UseMana; skill->Skill_Info.SkillNum = 18;

	skill = &sinSkill.UseSkill[20];
	if(skill->CODE != SKILL_T5_TWISTER) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Sinister Spin");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TP110 Twister" : "TP23 Tornado");
	skill->CODE = SKILL_T5_TWISTER; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 110;
	skill->Skill_Info.UseStamina[0] = 81; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 100; skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWP1; skill->Skill_Info.UseWeaponCode[1] = sinWS2; for(int i = 2; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Sinister Spin");
	strcpy_s(skill->Skill_Info.SkillDoc, "Spins vigorously, creating a deadly twister around the Pikeman.");
	skill->Skill_Info.FuncPointer = F_Pike_T5_Twister; skill->Skill_Info.CODE = SKILL_T5_TWISTER;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Pike_T5_Twister_UseMana; skill->Skill_Info.SkillNum = 19;
}

static void InitSummitTier5ArcherSkillMetadata()
{
	if(sinChar->JOB_CODE != JOBCODE_ARCHER) return;

	sSKILL *skill = &sinSkill.UseSkill[17];
	if(skill->CODE != SKILL_T5_LETHAL_SIGHT) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Lethal Sight");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TA100 Lethal_Sight" : "TA17 P_aim");
	skill->CODE = SKILL_T5_LETHAL_SIGHT; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 100;
	skill->Skill_Info.UseStamina[0] = 52; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 203; skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWS1; for(int i = 1; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Lethal Sight");
	strcpy_s(skill->Skill_Info.SkillDoc, "Focuses on a distant target so the next attack becomes fatal.");
	skill->Skill_Info.FuncPointer = F_Archer_T5_Lethal_Sight; skill->Skill_Info.CODE = SKILL_T5_LETHAL_SIGHT;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Archer_T5_Lethal_UseMana; skill->Skill_Info.SkillNum = 16;

	skill = &sinSkill.UseSkill[18];
	if(skill->CODE != SKILL_T5_FIERCE_WIND) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Fierce Wind");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TA103 Fierce_Wind" : "TA66 P_Shot");
	skill->CODE = SKILL_T5_FIERCE_WIND; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 103;
	skill->Skill_Info.UseStamina[0] = 52; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 82; skill->Skill_Info.RequireMastery[1] = 2;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWS1; for(int i = 1; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Fierce Wind");
	strcpy_s(skill->Skill_Info.SkillDoc, "Harnesses the wind into a penetrating arrow that deals heavy damage.");
	skill->Skill_Info.FuncPointer = F_Archer_T5_Fierce_Wind; skill->Skill_Info.CODE = SKILL_T5_FIERCE_WIND;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Archer_T5_Fierce_UseMana; skill->Skill_Info.SkillNum = 17;

	skill = &sinSkill.UseSkill[19];
	if(skill->CODE != SKILL_T5_ENTALING_ROOTS) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Entaling Roots");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TA106 Entailing_Roots" : "TA60 R_Wolverin");
	skill->CODE = SKILL_T5_ENTALING_ROOTS; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 106;
	skill->Skill_Info.UseStamina[0] = 57; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 87; skill->Skill_Info.RequireMastery[1] = 3;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWS1; for(int i = 1; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Entaling Roots");
	strcpy_s(skill->Skill_Info.SkillDoc, "Roots the enemy to the ground with nature power.");
	skill->Skill_Info.FuncPointer = F_Archer_T5_Entaling_Roots; skill->Skill_Info.CODE = SKILL_T5_ENTALING_ROOTS;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Archer_T5_Roots_UseMana; skill->Skill_Info.SkillNum = 18;

	skill = &sinSkill.UseSkill[20];
	if(skill->CODE != SKILL_T5_BOMBARDMENT) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Bombardment");
	strcpy_s(skill->FileName, LegacySinSkillPolicy::UseCanonicalTier5Assets() ? "TA110 Bombardment" : "TA30 Avalanchie");
	skill->CODE = SKILL_T5_BOMBARDMENT; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 110;
	skill->Skill_Info.UseStamina[0] = 95; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 100; skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWS1; for(int i = 1; i < 8; i++) skill->Skill_Info.UseWeaponCode[i] = 0;
	strcpy_s(skill->Skill_Info.SkillName, "Bombardment");
	strcpy_s(skill->Skill_Info.SkillDoc, "Bombards the target with a continuous rain of arrows.");
	skill->Skill_Info.FuncPointer = F_Archer_T5_Bombardment; skill->Skill_Info.CODE = SKILL_T5_BOMBARDMENT;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Archer_T5_Bombard_UseMana; skill->Skill_Info.SkillNum = 19;
}

static void InitSummitTier5AtalantaSkillMetadata()
{
	if(sinChar->JOB_CODE != JOBCODE_ATALANTA) return;
	sSKILL *skill = &sinSkill.UseSkill[17];
	if(skill->CODE != SKILL_T5_SUMMON_TIGER) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Nature's Call"); strcpy_s(skill->FileName, "MA100 S_Tiger"); skill->CODE=SKILL_T5_SUMMON_TIGER; skill->Flag=1;
	skill->Skill_Info.RequireLevel=100; skill->Skill_Info.UseStamina[0]=95; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=169; skill->Skill_Info.RequireMastery[1]=0; skill->Skill_Info.Element[0]=2;
	for(int i=0;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Nature's Call"); strcpy_s(skill->Skill_Info.SkillDoc,"Summons the predator of the jungle to your aid.");
	skill->Skill_Info.FuncPointer=F_Atalanta_T5_Summon_Tiger; skill->Skill_Info.CODE=SKILL_T5_SUMMON_TIGER; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Atalanta_T5_SummonTiger_UseMana; skill->Skill_Info.SkillNum=16;

	skill=&sinSkill.UseSkill[18]; if(skill->CODE != SKILL_T5_GOLDEN_APPLE) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Golden Apple"); strcpy_s(skill->FileName,"MA103 Golden_Apple"); skill->CODE=SKILL_T5_GOLDEN_APPLE; skill->Flag=1; skill->Skill_Info.RequireLevel=103; skill->Skill_Info.UseStamina[0]=52; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.Element[0]=2;
	skill->Skill_Info.UseWeaponCode[0]=sinWT1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Golden Apple"); strcpy_s(skill->Skill_Info.SkillDoc,"Permanently increases movement speed and evade.");
	skill->Skill_Info.FuncPointer=F_Atalanta_T5_Golden_Apple; skill->Skill_Info.CODE=SKILL_T5_GOLDEN_APPLE; skill->Skill_Info.USECODE=SIN_SKILL_USE_NOT; skill->Skill_Info.UseMana=0; skill->Skill_Info.SkillNum=17;

	skill=&sinSkill.UseSkill[19]; if(skill->CODE != SKILL_T5_PLAGUE_JAVELIN) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Plague Javelin"); strcpy_s(skill->FileName,"MA106 Plague_Javelin"); skill->CODE=SKILL_T5_PLAGUE_JAVELIN; skill->Flag=1; skill->Skill_Info.RequireLevel=106; skill->Skill_Info.UseStamina[0]=68; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=100; skill->Skill_Info.RequireMastery[1]=4; skill->Skill_Info.Element[0]=2;
	int plagueWeapons[6]={sinWA1,sinWC1,sinWH1,sinWP1,sinWS1,sinWT1}; for(int i=0;i<6;i++) skill->Skill_Info.UseWeaponCode[i]=plagueWeapons[i]; skill->Skill_Info.UseWeaponCode[6]=0; skill->Skill_Info.UseWeaponCode[7]=0; strcpy_s(skill->Skill_Info.SkillName,"Plague Javelin"); strcpy_s(skill->Skill_Info.SkillDoc,"Infuses the weapon with jungle poison.");
	skill->Skill_Info.FuncPointer=F_Atalanta_T5_Plague_Javelin; skill->Skill_Info.CODE=SKILL_T5_PLAGUE_JAVELIN; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Atalanta_T5_Plague_UseMana; skill->Skill_Info.SkillNum=18;

	skill=&sinSkill.UseSkill[20]; if(skill->CODE != SKILL_T5_COMBO_JAVELIN) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Triple Threat"); strcpy_s(skill->FileName,"MA110 Combo_Javelin"); skill->CODE=SKILL_T5_COMBO_JAVELIN; skill->Flag=1; skill->Skill_Info.RequireLevel=110; skill->Skill_Info.UseStamina[0]=57; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=180; skill->Skill_Info.RequireMastery[1]=4; skill->Skill_Info.Element[0]=2; skill->Skill_Info.UseWeaponCode[0]=sinWT1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Triple Threat"); strcpy_s(skill->Skill_Info.SkillDoc,"Launches a three-hit javelin assault.");
	skill->Skill_Info.FuncPointer=F_Atalanta_T5_Combo_Javelin; skill->Skill_Info.CODE=SKILL_T5_COMBO_JAVELIN; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Atalanta_T5_Combo_UseMana; skill->Skill_Info.SkillNum=19;
}

static void InitSummitTier5KnightSkillMetadata()
{
	if(sinChar->JOB_CODE != JOBCODE_KNIGHT) return;

	sSKILL *skill = &sinSkill.UseSkill[17];
	if(skill->CODE != SKILL_T5_HOLY_CONVICTION) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Holy Conviction");
	strcpy_s(skill->FileName, "MN100 Holy_Conviction");
	skill->CODE = SKILL_T5_HOLY_CONVICTION; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 100;
	skill->Skill_Info.UseStamina[0] = 69; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 203; skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWS2; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0;
	strcpy_s(skill->Skill_Info.SkillName, "Holy Conviction");
	strcpy_s(skill->Skill_Info.SkillDoc, "Strengthens nearby allies against undead enemies.");
	skill->Skill_Info.FuncPointer = F_Knight_T5_Holy_Conviction; skill->Skill_Info.CODE = SKILL_T5_HOLY_CONVICTION;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Knight_T5_HolyConviction_UseMana; skill->Skill_Info.SkillNum = 16;

	skill = &sinSkill.UseSkill[18];
	if(skill->CODE != SKILL_T5_DIVINE_INQUISITON) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Divine Inquisition");
	strcpy_s(skill->FileName, "MN103 Divine_Inquisiton");
	skill->CODE = SKILL_T5_DIVINE_INQUISITON; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 103;
	skill->Skill_Info.UseStamina[0] = 70; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 70; skill->Skill_Info.RequireMastery[1] = 3;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWS2; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0;
	strcpy_s(skill->Skill_Info.SkillName, "Divine Inquisition");
	strcpy_s(skill->Skill_Info.SkillDoc, "Invokes divine swords to pierce enemies in an area.");
	skill->Skill_Info.FuncPointer = F_Knight_T5_Divine_Inquisition; skill->Skill_Info.CODE = SKILL_T5_DIVINE_INQUISITON;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Knight_T5_DivineInquisition_UseMana; skill->Skill_Info.SkillNum = 17;

	skill = &sinSkill.UseSkill[19];
	if(skill->CODE != SKILL_T5_GLORIOUS_SHIELD) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Glorious Shield");
	strcpy_s(skill->FileName, "MN106 Glorious_Shield");
	skill->CODE = SKILL_T5_GLORIOUS_SHIELD; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 106;
	skill->Skill_Info.UseStamina[0] = 75; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 203; skill->Skill_Info.RequireMastery[1] = 0;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	skill->Skill_Info.UseWeaponCode[0] = sinWS2; skill->Skill_Info.UseWeaponCode[1] = sinWH1; for(int i=2;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0;
	strcpy_s(skill->Skill_Info.SkillName, "Glorious Shield");
	strcpy_s(skill->Skill_Info.SkillDoc, "Raises the Knight's defense for a limited time.");
	skill->Skill_Info.FuncPointer = F_Knight_T5_Glorious_Shield; skill->Skill_Info.CODE = SKILL_T5_GLORIOUS_SHIELD;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Knight_T5_GloriousShield_UseMana; skill->Skill_Info.SkillNum = 18;

	skill = &sinSkill.UseSkill[20];
	if(skill->CODE != SKILL_T5_DIVINE_CROSS) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Divine Cross");
	strcpy_s(skill->FileName, "MN110 Divine_Cross");
	skill->CODE = SKILL_T5_DIVINE_CROSS; skill->Flag = 1;
	skill->Skill_Info.RequireLevel = 110;
	skill->Skill_Info.UseStamina[0] = 93; skill->Skill_Info.UseStamina[1] = 2;
	skill->Skill_Info.RequireMastery[0] = 180; skill->Skill_Info.RequireMastery[1] = 4;
	skill->Skill_Info.Element[0] = 2; skill->Skill_Info.Element[1] = 0; skill->Skill_Info.Element[2] = 0;
	int weapons[7] = {sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1};
	for(int i=0;i<7;i++) skill->Skill_Info.UseWeaponCode[i]=weapons[i]; skill->Skill_Info.UseWeaponCode[7]=0;
	strcpy_s(skill->Skill_Info.SkillName, "Divine Cross");
	strcpy_s(skill->Skill_Info.SkillDoc, "Performs a final cross strike empowered by divine power.");
	skill->Skill_Info.FuncPointer = F_Knight_T5_Divine_Cross; skill->Skill_Info.CODE = SKILL_T5_DIVINE_CROSS;
	skill->Skill_Info.USECODE = SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana = Knight_T5_DivineCross_UseMana; skill->Skill_Info.SkillNum = 19;
}

int cSKILL::SearchUseSkill()
{

	switch(sinChar->JOB_CODE)
	{
		case 1:
		UseSkillBox = GROUP_FIGHTER;

		break;
		case 2:
		UseSkillBox = GROUP_MECHANICIAN;

		break;
		case 3:
		UseSkillBox = GROUP_ARCHER;

		break;
		case 4:
		UseSkillBox = GROUP_PIKEMAN;

		break;
		case 5:
		UseSkillBox = GROUP_ATALANTA;
		break;
		case 6:
		UseSkillBox = GROUP_KNIGHT;
		break;
		case 7:
		UseSkillBox = GROUP_MAGICIAN;
		break;
		case 8:
		UseSkillBox = GROUP_PRIESTESS;
		break;

	}
	if(!UseSkillBox)
		return FALSE;

	int Index = 0;
	memcpy(&sinSkill.UseSkill[0], &sSkill[0], sizeof(sSKILL));
	sinSkill.UseSkill[0].Flag = 1;
	sinSkill.UseSkill[0].Use = 1;
	sinSkill.UseSkill[0].Skill_Info.USECODE = SIN_SKILL_USE_ALL;
	sinSkill.UseSkill[0].Point = 1;

	for(int i = 0; i < SIN_MAX_SKILL; i++)
	{
		if((sSkill[i].CODE & sinSKILL_MASK1) == UseSkillBox)
		{
			for(int j = 1; j < SIN_MAX_USE_SKILL; j++)
			{
				if(!sinSkill.UseSkill[j].Flag)
				{
					memcpy(&sinSkill.UseSkill[j], &sSkill[i], sizeof(sSKILL));
					for(int k = 0; k < SIN_MAX_SKILL; k++)
					{
						if(sinSkill.UseSkill[j].CODE == sSkill_Info[k].CODE)
						{
							memcpy(&sinSkill.UseSkill[j].Skill_Info, &sSkill_Info[k], sizeof(sSKILL_INFO));
							sinSkill.UseSkill[j].Skill_Info.SkillNum = j - 1;
							break;
						}
					}
					sinSkill.UseSkill[j].Flag = 1;




					break;
				}
			}
		}
	}
	InitSummitTier5MechanicianSkillMetadata();
	InitSummitTier5FighterSkillMetadata();
	InitSummitTier5PikemanSkillMetadata();
	InitSummitTier5ArcherSkillMetadata();
	InitSummitTier5AtalantaSkillMetadata();
	InitSummitTier5KnightSkillMetadata();
	LegacySinSkillPolicy::InitAdditionalTier5Metadata();

	memcpy(&sinSkill.SkillBox, &sSkillBox, sizeof(sSKILLBOX)*SIN_MAX_USE_SKILL);

	LoadUseSkillImage();

	sinSkill.UseSkill[0].Use = 1;
	sinSkill.UseSkill[0].Skill_Info.USECODE = SIN_SKILL_USE_ALL;
	sinSkill.UseSkill[0].Point = 1;
	ReFormCharSkillInfo();
	return FALSE;
}

void cSKILL::CheckingNowSkillState()
{


	switch(sinChar->ChangeJob)
	{
		case 0:
		ChangeJobSkillPlus = 5;
		break;
		case 1:
		ChangeJobSkillPlus = 9;
		break;
		case 2:
		ChangeJobSkillPlus = 13;
		break;
		case 3:
		ChangeJobSkillPlus = 17;
		break;
		case CHARACTER_RANK5:
		ChangeJobSkillPlus = 21;
		break;

	}
	if(JobSkillTemp != ChangeJobSkillPlus)
	{
		JobSkillTemp = ChangeJobSkillPlus;
		cInvenTory.SetItemToChar();
	}

	if(sinChar->ChangeJob < CHARACTER_RANK5 && SkillMain2OpenFlag)
		SkillMain2OpenFlag = 0;

	for(int j = 0; j < ChangeJobSkillPlus; j++)
	{
		if(j >= SIN_TIER5_SKILL_START)
		{
			// Do not synthesize placeholder skills before their metadata/handlers are ported.
			if(!sinSkill.UseSkill[j].Flag || !sinSkill.UseSkill[j].CODE)
			{
				sinSkill.UseSkill[j].Use = 0;
				continue;
			}

			if(sinSkill.SkillPoint5)
			{
				if(sinSkill.UseSkill[j].Point <= MAX_USE_SKILL_POINT)
				{
					if(sinSkill.UseSkill[j - 1].Point)
					{
						if(sinSkill.UseSkill[j].Skill_Info.RequireLevel + (sinSkill.UseSkill[j].Point * 2) <= sinChar->Level)
							sinSkill.UseSkill[j].Use = 1;
						else
							sinSkill.UseSkill[j].Use = 0;
					}
				}
			}
			else
			{
				sinSkill.UseSkill[j].Use = 0;
			}
			continue;
		}
		if(j > 12)
		{
			if(sinSkill.SkillPoint4)
			{
				sinSkill.UseSkill[j].Flag = 1;
				if(sinSkill.UseSkill[j].Point <= MAX_USE_SKILL_POINT)
				{
					if(sinSkill.UseSkill[j - 1].Point)
					{
						if(sinSkill.UseSkill[j].Skill_Info.RequireLevel + (sinSkill.UseSkill[j].Point * 2) <= sinChar->Level)
							sinSkill.UseSkill[j].Use = 1;
						else
							sinSkill.UseSkill[j].Use = 0;
					}
				}
			}
			else
			{
				sinSkill.UseSkill[j].Use = 0;
			}
			continue;
		}
		if(sinSkill.SkillPoint)
		{
			sinSkill.UseSkill[j].Flag = 1;
			if(sinSkill.UseSkill[j].Point <= MAX_USE_SKILL_POINT)
			{
				if(j == 0)continue;
				if(sinSkill.UseSkill[j - 1].Point)
				{
					if(sinSkill.UseSkill[j].Skill_Info.RequireLevel + (sinSkill.UseSkill[j].Point * 2) <= sinChar->Level)
						sinSkill.UseSkill[j].Use = 1;
					else
						sinSkill.UseSkill[j].Use = 0;
				}
			}
		}
		else
		{
			sinSkill.UseSkill[j].Use = 0;
		}
	}
}

void cSKILL::CheckSkillMastery()
{

	int TempLenght = 0;

	int TempTalent = 0;



	if(sinSkill.pLeftSkill)
	{
		if(!sinSkill.pLeftSkill->Point)
			sinSkill.pLeftSkill->UseSkillFlag = 0;
	}
	if(sinSkill.pRightSkill)
	{
		if(!sinSkill.pRightSkill->Point)
			sinSkill.pRightSkill->UseSkillFlag = 0;
	}

	for(int j = 1; j < SIN_MAX_USE_SKILL; j++)
	{
		if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Point)
		{

			CheckSkillMasteryForm(0, j);

			if(sinSkill.UseSkill[j].Mastery == 0)
			{
				sinSkill.UseSkill[j].GageLength = 0;
				sinSkill.UseSkill[j].GageLength2 = 0;
				SkillCountTime2[j] = 0;

			}


			TempTalent = (int)(sinChar->Talent / 3) + (int)sinAdd_fMagic_Mastery;
			if(TempTalent > 50)TempTalent = 50;
			sinSkill.UseSkill[j].UseSkillMastery = (int)(TempTalent * 100) + sinSkill.UseSkill[j].UseSkillCount;
			if(sinSkill.UseSkill[j].Skill_Info.Element[0])
			{
				sinSkill.UseSkill[j].UseSkillMastery = 10000;

			}

			if(sinSkill.UseSkill[j].UseSkillMastery >= 10000)sinSkill.UseSkill[j].UseSkillMastery = 10000;
			if(sinSkill.UseSkill[j].UseSkillMastery)
			{
				sinSkill.UseSkill[j].UseSkillMasteryGage = (int)((35 * ((float)sinSkill.UseSkill[j].UseSkillMastery / 10000)));
				if(sinSkill.UseSkill[j].UseSkillMasteryGage > 35)sinSkill.UseSkill[j].UseSkillMasteryGage = 35;
			}

			sinSkill.UseSkill[j].Mastery = (sinSkill.UseSkill[j].Skill_Info.RequireMastery[0] + (sinSkill.UseSkill[j].Skill_Info.RequireMastery[1] * sinSkill.UseSkill[j].Point) -
				(sinSkill.UseSkill[j].UseSkillMastery / 100));
			if(sinSkill.UseSkill[j].Mastery <= 0) sinSkill.UseSkill[j].Mastery = 1;
			if(sinSkill.UseSkill[j].Mastery > 70)sinSkill.UseSkill[j].Mastery = 70;


			if(sinAddSkillDelay)
			{
				sinSkill.UseSkill[j].Mastery = 1;
			}

			else
			{
				sinSkill.UseSkill[j].Mastery = (sinSkill.UseSkill[j].Skill_Info.RequireMastery[0] + (sinSkill.UseSkill[j].Skill_Info.RequireMastery[1] * sinSkill.UseSkill[j].Point) -
					(sinSkill.UseSkill[j].UseSkillMastery / 100));

				if(sinSkill.UseSkill[j].Mastery <= 0) sinSkill.UseSkill[j].Mastery = 1;
				if(sinSkill.UseSkill[j].Mastery > 70)sinSkill.UseSkill[j].Mastery = 70;
			}


			TempLenght = (int)(35 / ((float)sinSkill.UseSkill[j].Mastery / 2));

			sinSkill.UseSkill[j].GageLength2 += TempLenght;

			if(sinSkill.UseSkill[j].GageLength < sinSkill.UseSkill[j].GageLength2)
			{
				SkillCountTime2[j]++;
				if(SkillCountTime2[j] >= (int)(35 / (float)TempLenght))
				{
					sinSkill.UseSkill[j].GageLength++;
					SkillCountTime2[j] = 0;

				}

			}

			if(sinSkill.UseSkill[j].GageLength >= 35)
			{
				sinSkill.UseSkill[j].GageLength = 35;
				sinSkill.UseSkill[j].GageLength2 = 35;
				sinSkill.UseSkill[j].UseSkillFlag = 1;
				CheckUseSkillState(&sinSkill.UseSkill[j]);


			}
			else
				sinSkill.UseSkill[j].UseSkillFlag = 0;

			ReformSkillMasteryForm(0, j);

		}

	}


}

int cSKILL::GetInfoBoxSize(sSKILLBOX *pSkillBox, int Col, int Icon)
{

	int ty, tx;

	SkillBoxSize.x = 14;
	SkillBoxSize.y = Col + 1;

	SkillInfoBoxPosi.x = (pSkillBox->BoxRect.left - 112) + 20;
	SkillInfoBoxPosi.y = pSkillBox->BoxRect.top - (SkillBoxSize.y * 16);

	if(Icon)
	{
		SkillInfoBoxPosi.x = pSkillBox->BoxRect.left;
		SkillInfoBoxPosi.y = pSkillBox->BoxRect.top;

	}

	ty = SkillInfoBoxPosi.y + (SkillBoxSize.y * 16);
	tx = SkillInfoBoxPosi.x + (SkillBoxSize.x * 16);


	if(SkillInfoBoxPosi.y < 0)
		SkillInfoBoxPosi.y = 0;
	if(SkillInfoBoxPosi.x < 0)
		SkillInfoBoxPosi.x = 0;

	if(ty > 600)
		SkillInfoBoxPosi.y -= (ty - 600);
	if(tx > 800)
		SkillInfoBoxPosi.x -= (tx - 800);

	return TRUE;
}

int cSKILL::GetSkillMastery(sSKILL *pSkill, sSKILLBOX *pSkillBox)
{
	int SkillMasteryTemp = 0;
	SkillMasteryTemp = ((int)pSkill->UseSkillMastery / 100);
	wsprintf(szUseSkillMastery, "%d", SkillMasteryTemp);

	UseSkillMasteryPosi.x = pSkillBox->GageRect.left - (34 / 2);
	UseSkillMasteryPosi.y = pSkillBox->GageRect.top - 29;

	return TRUE;
}

int cSKILL::CheckUseSkillState(sSKILL *pSkill)
{
	if(pSkill->Skill_Info.UseMana)
	{


		if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1 && chaPremiumitem.m_StaminaReducePotiontime > 0)
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - ((pSkill->Skill_Info.UseMana[pSkill->Point - 1] *
																				 chaPremiumitem.m_ManaReducePotionValue / 100) + (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * 20 / 100)) &&
			   sinGetStamina() >= (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1))) - ((pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1))) * chaPremiumitem.m_StaminaReducePotionValue / 100))
			{
				pSkill->UseSkillFlag = 1;
			}

			else
				pSkill->UseSkillFlag = 0;
		}


		else if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_StaminaReducePotiontime > 0)
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) &&
			   sinGetStamina() >= (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1))) - ((pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1))) * chaPremiumitem.m_StaminaReducePotionValue / 100))
			{
				pSkill->UseSkillFlag = 1;
			}
			else
				pSkill->UseSkillFlag = 0;
		}


		else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1 && chaPremiumitem.m_StaminaReducePotiontime > 0)
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * 20 / 100) &&
			   sinGetStamina() >= (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1))) - ((pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1))) * chaPremiumitem.m_StaminaReducePotionValue / 100))
			{
				pSkill->UseSkillFlag = 1;
			}
			else
				pSkill->UseSkillFlag = 0;
		}


		else if(chaPremiumitem.m_ManaReducePotiontime > 0)
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) &&
			   sinGetStamina() >= pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1)))
			{
				pSkill->UseSkillFlag = 1;
			}
			else
				pSkill->UseSkillFlag = 0;
		}


		else if(chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * 20 / 100) &&
			   sinGetStamina() >= pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1)))
			{
				pSkill->UseSkillFlag = 1;
			}
			else
				pSkill->UseSkillFlag = 0;
		}


		else if(chaPremiumitem.m_StaminaReducePotiontime > 0)
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] &&
			   sinGetStamina() >= (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1))) - ((pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1))) * chaPremiumitem.m_StaminaReducePotionValue / 100))
			{
				pSkill->UseSkillFlag = 1;

			}
			else
				pSkill->UseSkillFlag = 0;
		}


		else
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] &&
			   sinGetStamina() >= pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1] * (pSkill->Point - 1)))
			{
				pSkill->UseSkillFlag = 1;

			}
			else
				pSkill->UseSkillFlag = 0;
		}
	}
	return TRUE;
}

int cSKILL::InitSkillPoint(int kind)
{

	switch(kind)
	{
		case 0:
		if(!sinInitPointPassFlag)
		{
			if(sinChar->wVersion[1] == 1)
			{
				cMessageBox.ShowMessage(MESSAGE_ALREADY_INITSTATE);
				return FALSE;
			}
		}
		break;
	}
	sinInitPointPassFlag = 0;

	CheckingNowSkillState();
	CheckCharForm();
	int Count = 0;
	int Count2 = 0;
	int Count5 = 0;

	for(int j = 1; j < ChangeJobSkillPlus; j++)
	{
		if(j >= SIN_TIER5_SKILL_START)
		{
			if(sinSkill.UseSkill[j].Point > 0)
				Count5 += sinSkill.UseSkill[j].Point;
		}
		else if(j > 12)
		{
			if(sinSkill.UseSkill[j].Point > 0)
				Count2 += sinSkill.UseSkill[j].Point;
		}
		else
		{
			if(sinSkill.UseSkill[j].Point > 0)
				Count += sinSkill.UseSkill[j].Point;
		}

		cSkill.CheckSkillMasteryForm(0, j);
		CheckSkillPointForm(&sinSkill.UseSkill[j]);
		sinSkill.UseSkill[j].Point = 0;
		sinSkill.UseSkill[j].GageLength = 0;
		sinSkill.UseSkill[j].GageLength2 = 0;
		sinSkill.UseSkill[j].Mastery = 0;
		sinSkill.UseSkill[j].UseSKillIncreCount = 0;
		sinSkill.UseSkill[j].UseSkillCount = 0;
		sinSkill.UseSkill[j].UseSkillMastery = 0;
		sinSkill.UseSkill[j].UseSkillMasteryGage = 0;
		sinSkill.UseSkill[j].Use = 0;
		ReformSkillPointForm(&sinSkill.UseSkill[j]);
		ReformSkillMasteryForm(0, j);

	}
	sinSkill.SkillPoint += Count;
	sinSkill.SkillPoint4 += Count2;
	sinSkill.SkillPoint5 += Count5;
	sinChar->wVersion[1] = 1;
	ReformCharForm();
	cMessageBox.ShowMessage(MESSAGE_GYUNGFUM_OK);
	cInvenTory.SetItemToChar();

	return TRUE;
}

int cSKILL::SearchClearSkillPoint(DWORD CODE)
{

	for(int i = 0; i < 10; i++)
	{
		if(ClearSkillPointCODE[i] == CODE)
		{
			ClearSkillCODE = ClearSkillPointCODE[i];
			return TRUE;
		}
		else
			ClearSkillCODE = 0;
	}

	return FALSE;

}

int cSKILL::ShowClearSkillPointButton(int x, int y)
{
	ClearSkillButtonPosi.x = -50;
	ClearSkillButtonPosi.y = -50;

	if(ClearSkillCODE && !sinMessageBoxShowFlag)
	{
		ClearSkillButtonPosi.x = x - 57;
		ClearSkillButtonPosi.y = y - 51;
		DrawSprite(ClearSkillButtonPosi.x, ClearSkillButtonPosi.y, lpSkillDown, 0, 0, 50, 44);
	}

	return TRUE;
}

int cSKILL::ClearSkillPointSection()
{

	CheckingNowSkillState();
	CheckCharForm();
	int Count = 0;
	for(int j = 1; j < ChangeJobSkillPlus; j++)
	{
		if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Point > 1)
		{
			for(int i = 0; i < 10; i++)
			{
				if(sinSkill.UseSkill[j].CODE == ClearSkillPointCODE[i])
				{
					cSkill.CheckSkillMasteryForm(0, j);
					CheckSkillPointForm(&sinSkill.UseSkill[j]);
					sinSkill.UseSkill[j].Point--;
					sinSkill.UseSkill[j].GageLength = 0;
					sinSkill.UseSkill[j].GageLength2 = 0;
					sinSkill.UseSkill[j].Mastery = 0;
					sinSkill.UseSkill[j].UseSKillIncreCount = 0;
					sinSkill.UseSkill[j].UseSkillCount = 0;
					sinSkill.UseSkill[j].UseSkillMastery = 0;
					sinSkill.UseSkill[j].UseSkillMasteryGage = 0;
					sinSkill.UseSkill[j].Use = 0;
					ReformSkillPointForm(&sinSkill.UseSkill[j]);
					ReformSkillMasteryForm(0, j);
					sinSkill.SkillPoint++;
					ClearSkillCODE = 0;
					LearnSkillEffect(0);
					ReformCharForm();
					return TRUE;
				}
			}
		}
	}
	return TRUE;
}

static std::map<int, unsigned int> SharedCharSkillInfoData;
static std::map<int, int> SharedCharSkillInfoChangeJob;

int cSKILL::ReFormCharSkillInfo()
{
	int j = 0, k = 0;

	const int classCode = sinChar->JOB_CODE;
	const std::map<int, int>::const_iterator lastChangeJob = SharedCharSkillInfoChangeJob.find(classCode);
	if(lastChangeJob == SharedCharSkillInfoChangeJob.end() || lastChangeJob->second != sinChar->ChangeJob)
	{
		SharedCharSkillInfoChangeJob[classCode] = sinChar->ChangeJob;
		unsigned int& classSkillInfo = SharedCharSkillInfoData[classCode];
		classSkillInfo = 0;
		for(j = 1; j < SIN_MAX_USE_SKILL; j++)
		{
			if(sinSkill.UseSkill[j].Flag)
			{
				k = j + 1;
				classSkillInfo += k*sinSkill.UseSkill[j].Skill_Info.RequireLevel;
				classSkillInfo += k*sinSkill.UseSkill[j].Skill_Info.UseStamina[0];
				classSkillInfo += k*sinSkill.UseSkill[j].Skill_Info.UseStamina[1];
			}
		}
	}
	return TRUE;
}

int cSKILL::CheckSkillMasteryForm(sSKILL *pSkill, int Index)
{
	if(pSkill)
	{
		for(int j = 1; j < SIN_MAX_USE_SKILL; j++)
		{
			if(sinSkill.UseSkill[j].Flag)
			{
				if(pSkill->CODE == sinSkill.UseSkill[j].CODE)
				{
					Index = j;
					break;
				}
			}
		}
	}



	int CheckMasteryDataSum = lpCurPlayer->dwObjectSerial + sinSkill.UseSkill[Index].CODE;

	CheckMasteryDataSum += (sinSkill.UseSkill[Index].GageLength*Index);
	CheckMasteryDataSum += (sinSkill.UseSkill[Index].GageLength2*Index);
	CheckMasteryDataSum += (sinSkill.UseSkill[Index].Mastery*Index);

	if(CheckMasteryData[Index] != CheckMasteryDataSum)
	{


		SendSetHackUser3(52, CheckMasteryData[Index], CheckMasteryDataSum);
		return FALSE;

	}

	return TRUE;
}

int cSKILL::ReformSkillMasteryForm(sSKILL *pSkill, int Index)
{

	if(pSkill)
	{
		for(int j = 1; j < SIN_MAX_USE_SKILL; j++)
		{
			if(sinSkill.UseSkill[j].Flag)
			{
				if(pSkill->CODE == sinSkill.UseSkill[j].CODE)
				{
					Index = j;
				}
			}
		}
	}

	CheckMasteryData[Index] = lpCurPlayer->dwObjectSerial + sinSkill.UseSkill[Index].CODE;
	CheckMasteryData[Index] += (sinSkill.UseSkill[Index].GageLength*Index);
	CheckMasteryData[Index] += (sinSkill.UseSkill[Index].GageLength2*Index);
	CheckMasteryData[Index] += (sinSkill.UseSkill[Index].Mastery*Index);

	return TRUE;
}

int cSKILL::CheckSkillPointForm(sSKILL *pSkill)
{
	DWORD	dwXorKey = lpCurPlayer->dwObjectSerial & 0x5F;

	for(int j = 1; j < SIN_MAX_USE_SKILL; j++)
	{
		if(sinSkill.UseSkill[j].Flag &&sinSkill.UseSkill[j].Use &&sinSkill.UseSkill[j].Point)
		{
			if(pSkill->CODE == sinSkill.UseSkill[j].CODE)
			{

				if(CheckSkillPointData[j] != (sinSkill.UseSkill[j].Point^dwXorKey))
				{
					SendSetHackUser3(54, CheckSkillPointData[j], sinSkill.UseSkill[j].Point);
					return FALSE;
				}

			}
		}
	}

	return TRUE;
}

int cSKILL::ReformSkillPointForm(sSKILL *pSkill)
{
	DWORD	dwXorKey = lpCurPlayer->dwObjectSerial & 0x5F;

	for(int j = 1; j < SIN_MAX_USE_SKILL; j++)
	{
		if(sinSkill.UseSkill[j].Flag)
		{
			if(pSkill->CODE == sinSkill.UseSkill[j].CODE)
			{
				CheckSkillPointData[j] = (sinSkill.UseSkill[j].Point^dwXorKey);
				break;
			}
		}
	}
	return TRUE;
}

int cSKILL::GetSkillPoint(DWORD CODE)
{
	for(int j = 1; j < SIN_MAX_USE_SKILL; j++)
	{
		if(sinSkill.UseSkill[j].Flag)
		{
			if(CODE == sinSkill.UseSkill[j].CODE)
			{
				return sinSkill.UseSkill[j].Point;
			}
		}
	}

	return 0;
}

int cSKILL::SetVirtualLife(int Time, int Point)
{


	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag)
		{
			if(ContinueSkill[i].CODE == SKILL_VIRTUAL_LIFE)
			{

				return FALSE;
			}
		}
	}

	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(!ContinueSkill[i].Flag)
		{
			ContinueSkill[i].Flag = 1;
			ContinueSkill[i].CODE = SKILL_VIRTUAL_LIFE;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}

	AddVirtualLife[1] = ((int)sinChar->Life[1] * Virtual_Life_Percent[Point - 1]) / 100;
	sinVirtualLifeTime = Time;
	sinVirtualLifePercent = Virtual_Life_Percent[Point - 1];
	CheckVirtualLifeTime = 0;
	cInvenTory.SetItemToChar();
	return TRUE;
}

int cSKILL::UseBoosterItem(sITEM *pItem)
{
	if(lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_VILLAGE)
		return FALSE;

	DWORD BoosterCODE;
	DWORD ItemMask;
	int BoosterTime = 0;
	int cnt = 0;

	ItemMask = pItem->CODE&sinITEM_MASK3;
	if(ItemMask == sin21 || ItemMask == sin24 || ItemMask == sin27) cnt = 0;
	if(ItemMask == sin22 || ItemMask == sin25 || ItemMask == sin28) cnt = 1;
	if(ItemMask == sin23 || ItemMask == sin26 || ItemMask == sin29) cnt = 2;

	switch(ItemMask)
	{
		case sin21:
		case sin22:
		case sin23:
		BoosterCODE = BOOSTER_ITEM_LIFE;
		BoosterTime = BoosterItem_UseTime[cnt];

		AddBoosterLife = ((int)sinChar->Life[1] * BoosterItem_DataPercent[0]) / 100;
		BoosterLifeTime = BoosterTime;
		BoosterLifePercent = BoosterItem_DataPercent[0];
		CheckBoosterLifeTime = 0;
		break;
		case sin24:
		case sin25:
		case sin26:
		BoosterCODE = BOOSTER_ITEM_MANA;
		BoosterTime = BoosterItem_UseTime[cnt];


		AddVirtualMana[1] = ((int)sinChar->Mana[1] * BoosterItem_DataPercent[1]) / 100;
		sinVirtualManaTime = BoosterTime;
		sinVirtualManaPercent = BoosterItem_DataPercent[1];
		CheckVirtualManaTime = 0;
		break;
		case sin27:
		case sin28:
		case sin29:
		BoosterCODE = BOOSTER_ITEM_STAMINA;
		BoosterTime = BoosterItem_UseTime[cnt];


		AddVirtualStamina[1] = ((int)sinChar->Stamina[1] * BoosterItem_DataPercent[2]) / 100;
		sinVirtualStaminaTime = BoosterTime;
		sinVirtualStaminaPercent = BoosterItem_DataPercent[2];
		CheckVirtualStaminaTime = 0;
		break;
	}

	sSKILL TempSkill;

	if(cSkill.SearchContiueSkillCODE(BoosterCODE) == BoosterCODE && BoosterCODE != 0)
	{
		cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
		haCastleSkillUseFlag = 0;
		return TRUE;
	}
	haCastleSkillUseFlag = 1;

	for(int j = 0; j < SIN_MAX_SKILL; j++)
	{
		if(sSkill[j].CODE == BoosterCODE)
		{
			memcpy(&TempSkill, &sSkill[j], sizeof(sSKILL));
			TempSkill.UseTime = BoosterTime;
			sinContinueSkillSet(&TempSkill);
			SwitchSkill(&TempSkill);
			break;
		}
	}

	tempLife[1] = sinChar->Level;
	tempMana[1] = sinChar->Level;
	tempStamina[1] = sinChar->Level;

	sinPlaySound(SIN_SOUND_EAT_POTION2);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int cSKILL::SetBoosterItem(DWORD ItemCODE, int Time)
{
	int UseFlag = 0;
	sSKILL TempSkill;
	memset(&TempSkill, 0, sizeof(sSKILL));
	char *Path = "";

	DWORD ItemMask;
	int cnt = 0;

	ItemMask = ItemCODE&sinITEM_MASK3;

	if(ItemMask == sin21 || ItemMask == sin24 || ItemMask == sin27) cnt = 0;
	if(ItemMask == sin22 || ItemMask == sin25 || ItemMask == sin28) cnt = 1;
	if(ItemMask == sin23 || ItemMask == sin26 || ItemMask == sin29) cnt = 2;

	switch(ItemMask)
	{

		case sin21:
		case sin22:
		case sin23:
		UseFlag = 0;
		Path = "image//Sinimage//skill//keep//Booster_01.tga";
		TempSkill.CODE = BOOSTER_ITEM_LIFE;
		TempSkill.Point = 0;
		TempSkill.UseTime = BoosterItem_UseTime[cnt];
		TempSkill.CheckTime = (BoosterItem_UseTime[cnt] * 70) - (Time * 70);
		TempSkill.MatIcon = Mat_BoosterItem[UseFlag];
		break;


		case sin24:
		case sin25:
		case sin26:
		UseFlag = 1;
		Path = "image//Sinimage//skill//keep//Booster_02.tga";
		TempSkill.CODE = BOOSTER_ITEM_MANA;
		TempSkill.Point = 0;
		TempSkill.UseTime = BoosterItem_UseTime[cnt];
		TempSkill.CheckTime = (BoosterItem_UseTime[cnt] * 70) - (Time * 70);
		TempSkill.MatIcon = Mat_BoosterItem[UseFlag];
		break;


		case sin27:
		case sin28:
		case sin29:
		UseFlag = 2;
		Path = "image//Sinimage//skill//keep//Booster_03.tga";
		TempSkill.CODE = BOOSTER_ITEM_STAMINA;
		TempSkill.Point = 0;
		TempSkill.UseTime = BoosterItem_UseTime[cnt];
		TempSkill.CheckTime = (BoosterItem_UseTime[cnt] * 70) - (Time * 70);
		TempSkill.MatIcon = Mat_BoosterItem[UseFlag];
		break;
	}

	char szBuff[128];
	if(!Mat_BoosterItem[UseFlag])
	{
		wsprintf(szBuff, Path, UseFlag);
		Mat_BoosterItem[UseFlag] = CreateTextureMaterial(szBuff, 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
		ReadTextures();
	}

	sinContinueSkillSet(&TempSkill);

	if(UseFlag == 0)
	{

		AddBoosterLife = ((int)sinChar->Life[1] * BoosterItem_DataPercent[UseFlag]) / 100;
		BoosterLifeTime = Time;
		BoosterLifePercent = BoosterItem_DataPercent[UseFlag];
		CheckBoosterLifeTime = 0;
	}
	else if(UseFlag == 1)
	{

		AddVirtualMana[1] = ((int)sinChar->Mana[1] * BoosterItem_DataPercent[UseFlag]) / 100;
		sinVirtualManaTime = Time;
		sinVirtualManaPercent = BoosterItem_DataPercent[UseFlag];
		CheckVirtualManaTime = 0;
	}
	else if(UseFlag == 2)
	{

		AddVirtualStamina[1] = ((int)sinChar->Stamina[1] * BoosterItem_DataPercent[UseFlag]) / 100;
		sinVirtualStaminaTime = Time;
		sinVirtualStaminaPercent = BoosterItem_DataPercent[UseFlag];
		CheckVirtualStaminaTime = 0;
	}

	cInvenTory.SetItemToChar();

	return TRUE;
}

int cSKILL::SetSkillDelayItem(DWORD ItemCODE, int Time)
{
	int UseFlag = 0;
	sSKILL TempSkill;
	memset(&TempSkill, 0, sizeof(sSKILL));
	char *Path = "";

	DWORD ItemMask;
	int cnt = 0;

	ItemMask = ItemCODE&sinITEM_MASK3;

	if(ItemMask == sin30 || ItemMask == sin33 || ItemMask == sin36) cnt = 0;
	if(ItemMask == sin31 || ItemMask == sin34 || ItemMask == sin37) cnt = 1;
	if(ItemMask == sin32 || ItemMask == sin35 || ItemMask == sin38) cnt = 2;

	switch(ItemMask)
	{

		case sin30:
		case sin31:
		case sin32:
		UseFlag = 0;
		Path = "image//Sinimage//skill//keep//Booster_01.tga";
		TempSkill.CODE = SKILLDELAY_ITEM_LIFE;
		TempSkill.Point = 0;
		TempSkill.UseTime = nSkillDelay_UseTime[cnt];
		TempSkill.CheckTime = (nSkillDelay_UseTime[cnt] * 70) - (Time * 70);
		TempSkill.MatIcon = nMat_SkillDelayItem[UseFlag];
		break;
	}


	char szBuff[128];
	if(!nMat_SkillDelayItem[UseFlag])
	{
		wsprintf(szBuff, Path, UseFlag);
		nMat_SkillDelayItem[UseFlag] = CreateTextureMaterial(szBuff, 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
		ReadTextures();
	}

	sinContinueSkillSet(&TempSkill);

	if(UseFlag == 0)
	{
		sinAddSkillDelay = 10;
		sinSkillDelayTime = Time;
		BoosterLifePercent = BoosterItem_DataPercent[UseFlag];
		CheckSkillDelayTime = 0;
	}


	cInvenTory.SetItemToChar();

	return TRUE;
}

int cSKILL::UseSkillDelayItem(sITEM *pItem)
{
	DWORD SkillDelayCODE;
	DWORD ItemMask;
	int SkillDelayTime = 0;
	int cnt = 0;

	ItemMask = pItem->CODE&sinITEM_MASK3;
	if(ItemMask == sin30 || ItemMask == sin33 || ItemMask == sin36) cnt = 0;
	if(ItemMask == sin31 || ItemMask == sin34 || ItemMask == sin37) cnt = 1;
	if(ItemMask == sin32 || ItemMask == sin35 || ItemMask == sin38) cnt = 2;

	switch(ItemMask)
	{
		case sin30:
		case sin31:
		case sin32:
		SkillDelayCODE = SKILLDELAY_ITEM_LIFE;
		SkillDelayTime = nSkillDelay_UseTime[cnt];

		sinAddSkillDelay = 10;
		sinSkillDelayTime = SkillDelayTime;
		CheckSkillDelayTime = 0;
		break;
	}

	sSKILL TempSkill;

	if(cSkill.SearchContiueSkillCODE(SkillDelayCODE) == SkillDelayCODE && SkillDelayCODE != 0)
	{
		cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
		haCastleSkillUseFlag = 0;
		return TRUE;
	}
	haCastleSkillUseFlag = 1;

	for(int j = 0; j < SIN_MAX_SKILL; j++)
	{
		if(sSkill[j].CODE == SkillDelayCODE)
		{
			memcpy(&TempSkill, &sSkill[j], sizeof(sSKILL));
			TempSkill.UseTime = SkillDelayTime;
			sinContinueSkillSet(&TempSkill);
			SwitchSkill(&TempSkill);
			break;
		}
	}

	sinPlaySound(SIN_SOUND_EAT_POTION2);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int cSKILL::SetT_Of_Valhalla(int Point, int LV)
{


	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag)
		{
			if(ContinueSkill[i].CODE == SKILL_TRIUMPH_OF_VALHALLA || ContinueSkill[i].CODE == SKILL_HALL_OF_VALHALLA)
			{

				return FALSE;
			}
		}
	}

	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(!ContinueSkill[i].Flag)
		{
			ContinueSkill[i].Flag = 1;
			ContinueSkill[i].CODE = SKILL_TRIUMPH_OF_VALHALLA;
			ContinueSkill[i].UseTime = T_Of_Valhalla_Time[Point - 1];
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}

	SetT_Of_ValhallaLV = LV;

	if(SetT_Of_ValhallaFlag2)
	{
		SetT_Of_ValhallaFlag2 = 0;
		SetT_Of_ValhallaFlag = 0;
	}
	else
	{
		SetT_Of_ValhallaFlag = 1;

	}
	cInvenTory.SetItemToChar();
	return TRUE;
}

int cSKILL::SetEnchant_Weapon(int Time, int Point)
{


	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag)
		{
			if(ContinueSkill[i].CODE == SKILL_ENCHANT_WEAPON)
			{

				return FALSE;
			}
		}
	}

	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(!ContinueSkill[i].Flag)
		{
			ContinueSkill[i].Flag = 1;
			ContinueSkill[i].CODE = SKILL_ENCHANT_WEAPON;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}
	cInvenTory.SetItemToChar();
	return TRUE;
}

int cSKILL::ForeceOfNature(int Time, int Point, int Party)
{


	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag)
		{
			if(ContinueSkill[i].CODE == SKILL_FORCE_OF_NATURE)
			{
				if(ContinueSkill[i].Flag == 2 && !Party)
				{
					memset(&ContinueSkill[i], 0, sizeof(sSKILL));
					break;
				}
				else return FALSE;
			}
		}
	}

	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(!ContinueSkill[i].Flag)
		{
			ContinueSkill[i].Flag = Party + 1;
			ContinueSkill[i].CODE = SKILL_FORCE_OF_NATURE;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}
	cInvenTory.SetItemToChar();
	return TRUE;
}

int cSKILL::HellOfValhalla(int Time, int Point, int TVPoint, int LV, int Party)
{


	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag)
		{
			if(ContinueSkill[i].CODE == SKILL_HALL_OF_VALHALLA)
			{
				if(ContinueSkill[i].Flag == 2 && !Party)
				{
					memset(&ContinueSkill[i], 0, sizeof(sSKILL));
					break;
				}
				else return FALSE;
			}
		}
	}

	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(!ContinueSkill[i].Flag)
		{
			ContinueSkill[i].Flag = Party + 1;
			ContinueSkill[i].CODE = SKILL_HALL_OF_VALHALLA;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}
	SetT_Of_ValhallaPOINT = TVPoint;
	SetH_Of_ValhallaLV = LV;
	cInvenTory.SetItemToChar();
	return TRUE;
}

int cSKILL::Regeneration_Field(int Time, int Point, int Party)
{


	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag)
		{
			if(ContinueSkill[i].CODE == SKILL_REGENERATION_FIELD)
			{
				if(ContinueSkill[i].UseTime > Time)return FALSE;
				ContinueSkill[i].UseTime = Time;
				return TRUE;
			}
		}
	}

	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(!ContinueSkill[i].Flag)
		{
			ContinueSkill[i].Flag = 1 + Party;
			ContinueSkill[i].CODE = SKILL_REGENERATION_FIELD;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}
	cInvenTory.SetItemToChar();
	return TRUE;
}

int cSKILL::SearchSkillDoc(sSKILL *pSkill)
{

	for(int i = 0; i < SIN_MAX_SKILL; i++)
	{
		if(pSkill->CODE == sSkill_Info[i].CODE)
		{
			if(pSkill->CODE == SKILL_FORCE_ORB)
			{
			#ifdef _LANGUAGE_CHINESE
				wsprintf(pSkill->Skill_Info.SkillName, "%s%s", SheltomName2[pSkill->Point - 1], sSkill_Info[i].SkillName);
			#else
				if(pSkill->ElementIndex == 1000)
				{
					wsprintf(pSkill->Skill_Info.SkillName, "%s %s", BillingMagicForceName, sSkill_Info[i].SkillName);
				}
				else if(pSkill->ElementIndex == 2000)
				{
					wsprintf(pSkill->Skill_Info.SkillName, "%s %s %s", MagicForceName, SheltomName2[pSkill->Point - 1], sSkill_Info[i].SkillName);
				}
				else
					wsprintf(pSkill->Skill_Info.SkillName, "%s %s", SheltomName2[pSkill->Point - 1], sSkill_Info[i].SkillName);
			#endif

			}
			else
			{
				strcpy_s(pSkill->Skill_Info.SkillName, sSkill_Info[i].SkillName);
			}
			strcpy_s(pSkill->Skill_Info.SkillDoc, sSkill_Info[i].SkillDoc);
			break;

		}

	}

	return TRUE;
}

int cSKILL::CheckContinueIcon()
{
	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag)
		{
			if(!ContinueSkill[i].MatIcon)
			{
				for(int j = 0; j < SIN_MAX_SKILL; j++)
				{
					if(ContinueSkill[i].CODE == sSkill[j].CODE)
					{
						if(sSkill[j].MatIcon)
						{
							ContinueSkill[i].MatIcon = sSkill[j].MatIcon;
							LegacySinSkillPolicy::OnContinueIconResolved(i, &ContinueSkill[i]);
							return TRUE;
						}
					}
				}
			}

			if(LegacySinSkillPolicy::ResolveTier5ContinueIcon(i, &ContinueSkill[i]))
				return TRUE;
		}
	}
	return FALSE;
}

int cSKILL::CancelContinueSkill(DWORD CODE)
{

	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(ContinueSkill[i].Flag)
		{
			if(ContinueSkill[i].CODE == CODE)
			{
				memset(&ContinueSkill[i], 0, sizeof(sSKILL));
				cInvenTory.SetItemToChar();
				break;
			}
		}
	}
	return FALSE;
}

int cSKILL::SkillShortKeyInit()
{
	for(int j = 1; j < SIN_MAX_USE_SKILL; j++)
	{
		sinSkill.UseSkill[j].ShortKey = 0;
	}
	return TRUE;
}

int cSKILL::CheckMaturedSkill(int ChangeJob)
{
	int matured_skill_count = 0;

	switch(ChangeJob)
	{
		case 1:
		for(int j = 1; j < 5; j++)
		{
			if(sinSkill.UseSkill[j].UseSkillCount < 10000 && sinSkill.UseSkill[j].Skill_Info.USECODE != SIN_SKILL_USE_NOT &&
			   sinSkill.UseSkill[j].UseSkillMastery)
			{
				matured_skill_count++;
			}
		}
		break;
		case 2:
		for(int j = 5; j < 9; j++)
		{
			if(sinSkill.UseSkill[j].UseSkillCount < 10000 && sinSkill.UseSkill[j].Skill_Info.USECODE != SIN_SKILL_USE_NOT &&
			   sinSkill.UseSkill[j].UseSkillMastery)
			{
				matured_skill_count++;
			}
		}
		break;
		case 3:
		for(int j = 9; j < 13; j++)
		{
			if(sinSkill.UseSkill[j].UseSkillCount < 10000 && sinSkill.UseSkill[j].Skill_Info.USECODE != SIN_SKILL_USE_NOT &&
			   sinSkill.UseSkill[j].UseSkillMastery)
			{
				matured_skill_count++;
			}
		}
		break;
	}

	if(matured_skill_count <= 4 && matured_skill_count > 0)
		return TRUE;
	else
		return FALSE;
}
