 	

#include "core/server/legacy_sin_runtime/sinLinkHeader.h"  

sMESSAGEBOX	sMessageBox[100] = {
	{  "  ��鹷�������� "  },
	{  "  ���˹ѡ��?"  },
	{  "  �ѧ�������ö�����������  "  },
	{  "  �Թ�����§��  "  },
	{  "  ���������������?mix �? "  },
	{  "  �ӹǹ�ͧ����š����¹�Թ�մ�ӡ�? "  },
	{  "  ����š����¹�١¡��ԡ  "  },
	{  "  �������ö��Potion �? "  },
	{  "  ��?mix ������?. "  },
	{  "  �ͺ�س����Ѻ ������سҢͧ��ҹ!  " },
	{  "  ����㹤�ѧ�Թ������ҡ�Թ� "  },
	{  "  �����������ö age �? "  },
	{  "  �����١����¹��سҵ�Ǩ�ͺ�ա����? "  },
	{  "  �ӹǹ�Թ�Թ�մ�ӡ�? "  },
	{  "  ��?aging ������?"  },
	{  "  ʶԵԵ�ҧ�ͧ����Фö١ź���? "  },
	{  "  ������Ҿ  "  },
	{  "  ��ҹ����öźʶԵԵ�ҧ���������? " },
	{  "  ��ͧ����Ҫԡ clan ��ҹ�鹨֧������ö���? "  },
	{  "  ��ҹ��ͧ����� clan �֧������ö����������? "  },
	{  "  �дѺ�����ͧ��ҹ�ѧ�������ö�Ѿ�ô�? "  },
	{  "  �дѺ��� aging �ͧ��ҹ������ -1  "  },
	{  "  �дѺ��� aging �ͧ��ҹ������ -2  "  },
	{  "  �дѺ��� aging �ͧ��ҹ������ +1  "  },
	{  "  �дѺ��� aging �ͧ��ҹ������ +2  " },
	{ "  ��?atk speed ��?quest item Ŵŧ  " },
	{ " ������?Level Up " },
	{ " ��?HP �������?15  " },
	{ "  puzzle ����ó����Ǥ��?" },
	{ "  puzzle �ѧ�������ó���? " },
	{ "  ������������Т�� " },
	{ "  ������١��������  " },
	{ " ��ҹ�����١�Դ�����  " },
	{ " �������ö�Ҥ�����?" },
	{ " �ӹǹ���١���?" },
	{ "  ���ä���������������ö���?" },
	{ "  ���ä�������?��������ö���� " },
	{ " ���ä�������?��������ö�����������  " },
	{ " ���ä���������������ö����������? " },
	{ " �ҤҢͧ�������ա������¹��? " },
	{ " ������Թ�ҧ��������?clan ����?" },
	{ " ����Ź���������ö�� force orb �? " },
	{ " force orb �١�����袳й��  " },
	{ "  ���ѧ���ͺ�к� atk pow " },
	{ " ����кǹ��?force orb " },
	{ " stat point �������?5 " },
	{ " skill point �������?1 " },
	{ " stat ��?skill point �������?5 ��?1  " },
	{ " stat ��?skill point �������? ��?2  " },
	{ "  ���� exp �ҡ��õ��Ŵ�?1% " },
	{ " HP �������?40  "  },
	{ " ��ҹ���������ҧ������ "  },
	{ " ������дѺ 80 ���� ��Ҥ�������ö�������� +7 �ء�����? "  },
	{ " ������дѺ 90 ���� ��Ҥ�������ö�������� +10 �ء�����? "  },
	{ " ��ҹǹ�͹�����������繤�ҵ�駵�? "  },
	{ " ����ҹ��÷��ͺ���ͧ�ҡ������ҷ����˹�  "  },
	{ " �ӹǹ�����Ѻ��ҧ�Թ����  " },
	{ " ���������ʡ�����������ѧ������㹻Ѩ�غѹ  " },
	{ " ����� bless castle ����� (��سҵ�Ǩ�ͺ client version)  " },
	{ " ��������������ö������ҹ��ǹ�����  " },
	{ " ������Թ���շ����Ѻ��㹢�й��  " },
	{ " ��ҹ��ͺẺ�ͺ�������? " },
	{ " �ͺ�س�����µͺẺ�ͺ��? " },
	{ " ���������������ҡѺ�����  " },
	{ " ��������������? " },
	{ "  ���Ѻ��ª��㹡�õ�����Ѻ Fury  " },
	{ "  ������㹡�õ������?Fury. " },
	{ "  ���Ѻ�������ҧ��? " },
	{ "  �����������ö�������ѹ�? " },
};


