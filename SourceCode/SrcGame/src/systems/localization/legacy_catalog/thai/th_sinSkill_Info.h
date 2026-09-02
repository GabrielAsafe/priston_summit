

#define SIN_SKILL_USE_RIGHT	0x01000000		
#define SIN_SKILL_USE_LEFT		0x02000000		
#define SIN_SKILL_USE_ALL		0x03000000		
#define SIN_SKILL_USE_NOT		0x04000000		


{ "Extreme Shield" ,		 "������ѧ��ͧ�ѹ���Ѻ��� " ,
10,		34,2,		110,4,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Extreme_Shield,SKILL_EXTREME_SHIELD,SIN_SKILL_USE_RIGHT,E_Shield_UseMana},

{ "Mechanic Bomb" ,	 "���ҧ���Դ " ,
12,		28,2,		73,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Mechanic_Bomb,SKILL_MECHANIC_BOMB,SIN_SKILL_USE_RIGHT,Mechanic_Bomb_UseMana},

{ "Poison Attribute" ,	 "��ͧ�ѹ��ɶ��� ",
14,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Poison_Attribute,SKILL_POISON_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{ "Physical Absorption" ,	 "������������� ",
17,		30,2,		110,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Physical_Absorption,SKILL_PHYSICAL_ABSORB,SIN_SKILL_USE_RIGHT,P_Absor_bUseMana},

{ "Great Smash" ,		 "�������ҧ�ع�ç ",
20,		38,2,		73,3,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Great_Smash,SKILL_GREAT_SMASH,SIN_SKILL_USE_ALL,G_Smash_UseMana},

{ "Maximize" ,		 "������ѧ����������Ѻ���ظ" ,
23,		41,2,		110,4,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0},
F_Maximize,SKILL_MAXIMIZE,SIN_SKILL_USE_RIGHT,Maximize_UseMana},

{ "Automation" ,		 "������ѧ ��Ф�������㹡������ Ẻ���Ǥ��� ���Ѻ���ظ������ " ,
26,		42,2,		110,4,
{0,0,0},			{sinWS1,sinWT1,0,0,0,0,0,0},
F_Automation,SKILL_AUTOMATION,SIN_SKILL_USE_RIGHT,Automation_UseMana},

{ "Spark" ,			 "���մ�����¿�� " ,
30,		40,2,		82,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Spark,SKILL_SPARK,SIN_SKILL_USE_RIGHT,Spark_UseMana},

{ "Metal Armor" ,		 "������� " ,
40,		48,2,		110,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Metal_Armor,SKILL_METAL_ARMOR,SIN_SKILL_USE_RIGHT,Metal_Armor_UseMana},

{ "Grand Smash" ,		 "������ 2 ���駫�͹ " ,
43,		44,2,		82,2,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Grand_Smash,SKILL_GRAND_SMASH,SIN_SKILL_USE_RIGHT,Grand_Smash_UseMana},

{ "Mechanician Weapon Mastery" ,		 "���������ӹҭ㹡�������ظ       ��Шӵ�� " ,
46,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_M_W_Mastery,SKILL_MECHANIC_WEAPON,SIN_SKILL_USE_NOT,0},

{ "Spark Shield" ,			 "������ѧ��ͧ�ѹ���������¿�� " ,
50,		52,2,		110,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Spark_Shield,SKILL_SPARK_SHIELD,SIN_SKILL_USE_RIGHT,Spark_Shield_UseMana},

