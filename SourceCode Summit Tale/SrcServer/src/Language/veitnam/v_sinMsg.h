#include "sinLinkHeader.h"

sMESSAGEBOX	sMessageBox[100] = {
	{"Kh�ng �u� khoa�ng tr��ng trong ng�n ch��a ���  "},
	{"V���t qua� c�n n��ng cho phe�p "},   
	{"Mo�n ��� na�y kh�ng th�� s�� du�ng ����c "},	   
	{"Kh�ng �u� va�ng "},
	{"Mo�n ��� na�y kh�ng th�� Mix ����c "},
	{"V���t qua� gi��i ha�n cu�a s�� l���ng ��� ����c trao ���i "},	
	{"Giao di�ch bi� huy� bo�  "},
	{"Kh�ng th�� l�u tr�� Potions "},
	{"Mix mo�n ��� na�y th��t ba�i "},
	{"Ca�m �n s�� �o�ng go�p quy� ba�u cu�a ba�n "},
	{"Co� qua� nhi��u mo�n ��� trong nha� kho "},
	{"Mo�n ��� na�y kh�ng th�� Age ����c "},
	{"M��t va�i ��� v��t �a� bi� thay ���i. Vui lo�ng ki��m tra la�i "},
	{"V���t qua� gi��i ha�n va�ng ����c phe�p s�� h��u "}, 
	{"Age mo�n ��� na�y th��t ba�i "},
	{"�i��m Statistics �a� ����c thi��t l��p la�i. "},
	{"Item image isn't prepared "},
	{"Ba�n chi� co� th�� la�m �i��u na�y m��t l��n th�i "},
	{"Chi� da�nh cho tha�nh vi�n cu�a Clan "},		
	{"Ba�n pha�i gia nh��p m��t Clan m��i co� th�� s�� du�ng "},		
	{"Kh�ng th�� di�ch chuy��n vi� gi��i ha�n Level "},
	{"C��p ��� Age �a� bi� tr�� �i -1 "},
	{"C��p ��� Age �a� bi� tr�� �i -2 "},
	{"C��p ��� Age �a� ����c t�ng l�n +1 "},
	{"C��p ��� Age �a� ����c t�ng l�n +2 "},
	{"T��c ��� ATK cu�a mo�n ��� nhi��m vu�  "},
	{"   N�ng level mo�n ��� nhi��m vu� "},
	{"   HP ����c t�ng th�m +15 "},
	{"   Puzzle �a� hoa�n t��t  "},
	{"   Puzzle s��p x��p kh�ng �u�ng "},
	{"   Kh�ng co� gi� ��� ba�n ca�  "},
	{"Mo�n ��� na�y �a� ����c ba�n r��i "},
	{"Gian ha�ng ca� nh�n �a� bi� �o�ng la�i "},
	{"   Kh�ng ti�m th��y ng���i ba�n "},
	{"    S�� l���ng mo�n ��� kh�ng �u�ng  "},
	{"    Nh�n v��t nam kh�ng th�� s�� du�ng "},
	{"    Nh�n v��t n�� kh�ng th�� s�� du�ng "},
	{" Nh�n v��t n�� kh�ng th�� s�� du�ng ����c mo�n ��� na�y "},
	{" Nh�n v��t nam kh�ng th�� s�� du�ng ����c mo�n ��� na�y "},
	{"    Gia� cu�a mo�n ��� �a� thay ���i "},
	{"    Kh�ng co� ti��n th���ng cho Clan "},
	{"  Level cu�a ba�n ch�a th�� du�ng ����c loa�i force na�y   "},
	{"     Force Orb �ang ����c s�� du�ng  "},
	{"     Ch��c n�ng ATK POW �ang ����c th�� nghi��m    "},
	{"    Qua� tri�nh Force Orb �a� hoa�n t��t     "},
	{"    �i��m Stats ����c t�ng th�m 5  "},
	{"    �i��m Skill ����c t�ng th�m 1  "},
	{" �i��m Stats & Skill ����c t�ng th�m 5 va� 1 "},
	{" Stat & skill point ����c t�ng th�m 5 va� 2 "},
	{"   Ph��n tr�m XP bi� m��t khi ch��t gia�m �i 1   "},
	{"   Vitality t�ng th�m 40   " },
	{"    Ba�n �ang th��c hi��n nhi��m vu�     " },
	{"T�� level 80, ba�n se� ����c 7 �i��m khi �a�t th�m 1 level." },
	{"T�� level 90, ba�n se� ����c 10 �i��m khi �a�t th�m 1 level." },
	{"Resetted your getting-number for tranning." },
	{"Nhi��m vu� kh�ng th�� hoa�n tha�nh do h��t th��i gian cho phe�p" },
	{"Co� qua� nhi��u li�nh �a�nh thu�. "},
	{"M��t mo�n ��� cu�ng loa�i �ang ����c s�� du�ng. "},
	{"Kh�ng th�� di�ch chuy��n t��i Bless Castle "},	
	{"Mo�n ��� na�y kh�ng th�� ba�n trong gian ha�ng ca� nh�n "},
	{"Kh�ng co� �u� s�� ti��n thu�� ��� ru�t ra. "},
	{"Ba�n �a� tra� l��i c�u ho�i na�y r��i. "},
	{"Ca�m �n ba�n �a� tra� l��i c�u ho�i. "}, 
	{"Level cu�a mo�n ��� kh�ng phu� h��p. "},
	{"Mo�n ��� �a� ����c bi��n ���i hoa�n t��t. "},
	{"Chi��n th��ng trong tr��n ���u v��i Fury. "},
	{"Ba�n �a� thua trong tr��n ���u v��i Fury "},
	{"Acquierd items in compensation. "},
	{"Ca�c mo�n ��� na�y kh�ng th�� s�� du�ng ����c cu�ng m��t lu�c. "},
	{"Able to use later "},
	{"Kh�ng th�� huy� bo� khi ca�c Sheltom �ang ����c Mix/Age. "},
	{"Kh�ng th�� s�� du�ng khi ca�c Sheltom �ang ����c Mix/Age. "},
	{"Cu�ng m��t ki��u to�c �ang ����c s�� du�ng. "},
	{"Qua� tri�nh luy��n kim th��t ba�i. "}, 
	{"Qua� tri�nh ch�� ta�o th��t ba�i. "}, 
	{"Trong khi tr��n Sheltoms, kh�ng th�� s�� du�ng Ore/Crystal. "}, 
	{"Trong khi tr��n Runes, kh�ng th�� s�� du�ng Recipe. "}, 
	{"Ore/Crystal kh�ng cu�ng loa�i "}, 
	{"C�ng th��c ch�� ta�o kh�ng �u�ng "}, 
};