sMESSAGEBOX2 sMessageBox2[20] = {
	{  "  ��ҹ��ͧ��ë���������? "  ,  "  ����ѹ?  " },
	{  "  ��ҹ��ͧ��â���������  "  ,  "  ����ѹ?  "  },
	{  "  ��ҹ��ͧ��÷��������? "  ,  "  ����ѹ?  " },
	{  "  ��ҹ��ͧ��÷����? "  ,  "  �ӹǹ����? "  },
	{  " "  ,  " "  },
	{  " ��ҹ��ͧ����͹�Թ  "  ,  "  �ӹǹ����? "  },
	{  " ��ҹ��ͧ�������? "  ,  "  �ӹǹ����? "  },
	{  " ��ҹ��ͧ��ö͹�Թ  "  ,  "  �ӹǹ����? "  },
	{ " �Ы���  " , " ��ǡ���?  "  },
	{ " �к�ԨҤ " , "  �Թ�����?  "  }, 
	{ " ���к�  " , "  �Թ�������?  "  }, 
	{ " �Ы������? " , "  ����ѹ?  " },
	{ " ���Ѻ�Թ�ҧ��? " , "  �����?  " },
	{ " �Ы������? " , " ������?  " }, 
	{ " �Ы������? " , " ������?  " }, 
	{ " ���ԡ  " , "  �Թ�����  " },
	{ " �Ы���  " , " �����Ѻ��ҧ��褹?  "  },
};

sMESSAGEBOX3 sMessageBox3[MESSAGE3_MAX] = {
	{  "  ��ҹ��  "  ,  "  ���¹ʡ�Ź�� ?  "  },
	{  "  ��ҹ��ͧ��? "  ,  "  ����͹�д�? "  }, 
	{  "  ��ҹ��  "  ,  "  �Ѻ���СԨ��?  "  },
	{  "  ��ҹ��ͧ��? "  ,  "  ź���ʡ�ŷ�����?  "  },
	{  "  ��͹�������¹  "  ,  "  ��ҹ��ͧ�����Թ��͹!  "  },
	{  "  ��ҹ��ͧ��? "  ,  "  ź���ʶԵԷ����? "  },
	{  "  ��ҹ��  "  ,  "  �Ѻ���СԨ��??  "  }, 
	{  "  ��ҹ��  "  ,  "  �Ѻ������鹹�?  "  }, 
	{  "  ��ҹ��ŧ�����?  "  , " Union Core 㹪��ͷ�ҹ? " },
	{ " ��ҹ�� " , "  �Ѻ������? " },
	{ " ��ҹ��ͧ��è?" , "  �һ价����? " },
	{ " "  , "  ��س����ӹ�?" },
	{ "  ��ҹ��ͧ��ë��?" , "  Star? " },
	{ "  ��ҹ��ͧ��â�� " , "  �������? " },
	{  "  ��ҹ����?"  , "  ��ʹ�? "  }, 
	{  " ��ҹ�Ш��� "  , "  ��Һ�ԨҤ�������? "  }, 
	{  "  ��سҡ�˹�?ͤ��?"  , "  ��С������Ѻ��ҹ���¤�Ѻ "  }, 
	{  "  ��ҹ���š����¹ "  , "  �����������? "  }, 
	{  "  ��ҹ�Ы��� "  , "  �������? "  }, 
	{  "  ��ҹ��¡��ԡ "  , "  ���ŧ����¹? "  }, 
	{  "  ��ҹ��ͧ��è?"  , "  ����¹��������������? "  }, 
	{  "  ��ҹ��ͧ��?"  , "  ����¹����? "  },
	{  "  ��ҹ����?"  , "  �ͧ������������? "  }, 
	{  "  ��ҹ����?"  , "  �ͧ������������? "  }, 
	{  "  ��ҹ����?"  , "  �ͧ������������? "  }, 
	{  "  ��ҹ����?"  , "  �ͧ������������? "  }, 
	{  "  ��ҹ����?"  , "  �ͧ������������? "  }, 
	{  "  ��ҹ����?"  , "  �ͧ������������? "  }, 
	{  "  ��ҹ����?"  , "  �ͧ������������? "  }, 
	{  "  ��ž����?"  , "  ������? "  },
	{  "  ���Ѻ��?"  , "  ������? "  },  
	{  "  ���Ѻ��?"  , "  ������?"  },  
	{  "  ���Ѻ��?"  , "  ������?"  },  
	{  "  ������¹�Ҫվ  "  , "  ������? "  }, 
	{  "  ���š��?"  , "  �����������? "  },
	{  "  ��ž����?"  , "  ������?"  },
	{  "  �Ы��� "  ,  "  ���ʵ�ŷ������?"  }, 
	{  "  ��ҹ����?"  , "  �Ѻ��? "  }, 
	{  "  ��ҹ����?"  , "  �Ѻ��? "  }, 
	{  "  ��ҹ����?"  , "  �Ѻ��? "  }, 
	{  "  ��ҹ����?"  , "  �Ѻ��? "  }, 
	{  "  ��ҹ����?"  , "  �Ѻ��? "  }, 
	{  "  ��ҹ����?"  , "  �Ѻ��? "  }, 
	{  "  ��ҹ����?"  , "  �Ѻ��? "  }, 
	{  "  ��ҹ����?"  , "  ������? "  }, 
	{  "  ��ҹ����?"  , "  �Ѻ��? "  },
	{  "  ��ҹ��ͧ��?"  , "  �Ѻ����? "  }, 
	{  "  ��ҹ��ͧ��?"  ,  "  ����价�?Battle Zone? "  }, 
	{"�� �ڷ���Ʈ", "�Ͻðڽ��ϱ�?"},   
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "}, 
	{"�ڷ���Ʈ", "�Ͻðڽ��ϱ�?"},    
	{"��������", "���� �Ͻðڽ��ϱ�?"}, 
	{"�ڷ���Ʈ", "�Ͻðڽ��ϱ�?"},    
};

