

#define   SIN_ADD_FIRE				0x00000001	
#define   SIN_ADD_ICE				0x00000002	
#define   SIN_ADD_LIGHTNING			0x00000004	
#define   SIN_ADD_POISON			0x00000008	
#define   SIN_ADD_BIO				0x00000010	
#define   SIN_ADD_CRITICAL			0x00000020	
#define   SIN_ADD_ATTACK_RATE		0x00000040	
#define   SIN_ADD_DAMAGE_MIN		0x00000080	
#define   SIN_ADD_DAMAGE_MAX		0x00000100	
#define   SIN_ADD_ATTACK_SPEED		0x00000200	
#define   SIN_ADD_ABSORB			0x00000400	
#define   SIN_ADD_DEFENCE			0x00000800	
#define   SIN_ADD_BLOCK_RATE		0x00001000	
#define   SIN_ADD_MOVE_SPEED		0x00002000	
#define   SIN_ADD_LIFE				0x00004000	
#define   SIN_ADD_MANA				0x00008000	
#define   SIN_ADD_STAMINA			0x00010000	
#define   SIN_ADD_LIFEREGEN			0x00020000  
#define   SIN_ADD_MANAREGEN			0x00040000  
#define   SIN_ADD_STAMINAREGEN		0x00080000  

#define   SIN_ADD_NUM				1
#define   SIN_ADD_PERCENT			2

#include "systems/localization/config/language.h"

#ifdef	_LANGUAGE_KOREAN



