sMESSAGEBOX2 sMessageBox2[20] = {
	{"Bao nhi�u mo�n ��� ","ba�n mu��n mua? "},
	{"Bao nhi�u mo�n ��� ","ba�n mu��n ba�n? "},
	{"Bao nhi�u mo�n ��� ","ba�n mu��n qu��ng �i? "},
	{"Bao nhi�u va�ng","ba�n mu��n qu��ng �i? "},
	{""," "}, 
	{"Bao nhi�u va�ng"," ba�n mu��n trao ���i? "},
	{"Bao nhi�u va�ng","ba�n mu��n c��t �i? "},
	{"Bao nhi�u va�ng","ba�n mu��n l��y la�i? "},
	{" Bao nhi�u ng�i sao ","ba�n mu��n mua? "},
	{"  Bao nhi�u ti��n  ","ba�n mu��n quy�n go�p? "}, 
	{"Do you wanna register","gia� ti��n cu�a mo�n ���?  "}, 
	{"Bao nhi�u mo�n ��� ","ba�n mu��n mua? "},  
	{"Ba�n mu��n ���t gia� ","ba�n mu��n l��y la�i? "}, 
	{"  Ba�n co� mu��n ","mua mo�n ��� na�y kh�ng? "}, 
	{"  Ba�n co� mu��n  ","mua mo�n ��� na�y kh�ng? "},
	{"Bao nhi�u ti��n","ba�n mu��n ru�t ra? "},
	{"Bao nhi�u li�nh �a�nh thu� ","ba�n mu��n mua? "} 
};



sMESSAGEBOX3 sMessageBox3[MESSAGE3_MAX] = {
	{"Ba�n co� mu��n","ho�c Skill na�y kh�ng? "},
	{"Ba�n co� mu��n", "phong ch��c kh�ng? "},
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "},
	{"Ba�n co� mu��n","ho�c la�i ca�c Skill kh�ng? "},		
	{"Se� t��n va�ng"," cho s�� ki��n na�y "},
	{"Ba�n co� mu��n","�i��u chi�nh la�i STATS kh�ng? "},
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "},
	{"Ba�n co� mu��n", "nh��n ��� kh�ng? "}, 
	{"Ba�n co� mu��n", "ky� va�o Union Core kh�ng? "}, 
	{"Ba�n co� mu��n", "nh��n ��� kh�ng? "}, 
	{"Ba�n co� mu��n", "chuy��n t��i ? "}, 
	{"","Vui lo�ng nh��p va�o ch�� s�� "}, 
	{"Ba�n co� mu��n","purchase star item? "}, 
	{"Ba�n co� mu��n", "ba�n v��t na�y kh�ng? "}, 
	{"Ba�n co� mu��n", "nhi��m vu�? "}, 
	{" Ba�n co� mu��n", "quy�n go�p kh�ng? "}, 
	{" Private Store ", "fffffffffff "}, 
	{"Ba�n co� mu��n", "���i mo�n ��� kh�ng?  "}, 
	{"Ba�n co� mu��n", "nh��ng mo�n ���? "}, 
	{"Ba�n co� mu��n ", "huy� bo� ��ng ky�? "}, 
	{"Ba�n co� mu��n", "���i mo�n ���?  "}, 
	{"Ba�n co� mu��n", "���i mo�n ���?  "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"N�i na�o", "ba�n mu��n di chuy��n t��i "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th�ng ha�ng kh�ng? "}, 
	{"Ba�n co� mu��n", "trao ���i mo�n ���?  "},
	{"Ba�n mu��n di�ch chuy��n", "t��i ��y kh�ng? "},    
	{"Ba�n co� mu��n mua","m��t toa� tha�p kh�ng? "},      
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "th��c hi��n nhi��m vu� kh�ng? "}, 
	{"Ba�n co� mu��n", "s�� du�ng mo�n ��� na�y? "},       
	{"Ba�n co� mu��n ", "th��c hi��n nhi��m vu� kh�ng? "},
	{"Ba�n co� mu��n ", "nh��n mo�n ��� kh�ng? "},   
	{"Ba�n co� mu��n","di chuy��n t��i Zone kh�ng? "},   
	{"Ba�n co� mu��n", "di�ch chuy��n kh�ng? "},
	{"Di�ch chuy��n", "Ba�n ch��c ch��n ch��? "},    
	{"Item", "se� ��a va�o lo� luy��n kim? "}, 
	{"Ch�� ta�o", "Ba�n ch��c ch��n ch��?? "},    
};