char *SkillMaster[4] = {
	 "  ���¹ʡ�?" ,
	 "  ����¹���?" ,
	 "  �͡�ҡ�?" 
};

char *SkillMaster2[4] = {
	 " "  ,
	 " "  ,
	 " "  

};

char *ReStartMsg[4] = {
	 "  ������ҡ Field " ,
	 "  ������ҡ����?" ,
	 "  �͡�ҡ�?" ,
	 "  XP �����§�� " 

};
char *ReStartMsg2[4] = {
	 "  ��һ�ѺGold/XP 1% " ,
	 " " ,
	 " " ,
	 "  ��Ҩ��������?Field " ,

};

char *SkillUseInfoText[10]= {  "  ���¹���ѡ�?" , "  ��س����͡�ѡ�?" , "  �������¹ " , "  Skill Point ���?" , "  �������¹ "  };
char *sinMsgSkillPoint =  "  Skill Points : %d " ;
char *sinMsgEliteSkillPoint = "  Elite Skill Points : %d " ;
char *sinStateName[5]={ "  H P : %d/%d " , "  M P : %d/%d " , "  STM : %d/%d " , "  X P : %d/%d " , "  X P : %dM/%dM " };


char *TownName[3] = { "  ���¡��Ѻ���ͧ Ricarten/r " , "  ���¡��Ѻ���ͧ Navisko/r " , "  ���¡��Ѻ���ͧ Pillai/r " };


char *sinAbilityName[50] = {  "  ATK Power:  /r "  ,  "  Weapon Speed:  /r "  ,  "  Range:  /r "  ,  "  Critical:  /r "  ,  "  Defense Rating:  /r "  ,  "  ATK Rating:  /r "  ,
			  "  Absorb :  /r "  ,  "  Block :  /r "  ,  "  Speed:  /r "  ,  "  Integrity:  /r "  ,  "  ��� MP :  /r "  ,  "  ��� HP :  /r "  ,
			  "  ��� STM :  /r "  ,  "  Organic Type:  /r "  ,  "  Nature Type:  /r "  ,  "  Flame Type:  /r "  ,  "  Frost Type:  /r "  ,  "  Lightning Type:  /r "  ,  "  Poison Type: /r "  ,
			  "  Water Type:  /r "  ,  "  Wind Type:  /r "  ,  "  HP Regen:  /r "  ,  "  MP Regen: /r  "  ,  "  STM Regen: /r "  ,  "  ����HP: /r "  ,
			  "  ����MP:  /r "  ,  "  ����STM:  /r "  ,  "  �� Potion �? /r "  ,  "  ��ͧ���Level:  /r  "  ,  "  ��ͧ���Strength:  /r "  ,  "  ��ͧ���Willpower:  /r "  ,
			  "  ��ͧ���Talent:  /r "  ,  "  ��ͧ���Agility:  /r "  ,  "  ��ͧ���Health:  /r "  };
			
char *sinSpecialName[50] = { "  Spec. ATK SPD:  /r  " , "  Spec. CRIT:  /r  " , "  Spec. DEF RTG:  /r  " , "  Spec. ABS RTG:  /r  " ,
			 "  Spec. BLK RTG:  /r  " , " Magic APT:  /r " , " Spec. SPD:  /r  " , "  Spec. Organic:  /r  " , "  Spec. Nature:  /r  " ,
			 "  Spec. Flame:  /r  " , "  Spec. Frost:  /r  " , "  Spec. Lightning:  /r  " , "  Spec. Poison:  /r " , "  Spec. Water:  /r  " , "  Spec. Wind:  /r  " ,
			 "  Spec. ATK POW:  /r  " ,  "  Spec. ATK RTG:  /r  " , "  Spec. RNG:  /r  " , "  Spec. Organic ATK:  /r  " , "  Spec. Nature ATK:  /r  " , "  Spec. Flame ATK:  /r  " , "  Spec. Frost ATK:  /r  " ,
			 "  Spec. Lightning ATK:  /r  " , "  Spec. Poison ATK:  /r  " , "  Spec. Water ATK:  /r  " , "  Spec. Wind ATK:  /r  " , "  Max HP Boost:  /r  " , "  Max MP Boost:  /r  " ,
			 "  ��� HP:  /r  " , "  ��� MP:  /r  " , "  ��� STM:  /r  " , " �ҤҢ�? /r " , " �Ҥҫ���:  /r " };


char *SpecialName3 =  "  %s Spec. /r " ;
char *MixResultTitle = "  �Ţͧ��� mix���?"  ;
char *NowLevelText = "  ����ŻѨ�غ�?/r "  ;
char *NextLevelText = "  ����ŶѴ� /r "  ;

char *NormalAttckName = "  Default Attack " ;
char *RequirLevel =  " %s (��ͧ���LVL : %d)/r " ;
char *UseItemGroupName =  " �����������ö�������Ѻ�ѡ�й���?/r " ;

