#include "gameplay/quests/legacy/LegacyClassQuestAdapter.h"

switch(lpTransCommand->WParam)
{

	case HAQUEST_CODE_ELEMENTARY_A:
	if(lpTransCommand->LParam == 1)
	{
		rsPutItem(lpPlayInfo, sinQT1 | sin15);
	}

	if(lpTransCommand->LParam == 2)
	{
		dwItemCode = 0;

		const std::uint32_t starterLowCode =
			gameplay::quests::legacy::UsesCasterStarterDefense(lpPlayInfo->smCharInfo.JOB_CODE) ? sin01 : sin02;
		dwItemCode = gameplay::quests::legacy::DefaultWeaponRewardCode(
			lpPlayInfo->smCharInfo.JOB_CODE, starterLowCode);
		if(dwItemCode)
		{
			rsPutItem(lpPlayInfo, dwItemCode, lpPlayInfo->smCharInfo.JOB_CODE);
		}
	#ifdef _BEGINNER_EVENT

		dwItemCode = 0;

		dwItemCode = (gameplay::quests::legacy::UsesCasterStarterDefense(lpPlayInfo->smCharInfo.JOB_CODE)
			? sinDA2 : sinDA1) | sin02;
		if(dwItemCode)
		{
			rsPutItem(lpPlayInfo, dwItemCode, lpPlayInfo->smCharInfo.JOB_CODE);
		}
	#endif
	}
	break;

	case HAQUEST_CODE_ELEMENTARY_B:
	if(lpTransCommand->LParam == 2)
	{
		rsPutItem(lpPlayInfo, sinOA2 | sin02, lpPlayInfo->smCharInfo.JOB_CODE);

	#ifdef _BEGINNER_EVENT	
		rsPutItem(lpPlayInfo, sinPL1 | sin01, 10);
		rsPutItem(lpPlayInfo, sinPS1 | sin01, 10);
		rsPutItem(lpPlayInfo, sinPM1 | sin01, 10);

		rsSendMoneyToUser(lpPlayInfo, 1000);
	#endif
	}
	break;

	case HAQUEST_CODE_ELEMENTARY_C:
	if(lpTransCommand->LParam == 1)
	{
		rsPutItem(lpPlayInfo, sinQT1 | sin14);
	}
	if(lpTransCommand->LParam == 2)
	{
		rsPutItem(lpPlayInfo, sinPL1 | sin01, 10);
		rsPutItem(lpPlayInfo, sinPS1 | sin01, 10);
		rsPutItem(lpPlayInfo, sinPM1 | sin01, 10);

	#ifdef _BEGINNER_EVENT
		rsPutItem(lpPlayInfo, sinGP1 | sin17);
		rsPutItem(lpPlayInfo, sinGP1 | sin17);
	#endif
	}
	break;

	case HAQUEST_CODE_ELEMENTARY_D:
	if(lpTransCommand->LParam == 2)
	{
		rsPutItem(lpPlayInfo, sinOA1 | sin06, lpPlayInfo->smCharInfo.JOB_CODE);
		rsPutItem(lpPlayInfo, sinOR1 | sin06, lpPlayInfo->smCharInfo.JOB_CODE);
		rsPutItem(lpPlayInfo, sinOR1 | sin06, lpPlayInfo->smCharInfo.JOB_CODE);
	}
#ifdef	_BEGINNER_EVENT
	if(lpTransCommand->LParam == 2)
	{
		dwItemCode = 0;

		dwItemCode = gameplay::quests::legacy::DefaultWeaponRewardCode(
			lpPlayInfo->smCharInfo.JOB_CODE, sin06);
		if(dwItemCode)
		{
			rsPutItem(lpPlayInfo, dwItemCode, lpPlayInfo->smCharInfo.JOB_CODE);
		}
	}
#endif
	break;

	case HAQUEST_CODE_ELEMENTARY_E:
	if(lpTransCommand->LParam == 2)
	{
		rsPutItem(lpPlayInfo, sinFO1 | sin04);
		rsPutItem(lpPlayInfo, sinFO1 | sin04);
	#ifdef _BEGINNER_EVENT
		rsPutItem(lpPlayInfo, sinGP1 | sin17);
		rsPutItem(lpPlayInfo, sinGP1 | sin17);
		rsPutItem(lpPlayInfo, sinGP1 | sin17);
		rsPutItem(lpPlayInfo, sinGP1 | sin17);
		rsPutItem(lpPlayInfo, sinGP1 | sin17);
	#endif
	}
	break;

	case HAQUEST_CODE_ELEMENTARY_F:
	if(lpTransCommand->LParam == 2)
	{
		rsPutItem(lpPlayInfo, sinPL1 | sin03, 20);
		rsPutItem(lpPlayInfo, sinPS1 | sin03, 20);
		rsPutItem(lpPlayInfo, sinPM1 | sin03, 20);
	#ifdef _BEGINNER_EVENT
		rsPutItem(lpPlayInfo, sinBI2 | sin01);
	#endif
	}
	break;

	case HAQUEST_CODE_ELEMENTARY_G:
	if(lpTransCommand->LParam == 2)
	{
		rsPutItem(lpPlayInfo, sinFO1 | sin06);
		rsPutItem(lpPlayInfo, sinFO1 | sin06);

	#ifdef	_BEGINNER_EVENT
		rsPutItem(lpPlayInfo, sinBI1 | sin13);
	#endif
	}

#ifdef	_BEGINNER_EVENT
	if(lpTransCommand->LParam == 2)
	{
		dwItemCode = 0;

		dwItemCode = gameplay::quests::legacy::DefaultWeaponRewardCode(
			lpPlayInfo->smCharInfo.JOB_CODE, sin11);
		if(dwItemCode)
		{
			rsPutItem(lpPlayInfo, dwItemCode, lpPlayInfo->smCharInfo.JOB_CODE);
		}
	}
#endif
	break;
	case HAQUEST_CODE_FURYOFPHANTOM:
	if(lpTransCommand->LParam == 2)
	{
		switch(lpTransCommand->SParam)
		{
			case 1:
			rsPutItemUnique(lpPlayInfo, sinWA1 | sin20, lpPlayInfo->smCharInfo.JOB_CODE, ITEM_UNIQUE_CODE_QUEST);
			break;
			case 2:
			rsPutItemUnique(lpPlayInfo, sinWC1 | sin20, lpPlayInfo->smCharInfo.JOB_CODE, ITEM_UNIQUE_CODE_QUEST);
			break;
			case 3:
			rsPutItemUnique(lpPlayInfo, sinWH1 | sin21, lpPlayInfo->smCharInfo.JOB_CODE, ITEM_UNIQUE_CODE_QUEST);
			break;
			case 4:
			rsPutItemUnique(lpPlayInfo, sinWP1 | sin21, lpPlayInfo->smCharInfo.JOB_CODE, ITEM_UNIQUE_CODE_QUEST);
			break;
			case 5:
			rsPutItemUnique(lpPlayInfo, sinWS1 | sin22, lpPlayInfo->smCharInfo.JOB_CODE, ITEM_UNIQUE_CODE_QUEST);
			break;
			case 6:
			rsPutItemUnique(lpPlayInfo, sinWS2 | sin23, lpPlayInfo->smCharInfo.JOB_CODE, ITEM_UNIQUE_CODE_QUEST);
			break;
			case 7:
			rsPutItemUnique(lpPlayInfo, sinWT1 | sin21, lpPlayInfo->smCharInfo.JOB_CODE, ITEM_UNIQUE_CODE_QUEST);
			break;
			case 8:
			rsPutItemUnique(lpPlayInfo, sinWM1 | sin21, lpPlayInfo->smCharInfo.JOB_CODE, ITEM_UNIQUE_CODE_QUEST);
			break;
		

		}
	}
	break;
}