char *SkillMaster[4] = {
	"Ho�c Skill",
	"Change Class",
	"Thoa�t Game"

};
char *SkillMaster2[4] = {
	"",
	"",
	""

};

char *ReStartMsg[4] = {
	" B��t ���u la�i t�� ch�� na�y",
	" Quay tr�� v�� tha�nh",
	" Thoa�t Game",
	" Kh�ng �u� XP"

};
char *ReStartMsg2[4] = {
	" Pha�t th�m (Gold/XP) 1%",
	"",
	"",
	" ��� b��t ���u la�i t�� ch�� na�y",

};

char *SkillUseInfoText[10]= { "    Ho�c Skill/Magic     ","    L��a cho�n m��t Skill/Magic     ","      Ho�c ho��c luy��n t��p       ","  Kh�ng co� �u� �i��m Skill ","   ��� ho�c hay luy��n t��p" };
char *sinMsgSkillPoint = "    Skill Points : %d";
char *sinMsgEliteSkillPoint = "Elite Skill Points : %d " ;
char *sinStateName[5]={"H P : %d/%d  ","M P : %d/%d  ","STM : %d/%d  ","X P : %d/%d  ","X P : %dM/%dM   "};


char *TownName[3] = {"Quay v�� Ricarten\r","Quay v�� Navisko\r","Quay v�� Pillai\r "};

char *sinAbilityName[50] = {"S��c �a�nh:  \r","T��c ���:  \r","T��m xa:  \r","Critical:  \r","S��c ch��ng ���:  \r","Ty� l�� �a�nh:  \r",
					"S��c chi�u ���ng:  \r","Kha� n�ng ���:  \r","T��c ���:  \r","S�� toa�n ve�n:  \r","Phu�c h��i MP:  \r","Phu�c h��i HP :  \r",
					"Phu�c h��i STM:  \r","Ch��ng v��t ly�:  \r","Nature Type:  \r","Ch��ng l��a:  \r","Ch��ng b�ng:  \r","Ch��ng se�t:  \r","Ch��ng ���c: \r",
					"Water Type:  \r","Wind Type:  \r","Phu�c h��i HP:  \r","Phu�c h��i MP: \r","Phu�c h��i STM: \r","C��ng th�m HP: \r",
					"C��ng th�m MP:  \r","C��ng th�m STM:  \r","Ch��a Potion:  \r","C��n Level:  \r","C��n Strength:  \r","C��n Spirit:  \r",
					"C��n Talent:  \r","C��n Agility:  \r","C��n Health:  \r "};


char *sinSpecialName[50] = {"Spec. ATK SPD:  \r","Spec. CRIT:  \r","Spec. DEF RTG:  \r","Spec. ABS RTG:  \r",
						"Spec. BLK RTG:  \r","Magic APT:  \r","Spec. SPD:  \r","Spec. Organic:  \r","Spec. Nature:  \r",
						"Spec. Flame:  \r","Spec. Frost:  \r","Spec. Lightning:  \r","Spec. Poison:  \r","Spec. Water:  \r","Spec. Wind:  \r",
						"Spec. ATK POW:  \r","Spec. ATK RTG:  \r","Spec. RNG:  \r","Spec. Organic ATK:  \r","Spec. Nature ATK:  \r","Spec. Flame ATK:  \r","Spec. Frost ATK:  \r",
						"Spec. Lightning ATK:  \r","Spec. Poison ATK:  \r","Spec. Water ATK:  \r","Spec. Wind ATK:  \r","Max HP Boost:  \r","Max MP Boost:  \r",
						"Phu�c h��i HP:  \r","Phu�c h��i MP:  \r","Phu�c h��i STM:  \r","Gia� ba�n  \r","Gia�:  \r "};


char *SpecialName3 = "%s Spec. \r";
char *MixResultTitle = "K��t qua� Mix";
char *NowLevelText = "Level hi��n ta�i\r";
char *NextLevelText = "Level k�� ti��p\r";

char *NormalAttckName ="Default Attack";
char *RequirLevel = "%s (Req. LVL:%d)\r";
char *UseItemGroupName = "Chi� s�� du�ng ����c v��i:\r";