char *PoisonName = "  Poison Type:/r "  ;
char *FireName = "  Flame Type:/r "  ;
char *AttackAreaName = "  �������?/r "  ;
char *Attack_RateName = "  ��������� /r "  ;
char *Max_DamageIncre = "  ��ѧ�����٧��?/r "  ;
char *DamageAddName = "  ������ѧ����: /r "  ;
char *Attack_SpeedAdd = "  ����������������: /r "  ;
char *IncreBlock = "  �����ѵ�ҡ�û�ͧ�ѹ: /r "  ;
char *CountinueTime = "  ��������: /r "  ;
char *UseManaName = "  �?MP: /r "  ;
char *DamageName = "  ��ѧ���? /r "  ;
char *ShootingRangeName = "  ����: /r "  ;
char *IncreAsorb = "  Ŵ�ѵ�Ҥ���������¨ҡ������?/r "  ;
char *SecName = " �� "  ;
char *PlusDamageName = "  ������ѧ���� /r "  ;
char *HitNumName = "  �ӹǹ���� : /r "  ;
char *DecreLifeName = "  HP Ŵ : /r "  ;
char *Attck_RateAdd = "  ������������������� : /r  "  ;
char *Add_CriticalName = "  ���?Critical : /r "  ;
char *Push_AreaName = "  Push-back Length : /r "  ;
char *FireAttackDamageAdd =  "  Flame ATK : /r "  ;
char *IceName = "  Frost Property : /r "  ;
char *IceAttackDamageAdd = "  Frost ATK : /r "  ;
char *StuneRate = "  % Stun : /r "  ;
char *DefenseRateIncre = "  ��������ѵ�ҡ�û�ͧ��?/r "  ;
char *WeaponSizeName =  "  �������?/r "  ;
char *HwakRotationNum = "  Hawk Rotations:/r "  ;
char *WeaponSpeedAddName  = "  ���������������ظ:/r "  ;
char *IncreAttack_RateName = "  ������ѧ����:/r "  ;
char *ShootingNumName = "  �ӹ�?Shots:/r "  ;
char *SeriesShootingCount = "  �ԧ������ͧ :/r "  ;
char *UseStaminaName = "  �?Stamina :/r "  ;
char *MasterSkillMoneyName = "  �Ҥҷѡ�?/r "  ;

char *ItemAgingResult;		

char *sinGold = " %d Gold " ;
char *sinGiveItem5 =  "  Gift Item/r " ;
char *sinCopyItem5 =  "  Copied Item/r " ;
char *sinNum7 =  " ��  " ;
char *sinSkillPointName =  "  Skill Point " ;	
char *sinG_Pike_Time3 =  "  Frozen Time:/r " ;	


char *sinItemLimitTimeOverMsg = "  ����������Ǥ�� /r "  ;
char *sinDeadSongPyeunEat = "  ���������ث�����!  "  ;
char *SmeltingResultTitle = "������ ���� ���";		
char *ManufactureResultTitle = "������ ���� ���";	


char *sinMaxDamageAdd = "  ������ѧ�����٧�ش + /r "  ;
char *sinDamagePiercing = "  (Piercing attack)/r "  ;
char *sinAddStamina = "  ����stamina: /r "  ;
char *sinAddLife = "  ��� HP: /r "  ;

char *sinDamageUndead50 = "  ��������������� 50% �?Undead :/r "  ;
char *sinNumCount2 = "  number:/r "  ;
char *sinNumCount3 = " ��?/r "  ;
char *sinDecreDamage = "  Ŵ damage:/r "  ;
char *sinConvert4 = "  conversion rate:/r "  ;
char *sinIncreElement = "  ���?Element feature:/r "  ;
char *sinPartyIncreElement = "  ��ѧ�ҵآͧ��Ҫԡ㹻������������?50% /r "  ;
char *sinFireBallDamage2 = "  (100% damage to the targeted enemy)/r "  ;
char *sinFireBallDamage3 = "  (targeted area 60 (1/3 damage))/r "  ;
char *sinAddDamage7 = "  ����damage : /r "  ;
char *PlusDamageWeapon = "  ����?"  ;
char *SparkDamage = "  ���յ����?/r "  ;
char *SparkNum7 = "  �ӹǹspark max:/r "  ;
char *StunRage = "  ���� Stun : /r "  ;
char *RecvPotion7 =  "  ���Ǵ " ;


char *PartyArea7 =  "  ���� Party:/r "  ;
char *Area17 =  "  ����:/r "  ;
char *AddAttack_Rate9 =  "  ��������� /r "  ;
char *PiercingRange =  "  ���� Piercing: /r "  ;
char *IncreDefense = "  ������һ�ͧ��?/r "  ;
char *AddShootingRange = "  �������С����?/r "  ;
char *LifeAbsorb = "  Life absorb.: /r "  ;
char *FireDamage = "  Fire Damage: /r "  ;
char *IceDamage = "  Ice Damage: /r "  ;
char *LightningDamage = "  Lighting Damage: /r "  ;
char *AddManaRegen = "  �����ѵ�ҡ�ÿ�鹤׹ Mana: /r "  ;
char *LightNum = "  # of Lighting: /r "  ;
char *ReturnDamage = "  �з�͹��ѧ���? /r "  ;
char *Area18 = "  ���Ф�ͺ����: /r "  ;
char *IncreMana5 = "  ����Mana: /r "  ;
char *Area19 = "  ���Ф�ͺ����: /r "  ;
char *HelpTitle8 = "  ��������?"  ;
char *QuestTitle8 = "  ��� "  ;
char *ResearchTitle8 = "  Ẻ�ͺ��?"  ;
char *TeleportTitle8 = "  TELEPORT "  ;

