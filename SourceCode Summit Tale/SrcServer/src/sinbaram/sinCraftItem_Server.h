

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

#include "Language\\Language.h"

#ifdef	_LANGUAGE_KOREAN



















{
	{
		sinWA1, sinWC1, sinWH1, sinWM1, sinWP1, sinWS1, sinWS2, sinWT1
	},
	{ 5,0,0,0,0,0,0,0,0 },
	{ SIN_ADD_BIO,0,0,0,0,0,0,0 },
	{ 5,0,0,0,0,0,0,0 },
	{ SIN_ADD_NUM,0,0,0,0,0,0,0 },
			"�� �Ӽ� +5\r"},

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,5,0,0,0,0,0,0,0},
			{SIN_ADD_POISON,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,5,0,0,0,0,0,0},
			{SIN_ADD_ICE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,5,0,0,0,0,0},
			{SIN_ADD_FIRE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"ȭ �Ӽ� +5\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,5,0,0,0,0},
			{SIN_ADD_LIGHTNING,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,0,1,0,0,0},
			{SIN_ADD_CRITICAL,0,0,0,0,0,0,0},
			{1,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"ũ��Ƽ�� +1%\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,0,0,1,0,0},
			{SIN_ADD_ATTACK_RATE,0,0,0,0,0,0,0},
			{50,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���߷� +50\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,0,0,0,1,0},
			{SIN_ADD_STAMINA,0,0,0,0,0,0,0},
			{20,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�ٷ� +20\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,1,0,0,0,0,0,0,0},
			{SIN_ADD_ATTACK_RATE,0,0,0,0,0,0,0},
			{20,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,0,0,0,0,0,0,0},
			"���߷� +20%\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{3,3,0,0,0,0,0,0,0},
			{SIN_ADD_ATTACK_RATE,0,0,0,0,0,0,0},
			{40,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,0,0,0,0,0,0,0},
			"���߷� +40%\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,0,2,0,0,0,0,0,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,0,0,0,0,0,0},
			{10,1,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���߷� +10\r�ּ� ���ݷ� +1\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,0,0,2,0,0,0,0,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MAX,0,0,0,0,0,0},
			{10,1,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���߷� +10\r�ִ� ���ݷ� +1\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{2,0,2,2,0,0,0,0,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0},
			{20,1,1,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���߷� +20\r�ּ� ���ݷ� +1\r�ִ� ���ݷ� +1\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,0,0,1,1,0,0,0,0},
			{SIN_ADD_ATTACK_RATE,0,0,0,0,0,0,0},
			{100,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,0,0,0,0,0,0,0},
			"���߷� +100%\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,2,0,0,0,0,0,0},
			{SIN_ADD_DAMAGE_MIN,0,0,0,0,0,0,0},
			{1,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�ּ� ���ݷ� +1\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,0,2,0,0,0,0,0},
			{SIN_ADD_DAMAGE_MAX,0,0,0,0,0,0,0},
			{2,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�ִ� ���ݷ� +2\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,2,2,0,1,0,0,0},
			{SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0},
			{2,1,1,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"ũ��Ƽ�� +2%\r�ּ� ���ݷ� +1\r�ִ� ���ݷ� +1\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,0,4,4,0,0,0,0},
			{SIN_ADD_DAMAGE_MAX,0,0,0,0,0,0,0},
			{4,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�ִ� ���ݷ� +4\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,1,0,4,4,0,0,0,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MAX,0,0,0,0,0,0},
			{10,4,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,SIN_ADD_NUM,0,0,0,0,0,0},
			"���߷� +10%\r�ִ� ���ݷ� +4\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,2,4,1,0,0,0,0},
			{SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0,0},
			{1,3,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"�ּ� ���ݷ� +1\r�ִ� ���ݷ� +3\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,4,2,1,0,0,0,0},
			{SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0,0},
			{2,2,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"�ּ� ���ݷ� +2\r�ִ� ���ݷ� +2\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,2,2,1,3,0,0,0},
			{SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0},
			{4,3,3,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"ũ��Ƽ�� �� +4%\r�ּ� ���ݷ� +3\r�ִ� ���ݷ� +3\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{1,1,2,4,1,0,0,0,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0},
			{10,1,4,0,0,0,0,0},
			{SIN_ADD_PERCENT,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���߷� +10%\r�ּ� ���ݷ� +1\r�ִ� ���ݷ� +4\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,1,3,5,0,0,1,0,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,0},
			{40,3,5,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���߷� +40\r�ּ� ���ݷ� +3\r�ִ� ���ݷ� +5\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,5,3,2,0,0},
			{SIN_ADD_CRITICAL,SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MAX,0,0,0,0,0},
			{4,60,6,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_PERCENT,SIN_ADD_NUM,0,0,0,0,0},
			"ũ��Ƽ�� +4%\r���߷� +60%\r�ִ� ���ݷ� +6\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,2,2,2,2,2,0,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0},
			{80,2,4,4,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"���߷� +80\rũ��Ƽ�� +2%\r�ּ� ���ݷ� +4\r�ִ� ���ݷ� +4\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,3,3,0,2,0,2,0},
			{SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,SIN_ADD_MANA,0,0,0,0},
			{4,6,6,20,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"ũ��Ƽ�� +4%\r�ּ� ���ݷ� +6\r�ִ� ���ݷ� +6\r��� +20\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,2,2,0,0,1,3,0},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,SIN_ADD_LIFE,0,0,0,0},
			{60,6,6,20,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"���߷� +60\r�ּ� ���ݷ� +6\r�ִ� ���ݷ� +6\r����� +20\r" },




			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,0,0,0,0,1},
			{SIN_ADD_CRITICAL,0,0,0,0,0,0,0},
			{4,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"ũ��Ƽ�� +4%\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,2,0,0,2,3,3},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{100,3,6,6,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"���߷� +100\rũ��Ƽ�� +3%\r�ּ� ���ݷ� +6\r�ִ� ���ݷ� +6\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,1,0,4,0,2,4},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_LIFE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{80,30,7,7,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"���߷� +80\r����� +30\r�ּ� ���ݷ� +7\r�ִ� ���ݷ� +7\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,2,1,2,3,2},
			{SIN_ADD_CRITICAL,SIN_ADD_MANA,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{5,30,7,7,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"ũ��Ƽ�� +5%\r��� +30\r�ּ� ���ݷ� +7\r�ִ� ���ݷ� +7\r" },


			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,0,0,5,4,3},
			{SIN_ADD_CRITICAL,SIN_ADD_MANA,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{4,50,7,7,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"ũ��Ƽ�� +4%\r��� +50\r�ּ� ���ݷ� +7\r�ִ� ���ݷ� +7\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,0,0,0,5,5},
			{SIN_ADD_CRITICAL,SIN_ADD_LIFE,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{4,50,7,7,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"ũ��Ƽ�� +4%\r����� +50\r�ּ� ���ݷ� +7\r�ִ� ���ݷ� +7\r" },

			{ {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1},
			{0,0,0,0,0,1,1,5,5},
			{SIN_ADD_ATTACK_RATE,SIN_ADD_CRITICAL,SIN_ADD_DAMAGE_MIN,SIN_ADD_DAMAGE_MAX,0,0,0,0,},
			{80,5,7,7,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0},
			"���߷� +80\rũ��Ƽ�� +5%\r�ּ� ���ݷ� +7\r�ִ� ���ݷ� +7\r" },






			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0,0},
			{SIN_ADD_BIO,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,5,0,0,0,0,0,0,0},
			{SIN_ADD_POISON,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,5,0,0,0,0,0,0},
			{SIN_ADD_ICE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,5,0,0,0,0,0},
			{SIN_ADD_FIRE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"ȭ �Ӽ� +5\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,5,0,0,0,0},
			{SIN_ADD_LIGHTNING,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,1,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{20,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +20\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,0,1,0,0},
			{SIN_ADD_ABSORB,0,0,0,0,0,0,0},
			{0.6f,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +0.6\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,1,0},
			{SIN_ADD_STAMINA,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�ٷ� +10\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{2,2,0,0,0,0,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,0,0,0,0,0,0,0},
			"���� +10%\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{1,0,2,0,0,0,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{20,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +20\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,2,1,1,0,0,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{15,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,0,0,0,0,0,0,0},
			"���� +15%\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,3,0,3,0,0,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{30,0.6f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +30\r����� +0.6\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,1,0,2,1,1,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{40,0.4f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +40\r����� +0.4\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,1,0,0,2,5,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{45,1.0f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +45\r����� +1.0\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,1,0,4,0,0,1,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{10,1.5f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +10\r����� +1.5\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,1,0,0,0,0,2,0,0},
			{SIN_ADD_ABSORB,0,0,0,0,0,0,0},
			{2.0f,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +2.0\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,1,2,2,0,0},
			{SIN_ADD_ABSORB,0,0,0,0,0,0,0},
			{3.0f,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +3.0\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,1,0,0,1,1,3,0,0},
			{SIN_ADD_ABSORB,0,0,0,0,0,0,0},
			{3.5f,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +3.5\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,4,0,4,0,3,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{40,3.0f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +40\r����� +3.0\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,3,3,0,0},
			{SIN_ADD_ABSORB,0,0,0,0,0,0,0},
			{30,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,0,0,0,0,0,0,0},
			"����� +30%\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,2,0,2,2,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{20,3.4f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +20\r����� +3.4\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,1,0,0,4,0},
			{SIN_ADD_LIFE,0,0,0,0,0,0,0},
			{40,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +40\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,1,1,2,0,3,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,0,0,0,0,0,0},
			{40,30,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +40\r��� +30\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,1,0,3,2,0},
			{SIN_ADD_ABSORB,SIN_ADD_LIFE,0,0,0,0,0,0},
			{3.5f,20,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +3.5\r����� +20\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{2,2,2,2,2,2,0,0,0},
			{SIN_ADD_BIO,SIN_ADD_FIRE,SIN_ADD_ICE,SIN_ADD_LIGHTNING,SIN_ADD_POISON,0,0,0},
			{5,5,5,5,5,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0},
			"�� �Ӽ� +5\r�� �Ӽ� +5\r�� �Ӽ� +5\rȭ �Ӽ� +5\r�� �Ӽ� +5\r" },




			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{50,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +50\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,0,2,2,2},
			{SIN_ADD_LIFE,0,0,0,0,0,0,0},
			{50,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +50\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,2,0,2,0,3,3},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,0,0,0,0,0,0},
			{50,40,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +50\r��� +40\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,3,0,3,1,3},
			{SIN_ADD_ABSORB,SIN_ADD_LIFE,0,0,0,0,0,0},
			{4.0f,25,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +4.0\r����� +25\r" },


			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,0,1,2,5},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{50,4.0f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +50\r����� +4.0\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,5,5},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,0,0,0,0,0,0},
			{60,50,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +60\r��� +50\r" },

			{ {sinDA1,sinDA2,0,0,0,0,0,0},
			{0,0,0,0,0,1,5,1,5},
			{SIN_ADD_ABSORB,SIN_ADD_LIFE,0,0,0,0,0,0},
			{4.0f,40,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +4.0\r����� +40\r" },



			{ {sinDS1,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0,0},
			{SIN_ADD_BIO,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,5,0,0,0,0,0,0,0},
			{SIN_ADD_POISON,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,5,0,0,0,0,0,0},
			{SIN_ADD_ICE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,5,0,0,0,0,0},
			{SIN_ADD_FIRE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"ȭ �Ӽ� +5\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,5,0,0,0,0},
			{SIN_ADD_LIGHTNING,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,1,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +10\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,1,0,0},
			{SIN_ADD_BLOCK_RATE,0,0,0,0,0,0,0},
			{1,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +1\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,1,0},
			{SIN_ADD_LIFE,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +10\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{2,0,1,0,0,0,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +10\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,2,2,1,0,0,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,0,0,0,0,0,0,0},
			"���� +10%\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,3,1,0,1,0,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{15,0.3f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +15\r����� +0.3\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,2,0,2,0,0,0,0},
			{SIN_ADD_ABSORB,0,0,0,0,0,0,0},
			{0.3f,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +0.3\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,1,0,0,1,2,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{10,0.6f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +10\r����� +0.6\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,2,2,2,1,1,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{20,0.3f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +20\r����� +0.3\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,1,0,4,0,1,0,0},
			{SIN_ADD_ABSORB,SIN_ADD_BLOCK_RATE,0,0,0,0,0,0},
			{0.6f,4,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +0.6\r����� 4%\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,5,0,0,5,1,0,0},
			{SIN_ADD_BLOCK_RATE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +5%\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,4,4,0,0,0,1,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_BLOCK_RATE,0,0,0,0,0,0},
			{40,2,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +40\r����� +2%\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,2,1,1,0,1,2,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_BLOCK_RATE,0,0,0,0,0,0},
			{20,4,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +20\r����� +4%\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{4,1,0,0,0,1,0,1,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,SIN_ADD_STAMINA,0,0,0,0,0},
			{40,10,50,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���� +40\r��� +10\r�ٷ� +50\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,3,1,2,0},
			{SIN_ADD_BLOCK_RATE,SIN_ADD_LIFE,0,0,0,0,0,0},
			{3,20,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +3%\r����� +20\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,5,0,0,2,0},
			{SIN_ADD_ABSORB,SIN_ADD_MANA,0,0,0,0,0,0},
			{1.0f,20,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +1.0\r��� +20\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,3,1,0,2,0},
			{SIN_ADD_ABSORB,SIN_ADD_LIFE,0,0,0,0,0,0},
			{0.6f,40,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +0.6\r����� +40\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{2,2,2,2,2,2,0,0,0},
			{SIN_ADD_BIO,SIN_ADD_POISON,SIN_ADD_LIGHTNING,SIN_ADD_FIRE,SIN_ADD_ICE,0,0,0},
			{5,5,5,5,5,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0},
			"�� �Ӽ� +5\r�� �Ӽ� +5\r�� �Ӽ� +5\rȭ �Ӽ� +5\r�� �Ӽ� +5\r" },




			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1},
			{SIN_ADD_LIFE,0,0,0,0,0,0,0},
			{40,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +40\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,1,3,0,2,0,3},
			{SIN_ADD_DEFENCE,SIN_ADD_MANA,SIN_ADD_STAMINA,0,0,0,0,0},
			{50,20,60,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���� +50\r��� +20\r�ٷ� +60\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,2,3,3,0,2},
			{SIN_ADD_BLOCK_RATE,SIN_ADD_LIFE,0,0,0,0,0,0},
			{4,30,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +4%\r����� +30\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,3,1,2,2},
			{SIN_ADD_ABSORB,SIN_ADD_MANA,0,0,0,0,0,0},
			{2.0f,30,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +2.0\r��� +30\r" },


			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,3,2,3},
			{SIN_ADD_DEFENCE,SIN_ADD_BLOCK_RATE,0,0,0,0,0,0},
			{60,5,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +60\r����� +5%\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,5,5},
			{SIN_ADD_ABSORB,SIN_ADD_LIFE,SIN_ADD_MANA,0,0,0,0,0},
			{2.0f,20,20,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"����� +2.0\r����� +20\r��� +20\r" },

			{ {sinDS1,0,0,0,0,0,0,0},
			{0,0,0,0,0,2,2,2,5},
			{SIN_ADD_BLOCK_RATE,SIN_ADD_LIFE,0,0,0,0,0,0},
			{4,50,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"����� +4%\r����� +50\r" },



			{ {sinOM1,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0,0},
			{SIN_ADD_BIO,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,5,0,0,0,0,0,0,0},
			{SIN_ADD_POISON,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,5,0,0,0,0,0,0},
			{SIN_ADD_ICE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,5,0,0,0,0,0},
			{SIN_ADD_FIRE,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"ȭ �Ӽ� +5\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,5,0,0,0,0},
			{SIN_ADD_LIGHTNING,0,0,0,0,0,0,0},
			{5,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"�� �Ӽ� +5\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,1,0,0,0},
			{SIN_ADD_MANAREGEN,0,0,0,0,0,0,0},
			{0.3f,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"������ +0.3\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,1,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +10\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,1,0},
			{SIN_ADD_MANA,0,0,0,0,0,0,0},
			{20,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"��� +20\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{2,0,1,0,0,0,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +10\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,2,2,1,0,0,0,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{10,0,0,0,0,0,0,0},
			{SIN_ADD_PERCENT,0,0,0,0,0,0,0},
			"���� +10%\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,3,1,0,2,0,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{15,0.4f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +15\r����� +0.4\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,4,0,4,0,0,0,0},
			{SIN_ADD_ABSORB,0,0,0,0,0,0,0},
			{0.8f,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"����� +0.8\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,1,0,0,2,2,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,0,0,0,0,0,0},
			{10,0.4f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +10\r������ +0.4\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,2,2,2,2,1,0,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,0,0,0,0,0,0},
			{20,0.2f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +20\r������ +0.2\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,1,0,4,0,1,0,0},
			{SIN_ADD_DEFENCE,0,0,0,0,0,0,0},
			{40,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"���� +40\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,3,0,0,3,1,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,0,0,0,0,0,0},
			{10,0.6f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +10\r������ +0.6\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,5,5,0,0,0,1,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_ABSORB,0,0,0,0,0,0},
			{50,1.0f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +50\r����� +1.0\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,2,1,1,0,1,2,0,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,0,0,0,0,0,0},
			{40,0.4f,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"���� +40\r������ +0.4\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{5,1,0,0,0,2,0,2,0},
			{SIN_ADD_MANAREGEN,SIN_ADD_MANA,0,0,0,0,0,0},
			{0.5f,40,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"������ +0.5\r��� +40\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,2,1,2,0},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,SIN_ADD_STAMINA,0,0,0,0,0},
			{10,0.4f,40,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���� +10\r������ +0.4\r�ٷ� +40\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,2,5,0,0,2,0},
			{SIN_ADD_MANA,SIN_ADD_STAMINA,0,0,0,0,0,0},
			{50,50,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"��� +50\r�ٷ� +50\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,5,2,0,2,0},
			{SIN_ADD_ABSORB,SIN_ADD_MANAREGEN,SIN_ADD_LIFE,0,0,0,0,0},
			{1.0f,0.6f,20,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"����� +1.0\r������ +0.6\r����� +20\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{2,2,2,2,2,2,0,0,0},
			{SIN_ADD_BIO,SIN_ADD_POISON,SIN_ADD_ICE,SIN_ADD_FIRE,SIN_ADD_LIGHTNING,0,0,0},
			{5,5,5,5,5,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0},
			"�� �Ӽ� +5\r�� �Ӽ� +5\r�� �Ӽ� +5\rȭ �Ӽ� +5\r�� �Ӽ� +5\r" },




			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1},
			{SIN_ADD_MANA,0,0,0,0,0,0,0},
			{30,0,0,0,0,0,0,0},
			{SIN_ADD_NUM,0,0,0,0,0,0,0},
			"��� +30\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,2,0,0,3,2,2},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,SIN_ADD_STAMINA,0,0,0,0,0},
			{20,0.8f,50,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���� +20\r������ +0.8\r�ٷ� +50\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,1,1,2,2,3},
			{SIN_ADD_MANA,SIN_ADD_STAMINA,0,0,0,0,0,0},
			{60,50,0,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0,0},
			"��� +60\r�ٷ� +50\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,2,0,0,3,3},
			{SIN_ADD_ABSORB,SIN_ADD_MANAREGEN,SIN_ADD_LIFE,0,0,0,0,0},
			{1.5f,1.0f,30,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"����� +1.5\r������ +1.0\r����� +30\r" },


			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,2,2,4},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,SIN_ADD_STAMINA,0,0,0,0,0},
			{30,1.0f,50,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���� +30\r������ +1.0\r�ٷ� +50\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,5,5},
			{SIN_ADD_ABSORB,SIN_ADD_MANAREGEN,SIN_ADD_LIFE,0,0,0,0,0},
			{2.0f,1.2f,30,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"����� +2.0\r������ +1.2\r����� +30\r" },

			{ {sinOM1,0,0,0,0,0,0,0},
			{0,0,0,0,0,1,5,1,5},
			{SIN_ADD_DEFENCE,SIN_ADD_MANAREGEN,SIN_ADD_MANA,0,0,0,0,0},
			{20,1.2f,60,0,0,0,0,0},
			{SIN_ADD_NUM,SIN_ADD_NUM,SIN_ADD_NUM,0,0,0,0,0},
			"���� +20\r������ +1.2\r��� +60\r" },


		#endif

		#ifdef	_LANGUAGE_CHINESE
		#include "Language\\chinese\\C_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_JAPANESE
		#include "Language\\Japanese\\J_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_TAIWAN
		#include "Language\\Taiwan\\T_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_ENGLISH
		#include "Language\\English\\E_sinCraftItem.h"
		#endif

		#ifdef	_LANGUAGE_THAI
		#include "Language\\THAI\\th_sinCraftItem.h"
		#endif

		#ifdef _LANGUAGE_VEITNAM
		#include "..\\VEITNAM\\V_sinCraftItem.h"
		#endif

		#ifdef _LANGUAGE_BRAZIL
        #include "B_sinCraftItem.h"
		#endif

		#ifdef _LANGUAGE_ARGENTINA
		#include "Language\\ARGENTINA\\A_sinCraftItem.h"
		#endif