char *PoisonName = "Poison Type:\r";
char *FireName = "Flame Type:\r";
char *AttackAreaName = "T��m a�nh h���ng:\r";
char *Attack_RateName = "Ty� L�� �a�nh:\r";
char *Max_DamageIncre = "T�ng Max Damage:\r";
char *DamageAddName = "T�ng s��c sa�t th��ng:\r";
char *Attack_SpeedAdd = "T�ng t��c ��� �a�nh:\r";
char *IncreBlock = "T�ng kha� n�ng ���:\r";
char *CountinueTime = "Th��i gian:\r";
char *UseManaName = "MP s�� du�ng:\r";
char *DamageName = "Damage:\r";
char *ShootingRangeName = "t��m xa:\r";
char *IncreAsorb = "T�ng chi� s�� chi�u ���ng:\r";
char *SecName = "gi�y";
char *PlusDamageName = "C��ng s��c �a�nh:\r";
char *HitNumName = "S�� l��n �a�nh:\r";
char *DecreLifeName = "Gia�m HP:\r";
char *Attck_RateAdd = "C��ng ty� l�� �a�nh:\r";
char *Add_CriticalName = "C��ng ty� l�� Critical:\r";
char *Push_AreaName = "T��m ���y lu�i:\r";
char *FireAttackDamageAdd ="C��ng phe�p l��a:\r";
char *IceName = "Thu��c ti�nh B�ng:\r";
char *IceAttackDamageAdd = "C��ng phe�p b�ng:\r";
char *StuneRate = "Ty� l�� g�y choa�ng:\r";
char *DefenseRateIncre = "T�ng s��c ch��ng ���:\r";
char *WeaponSizeName ="Ki�ch c�� vu� khi�:\r";
char *HwakRotationNum = "S�� l��n t��n c�ng:\r";
char *WeaponSpeedAddName  = "C��ng t��c ��� �a�nh:\r";
char *IncreAttack_RateName = "T�ng ty� l�� �a�nh:\r";
char *ShootingNumName = "S�� l��n b��n:\r";
char *SeriesShootingCount = "S�� l��n t��n c�ng:\r";
char *UseStaminaName = "Stamina s�� du�ng:\r";
char *MasterSkillMoneyName = "Gia� ti��n:\r";

char *ItemAgingResult;		

char *sinGold ="%d Gold";
char *sinGiveItem5 = "Gift Item\r";
char *sinCopyItem5 = "Copied Item\r";
char *sinNum7 = "RND";
char *sinSkillPointName = "Skill Point";	
char *sinG_Pike_Time3 = "Frozen Time:\r";	
char *sinItemLimitTimeOverMsg = "Th��i gian cho phe�p �a� h��t\r";
char *sinDeadSongPyeunEat = "Darn it! It's rotten rice cake!";
char *SmeltingResultTitle = "K��t qua� luy��n kim";		
char *ManufactureResultTitle = "K��t qua� ch�� ta�o";	
char *ManufacturingTitle = "Mo�n ��� ng��u nhi�n";	


char *sinMaxDamageAdd = "Increase in Maximum damage+\r";
char *sinDamagePiercing = "(Piercing attack)\r";
char *sinAddStamina = "Added stamina:\r";
char *sinAddLife = "Life recovery:\r";

char *sinDamageUndead50 = "50% more damages to Undead units:\r";
char *sinNumCount2 = "number:\r";
char *sinNumCount3 = "quantity\r";
char *sinDecreDamage = "Decrease in damage:\r";
char *sinConvert4 = "conversion rate:\r";
char *sinIncreElement = "Increase in element feature:\r";
char *sinPartyIncreElement = "Party member gather 50% increase of element\r";
char *sinFireBallDamage2 = "(100% damage to the targeted enemy)\r";
char *sinFireBallDamage3 = "(targeted area 60 (1/3 damage))\r";
char *sinAddDamage7 = "added damage:\r";
char *PlusDamageWeapon = "weapon";
char *SparkDamage = "damage per unit:\r";
char *SparkNum7 = "# of maximum spark:\r";
char *StunRage = "Stun Range:\r";
char *RecvPotion7 = "Potion";


char *PartyArea7 ="Party Area:\r";
char *Area17 ="Area:\r";
char *AddAttack_Rate9 ="Attack Rating:\r";
char *PiercingRange ="Piercing Range:\r";
char *IncreDefense = "Increase Defense:\r";
char *AddShootingRange = "Increased SHT range:\r";
char *LifeAbsorb = "Life absorb.:\r";
char *FireDamage = "Fire Damage:\r";
char *IceDamage = "Ice Damage:\r";
char *LightningDamage = "Lightning Damage:\r";
char *AddManaRegen = "Add Mana Regen:\r";
char *LightNum = "# of Lightning:\r";
char *ReturnDamage = "Return Damage:\r";
char *Area18 = "Available Range:\r";
char *IncreMana5 = "Increase Mana:\r";
char *Area19 = "Available Area:\r";
char *HelpTitle8 = " T R ��  G I U� P ";
char *QuestTitle8 = " N H I �� M  V U� ";
char *ResearchTitle8 = "RESEARCH";
char *TeleportTitle8 = "TELEPORT";

