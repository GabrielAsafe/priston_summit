
#define SIN_SKILL_USE_LEFT		0x02000000		
#define SIN_SKILL_USE_ALL		0x03000000		
#define SIN_SKILL_USE_NOT		0x04000000		



{"Extreme Shield",		"T�ng kha� n�ng ��� �o�n khi s�� du�ng khi�n",
10,		34,2,		110,4,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Extreme_Shield,SKILL_EXTREME_SHIELD,SIN_SKILL_USE_RIGHT,E_Shield_UseMana},

{"Mechanic Bomb",		"Ne�m m��t tra�i bom g�y sa�t   th��ng",
12,		28,2,		73,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Mechanic_Bomb,SKILL_MECHANIC_BOMB,SIN_SKILL_USE_RIGHT,Mechanic_Bomb_UseMana},

{"Poison Attribute",		"T�ng kha� n�ng ch��ng la�i     ch��t ���c",
14,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Poison_Attribute,SKILL_POISON_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{"Physical Absorption",		"T�ng kha� n�ng h��p thu� s��c  sa�t th��ng v��i c� th��",
17,		30,2,		110,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Physical_Absorption,SKILL_PHYSICAL_ABSORB,SIN_SKILL_USE_RIGHT,P_Absor_bUseMana},

{"Great Smash",			"T��p trung s��c ma�nh va�o m��t cu� �a�nh",
20,		38,2,		73,3,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Great_Smash,SKILL_GREAT_SMASH,SIN_SKILL_USE_ALL,G_Smash_UseMana},

{"Maximize",			"S�� du�ng vu� khi� v��i m��c   sa�t th��ng cao nh��t",
23,		41,2,		110,4,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0},
F_Maximize,SKILL_MAXIMIZE,SIN_SKILL_USE_RIGHT,Maximize_UseMana},

{"Automation",			"T�� ���ng hoa� ca�c vu� khi�   t��m xa ��� t�ng s�� chi�nh    xa�c",
26,		42,2,		110,4,
{0,0,0},			{sinWS1,sinWT1,0,0,0,0,0,0},
F_Automation,SKILL_AUTOMATION,SIN_SKILL_USE_RIGHT,Automation_UseMana},

{"Spark",			"T��n c�ng b��ng m��t qua� c��u �i��n c��c ma�nh",
30,		40,2,		82,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Spark,SKILL_SPARK,SIN_SKILL_USE_RIGHT,Spark_UseMana},

{"Metal Armor",		"T�ng cao kha� n�ng pho�ng thu� cu�a a�o gia�p",
40,		48,2,		110,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Metal_Armor,SKILL_METAL_ARMOR,SIN_SKILL_USE_RIGHT,Metal_Armor_UseMana},

{"Grand Smash",	"T��n c�ng li�n tu�c. T�ng th�m s��c ma�nh cu�a �o�n t��n c�ng",
43,		44,2,		82,2,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Grand_Smash,SKILL_GRAND_SMASH,SIN_SKILL_USE_RIGHT,Grand_Smash_UseMana},

{"Mechanic Weapon Mastery",	"S�� du�ng hoa�n ha�o ca�c vu�  khi� ���c bi��t chuy�n du�ng",
46,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_M_W_Mastery,SKILL_MECHANIC_WEAPON,SIN_SKILL_USE_NOT,0},

{"Spark Shield",	    	"Du�ng �i��n n�ng ��� ba�o v��",
50,		52,2,		110,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Spark_Shield,SKILL_SPARK_SHIELD,SIN_SKILL_USE_RIGHT,Spark_Shield_UseMana},