char *IncreWeaponAttack_RateName = "  ��������������: /r "  ;
char *AddMaxDamage = "  ������ѧ�����٧�ش: /r "  ;
char *LinkCoreName =  "  ������ѧ��Ҫԡ��Ź  /r " ;
char *DesLinkCore =  "  ����: " ;
char *Itemul =  "  ���?" ;
char *ItemRecvOk =  "  ��ҹ��ͧ����Ѻ? " ;
char *Money5 =  "  Gold " ;
char *Exp5 =  "  ��һ��ʺ��ó?" ;
char *WingItemName[] = { "  Metal Wing  " , "  Silver Wing  " , "  Gold Wing  " , "  Diamond Wing  "  , "  Chaos Wing  " };
char *WarpGateName[] = { "  Warp Gate  " , "  Ricarten  " , "  Pillai Town  " , "  Land of Dusk  " , "  Bamboo Forest  " , "  Ruinen Village  " , "  Navisko town  " , "  Forbidden Land  " ,  "  Eura Village  "  };
char *sinWarningName5 =  "  Warning!!! " ;
char *ClanCristalName =  "  �Ź���ʵ�?/r " ;
char *NowMyShopSell =  "  ���ѧ������� /r " ;
char *SecretNumName =  "  ��س���������Ţ���?" ;
char *MyShopExpDoc7 =  "  ��ҹ��?" ;
char *NotAgingItem2 =  "  ���������������?age �?" ;
char *ExpPercent2 =  "  ��һ��ʺ��ó?%d.%d%s " ;
char *ExpPercent3 = "  ��һ��ʺ��ó?%d.0%d%s "  ;
char *Mutant7 =  "  Mutant " ;
char *Mechanic7 =  "  Mechanic " ;
char *Demon7  =  "  Demon " ;
char *Nomal7  =  "  Normal  " ;
char *Undead7  = "  Undead "  ;
char *MonsterAddDamage2 =  "  ���?(Damage)/r " ;
char *MonsterAddDamageFire =  "  ���?Fire Damage/r " ;


char *HoldStarNumDoc = "  �ӹ�?Star ��������?"  ;
char *StarItemBuy7 = "  ���� Star ���?"  ;
char *ChangeMoney7 = "  (Star 1 ��?�Ҥ� : 100000 Gold) "  ; 
char *HaWarpGateName[] = {  "  Bless Castle  "  ,};
char *FallGameName = "�ϱ׷��� ��";



char *LightningDamage2 = "  Lightning Damage:/r "  ;
char *SheildDefense = "  ������ҡ�û�ͧ�ѹ: /r "  ;

char *DemonDamage4  = "  Demon Damage: /r "  ;
char *PoisonDamage3  = "  Poison Attack: /r "  ;
char *PikeNum4   = "  �ӹ�? /r "  ;
char *poisoningTime  = "  ���� Poisoning: /r "  ;
char *PlusCriticalName = "  ���?Critical: /r "  ;
char *SpiritFalconDamage2 = "  Spirit Falcon Damage: /r "  ;
char *LifeGegenPlus = "  ��� HP: /r "  ;
char *PlusDamage4 = "  ������ѧ����: /r "  ;
char *PiercingPercent = "  Piercing Percent: /r "  ;
char *DamageToLife = "  ����¹��Ҥ������������?HP: /r "  ;
char *MyMonster7 = "  % ����С?/r "  ;
char *LifeIncre4 = "  ������ѧ���Ե: /r "  ;
char *UndeadDamage3 = "  Undead Damage: /r "  ;

char *AttackNum3   = "  ���յ�����ͧ: /r "  ; 
char *AttackRateMinus = "  Ŵ��������� /r "  ; 
char *MaxDamagePlus2 = "  ������ѧ�����٧�ش /r "  ; 
char *LightningAddDamage3 = " Lightning :/r "  ; 
char *ReLifePercent4  = "  % �����: /r "  ; 
char *GetExp3  = "  ��?Exp:/r "  ; 
char *ResurrectionChar4  = "  Resurrection Character LV "  ; 
char *ExtinctionPercent2  = "  % �����: /r "  ; 
char *ExtinctionAmount2  = "  �ӹ�?Extinction: /r "  ; 
char *IncreLifePercent2 = "  ���?HP: /r "  ; 

char *ReduceDamage3 = "  Ŵ��Ҥ����������: /r "  ; 
char *IncreMagicDamage = "  ������ѧ���ըҡ����?Magic: /r "  ; 
char *AttackDelay3 = "  Attack Delay: /r "  ; 

char *FireDamage2 = "  Fire Damage: /r "  ;
char *IceDamage2 = "  Ice Damage: /r "  ;
char *AddDefense8 = "  ������ѧ��ͧ�ѹ: /r "  ;
char *SkillDamage5 = "  Skill Damage: /r "  ;

char *FireDamage3 = "  Fire Damage: /r "  ;
char *AddSpeed7 = "  �����������?/r "  ;
char *AfterDamage7 = "  After Damage: /r "  ;
char *MonsterSight7 = "  Monster ���: /r "  ;



char *LinghtingAddDamage4=  "  Add attack rate on lightning: /r  "  ;
char *MaxBoltNum4=  "  Max voltage: /r  "  ;
char *AddAbsorb4=  "  Add rate on absorb: /r  "  ;
char *IncreArea4=  "  Increasing range: /r  "  ;
char *IncreAttack4=  "  More attacking power: /r  "  ;
char *GolemLife4=  "  Life+strength: /r  "  ;


