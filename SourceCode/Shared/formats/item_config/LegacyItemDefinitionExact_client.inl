int DecodeItemInfo(char *szItemFile, sDEF_ITEMINFO *lpDefItem)
{

	if(summit::content::definition::CompiledDefinitionsEnabled())
	{
		if(summit::content::definition::ApplyCompiledItemDefinition(szItemFile, *lpDefItem))
			return TRUE;
		if(summit::content::definition::DefinitionStrictMode())
			return FALSE;
	}

	// Legacy OpenItem parser tokens.
	// OpenItem files use historical CP949 byte sequences.
	// Byte escapes keep this source independent of source-file encoding.
	static const char *ITEM_TOKEN_CODE =
		"\x2A\xC4\xDA\xB5\xE5";                  // CODE
	static const char *ITEM_TOKEN_DURABILITY =
		"\x2A\xB3\xBB\xB1\xB8\xB7\xC2";          // DURABILITY
	static const char *ITEM_TOKEN_WEIGHT =
		"\x2A\xB9\xAB\xB0\xD4";                  // WEIGHT
	static const char *ITEM_TOKEN_PRICE =
		"\x2A\xB0\xA1\xB0\xDD";                  // PRICE
	static const char *ITEM_TOKEN_ATTACK_DAMAGE =
		"\x2A\xB0\xF8\xB0\xDD\xB7\xC2";          // ATTACK DAMAGE
	static const char *ITEM_TOKEN_SHOOTING_RANGE =
		"\x2A\xBB\xE7\xC1\xA4\xB0\xC5\xB8\xAE"; // RANGE
	static const char *ITEM_TOKEN_ATTACK_SPEED =
		"\x2A\xB0\xF8\xB0\xDD\xBC\xD3\xB5\xB5"; // ATTACK SPEED
	static const char *ITEM_TOKEN_ATTACK_RATING =
		"\x2A\xB8\xED\xC1\xDF\xB7\xC2";          // ATTACK RATING
	static const char *ITEM_TOKEN_CRITICAL =
		"\x2A\xC5\xA9\xB8\xAE\xC6\xBC\xC4\xC3"; // CRITICAL


	FILE	*fp;
	char strBuff[128];
	char *p;
	char *pb;
	int cnt;
	int NextFlag;
	sITEMINFO	*lpItem;

	lpItem = &lpDefItem->Item;

	ZeroMemory(lpDefItem, sizeof(sDEF_ITEMINFO));

	fp = summit::content::text_resource::OpenTextResourceFile(szItemFile, "rb");
	if(fp == NULL) return FALSE;

	while(!feof(fp))
	{
		if(fgets(line, 255, fp) == NULL)
			break;

		p = fileread_GetWord(decode, line);
		NextFlag = 0;


		if(lstrcmp(decode, "*이름") == 0)
		{
			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			strcpy_s(lpItem->ItemName, strBuff);
			NextFlag = TRUE;
		}


#ifdef _LANGUAGE_BRAZIL
		if(lstrcmpi(decode, "*J_NAME") == 0)
		{
			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			strcpy_s(lpItem->ItemName, strBuff);
			NextFlag = TRUE;
		}
#endif



		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_CODE) == 0)
		{
			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			for(cnt = 0; cnt < MAX_ITEM; cnt++)
			{
				if(lstrcmpi(strBuff, sItem[cnt].LastCategory) == 0)
				{
					lpItem->CODE = sItem[cnt].CODE; break;
				}
			}

			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmp(decode, "*유니크") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			if(strBuff[0])
				lpItem->UniqueItem = atoi(strBuff);
			else
				lpItem->UniqueItem = TRUE;

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*유니크색상") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->EffectColor[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpItem->EffectColor[1] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpItem->EffectColor[2] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpItem->EffectColor[3] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpItem->EffectBlink[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			if(strBuff[0])
				lpItem->ScaleBlink[0] = (int)(atof(strBuff)*fONE);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*이펙트설정") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->DispEffect = atoi(strBuff);
			NextFlag = TRUE;
		}





		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_DURABILITY) == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sDurability[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sDurability[1] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_WEIGHT) == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Weight = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_PRICE) == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Price = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*생체") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_BIONIC][0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_BIONIC][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if(!NextFlag && lstrcmp(decode, "*대자연") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_EARTH][0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_EARTH][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if(!NextFlag && lstrcmp(decode, "*불") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_FIRE][0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_FIRE][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if(!NextFlag && lstrcmp(decode, "*냉기") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_ICE][0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_ICE][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if(!NextFlag && lstrcmp(decode, "*번개") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_LIGHTING][0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_LIGHTING][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if(!NextFlag && lstrcmp(decode, "*독") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_POISON][0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_POISON][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if(!NextFlag && lstrcmp(decode, "*물") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_WATER][0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_WATER][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if(!NextFlag && lstrcmp(decode, "*바람") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_WIND][0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sResistance[sITEMINFO_WIND][1] = atoi(strBuff);
			NextFlag = TRUE;
		}






		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_ATTACK_DAMAGE) == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sDamage[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sDamage[1] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sDamage[2] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sDamage[3] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_SHOOTING_RANGE) == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Shooting_Range = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_ATTACK_SPEED) == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Attack_Speed = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_ATTACK_RATING) == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sAttack_Rating[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sAttack_Rating[1] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, ITEM_TOKEN_CRITICAL) == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Critical_Hit = atoi(strBuff);
			NextFlag = TRUE;
		}





		if(!NextFlag && lstrcmp(decode, "*흡수력") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fAbsorb[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fAbsorb[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*방어력") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sDefence[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sDefence[1] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*흡수력") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fBlock_Rating[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fBlock_Rating[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmp(decode, "*이동속도") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fSpeed[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fSpeed[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmp(decode, "*보유공간") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Potion_Space = atoi(strBuff);
			NextFlag = TRUE;
		}



		if(!NextFlag && lstrcmp(decode, "*마법숙련도") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->fMagic_Mastery = (float)atof(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "*마나재생") == 0 || lstrcmp(decode, "*기력재생") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fMana_Regen[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fMana_Regen[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "*라이프재생") == 0 || lstrcmp(decode, "*생명력재생") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fLife_Regen[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fLife_Regen[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "*스테미나재생") == 0 || lstrcmp(decode, "*근력재생") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fStamina_Regen[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fStamina_Regen[1] = (float)atof(strBuff);
			NextFlag = TRUE;

		}

		if(!NextFlag && (lstrcmp(decode, "*마나추가") == 0 || lstrcmp(decode, "*기력추가") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->Increase_Mana[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->Increase_Mana[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "*라이프추가") == 0 || lstrcmp(decode, "*생명력추가") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->Increase_Life[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->Increase_Life[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "*스테미나추가") == 0 || lstrcmp(decode, "*근력추가") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->Increase_Stamina[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->Increase_Stamina[1] = atoi(strBuff);
			NextFlag = TRUE;
		}





		if(!NextFlag && lstrcmp(decode, "*레벨") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Level = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*힘") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Strength = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*정신력") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Spirit = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*재능") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Talent = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*민첩성") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Dexterity = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*건강") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Health = atoi(strBuff);
			NextFlag = TRUE;
		}





		if(!NextFlag && (lstrcmp(decode, "*스테미너상승") == 0 || lstrcmp(decode, "*근력상승") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Stamina[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpItem->Stamina[1] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && (lstrcmp(decode, "*마나상승") == 0 || lstrcmp(decode, "*기력상승") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Mana[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpItem->Mana[1] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && (lstrcmp(decode, "*라이프상승") == 0 || lstrcmp(decode, "*생명력상승") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->Life[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpItem->Life[1] = atoi(strBuff);
			NextFlag = TRUE;
		}






		if(!NextFlag && lstrcmp(decode, "**특화") == 0)
		{
			while(1)
			{
				p = fileread_GetWord(strBuff, p);
				if(strBuff[0] == 0) break;
				DWORD resolvedJobBit = 0;
				if(ResolveClassSpecializationBit(strBuff, resolvedJobBit))
					lpItem->JobCodeMask = resolvedJobBit;
			}

			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "**특화랜덤") == 0)
		{
			while(1)
			{
				p = fileread_GetWord(strBuff, p);
				if(strBuff[0] == 0) break;
				DWORD resolvedJobBit = 0;
				if(ResolveClassSpecializationBit(strBuff, resolvedJobBit) &&
					lpDefItem->JobBitCodeRandomCount < SPECIAL_JOB_RANDOM_MAX)
				{
					lpDefItem->dwJobBitCode_Random[lpDefItem->JobBitCodeRandomCount++] = resolvedJobBit;
				}
			}

			NextFlag = TRUE;
		}







		if(!NextFlag && lstrcmp(decode, "**흡수력") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fSpecial_Absorb[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fSpecial_Absorb[1] = (float)atof(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "**방어력") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sSpecial_Defence[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sSpecial_Defence[1] = atoi(strBuff);


			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "**이동속도") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fSpecial_fSpeed[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fSpecial_fSpeed[1] = (float)atof(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "**흡수력") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Add_fBlock_Rating = (float)atof(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "**공격속도") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Add_Attack_Speed = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "**크리티컬") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Add_Critical_Hit = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "**사정거리") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Add_Shooting_Range = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "**마법숙련도") == 0)
		{
			p = fileread_GetWord(strBuff, p);

			lpDefItem->fSpecial_Magic_Mastery[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fSpecial_Magic_Mastery[1] = (float)atof(strBuff);

			NextFlag = TRUE;
		}








		if(!NextFlag && (lstrcmp(decode, "**마나추가") == 0 || lstrcmp(decode, "**기력추가") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Lev_Mana = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "**라이프추가") == 0 || lstrcmp(decode, "**생명력추가") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Lev_Life = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "**명중력") == 0)
		{

			p = fileread_GetWord(strBuff, p);
			lpDefItem->Lev_Attack_Rating[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->Lev_Attack_Rating[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "**공격력") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Lev_Damage[0] = atoi(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Lev_Damage[1] = atoi(strBuff);
			NextFlag = TRUE;
		}






		if(!NextFlag && (lstrcmp(decode, "**마나재생") == 0 || lstrcmp(decode, "**기력재생") == 0))
		{
			p = fileread_GetWord(strBuff, p);

			lpDefItem->fSpecial_Mana_Regen[0] = (float)atof(strBuff);
			p = fileread_GetWord(strBuff, p);
			lpDefItem->fSpecial_Mana_Regen[1] = (float)atof(strBuff);

			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "**라이프재생") == 0 || lstrcmp(decode, "**생명력재생") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Per_Life_Regen = (float)atof(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && (lstrcmp(decode, "**스테미나재생") == 0 || lstrcmp(decode, "**근력재생") == 0))
		{
			p = fileread_GetWord(strBuff, p);
			lpItem->JobItem.Per_Stamina_Regen = (float)atof(strBuff);
			NextFlag = TRUE;
		}

		if(!NextFlag && lstrcmp(decode, "*발생제한") == 0)
		{
			p = fileread_GetWord(strBuff, p);
			lpDefItem->sGenDay[0] = atoi(strBuff);
			NextFlag = TRUE;
		}


		if(!NextFlag && lstrcmp(decode, "*연결파일") == 0)
		{
			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

			char szNextFile[128];
			GetDirectoryFromFile(szItemFile, szNextFile, sizeof(szNextFile));
			lstrcat(szNextFile, strBuff);
			NextFlag = TRUE;
			fclose(fp);

			fp = summit::content::text_resource::OpenTextResourceFile(szNextFile, "rb");
			if(fp == NULL) break;
		}

	}

	if(lpItem->EffectBlink[0])
	{
		lpItem->EffectBlink[1] =
			(lpItem->EffectColor[0] + lpItem->EffectColor[1] + lpItem->EffectColor[2] + lpItem->EffectColor[3] + (short)lpItem->DispEffect);
	}

	if(fp) fclose(fp);
	return TRUE;
}


struct szSTATE_CODE
{
	char	*szCodeName;
	DWORD	dwCode;
};

