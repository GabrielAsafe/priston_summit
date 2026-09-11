int sinThrowItemToFeild(sITEM *pItem)
{

	sITEM	sThrowItem;
	int		ThrowItemBackup = 0;

	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI2 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI3)
	{
		if (pItem->sItemInfo.PotionCount == 0)
			return FALSE;
	}



	if (cWareHouse.OpenFlag)
	{
		if (!cWareHouse.CloseWareHouse())
		{
			pItem->Flag = 0;
			return FALSE;
		}

	}

	if (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->FieldCode == QUEST_ARENA_FIELD)
	{

		if ((pItem->sItemInfo.CODE&sinITEM_MASK2) != sinFO1 && (pItem->sItemInfo.CODE&sinITEM_MASK2) != sinCH1)
			return FALSE;

		if (pItem->sItemInfo.PotionCount != 100)
			return FALSE;

	}

	if (pItem->sItemInfo.CODE != (sinGG1 | sin01) && !pItem->Flag) return FALSE;

	if (pItem->sItemInfo.PotionCount == 100)
	{


		int BlessCristalCode = -1;
		int BlessSolderTotal;

		switch (pItem->sItemInfo.CODE)
		{
		case sinGP1 | sin14:
			BlessCristalCode = 0;
			break;
		case sinGP1 | sin15:
			BlessCristalCode = 1;
			break;
		case sinGP1 | sin16:
			BlessCristalCode = 2;
			break;
		}
		if (BlessCristalCode >= 0)
		{
			if (!rsBlessCastle.CastleMode ||
				(lpCurPlayer->OnStageField >= 0 && sField[lpCurPlayer->OnStageField].FieldCode != rsCASTLE_FIELD) &&
				!lpCurPlayer->Clan_CastleMasterFlag)
			{

				cMessageBox.ShowMessageEvent(mgBlessCrystal_03);
				return FALSE;
			}

			BlessSolderTotal = rsBlessCastle.bCrystalSolderCount[0] +
				rsBlessCastle.bCrystalSolderCount[1] +
				rsBlessCastle.bCrystalSolderCount[2] +
				rsBlessCastle.bCrystalSolderCount[3];

			if (rsBlessCastle.bCrystalSolderCount[BlessCristalCode] >= srCASLTE_SOLDER_KIND_MAX ||
				BlessSolderTotal >= srCASLTE_SOLDER_CLAN_MAX)
			{

				cMessageBox.ShowMessageEvent(mgBlessCrystal_01);
				return FALSE;
			}
		}
	}




	if (!CheckItemForm(&pItem->sItemInfo))
	{
		SendSetHackUser(80);
		return TRUE;
	}





	if (cShop.OpenFlag || SkillMasterFlag || cCraftItem.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag)
	{
		if (pItem->sItemInfo.CODE == (sinGG1 | sin01)) return FALSE;
	}

	if ((pItem->sItemInfo.CODE&sinITEM_MASK1) == (sinPM1&sinITEM_MASK1))
	{

		ThrowItemBackup = 1;
		memcpy(&sThrowItem, pItem, sizeof(sITEM));
		ReformItem(&pItem->sItemInfo);
	}

	if (ThrowPutItem(pItem, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ) == TRUE)
	{
		SubInvenItemCode(&pItem->sItemInfo);
		pItem->Flag = 0;



		return TRUE;
	}

	if (ThrowItemBackup)
		memcpy(pItem, &sThrowItem, sizeof(sITEM));

	return FALSE;
}


int sinActionPotion()
{
	if (sinGetLife() > 0 && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_EAT
		&& lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
	{
		if (lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_ATTACK || lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_SKILL)
			lpCurPlayer->dwNextMotionCode = CHRMOTION_STATE_EAT;
		else
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_EAT);
		return TRUE;
	}

	return FALSE;
}


int	ActionEtherCore(sITEM	*lpItem)
{

	DWORD	dwCode = lpItem->sItemInfo.CODE;

	if (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_ROOM)
	{

		return TRUE;
	}
	if (lpCurPlayer->smCharInfo.wPlayerKilling[0])
	{

		return TRUE;
	}

	if (sinGetLife() > 0 && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
	{

		if (lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_ATTACK || lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_SKILL)
			lpCurPlayer->dwNextMotionCode = CHRMOTION_STATE_EAT;
		else
		{
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_EAT);
			StartEffect(lpCurPlayer->pX, lpCurPlayer->pY + 48 * fONE, lpCurPlayer->pZ, EFFECT_RETURN1);
			SkillPlaySound(SKILL_SOUND_LEARN, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);
		}

		UseEtherCoreCode = dwCode;



		if ((lpItem->sItemInfo.CODE&sinITEM_MASK2) == sinEC1)
		{

			SendUseItemCodeToServer(&lpItem->sItemInfo);
		}

		return TRUE;
	}

	return FALSE;
}
