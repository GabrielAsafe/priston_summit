static int SummitLegacySkillTokenIsSeparator(char ch)
{
	return ch == ' ' || ch == '\t' || ch == '_' || ch == '-';
}

static unsigned char SummitLegacySkillTokenFold(unsigned char ch)
{
	if(ch >= 'A' && ch <= 'Z')
		return (unsigned char)(ch + ('a' - 'A'));
	return ch;
}

static int SummitLegacySkillTokenEquals(const char *left, const char *right)
{
	if(!left || !right) return FALSE;

	while(true)
	{
		while(*left && SummitLegacySkillTokenIsSeparator(*left)) left++;
		while(*right && SummitLegacySkillTokenIsSeparator(*right)) right++;

		if(!*left || !*right) break;
		if(SummitLegacySkillTokenFold((unsigned char)*left) != SummitLegacySkillTokenFold((unsigned char)*right))
			return FALSE;

		left++;
		right++;
	}

	while(*left && SummitLegacySkillTokenIsSeparator(*left)) left++;
	while(*right && SummitLegacySkillTokenIsSeparator(*right)) right++;

	return *left == 0 && *right == 0;
}

static char *fileread_GetWord(char *q, char *p)
{


	while((*p == 32) || (*p == 9))
	{
		p++;
	}

	while((*p != 32) && (*p != 9))
	{
		if(*p == '\n' || *p == '\r') break;
		*q++ = *p++;
	}
	*q++ = 0;



	return p;
}


char *szIniCommand[] = {
	"*동작파일",
	"*동작모음",
	"*모양파일",
	"*모양이름",
	"*파일연결",
	"*보조동작파일",

	"*정밀모양",
	"*보통모양",
	"*저질모양",

	"*XX걷는동작",
	"*XX서있기동작",

	"*EQUIPPED_WEAPON",
	"*APPLICABLE_JOB",
	"*LOCATION",
	"*APPLICABLE_SKILL",



	"*표정파일연결",
	"*표정파일",
	"*표정모음",


	""
};

char *szIniRepeat = "반복";
char *szMotionWord = "동작";

char *szRunWord = "뛰는";
char *szStandWord = "*서있기";
char *szStandWord2 = "*정지";
char *szWalkWord = "*걷는";

char *szAttackWord = "공격";
char *szDeadWord = "죽기";
char *szDeadWord2 = "죽는";
char *szDamageWord = "타격";
char *szDamageWord2 = "데미지";
char *szDamageWord3 = "막기";

char *szWarpWord = "울기";
char *szWarpWord2 = "워프";

char *szSometimeWord = "가끔";

char *szFallDownWord = "떨어";
char *szFallStandLargeWord = "큰착지";
char *szFallStandSmallWord = "작은착지";

char *szRestartWord = "다시시작";
char *szEatWord = "먹기";
char *szSkillWord = "기술";
char *szYahooWord = "만세";



char *szMotionTalkWord = "표정";

char *szArTalkWord = "아표정";
char *szETalkWord = "이표정";
char *szOhTalkWord = "오표정";
char *szEyeTalkWord = "눈깜빡표정";

char *szBlankTalkWord = "무표정(작동않함)";
char *szSmileTalkWord = "웃는표정";
char *szGrumbleTalkWord = "화난표정";
char *szSorrowTalkWord = "슬픈표정";
char *szStartledTalkWord = "놀란표정";
char *szNatureTalkWord = "고유표정";
char *szSpecialTalkWord = "특별표정";

// A pre-T5 .INX can still be structurally valid and decode correctly while
// lacking motions that were added later to its source .INI.  Registry ABI
// translation cannot recover an entry that is not present in MotionInfo at all.
// Keep this check local to model configs that actually declare T5 skill motions;
// head/submodel configs do not declare *APPLICABLE_SKILL T5 entries and are not
// invalidated by this path.
static int SummitLegacyIsCurrentT5SkillIndex(int skillIndex)
{
	return (skillIndex >= 16 && skillIndex <= 23) ||
		(skillIndex >= 39 && skillIndex <= 46) ||
		(skillIndex >= 62 && skillIndex <= 73) ||
		(skillIndex >= 89 && skillIndex <= 97) ||
		(skillIndex >= 116 && skillIndex <= 123) ||
		(skillIndex >= 140 && skillIndex <= 148) ||
		(skillIndex >= 165 && skillIndex <= 172) ||
		(skillIndex >= 189 && skillIndex <= 195);
}