{
	{
		sinWA1, sinWC1, sinWH1, sinWM1, sinWP1, sinWS1, sinWS2, sinWT1
	},
	{ 5,1,2,3,4,5,6,7,8 },
	{ SIN_ADD_BIO,7,6,5,4,3,2,1 },
	{ 5,4,3,2,1,10,10,10 },
	{ SIN_ADD_NUM,2,22,21,31,32,45,46 },
			"�� �Ӽ� +5/r"},

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,5,1,2,0,3,0,4,0},
			{SIN_ADD_POISON,1,0,7,0,8,0,9},
			{51,52,53,54,55,55,55,55},
			{SIN_ADD_NUM,5,4,3,4,5,7,6},
			"�� �Ӽ� +5/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{5,5,5,5,5,0,0,0,0},
			{SIN_ADD_ICE,7,6,5,4,3,2,1},
			{5,15,13,13,12,12,12,12},
			{SIN_ADD_NUM,21,22,23,24,25,26,27},
			"�� �Ӽ� +5/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,1,1,5,1,1,1,1,1},
			{SIN_ADD_FIRE,5,10,15,20,25,30,35},
			{5,5,5,5,5,5,5,5},
			{SIN_ADD_NUM,20,21,22,23,24,25,26},
			"ȭ �Ӽ� +5/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,1,1,5,5,5,5,1,1},
			{SIN_ADD_LIGHTNING,4,8,12,24,28,32,0},
			{5,4,3,2,1,1,1,1},
			{SIN_ADD_NUM,10,20,30,0,0,0,0},
			"�� �Ӽ� +5/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{5,5,5,5,1,1,1,1,1},
			{SIN_ADD_CRITICAL,2,4,6,8,10,12,14},
			{1,2,3,4,5,6,7,8},
			{SIN_ADD_NUM,10,20,30,40,50,61,62},
			"ũ��Ƽ�� +1%/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,1,1,1,5,1,1,1,1},
			{SIN_ADD_ATTACK_RATE,23,24,25,26,27,28,29},
			{50,10,20,30,40,50,60,70},
			{SIN_ADD_NUM,10,10,10,10,10,10,5},
			"���߷� +50/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{5,5,5,5,1,1,1,1,1},
			{SIN_ADD_STAMINA,12,13,15,16,17,18,22},
			{20,10,10,10,10,20,20,20},
			{SIN_ADD_NUM,1,2,3,4,5,6,7},
			"�ٷ� +20/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{5,5,5,5,5,5,5,5,5},
			{SIN_ADD_ATTACK_RATE,1,2,3,4,5,6,7},
			{20,4,4,4,4,3,4,1},
			{SIN_ADD_PERCENT,1,2,2,2,2,2,2},
			"���߷� +20%/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{3,3,5,5,5,5,5,5,5},
			{SIN_ADD_ATTACK_RATE,1,1,1,1,1,1,1},
			{11,8,7,6,5,4,3,2},
			{SIN_ADD_PERCENT,1,2,3,4,5,6,1},
			"���߷� +40%/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,0,2,0,1,1,1,1,1},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,2,2,1,1,2,0},
			{10,1,1,2,0,3,2,1},
			{SIN_ADD_NUM,SIN_ADD_NUM,1,0,1,0,1,0},
			"���߷� +10/r�ּ� ���ݷ� +1/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,0,0,2,0,6,6,6,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MAX,5,4,3,2,1,1},
			{10,10,10,10,40,30,20,10},
			{SIN_ADD_NUM,SIN_ADD_NUM,10,10,10,10,10,10},
			"���߷� +10/r�ִ� ���ݷ� +1/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{2,0,2,2,1,1,1,1,1},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0},
			{20,1,1,0,2,1,2,1},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,1,2,3,4,5},
			"���߷� +20/r�ּ� ���ݷ� +1/r�ִ� ���ݷ� +1/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,2,0,1,2,2,2,0,2},
			{SIN_ADD_ATTACK_RATE,1,1,0,0,1,1,1},
			{100,1,1,1,1,1,2,2},
			{SIN_ADD_PERCENT,2,2,2,2,2,3,4},
			"���߷� +100%/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,2,0,1,0,1,0,1},
			{SIN_ADD_DAMAGE_MIN,2,2,2,0,2,2,0},
			{1,0,2,2,2,2,0,0},
			{SIN_ADD_NUM,0,3,3,3,3,0,1},
			"�ּ� ���ݷ� +1/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,0,2,0,1,0,0,1},
			{SIN_ADD_DAMAGE_MAX,7,7,7,7,7,1,1},
			{2,2,0,0,1,1,0,1},
			{SIN_ADD_NUM,0,2,3,4,5,1,0},
			"�ִ� ���ݷ� +2/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,2,2,0,1,1,1,1},
			{SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0},
			{2,1,1,0,0,8,8,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,1,1,2,3,5},
			"ũ��Ƽ�� +2%/r�ּ� ���ݷ� +1/r�ִ� ���ݷ� +1/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,0,4,4,0,0,5,5},
			{SIN_ADD_DAMAGE_MAX,1,2,3,4,5,6,7},
			{4,4,4,5,5,5,5,5},
			{SIN_ADD_NUM,2,2,2,2,1,1,1},
			"�ִ� ���ݷ� +4/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,1,1,4,4,1,1,1,1},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MAX,1,2,3,4,5,6},
			{10,4,0,0,4,4,4,4},
			{SIN_ADD_PERCENT,SIN_ADD_NUM,1,2,3,4,5,6},
			"���߷� +10%/r�ִ� ���ݷ� +4/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,2,4,1,1,1,1,1},
			{SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,1,1,1,1,1,1},
			{1,3,0,1,2,3,3,3},
			{SIN_ADD_NUM,SIN_ADD_NUM,1,2,3,4,1,1},
			"�ּ� ���ݷ� +1/r�ִ� ���ݷ� +3/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,1,2,1,0,0,2,1},
			{SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,1,0,3,3,2,1},
			{2,2,0,1,1,1,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,4,4,0},
			"�ּ� ���ݷ� +2/r�ִ� ���ݷ� +2/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,0,1,1,1,3,0,0,0},
			{SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,7,0,0,7,7},
			{1,3,3,0,0,7,7,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,1,2,1,0,1},
			"ũ��Ƽ�� �� +4%/r�ּ� ���ݷ� +3/r�ִ� ���ݷ� +3/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,1,2,4,1,8,8,8,8},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,8,8,8,8,8},
			{10,1,7,0,8,0,5,0},
			{SIN_ADD_PERCENT,SIN_ADD_NUM,SIN_ADD_NUM,0,0,2,0,1},
			"���߷� +10%/r�ּ� ���ݷ� +1/r�ִ� ���ݷ� +4/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,3,5,0,0,1,0,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,5,4,3,2,1},
			{40,7,7,7,7,7,7,7},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,6,6,6,6,6},
			"���߷� +40/r�ּ� ���ݷ� +3/r�ִ� ���ݷ� +5/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,1,0,5,3,2,1,0},
			{SIN_ADD_CRITICAL,SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MAX,5,5,5,5,0},
			{4,60,6,3,20,20,20,4},
			{SIN_ADD_NUM,SIN_ADD_PERCENT,SIN_ADD_NUM,10,20,12,14,15},
			"ũ��Ƽ�� +4%/r���߷� +60%/r�ִ� ���ݷ� +6/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,11,2,25,2,42,21,0,10},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,10,20,30,40},
			{80,2,4,4,12,13,43,12},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,10,10,10,10},
			"���߷� +80/rũ��Ƽ�� +2%/r�ּ� ���ݷ� +4/r�ִ� ���ݷ� +4/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,3,3,0,2,0,2,0},
			{SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,SIN_ADD_MANA,23,42,25,54},
			{4,6,6,20,10,10,10,10},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,1,2,3,4},
			"ũ��Ƽ�� +4%/r�ּ� ���ݷ� +6/r�ִ� ���ݷ� +6/r��� +20/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,2,2,1,1,1,3,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,SIN_ADD_LIFE,1,0,0,1},
			{60,6,6,1,1,1,1,1},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,2,2,2,2},
			"���߷� +60/r�ּ� ���ݷ� +6/r�ִ� ���ݷ� +6/r����� +20/r" },



			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,0,0,0,0,1},
			{SIN_ADD_CRITICAL,0,0,0,0,0,0,0},
			{4,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"ũ��Ƽ�� +4%/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,2,0,0,2,3,3},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{100,3,6,6,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"���߷� +100/rũ��Ƽ�� +3%/r�ּ� ���ݷ� +6/r�ִ� ���ݷ� +6/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,1,0,4,0,2,4},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_LIFE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{80,30,7,7,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"���߷� +80/r����� +30/r�ּ� ���ݷ� +7/r�ִ� ���ݷ� +7/r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,2,1,2,3,2},
			{SIN_ADD_CRITICAL,SIN_ADD_MANA,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{5,30,7,7,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"ũ��Ƽ�� +5%/r��� +30/r�ּ� ���ݷ� +7/r�ִ� ���ݷ� +7/r" },





			{ {sinDA1,sinDA2,2,10,0,0,0,1},
			{5,0,0,10,0,0,4,10,1},
			{SIN_ADD_BIO,4,10,0,1,1,1,0},
			{5,0,0,7,4,4,4,10},
			{SIN_ADD_NUM,0,2,40,30,20,10,0},
			"�� �Ӽ� +5/r" },

			{ {sinDA1,sinDA2,2,2,1,2,3,4},
			{0,5,0,5,5,5,5,5,5},
			{SIN_ADD_POISON,1,0,1,0,1,0,1},
			{5,2,2,4,3,2,1,1},
			{SIN_ADD_NUM,1,2,3,4,5,6,7},
			"�� �Ӽ� +5/r" },

			{ {sinDA1,sinDA2,1,1,1,1,1,1},
			{1,1,1,0,1,2,2,2,2},
			{SIN_ADD_ICE,0,1,1,1,0,2,2},
			{5,0,0,5,5,5,5,1},
			{SIN_ADD_NUM,0,5,0,0,5,4,4},
			"�� �Ӽ� +5/r" },

			{ {sinDA1,sinDA2,4,4,0,3,3,3},
			{1,2,3,5,2,2,2,2,2},
			{SIN_ADD_FIRE,1,2,3,4,5,6,7},
			{5,0,0,0,5,5,5,5},
			{SIN_ADD_NUM,1,2,3,3,3,3,3},
			"ȭ �Ӽ� +5/r" },

			{ {sinDA1,sinDA2,0,2,1,0,2,1},
			{0,4,0,4,4,0,0,0,4},
			{SIN_ADD_LIGHTNING,1,2,0,1,2,0,1},
			{4,4,0,0,0,0,4,4},
			{SIN_ADD_NUM,1,2,3,4,5,6,7},
			"�� �Ӽ� +5/r" },

			{ {sinDA1,sinDA2,0,0,7,1,7,1},
			{0,0,0,7,7,7,0,0,7},
			{SIN_ADD_DEFENCE,5,0,2,3,3,3,0},
			{20,0,0,0,0,3,3,3},
			{SIN_ADD_NUM,3,0,0,1,1,2,3},
			"���� +20/r" },

			{ {sinDA1,sinDA2,3,3,3,1,1,1},
			{2,3,4,5,6,1,1,1,1},
			{SIN_ADD_ABSORB,2,2,2,2,0,2,2},
			{0.6f,3.0f,0,0,0,0,0,0},
			{SIN_ADD_NUM,1,1,1,1,1,1,2},
			"����� +0.6/r" },

			{ {sinDA1,sinDA2,0,3,0,0,2,1},
			{1,1,1,0,0,0,0,1,3},
			{SIN_ADD_STAMINA,3,0,1,0,0,0,0},
			{10,0,6,6,6,6,7,7},
			{SIN_ADD_NUM,5,6,7,8,9,1,1},
			"�ٷ� +10/r" },

			{ {sinDA1,sinDA2,0,1,2,0,0,0},
			{2,2,0,1,2,2,2,1,0},
			{SIN_ADD_DEFENCE,0,2,2,0,5,0,5},
			{0,1,0,1,1,1,1,1},
			{SIN_ADD_PERCENT,1,0,5,0,0,5,0},
			"���� +10%/r" },

			{ {sinDA1,sinDA2,3,0,3,3,0,2},
			{1,1,1,0,1,0,1,2,2},
			{SIN_ADD_DEFENCE,1,2,3,4,5,5,6},
			{1,0,0,2,0,1,2,1},
			{SIN_ADD_NUM,1,0,0,0,1,0,2},
			"���� +20/r" },

			{ {sinDA1,sinDA2,0,1,0,2,0,2},
			{0,2,1,1,1,1,1,1,1},
			{SIN_ADD_DEFENCE,10,20,30,40,50,60,70},
			{15,20,10,10,10,10,10,10},
			{SIN_ADD_PERCENT,10,20,1,0,1,1,1},
			"���� +15%/r" },

			{ {sinDA1,sinDA2,1,6,6,6,6,5},
			{0,3,0,3,0,2,2,3,3},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,1,2,3,4,5,6},
			{30,0.6f,1,1,1,1,1,1},
			{SIN_ADD_NUM,SIN_ADD_NUM,4,4,4,0,4,1},
			"���� +30/r����� +0.6/r" },

			{ {sinDA1,sinDA2,2,3,4,3,0,3},
			{0,1,0,2,1,1,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,4,0,4,0,4,4},
			{40,0.4f,4,4,4,4,4,4},
			{SIN_ADD_NUM,SIN_ADD_NUM,1,2,3,4,5,7},
			"���� +40/r����� +0.4/r" },

			{ {sinDA1,sinDA2,8,0,0,7,0,8},
			{0,1,8,0,8,8,8,8,8},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,1,2,3,4,5,8},
			{5,1.0f,0,1,1,0,2,2},
			{SIN_ADD_NUM,SIN_ADD_NUM,1,2,1,0,1,1},
			"���� +45/r����� +1.0/r" },

			{ {sinDA1,sinDA2,4,4,4,4,3,2},
			{0,1,0,4,0,0,1,4,4},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,3,3,3,3,2,2},
			{10,1.5f,0,1,0,1,0,1},
			{SIN_ADD_NUM,SIN_ADD_NUM,1,2,3,4,5,6},
			"���� +10/r����� +1.5/r" },

			{ {sinDA1,sinDA2,2,2,2,2,2,2},
			{0,1,1,1,1,1,2,2,2},
			{SIN_ADD_ABSORB,7,7,5,5,5,5,5},
			{2.0f,4,0,0,0,4,0,4},
			{SIN_ADD_NUM,0,0,4,4,0,4,0},
			"����� +2.0/r" },

			{ {sinDA1,sinDA2,1,2,3,4,3,0},
			{0,0,0,0,1,2,2,0,0},
			{SIN_ADD_ABSORB,10,0,5,0,3,0,0},
			{3.0f,0,3,3,3,34,42,0},
			{SIN_ADD_NUM,0,0,1,1,0,1,1},
			"����� +3.0/r" },

			{ {sinDA1,sinDA2,0,7,1,1,1,1},
			{0,1,0,0,1,1,3,0,7},
			{SIN_ADD_ABSORB,0,0,7,4,7,0,4},
			{3.5f,0,0,0,4,4,4,4},
			{SIN_ADD_NUM,1,0,0,2,0,2,2},
			"����� +3.5/r" },

			{ {sinDA1,sinDA2,0,0,6,6,0,6},
			{0,0,4,0,4,0,3,6,6},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,7,0,7,7,7,7},
			{40,3.0f,1,1,0,3,3,3},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,2,2,0,1,1},
			"���� +40/r����� +3.0/r" },

			{ {sinDA1,sinDA2,2,3,4,5,6,7},
			{0,0,0,0,0,3,3,0,0},
			{SIN_ADD_ABSORB,0,0,0,0,1,2,3},
			{30,5,5,5,5,5,5,5},
			{SIN_ADD_PERCENT,0,0,1,1,1,1,1},
			"����� +30%/r" },

			{ {sinDA1,sinDA2,5,5,5,4,4,4},
			{0,0,0,1,0,1,1,0,4},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,1,2,3,4,5,6},
			{20,3.4f,1,1,1,1,1,1},
			{SIN_ADD_NUM,SIN_ADD_NUM,2,2,2,2,2,1},
			"���� +20/r����� +3.4/r" },

			{ {sinDA1,sinDA2,0,0,0,7,7,7},
			{7,7,7,7,1,7,0,4,7},
			{SIN_ADD_LIFE,1,2,3,5,4,4,0},
			{40,0,4,3,2,1,0,1},
			{SIN_ADD_NUM,0,1,1,1,1,1,0},
			"����� +40/r" },

			{ {sinDA1,sinDA2,3,3,3,3,2,0},
			{0,0,0,1,3,3,3,3,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,2,2,2,2,0,0},
			{40,30,0,0,2,2,2,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,1,2,3,4,5,6},
			"���� +40/r��� +30/r" },

			{ {sinDA1,sinDA2,0,0,0,3,2,1},
			{0,0,0,0,1,0,3,2,1},
			{SIN_ADD_ABSORB,SIN_ADD_LIFE,7,8,8,8,8,8},
			{3.5f,20,0,0,5,5,4,4},
			{SIN_ADD_NUM,SIN_ADD_NUM,6,5,4,3,2,1},
			"����� +3.5/r����� +20/r" },

			{ {sinDA1,sinDA2,0,5,4,4,3,2},
			{2,2,2,2,2,2,1,1,1},
			{SIN_ADD_BIO,SIN_ADD_FIRE,SIN_ADD_ICE,SIN_ADD_LIGHTNING,SIN_ADD_POISON,10,10,10},
			{5,5,1,1,1,0,1,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,12,12,12},
			"�� �Ӽ� +5/r�� �Ӽ� +5/r�� �Ӽ� +5/rȭ �Ӽ� +5/r�� �Ӽ� +5/r" },




			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{50,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +50/r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,0,2,2,2},
			{SIN_ADD_LIFE,0,0,0,0,0,0,0},
			{50,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +50/r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,2,0,2,0,3,3},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,0,0,0,0,0,0},
			{50,40,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +50/r��� +40/r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,3,0,3,1,3},
			{SIN_ADD_ABSORB,SIN_ADD_LIFE,0,0,0,0,0,0},
			{4.0f,25,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +4.0/r����� +25/r" },



			{ {sinDS1,0,0,0,0,1,0,0},
			{5,0,0,3,3,3,3,3,2},
			{SIN_ADD_BIO,1,2,3,4,5,6,7},
			{5,5,5,5,5,5,0,0},
			{SIN_ADD_NUM,1,2,2,2,2,2,0},
			"�� �Ӽ� +5/r" },

			{ {sinDS1,1,1,1,1,1,1,1},
			{0,1,1,1,1,1,1,2,0},
			{SIN_ADD_POISON,0,2,2,3,3,3,3},
			{4,4,0,0,0,4,4,4},
			{SIN_ADD_NUM,1,2,3,4,5,6,7},
			"�� �Ӽ� +5/r" },

			{ {sinDS1,7,7,7,7,7,7,7},
			{0,0,5,0,0,7,7,3,3},
			{SIN_ADD_ICE,0,0,3,3,3,0,0},
			{5,0,2,2,2,2,2,1},
			{SIN_ADD_NUM,1,1,1,1,1,0,0},
			"�� �Ӽ� +5/r" },

			{ {sinDS1,0,3,3,3,3,3,2},
			{0,1,1,5,1,1,1,0,0},
			{SIN_ADD_FIRE,1,2,3,4,5,6,7},
			{1,0,1,1,1,0,0,1},
			{SIN_ADD_NUM,1,1,1,1,1,1,0},
			"ȭ �Ӽ� +5/r" },

			{ {sinDS1,0,0,3,2,2,0,2},
			{0,0,0,0,5,0,3,3,3},
			{SIN_ADD_LIGHTNING,3,1,0,3,0,3,1},
			{5,5,5,5,5,5,5,5},
			{SIN_ADD_NUM,0,1,0,1,0,1,0},
			"�� �Ӽ� +5/r" },

			{ {sinDS1,2,2,2,2,1,2,2},
			{3,3,3,3,3,1,2,2,2},
			{SIN_ADD_DEFENCE,1,2,3,4,5,6,7},
			{10,10,10,10,10,10,10,10},
			{SIN_ADD_NUM,1,2,3,4,5,6,7},
			"���� +10/r" },

			{ {sinDS1,0,0,2,2,2,2,0},
			{0,0,0,0,0,0,1,0,0},
			{SIN_ADD_BLOCK_RATE,3,3,0,0,0,4,4},
			{1,0,0,3,3,3,3,3},
			{SIN_ADD_NUM,1,3,1,1,1,1,1},
			"����� +1/r" },

			{ {sinDS1,0,0,1,2,2,2,2},
			{0,0,2,2,2,2,1,1,2},
			{SIN_ADD_LIFE,1,2,3,3,3,3,3},
			{1,5,5,0,0,0,0,5},
			{SIN_ADD_NUM,0,0,6,6,6,2,1},
			"����� +10/r" },

			{ {sinDS1,1,1,1,1,0,0,2},
			{2,0,1,0,2,2,2,2,0},
			{SIN_ADD_DEFENCE,2,0,0,2,0,2,2},
			{10,8,8,8,8,8,1,1},
			{SIN_ADD_NUM,0,0,2,3,3,0,3},
			"���� +10/r" },

			{ {sinDS1,6,0,5,5,3,0,1},
			{0,2,2,1,0,3,4,5,5},
			{SIN_ADD_DEFENCE,1,2,3,4,5,6,7},
			{10,10,10,10,10,10,10,10},
			{SIN_ADD_PERCENT,11,10,11,10,0,10,10},
			"���� +10%/r" },

			{ {sinDS1,1,1,1,1,0,0,2},
			{2,0,1,0,2,2,2,2,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,3,3,3,1,1,1},
			{15,0.3f,0,0,0,1,1,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,2,0,2,0,2,2},
			"���� +15/r����� +0.3/r" },

			{ {sinDS1,0,0,2,2,1,0,0},
			{0,0,2,0,2,0,2,2,1},
			{SIN_ADD_ABSORB,0,3,3,3,3,0,0},
			{0.3f,0,0,1,0,1,1,1},
			{SIN_ADD_NUM,0,2,2,2,2,2,2},
			"����� +0.3/r" },

			{ {sinDS1,1,2,3,4,5,6,7},
			{0,1,0,0,1,2,0,1,1},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,2,2,2,2,2,2},
			{10,0.6f,1,0,3.0f,10.0f,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,1,1,1,1,2,2},
			"���� +10/r����� +0.6/r" },

			{ {sinDS1,0,0,0,2,0,2,1},
			{0,2,2,2,1,1,2,0,2},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,1,2,3,4,5,6},
			{20,0.3f,6,7,6,8,8,8},
			{SIN_ADD_NUM,SIN_ADD_NUM,3,3,3,0,3,3},
			"���� +20/r����� +0.3/r" },

			{ {sinDS1,7,6,5,4,3,2,1},
			{0,0,1,0,4,0,1,1,0},
			{SIN_ADD_ABSORB,SIN_ADD_BLOCK_RATE,0,3,0,3,0,3},
			{0.6f,7,0,6,7,7,3,2},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,1,1,0,1,1},
			"����� +0.6/r����� 4%/r" },

			{ {sinDS1,0,4,4,3,3,2,2},
			{0,0,5,5,5,5,1,1,0},
			{SIN_ADD_BLOCK_RATE,3,0,3,3,3,3,2},
			{5,0,0,0,5,5,5,5},
			{SIN_ADD_NUM,0,3,0,3,3,0,3},
			"����� +5%/r" },

			{ {sinDS1,0,0,2,2,2,0,2},
			{0,4,4,0,0,0,1,1,1},
			{SIN_ADD_DEFENCE,SIN_ADD_BLOCK_RATE,7,7,7,7,7,7},
			{40,2,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,5,5,5,6,6,7},
			"���� +40/r����� +2%/r" },

			{ {sinDS1,0,4,5,4,3,2,2},
			{0,2,1,1,0,1,2,0,1},
			{SIN_ADD_DEFENCE,SIN_ADD_BLOCK_RATE,2,0,2,0,2,0},
			{20,4,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,1,0,2,0,3,0},
			"���� +20/r����� +4%/r" },

			{ {sinDS1,1,1,0,2,2,0,0},
			{4,1,0,1,0,1,1,1,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,SIN_ADD_STAMINA,0,0,0,0,0},
			{40,10,50,1,0,1,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,7,7,0,0},
			"���� +40/r��� +10/r�ٷ� +50/r" },

			{ {sinDS1,0,0,0,2,0,0,1},
			{0,0,0,0,0,3,1,2,0},
			{SIN_ADD_BLOCK_RATE,SIN_ADD_LIFE,0,0,3,3,3,0},
			{3,20,0,0,3,3,0,3},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,1,0,3,0,3},
			"����� +3%/r����� +20/r" },

			{ {sinDS1,0,3,0,0,4,0,0},
			{0,0,0,0,5,0,0,2,0},
			{SIN_ADD_ABSORB,SIN_ADD_MANA,0,0,4,3,2,1},
			{1.0f,20,0,0,0,0,4,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,4,0,1,0},
			"����� +1.0/r��� +20/r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,3,1,0,2,0},
			{SIN_ADD_ABSORB,SIN_ADD_LIFE,3,0,0,0,0,0},
			{0.6f,40,0,0,0,0,3,3},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,3,0,3,3},
			"����� +0.6/r����� +40/r" },

			{ {sinDS1,0,5,5,5,0,0,0},
			{2,2,2,2,2,2,0,0,0},
			{SIN_ADD_BIO,SIN_ADD_POISON,SIN_ADD_LIGHTNING,SIN_ADD_FIRE,SIN_ADD_ICE,0,0,0},
			{5,5,5,5,5,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0},
			"�� �Ӽ� +5/r�� �Ӽ� +5/r�� �Ӽ� +5/rȭ �Ӽ� +5/r�� �Ӽ� +5/r" },




			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1},
			{SIN_ADD_LIFE,0,0,0,0,0,0,0},
			{40,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +40/r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,1,3,0,2,0,3},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,SIN_ADD_STAMINA,0,0,0,0,0},
			{50,20,60,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���� +50/r��� +20/r�ٷ� +60/r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,2,3,3,0,2},
			{SIN_ADD_BLOCK_RATE,SIN_ADD_LIFE,0,0,0,0,0,0},
			{4,30,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +4%/r����� +30/r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,3,1,2,2},
			{SIN_ADD_ABSORB,SIN_ADD_MANA,0,0,0,0,0,0},
			{2.0f,30,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +2.0/r��� +30/r" },



			{ {sinOM1,5,0,0,0,0,0,5},
			{5,0,0,5,5,5,0,0,0},
			{SIN_ADD_BIO,0,0,0,0,5,0,0},
			{5,0,0,0,0,0,0,5},
			{SIN_ADD_NUM,0,5,0,0,0,0,0},
			"�� �Ӽ� +5/r" },

			{ {sinOM1,0,4,5,5,3,0,0},
			{0,5,0,0,0,0,0,0,3},
			{SIN_ADD_POISON,3,3,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,3,0,0,0,0},
			"�� �Ӽ� +5/r" },

			{ {sinOM1,0,0,0,0,5,0,0},
			{0,0,5,0,0,0,3,3,4},
			{SIN_ADD_ICE,0,3,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,5,0,0,0,0},
			"�� �Ӽ� +5/r" },

			{ {sinOM1,0,0,0,0,5,0,0},
			{0,0,0,5,0,0,0,0,5},
			{SIN_ADD_FIRE,0,5,0,0,0,0,0},
			{5,0,0,0,5,5,5,5},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"ȭ �Ӽ� +5/r" },

			{ {sinOM1,5,0,0,1,0,1,0},
			{0,0,0,5,5,0,0,0,0},
			{SIN_ADD_LIGHTNING,0,0,0,0,0,0,0},
			{5,0,0,1,0,0,0,1},
			{SIN_ADD_NUM,0,1,0,0,0,0,0},
			"�� �Ӽ� +5/r" },

			{ {sinOM1,0,0,0,2,0,1,2},
			{0,0,0,0,0,1,0,0,0},
			{SIN_ADD_MANAREGEN,0,2,2,0,0,0,0},
			{0.3f,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,1},
			"������ +0.3/r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,1,0,0},
			{SIN_ADD_DEFENCE,3,0,0,6,1,0,0},
			{10,0,0,0,0,0,0,3},
			{SIN_ADD_NUM,0,0,0,3,0,6,0},
			"���� +10/r" },

			{ {sinOM1,0,0,0,0,3,0,0},
			{0,0,0,0,0,0,0,1,3},
			{SIN_ADD_MANA,0,3,0,0,0,0,0},
			{20,0,0,0,0,1,0,3},
			{SIN_ADD_NUM,2,3,0,0,0,0,0},
			"��� +20/r" },

			{ {sinOM1,0,0,0,0,2,2,0},
			{2,0,1,0,0,0,0,0,0},
			{SIN_ADD_DEFENCE,0,2,0,0,0,0,0},
			{10,0,0,0,0,0,0,1},
			{SIN_ADD_NUM,0,0,1,0,0,0,0},
			"���� +10/r" },

			{ {sinOM1,0,0,0,0,1,0,0},
			{0,2,2,1,0,0,0,0,1},
			{SIN_ADD_DEFENCE,0,1,1,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,1,0,1,0,0,0,0},
			"���� +10%/r" },

			{ {sinOM1,0,0,0,0,2,2,2},
			{0,3,1,0,2,0,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,4,4,0,0,0,0},
			{15,0.4f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,4,0,4,0},
			"���� +15/r����� +0.4/r" },

			{ {sinOM1,0,0,0,4,4,0,0},
			{0,0,4,0,4,0,0,4,0},
			{SIN_ADD_ABSORB,7,7,0,0,0,0,0},
			{0.8f,0,0,0,0,0,0,7},
			{SIN_ADD_NUM,0,0,7,0,0,0,0},
			"����� +0.8/r" },

			{ {sinOM1,0,0,0,0,7,0,0},
			{0,1,0,0,2,2,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,8,8,0,0,0,0},
			{10,0.4f,0,0,0,0,8,8},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,5,0,0},
			"���� +10/r������ +0.4/r" },

			{ {sinOM1,0,0,0,0,0,0,5},
			{0,2,2,2,2,1,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,5,0,0,0,0,0},
			{20,0.2f,0,0,5,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,5},
			"���� +20/r������ +0.2/r" },

			{ {sinOM1,0,0,3,0,0,3,0},
			{0,0,1,0,4,0,1,0,0},
			{SIN_ADD_DEFENCE,0,3,0,0,0,0,0},
			{40,0,0,0,0,0,0,3},
			{SIN_ADD_NUM,0,3,0,0,0,0,0},
			"���� +40/r" },

			{ {sinOM1,0,0,0,0,2,0,1},
			{0,0,3,0,0,3,1,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,0,1,0,2,0,3},
			{10,0.6f,0,0,3,3,3,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,5,5,5,5,0},
			"���� +10/r������ +0.6/r" },

			{ {sinOM1,0,3,0,0,3,0,1},
			{0,5,5,0,0,0,1,0,3},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,2,2,0,0,0},
			{50,1.0f,0,0,0,3,3,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,2},
			"���� +50/r����� +1.0/r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,2,1,1,0,1,2,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,0,1,3,3,3,0},
			{40,0.4f,0,1,3,0,3,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +40/r������ +0.4/r" },

			{ {sinOM1,0,3,2,0,1,0,0},
			{5,1,0,0,0,2,0,2,0},
			{SIN_ADD_MANAREGEN,SIN_ADD_MANA,0,9,0,0,9,0},
			{0.5f,40,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,9,0,9,0},
			"������ +0.5/r��� +40/r" },

			{ {sinOM1,0,0,2,2,0,2,0},
			{0,0,0,0,0,2,1,2,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,SIN_ADD_STAMINA,1,0,0,0,1},
			{10,0.4f,40,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,1,0,0,2,2},
			"���� +10/r������ +0.4/r�ٷ� +40/r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,2,5,0,0,2,0},
			{SIN_ADD_MANA,SIN_ADD_STAMINA,0,0,1,1,1,1},
			{50,50,0,5,0,0,5,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,3,0,0,3,3},
			"��� +50/r�ٷ� +50/r" },

			{ {sinOM1,5,0,0,5,5,0,0},
			{0,0,0,0,5,2,0,2,5},
			{SIN_ADD_ABSORB,SIN_ADD_MANAREGEN,SIN_ADD_LIFE,0,0,0,0,0},
			{1.0f,0.6f,20,0,4,0,4,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"����� +1.0/r������ +0.6/r����� +20/r" },

			{ {sinOM1,0,0,2,0,0,2,0},
			{2,2,2,2,2,2,0,0,0},
			{SIN_ADD_BIO,SIN_ADD_POISON,SIN_ADD_ICE,SIN_ADD_FIRE,SIN_ADD_LIGHTNING,0,0,0},
			{5,5,5,5,5,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0},
			"�� �Ӽ� +5/r�� �Ӽ� +5/r�� �Ӽ� +5/rȭ �Ӽ� +5/r�� �Ӽ� +5/r" },




			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1},
			{SIN_ADD_MANA,0,0,0,0,0,0,0},
			{30,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"��� +30/r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,2,0,0,3,2,2},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,SIN_ADD_STAMINA,0,0,0,0,0},
			{20,0.8f,50,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���� +20/r������ +0.8/r�ٷ� +50/r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,1,1,2,2,3},
			{SIN_ADD_MANA,SIN_ADD_STAMINA,0,0,0,0,0,0},
			{60,50,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"��� +60/r�ٷ� +50/r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,2,0,0,3,3},
			{SIN_ADD_ABSORB,SIN_ADD_MANAREGEN,SIN_ADD_LIFE,0,0,0,0,0},
			{1.5f,1.0f,30,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"����� +1.5/r������ +1.0/r����� +30/r" },




		#endif

		#ifdef	_LANGUAGE_CHINESE
		#include "Language/chinese/C_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_JAPANESE
		#include "systems/localization/locales/Japanese/J_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_TAIWAN
		#include "Language/Taiwan/T_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_ENGLISH
		#include "Language/English/E_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_JAPANESE
		#include "systems/localization/locales/Japanese/J_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_TAIWAN
		#include "Language/Taiwan/T_sinCraftItem.h"
		#endif
		#ifdef	_LANGUAGE_THAI
		#include "Language/THAI/TH_sinCraftItem.h"


		#endif
		#ifdef _LANGUAGE_VEITNAM
		#include "Language/VEITNAM/V_sinCraftItem.h"
		#endif

		#ifdef _LANGUAGE_BRAZIL
        #include "b_sinCraftItem.h"
		#endif

		#ifdef _LANGUAGE_ARGENTINA
		#include "Language/ARGENTINA/A_sinCraftItem.h"
		#endif