char *IncreWeaponAttack_RateName = "Increased ATK rating:\r";
char *AddMaxDamage = "Increase maximum damage:\r";
char *LinkCoreName = "Di�ch chuy��n t��c th��i t��i tha�nh vi�n cu�a Clan\r";
char *DesLinkCore = "Name:";
char *Itemul = "item";
char *ItemRecvOk = "Ba�n co� mu��n v��t na�y kh�ng?";
char *Money5 = "gold";
char *Exp5 = "EXP";
char *WingItemName[] = {"Metal wing","Silver wing","Gold wing","Diamond wing", "Chaos wing", "Extreme wing "};
char *WarpGateName[] = {"Warp gate","Ricarten","Pillai","R��ng c��","R��ng tru�c","La�ng Ruinen","Thi� tr��n Navisko","Vu�ng ���t c��m", "La�ng Eura", "Lost Island "};
char *sinWarningName5 = "Chu� y� ";
char *ClanCristalName = "Da�nh ri�ng cho tha�nh vi�n Clan\r";
char *NowMyShopSell = "�ang ba�n\r";
char *SecretNumName = "Nh��p va�o ma� s�� ki��m soa�t";
char *MyShopExpDoc7 = "Th�ng tin v�� c��a ha�ng";
char *NotAgingItem2 = "'Kh�ng th�� Age mo�n ��� na�y";
char *ExpPercent2 = "EXP : %d.%d%s";
char *ExpPercent3 = "EXP : %d.0%d%s";
char *Mutant7   =   "Mutant";
char *Mechanic7 = "Mechanic";
char *Demon7    = "Demon";
char *Nomal7    = "Normal";
char *Undead7    = "Undead";
char *MonsterAddDamage2 = "add extra damamge)\r";
char *MonsterAddDamageFire = "add fire element damage)\r";
char *HaWarpGateName[] = {"Bless Castle", 0};
char *FallGameName = "Twisted forest";
char *EndlessGameName	= "Endless Tower";
char *LookHelp = ">>more..";


char *HoldStarNumDoc = "The number of your stars";
char *StarItemBuy7   = "Purchase the stars";
char *ChangeMoney7   = "(The price of one star: 100000)"; 



char *LightningDamage2 = "LightningDamage:\r";
char *SheildDefense = "Increasing Defense:\r";

char *DemonDamage4  = "DemonVSATKpower:\r";
char *PoisonDamage3  = "PoisonDamage:\r";
char *PikeNum4       = "Number of spear:\r";
char *poisoningTime  = "poisoningTime:\r";
char *PlusCriticalName = "Increasing Critical%:\r";
char *SpiritFalconDamage2 = "SpritFalconDamage:\r";
char *LifeGegenPlus = "Increaing HP:\r";
char *PlusDamage4 = "PlusDamage:\r";
char *PiercingPercent = "PiercingPercent:\r";
char *DamageToLife = "DamageToLife:\r";
char *MyMonster7 = "MyMonster:\r";
char *LifeIncre4 = "LifeIncre:\r";
char *UndeadDamage3 = "UndeadDamage:\r";

char *AttackNum3     = "NumberofATK:\r"; 
char *AttackRateMinus = "DecresingATKRATING:\r"; 
char *MaxDamagePlus2 = "MaxDamagePlus:\r"; 
char *LightningAddDamage3 = "LightningAddDamage:\r"; 
char *ReLifePercent4    = "ReLifePercent:\r"; 
char *GetExp3    = "GetExp:\r"; 
char *ResurrectionChar4    = "ResurrectionCharLV"; 
char *ExtinctionPercent2    = "ExtinctionPercen:\r"; 
char *ExtinctionAmount2    = "ExtinctionAmount:\r"; 
char *IncreLifePercent2   = "IncreLifePercent:\r"; 

char *ReduceDamage3 = "ReduceDamage:\r"; 
char *IncreMagicDamage = "IncreMagicDamage:\r"; 
char *AttackDelay3   = "AttackDelay:\r"; 

char *FireDamage2 = "FireDamage:\r";
char *IceDamage2 = "IceDamage:\r";
char *AddDefense8 = "AddDefense:\r";
char *SkillDamage5 = "SkillDamage:\r";

char *FireDamage3 = "FireDamage:\r";
char *AddSpeed7 = "AddSpeed:\r";
char *AfterDamage7 = "AfterDamage:\r";
char *MonsterSight7 = "MonsterSight:\r";


char *LinghtingAddDamage4="Add attack rate on lightening:\r";
char *MaxBoltNum4="Max voltage:\r";
char *AddAbsorb4="Add rate on absorb:\r";
char *IncreArea4="Increasing range:\r";
char *IncreAttack4="More attacking power:\r";
char *GolemLife4="Life+STR:\r";


char *SubAbsorb4="Decreasing absorption:\r";
char *AreaDamage4="Attacking rate for range:\r";
char *MaxAttackNum4="Max hits:\r";
char *LifeUp4="Add vitality:\r";

char *CriticalAddDamage4="Increase of critical:\r";
char *MonstervsSubCritical4="Weaken critical of monster:\r";
char *ChargingAddPercentDamage4="Add attacking rate on charging:\r";