static int SummitLegacyCurrentT5IndexToTemporaryAppended(int currentIndex)
{
	// MOTION_REGISTRY_ABI_v1 temporarily wrote legacy 1..126 followed by the
	// T5 aliases.  Accept those caches as containing the same source motion.
	if(currentIndex >= 16 && currentIndex <= 23) return currentIndex + 111;
	if(currentIndex >= 39 && currentIndex <= 46) return currentIndex + 96;
	if(currentIndex >= 62 && currentIndex <= 73) return currentIndex + 81;
	if(currentIndex >= 89 && currentIndex <= 97) return currentIndex + 66;
	if(currentIndex >= 116 && currentIndex <= 123) return currentIndex + 48;
	if(currentIndex >= 140 && currentIndex <= 148) return currentIndex + 32;
	if(currentIndex >= 165 && currentIndex <= 172) return currentIndex + 16;
	if(currentIndex >= 189 && currentIndex <= 195) return currentIndex;
	return 0;
}

static int SummitLegacyFindSkillRegistryIndex(const char *skillName)
{
	if(!skillName || !skillName[0]) return 0;

	for(int index = 1; SkillDataCode[index].szSkillName[0]; index++)
	{
		if(lstrcmpi(SkillDataCode[index].szSkillName, skillName) == 0 ||
			SummitLegacySkillTokenEquals(SkillDataCode[index].szSkillName, skillName))
			return index;
	}

	return 0;
}

static int SummitLegacyCachedModelHasT5SkillIndex(
	const smMODELINFO *cachedInfo,
	int currentIndex)
{
	if(!cachedInfo || !SummitLegacyIsCurrentT5SkillIndex(currentIndex))
		return FALSE;

	const int temporaryIndex = SummitLegacyCurrentT5IndexToTemporaryAppended(currentIndex);
	int motionCount = (int)cachedInfo->MotionCount;
	if(motionCount > MOTION_INFO_MAX) motionCount = MOTION_INFO_MAX;

	for(int motionIndex = 0; motionIndex < motionCount; motionIndex++)
	{
		const smMOTIONINFO *motion = &cachedInfo->MotionInfo[motionIndex];
		if(motion->State != CHRMOTION_STATE_SKILL)
			continue;

		for(int skillPos = 0; skillPos < MOTION_SKIL_MAX; skillPos++)
		{
			const int storedIndex = motion->SkillCodeList[skillPos];
			if(storedIndex == 0) break;
			if(storedIndex == currentIndex ||
				(temporaryIndex && storedIndex == temporaryIndex))
				return TRUE;
		}
	}

	return FALSE;
}

static int SummitLegacySourceDeclaresMissingT5Motion(
	const char *sourceFile,
	const smMODELINFO *cachedInfo,
	char *missingSkill,
	size_t missingSkillSize)
{
	if(missingSkill && missingSkillSize) missingSkill[0] = 0;
	if(!sourceFile || !sourceFile[0] || !cachedInfo)
		return FALSE;

	FILE *source = nullptr;
	fopen_s(&source, sourceFile, "rb");
	if(!source)
		return FALSE;

	char sourceLine[512] = { 0 };
	char command[64] = { 0 };
	char skillToken[64] = { 0 };
	int lastMotionWasSkill = FALSE;
	int missing = FALSE;

	while(!missing && fgets(sourceLine, sizeof(sourceLine), source))
	{
		char *cursor = fileread_GetWord(command, sourceLine);

		// Mirror the legacy parser's motion-state gate so an orphaned
		// *APPLICABLE_SKILL line cannot force a permanent rebuild loop.
		if(command[0] == '*' && strstr(command, szMotionWord))
		{
			lastMotionWasSkill = strstr(command, szSkillWord) ? TRUE : FALSE;
			continue;
		}

		if(!lastMotionWasSkill || lstrcmp(command, "*APPLICABLE_SKILL") != 0)
			continue;

		int recognizedCount = 0;
		while(recognizedCount < (MOTION_SKIL_MAX - 1))
		{
			cursor = fileread_GetWord(skillToken, cursor);
			if(!skillToken[0]) break;

			const int currentIndex = SummitLegacyFindSkillRegistryIndex(skillToken);
			if(!currentIndex)
				continue;

			recognizedCount++;
			if(!SummitLegacyIsCurrentT5SkillIndex(currentIndex))
				continue;

			if(!SummitLegacyCachedModelHasT5SkillIndex(cachedInfo, currentIndex))
			{
				missing = TRUE;
				if(missingSkill && missingSkillSize)
				{
					size_t copyLen = strlen(skillToken);
					if(copyLen >= missingSkillSize) copyLen = missingSkillSize - 1;
					memcpy(missingSkill, skillToken, copyLen);
					missingSkill[copyLen] = 0;
				}
				break;
			}
		}
	}

	fclose(source);
	return missing;
}