char *SubAbsorb4=  "  Decreasing absorption: /r  "  ;
char *AreaDamage4=  "  Attacking rate for range: /r  "  ;
char *MaxAttackNum4=  "  Max hits: /r  "  ;
char *LifeUp4=  "  Add vitality: /r  "  ;

char *CriticalAddDamage4=  "  Increase of critical: /r  "  ;
char *MonstervsSubCritical4=  "  Weaken critical of monster: /r  "  ;
char *ChargingAddPercentDamage4=  "  Add attacking rate on charging: /r  "  ;

char *IncreEvasionPercent4=  "  Additinal percentage of evasion: /r  "  ;
char *AddShadowNum4=  "  Increasing combination of ocular: /r  "  ;

char *WolverinLife4=  "  Vitality:/r  "  ;
char *WolverinRate4=  "  Accuracy:/r  "  ;
char *WolverinDefense4=  "  Defence rate:/r  "  ;
char *AddEvasion4=  "  Increasing ability of evasion:/r  "  ;
char *AddDamage4=  "  Increasing attacking rate(physic):/r  "  ;
char *FalconAddDamage4=  "  Increasing attacking rate of falcon:/r  "  ;


char *IncreAttackAbsorb4=  "  Increasing absorption rate:/r  "  ;

char *RectAngleArea4=  "  Attacking at sharp angled range:/r  "  ; 

char *MonsterSubSpeed4=  "  Decreasing speed of attacted monster:/r  "  ;


char *AddLifeRegen4=  "  Add life-regeneration:/r  "  ;
char *AddManaRegen4=  "  Add mana-regeneration:/r  "  ;
char *MagicArea4=  "  Magic affected range:/r  "  ;
char *ChainNum4=  "  A number of affected monster:/r  "  ;
char *ChainRange4=  "  The rage of affected monster:/r  "  ;
char *UndeadDamageAbsorb4=  "  Undead attacking absorption:/r  "  ;
char *UserBlockPercent4=  "  Blocking rate for character:/r  "  ;

char *SecondIncreMana4=  "  Increasing mana rate per second:/r  "  ;
char *FireDamage4=  "  Fire attacking rate:/r  "  ;
char *DecreSpeed4=  "  Decreasing Speed:/r  "  ;
char *DecreAttack4=  "  Decreasing attacking rate:/r  "  ;
char *AddPercentDamage4=  "  Adding attacking rate:/r  "  ;


char *ChainDamage3  = "  ChainDamage)/r  "  ;
char *BrandishDamage3 = "  BrandishDamage  "  ;
char *PhysicalAbsorb3 = "  PhysicalAbsorb  "  ;
char *SparkDamage3  = "  SparkDamage  "  ;
char *BrutalSwingCritical3 = "  BrutalSwingCritical  "  ;
char *Attck_RateAdd5 = "  ���?Attack Rate:/r  " ;
char *Triumph4= " Triumph " ;
char *Use4= " you " ;
char *DivineShield4= " Blocking rate of divine shiled " ;



char *QuestMonsterName[] = { " Bagon " , " Skeleton Warrior " , " Head Cutter " , " Armored Beetle " , " Skeleton Ranger " , " Titan "  ,
          " Forgetten Land Monster " , " Oasis Monster " , " Dungeon 1F Monster " };



char *VampOption[10] ={ " Aging Rating is increased: /r " , " blood sucking: /r " , " Damage: /r " , " Reduce Life : /r " , " good luck: /r " };


char *Quest3ItemName[]={ " Demon Hunter " , " Ancient Revenge " , " Myst " , " Double Side " , " Anaconda " , " Platinum Sword " , " Ghost " };

char *Quset3ItemDoc15 =  " ������? " ;
char *Quset3ItemDoc16 =  "  ������  " ;

char *SparkDamage10 =  " Spark Damage:/r " ;

char *RequirLevel3 =  " (��ͧ���Level:%d)/r  " ;

char *RecvItemTT =  " ���Ѻ�������? " ;
char *PuzzleEvent5 =  " Puzzle �������ó�  " ;
char *BabelHorn =  " Babel Horn  " ;
char *NineTailFoxItem =  " NineTail amulet  " ;
char *MyShopItemSell5 =  " �ҤҢ�?/r  " ;
char *CristalItem   = " ���ʵ�?7 ���  " ;
char *StarDust = "Glitering powder";
char *ChristMas = "Merry Christmas";

char *sinDeadCandyEat =  " ����! ����ѹ��Ҩ�����������ǹ���!  " ;

char *PotionCntDoc2 =  " �ҤҢ�µ����? " ;
char *ItemPrice7 =  " �Ҥ����? " ;

char *BuyMyShopItem7 =  "  %s ��������� %s �  %d �ѹ����?%d ��? " ;  
char *BuyMyShopItem8 =  "  ���������  %s ��?%s �� %d �ѹ����?%d ��? " ;  
char *ItemName87 =  " ���? " ;