char *IncreEvasionPercent4="Additinal percentage of evasion:\r";
char *AddShadowNum4="Increasing combination of ocular:\r";

char *WolverinLife4="Vitality:\r";
char *WolverinRate4="Accuracy:\r";
char *WolverinDefense4="Defence rate:\r";
char *AddEvasion4="Increasing ability of evasion:\r";
char *AddDamage4="Increasing attacking rate(physic):\r";
char *FalconAddDamage4="Increasing attacking rate of falcon:\r";


char *IncreAttackAbsorb4="Increasing absorption rate:\r";

char *RectAngleArea4="Attacking at sharp angled range:\r"; 

char *MonsterSubSpeed4="Decreasing speed of attacked monster:\r";


char *AddLifeRegen4="Add life-regeneration:\r";
char *AddManaRegen4="Add mana-regeneration:\r";
char *MagicArea4="Magic affected range:\r";
char *ChainNum4="A number of affected monster:\r";
char *ChainRange4="The rage of affected monster:\r";
char *UndeadDamageAbsorb4="Undead attacking absorption:\r";
char *UserBlockPercent4="Blocking rate for character:\r";

char *SecondIncreMana4="Increasing mana rate per second:\r";
char *FireDamage4="Fire attacking rate:\r";
char *DecreSpeed4="Decreasing Speed:\r";
char *DecreAttack4="Decreasing attacking rate:\r";
char *AddPercentDamage4="Adding attacking rate:\r";



char *ChainDamage3    = "ChainDamage)\r";
char *BrandishDamage3 = "BrandishDamage";
char *PhysicalAbsorb3 = "PhysicalAbsorb";
char *SparkDamage3    = "SparkDamage";
char *BrutalSwingCritical3   = "BrutalSwingCritical";
char *Attck_RateAdd5 = "Attck_RateAdd:\r";
char *Triumph4="Triumph";
char *Use4="you";
char *DivineShield4="Blocking rate of divine shiled";


char *QuestMonsterName[] = {"Tha�ch nh�n ","Ta�n binh c��t ","Tra�m thu� qua�i ","Bo� gia�p ","C��t xa� thu� ","Tha�ch tinh " ,
                         "Qua�i v��t ta�i Vu�ng ���t bi� la�ng qu�n ","Qua�i v��t ta�i ��c �a�o ","Qua�i v��t ta�i Nha� ngu�c c��, t��ng 1  "};




char *VampOption[10] ={"IncreaseAging%: \r","Vampire: \r","RefDamage: \r","DecreaseHP: \r","Lucky: \r "};


char *Quest3ItemName[]={"Demon Hunter","Ancient Revenge","Mist","Devil Side","Anaconda","Plated Sword","Ghost "};

char *Quset3ItemDoc15 = "Mo�n ��� nhi��m vu� ";
char *Quset3ItemDoc16 = " �a� bi��n m��t";

char *SparkDamage10 = "SparkDamage:\r";

char *RequirLevel3 = "(RequirLevel:%d)\r";

char *RecvItemTT = "Nh��n ����c mo�n ��� ";
char *PuzzleEvent5 = "Completed Puzzle";
char *BabelHorn = "The horn of Babel";
char *NineTailFoxItem = "Ha�i l��c ���u xu�n";
char *MyShopItemSell5 = "The Price: \r";
char *CristalItem     ="the 7 Crystals";
char *StarDust	= "B��t tinh tu� ";
char *ChristMas = "Merry ChristMas";

char *sinDeadCandyEat = "��c! Ca�i ke�o na�y bi� ho�ng r��i -_-";

char *PotionCntDoc2 = "Ba�n Potions";
char *ItemPrice7 = "Ba�n mo�n ��� ";

char *BuyMyShopItem7 = "%s �a� mua %d %s b��ng %d va�ng";
char *BuyMyShopItem8 = "%s �a� ba�n %d %s v��i gia� %d va�ng";
char *ItemName87 = "item";

char *sinEvent87 = "Request of the danger collector";
char *sinAddDamageItem = "AddedATKPOWER: \r";
char *sinContinueTimeItem = "Duration: \r";
char *BuyPotionMoney = "Gold:";
char *BuyPotionKind = "S�� l���ng:";
char *SheltomName2[] = {"Lucidy","Sereno"," Fadeo","Sparky","Raident","Transparo","Murky","Devine","Celesto ","Mirage","Inferna","Enigma"};

char *AddPercentDamage3= "AddATKPOWER%: \r";

char *sinLuckyBox = "Lucky Bag";
char *SodMessage_Etc[]= {
			"Thu nh��p cu�a Clan qua�n ly� Bellatra la� ",
			"Chi� co� chu� Clan m��i ����c nh��n.",
			"Ti� l�� thu�� hi��n ha�nh la� %d%s.",
			};
			
char *SodMessage_Clan[] = {
			"T��ng thu nh��p ",
			"Bellatra cung c��p ti��n la�i nga�y h�m qua.",
			};
			
char *SodMessage_Master[] = {
			"T��ng thu nh��p",
			"Bellatra cung c��p ti��n la�i nga�y h�m qua.",
			"Ba�n co� mu��n l��y ra kh�ng?",
			};
			