{"Impulsion","T��n c�ng ���i thu� b��ng ca�ch pha�t ra tia l��a �i��n",
60,     	55,2,		130,0,
{1,0,0},            {sinWC1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Impulsion,SKILL_IMPULSION,SIN_SKILL_USE_RIGHT,Impulsion_UseMana},

{"Completion","Thu hu�t qua�i v��t xung quanh b��ng so�ng �i��n t��",
63,     	65,2,		440,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Compulsion,SKILL_COMPULSION,SIN_SKILL_USE_RIGHT,Compulsion_UseMana},

{"Magnetic Sphere","Ta�o ca�c vi�n ngo�c �i��n t�� t��n c�ng b��t ky� mu�c ti�u   na�o ���ng g��n",
66,     	71,2,		600,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Magnetic_Sphere,SKILL_MAGNETIC_SPHERE,SIN_SKILL_USE_RIGHT,Magnetic_Sphere_UseMana},

{"Metal Glolem","Tri��u h��i ng���i ma�y b��ng  s��t the�p",
70,     	71,2,		1000,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Metal_Golem,SKILL_METAL_GOLEM,SIN_SKILL_USE_RIGHT,Metal_Golem_UseMana},


{"Melee Mastery",		"Gia t�ng s��c t��n c�ng cho vu� khi� gia�p la� ca� ",
10,		0,0,		0,0,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Melee_Mastery,SKILL_MELEE_MASTERY,SIN_SKILL_USE_NOT,0},

{"Fire Attribute",		"Gia t�ng s��c chi�u ���ng v��i l��a",
12,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Fire_Attribute,SKILL_FIRE_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{"Raving",			"T�ng m��c sa�t th��ng nh�ng bi� m��t HP trong m��i cu� �a�nh",
14,		35,2,		93,3,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Raving,SKILL_RAVING,SIN_SKILL_USE_ALL,Raving_UseMana},

{"Impact",			"T��n c�ng 2 chi�u li�n tu�c    m��t ca�ch ma�nh me� ",
17,		37,2,		86,4,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Impact,SKILL_IMPACT,SIN_SKILL_USE_ALL,Impact_UseMana},

{"Triple Impact",		"M��t ky� thu��t ra �o�n ma�nh  h�n Impact",
20,		39,2,		80,4,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Triple_Impact,SKILL_TRIPLE_IMPACT,SIN_SKILL_USE_ALL,T_Impact_UseMana},

{"Brutal Swing",		"Gia�ng �o�n chi� ma�ng",
23,		41,2,		82,2,
{0,0,0},			{sinWA1,sinWS2,0,0,0,0,0,0},
F_Brutal_Swing,SKILL_BRUTAL_SWING,SIN_SKILL_USE_ALL,B_Swing_UseMana},

{"Roar",			"G��m l�n la�m cho qua�i v��t   choa�ng va�ng, s�� ha�i",
26,		39,2,		112,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Roar,SKILL_ROAR,SIN_SKILL_USE_RIGHT,Roar_UseMana},

{"Rage of Zechram",		"M��t ky� n�ng ho�c t�� ng���i  hu�ng Zechram huy��n thoa�i",
30,		45,2,		84,4,
{0,0,0},			{sinWA1,sinWH1,sinWS2,0,0,0,0,0},
F_RageOfZecram,SKILL_RAGE_OF_ZECRAM,SIN_SKILL_USE_RIGHT,R_Zecram_UseMana},

{"Concentration",	"S�� chi�nh xa�c ����c gia t�ng trong m��t khoa�ng th��i gian",
40,		53,2,		110,3,
{0,0,0},			{sinWA1,sinWS2,0,0,0,0,0,0},
F_Concentration,SKILL_CONCENTRATION,SIN_SKILL_USE_RIGHT,Concentration_UseMana},

{"Avenging Crash",	"�o�n t��n c�ng ���i thu� c��c  ma�nh",
43,		48,2,		73,3,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Avanging_Crash,SKILL_AVANGING_CRASH,SIN_SKILL_USE_RIGHT,A_Crash_UseMana},

{"Swift Axe",	"T�ng t��c ��� �a�nh cho Ri�u   trong m��t khoa�ng th��i gian",
46,		53,2,		200,3,
{0,0,0},			{sinWA1,0,0,0,0,0,0,0},
F_Swift_Axe,SKILL_SWIFT_AXE,SIN_SKILL_USE_RIGHT,Swift_Axe_UseMana},

{"Bone Crash",		"�o�n t��n c�ng ma�nh du�ng ��� ti�u di��t Demon",
50,		54,2,		73,3,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Bone_Crash,SKILL_BONE_CRASH,SIN_SKILL_USE_RIGHT,B_Crash_UseMana},

{"Destroyer",		"T��n c�ng li�n hoa�n ta�o n�n  sa�t th��ng c��c l��n ���i v��i Demon",
60,		45,2,		150,0,
{1,0,0},			{sinWA1,sinWH1,0,0,0,0,0,0},
F_Destoryer,SKILL_DETORYER,SIN_SKILL_USE_ALL,Destoryer_UseMana},

{"Berserker","T�ng kha� n�ng sa�t th��ng     nh�ng la�i gia�m kha� n�ng hoa� gia�i �o�n t��n c�ng",
63,		60,2,		700,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Berserker,SKILL_BERSERKER,SIN_SKILL_USE_RIGHT,Berserker_UseMana},

{"Cyclone Strike","�o�n t��n c�ng nhi��u ���i thu� l��i du�ng l��c ly t�m",
66,		55,2,		135,0,
{1,0,0},			{sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Cyclone_Strike,SKILL_CYCLONE_STRIKE,SIN_SKILL_USE_RIGHT,Cyclone_Strike_UseMana},

{"Boost Health","T�ng chi� s�� HP",
70,		0,0,		0,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Boost_Health,SKILL_BOOST_HEALTH,SIN_SKILL_USE_NOT,},


{"Pike Wind",			"Ta�o gio� xoa�y ���y ng���c ra va� g�y sa�t th��ng",
10,		27,2,		80,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Pike_Wind,SKILL_PIKE_WIND,SIN_SKILL_USE_RIGHT,Pike_Wind_UseMana},

{"Ice Attribute",		"T�ng s��c ch��ng ��� v��i b�ng gia� ",
12,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Ice_Attribute,SKILL_ICE_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{"Critical Hit",		"Ra �o�n c��c ma�nh va�o �i��m  y��u",
14,		31,2,		84,3,
{0,0,0},			{sinWP1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Critical_Hit,SKILL_CRITICAL_HIT,SIN_SKILL_USE_ALL,Critical_Hit_UseMana},

{"Jumping Crash",		"Nha�y l�n va� ra �o�n t�� tr�n cao c��c ma�nh",
17,		34,2,		76,4,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Jumping_Crash,SKILL_JUMPING_CRASH,SIN_SKILL_USE_ALL,Jumping_Crash_UseMana},

{"Ground Pike",			"La�m �o�ng b�ng ���i thu� ",
20,		36,2,		84,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Ground_Pike,SKILL_GROUND_PIKE,SIN_SKILL_USE_RIGHT,G_Pike_UseMana},

{"Tornado",			"T��n c�ng ca�c mu�c ti�u xung  quanh b��ng nh��ng c�n l��c    xoa�y",
23,		38,2,		83,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Tornado_Pike,SKILL_TORNADO,SIN_SKILL_USE_RIGHT,Tornado_UseMana},

{"Weapon DEF Mastery",		"T�ng kha� n�ng chi�u �o�n khi  c��m gia�o",
26,		0,0,		0,0,
{0,0,0},			{sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Weapone_Defence_Mastery,SKILL_WEAPONE_DEFENCE_MASTERY,SIN_SKILL_USE_NOT,0},


{"Expansion",			"Truy��n s��c ma�nh va�o vu�    khi� ��� t��n c�ng",
30,		42,2,		82,4,
{0,0,0},			{sinWA1,sinWH1,sinWP1,0,0,0,0,0},
F_Expansion,SKILL_EXPANSION,SIN_SKILL_USE_ALL,Expansion_UseMana},

{"Venom Spear",		"��m th��ng xu��ng ���t ��� t��n c�ng",
40,		50,2,		110,2,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Venom_Spear,SKILL_VENOM_SPEAR,SIN_SKILL_USE_RIGHT,VenomSpear_UseMana},

{"Vanish",			"Ta�ng hi�nh trong m��t khoa�ng th��i gian nh��t �i�nh",
43,		53,2,		140,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Vanish,SKILL_VANISH,SIN_SKILL_USE_RIGHT,Vanish_UseMana},

{"Critical Mastery",		"T�ng ty� l�� ra �o�n c��c ma�nh",
46,		0,0,		0,0,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Critical_Mastery,SKILL_CRITICAL_MASTERY,SIN_SKILL_USE_NOT,0},

{"Chain Lancer",	  	"Li�n hoa�n tam chi�u",
50,		51,2,		88,3,
{0,0,0},			{sinWC1,sinWP1,sinWS2,0,0,0,0,0},
F_Chain_Lance,SKILL_CHAIN_LANCE,SIN_SKILL_USE_RIGHT,Chain_Lance_UseMana},

{"Assassin Eye",	 "Ti�m �i��m y��u cu�a ���i thu�, t�ng kha� n�ng ra �o�n chi�   ma�ng",
60,		61,2,		520,0,
{1,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Assassin_Eye,SKILL_ASSASSIN_EYE,SIN_SKILL_USE_RIGHT,Assassin_Eye_UseMana},

{"Charging Strike",	 "D��n s��c t��n c�ng li�n tu�c  b��ng chi�u ��m",
63,		75,2,		150,0,
{1,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Charging_Strike,SKILL_CHARGING_STRIKE,SIN_SKILL_USE_RIGHT,Charging_Strike_UseMana},

{"Vague",	"T�ng kha� n�ng tra�nh �o�n t��n c�ng t�� qua�i v��t",
66,		71,2,		650,0,
{1,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Vague,SKILL_VAGUE,SIN_SKILL_USE_RIGHT,Vague_UseMana},

{"Shadow Master",	"T��n c�ng li�n hoa�n v��i t��c ��� c��c l��n",
70,		51,2,		160,0,
{1,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Shadow_Master,SKILL_SHADOW_MASTER,SIN_SKILL_USE_RIGHT,Shadow_Master_UseMana},


{"Scout Hawk",			"Tri��u h��i chim �ng gia t�ng  s�� chi�nh xa�c",
10,		25,2,		115,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Scout_Hawk ,SKILL_SCOUT_HAWK,SIN_SKILL_USE_RIGHT,Scout_Hawk_UseMana },

{"Shooting Mastery",		"T�ng s��c sa�t th��ng khi du�ng cung ho��c No� ",
12,		0,0,		0,0,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Shooting_Mastery,SKILL_SHOOTING_MASTERY,SIN_SKILL_USE_NOT,0},

{"Wind Arrow",			"B��n t�n xuy�n gio� v��i s��c  t��n c�ng ma�nh",
14,		25,2,		82,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Wind_Arrow,SKILL_WIND_ARROW,SIN_SKILL_USE_ALL,Wind_Arrow_UseMana},

{"Perfect Aim",			"B��n mu�i t�n ����c gia t�ng   s��c t��n c�ng va� s�� chi�nh  xa�c",
17,		29,2,		81,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Perfect_Aim,SKILL_PERFECT_AIM,SIN_SKILL_USE_ALL,Perfect_Aim_UseMana},

{"Dion's Eye",			"Luy��n t��m nhi�n nh� xa� thu� Dion huy��n thoa�i t�ng s��    chi�nh xa�c",
20,		0,0,		0,0,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Dions_Eye,SKILL_DIONS_EYE,SIN_SKILL_USE_NOT,0},

{"Falcon",			"Tri��u h��i a�c �i��u t��n c�ng ca�c mu�c ti�u g��n",
23,		36,2,		115,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Falcon ,SKILL_FALCON,SIN_SKILL_USE_RIGHT,Falcon_UseMana },

{"Arrow of Rage",		"B��n nh��ng mu�i t�n l��a ���  xu��ng",
26,		38,2,		82,4,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Arrow_of_Rage,SKILL_ARROW_OF_RAGE,SIN_SKILL_USE_RIGHT,A_Rage_UseMana},

{"Avalanche",			"B��n nh��ng mu�i t�n be�n nho�n",
30,		41,2,		103,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Avalanche,SKILL_AVALANCHE,SIN_SKILL_USE_RIGHT,Avalanche_UseMana},

{"Elemental Shot",		"T��n c�ng li�n tu�c d��a tr�n  hai thu��c ti�nh L��a va� Se�t",
40,		43,2,		82,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_E_Shot,SKILL_ELEMENTAL_SHOT,SIN_SKILL_USE_RIGHT,Elemental_Shot_UseMana},

{"Golden Falcon",		"Tri��u h��i Chim �ng t��n c�ng va� phu�c h��i HP nhanh",
43,		52,2,		120,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_G_Falcon,SKILL_GOLDEN_FALCON,SIN_SKILL_USE_RIGHT,Golden_Falcon_UseMana},

{"Bomb shot",		    "B��n hai mu�i t�n g�y n��",
46,		46,2,		87,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_B_Shot,SKILL_BOMB_SHOT,SIN_SKILL_USE_RIGHT,BombShot_UseMana},

{"Perforation",		"Xuy�n t�m, t��n c�ng nhi��u    mu�c ti�u theo ����ng th��ng",
50,		48,2,		82,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Perforation,SKILL_PERFORATION,SIN_SKILL_USE_RIGHT,Perforation_UseMana},

{"Recall Wolverine",		"Tri��u t��p so�i th��n",
60,		58,2,		900,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Recall_Wolverin,SKILL_RECALL_WOLVERIN,SIN_SKILL_USE_RIGHT,Recall_Wolverin_UseMana},

{"Evasion Mastery",	"T�ng kha� n�ng tra�nh �o�n t��n c�ng t�� ���i thu� ",
63,		0,0,		0,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Evasion_Mastery,SKILL_EVASION_MASTERY,SIN_SKILL_USE_NOT,0},

{"Phoenix Shot",	"T��p trung s��c ma�nh ph���ng  hoa�ng b��t t�� g�y sa�t th��ng ha�ng loa�t",
66,		55,2,		100,0,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Phoenix_Shot,SKILL_PHOENIX_SHOT,SIN_SKILL_USE_RIGHT,Phoenix_Shot_UseMana},

{"Force of Natural","H��i tu� s��c ma�nh thi�n nhi�n t�ng kha� n�ng chi��n ���u cho ca�c tha�nh vi�n nho�m",
70,		56,2,		900,0,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Force_Of_Nature,SKILL_FORCE_OF_NATURE,SIN_SKILL_USE_RIGHT,Force_Of_Nature_UseMana},


	


{"Sword Blast",			"S�� du�ng n�ng l���ng cu�a     ki��m",
10,		22,2,		82,2,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Sword_Blast,SKILL_SWORD_BLAST,SIN_SKILL_USE_ALL,Sword_Blast_UseMana},


{"Holy Body",			"Gia�m sa�t th��ng khi bi�      Undead t��n c�ng",
12,		28,2,		110,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_HolyBody ,SKILL_HOLY_BODY,SIN_SKILL_USE_RIGHT,HolyBody_UseMana },

{"Physical Training",		"T�ng th�m Stamina",
14,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Physical_Traning,SKILL_PHYSICAL_TRANING,SIN_SKILL_USE_NOT,0},

{"Double Crash",		"T��n c�ng hai chi�u li�n tu�c",
17,		35,2,		82,2,
{0,0,0},			{sinWA1,sinWC1,sinWS2,0,0,0,0,0},
F_Double_Crash,SKILL_DOUBLE_CRASH,SIN_SKILL_USE_RIGHT,D_Crash_UseMana},

{"Holy Valor",			"T�ng s��c g�y sa�t th��ng l�n  Undead cho Party",
20,		36,2,		100,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Holy_Valor,SKILL_HOLY_VALOR,SIN_SKILL_USE_RIGHT,Holy_Valor_Mana},

{"Brandish",			"Ra �o�n t�� tr�n cao g�y sa�t th��ng di��n r��ng",
23,		38,2,		82,2,
{0,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Brandish,SKILL_BRANDISH,SIN_SKILL_USE_ALL,Brandish_Mana},

{"Piercing",			"Du�ng mu�i ki��m g�y sa�t      th��ng cao",
26,		41,2,		82,2,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Piercing,SKILL_PIERCING,SIN_SKILL_USE_ALL,Piercing_Mana},

{"Drastic Spirit",		"T�ng th�m s��c pho�ng thu� ",
30,		44,2,		100,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Drastic_Spirit,SKILL_DRASTIC_SPIRIT,SIN_SKILL_USE_RIGHT,Drastic_Spirit_Mana},

{"Sword Mastery",	"T�ng s��c �a�nh khi du�ng ki��m",
40,		0,0,		0,0,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_S_Mastery,SKILL_SWORD_MASTERY,SIN_SKILL_USE_NOT,0},

{"Divine Shield",		"H��p thu� �o�n t��n c�ng cu�a  Undead va� chuy��n m��t ph��n  tha�nh HP",
43,		46,2,		130,1,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_D_Inhalation,SKILL_DIVINE_INHALATION,SIN_SKILL_USE_RIGHT,D_Inhalation_UseMana},

{"Holy Incantation",	"Thu phu�c Undead",
46,		50,2,		120,1,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Holy_Incantation,SKILL_HOLY_INCANTATION,SIN_SKILL_USE_RIGHT,H_Incantation_UseMana},

{"Grand Cross",	"T��n c�ng hai chi�u li�n tu�c  g�y sa�t th��ng cao",
50,		45,2,		82,2,
{0,0,0},			{sinWH1,sinWS2,0,0,0,0,0,0},
F_Grand_Cross,SKILL_GRAND_CROSS,SIN_SKILL_USE_RIGHT,Grand_Cross_UseMana},

{"Devine piercing",	"T�ng kha� n�ng t��n c�ng b��ng s��c ma�nh t�m linh",
70,		51,2,		160,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Divine_Piercing,SKILL_DIVINE_PIERCING,SIN_SKILL_USE_RIGHT,Divine_Piercing_UseMana},

{"Godly Shield",	"T�ng kha� n�ng ch��ng ��� khi  s�� du�ng khi�n",
63,		50,2,		400,0,
{1,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Godly_Shield,SKILL_GODLY_SHIELD,SIN_SKILL_USE_RIGHT,Godly_Shield_UseMana},

{"God Bless",	"T�ng kha� n�ng t��n c�ng b��ng s��c ma�nh t�m linh",
66,		51,2,		330,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_God_Bless,SKILL_GOD_BLESS,SIN_SKILL_USE_RIGHT,God_Bless_UseMana},

{"Sword of Justice","Che�m ma�nh t�� tr�n xu��ng g�y sa�t th��ng ha�ng loa�t",
60,		53,2,		250,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Sword_Of_Justice,SKILL_SWORD_OF_JUSTICE,SIN_SKILL_USE_RIGHT,Sword_Of_Justice_UseMana},


{"Shield Strike",		"G�y choa�ng ���i thu� b��ng    khi�n",
10,		23,2,		82,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_S_Strike ,SKILL_SHIELD_STRIKE,SIN_SKILL_USE_RIGHT,S_Strike_UseMana },

{"Farina",			"Pho�ng lao ta�o a�o a�nh",
12,		26,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Farina,SKILL_FARINA,SIN_SKILL_USE_ALL,Farina_UseMana},

{"Throwing Mastery",		"T�ng s��c sa�t th��ng cu�a lao",
14,		0,0,		0,0,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_D_Mastery,SKILL_THROWING_MASTERY,SIN_SKILL_USE_NOT,0},

{"Bigger Spear",		"T��n c�ng b��ng ngo�n gia�o    a�nh sa�ng",
17,		32,2,		82,2,
{0,0,0},			{sinWP1,sinWT1,0,0,0,0,0,0},
F_V_Spear,SKILL_VIGOR_SPEAR,SIN_SKILL_USE_ALL,V_Spear_UseMana},

{"Windy",			"T�ng khoa�ng ca�ch t��n c�ng   va� s�� chi�nh xa�c cu�a lao",
20,		34,2,		100,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Windy,SKILL_WINDY,SIN_SKILL_USE_RIGHT,Windy_Mana},

{"Twist Javelin",		"Pho�ng lao xoa�y t�ng s��c     �a�nh va� ��� chi�nh xa�c",
23,		36,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Twist_Javelin,SKILL_TWIST_JAVELIN,SIN_SKILL_USE_RIGHT,Twist_Javelin_Mana},

{"Soul Sucker",			"Hu�t ma�u qua�i v��t",
26,		42,2,		110,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Soul_Sucker,SKILL_SOUL_SUCKER,SIN_SKILL_USE_RIGHT,Soul_Sucker_Mana},

{"Fire Javelin",		"T��n c�ng b��ng gia�o l��a",
30,		40,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Fire_Javelin,SKILL_FIRE_JAVELIN,SIN_SKILL_USE_ALL,Fire_Javelin_Mana},

{"Split javelin",	"T��n c�ng li�n tu�c v��i t��c  ��� nhanh",
40,		43,2,		115,1,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Split_Javelin,SKILL_SPLIT_JAVELIN,SIN_SKILL_USE_RIGHT,Split_Javelin_UseMana},

{"Triumph of Valhalla",	"T�ng s��c g�y sa�t th��ng cho  Party",
43,		50,2,		120,1,
{0,0,0},			{sinWT1,sinWS2,sinWS1,sinWP1,sinWH1,sinWA1,sinWC1,sinWM1},
F_T_Of_Valhalla,SKILL_TRIUMPH_OF_VALHALLA,SIN_SKILL_USE_RIGHT,T_Of_Valhalla_UseMana},

{"Lightning javelin",	"T��n c�ng b��ng gia�o �i��n",
46,		46,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Lightning_Javelin,SKILL_LIGHTNING_JAVELIN,SIN_SKILL_USE_ALL,Lightning_Javelin_UseMana},

{"Storm javelin",	    "T��n c�ng b��ng cu��ng phong",
50,		48,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Storm_Javelin,SKILL_STORM_JAVELIN,SIN_SKILL_USE_RIGHT,Storm_Javelin_UseMana},

{"Hell of Vahalla",	"T�ng kha� n�ng t��n c�ng va�   tra�nh �o�n",
60,		52,2,		750,0,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS1,sinWT1,0,0},
F_Hall_Of_Valhalla,SKILL_HALL_OF_VALHALLA,SIN_SKILL_USE_RIGHT,Hall_Of_Valhalla_UseMana},

{"X Rage",	"T��n c�ng li�n tu�c ���i thu�  t�� tr�n cao",
63,		48,2,		210,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_X_Rage,SKILL_X_RAGE,SIN_SKILL_USE_RIGHT,X_Rage_UseMana},

{"Frost Javelin",	"T�ng s��c t��n c�ng vu� khi�   b��ng s��c ma�nh b�ng gia� ",
66,		50,2,		720,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Frost_Javelin,SKILL_FROST_JAVELIN,SIN_SKILL_USE_RIGHT,Frost_Javelin_UseMana},

{"Venchence",	"�o�n t��n c�ng li�n tu�c nhanh va� ma�nh",
70,		55,2,		100,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Vengeance,SKILL_VENGEANCE,SIN_SKILL_USE_RIGHT,Vengeance_UseMana},


{"Healing",			 "Phu�c h��i ma�u b��ng phe�p    thu��t",
10,		15,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Healing,SKILL_HEALING,SIN_SKILL_USE_RIGHT,Healing_UseMana},

{"Holy Bolt",			"Tri��u h��i c��u se�t g�y sa�t th��ng",
12,		11,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_HolyBolt,SKILL_HOLY_BOLT,SIN_SKILL_USE_ALL,HolyBolt_UseMana},

{"Multi Spark",			"Go�i tia l��a ma thu��t t��n   c�ng mu�c ti�u",
14,		14,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_M_Spark,SKILL_MULTISPARK,SIN_SKILL_USE_RIGHT,M_Spark_UseMana},

{"Holy Mind",			"La�m mu�c ti�u y��u �i",
17,		19,1,		100,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_HolyMind,SKILL_HOLY_MIND,SIN_SKILL_USE_RIGHT,HolyMind_UseMana},

{"Meditation",			"T�ng kha� n�ng phu�c h��i Mana",
20,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Meditation,SKILL_MEDITATION,SIN_SKILL_USE_NOT,0},

{"Divine Lightning",		"T��n c�ng b��ng nh��ng tia se�t thi�ng",
23,		21,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Divine_Lightning,SKILL_DIVINE_LIGHTNING,SIN_SKILL_USE_RIGHT,Divine_Lightning_Mana},

{"Holy Reflection",		"Pha�n h��i �o�n t��n c�ng cu�a Undead b��ng vo�ng ma thu��t",
26,		23,1,		120,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Holy_Reflection,SKILL_HOLY_REFLECTION,SIN_SKILL_USE_RIGHT,Holy_Reflection_Mana},

{"Grand Healing",		"H��i s��c cho ca� Party",
30,		27,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Grand_Healing,SKILL_GRAND_HEALING,SIN_SKILL_USE_RIGHT,Grand_Healing_Mana},

{"Vigor Ball",			"Ne�m hai qua� c��u Vigor ���n  mu�c ti�u",
40,		24,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Vigor_Ball,SKILL_VIGOR_BALL,SIN_SKILL_USE_ALL,Vigor_Ball_UseMana},

{"Resurrection",		"H��i sinh m��t tha�nh vi�n     Party",
43,		31,1,		130,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Resurrection,SKILL_RESURRECTION,SIN_SKILL_USE_RIGHT,Resurrection_UseMana},

{"Extinction",		"Th��n chu� g�y ra sa�t th��ng  cho Undead",
46,		28,1,		120,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Extinction,SKILL_EXTINCTION,SIN_SKILL_USE_RIGHT,Extinction_UseMana},

{"Virtual life",	"T�ng ma�u cho tha�nh vi�n Party trong m��t khoa�ng th��i gian nh��t �i�nh",
50,		31,1,		134,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Virtual_Life,SKILL_VIRTUAL_LIFE,SIN_SKILL_USE_RIGHT,Virtual_Life_UseMana},

{"Glacial Spike",	"Tri��u h��i b�ng gia� t��n c�ng ���i thu� ",
60,		35,1,		120,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Glacial_Spike,SKILL_GLACIAL_SPIKE,SIN_SKILL_USE_RIGHT,Glacial_Spike_UseMana},

{"Regeneration Field",	"T�ng kha� n�ng h��i phu�c HP   va� MP cho ���ng ���i",
63,		41,1,		950,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Regeneration_Field,SKILL_REGENERATION_FIELD,SIN_SKILL_USE_RIGHT,Regeneration_Field_UseMana},

{"Chain Lightening",	"S�� du�ng s��c ma�nh cu�a l�i  th��n t��n c�ng nhi��u mu�c    ti�u",
66,		41,1,		120,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Chain_Lightning,SKILL_CHAIN_LIGHTNING,SIN_SKILL_USE_RIGHT,Chain_Lightning_UseMana},

{"Summon Muspell",	"K�u go�i s�� giu�p ��� cu�a    ca�c thi�n th��n",
70,		47,1,		800,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Summon_Muspell,SKILL_SUMMON_MUSPELL,SIN_SKILL_USE_RIGHT,Summon_Muspell_UseMana},


{"Agony",			"Chuy��n HP tha�nh Mana theo ti� l��",
10,		12,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Agony,SKILL_AGONY,SIN_SKILL_USE_RIGHT,Agony_UseMana},

{"Fire Bolt",			"T��n c�ng b��ng phe�p l��a c�  ba�n",
12,		11,1,		84,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_FireBolt,SKILL_FIRE_BOLT,SIN_SKILL_USE_ALL,FireBolt_UseMana},

{"Zenith",			"T�ng c���ng kha� n�ng ch��ng   ��� b��ng phe�p thu��t",
14,		15,1,		100,3,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Zenith,SKILL_ZENITH,SIN_SKILL_USE_RIGHT,Zenith_UseMana},

{"Fire Ball",			"Phe�p t��n c�ng thu��c ti�nh   l��a co� t��m sa�t th��ng r��ng",
17,		19,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_FireBall,SKILL_FIRE_BALL,SIN_SKILL_USE_RIGHT,FireBall_UseMana},

{"Mental Mastery",		"T�ng t��ng s�� mana hi��n co� ",
20,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Mental_Mastery,SKILL_MENTAL_MASTERY,SIN_SKILL_USE_NOT,0},

{"Watorado",			"Phe�p thu��t t��n c�ng b��ng   c��t n���c xoa�y",
23,		21,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Watornado,SKILL_WATORNADO,SIN_SKILL_USE_RIGHT,Watornado_Mana},

{"Enchant Weapon",		"��m bu�a cho vu� khi� cu�a     tha�nh vi�n party",
26,		28,1,		104,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Enchant_Weapon,SKILL_ENCHANT_WEAPON,SIN_SKILL_USE_RIGHT,Enchant_Weapon_Mana},

{"Death Ray",			"T��n c�ng b��ng m��t lu��ng    sa�ng",
30,		26,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Dead_Ray,SKILL_DEAD_RAY,SIN_SKILL_USE_ALL,Dead_Ray_Mana},

{"Energy shield",		"Ta�o vo�ng ma thu��t ch��ng ��� xung quanh c� th��",
40,		40,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Energy_Shield,SKILL_ENERGY_SHIELD,SIN_SKILL_USE_RIGHT,Energy_Shield_UseMana},

{"Diastrophism",	"Phe�p thu��t t��n c�ng ca�c    mu�c ti�u xung quanh",
43,		33,1,		106,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Diastrophism,SKILL_DIASTROPHISM,SIN_SKILL_USE_RIGHT,Diastrophism_UseMana},

{"Spirit elemental",	"Tri��u h��i tinh linh giu�p gia t�ng phe�p thu��t",
46,		35,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Spirit_Elemental,SKILL_SPIRIT_ELEMENTAL,SIN_SKILL_USE_RIGHT,Spirit_Elemental_UseMana},

{"Dancing sword",	"Tri��u t��p ma ki��m t��n c�ng",
50,		30,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_D_Sword,SKILL_DANCING_SWORD,SIN_SKILL_USE_RIGHT,Dancing_Sword_UseMana},

{"Mana Recharging",	"Phu�c h��i Mana cho ca�c tha�nh vi�n",
60,		120,2,		900,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Fire_Elemental,SKILL_FIRE_ELEMENTAL,SIN_SKILL_USE_RIGHT,Fire_Elemental_UseMana},

{"Frame Wave",	"T��n c�ng di��n r��ng b��ng    phe�p thu��t l��a",
63,		32,1,		124,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Flame_Wave,SKILL_FLAME_WAVE,SIN_SKILL_USE_RIGHT,Flame_Wave_UseMana},

{"Destruction",	"S�� du�ng a�m thu��t la�m y��u ���i thu� ",
66,		43,1,		460,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Distortion,SKILL_DISTORTION,SIN_SKILL_USE_RIGHT,Distortion_UseMana},

{"Meteor",	"Tri��u h��i thi�n tha�ch t��n  c�ng nhi��u ���i thu� ",
70,		39,1,		190,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Meteo,SKILL_M_METEO,SIN_SKILL_USE_RIGHT,Meteo_UseMana},

{"Orb",		"T�ng kha� n�ng t��n c�ng b��ng s��c ma�nh cu�a Orb",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SKILL_FORCE_ORB,0,0},


{"Quest",		"Ti�m va� di��t qua�i thu�!",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CHANGE_JOB3_QUEST,0,0},


{"Quest",		"Ti�m va� di��t qua�i thu�!",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CHANGE_ELEMENTARY_QUEST,0,0},


{"Absorption of Glory","S��c chi�u ���ng +10 trong vo�ng 1 tu��n",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ABSORB,0,0},

{"Power Siege","S��c �a�nh t�ng th�m 10% trong vo�ng 1 tu��n",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ATTACK,0,0},

{"Evasion Adaption","Kha� n�ng tra�nh �o�n t�ng th�m 10% trong 1 tu��n",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_EVASION,0,0},


{"Atanasia","S��c �a�nh gia�m �i 1/2 va� b��t t�� trong m��t khoa�ng th��i gian",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_INVULNERABILITY,0,0},

{"Deadly Edge","T�ng 5% kha� n�ng ra �o�n chi� ma�ng",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_CRITICAL,0,0},

{"Avail of Evasion","T�ng 5% kha� n�ng tra�nh �o�n",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_EVASION,0,0},

{"Defiance Stone","Gia�m 50% s��c sa�t th��ng cu�a Tha�p L��a v��i c� th��",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_FIRECRYTAL,0,0},

{"Defiance Stone","Gia�m 50% s��c sa�t th��ng cu�a Tha�p B�ng v��i c� th��",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_ICECRYTAL,0,0},

{"Defiance Stone","Gia�m 50% s��c sa�t th��ng cu�a Tha�p A�nh Sa�ng v��i c� th��",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_LINGHTINGCRYTAL,0,0},

{"Might Stone","Gia t�ng th�m 20% s��c ma�nh v��i ���i thu� la� Fighter",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_FIGHTER,0,0},

{"Might Stone","Gia t�ng th�m 20% s��c ma�nh v��i ���i thu� la� Mechanician",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_MECHANICIAN,0,0},

{"Might Stone","Gia t�ng th�m 20% s��c ma�nh v��i ���i thu� la� Pikeman",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_PIKEMAN,0,0},

{"Might Stone","Gia t�ng th�m 20% s��c ma�nh v��i ���i thu� la� Archer",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_ARCHER,0,0},

{"Might Stone","Gia t�ng th�m 20% s��c ma�nh v��i ���i thu� la� Knight",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_KNIGHT,0,0},

{"Might Stone","Gia t�ng th�m 20% s��c ma�nh v��i ���i thu� la� Atalanta",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_ATALANTA,0,0},

{"Might Stone","Gia t�ng th�m 20% s��c ma�nh v��i ���i thu� la� Magician",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_MAGICIAN,0,0},

{"Might Stone","Gia t�ng th�m 20% s��c ma�nh v��i ���i thu� la� Priestess",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_PRIESTESS,0,0},

{"Eternal Life","Tr�� n�n b��t t�� trong m��t khoa�ng th��i gian. Nh�ng s��c �a�nh se� gia�m �i 1/2",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_INVULNERABILITY,0,0},

{"T�� �ao","T�ng th�m 15% kha� n�ng ra �o�n chi� ma�ng",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_CRITICAL,0,0},

{"Bu�a H�� th�n","T�ng th�m 15% kha� n�ng ne� �o�n",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_EVASION,0,0},


