int smCharDecode(char *szCharFile, smCHAR_INFO *lpCharInfo, smCHAR_MONSTER_INFO	*lpMonInfo, char *lpDialogMessage, size_t DialogMessageSize)
{
	// MINGW_CHARINF_CP949_TOKEN_FIX_V1 - legacy CP949 literals restored as ASCII hex escapes

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


		if(lstrcmp(decode, "\x2A\xC0\xCC\xB8\xA7") == 0)
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

		if(!NextFlag && lstrcmp(decode, "\x2A\xB8\xF0\xBE\xE7\xC6\xC4\xC0\xCF") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);



			strcpy_s(lpCharInfo->szModelName, strBuff);
			NextFlag = TRUE;
		}



		DialogFlag = 0;

		if(!NextFlag && lpDialogMessage && lstrcmp(decode, "\x2A\xB4\xEB\xC8\xAD") == 0)
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


		if(!NextFlag && lstrcmp(decode, "\x2A\xBC\xD3\xBC\xBA") == 0)
		{
			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			if(lstrcmp(strBuff, "\xC0\xFB") == 0)
				lpCharInfo->State = TRUE;
			else
				lpCharInfo->State = FALSE;

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xC5\xA9\xB1\xE2") == 0)
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


		if(!NextFlag && (lstrcmp(decode, "\x2A\xBC\xD2\xB8\xAE") == 0 || lstrcmp(decode, "\x2A\xC8\xBF\xB0\xFA\xC0\xBD") == 0))
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



		if(!NextFlag && lstrcmp(decode, "\x2A\xB7\xB9\xBA\xA7") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Level = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xB5\xCE\xB8\xF1") == 0)
		{

			lpCharInfo->wPlayClass[0] = MONSTER_CLASS_BOSS;
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xB0\xE8\xB1\xDE") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->wPlayClass[0] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xB8\xF0\xB5\xA8\xC5\xA9\xB1\xE2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->wPlayClass[1] = (short)(atof(strBuff)*fONE);
			if(lpCharInfo->wPlayClass[1] == fONE) lpCharInfo->wPlayClass[1] = 0;
			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xCC\xB5\xBF\xB7\xC2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Move_Speed = ConvMoveSpeed((float)atof(strBuff));
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xB0\xF8\xB0\xDD\xB7\xC2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Damage[0] = atoi(strBuff);
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Damage[1] = atoi(strBuff);

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xB0\xF8\xB0\xDD\xBC\xD3\xB5\xB5") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Speed = (int)(atof(strBuff)*fONE);
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Speed = (int)(atof(strBuff)*fONE);

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xB0\xF8\xB0\xDD\xB9\xFC\xC0\xA7") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Shooting_Range = atoi(strBuff)*fONE;
			NextFlag = TRUE;
		}




		if(!NextFlag && lstrcmp(decode, "\x2A\xB8\xED\xC1\xDF\xB7\xC2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Attack_Rating = atoi(strBuff);

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xE6\xBE\xEE\xB7\xC2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Defence = atoi(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xC8\xED\xBC\xF6\xC0\xB2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Absorption = atoi(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xBA\xED\xB7\xB0\xC0\xB2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Chance_Block = atoi(strBuff);

			NextFlag = TRUE;
		}


		if(!NextFlag && (lstrcmp(decode, "\x2A\xBB\xFD\xB8\xED\xB7\xC2") == 0 || lstrcmp(decode, "\x2A\xB6\xF3\xC0\xCC\xC7\xC1") == 0))
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Life[0] = atoi(strBuff);
			lpCharInfo->Life[1] = lpCharInfo->Life[0];

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xBB\xFD\xC3\xBC") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_BIONIC] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xB0") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_WATER] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xF8\xB0\xB3") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_LIGHTING] = (short)atoi(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xF3\xC0\xBD") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_ICE] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xD9\xB6\xF7") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_WIND] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xC1\xF6\xB5\xBF\xB7\xC2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_EARTH] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xBA\xD2") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_FIRE] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "\x2A\xB5\xB6") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Resistance[sITEMINFO_POISON] = (short)atof(strBuff);
			NextFlag = TRUE;
		}








		if(!NextFlag && lstrcmp(decode, "\x2A\xBD\xC3\xBE\xDF") == 0)
		{
			p = local_GetWord(strBuff, p);
			lpCharInfo->Sight = atoi(strBuff);
			if(lpMonInfo) lpMonInfo->Real_Sight = lpCharInfo->Sight;

			lpCharInfo->Sight *= lpCharInfo->Sight;
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xC8\xAD\xB8\xE9\xBA\xB8\xC1\xA4") == 0)
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


			if(!NextFlag && lstrcmp(decode, "\x2A\xBF\xB9\xBA\xF1\xB8\xF0\xB5\xA8") == 0)
			{
				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				strcpy_s(lpCharInfo->szModelName2 + 1, sizeof(lpCharInfo->szModelName2) - 1, strBuff);
				lpMonInfo->UseEventModel = TRUE;
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xB1\xE2\xBC\xFA\xB0\xF8\xB0\xDD\xB7\xC2") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillDamage[0] = atoi(strBuff);
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillDamage[1] = atoi(strBuff);

				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xB1\xE2\xBC\xFA\xB0\xF8\xB0\xDD\xB0\xC5\xB8\xAE") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillDistance = atoi(strBuff);

				NextFlag = TRUE;
			}



			if(!NextFlag && lstrcmp(decode, "\x2A\xB1\xE2\xBC\xFA\xB0\xF8\xB0\xDD\xB9\xFC\xC0\xA7") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillRange = atoi(strBuff);

				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xB1\xE2\xBC\xFA\xB0\xF8\xB0\xDD\xB7\xFC") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillRating = atoi(strBuff);

				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xFA\xC1\xD6\xB1\xE2\xBC\xFA") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SkillCurse = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xCC\xB5\xBF\xC5\xB8\xC0\xD4") == 0)
			{
				p = local_GetWord(strBuff, p);

				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xCC\xB5\xBF\xB9\xFC\xC0\xA7") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->MoveRange = (int)(atof(strBuff)*fONE);
				NextFlag = TRUE;
			}



			if(!NextFlag && lstrcmp(decode, "\x2A\xC8\xB0\xB5\xBF\xBD\xC3\xB0\xA3") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->ActiveHour = 0;
				if(lstrcmpi(strBuff, "\xB3\xB7") == 0)
					lpMonInfo->ActiveHour = 1;
				if(lstrcmpi(strBuff, "\xB9\xE3") == 0)
					lpMonInfo->ActiveHour = -1;
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xC1\xB6\xC1\xF7") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->GenerateGroup[0] = atoi(strBuff);
				p = local_GetWord(strBuff, p);
				lpMonInfo->GenerateGroup[1] = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xC1\xF6\xB4\xC9") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->IQ = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xB1\xB8\xBA\xB0\xC4\xDA\xB5\xE5") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->ClassCode = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && (lstrcmp(decode, "\x2A\xBD\xBA\xC5\xCF\xC0\xB2") == 0 || lstrcmp(decode, "\x2A\xBD\xBA\xC5\xCF\xB7\xFC") == 0))
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->DamageStunPers = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xC7\xB0\xBC\xBA") == 0)
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


			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xCC\xBA\xA5\xC6\xAE\xC4\xDA\xB5\xE5") == 0)
			{
				p = local_GetWord(strBuff, p);

				lpMonInfo->EventCode = atoi(strBuff);
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xCC\xBA\xA5\xC6\xAE\xC1\xA4\xBA\xB8") == 0)
			{
				p = local_GetWord(strBuff, p);

				lpMonInfo->EventInfo = atoi(strBuff);
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xCC\xBA\xA5\xC6\xAE\xBE\xC6\xC0\xCC\xC5\xDB") == 0)
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



			if(!NextFlag && lstrcmp(decode, "\x2A\xC6\xAF\xBC\xF6\xB0\xF8\xB0\xDD\xB7\xFC") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->SpAttackPercetage = ConvPercent8(atoi(strBuff));
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xF0\xB5\xA5\xB5\xE5") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(lstrcmp(strBuff, "\xC0\xAF") == 0 || lstrcmp(strBuff, "\xC0\xD6\xC0\xBD") == 0)
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

			if(!NextFlag && lstrcmp(decode, "\x2A\xB8\xF3\xBD\xBA\xC5\xCD\xC1\xBE\xC1\xB7") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
				{

					lpCharInfo->Brood = smCHAR_MONSTER_NORMAL;

					if(lstrcmp(strBuff, "\xBE\xF0\xB5\xA5\xB5\xE5") == 0)
					{
						lpCharInfo->Brood = smCHAR_MONSTER_UNDEAD;
					}
					else if(lstrcmp(strBuff, "\xB9\xC2\xC5\xCF\xC6\xAE") == 0)
					{
						lpCharInfo->Brood = smCHAR_MONSTER_MUTANT;
					}
					else if(lstrcmp(strBuff, "\xB5\xF0\xB8\xD5") == 0)
					{
						lpCharInfo->Brood = smCHAR_MONSTER_DEMON;
					}
					else if(lstrcmp(strBuff, "\xB8\xDE\xC4\xAB\xB4\xD0") == 0)
					{
						lpCharInfo->Brood = smCHAR_MONSTER_MECHANIC;
					}
				}
			}



			if(!NextFlag && lstrcmp(decode, "\x2A\xB0\xE6\xC7\xE8\xC4\xA1") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->GetExp = atoi(strBuff);
				lpCharInfo->Exp = lpMonInfo->GetExp;
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xB0\xBE\xE0\xBA\xB8\xC0\xAF\xBC\xF6") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->PotionCount = atoi(strBuff);
				NextFlag = TRUE;
			}


			if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xB0\xBE\xE0\xBA\xB8\xC0\xAF\xB7\xFC") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->PotionPercent = atoi(strBuff);
				NextFlag = TRUE;
			}




			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xB8\xF0\xB5\xCE") == 0)
			{
				lpMonInfo->AllSeeItem = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xC4\xAB\xBF\xEE\xC5\xCD") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->FallItemMax = atoi(strBuff);
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xC3\xDF\xB0\xA1\xBE\xC6\xC0\xCC\xC5\xDB") == 0)
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

			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB") == 0)
			{
				p = local_GetWord(strBuff, p);
				percent = atoi(strBuff);

				lpMonInfo->FallItemPerMax += percent;

				p = local_GetWord(strBuff, p);
				if(lstrcmp(strBuff, "\xBE\xF8\xC0\xBD") == 0)
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
					if(lstrcmp(strBuff, "\xB5\xB7") == 0)
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


			if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xAB\xB1\xE2\xC6\xC7\xB8\xC5") == 0)
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

			if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xE6\xBE\xEE\xB1\xB8\xC6\xC7\xB8\xC5") == 0)
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

			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xE2\xC8\xAD\xC6\xC7\xB8\xC5") == 0)
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

			if(!NextFlag && lstrcmp(decode, "\x2A\xBD\xBA\xC5\xB3\xBC\xF6\xB7\xC3") == 0)
			{
				lpMonInfo->SkillMaster = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xC1\xF7\xBE\xF7\xC0\xFC\xC8\xAF") == 0)
			{
				lpMonInfo->SkillChangeJob = TRUE;

				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->SkillChangeJob = atoi(strBuff);

				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xCC\xBA\xA5\xC6\xAE\xB8\xC5\xC7\xA5\xBC\xD2") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->EventNPC = atoi(strBuff);
				else
					lpMonInfo->EventNPC = 1;

				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xBA\xB8\xB0\xFC") == 0)
			{
				lpMonInfo->WareHouseMaster = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xC1\xB6\xC7\xD5") == 0)
			{
				lpMonInfo->ItemMix = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xC1\xA6\xB7\xC3") == 0)
			{
				lpMonInfo->Smelting = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xC1\xA6\xC0\xDB") == 0)
			{
				lpMonInfo->Manufacture = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xBF\xAC\xB1\xDD") == 0)
			{
				lpMonInfo->ItemMix = 200;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xBF\xA1\xC0\xCC\xC2\xA1") == 0)
			{
				lpMonInfo->ItemAging = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xB9\xCD\xBD\xBA\xC3\xB3\xB8\xAE\xBC\xC2") == 0)
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
						
			if(!NextFlag && lstrcmp(decode, "\x2A\xB8\xF0\xB1\xDD\xC7\xD4") == 0)
			{
				lpMonInfo->CollectMoney = TRUE;
				NextFlag = TRUE;
			}
#ifdef _LANGUAGE_VEITNAM
			if(!NextFlag && lstrcmp(decode, "\x2A\xB2\xCE\xC0\xCC\xC1\xF6\xB7\xD5") == 0)
			{
				lpMonInfo->WowEvent = TRUE;
				NextFlag = TRUE;
			}
#endif
			if(!NextFlag && lstrcmp(decode, "\x2A\xB0\xE6\xC7\xB0\xC3\xDF\xC3\xB7") == 0)
			{
				lpMonInfo->EventGift = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xC5\xAC\xB7\xA3\xB1\xE2\xB4\xC9") == 0)
			{
				lpMonInfo->ClanNPC = TRUE;
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xB0\xE6\xC7\xB0\xB9\xE8\xB4\xDE") == 0)
			{
				lpMonInfo->GiftExpress = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xC0\xAE\xC4\xF9\xBD\xBA\xC6\xAE") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->WingQuestNpc = atoi(strBuff);
				else
					lpMonInfo->WingQuestNpc = 1;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xC4\xF9\xBD\xBA\xC6\xAE\xC0\xCC\xBA\xA5\xC6\xAE") == 0)
			{

				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->WingQuestNpc = atoi(strBuff);
				else
					lpMonInfo->WingQuestNpc = 2;

				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xBA\xB0\xC6\xF7\xC0\xCE\xC6\xAE\xC0\xFB\xB8\xB3") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->StarPointNpc = atoi(strBuff);
				else
					lpMonInfo->StarPointNpc = 20;

				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xB1\xE2\xBA\xCE\xC7\xD4") == 0)
			{
				lpMonInfo->GiveMoneyNpc = TRUE;
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xC5\xDA\xB7\xB9\xC6\xF7\xC6\xAE") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->TelePortNpc = atoi(strBuff);
				else
					lpMonInfo->TelePortNpc = 1;

				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xBA\xED\xB7\xB9\xBD\xBA\xC4\xB3\xBD\xBD") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->BlessCastleNPC = atoi(strBuff);
				else
					lpMonInfo->BlessCastleNPC = 1;

				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xBC\xB3\xB9\xAE\xC1\xB6\xBB\xE7") == 0)
			{
				p = local_GetWord(strBuff, p);
				if(strBuff[0])
					lpMonInfo->PollingNpc = atoi(strBuff);
				else
					lpMonInfo->PollingNpc = 1;

				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xB5\xBF\xBF\xB5\xBB\xF3\xC1\xA6\xB8\xF1") == 0)
			{
				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				if(strBuff[0])
				{
					lpMonInfo->szMediaPlayNPC_Title = SetNPCMsg(strBuff);
				}
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xB5\xBF\xBF\xB5\xBB\xF3\xB0\xE6\xB7\xCE") == 0)
			{
				pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
				if(strBuff[0])
				{
					lpMonInfo->szMediaPlayNPC_Path = SetNPCMsg(strBuff);
				}
				NextFlag = TRUE;
			}

			if(!NextFlag && lstrcmp(decode, "\x2A\xC3\xE2\xC7\xF6\xB0\xA3\xB0\xDD") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->OpenCount[0] = atoi(strBuff);
				p = local_GetWord(strBuff, p);
				lpMonInfo->OpenCount[1] = atoi(strBuff);
				NextFlag = TRUE;
			}
			if(!NextFlag && lstrcmp(decode, "\x2A\xC4\xF9\xBD\xBA\xC6\xAE\xC4\xDA\xB5\xE5") == 0)
			{
				p = local_GetWord(strBuff, p);
				lpMonInfo->QuestCode = atoi(strBuff);
				p = local_GetWord(strBuff, p);
				lpMonInfo->QuestParam = atoi(strBuff);
				NextFlag = TRUE;
			}
		}


		if(!NextFlag && lstrcmp(decode, "\x2A\xBF\xAC\xB0\xE1\xC6\xC4\xC0\xCF") == 0)
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