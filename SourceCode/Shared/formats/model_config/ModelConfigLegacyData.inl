#pragma once

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





