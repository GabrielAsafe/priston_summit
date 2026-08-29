int smCharDecode(char *szCharFile, smCHAR_INFO *lpCharInfo, smCHAR_MONSTER_INFO	*lpMonInfo, char *lpDialogMessage, size_t DialogMessageSize)
{

	FILE	*fp;

	char strBuff[256];
	char szFile[128];
	char *p;
	char *pb;
	int NextFlag;
	int cnt;
	int	percent;
	int	DialogFlag;

	DWORD	dwItemCode[32];
	int		ItemCodeCnt;


	strcpy_s(szFile, szCharFile);

	fp = fopen(szFile, "rb");
	if(fp == NULL) return FALSE;




	lpCharInfo->szModelName2[0] = 0;
	lpCharInfo->wPlayClass[1] = 0;
	lpCharInfo->ArrowPosi[0] = 0;
	lpCharInfo->ArrowPosi[1] = 0;

	if(lpMonInfo)
	{
		ZeroMemory(lpMonInfo, sizeof(smCHAR_MONSTER_INFO));
		lpMonInfo->MoveRange = 64 * fONE;
		lpMonInfo->FallItemCount = 0;
		lpMonInfo->FallItemMax = 0;
		lpMonInfo->FallItemPerMax = 0;

		lpMonInfo->DamageStunPers = 100;
		lpMonInfo->UseEventModel = 0;
	}

	while(!feof(fp))
	{
		if(fgets(line, 255, fp) == NULL)	break;

		p = local_GetWord(decode, line);

		NextFlag = 0;


		if(lstrcmp(decode, "*�̸�") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			strcpy_s(lpCharInfo->szName, strBuff);
			NextFlag = TRUE;
		}


#ifdef _LANGUAGE_BRAZIL
		if(lstrcmpi(decode, "*J_NAME") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			if(lpMonInfo)
				strcpy_s(lpMonInfo->szName, strBuff);
			else
				strcpy_s(lpCharInfo->szName, strBuff);

			NextFlag = TRUE;
		}
#endif

		if(!NextFlag && lstrcmp(decode, "*�������") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);



			strcpy_s(lpCharInfo->szModelName, strBuff);
			NextFlag = TRUE;
		}



		DialogFlag = 0;

		if(!NextFlag && lpDialogMessage && lstrcmp(decode, "*��ȭ") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			strcpy_s(lpDialogMessage, DialogMessageSize, strBuff);
			NextFlag = TRUE;
			DialogFlag++;
		}


#ifdef _LANGUAGE_BRAZIL
		if(lpDialogMessage && lstrcmpi(decode, "*J_CHAT") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			strcpy_s(lpDialogMessage, DialogMessageSize, strBuff);
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif


		if(DialogFlag && lpMonInfo)
		{
			if(lpMonInfo->NpcMsgCount < NPC_MESSAGE_MAX)
			{

				lpMonInfo->lpNpcMessage[lpMonInfo->NpcMsgCount++] = SetNPCMsg(lpDialogMessage);



			}
		}


		if(!NextFlag && lstrcmp(decode, "*�Ӽ�") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			if(lstrcmp(strBuff, "��") == 0)
				lpCharInfo->State = TRUE;
			else
				lpCharInfo->State = FALSE;

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*ũ��") == 0)
		{
			p = local_GetWord(strBuff, p);
			cnt = 0;

			lpCharInfo->SizeLevel = -1;

			while(1)
			{
				if(szCharSizeCodeName[cnt][0] == 0) break;
				if(lstrcmp(szCharSizeCodeName[cnt], strBuff) == 0)
				{
					lpCharInfo->SizeLevel = cnt;
				}
				cnt++;
			}
			NextFlag = TRUE;
		}


		if(!NextFlag && (lstrcmp(decode, "*�Ҹ�") == 0 || lstrcmp(decode, "*ȿ����") == 0))
		{
			p = local_GetWord(strBuff, p);
			cnt = 0;

			while(1)
			{
				if(dwCharSoundCode[cnt].dwCode == 0) break;
				if(lstrcmpi(dwCharSoundCode[cnt].szCodeName, strBuff) == 0)
				{
					lpCharInfo->dwCharSoundCode = dwCharSoundCode[cnt].dwCode;
				}
				cnt++;
			}
			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmp(decode, "*����") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Level = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*�θ�") == 0)
		{

			lpCharInfo->wPlayClass[0] = MONSTER_CLASS_BOSS;
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*���") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->wPlayClass[0] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*��ũ��") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->wPlayClass[1] = (short)(atof(strBuff)*fONE);
			if(lpCharInfo->wPlayClass[1] == fONE) lpCharInfo->wPlayClass[1] = 0;
			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmp(decode, "*�̵���") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Move_Speed = ConvMoveSpeed((float)atof(strBuff));
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*���ݷ�") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Damage[0] = atoi(strBuff);
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Damage[1] = atoi(strBuff);

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*���ݼӵ�") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Speed = (int)(atof(strBuff)*fONE);
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Speed = (int)(atof(strBuff)*fONE);

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*���ݹ���") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Shooting_Range = atoi(strBuff)*fONE;
			NextFlag = TRUE;
		}




		if(!NextFlag && lstrcmp(decode, "*���߷�") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Rating = atoi(strBuff);

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*����") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Defence = atoi(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*�����") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Absorption = atoi(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*�����") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Chance_Block = atoi(strBuff);

			NextFlag = TRUE;
		}


		if(!NextFlag && (lstrcmp(decode, "*�����") == 0 || lstrcmp(decode, "*������") == 0))
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Life[0] = atoi(strBuff);
			lpCharInfo->Life[1] = lpCharInfo->Life[0];

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*��ü") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_BIONIC] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*��") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_WATER] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*����") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_LIGHTING] = (short)atoi(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*����") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_ICE] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*�ٶ�") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_WIND] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*������") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_EARTH] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*��") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_FIRE] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*��") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_POISON] = (short)atof(strBuff);
			NextFlag = TRUE;
		}








		if(!NextFlag && lstrcmp(decode, "*�þ�") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Sight = atoi(strBuff);
			if(lpMonInfo) lpMonInfo->Real_Sight = lpCharInfo->Sight;

			lpCharInfo->Sight *= lpCharInfo->Sight;
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*ȭ�麸��") == 0)
		{
			p = local_GetWord(strBuff, p);
			if(strBuff[0])
				lpCharInfo->ArrowPosi[0] = atoi(strBuff);

			p = local_GetWord(strBuff, p);
			if(strBuff[0])
				lpCharInfo->ArrowPosi[1] = atoi(strBuff);

		}

		if(lpMonInfo && !NextFlag)
		{


			if(!NextFlag && lstrcmp(decode, "*�����") == 0)
			{
				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				strcpy_s(lpCharInfo->szModelName2 + 1, sizeof(lpCharInfo->szModelName2) - 1, strBuff);
				lpMonInfo->UseEventModel = TRUE;
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*������ݷ�") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillDamage[0] = atoi(strBuff);
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillDamage[1] = atoi(strBuff);

				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*������ݰŸ�") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillDistance = atoi(strBuff);

				NextFlag = TRUE;
			}



			if(!NextFlag && lstrcmp(decode, "*������ݹ���") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillRange = atoi(strBuff);

				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*������ݷ�") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillRating = atoi(strBuff);

				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*���ֱ��") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillCurse = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*�̵�Ÿ��") == 0)
			{
				p = local_GetWord(strBuff, p);

				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*�̵�����") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->MoveRange = (int)(atof(strBuff)*fONE);
				NextFlag = TRUE;
			}



			if(!NextFlag && lstrcmp(decode, "*Ȱ���ð�") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->ActiveHour = 0;
				if(lstrcmpi(strBuff, "��") == 0)
					lpMonInfo->ActiveHour = 1;
				if(lstrcmpi(strBuff, "��") == 0)
					lpMonInfo->ActiveHour = -1;
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*����") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->GenerateGroup[0] = atoi(strBuff);
				p = local_GetWord(strBuff, p);
				lpMonInfo->GenerateGroup[1] = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*����") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->IQ = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*�����ڵ�") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->ClassCode = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && (lstrcmp(decode, "*������") == 0 || lstrcmp(decode, "*���Ϸ�") == 0))
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->DamageStunPers = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*ǰ��") == 0)
			{
				lpMonInfo->Nature = smCHAR_MONSTER_NATURAL;


				p = local_GetWord(strBuff, p);
				if(lstrcmpi(strBuff, "good") == 0)
				{
					lpMonInfo->Nature = smCHAR_MONSTER_GOOD;

				}
				if(lstrcmpi(strBuff, "evil") == 0)
				{
					lpMonInfo->Nature = smCHAR_MONSTER_EVIL;

				}
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*�̺�Ʈ�ڵ�") == 0)
			{
				p = local_GetWord(strBuff, p);

				lpMonInfo->EventCode = atoi(strBuff);
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*�̺�Ʈ����") == 0)
			{
				p = local_GetWord(strBuff, p);

				lpMonInfo->EventInfo = atoi(strBuff);
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*�̺�Ʈ������") == 0)
			{
				p = local_GetWord(strBuff, p);


				for(cnt = 0; cnt < MAX_ITEM; cnt++)
				{
					if(lstrcmpi(strBuff, sItem[cnt].LastCategory) == 0)
					{
						lpMonInfo->dwEvnetItem = sItem[cnt].CODE;
						break;
					}
				}
				NextFlag = TRUE;
			}



			if(!NextFlag && lstrcmp(decode, "*Ư�����ݷ�") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SpAttackPercetage = ConvPercent8(atoi(strBuff));
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*�𵥵�") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(lstrcmp(strBuff, "��") == 0 || lstrcmp(strBuff, "����") == 0)
				{
					lpMonInfo->Undead = TRUE;
					lpCharInfo->Brood = smCHAR_MONSTER_UNDEAD;
				}
				else
				{
					lpMonInfo->Undead = FALSE;
					lpCharInfo->Brood = smCHAR_MONSTER_NORMAL;
				}

				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*��������") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
				{

					lpCharInfo->Brood = smCHAR_MONSTER_NORMAL;

					if(lstrcmp(strBuff, "�𵥵�") == 0)
					{
						lpCharInfo->Brood = smCHAR_MONSTER_UNDEAD;
					}
					else if(lstrcmp(strBuff, "����Ʈ") == 0)
					{
						lpCharInfo->Brood = smCHAR_MONSTER_MUTANT;
					}
					else if(lstrcmp(strBuff, "���") == 0)
					{
						lpCharInfo->Brood = smCHAR_MONSTER_DEMON;
					}
					else if(lstrcmp(strBuff, "��ī��") == 0)
					{
						lpCharInfo->Brood = smCHAR_MONSTER_MECHANIC;
					}
				}
			}



			if(!NextFlag && lstrcmp(decode, "*����ġ") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->GetExp = atoi(strBuff);
				lpCharInfo->Exp = lpMonInfo->GetExp;
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*���ຸ����") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->PotionCount = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*���ຸ����") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->PotionPercent = atoi(strBuff);
				NextFlag = TRUE;
			}




			if(!NextFlag && lstrcmp(decode, "*�����۸��") == 0)
			{
				lpMonInfo->AllSeeItem = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*������ī����") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->FallItemMax = atoi(strBuff);
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*�߰�������") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->FallItems_Plus[lpMonInfo->FallItemPlusCount].Percentage = atoi(strBuff);

				p = local_GetWord(strBuff, p);
				if(strBuff[0] && lpMonInfo->FallItemPlusCount < FALLITEM2_MAX)
				{
					for(cnt = 0; cnt < MAX_ITEM; cnt++)
					{
						if(lstrcmpi(strBuff, sItem[cnt].LastCategory) == 0)
						{
							lpMonInfo->FallItems_Plus[lpMonInfo->FallItemPlusCount].dwItemCode = sItem[cnt].CODE;
							lpMonInfo->FallItemPlusCount++;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*������") == 0)
			{
				p = local_GetWord(strBuff, p);
				percent = atoi(strBuff);

				lpMonInfo->FallItemPerMax += percent;

				p = local_GetWord(strBuff, p);
				if(lstrcmp(strBuff, "����") == 0)
				{

					if(rsServerConfig.TestSeverMode)
					{
						lpMonInfo->FallItemPerMax -= percent;
						percent /= 2;
						lpMonInfo->FallItemPerMax += percent;
					}


					lpMonInfo->FallItems[lpMonInfo->FallItemCount].dwItemCode = 0;
					lpMonInfo->FallItems[lpMonInfo->FallItemCount].Percentage = percent;
					lpMonInfo->FallItemCount++;
				}
				else
				{
					if(lstrcmp(strBuff, "��") == 0)
					{

						lpMonInfo->FallItems[lpMonInfo->FallItemCount].dwItemCode = sinGG1 | sin01;
						p = local_GetWord(strBuff, p);
						lpMonInfo->FallItems[lpMonInfo->FallItemCount].sPrice[0] = atoi(strBuff);
						p = local_GetWord(strBuff, p);
						lpMonInfo->FallItems[lpMonInfo->FallItemCount].sPrice[1] = atoi(strBuff);
						lpMonInfo->FallItems[lpMonInfo->FallItemCount].Percentage = percent;
						lpMonInfo->FallItemCount++;
					}
					else
					{

						ItemCodeCnt = 0;

						while(1)
						{
							if(strBuff[0])
							{

								for(cnt = 0; cnt < MAX_ITEM; cnt++)
								{
									if(lstrcmpi(strBuff, sItem[cnt].LastCategory) == 0)
									{
										dwItemCode[ItemCodeCnt++] = sItem[cnt].CODE;
										break;
									}
								}
							}
							else
							{
								break;
							}

							strBuff[0] = 0;
							if(p[0] == 0) break;
							p = local_GetWord(strBuff, p);
						}

						if(ItemCodeCnt > 0)
						{

							percent = percent / ItemCodeCnt;
							for(cnt = 0; cnt < ItemCodeCnt; cnt++)
							{
								lpMonInfo->FallItems[lpMonInfo->FallItemCount].dwItemCode = dwItemCode[cnt];
								lpMonInfo->FallItems[lpMonInfo->FallItemCount].Percentage = percent;
								lpMonInfo->FallItemCount++;
							}
						}
					}
				}
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "*�����Ǹ�") == 0)
			{
				while(1)
				{
					pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
					if(strBuff[0] == 0) break;
					if(lpMonInfo->SellAttackItemCount >= 32) break;
					for(cnt = 0; cnt < MAX_ITEM; cnt++)
					{
						if(lstrcmpi(strBuff, sItem[cnt].LastCategory) == 0)
						{
							lpMonInfo->SellAttackItem[lpMonInfo->SellAttackItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*���Ǹ�") == 0)
			{
				while(1)
				{
					pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
					if(strBuff[0] == 0) break;
					if(lpMonInfo->SellDefenceItemCount >= 32) break;
					for(cnt = 0; cnt < MAX_ITEM; cnt++)
					{
						if(lstrcmpi(strBuff, sItem[cnt].LastCategory) == 0)
						{
							lpMonInfo->SellDefenceItem[lpMonInfo->SellDefenceItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*��ȭ�Ǹ�") == 0)
			{
				while(1)
				{
					pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
					if(strBuff[0] == 0) break;
					if(lpMonInfo->SellEtcItemCount >= 32) break;
					for(cnt = 0; cnt < MAX_ITEM; cnt++)
					{
						if(lstrcmpi(strBuff, sItem[cnt].LastCategory) == 0)
						{
							lpMonInfo->SellEtcItem[lpMonInfo->SellEtcItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*��ų����") == 0)
			{
				lpMonInfo->SkillMaster = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*������ȯ") == 0)
			{
				lpMonInfo->SkillChangeJob = TRUE;

				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->SkillChangeJob = atoi(strBuff);

				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*�̺�Ʈ��ǥ��") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->EventNPC = atoi(strBuff);
				else
					lpMonInfo->EventNPC = 1;

				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*�����ۺ���") == 0)
			{
				lpMonInfo->WareHouseMaster = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*����������") == 0)
			{
				lpMonInfo->ItemMix = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*����������") == 0)
			{
				lpMonInfo->Smelting = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*����������") == 0)
			{
				lpMonInfo->Manufacture = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*�����ۿ���") == 0)
			{
				lpMonInfo->ItemMix = 200;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*�����ۿ���¡") == 0)
			{
				lpMonInfo->ItemAging = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*�ͽ�ó����") == 0)
			{
				lpMonInfo->MixtureReset = TRUE;
				NextFlag = TRUE;
			}

			if (!NextFlag && lstrcmp(decode, "*ClanCustom") == 0)
			{
				lpMonInfo->ClanCustom = TRUE;
				NextFlag = TRUE;
			}

			if (!NextFlag && lstrcmp(decode, "*NPCinfo") == 0)
			{
				lpMonInfo->NPCinfo = TRUE;
				NextFlag = TRUE;
			}

			if (!NextFlag && lstrcmp(decode, "*Rank") == 0)
			{
				lpMonInfo->Rank = TRUE;
				NextFlag = TRUE;
			}	

			if (!NextFlag && lstrcmp(decode, "*BattlePass") == 0)//sergio
			{
				lpMonInfo->BattlePass = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*SkinChange") == 0)
			{
				lpMonInfo->SkinChange = TRUE;
				NextFlag = TRUE;
			}
			
			if (!NextFlag && lstrcmp(decode, "*Shop") == 0)
			{
				lpMonInfo->ItemShop = TRUE;
				NextFlag = TRUE;
			}
			if (!NextFlag && lstrcmp(decode, "*TShop") == 0)
			{
				lpMonInfo->TimeShop = TRUE;
				NextFlag = TRUE;
			}	
			if (!NextFlag && lstrcmp(decode, "*Ranking") == 0)
			{
				lpMonInfo->Ranking = TRUE;
				NextFlag = TRUE;
			}
						
			if(!NextFlag && lstrcmp(decode, "*�����") == 0)
			{
				lpMonInfo->CollectMoney = TRUE;
				NextFlag = TRUE;
			}
#ifdef _LANGUAGE_VEITNAM
			if(!NextFlag && lstrcmp(decode, "*��������") == 0)
			{
				lpMonInfo->WowEvent = TRUE;
				NextFlag = TRUE;
			}
#endif
			if(!NextFlag && lstrcmp(decode, "*��ǰ��÷") == 0)
			{
				lpMonInfo->EventGift = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*Ŭ�����") == 0)
			{
				lpMonInfo->ClanNPC = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*��ǰ���") == 0)
			{
				lpMonInfo->GiftExpress = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*������Ʈ") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->WingQuestNpc = atoi(strBuff);
				else
					lpMonInfo->WingQuestNpc = 1;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*����Ʈ�̺�Ʈ") == 0)
			{

				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->WingQuestNpc = atoi(strBuff);
				else
					lpMonInfo->WingQuestNpc = 2;

				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*������Ʈ����") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->StarPointNpc = atoi(strBuff);
				else
					lpMonInfo->StarPointNpc = 20;

				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*�����") == 0)
			{
				lpMonInfo->GiveMoneyNpc = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*�ڷ���Ʈ") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->TelePortNpc = atoi(strBuff);
				else
					lpMonInfo->TelePortNpc = 1;

				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*�����ĳ��") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->BlessCastleNPC = atoi(strBuff);
				else
					lpMonInfo->BlessCastleNPC = 1;

				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*��������") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->PollingNpc = atoi(strBuff);
				else
					lpMonInfo->PollingNpc = 1;

				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*����������") == 0)
			{
				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				if(strBuff[0])
				{
					lpMonInfo->szMediaPlayNPC_Title = SetNPCMsg(strBuff);
				}
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*��������") == 0)
			{
				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				if(strBuff[0])
				{
					lpMonInfo->szMediaPlayNPC_Path = SetNPCMsg(strBuff);
				}
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "*��������") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->OpenCount[0] = atoi(strBuff);
				p = local_GetWord(strBuff, p);
				lpMonInfo->OpenCount[1] = atoi(strBuff);
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "*����Ʈ�ڵ�") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->QuestCode = atoi(strBuff);
				p = local_GetWord(strBuff, p);
				lpMonInfo->QuestParam = atoi(strBuff);
				NextFlag = TRUE;
			}
		}


		if(!NextFlag && lstrcmp(decode, "*��������") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			char szNextFile[128];
			GetDirectoryFromFile(szFile, szNextFile, sizeof(szNextFile));
			lstrcat(szNextFile, strBuff);
			NextFlag = TRUE;
			fclose(fp);

			fp = fopen(szNextFile, "rb");
			if(fp == NULL) break;
		}

	}


	if(lpMonInfo)
		lpCharInfo->lpMonInfo = lpMonInfo;
	else
		lpCharInfo->lpMonInfo = 0;


	if(fp) fclose(fp);

	return TRUE;
}