char *sinEvent87 =  " �Ӣͧ͢�ѡ�纵�ǩ��? " ;
char *sinAddDamageItem =  " ������ѧ����: /r " ;
char *sinContinueTimeItem =  " ��������: /r " ;
char *BuyPotionMoney =  " �ӹǹ��? " ;
char *BuyPotionKind =  " �ӹ�? " ;
char *SheltomName2[] = { " Lucidy " , " Sereno " , "  Fadeo " , " Sparky " , " Raident " , " Transparo " , " Murky " , " Devine " , " Celesto " };

char *AddPercentDamage3=  " ������ѧ���� %: /r " ;

char *sinLuckyBox=  " �ا��⪤  " ;

char *SodMessage_Etc[] = {
 " �Թ�������Ѻ�ҡ����?����?Bellatra Clan  " ,
 " ����§���˹�� Clan ��ҹ�鹷����Ѻ�? " ,
 " ��Ф�Ҹ���������� %d%s. " ,
};

char *SodMessage_Clan[] = {
 " �Թ�������������  " ,
 " �Թ�ҧ��Ũ�?Bellatra ����ͤ��駡�͹  " ,
};

char *SodMessage_Master[] = {
 " �Թ�������������  " ,
 " �Թ�ҧ��Ũ�?Bellatra ����ͤ��駡�͹  " ,
 " ��ҹ��ͧ��è��Ѻ������? " ,
};

char *OtherClanMaster[] = {
 " �ѧ���Թ�ҧ������������ҧ���? " ,
 " ��س��к��ʹ�Թ����ͧ�����? " ,

};

	
char *SiegeMessage_Taxrates[] = {
	 " �ѵ����������¹�� %d%s  " ,
	 " �ʹ���շ���Ǻ�����֧�Ѩ�غѹ  " ,
	 " gold  " ,
	 " ���ԡ����������? " ,
	 " �ѵ�����ջѨ�غѹ��� %d%s  " ,
};
char *SiegeMessage_MercenrayA[] = {
	 " ���áͧ˹ع�Ԥ���෹  " ,
	 " �ͧ���������Ҩҡ�ͧ˹ع  " ,
	 " �ҤҶ١����Ҿ�ѧ��������͹�����? " ,
	 " ��Ҫԡ�����Ѻ��ҧ�ҡ��?0 / %d  " ,
};
char *SiegeMessage_MercenrayB[] = {
	 " �ͧ���û�ͧ�ѹ�դ����? " ,
	 " �դ�������Ţͧ�ѡ��Ҿ�ء��ҹ  " ,	
	 " �վ�ѧ�������鹡�ҧ  " ,
	 " ��Ҫԡ�����Ѻ��ҧ�ҡ��?20 / %d  " ,
};
char *SiegeMessage_MercenrayC[] = {
	 " �ͧ���û�ͧ�ѹ�Ҫ�ҳҨѡ?bless  " ,
	 " 㹰ҹз����Ѻ��ҧ����٧��? " ,
	 " ��ѧ����������觷����? " ,
	 " ��Ҫԡ�����Ѻ��ҧ�ҡ��?20 / %d  " ,
};
char *SiegeMessage_TowerIce[] = {
	 " �ͫ� ���ʵ�? " ,
	 " �ҵع���秤�����������͹��赡ŧ���  " ,
	 " �դ�������ö����ɤ�����������Ŵŧ  " ,
	 " ����ͫ���ʵ�����º����  " ,
};
char *SiegeMessage_TowerFire[] = {
	 " 俤��ʵ�? " ,
	 " ���¸ҵ�俾�ѧ�����٧����ش����? " ,
	 " ����դ�������ö�����  " ,
	 " ���俤��ʵ�����º����  " ,
};
char *SiegeMessage_TowerLighting[] = {
	 " �ŷ��觤��ʵ�? " ,
	 " ���¸ҵ���¿�Ҥ�����������͹��赡ŧ����  " ,
	 " �դ�������ö����ɤ�����������Ŵŧ  " ,
	 " ����ŷ�ԧ���ʵ�����º���? " ,
};
char *SiegeMessage_MerMoney =  " �������·����Ѻ���? " ;
char *SiegeMessage_TowerMoney =  " �������·�������  " ;


char *sinClanMaster7 =  " ���˹����? " ;
char *sinPrize7 =  " �Թ�ҧ���:  " ;

char *sinLevelQuestMonster[]={ " Bargon " , " Muffin " , " Dire Bee " , " Typhoon " , " Ratoo " , " Grotesque " , " Iron Fist " };
char *sinLevelQusetDoc= " Quest> %s %d number of monster needed " ;

char *sinTeleportDoc[] = { " �ҧ�?Ҷ������? " , " �ҧ�?Ҷ��Ҽ��? " , " �Թᴹ��ͧ����һ  " , " �Թᴹ��ͧ����  " };
char *sinLevelQuest90_2Mon[] = { " Omeca " , " D-Machine " , " Mountain " };
char *sinLevelQusetDoc2= " <���> ���?%s �Ҩ�ҹǹ %d ��?" ;
char *sinLevelQusetDoc3= " <���> ����������?%s  " ;
char *sinLimitTiem2 =  " ���ҷ����˹? /r " ;
char *sinMinute2 =  " �ҷ�  " ;
char *sinDay4 =  " day " ;
char *sinHour4 =  " hour " ;
char *sinLevelQusetDoc4= " <���> ��?%s %d ������º����  " ;

sinSTRING sinTestMsg7( " ������ ��͹�ѹ��駹��!  " );

