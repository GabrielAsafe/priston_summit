#pragma once

char *szIniCommand[] = {
	"*��������",
	"*���۸���",
	"*�������",
	"*����̸�",
	"*���Ͽ���",
	"*������������",

	"*���и��",
	"*������",
	"*�������",

	"*XX�ȴµ���",
	"*XX���⵿ֱ��",

	"*EQUIPPED_WEAPON",
	"*APPLICABLE_JOB",
	"*LOCATION",
	"*APPLICABLE_SKILL",



	"*ǥ�����Ͽ���",
	"*ǥ������",
	"*ǥ������",


	""
};

char *szIniRepeat = "�ݺ�";
char *szMotionWord = "����";

char *szRunWord = "�ٴ�";
char *szStandWord = "*���ֱ�";
char *szStandWord2 = "*����";
char *szWalkWord = "*�ȴ�";

char *szAttackWord = "����";
char *szDeadWord = "�ױ�";
char *szDeadWord2 = "�״�";
char *szDamageWord = "Ÿ��";
char *szDamageWord2 = "������";
char *szDamageWord3 = "����";

char *szWarpWord = "���";
char *szWarpWord2 = "����";

char *szSometimeWord = "����";

char *szFallDownWord = "����";
char *szFallStandLargeWord = "ū����";
char *szFallStandSmallWord = "��������";

char *szRestartWord = "�ٽý���";
char *szEatWord = "�Ա�";
char *szSkillWord = "���";
char *szYahooWord = "����";



char *szMotionTalkWord = "ǥ��";

char *szArTalkWord = "��ǥ��";
char *szETalkWord = "��ǥ��";
char *szOhTalkWord = "��ǥ��";
char *szEyeTalkWord = "�����ǥ��";

char *szBlankTalkWord = "��ǥ��(�۵�����)";
char *szSmileTalkWord = "���ǥ��";
char *szGrumbleTalkWord = "ȭ��ǥ��";
char *szSorrowTalkWord = "����ǥ��";
char *szStartledTalkWord = "���ǥ��";
char *szNatureTalkWord = "����ǥ��";
char *szSpecialTalkWord = "Ư��ǥ��";


static summit::formats::model_config::MotionKeywords GetModelMotionKeywords()
{
	return summit::formats::model_config::MotionKeywords{
		szIniRepeat, szRunWord, szAttackWord, szDeadWord, szDeadWord2,
		szDamageWord, szDamageWord2, szDamageWord3, szSometimeWord,
		szFallDownWord, szFallStandSmallWord, szFallStandLargeWord,
		szStandWord, szStandWord2, szWalkWord, szRestartWord, szEatWord,
		szSkillWord, szWarpWord, szWarpWord2, szYahooWord
	};
}

static summit::formats::model_config::TalkMotionKeywords GetTalkMotionKeywords()
{
	return summit::formats::model_config::TalkMotionKeywords{
		szArTalkWord, szETalkWord, szOhTalkWord, szEyeTalkWord, szBlankTalkWord,
		szSmileTalkWord, szGrumbleTalkWord, szSorrowTalkWord, szStartledTalkWord,
		szNatureTalkWord, szSpecialTalkWord
	};
}



JOBNAME_BIT BitMaskJobNames[] = {
	{	"Fighter"		,	0x0001	},
	{	"Mechanician"	,	0x0002	},
	{	"Archer"		,	0x0004	},
	{	"Pikeman"		,	0x0008	},

	{	"Atalanta"		,	0x0010	},
	{	"Knight"		,	0x0020	},
	{	"Magician"		,	0x0040	},
	{	"Priest"		,	0x0080	},

	{	"" , 0 },

	{	"PureBody"		,	0x0040	},
	{	"PureBody"		,	0x0080	},
	{	"PureBody"		,	0x0100	},
	{	"PureBody"		,	0x0200	},
	{	"PureBody"		,	0x0400	},
	{	"PureBody"		,	0x0800	},
	{	"PureBody"		,	0x1000	}
};