static void SummitLegacyTraceT5CacheRefresh(
	const char *sourceFile,
	const char *cacheFile,
	const char *missingSkill)
{
	FILE *trace = fopen("LOGS\\summit_t5_body_cache_refresh.txt", "a");
	if(!trace) return;
	fprintf(trace, "source=[%s] cache=[%s] missing=[%s] -> rebuild\n",
		sourceFile ? sourceFile : "",
		cacheFile ? cacheFile : "",
		missingSkill ? missingSkill : "");
	fclose(trace);
}



int SetIniMotionInfo(smMOTIONINFO *motion, char *p, int *lpRate = NULL)

{
	char strBuff[256];
	int EventCount;

	EventCount = 0;


	p = fileread_GetWord(strBuff, p);
	motion->StartFrame = atoi(strBuff);
	p = fileread_GetWord(strBuff, p);
	motion->EndFrame = atoi(strBuff);

	motion->EventFrame[0] = 0;
	motion->EventFrame[1] = 0;
	motion->EventFrame[2] = 0;
	motion->EventFrame[3] = 0;

	while(1)
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0]<'0' || strBuff[0]>'9') break;
		if(EventCount >= 4) break;

		motion->EventFrame[EventCount] = (atoi(strBuff) - motion->StartFrame) * 160;
		EventCount++;
	}



	if(lstrcmp(strBuff, szIniRepeat) == 0)
	{
		motion->Repeat = TRUE;
	}




	if(strBuff[0] != '(' && strBuff[0] >= ' ' && strBuff[0] <= 0x7F)
		motion->KeyCode = strBuff[0];

	if(lpRate)
	{
		p = fileread_GetWord(strBuff, p);

		int  len = strlen(strBuff);
		char NumBuff[10] = { 0, };

		if(len > 2 && len < 6 && strBuff[0] == '(' && strBuff[len - 1] == ')')
		{
			::CopyMemory(NumBuff, &strBuff[1], len - 2);
			*lpRate = atoi(NumBuff);
			if(*lpRate > 100)
				*lpRate = 100;
		}
	}


	return TRUE;
}

static char *szFileExt = "inx";





int AddModelDecode(char *iniFile, smMODELINFO *Info)