char *OtherClanMaster[] = {
			"Clan v��n co�n co� th�� nh��n ti��n la�i",
			"nh��p va�o s�� ti��n c��n l��y ra.",
			};
	
char *SiegeMessage_Taxrates[] = {
	"Ti� l�� thu�� �a� thay ���i %d%s.",
	"T��ng s�� thu�� �a� ti�ch luy� ",
	"Gold.",
	"Ba�n co� mu��n ru�t ti��n thu�� ra kh�ng?.",
	"Ti� l�� thu�� hi��n ta�i la� %d%s.",
};
char *SiegeMessage_MercenrayA[] = {
	"D�n qu�n",
	"La� l��c l���ng d�n qu�n",
	"S��c chi��n ���u kh�ng cao nh�ng gia� tha�nh re�.",
	"T��ng s�� d�n qu�n: 20 / %d",
};
char *SiegeMessage_MercenrayB[] = {
	"V�� binh",
	"Gia� ca� phu� h��p v��i kha� n�ng chi��n ���u.",
	"Co� s��c chi��n ���u trung bi�nh.",
	"T��ng s�� V�� binh: 20 / %d",
};
char *SiegeMessage_MercenrayC[] = {
	"C��m v�� qu�n",
	"Co� kha� n�ng chi��n ���u cao nh��t",
	"S��c chi��n ���u va� pho�ng v�� r��t du�ng ma�nh.",
	"T��ng s�� C��m v�� qu�n: 20 / %d",
};
char *SiegeMessage_TowerIce[] = {
	"Toa� tha�p B�ng",
	"Ti�nh b�ng la�m gia�m t��c ��� di chuy��n va� ",
	"co� kha� n�ng ���c bi��t la�m gia�m t��c ��� �a�nh.",
	"Thi��t l��p Toa� tha�p B�ng �a� hoa�n t��t",
};
char *SiegeMessage_TowerFire[] = {
	"Toa� tha�p L��a",
	"Co� kha� n�ng t��n c�ng b��ng l��a r��t ma�nh",
	"Ngoa�i ra kh�ng co� kha� n�ng ���c bi��t.",
	"Thi��t l��p Toa� tha�p L��a �a� hoa�n t��t",
};
char *SiegeMessage_TowerLighting[] = {
	"Toa� tha�p �i��n",
	"La�m gia�m t��c ��� di chuy��n va� ",
	"co� kha� n�ng ���c bi��t la�m gia�m t��c ��� �a�nh.",
	"Thi��t l��p Toa� tha�p �i��n �a� hoa�n t��t",
};
char *SiegeMessage_MerMoney   = "Gia� cu�a li�nh �a�nh thu�: ";
char *SiegeMessage_TowerMoney = "Gia� cu�a Toa� tha�p: ";

char *sinClanMaster7 = "Chu� Clan";
char *sinPrize7 = "Ph��n th���ng : ";


char *sinLevelQuestMonster[]={"Tha�ch nh�n ","N��m quy� ","Ong ���c ","�a�i tinh tinh ","Xi�ch ma v��ng ","Thi��t ma v��ng ","Thi��t thu� v��ng  "};
char *sinLevelQusetDoc="Nhi��m vu�> �a� ti�u di��t ����c %s %d";

char *sinTeleportDoc[] = {"Hang N��m ","Hang Tha�ch Nhu� ","Vu�ng ���t bi� nguy��n ru�a ","Vu�ng ���t c��m  "};
char *sinLevelQuest90_2Mon[] = {"�i�a ��n v��ng ","Thi��t khu�ng long ","Song chuy� quy�  "};
char *sinLevelQusetDoc2="<Nhi��m vu�> Ha�y ti�u di��t %s %d";
char *sinLevelQusetDoc3="<Nhi��m vu�> Ti�m mo�n ��� t�� %s";
char *sinLimitTiem2 = "Th��i gian gi��i ha�n: \r";
char *sinMinute2 = "phu�t";
char *sinDay4 = "day";
char *sinHour4 = "hour";
char *sinLevelQusetDoc4="<Nhi��m vu�> �a� ti��u di��t ����c %s %d";

sinSTRING sinTestMsg7("Hurray, Neh Neh Neh Boo Boo, You're losers!");

char *sinMonCodeName[]={
	0,
	"Tha�ch nh�n ",
	"Ta�n binh c��t ",
	"Tra�m thu� qua�i ",
	"Bo� gia�p ",
	"C��t xa� thu� ",     
	"Tha�ch tinh ",
	"N��m quy� ",
	"Ong ���c ",
	"�a�i tinh tinh ",
	"Xi�ch ma v��ng ",                
	"Thi��t ma v��ng ",
	"Thi��t thu� v��ng ",
	"�i�a ��n v��ng ",
	"Thi��t khu�ng long ",
	"Song chuy� quy� ",              
	"Thi��t binh c��t ",
	"S�n h�� ",
	"Thi��t ca�ng ",
	"Xa�c ���p ",
	"Qua�n ngu�c ",             
	"Hoa� nh�n",            
	"Tha�ch v��ng ",
	"Tha�ch th��n ",
	"V�� gia�p binh ",
	"Nh�n ma� v��ng ",       
	"Nh�n ma� th��ng ",
	"Ong chu�a ",
	"Ky� si� a�o ",
	"A�c ma ",
	"Phu� thu�y ",                
	"�a�i qua�i ",
	"Song ���u qua�i ",
	"T�� th��n ",
	"Xu� di��n ma ",
	"Xi�ch quy� ",           
	"H��c ma v��ng ",
	"Thi��t h��u ",
	"Nh�n ma� chu�y ",
	"H��ng sa�t thu� ",
	"S��u quy� ",            
	0,
};