char *sinMonCodeName[]={
	0,
	 " Bargon " ,
	 " Skeleton Warrior " ,
	 " Head Cutter " ,
	 " Armored Bettle " ,
	 " Skeleton Ranger " ,   
	 " Titan " ,
	 " Muffin " ,
	 " Dire Bee " ,
	 " Typhoon " ,
	 " Ratoo " ,      
	 " Grotesque " ,
	 " Iron Fist " ,
	 " Omega " ,
	 " D-Machine " ,
	 " Mountain " ,      
	 " Skeleton Knight " ,
	 " Solid Snail " ,
	 " Thron Crawler " ,
	 " Mummy " ,
	 " Doom Guard " ,     
	 " Figon " ,    
	 " Stone Giant " ,
	 " Stone Golem " ,
	 " Iron Guard " ,
	 " Avelin " ,      
	 " Avelisk-L " ,
	 " BeEvil " ,
	 " Illusion Knight " ,
	 " Nightmare " ,
	 " Witch " ,      
	 " Heavy Goblin " ,
	 " Dawlin " ,
	 " Stygian " ,
	 " Incubus " ,
	 " Chain Golem " ,     
	 " Dark Specter " ,
	 " Metron " ,
	 " Avelisk Lord " ,
	 " Dusk " ,
	 " Sadness " ,    
	0,
};

char *ChargingTime4= " �����������Ǫ��� /r  " ;
char *AddIceDamage4= " ��ѧ���ոҵع������������ /r  " ;
char *IceTime4= " �������� : /r  " ;
char *sinAttackType =  " ����Ẻ : /r "  ;
char *sinAttackType2[2] ={ " �����ѵ��ѵ� /r  " , " Auto/r " };

char *Won= " Gold " ;


char *CastlItemInfo[] ={
	{ " �����������������ѵ�٪�ǧ˹������� /r  " },  
	{ " �ѵ�Ҥ�ԵԤ�?/r  " },
	{ " �ѵ���ź���?/r  " },
	{ " HP MP Stam �ͧ��㹻����� /r  " },
	{ " ��鹷ѹ�շ���?/r  " },
	{ " ������դ�ԵԤ�ŷ������� /r  " },   
	{ " ��ѧ������������������� 20% /r " },
	0,
};
char *CastlItemInfo2[] ={
	{ " ��ѧ����Ŵ�?/2 /r  " },
	{ " �������?% /r  " },
	{ " �٧���?/r  " },
	{ " ��鹷����?/r  " },
	{ " /r " },
	{ " ������� 50% /r  " },
	{ " ��������ç���?/r  " },
	0,
};


char *CharClassTarget[8]={
	 " �������(fighter)/r " ,
	 " �������(mechanician)/r " ,
	 " �������(pikeman)/r " ,
	 " �������(archer)/r " ,
	 " �������(knight)/r " ,
	 " �������(atalanta)/r " ,
	 " �������(magician)/r " ,
	 " �������(priestess)/r " ,
};
char *AttributeTower[4]={
	 " �ҵ�(�) /r " ,
	 " �ҵ�(�����? /r " ,
	 " �ҵ�(��¿��) /r " ,
	 " �����ʴ��� %d �Թҷ� /r " ,
};

char *SiegeMessage_MerComplete = " �Ѵ�ҧ�����Ѻ��ҧ���º��������  " ;


char *haQuestMonsterName[]={
	 " Hopy " ,
	 " Rabie " ,
	 " Hobgoblin " ,
	 " NorthGoblin " ,
	 " Skeleton " ,
	 " Corrupt " ,
	 " Cyclops " ,
	 " Bagon " ,
	0,
};
char *ha100LVQuestItemName[]={
	 " Minotaur Axe " ,
	 " Extreme Talon " ,
	 " Dragon Bone Hammer  " ,
	 " Hellfire Scythe " ,
	 " Revenge Bow " ,
	 " Immortal Sword " ,
	 " Salamander Javelin " ,
	 " Gothic Staff " ,
	0,
};

char *ha100LVQuestMonterName[]={
	 " Monsters in the Heart of Perum " ,
	 " Monsters of the Galluvia Valley " ,
	0,
};
char *ha100LVQuestName =  " Fury's phantasma " ;


char *PremiumItemDoc[][2]={
	{ "It makes stat points/r "  , "to be restributed./r " },   
	{ "It makes skill points/r "  , " to be restributed./r " },  
	{ "It makes stat/skill points/r " , "to be restributed./r " },   
	{ "It makes the dead character /r "   , "to be revived./r " },    
	{ "Becomes invincible for the set time./r " , "The attack power decrease in 1/2/r " },  
	{ "15% increase /r "   , "on the critical attack./r " },		
	{ "15% increase of /r "    , "evade chance./r " },		
};
char *UpKeepItemDoc[] = {
	 "Remaining Time : %dMin" ,
	 "Extra Item Won %d%s Increase" ,
	 "Extra EXP Won %d%s Increase" ,
	 "Attack %d%s Increase" ,
	 "Restore monster's HP(part)",
	 "Restore monster's MP(part)",
	0,
};
char *UpKeepItemName[] = {
	 "Third Eyes " ,
	 "EXP Increase Potion " ,
	 "Vampiric Cuspid",
	 "Mana Recharge Potion",
	0,
};