{"Impulsion","�������л�ЪԴ���¾�ѧ俿��",
60,     	55,2,		130,0,
{1,0,0},            {sinWC1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Impulsion,SKILL_IMPULSION,SIN_SKILL_USE_RIGHT,Impulsion_UseMana},

{"Compulsion","�ٴ�͹���������������",
63,     	65,2,		440,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Compulsion,SKILL_COMPULSION,SIN_SKILL_USE_RIGHT,Compulsion_UseMana},

{"Magnetic Sphere","���ҧʹ��俿��",
66,     	71,2,		600,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Magnetic_Sphere,SKILL_MAGNETIC_SPHERE,SIN_SKILL_USE_RIGHT,Magnetic_Sphere_UseMana},

{"Metal Golem","Metal Golem",
70,     	71,2,		1000,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Metal_Golem,SKILL_METAL_GOLEM,SIN_SKILL_USE_RIGHT,Metal_Golem_UseMana},



{ "Melee Mastery" ,		 "������ѧ���ն��� " ,
10,		0,0,		0,0,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Melee_Mastery,SKILL_MELEE_MASTERY,SIN_SKILL_USE_NOT,0},

{ "Fire Attribute" ,		 "����������ҹ�ҹ�ҵ�俶��� ",
12,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Fire_Attribute,SKILL_FIRE_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{ "Raving" ,			 " ����Ẻ�š���ʹ" ,
14,		35,2,		93,3,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Raving,SKILL_RAVING,SIN_SKILL_USE_ALL,Raving_UseMana},

{ "Impact" ,			 "�� 2 ���駫�͹ " ,
17,		37,2,		86,4,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Impact,SKILL_IMPACT,SIN_SKILL_USE_ALL,Impact_UseMana},

{ "Triple Impact" ,		 "���յ�����ͧ 3 ���駫�͹ ",
20,		39,2,		80,4,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Triple_Impact,SKILL_TRIPLE_IMPACT,SIN_SKILL_USE_ALL,T_Impact_UseMana},

{ "Brutal Swing" ,		 "�����ѵ�ٴ��¡�������§���ظ ",
23,		41,2,		82,2,
{0,0,0},			{sinWA1,sinWS2,0,0,0,0,0,0},
F_Brutal_Swing,SKILL_BRUTAL_SWING,SIN_SKILL_USE_ALL,B_Swing_UseMana},

{ "Roar" ,			 "��⡹����ѵ����ش�Чѡ " ,
26,		39,2,		112,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Roar,SKILL_ROAR,SIN_SKILL_USE_RIGHT,Roar_UseMana},

{ "Rage of Zecram" ,		 "���������෾�Ѥ�� " ,
30,		45,2,		84,4,
{0,0,0},			{sinWA1,sinWH1,sinWS2,0,0,0,0,0},
F_RageOfZecram,SKILL_RAGE_OF_ZECRAM,SIN_SKILL_USE_RIGHT,R_Zecram_UseMana},

{ "Concentration" ,		 "������Ҹ� " ,
40,		53,2,		110,3,
{0,0,0},			{sinWA1,sinWS2,0,0,0,0,0,0},
F_Concentration,SKILL_CONCENTRATION,SIN_SKILL_USE_RIGHT,Concentration_UseMana},

{ "Avanging Crash" ,		 "���ⴴ���� 2 ���駫�͹ ",
43,		48,2,		73,3,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Avanging_Crash,SKILL_AVANGING_CRASH,SIN_SKILL_USE_RIGHT,A_Crash_UseMana},

{ "Swift Axe" ,		 "������������㹡������ " ,
46,		53,2,		200,3,
{0,0,0},			{sinWA1,0,0,0,0,0,0,0},
F_Swift_Axe,SKILL_SWIFT_AXE,SIN_SKILL_USE_RIGHT,Swift_Axe_UseMana},

{ "Bone Crash" ,		 "�������˹ѡ��Ƿ�����㹡������ ���ҧ�ع�ç " ,
50,		54,2,		73,3,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Bone_Crash,SKILL_BONE_CRASH,SIN_SKILL_USE_RIGHT,B_Crash_UseMana},

{"Destroyer",		"���� 4 ���駫�͹ ",
60,		45,2,		150,0,
{1,0,0},			{sinWA1,sinWH1,0,0,0,0,0,0},
F_Destoryer,SKILL_DETORYER,SIN_SKILL_USE_ALL,Destoryer_UseMana},

{"Berserker","���������",
63,		60,2,		700,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Berserker,SKILL_BERSERKER,SIN_SKILL_USE_RIGHT,Berserker_UseMana},

{"Cyclon Strike","�����ѵ�ٷ���������������ͺ",
66,		55,2,		135,0,
{1,0,0},			{sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Cyclone_Strike,SKILL_CYCLONE_STRIKE,SIN_SKILL_USE_RIGHT,Cyclone_Strike_UseMana},

{"Boost Health","������ѧ���Ե",
70,		0,0,		0,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Boost_Health,SKILL_BOOST_HEALTH,SIN_SKILL_USE_NOT,},



{ "Pike Wind" ,			 "���ҧ����ع�ͺ��� " ,
10,		27,2,		80,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Pike_Wind,SKILL_PIKE_WIND,SIN_SKILL_USE_RIGHT,Pike_Wind_UseMana},

{ "Ice Attribute" ,		 "����������ҹ�ҹ����秶��� " ,
12,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Ice_Attribute,SKILL_ICE_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{ "Critical Hit" ,		 "���ըش��� " ,
14,		31,2,		84,3,
{0,0,0},			{sinWP1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Critical_Hit,SKILL_CRITICAL_HIT,SIN_SKILL_USE_ALL,Critical_Hit_UseMana},

{ "Jumping Crash" ,		 "���ⴴ��ع��ǿѹ" ,
17,		34,2,		76,4,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Jumping_Crash,SKILL_JUMPING_CRASH,SIN_SKILL_USE_ALL,Jumping_Crash_UseMana},

{ "Ground Pike" ,			 "�����ѵ���ͺ��ȷҧ���¹���� " ,
20,		36,2,		84,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Ground_Pike,SKILL_GROUND_PIKE,SIN_SKILL_USE_RIGHT,G_Pike_UseMana},

{ "Tornado" ,			 "���ѧ�͹�������ѵ�� " ,
23,		38,2,		83,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Tornado_Pike,SKILL_TORNADO,SIN_SKILL_USE_RIGHT,Tornado_UseMana},

{ "Weapon DEF Mastery" ,		 "������ѧ��ͧ�ѹ�������ظ����  " ,
26,		0,0,		0,0,
{0,0,0},			{sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Weapone_Defence_Mastery,SKILL_WEAPONE_DEFENCE_MASTERY,SIN_SKILL_USE_NOT,0},

{ "Expansion" ,			 "������Ҵ���ظ�������������� " ,
30,		42,2,		82,4,
{0,0,0},			{sinWA1,sinWH1,sinWP1,0,0,0,0,0},
F_Expansion,SKILL_EXPANSION,SIN_SKILL_USE_ALL,Expansion_UseMana},

{ "Venom Spear" ,			 "���������ѵ�� " ,
40,		50,2,		110,2,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Venom_Spear,SKILL_VENOM_SPEAR,SIN_SKILL_USE_RIGHT,VenomSpear_UseMana},

{"Vanish",			"��ҧ��� ",
43,		53,2,		140,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Vanish,SKILL_VANISH,SIN_SKILL_USE_RIGHT,Vanish_UseMana},

{"Critical Mastery",	                     "�������������㹡�����ըش��� ",
46,		0,0,		0,0,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Critical_Mastery,SKILL_CRITICAL_MASTERY,SIN_SKILL_USE_NOT,0},

{"Chain Lance",	  		  "���ⴴ���� 3 ���駫�͹ ",
50,		51,2,		88,3,
{0,0,0},			{sinWC1,sinWP1,sinWS2,0,0,0,0,0},
F_Chain_Lance,SKILL_CHAIN_LANCE,SIN_SKILL_USE_RIGHT,Chain_Lance_UseMana},

{"Assassin Eye",	 "�ͧ�Ҩش��͹ ",
60,		61,2,		520,0,
{1,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Assassin_Eye,SKILL_ASSASSIN_EYE,SIN_SKILL_USE_RIGHT,Assassin_Eye_UseMana},

{"Charging Strike",	 "���͡���������ҧ�ع�ç ",
63,		75,2,		150,0,
{1,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Charging_Strike,SKILL_CHARGING_STRIKE,SIN_SKILL_USE_RIGHT,Charging_Strike_UseMana},

{"Vague",	"������������ö㹡���ź��ա ",
66,		71,2,		650,0,
{1,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Vague,SKILL_VAGUE,SIN_SKILL_USE_RIGHT,Vague_UseMana},

{"Shadow Master",	"�������ҧ ",
70,		51,2,		160,0,
{1,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Shadow_Master,SKILL_SHADOW_MASTER,SIN_SKILL_USE_RIGHT,Shadow_Master_UseMana},


{ "Scout Hawk" ,			 "���¡����������Ǩ�ѵ�� " ,
10,		25,2,		115,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Scout_Hawk ,SKILL_SCOUT_HAWK,SIN_SKILL_USE_RIGHT,Scout_Hawk_UseMana },

{ "Shooting Mastery" ,		 "������ѧ���ո�ٶ��� ",
12,		0,0,		0,0,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Shooting_Mastery,SKILL_SHOOTING_MASTERY,SIN_SKILL_USE_NOT,0},

{ "Wind Arrow" ,			 " ���մ��¸���� " ,
14,		25,2,		82,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Wind_Arrow,SKILL_WIND_ARROW,SIN_SKILL_USE_ALL,Wind_Arrow_UseMana},

{ "Perfect Aim" ,			 "�������ҧ����� " ,
17,		29,2,		81,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Perfect_Aim,SKILL_PERFECT_AIM,SIN_SKILL_USE_ALL,Perfect_Aim_UseMana},


{ "Dion's Eye" ,			 "��������������Ẻ���� " ,
20,		0,0,		0,0,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Dions_Eye,SKILL_DIONS_EYE,SIN_SKILL_USE_NOT,0},

{ "Falcon" ,			 "���¡������Ǫ������� " ,
23,		36,2,		115,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Falcon ,SKILL_FALCON,SIN_SKILL_USE_RIGHT,Falcon_UseMana },

{ "Arrow of Rage" ,		 "�����繡�������ҧ�ع�ç ",
26,		38,2,		82,4,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Arrow_of_Rage,SKILL_ARROW_OF_RAGE,SIN_SKILL_USE_RIGHT,A_Rage_UseMana},

{ "Avalanche" ,			 "����Ẻ������ͧ " ,
30,		41,2,		103,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Avalanche,SKILL_AVALANCHE,SIN_SKILL_USE_RIGHT,Avalanche_UseMana},

{ "Elemental shot" ,			 "���մ��¸�ٸҵ�" ,
40,		43,2,		82,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_E_Shot,SKILL_ELEMENTAL_SHOT,SIN_SKILL_USE_RIGHT,Elemental_Shot_UseMana},

{ "Golden Falcon" ,			 "���¡������Ƿͧ�������� ",
43,		52,2,		120,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_G_Falcon,SKILL_GOLDEN_FALCON,SIN_SKILL_USE_RIGHT,Golden_Falcon_UseMana},

{ "Bomb Shot" ,		     "����ԧ������Դ " ,
46,		46,2,		87,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_B_Shot,SKILL_BOMB_SHOT,SIN_SKILL_USE_RIGHT,BombShot_UseMana},

{ "Perforation" ,			 "�ԧ���Ẻ���ط��ǧ" ,
50,		48,2,		82,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Perforation,SKILL_PERFORATION,SIN_SKILL_USE_RIGHT,Perforation_UseMana},

{"Recall Wolverine",		"���¡��һ�Ҫ������� ",
60,		58,2,		900,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Recall_Wolverin,SKILL_RECALL_WOLVERIN,SIN_SKILL_USE_RIGHT,Recall_Wolverin_UseMana},

{"Evasion Mastery",	"������������ö㹡���ź��ա ",
63,		0,0,		0,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Evasion_Mastery,SKILL_EVASION_MASTERY,SIN_SKILL_USE_NOT,0},

{"Phoenix Shot",	"���մ��¹��Թԡ�� ",
66,		55,2,		100,0,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Phoenix_Shot,SKILL_PHOENIX_SHOT,SIN_SKILL_USE_RIGHT,Phoenix_Shot_UseMana},

{"Force of Nature","������������ö�ҡ����Ǵ���� ",
70,		56,2,		900,0,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Force_Of_Nature,SKILL_FORCE_OF_NATURE,SIN_SKILL_USE_RIGHT,Force_Of_Nature_UseMana},


	

{ "Sword Blast" ,		 "�����ѵ�ٴ��¾�ѧ��觴Һ " ,
10,		22,2,		82,2,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Sword_Blast,SKILL_SWORD_BLAST,SIN_SKILL_USE_ALL,Sword_Blast_UseMana},

{ "Holy Body" ,			 "��ҧ����ѡ����Է��� " ,
12,		28,2,		110,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_HolyBody ,SKILL_HOLY_BODY,SIN_SKILL_USE_RIGHT,HolyBody_UseMana },

{ "Physical Training" ,		 "��Ѻ���������ҧ���" ,
14,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Physical_Traning,SKILL_PHYSICAL_TRANING,SIN_SKILL_USE_NOT,0},

{ "Double Crash" ,		 "�������ҧ�ع�ç�ͧ���駫�͹" ,
17,		35,2,		82,2,
{0,0,0},			{sinWA1,sinWC1,sinWS2,0,0,0,0,0},
F_Double_Crash,SKILL_DOUBLE_CRASH,SIN_SKILL_USE_RIGHT,D_Crash_UseMana},

{ "Holy Valor" ,			 "������ѧ������� Undead ���Ѻ���ͧ������͹���� Party ���   ��������� " ,
20,		36,2,		100,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Holy_Valor,SKILL_HOLY_VALOR,SIN_SKILL_USE_RIGHT,Holy_Valor_Mana},

{ "Brandish" ,			 "���ⴴ�����ѵ����ǧ���ҧ ",
23,		38,2,		82,2,
{0,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Brandish,SKILL_BRANDISH,SIN_SKILL_USE_ALL,Brandish_Mana},

{ "Piercing" ,			 "ᷧ�ѵ��Ẻ���ط��ǧ" ,
26,		41,2,		82,2,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Piercing,SKILL_PIERCING,SIN_SKILL_USE_ALL,Piercing_Mana},

{ "Drastic Spirit" ,		 "��ѧ��ͧ�ѹ������鹪��Ǣ�� " ,
30,		44,2,		100,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Drastic_Spirit,SKILL_DRASTIC_SPIRIT,SIN_SKILL_USE_RIGHT,Drastic_Spirit_Mana},

{"Sword Mastery",	"������ѧ�Һ���� ",
40,		0,0,		0,0,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_S_Mastery,SKILL_SWORD_MASTERY,SIN_SKILL_USE_NOT,0},

{ "Divine Inhalation" ,		 "���ҧ����ѡ����Է��� " ,
43,		46,2,		130,1,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_D_Inhalation,SKILL_DIVINE_INHALATION,SIN_SKILL_USE_RIGHT,D_Inhalation_UseMana},

{ "Holy Incantation" ,		 "�С��Ե Undead ��������١��ͧ " ,
46,		50,2,		120,1,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Holy_Incantation,SKILL_HOLY_INCANTATION,SIN_SKILL_USE_RIGHT,H_Incantation_UseMana},

{ "Grand Cross" ,		 "�ҧࢹ�ѡ����Է��� ",
50,		45,2,		82,2,
{0,0,0},			{sinWH1,sinWS2,0,0,0,0,0,0},
F_Grand_Cross,SKILL_GRAND_CROSS,SIN_SKILL_USE_RIGHT,Grand_Cross_UseMana},

{"Devine piercing",	"�����ѵ�ٴ��¾�ѧ�Һ�����ѷ�� ",
70,		51,2,		160,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Divine_Piercing,SKILL_DIVINE_PIERCING,SIN_SKILL_USE_RIGHT,Divine_Piercing_UseMana},

{"Godly Shield",	"�����觾����� ",
63,		50,2,		400,0,
{1,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Godly_Shield,SKILL_GODLY_SHIELD,SIN_SKILL_USE_RIGHT,Godly_Shield_UseMana},

{"God Bless",	"�;�ѧ�ѹ����˭�ҡ������ ",
66,		51,2,		330,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_God_Bless,SKILL_GOD_BLESS,SIN_SKILL_USE_RIGHT,God_Bless_UseMana},

{"Sword of Justice","�Һ��觤س����",
60,		53,2,		250,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Sword_Of_Justice,SKILL_SWORD_OF_JUSTICE,SIN_SKILL_USE_RIGHT,Sword_Of_Justice_UseMana},


{ "Shield Strike" ,		 "ʵ���ѵ�ٴ��¡�â��ҧ��� ",
10,		23,2,		82,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_S_Strike ,SKILL_SHIELD_STRIKE,SIN_SKILL_USE_RIGHT,S_Strike_UseMana },

{ "Farina" ,			 "���ҧ�͡�͡仴��¤��������٧ " ,
12,		26,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Farina,SKILL_FARINA,SIN_SKILL_USE_ALL,Farina_UseMana},

{ "Throwing Mastery" ,		 "������ѧ�����͡���� ",
14,		0,0,		0,0,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_D_Mastery,SKILL_THROWING_MASTERY,SIN_SKILL_USE_NOT,0},

{ "Vigor Spear" ,		 "���մ����͡�ѡ����Է��� " ,
17,		32,2,		82,2,
{0,0,0},			{sinWP1,sinWT1,0,0,0,0,0,0},
F_V_Spear,SKILL_VIGOR_SPEAR,SIN_SKILL_USE_ALL,V_Spear_UseMana},

{ "Windy" ,			 "�������С�����բͧ�͡ " ,
20,		34,2,		100,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Windy,SKILL_WINDY,SIN_SKILL_USE_RIGHT,Windy_Mana},

{ "Twist Javelin" ,		 "���ҧ�͡Ẻ�ǧ���ҹ " ,
23,		36,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Twist_Javelin,SKILL_TWIST_JAVELIN,SIN_SKILL_USE_RIGHT,Twist_Javelin_Mana},

{ "Soul Sucker" ,			 "���ٴ�ԭ�ҳ " ,
26,		42,2,		110,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Soul_Sucker,SKILL_SOUL_SUCKER,SIN_SKILL_USE_RIGHT,Soul_Sucker_Mana},

{ "Fire Javelin" ,		 "���ҧ�͡�" ,
30,		40,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Fire_Javelin,SKILL_FIRE_JAVELIN,SIN_SKILL_USE_ALL,Fire_Javelin_Mana},

{ "Spilt Javelin" ,		 "���ҧ�͡Ẻ��Ш�� " ,
40,		43,2,		115,1,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Split_Javelin,SKILL_SPLIT_JAVELIN,SIN_SKILL_USE_RIGHT,Split_Javelin_UseMana},

{ "Triumph of Valhalla" ,		 "���ҧ���ѧ������Ҫԡ㹡����     �����ѵ�ҡ������ " ,
43,		50,2,		120,1,
{0,0,0},			{sinWT1,sinWS2,sinWS1,sinWP1,sinWH1,sinWA1,sinWC1,sinWM1},
F_T_Of_Valhalla,SKILL_TRIUMPH_OF_VALHALLA,SIN_SKILL_USE_RIGHT,T_Of_Valhalla_UseMana},

{ "Lightning Javelin" ,		 "���ҧ�͡��¿��" ,
46,		46,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Lightning_Javelin,SKILL_LIGHTNING_JAVELIN,SIN_SKILL_USE_ALL,Lightning_Javelin_UseMana},

{ "Storm Javelin" ,		 "���ҧ�͡���ؤ�� " ,
50,		48,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Storm_Javelin,SKILL_STORM_JAVELIN,SIN_SKILL_USE_RIGHT,Storm_Javelin_UseMana},

{"Hall of Valhalla",	"������������ö����ź��ա ",
60,		52,2,		750,0,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS1,sinWT1,0,0},
F_Hall_Of_Valhalla,SKILL_HALL_OF_VALHALLA,SIN_SKILL_USE_RIGHT,Hall_Of_Valhalla_UseMana},

{"X Rage",	"���ⴴ���� 3 ���� ",
63,		48,2,		210,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_X_Rage,SKILL_X_RAGE,SIN_SKILL_USE_RIGHT,X_Rage_UseMana},

{"Frost Javelin",	"���ҧ�͡����� ",
66,		50,2,		720,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Frost_Javelin,SKILL_FROST_JAVELIN,SIN_SKILL_USE_RIGHT,Frost_Javelin_UseMana},

{"Vengeance",	"�͡��������� ",
70,		55,2,		100,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Vengeance,SKILL_VENGEANCE,SIN_SKILL_USE_RIGHT,Vengeance_UseMana},


{ "Healing" ,			  "�ѡ�Ҿ�ѧ���Ե" ,
10,		15,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Healing,SKILL_HEALING,SIN_SKILL_USE_RIGHT,Healing_UseMana},

{ "Holy Bolt" ,			 "���մ��¾�ѧ�ѡ����Է��� " ,
12,		11,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_HolyBolt,SKILL_HOLY_BOLT,SIN_SKILL_USE_ALL,HolyBolt_UseMana},

{ "Multi Spark" ,			 " ���մ��¾�ѧ��¿�� ",
14,		14,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_M_Spark,SKILL_MULTISPARK,SIN_SKILL_USE_RIGHT,M_Spark_UseMana},

{ "Holy Mind" ,			 "������ѵ����͹��ŧ " ,
17,		19,1,		100,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_HolyMind,SKILL_HOLY_MIND,SIN_SKILL_USE_RIGHT,HolyMind_UseMana},

{ "Meditation" ,			 "������� MP Ẻ���� " ,
20,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Meditation,SKILL_MEDITATION,SIN_SKILL_USE_NOT,0},

{ "Divine Lighting" ,		 "���մ�����¿�� ",
23,		21,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Divine_Lightning,SKILL_DIVINE_LIGHTNING,SIN_SKILL_USE_RIGHT,Divine_Lightning_Mana},

{ "Holy Reflection" ,		 "�з�͹�����������੾�� Undead " ,
26,		23,1,		120,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Holy_Reflection,SKILL_HOLY_REFLECTION,SIN_SKILL_USE_RIGHT,Holy_Reflection_Mana},

{ "Grand Heal" ,		 "������ѧ���Ե��駡���� " ,
30,		27,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Grand_Healing,SKILL_GRAND_HEALING,SIN_SKILL_USE_RIGHT,Grand_Healing_Mana},

{ "Vigor Ball " ,		 "���մ��º�ž�ѧ��� " ,
40,		24,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Vigor_Ball,SKILL_VIGOR_BALL,SIN_SKILL_USE_ALL,Vigor_Ball_UseMana},

{ "Resurrection" ,		 "�غ���Ե��Ҫԡ㹡���� " ,
43,		31,1,		130,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Resurrection,SKILL_RESURRECTION,SIN_SKILL_USE_RIGHT,Resurrection_UseMana},

{ "Extinction" ,		 "Ŵ�͹��ѧ���Ե Undead " ,
46,		28,1,		120,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Extinction,SKILL_EXTINCTION,SIN_SKILL_USE_RIGHT,Extinction_UseMana},

{ "Virtual Life" ,		 "������ѧ���Ե " ,
50,		31,1,		134,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Virtual_Life,SKILL_VIRTUAL_LIFE,SIN_SKILL_USE_RIGHT,Virtual_Life_UseMana},

{"Glacial Spike",	"���մ��¼�֡�����",
60,		35,1,		120,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Glacial_Spike,SKILL_GLACIAL_SPIKE,SIN_SKILL_USE_RIGHT,Glacial_Spike_UseMana},

{"Regeneration Field",	"�ط�ȵ����Ϳ�鹾�ѧ���Ե����ҹ� ",
63,		41,1,		950,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Regeneration_Field,SKILL_REGENERATION_FIELD,SIN_SKILL_USE_RIGHT,Regeneration_Field_UseMana},

{"Chain Lightning",	"���մ�����¿��Ẻ�١��",
66,		41,1,		120,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Chain_Lightning,SKILL_CHAIN_LIGHTNING,SIN_SKILL_USE_RIGHT,Chain_Lightning_UseMana},

{"Summon Muspell",	"���¡�Ƿٵ�͡�Ҥ�����ͧ",
70,		47,1,		800,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Summon_Muspell,SKILL_SUMMON_MUSPELL,SIN_SKILL_USE_RIGHT,Summon_Muspell_UseMana},


{ "Agony" ,			 "����¹��ѧ���Ե���ҹ� " ,
10,		12,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Agony,SKILL_AGONY,SIN_SKILL_USE_RIGHT,Agony_UseMana},

{ "Fire Bolt" ,			 "�����ѵ�ٴ����١� " ,
12,		11,1,		84,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_FireBolt,SKILL_FIRE_BOLT,SIN_SKILL_USE_ALL,FireBolt_UseMana},

{ "Zenith" ,			 "������������觴��¤س���ѵԢͧ�ҵ� ",
14,		15,1,		100,3,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Zenith,SKILL_ZENITH,SIN_SKILL_USE_RIGHT,Zenith_UseMana},

{ "Fire Ball" ,			 "���ա�����ѵ��  �����١� " ,
17,		19,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_FireBall,SKILL_FIRE_BALL,SIN_SKILL_USE_RIGHT,FireBall_UseMana},

{ "Mental Mastery" ,		 "��������ҹҶ��� " ,
20,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Mental_Mastery,SKILL_MENTAL_MASTERY,SIN_SKILL_USE_NOT,0},

{ "Water Tornado" ,			 "���մ��¾��ع��� ",
23,		21,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Watornado,SKILL_WATORNADO,SIN_SKILL_USE_RIGHT,Watornado_Mana},

{ "Enhance Weapon" ,		 "�����ҵص�ҧ����Ѻ���ظ����� ",
26,		28,1,		104,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Enchant_Weapon,SKILL_ENCHANT_WEAPON,SIN_SKILL_USE_RIGHT,Enchant_Weapon_Mana},

{ "Death Ray" ,			 "���մ������ʧ��觤������ " ,
30,		26,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Dead_Ray,SKILL_DEAD_RAY,SIN_SKILL_USE_ALL,Dead_Ray_Mana},

{ "Energy Shield" ,			 "���ҧ��ҹ�Ƿ������ͧ�ѹ���ͧ ",
40,		40,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Energy_Shield,SKILL_ENERGY_SHIELD,SIN_SKILL_USE_RIGHT,Energy_Shield_UseMana},

{ "Diastrophism" ,			 "���Դ�ó� " ,
43,		33,1,		106,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Diastrophism,SKILL_DIASTROPHISM,SIN_SKILL_USE_RIGHT,Diastrophism_UseMana},

{ "Spirit Elemental" ,			 "���¡�Ե�ԭ�ҳ�������ѧ�Ƿ����� ",
46,		35,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Spirit_Elemental,SKILL_SPIRIT_ELEMENTAL,SIN_SKILL_USE_RIGHT,Spirit_Elemental_UseMana},

{ "Dancing Sword" ,			 "���¡�Һ�ҵت������� " ,
50,		30,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_D_Sword,SKILL_DANCING_SWORD,SIN_SKILL_USE_RIGHT,Dancing_Sword_UseMana},

{"Fire Elemental",	"���¡�������ͧ�ҵ�� ",
60,		120,2,		900,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Fire_Elemental,SKILL_FIRE_ELEMENTAL,SIN_SKILL_USE_RIGHT,Fire_Elemental_UseMana},

{"Frame Wave",	"����¾�ѧ���������ԧ ",
63,		32,1,		124,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Flame_Wave,SKILL_FLAME_WAVE,SIN_SKILL_USE_RIGHT,Flame_Wave_UseMana},

{"Destruction",	"Ŵ��Ҥ�������ö�ͧ�ѵ�� ",
66,		43,1,		460,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Distortion,SKILL_DISTORTION,SIN_SKILL_USE_RIGHT,Distortion_UseMana},

{"Meteor",	"�Ƿ��ǵ� ",
70,		39,1,		190,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Meteo,SKILL_M_METEO,SIN_SKILL_USE_RIGHT,Meteo_UseMana},

{"Orb",		"������ѧ���� ���� Force Orb",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SKILL_FORCE_ORB,0,0},


{"���",		"�������͹�����!",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CHANGE_JOB3_QUEST,0,0},


{"Absorption of Glory","������ѧ absorb +10 ��ʹ˹���ѻ����",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ABSORB,0,0},

{"Power Source","������ѧ�����������10% ��ʹ˹���ѻ����",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ATTACK,0,0},

{"Evade Update","���� Evade 10% ˹���ѻ����",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_EVASION,0,0},


{"Atanasia","�������������ѵ�٪��Ǥ��� ��ѧ����Ŵŧ1/2",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_INVULNERABILITY,0,0},

{"Deadly Edge","�����͡�ʤ�ԵԤ��5%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_CRITICAL,0,0},

{"Evade Plus","�����͡�����Ǵ5%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_EVASION,0,0},

{"Defense Stun","���¡�����դ��ʵ�ŷ��������50%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_FIRECRYTAL,0,0},

{"Defense Stun","���¡�����դ��ʵ�ŷ�����������50%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_ICECRYTAL,0,0},

{"Defense Stun","���¡�����դ��ʵ�ŷ���������¿��50%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_LINGHTINGCRYTAL,0,0},

{"Might Stun","������ѧ���շ��˹ѡ����ش����� fighter ����˹ѡ����ա20%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_FIGHTER,0,0},

{"Might Stun","������ѧ���շ��˹ѡ����ش����� mechanician ����˹ѡ����ա20%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_MECHANICIAN,0,0},

{"Might Stun","������ѧ���շ��˹ѡ����ش����� pikeman ����˹ѡ����ա20%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_PIKEMAN,0,0},

{"Might Stun","������ѧ���շ��˹ѡ����ش����� archer ����˹ѡ����ա20%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_ARCHER,0,0},

{"Might Stun","������ѧ���շ��˹ѡ����ش����� knight ����˹ѡ����ա20%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_KNIGHT,0,0},

{"Might Stun","������ѧ���շ��˹ѡ����ش����� atalanta ����˹ѡ����ա20%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_ATALANTA,0,0},

{"Might Stun","������ѧ���շ��˹ѡ����ش�����Magician����˹ѡ����ա20%.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_MAGICIAN,0,0},

{"Might Stun","������ѧ���շ��˹ѡ����ش�����Priestess����˹ѡ����ա20%.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_PRIESTESS,0,0},

{"Eternal Life","�����Ъ�ǧ����˹�����ѧ����Ŵŧ50%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_INVULNERABILITY,0,0},

{"Fatal Edge","�����ç��� 5% ����͵Դ��ԵԤ��",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_CRITICAL,0,0},

{"Avert Scroll","�����͡��㹡���ź��ա 5%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_EVASION,0,0},