char *ChargingTime4="Raise speed for charging:\r";
char *AddIceDamage4="Added Damage of Frost Elemental:\r";
char *IceTime4="Freezed Time:\r";
char *sinAttackType = "Attack Type:\r";
char *sinAttackType2[2] ={"Manual\r","Auto\r "};

char *Won="Gold";


char *CastlItemInfo[] ={
	{"B��t t�� trong m��t khoa�ng th��i gian\r "},  
	{"Ti� l�� �a�nh Critical\r "},
	{"T�ng ti� l�� ne� �o�n\r "},
	{"STM,HP,MP cho tha�nh vi�n party\r "},
	{"H��i sinh t��c thi�.\r "},
	{"S��c t��n c�ng cu�a toa� tha�p\r "},     
	{"Khi t��n c�ng mu�c ti�u, s��c �a�nh t�ng th�m 20%\r "},
	0,
};
char *CastlItemInfo2[] ={
	{"S��c �a�nh gia�m �i 1/2.\r "},
	{"T�ng th�m 5%.\r "},
	{"t�ng th�m.\r "},
	{"phu�c h��i ���y �u�.\r "},
	{"\r "},
	{"gia�m �i 50%\r "},
	{"C��ng th�m.\r "},
	0,
};


char *CharClassTarget[8]={
	"Mu�c ti�u: Fighter\r",
	"Mu�c ti�u: Mechanician\r",
	"Mu�c ti�u: Pikeman\r",
	"Mu�c ti�u: Archer\r",
	"Mu�c ti�u: Knight\r",
	"Mu�c ti�u: Atalanta\r",
	"Mu�c ti�u: Magician\r",
	"Mu�c ti�u: Priestess\r",
};
char *AttributeTower[4]={
	"Thu��c ti�nh L��a\r",
	"Thu��c ti�nh B�ng\r",
	"Thu��c ti�nh �i��n\r",
	"Th��i gian:%d gi�y\r",
};

char *SiegeMessage_MerComplete ="Thi��t l��p hoa�n t��t";


char *haQuestMonsterName[]={
	"S��ng gai",
	"Tho� r��ng",
	"���c tinh",
	"H��c tinh",
	"Kh� c��t",
	"Hu� c��t",
	"���c nha�n ",
	"Tha�ch nh�n ",
	0,
};
char *ha100LVQuestItemName[]={
	"Minotaur Axe",
	"Extreme Talon",
	"Dragon Bone Hammer ",
	"Hellfire Scythe",
	"Revenge Bow",
	"Immortal Sword",
	"Salamander Javelin",
	"Gothic Staff",
	0,
};

char *ha100LVQuestMonterName[]={
	"Qua�i v��t ta�i Nha� ma�y Perum",
	"Qua�i v��t ta�i Thung lu�ng Gallubia",
	0,
};
char *ha100LVQuestName = "A�o t���ng Fury";


char *PremiumItemDoc[][2]={
	{"�i��m Stats\r"     ,"se� ����c thi��t l��p la�i.\r "},     
	{"�i��m Skill\r"     ,"se� ����c thi��t l��p la�i.\r "},    
	{"�i��m Stats/Skill\r","se� ����c thi��t l��p la�i.\r "},     
	{"H��i sinh t��c thi� \r"    ,"khi bi� ch��t\r "},        
	{"Tr�� n�n b��t t��.\r","S��c t��n c�ng gia�m �i 1/2\r "},    
	{"T�ng th�m 15% \r"    ,"ti� l�� ra �o�n chi� ma�ng.\r "},		
	{"T�ng th�m 15% \r"         ,"kha� n�ng ne� �o�n.\r "},		
};

char *UpKeepItemDoc[] = {
	"Co�n la�i : %d phu�t",
	"Ti� l�� r�i ��� t�ng th�m %d%s",
	"Ti� l�� XP t�ng th�m %d%s",
	"T�ng th�m s��c t��n c�ng %d%s",
	"H��p thu ma�u cu�a ���i thu� ",
	"H��p thu Mana cu�a ���i thu� ",
	"T�ng c�n n��ng th�m d%",
	"Gia�m MP khi du�ng ky� n�ng %d%s",
	0,
};
char *UpKeepItemName[] = {
	"Th��n nha�n",
	"T�ng XP",
	"Nanh h��p huy��t ",
	"Bi�nh h��p l��c ",
	"Bi�nh t�ng l��c",
	"Bi�nh tr�� l��c",
	"Red Pixie",
	0,
};