{
	char *szDirecotry = nullptr;
	char strBuff[64] = { 0 };
	char *p = nullptr;
	char *pb = nullptr;
	int CmdNum = 0;
	int cnt = 0;
	_MODELGROUP *lpModelGroup = nullptr;
	int JobCount = 0;

	char *szMotionFileList[128] = { 0 };

	int MotionFileListCnt = 0;
	int MotionFrameList[100];
	smPAT3D *pat3d = nullptr;

	int MotionLastPos = 0;


	MotionFileListCnt = 0;
	ZeroMemory(MotionFrameList, sizeof(int) * 100);

	FILE* fp = nullptr;

	fopen_s(&fp, iniFile, "rb");
	if(fp == NULL)
		return FALSE;


	szDirecotry = SetDirectoryFromFile(iniFile);

	while(!feof(fp))
	{
		if(fgets(line, 512, fp) == NULL)
			break;

		p = fileread_GetWord(decode, line);

		CmdNum = 0;

		while(true)
		{

			if(lstrcmp(decode, szIniCommand[CmdNum]) == 0)
				break;

			if(szIniCommand[CmdNum][0] == NULL)
			{
				if(decode[0] == '*' && strstr(decode, szMotionWord) && Info->MotionCount < MOTION_INFO_MAX)
				{



					if(Info->MotionCount < MOTION_INFO_MAX)
					{
						SetIniMotionInfo(&Info->MotionInfo[Info->MotionCount], p, Info->MotionCount < NPC_MOTION_INFO_MAX ? &Info->NpcMotionRate[Info->MotionCount] : NULL);
					}



					Info->MotionInfo[Info->MotionCount].MotionFrame = MotionFileListCnt;
					Info->MotionInfo[Info->MotionCount].State = TRUE;


					if(strstr(decode, szRunWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_RUN;
						Info->MotionInfo[Info->MotionCount].Repeat = TRUE;
					}


					if(strstr(decode, szAttackWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_ATTACK;
					}

					if(strstr(decode, szDeadWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DEAD;
					}

					if(strstr(decode, szDeadWord2))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DEAD;
					}


					if(strstr(decode, szDamageWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DAMAGE;
					}

					if(strstr(decode, szDamageWord2))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DAMAGE;
					}

					if(strstr(decode, szDamageWord3))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DAMAGE;
					}

					if(strstr(decode, szSometimeWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_SOMETIME;
					}


					if(strstr(decode, szFallDownWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_FALLDOWN;
					}

					if(strstr(decode, szFallStandSmallWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_FALLSTAND;
					}

					if(strstr(decode, szFallStandLargeWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_FALLDAMAGE;
					}


					if(strstr(decode, szStandWord) || strstr(decode, szStandWord2))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_STAND;
						Info->MotionInfo[Info->MotionCount].Repeat = TRUE;
					}

					if(strstr(decode, szWalkWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_WALK;
						Info->MotionInfo[Info->MotionCount].Repeat = TRUE;
					}

					if(strstr(decode, szRestartWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_RESTART;
					}

					if(strstr(decode, szEatWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_EAT;
					}

					if(strstr(decode, szSkillWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_SKILL;
						Info->MotionInfo[Info->MotionCount].SkillCodeList[0] = 0;
					}


					if(strstr(decode, szWarpWord) || strstr(decode, szWarpWord2))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_WARP;
					}


					if(strstr(decode, szYahooWord))
					{
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_YAHOO;
					}


					if(Info->MotionInfo[Info->MotionCount].State == CHRMOTION_STATE_DEAD)
					{
						Info->MotionInfo[Info->MotionCount].EndFrame -= 8;
					}



					MotionLastPos = Info->MotionCount;
					Info->MotionCount++;
				}


				else if(decode[0] == '*' && strstr(decode, szMotionTalkWord) && MotionFileListCnt && Info->TalkMotionCount < TALK_MOTION_INFO_MAX)
				{
					if(Info->TalkMotionCount < TALK_MOTION_INFO_MAX)
					{
						SetIniMotionInfo(&Info->TalkMotionInfo[Info->TalkMotionCount], p, &Info->TalkMotionRate[Info->TalkMotionCount]);
					}

					Info->TalkMotionInfo[Info->TalkMotionCount].MotionFrame = MotionFileListCnt - 1;


					if(strstr(decode, szArTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_AR;

					else if(strstr(decode, szETalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_E;

					else if(strstr(decode, szOhTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_OH;

					else if(strstr(decode, szEyeTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_EYE;

					else if(strstr(decode, szBlankTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_BLANK;

					else if(strstr(decode, szSmileTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_SMILE;

					else if(strstr(decode, szGrumbleTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_GRUMBLE;

					else if(strstr(decode, szSorrowTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_SORROW;

					else if(strstr(decode, szStartledTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_STARTLED;

					else if(strstr(decode, szNatureTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_NATURE;

					else if(strstr(decode, szSpecialTalkWord))
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_SPECIAL;

					Info->TalkMotionCount++;
				}

				break;
			}

			CmdNum++;
		}



		switch(CmdNum)
		{
		case 0:

			pb = p;
			p = fileread_GetWord(strBuff, p);
			if(strBuff[0] == 34)
			{
				p = GetString(strBuff, pb);
			}
			strcpy_s(Info->szMotionFile, szDirecotry);
			lstrcat(Info->szMotionFile, strBuff);
			break;



		case 17:


		case 1:
		{
			pb = p;
			p = fileread_GetWord(strBuff, p);
			if(strBuff[0] == 34)
			{
				p = GetString(strBuff, pb);
			}

			const size_t arraySize = 64;
			szMotionFileList[MotionFileListCnt] = new char[arraySize];
			strcpy_s(szMotionFileList[MotionFileListCnt], arraySize, szDirecotry);
			lstrcat(szMotionFileList[MotionFileListCnt], strBuff);
			MotionFileListCnt++;
		}
		break;

		case 2:

			p = fileread_GetWord(strBuff, p);
			strcpy_s(Info->szModelFile, szDirecotry);
			lstrcat(Info->szModelFile, strBuff);
			break;

		case 3:

			pb = p;
			p = fileread_GetWord(strBuff, p);
			if(strBuff[0] == 34)
			{
				p = GetString(strBuff, pb);
			}


			break;

		case 4:

			p = fileread_GetWord(strBuff, p);


			strcpy_s(Info->szLinkFile, szDirecotry);
			lstrcat(Info->szLinkFile, strBuff);
			break;


		case 5:

			pb = p;
			p = fileread_GetWord(strBuff, p);
			if(strBuff[0] == 34)
			{
				p = GetString(strBuff, pb);
			}



			strcpy_s(Info->szSubModelFile, szDirecotry);
			lstrcat(Info->szSubModelFile, strBuff);

			break;

		case 6:
		case 7:
		case 8:

			switch(CmdNum)
			{
			case 6:

				lpModelGroup = &Info->HighModel;
				break;
			case 7:
				lpModelGroup = &Info->DefaultModel;
				break;
			case 8:
				lpModelGroup = &Info->LowModel;
				break;
			}

			for(cnt = 0; cnt < 4; cnt++)
			{
				pb = p;
				p = fileread_GetWord(strBuff, p);
				if(!strBuff[0]) break;

				if(strBuff[0] == 34)
				{
					p = GetString(strBuff, pb);
				}
				if(lpModelGroup->ModelNameCnt < 4)
				{
					strcpy_s(lpModelGroup->szModelName[lpModelGroup->ModelNameCnt++], strBuff);
				}
			}
			break;

		case 9:



			break;
		case 10:



			break;




		case 11:

			while(true)
			{
				if(p[0] == 0)
					break;
				strBuff[0] = 0;
				p = fileread_GetWord(strBuff, p);
				if(strBuff[0])
				{
					if(lstrcmp(strBuff, "ALL") == 0)
					{
						Info->MotionInfo[MotionLastPos].ItemCodeCount = 0; break;
					}

					if(lstrcmp(strBuff, "NONE") == 0)
					{
						Info->MotionInfo[MotionLastPos].ItemCodeList[Info->MotionInfo[MotionLastPos].ItemCodeCount++] = 0xFF;
					}
					else
					{

						for(cnt = 0; cnt < MAX_ITEM; cnt++)
						{
							if(lstrcmp(strBuff, sItem[cnt].LastCategory) == 0)
							{
								Info->MotionInfo[MotionLastPos].ItemCodeList[
									Info->MotionInfo[MotionLastPos].ItemCodeCount++] = cnt;
								break;
							}
						}
					}

				}
				else
					break;
			}

			break;
		case 12:


			Info->MotionInfo[MotionLastPos].dwJobCodeBit = 0;
			while(true)
			{
				p = fileread_GetWord(strBuff, p);
				if(strBuff[0] == 0) break;
				DWORD motionJobMask = 0;
				if(ResolveClassMotionJobMask(strBuff, motionJobMask))
					Info->MotionInfo[MotionLastPos].dwJobCodeBit |= motionJobMask;
			}

			break;
		case 13:

			p = fileread_GetWord(strBuff, p);
			cnt = 0;

			if(lstrcmp(strBuff, "TOWN") == 0)
			{
				cnt |= 1;
			}
			if(lstrcmp(strBuff, "FIELD") == 0)
			{
				cnt |= 2;
			}

			p = fileread_GetWord(strBuff, p);
			if(strBuff[0])
			{
				if(lstrcmp(strBuff, "TOWN") == 0)
				{
					cnt |= 1;
				}
				if(lstrcmp(strBuff, "FIELD") == 0)
				{
					cnt |= 2;
				}
			}

			if(cnt == 0 || Info->MotionInfo[MotionLastPos].State == CHRMOTION_STATE_SKILL)
				cnt = 3;

			Info->MotionInfo[MotionLastPos].MapPosition = cnt;
			break;

		case 14:

			if(Info->MotionInfo[MotionLastPos].State == CHRMOTION_STATE_SKILL)
			{
				JobCount = 0;
				while(true)
				{
					p = fileread_GetWord(strBuff, p);
					if(strBuff[0] == 0) break;
					cnt = 0;
					while(true)
					{
						if(!SkillDataCode[cnt].szSkillName[0])
							break;
						if(JobCount >= (MOTION_SKIL_MAX - 1)) break;
						if(lstrcmpi(SkillDataCode[cnt].szSkillName, strBuff) == 0 ||
							SummitLegacySkillTokenEquals(SkillDataCode[cnt].szSkillName, strBuff))
						{
							Info->MotionInfo[MotionLastPos].SkillCodeList[JobCount++] = cnt;
							break;
						}
						cnt++;
					}
				}
				Info->MotionInfo[MotionLastPos].SkillCodeList[JobCount] = 0;
			}
			break;



		case 15:
			pb = p;
			p = fileread_GetWord(strBuff, p);
			if(strBuff[0] == 34)
				p = GetString(strBuff, pb);



			strcpy_s(Info->szTalkLinkFile, szDirecotry);
			lstrcat(Info->szTalkLinkFile, strBuff);

			break;

		case 16:
			pb = p;
			p = fileread_GetWord(strBuff, p);
			if(strBuff[0] == 34)
				p = GetString(strBuff, pb);



			strcpy_s(Info->szTalkMotionFile, szDirecotry);
			lstrcat(Info->szTalkMotionFile, strBuff);
			break;



		}
	}

	fclose(fp);




	int MaxRateNum, MaxRatePos;


	if(Info->NpcMotionRate[CHRMOTION_EXT])
	{


		MaxRateNum = MaxRatePos = CmdNum = 0;
		for(cnt = CHRMOTION_EXT; cnt < (int)Info->MotionCount; cnt++)
		{
			if(MaxRateNum < Info->NpcMotionRate[cnt])
			{
				MaxRateNum = Info->NpcMotionRate[cnt];
				MaxRatePos = cnt;
			}

			CmdNum += Info->NpcMotionRate[cnt];
		}

		if(CmdNum > 100)
		{
			MaxRateNum -= (CmdNum - 100);
			if(MaxRateNum <= 0)
			{


				Info->NpcMotionRate[CHRMOTION_EXT] = 0;
			}
			else
				Info->NpcMotionRate[MaxRatePos] = MaxRateNum;
		}
		else if(CmdNum < 100)
		{
			MaxRateNum += (100 - CmdNum);
			Info->NpcMotionRate[MaxRatePos] = MaxRateNum;
		}


		if(Info->NpcMotionRate[CHRMOTION_EXT])
		{
			MaxRatePos = 0;
			for(cnt = CHRMOTION_EXT; cnt < (int)Info->MotionCount; cnt++)
			{
				for(CmdNum = 0; CmdNum < Info->NpcMotionRate[cnt]; CmdNum++)
				{
					Info->NpcMotionRateCnt[MaxRatePos] = cnt;
					MaxRatePos++;
				}
			}
		}
	}


	if(Info->TalkMotionRate[CHRMOTION_EXT])
	{


		int MotionCntBuff[TALK_MOTION_FILE_MAX][TALK_MOTION_INFO_MAX] = { 0, };
		int RateBuff[TALK_MOTION_FILE_MAX][TALK_MOTION_INFO_MAX] = { 0, };
		int RateCnt[TALK_MOTION_FILE_MAX] = { CHRMOTION_EXT, CHRMOTION_EXT };

		for(cnt = CHRMOTION_EXT; cnt < (int)Info->TalkMotionCount; cnt++)
		{
			if(Info->TalkMotionInfo[cnt].MotionFrame == TALK_MOTION_FILE)
			{
				MotionCntBuff[TALK_MOTION_FILE][RateCnt[TALK_MOTION_FILE]] = cnt;
				RateBuff[TALK_MOTION_FILE][RateCnt[TALK_MOTION_FILE]] = Info->TalkMotionRate[cnt];
				RateCnt[TALK_MOTION_FILE]++;
			}
			else if(Info->TalkMotionInfo[cnt].MotionFrame == FACIAL_MOTION_FILE)
			{
				MotionCntBuff[FACIAL_MOTION_FILE][RateCnt[FACIAL_MOTION_FILE]] = cnt;
				RateBuff[FACIAL_MOTION_FILE][RateCnt[FACIAL_MOTION_FILE]] = Info->TalkMotionRate[cnt];
				RateCnt[FACIAL_MOTION_FILE]++;
			}
		}



		for(int loop = 0; loop < TALK_MOTION_FILE_MAX; loop++)
		{


			MaxRateNum = MaxRatePos = CmdNum = 0;

			int *lpRateBuff = RateBuff[loop];
			for(cnt = CHRMOTION_EXT; cnt < RateCnt[loop]; cnt++)
			{
				if(MaxRateNum < lpRateBuff[cnt])
				{
					MaxRateNum = lpRateBuff[cnt];
					MaxRatePos = cnt;
				}

				CmdNum += lpRateBuff[cnt];
			}

			if(CmdNum > 100)
			{
				MaxRateNum -= (CmdNum - 100);
				if(MaxRateNum <= 0)
				{


					Info->TalkMotionRate[CHRMOTION_EXT] = 0;
				}
				else
					lpRateBuff[MaxRatePos] = MaxRateNum;
			}
			else if(CmdNum < 100)
			{
				MaxRateNum += (100 - CmdNum);
				lpRateBuff[MaxRatePos] = MaxRateNum;
			}


			if(Info->TalkMotionRate[CHRMOTION_EXT])
			{
				MaxRatePos = 0;
				for(cnt = CHRMOTION_EXT; cnt < RateCnt[loop]; cnt++)
				{
					for(CmdNum = 0; CmdNum < lpRateBuff[cnt]; CmdNum++)
					{
						Info->TalkMotionRateCnt[loop][MaxRatePos] = MotionCntBuff[loop][cnt];
						MaxRatePos++;
					}
				}
			}
		}
	}






	if(MotionFileListCnt)

	{
		if(smFindPatFile(Info->szMotionFile, "smb") == FALSE)
		{
			pat3d = smASE_MergeBone(Info->szMotionFile, szMotionFileList, MotionFileListCnt);

			if(pat3d)
			{
				delete pat3d;
				pat3d = nullptr;
			}

		}
		else if(smFindPatFile(Info->szTalkMotionFile, "smb") == FALSE)
		{
			pat3d = smASE_MergeBone(Info->szTalkMotionFile, szMotionFileList, MotionFileListCnt, 0);

			if(pat3d)
			{
				delete pat3d;
				pat3d = nullptr;
			}
		}

		for(cnt = MotionFileListCnt - 1; cnt >= 0; cnt--)
		{
			if(szMotionFileList[cnt])
			{
				delete szMotionFileList[cnt];
				szMotionFileList[cnt] = nullptr;
			}

		}
	}


	char *szFile = ChangeFileExt(iniFile, szFileExt);
	DWORD dwCode = GetSpeedSum(szFile);
	ModelKeyWordEncode(dwCode, Info);


	if(Info->MotionCount > CHRMOTION_EXT && Info->TalkMotionCount <= CHRMOTION_EXT)
		MotionKeyWordEncode(dwCode, Info);


	szFile = ChangeFileExt(iniFile, szFileExt);
	fopen_s(&fp, szFile, "wb");
	if(fp)
	{
		fwrite(Info, sizeof(smMODELINFO), 1, fp);
		fclose(fp);
	}


	if(Info->MotionCount > CHRMOTION_EXT && Info->TalkMotionCount <= CHRMOTION_EXT)
		MotionKeyWordDecode(dwCode, Info);


	return TRUE;
}





int smModelDecode(char *iniFile, smMODELINFO *Info)
{
	char szFile[64] = { 0 };
	DWORD dwFileLen = 0;
	char *lpFile = nullptr;
	FILE *fp = nullptr;
	int len = 0;

	smMODELINFO smModelInfo;
	ZeroMemory(Info, sizeof(smMODELINFO));

	Info->TalkMotionCount = CHRMOTION_EXT;

	Info->MotionCount = CHRMOTION_EXT;
	Info->HighModel.ModelNameCnt = 0;
	Info->DefaultModel.ModelNameCnt = 0;
	Info->LowModel.ModelNameCnt = 0;

	len = lstrlen(iniFile);


	if(iniFile[len - 4] == '.' &&
		(iniFile[len - 3] == 'a' || iniFile[len - 3] == 'A') &&
		(iniFile[len - 2] == 's' || iniFile[len - 2] == 'S') &&
		(iniFile[len - 1] == 'e' || iniFile[len - 1] == 'E'))
	{

		strcpy_s(Info->szModelFile, iniFile);

		return TRUE;
	}


	for(int i = 0; i < 3; i++)
	{
		switch(i)
		{
		case 0:
			strcpy_s(szFile, iniFile);
			break;

		case 1:
			strcpy_s(szFile, Info->szLinkFile);
			break;

		case 2:
			strcpy_s(szFile, Info->szTalkLinkFile);
			break;
		}

		if(szFile[0])
		{
			lpFile = smFindFile(szFile, szFileExt, &dwFileLen);

			if(lpFile && dwFileLen == sizeof(smMODELINFO))
			{
				fopen_s(&fp, lpFile, "rb");
				if(fp)
				{
					ZeroMemory(&smModelInfo, sizeof(smMODELINFO));
					fread(&smModelInfo, sizeof(smMODELINFO), 1, fp);
					fclose(fp);

					// MINGW_MODEL_PATH_CHECKSUM_V2
#ifdef __MINGW32__
					char mingwModelCodePath[260] = { 0 };
					int mingwSrcPos = 0;
					int mingwDstPos = 0;
					int mingwLastWasSeparator = FALSE;

					while(
						lpFile[mingwSrcPos] &&
						mingwDstPos <
							(int)sizeof(mingwModelCodePath) - 1
					)
					{
						char mingwChar =
							lpFile[mingwSrcPos++];

						if(
							mingwChar == '/' ||
							mingwChar == '\\'
						)
						{
							if(mingwLastWasSeparator)
								continue;

							mingwChar = '\\';
							mingwLastWasSeparator = TRUE;
						}
						else
						{
							mingwLastWasSeparator = FALSE;
						}

						mingwModelCodePath[
							mingwDstPos++
						] = mingwChar;
					}

					mingwModelCodePath[
						mingwDstPos
					] = 0;

					DWORD dwCode =
						GetSpeedSum(
							mingwModelCodePath
						);

					int mingwKeywordResult =
						ModelKeyWordDecode(
							dwCode,
							&smModelInfo
						);

#ifdef SUMMIT_ENABLE_MINGW_RUNTIME_TRACE
					{
						FILE *mingwTrace =
							fopen(
								"LOGS\\mingw_model_decode_trace.txt",
								"a"
							);

						if(mingwTrace)
						{
							fprintf(
								mingwTrace,
								"input=[%s] "
								"canonical=[%s] "
								"size=%lu "
								"code=0x%08lX "
								"keyword=%d "
								"model=[%s] "
								"link=[%s] "
								"motionCount=%lu\n",
								lpFile,
								mingwModelCodePath,
								(unsigned long)dwFileLen,
								(unsigned long)dwCode,
								mingwKeywordResult,
								smModelInfo.szModelFile,
								smModelInfo.szLinkFile,
								(unsigned long)
									smModelInfo.MotionCount
							);

							fflush(mingwTrace);
							fclose(mingwTrace);
						}
					}
#endif

					if(!mingwKeywordResult)
					{
						// A cache can be structurally the right size but stale/corrupt.
						// Rebuild from the source config instead of making the model unloadable.
						if(AddModelDecode(szFile, Info) == FALSE)
							return FALSE;
						continue;
					}
#else
					DWORD dwCode =
						GetSpeedSum(lpFile);

					if(
						ModelKeyWordDecode(
							dwCode,
							&smModelInfo
						) == FALSE
					)
					{
						if(AddModelDecode(szFile, Info) == FALSE)
							return FALSE;
						continue;
					}
#endif

					if(Info->szModelFile[0] && i > 0)
					{
						if(smModelInfo.MotionCount > CHRMOTION_EXT && i == 1)
						{
							if(MotionKeyWordDecode(dwCode, &smModelInfo) == FALSE)
							{
								if(AddModelDecode(szFile, Info) == FALSE)
									return FALSE;
								continue;
							}

							char summitMissingT5Skill[64] = { 0 };
							if(SummitLegacySourceDeclaresMissingT5Motion(
								szFile, &smModelInfo, summitMissingT5Skill, sizeof(summitMissingT5Skill)))
							{
								SummitLegacyTraceT5CacheRefresh(szFile, lpFile, summitMissingT5Skill);
								if(AddModelDecode(szFile, Info) == FALSE)
									return FALSE;
								continue;
							}


							strcpy_s(Info->szMotionFile, smModelInfo.szMotionFile);
							CopyMemory(Info->MotionInfo, smModelInfo.MotionInfo, sizeof(smModelInfo.MotionInfo));
							Info->MotionCount = smModelInfo.MotionCount;


							CopyMemory(Info->NpcMotionRate, smModelInfo.NpcMotionRate, sizeof(smModelInfo.NpcMotionRate));
							CopyMemory(Info->NpcMotionRateCnt, smModelInfo.NpcMotionRateCnt, sizeof(smModelInfo.NpcMotionRateCnt));
						}
						else if(smModelInfo.TalkMotionCount > CHRMOTION_EXT && i == 2)
						{

							strcpy_s(Info->szTalkLinkFile, smModelInfo.szTalkLinkFile);
							strcpy_s(Info->szTalkMotionFile, smModelInfo.szTalkMotionFile);
							CopyMemory(Info->TalkMotionInfo, smModelInfo.TalkMotionInfo, sizeof(smModelInfo.TalkMotionInfo));
							Info->TalkMotionCount = smModelInfo.TalkMotionCount;


							CopyMemory(Info->TalkMotionRate, smModelInfo.TalkMotionRate, sizeof(smModelInfo.TalkMotionRate));
							CopyMemory(Info->TalkMotionRateCnt, smModelInfo.TalkMotionRateCnt, sizeof(smModelInfo.TalkMotionRateCnt));
						}
					}
					else
					{
						if(smModelInfo.MotionCount > CHRMOTION_EXT)
						{
							if(MotionKeyWordDecode(dwCode, &smModelInfo) == FALSE)
							{
								if(AddModelDecode(szFile, Info) == FALSE)
									return FALSE;
								continue;
							}

							char summitMissingT5Skill[64] = { 0 };
							if(SummitLegacySourceDeclaresMissingT5Motion(
								szFile, &smModelInfo, summitMissingT5Skill, sizeof(summitMissingT5Skill)))
							{
								SummitLegacyTraceT5CacheRefresh(szFile, lpFile, summitMissingT5Skill);
								if(AddModelDecode(szFile, Info) == FALSE)
									return FALSE;
								continue;
							}
						}

						memcpy(Info, &smModelInfo, sizeof(smMODELINFO));
					}
				}
				else
				{
					if(AddModelDecode(szFile, Info) == FALSE)
						return FALSE;
				}
			}
			else
			{
				if(AddModelDecode(szFile, Info) == FALSE)
					return FALSE;
			}
		}
	}

	return TRUE;
}

smCONFIG smConfig;


char *szWordTexQualty[4] = { "좋음" , "보통" , "나쁨" , "아주나쁨" };
char *InfoRegPath = "SOFTWARE//Triglow Pictures//PristonTale";


TRANS_SERVER_LIST	TransServerList;
rsSERVER_CONFIG	rsServerConfig;
static int	srAdminCmdCnt = 0;
DWORD GetSwapIPCode(char *szIP);



