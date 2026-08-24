

#include "sinLinkHeader.h"


#include "Language/language.h"

#ifdef	_LANGUAGE_KOREAN

sMESSAGEBOX	sMessageBox[100] = {
	{"������ ���� ������ �����մϴ�  "},
	{"�����Ҽ� �ִ� ���Ը� �ʰ��Ͽ����ϴ�"},
	{"���� ����Ҽ� ���� ������ �Դϴ�"},
	{"     ���� �����մϴ�      "},
	{"�������� �ͽ����� �� �����ϴ� "},
	{"�ŷ��Ҽ��ִ� ������ �ʰ��Ͽ����ϴ�"},
	{"�ŷ��� ��ҵǾ����ϴ�     "},
	{"������ �����Ҽ� �����ϴ�  "},
	{"������ �ͽ��ĸ� �����ϼ̽��ϴ�"},
	{"���� ����ϼ̽��ϴ� �����մϴ�"},
	{"�������� ������ ���� ������ �� �����ϴ�"},
	{"����¡�� �Ҽ����� �������Դϴ�"},
	{"�������� �ٲ�����ϴ� Ȯ�����ּ���"},
	{"�����Ҽ��ִ� ���� �ʰ��Ͽ����ϴ�"},
	{"������ ����¡�� �����ϼ̽��ϴ�"},
	{"    �ɷ�ġ�� �ʱ�ȭ �߽��ϴ�   "},
	{"   ������ �̹����� �����ϴ�   "},
	{"�ʱ�ȭ�� 1ȸ�� �����մϴ� "},
	{"Ŭ������ ��밡���մϴ� "},
	{"���� Ŭ�������� ����� �� �ֽ��ϴ�"},
	{"���� �������� �̵��Ͻ� �� �����ϴ�"},
	{"����¡ ������ -1 �϶� �ƽ��ϴ�"},
	{"����¡ ������ -2 �϶�	�ƽ��ϴ�"},
	{"����¡ ������ +1 ��� �ƽ��ϴ�"},
	{"����¡ ������ +2 ��� �ƽ��ϴ�"},
	{"����Ʈ �������� ���ݼӵ��� �϶����ϴ�"},
	{"   ����Ʈ ������ ������    "},
	{"   ������� +15 ��µǾ����ϴ� "},
	{"   ������ �ϼ��Ǿ����ϴ�    "},
	{"   ������ �߸��Ǿ����ϴ�    "},
	{"   �Ǹ��� �������� �����ϴ�  "},
	{"�ش�������� �̹� �ǸŵǾ����ϴ�"},
	{"�Ǹ����� ���λ����� ����Ǿ����ϴ�"},
	{"   �Ǹ��ڸ� ã���������ϴ�   "},
	{"    ������ �����ʽ��ϴ�     "},
	{"    ���� ĳ���ʹ� ����Ҽ� �����ϴ�    "},
	{"    ���� ĳ���ʹ� ����Ҽ� �����ϴ�    "},
	{" ���� ĳ���ʹ� �Լ��Ҽ� ���� �������Դϴ� "},
	{" ���� ĳ���ʹ� �Լ��Ҽ� ���� �������Դϴ� "},
	{"    ������ ������ ������ �־����ϴ�  "},
	{"    ã���� Ŭ�� ����� �����ϴ�    "},
	{"  ��� ���긦 ����Ҽ� ���� �����Դϴ�  "},
	{"     ���� ���갡 ������Դϴ�    "},
	{"     ���ݷ� �Լ� �׽�Ʈ���Դϴ�    "},
	{"    ���� ���� ������ �Ϸ�Ǿ����ϴ�    "},
	{"    ���� ����Ʈ +5 ��µǾ����ϴ�    "},
	{"    ��ų ����Ʈ +1 ��µǾ����ϴ�    "},
	{"���� +5 , ��ų +1 ����Ʈ ��µǾ����ϴ�"},
	{"���� +5 , ��ų +2 ����Ʈ ��µǾ����ϴ�"},
	{"   �������ġ 1% �϶�Ǿ����ϴ�  "},
	{"    ����� +40  ��µǾ����ϴ�   " },
	{"    �������� ����Ʈ�� �ֽ��ϴ�      " },
	{"80���� ������ ����+7�� ����Ǿ����ϴ�" },
	{"90���� ������ ����+10�� ����Ǿ����ϴ�" },
	{"����� �����Ǽ��ڰ� �ʱ�ȭ�Ǿ����ϴ�" },
	{"�ð��ʰ��� ����Ʈ�� �����ϼ̽��ϴ�" },
	{"�뺴���ڸ� �ʰ� �Ͽ����ϴ�."},
	{"���� ������� ��ų������ �Դϴ�."},
	{"����� ĳ���� �̵��Ҽ� �����ϴ�.(����� �������ּ���)"},
	{"���λ����� ��� �Ҽ����� ��ǰ�Դϴ�."},
	{"���� ã���� ������ �����ϴ�."},
	{"�̹� �������翡 �����ϼ̽��ϴ�."},
	{"�������翡 ������ �ּż� �����մϴ�"},
	{"������ ���� �ʴ� ���������Դϴ�."},
	{"�������� �籸���Ǿ����ϴ�"},
	{"ǻ������ ��ῡ�� �¸� �Ͽ����ϴ�."},
	{"ǻ������ ��ῡ�� �й� �Ͽ����ϴ�."},
	{" �������� �������� ���� �޾ҽ��ϴ�."},
	{"���� �Ӽ��� �������� �ߺ� ����� �� �����ϴ�."},
	{"����Ŀ� ��밡�� �մϴ�"},
	{"���� ���� ���¿��� ��Ұ� �Ұ��� �մϴ�."},
	{"���� ���� ���¿��� ���⸦ ��� �����ϴ�."},
	{"������ ���� ������ ���ƾƮ ������ ����� �� �����ϴ�."},
	{"������ ���ÿ� �����ϼ̽��ϴ�."},
	{"������ ���ۿ� �����ϼ̽��ϴ�."},
	{"���� ���� ���¿��� ����/������ ��� �����ϴ�."},
	{"�� ���� ���¿��� �����Ǹ� ��� �����ϴ�."},
	{"���� ������ ����/������ �ƴմϴ�"},
	{"���� �����̳� �迭 ������ ���� �ʽ��ϴ�"},
	{"�� �������� 10�������� ��밡���մϴ�."},
	{"������ų ����¡�� �������� �����ϴ�."},
	{"     ������ ��ų�� �����ϴ�.     "},
	{"����¡ �������� �����Ǿ����ϴ�."},
	{"��ų ������ �Ϸ�Ǿ����ϴ�."},
	{"�׶��Ƽ ������ 5�������� ��� ���� �մϴ�."},
	{"�׶��Ƽ ������ ��� �Ͽ����ϴ�."},
	{"������ �ִ� ���¿��� �������� �� �� �����ϴ�."},
	{"������ ���̴� ������ �ø� �� �����ϴ�."},
	{"������ �÷��� ������ ��Ұ� �Ұ��� �մϴ�."},
	{"�ͽ��ĵ� �������� ������ �����Ͽ����ϴ�."},
	{"�ͽ��ĵ� ������ ������ �����Ͽ����ϴ�."},
	{"������ �籸���� �� �� �����ϴ�."},
};

sMESSAGEBOX2 sMessageBox2[20] = {
	{"��� ��������","��ðڽ��ϱ�?"},
	{"��� ��������","�Ľðڽ��ϱ�?"},
	{"��� ��������","�����ðڽ��ϱ�?"},
	{"���� ����","�����ðڽ��ϱ�?"},
	{"",""},
	{"���� ����"," �ŷ��ڽ��ϱ�?"},
	{"���� ����","�̵��Ͻðڽ��ϱ�?"},
	{"���� ����","ȸ���Ͻðڽ��ϱ�?"},
	{" ��� ���� ","��ðڽ��ϱ�?"},
	{"  ���� ����  ","����Ͻðڽ��ϱ�?"},
	{"�������� �ݾ���","����Ͻðڽ��ϱ�? "},
	{"��� ��������","��ðڽ��ϱ�?"},
	{"���� �����","ã���ðڽ��ϱ�?"},
	{"  ��������  ","��ðڽ��ϱ�?"},
	{"  ��������  ","��ðڽ��ϱ�?"},
	{"���� ����","ã���ðڽ��ϱ�."},
	{"����� �뺴��","�����Ͻðڽ��ϱ�?"}

};

sMESSAGEBOX3 sMessageBox3[MESSAGE3_MAX] = {
	{"��ų ������ ","�Ͻðڽ��ϱ�?"},
	{"  ������  ", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������","�Ͻðڽ��ϱ�?"},
	{" ��й踦 ","�Ͻðڽ��ϱ�?"},
	{"�̺�Ʈ������� ","�� �Դϴ�"},
	{"�ɷ�ġ�� �ʱ�ȭ ","�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{" ��������", "�����ðڽ��ϱ�?"},
	{" ���Ͽ��ھ", "����Ͻðڽ��ϱ�?"},
	{"  ��������", "�����ðڽ��ϱ�?"},
	{"  ���� ����", "�̵��Ͻðڽ��ϱ�? "},
	{"","��ȣ�� �Է����ּ���"},
	{" ����������","�����Ͻðڽ��ϱ�?"},
	{" ��������", "�Ľðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{" ���� ��α��� ", "�����Ͻðڽ��ϱ�?"},
	{" ���λ��� ", "ȫ�������� ��߶��"},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"  ��������", "��ðڽ��ϱ�?"},
	{"  ����� ", "����Ͻðڽ��ϱ�?"},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"�� �ڷ���Ʈ", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"  ������  ", "�Ͻðڽ��ϱ�?"},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"�� �ڷ���Ʈ", "�Ͻðڽ��ϱ�?"},
	{"ũ����Ż Ÿ����","�����Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������", "�Ͻðڽ��ϱ�?"},
	{"�� ���", "�Ͻðڽ��ϱ�?"},
	{"����Ʈ ������ ", "�Ͻðڽ��ϱ�?"},
	{"�������� ", "�����ðڽ��ϱ�?"},
	{"���������","�̵��ϰڽ��ϱ�?"},
	{"�� �ڷ���Ʈ", "�Ͻðڽ��ϱ�?"},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"�ڷ���Ʈ", "�Ͻðڽ��ϱ�?"},
	{"��������", "���� �Ͻðڽ��ϱ�?"},
	{"�ڷ���Ʈ", "�Ͻðڽ��ϱ�?"},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"������", "�����ðڽ��ϱ�?"},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
	{"����������", "��ȯ�Ͻðڽ��ϱ�? "},
};


char *SkillMaster[4] = {
	"     ��ų ����",
	"        �����ϱ� ",
	"       ���� ���� "

};
char *SkillMaster2[4] = {
	"",
	"",
	""

};

char *ReStartMsg[4] = {
	"     �ʵ忡�� ����",
	"     �������� ����",
	"       ���� ���� ",
	"   ����ġ�� �����Ͽ�"

};
char *ReStartMsg2[4] = {
	"�߰��г�Ƽ(��/����ġ)1%",
	"",
	"",
	"�ʵ������ �Ҽ������ϴ�",

};

char *SkillUseInfoText[10] = { "      ���/���� ����      ","    ���� ���ų� ����ų   ","   ��ų/������ �����Ͻʽÿ�    ","  ���� ��ų����Ʈ�� �����Ƿ� ","   ��ų�� ���� �����ϴ� " };
char *sinMsgSkillPoint = "       ��ų ����Ʈ : %d";
char *sinMsgEliteSkillPoint = "����Ʈ ��ų ����Ʈ : %d";
char *sinStateName[5] = { "����� : %d/%d  ","�� �� : %d/%d  ","�� �� : %d/%d  ","����ġ : %d/%d  ","����ġ : %dM/%dM  " };


char *TownName[3] = { "��ī�������� ��ȯ/r","�׺��ڷ� ��ȯ/r","�ʶ��̷� ��ȯ/r" };

char *sinAbilityName[50] = { "���ݷ�: /r","����ӵ�: /r","�����Ÿ�: /r","ũ��Ƽ��: /r","����: /r","���߷�: /r",
					"�����: /r","�����: /r","�̵��ӵ�: /r","������: /r","���ȸ��: /r","�����ȸ��:/r",
					"�ٷ�ȸ��: /r","���Ӽ�: /r","�ڿ��Ӽ�: /r","ȭ�Ӽ�: /r","���Ӽ�: /r","�ڼӼ�: /r","���Ӽ�: /r",
					"���Ӽ�: /r","�ٶ��Ӽ�: /r","��������: /r","������: /r","�ٷ����: /r","������߰�: /r",
					"����߰�: /r","�ٷ��߰�: /r","���ຸ������: /r","�䱸����: /r","�䱸��: /r","�䱸���ŷ�: /r",
					"�䱸���: /r","�䱸��ø��: /r","�䱸�ǰ�: /r" };


char *sinSpecialName[50] = { "Ưȭ���ݼӵ�: /r","Ưȭũ��Ƽ��: /r","Ưȭ����: /r","Ưȭ�����: /r",
						"Ưȭ�����: /r","�������õ�: /r","Ưȭ�̵��ӵ�: /r","Ưȭ��ü�Ӽ�: /r","Ưȭ�ڿ��Ӽ�: /r",
						"Ưȭ�ҼӼ�: /r","Ưȭ�ñ�Ӽ�: /r","Ưȭ�����Ӽ�: /r","Ưȭ���Ӽ�: /r","Ưȭ���Ӽ�: /r","Ưȭ�ٶ��Ӽ�: /r",
						"Ưȭ���ݷ�: /r","Ưȭ���߷�: /r","Ưȭ�����Ÿ�: /r","Ưȭ��ü����: /r","Ưȭ�ڿ�����: /r","Ưȭ�Ұ���: /r","Ưȭ�ñ����: /r",
						"Ưȭ��������: /r","Ưȭ������: /r","Ưȭ������: /r","Ưȭ�ٶ�����: /r","Ưȭ������߰�: /r","Ưȭ����߰�: /r",
						"Ưȭ��������: /r","Ưȭ������: /r","Ưȭ�ٷ����: /r","�ǸŰ���: /r","���԰���: /r" };


char *SpecialName3 = "%s Ưȭ/r";
char *MixResultTitle = "������ �ͽ��� ���";
char *NowLevelText = "���� ����/r";
char *NextLevelText = "���� ����/r";

char *NormalAttckName = "�⺻ ����";
char *RequirLevel = "%s (�䱸����:%d)/r";
char *UseItemGroupName = "��밡�� �����۱�/r";

char *PoisonName = "�� �Ӽ�:/r";
char *FireName = "�� �Ӽ�:/r";
char *AttackAreaName = "���ݹ���:/r";
char *Attack_RateName = "���߷�:/r";
char *Max_DamageIncre = "�ִ� ����������:/r";
char *DamageAddName = "���ݷ°���:/r";
char *Attack_SpeedAdd = "���ݽ��ǵ�����:/r";
char *IncreBlock = "���������:/r";
char *CountinueTime = "�����ð�:/r";
char *UseManaName = "�����:/r";
char *DamageName = "���ݷ�:/r";
char *ShootingRangeName = "�����Ÿ�:/r";
char *IncreAsorb = "���������:/r";
char *SecName = "��";
char *PlusDamageName = "���ݷ��߰�ġ:/r";
char *HitNumName = "��ŸȽ��:/r";
char *DecreLifeName = "����°���ġ:/r";
char *Attck_RateAdd = "���߷��߰�ġ:/r";
char *Add_CriticalName = "ũ��Ƽ���߰�:/r";
char *Push_AreaName = "�о�°Ÿ�:/r";
char *FireAttackDamageAdd = "ȭ �迭�����߰� :/r";
char *IceName = "�� �Ӽ�:/r";
char *IceAttackDamageAdd = "���� ���ݷ� :/r";
char *StuneRate = "����Ȯ��:/r";
char *DefenseRateIncre = "����� ����:/r";
char *WeaponSizeName = "���� ũ��:/r";
char *HwakRotationNum = "����ȸ����:/r";
char *WeaponSpeedAddName = "���⽺�ǵ尡��:/r";
char *IncreAttack_RateName = "���߷�����:/r";
char *ShootingNumName = "�߻� ����:/r";
char *SeriesShootingCount = "���� �߻�ȭ���:/r";
char *UseStaminaName = "���ٷ�:/r";
char *MasterSkillMoneyName = "��ų���ú��:/r";

char *sinGold = "%d��";
char *sinGiveItem5 = "������ ������/r";
char *sinCopyItem5 = "����� ������/r";
char *sinNum7 = "ȸ";
char *sinSkillPointName = "��ų����Ʈ";
char *sinG_Pike_Time3 = "���� �ð�:/r";
char *sinItemLimitTimeOverMsg = "��������� �������ϴ�/r";
char *sinDeadSongPyeunEat = "�̷�! ���� �������ݾ�!";
char *SmeltingResultTitle = "������ ���� ���";
char *ManufactureResultTitle = "������ ���� ���";
char *ManufacturingTitle = "��������";


char *sinMaxDamageAdd = "�ִ� ���ݷ� ����+/r";
char *sinDamagePiercing = "(���� ����)/r";
char *sinAddStamina = "�ٷ� �߰���:/r";
char *sinAddLife = "����� ȸ��ġ:/r";

char *sinDamageUndead50 = "Undead �迭���� 50% ���ݷ� ����:/r";
char *sinNumCount2 = "����:/r";
char *sinNumCount3 = "��/r";
char *sinDecreDamage = "���ݷ� ����ġ:/r";
char *sinConvert4 = "��ȯ��:/r";
char *sinIncreElement = "���ҼӼ� ���ġ:/r";
char *sinPartyIncreElement = "��Ƽ���� �Ӽ������50% ����/r";
char *sinFireBallDamage2 = "(��Ȯ�ϰ� ���� ���� 100% ������)/r";
char *sinFireBallDamage3 = "(Ÿ�� �ݰ� 60 (1/3 ������))/r";
char *sinAddDamage7 = "�߰����ݷ�:/r";
char *PlusDamageWeapon = "����";
char *SparkDamage = "���� ���ݷ�:/r";
char *SparkNum7 = "�ִ뽺��ũ��:/r";
char *StunRage = "���ϰŸ�:/r";
char *RecvPotion7 = "���� ������";


char *PartyArea7 = "������ ��Ƽ�ݰ�:/r";
char *Area17 = "�ݰ�:/r";
char *AddAttack_Rate9 = "���߷°���:/r";
char *PiercingRange = "����Ÿ�:/r";
char *IncreDefense = "��������:/r";
char *AddShootingRange = "�����Ÿ�����:/r";
char *LifeAbsorb = "����������:/r";
char *FireDamage = "�ҼӼ� �߰� ���ݷ�:/r";
char *IceDamage = "�üӼ� �߰� ���ݷ�:/r";
char *LightningDamage = "���Ӽ� �߰� ���ݷ�:/r";
char *AddManaRegen = "������ �߰�:/r";
char *LightNum = "��������:/r";
char *ReturnDamage = "�ǵ���������:/r";
char *Area18 = "��ȿ�ݰ�:/r";
char *IncreMana5 = "�������:/r";
char *Area19 = "��ȿ�Ÿ�:/r";
char *HelpTitle8 = "��   ��   ��";
char *QuestTitle8 = "��   ��   Ʈ";
char *ResearchTitle8 = "�� ��  �� ��";
char *TeleportTitle8 = "�� ��  �� Ʈ";


char *IncreWeaponAttack_RateName = "������߷�����:/r";
char *AddMaxDamage = "������ݷ°���:/r";
char *LinkCoreName = "Ŭ�������Է� �̵�/r";
char *DesLinkCore = "���:";
char *Itemul = "��������";
char *ItemRecvOk = "�����ðڽ��ϱ�?";
char *Money5 = "����";
char *Exp5 = "����ġ��";
char *WingItemName[] = { "��Ż��","�ǹ���","�����","���̾���","���̾ƽ���", "�ͽ�Ʈ����" };
char *WarpGateName[] = { "���� ����Ʈ","�� ī �� ��","�ʶ��� Ÿ��","Ȳȥ��  ��","�볪��  ��","���̳� �����","�׺��� Ÿ��","�ݴ���  ��","���� �����","�ν�Ʈ���Ϸ���" };
char *sinWarningName5 = "��     ��";
char *ClanCristalName = "Ŭ���� ����/r";
char *NowMyShopSell = "�� �� ��/r";
char *SecretNumName = "������ȣ �Է�";
char *MyShopExpDoc7 = "�� �� ȫ ��";
char *NotAgingItem2 = "����¡ �Ҽ����� ������";
char *ExpPercent2 = "����ġ : %d.%d%s";
char *ExpPercent3 = "����ġ : %d.0%d%s";
char *Mutant7 = "����Ʈ";
char *Mechanic7 = "��ī��";
char *Demon7 = "���";
char *Nomal7 = "���";
char *Undead7 = "�𵥵�";
char *MonsterAddDamage2 = "�߰� ������)/r";
char *MonsterAddDamageFire = "ȭ�Ӽ� �߰� ������)/r";
char *HaWarpGateName[] = { "����� ĳ��",0 };
char *FallGameName = "�ϱ׷��� ��";
char *EndlessGameName = "������ ž";
char *LookHelp = ">>�ڼ�������...";



char *HoldStarNumDoc = "�����ϰ� �ִ� ���� ����";
char *StarItemBuy7 = "�������� ����";
char *ChangeMoney7 = "(�� 1�� ���� : 100000��)";




char *LightningDamage2 = "�� ������:/r";
char *SheildDefense = "���� �߰�:/r";

char *DemonDamage4 = "���VS���ݷ°���:/r";
char *PoisonDamage3 = "(�ʴ�)�� ���ݷ�:/r";
char *PikeNum4 = "â�� ����:/r";
char *poisoningTime = "�ߵ� �ð�:/r";
char *PlusCriticalName = "ũ��Ƽ��Ȯ�� �߰�:/r";
char *SpiritFalconDamage2 = "���Ǹ� ���� ������:/r";
char *LifeGegenPlus = "���� ����� ����:/r";
char *PlusDamage4 = "�߰� ���� ������:/r";
char *PiercingPercent = "������ Ȯ��:/r";
char *DamageToLife = "������ȯ����:/r";
char *MyMonster7 = "�Ʊ��̵� Ȯ��:/r";
char *LifeIncre4 = "����� ����ġ:/r";
char *UndeadDamage3 = "�𵥵� ���ݷ°���:/r";

char *AttackNum3 = "���Ӱ���Ƚ��:/r";
char *AttackRateMinus = "���߷°���:/r";
char *MaxDamagePlus2 = "�ִ���ݷ��߰�:/r";
char *LightningAddDamage3 = "�ڼӼ� �߰����ݷ�:/r";
char *ReLifePercent4 = "�һ�Ȯ��:/r";
char *GetExp3 = "ȹ�����ġ:/r";
char *ResurrectionChar4 = "�һ�ĳ����LV";
char *ExtinctionPercent2 = "�Ҹ��Ȯ��:/r";
char *ExtinctionAmount2 = "�Ҹ�ɷ�:/r";
char *IncreLifePercent2 = "�����������:/r";

char *ReduceDamage3 = "������ ����ġ:/r";
char *IncreMagicDamage = "�������ݷ°���:/r";
char *AttackDelay3 = "���ݵ����:/r";

char *FireDamage2 = "ȭ�Ӽ� ���ݷ�:/r";
char *IceDamage2 = "�üӼ� ���ݷ�:/r";
char *AddDefense8 = "���� ����:/r";
char *SkillDamage5 = "������ݷ�:/r";

char *FireDamage3 = "ȭ�Ӽ� �߰����ݷ�:/r";
char *AddSpeed7 = "�̵��ӵ� ����:/r";
char *AfterDamage7 = "�����ĵ���������:/r";
char *MonsterSight7 = "���ͽþ�:/r";



char *LinghtingAddDamage4 = "����Ʈ�߰����ݷ�:/r";
char *MaxBoltNum4 = "�ִ뺼Ʈ:/r";
char *AddAbsorb4 = "������߰�:/r";
char *IncreArea4 = "��������:/r";
char *IncreAttack4 = "���ݷ�����:/r";
char *GolemLife4 = "�����:/r";


char *SubAbsorb4 = "������϶�:/r";
char *AreaDamage4 = "�������ݷ�:/r";
char *MaxAttackNum4 = "�ִ�Ÿ�ݼ�:/r";
char *LifeUp4 = "������߰���:/r";

char *CriticalAddDamage4 = "ũ��Ƽ�ð��ݷ�����:/r";
char *MonstervsSubCritical4 = "����ũ��Ƽ�þ�ȭ:/r";
char *ChargingAddPercentDamage4 = "��¡�ð��ݷ°���:/r";

char *IncreEvasionPercent4 = "�߰�ȸ��Ȯ��:/r";
char *AddShadowNum4 = "�ܻ��޺�����:/r";

char *WolverinLife4 = "�����:/r";
char *WolverinRate4 = "���߷�:/r";
char *WolverinDefense4 = "����:/r";
char *AddEvasion4 = "�߰�ȸ�Ǵɷ�����:/r";
char *AddDamage4 = "���ݷ�����:/r";
char *FalconAddDamage4 = "�������ݷ�����:/r";


char *IncreAttackAbsorb4 = "�������������:/r";

char *RectAngleArea4 = "���ݹ���:/r";

char *MonsterSubSpeed4 = "Ÿ�ݵ����ӵ�����:/r";


char *AddLifeRegen4 = "����������߰�:/r";
char *AddManaRegen4 = "���������߰�:/r";
char *MagicArea4 = "��������:/r";
char *ChainNum4 = "���᰹��:/r";
char *ChainRange4 = "����Ÿ�:/r";
char *UndeadDamageAbsorb4 = "�𵥵�������ġ:/r";
char *UserBlockPercent4 = "�߰������:/r";

char *SecondIncreMana4 = "�ʴ�������:/r";
char *FireDamage4 = "�Ұ��ݷ�:/r";
char *DecreSpeed4 = "�ӵ�����:/r";
char *DecreAttack4 = "��������:/r";
char *AddPercentDamage4 = "���ݷ°���:/r";



char *ChainDamage3 = "�ΰ�)/r";
char *BrandishDamage3 = "�귻�� ������";
char *PhysicalAbsorb3 = "�����þۼҼ� �����";
char *SparkDamage3 = "����ũ ������";
char *BrutalSwingCritical3 = "�θ�Ʋ���� ũ��Ƽ��";
char *Attck_RateAdd5 = "���߷��߰�:/r";
char *Triumph4 = "Ʈ���̾���";
char *DivineShield4 = "����ν�������";





char *QuestMonsterName[] = { "�ٰ�","���̷��� ������","���Ŀ��","�Ƹӵ� ��Ʋ","���̷��� ������","Ÿ��ź" ,
						 "�������� ����","���ƽý� ����","��밨��1�� ����" };



char *VampOption[10] = { "����¡Ȯ�����: /r","����: /r","�������ݻ�: /r","����°���: /r","���: /r" };


char *Quest3ItemName[] = { "��� ����","�ؼ�Ʈ ������","�̽�Ʈ","���� ���̵�","�Ƴ��ܴ�","�÷�Ƽ�� �ҵ�","���Ʈ" };

char *Quset3ItemDoc15 = "����Ʈ ��������";
char *Quset3ItemDoc16 = " ����� ���ϴ� ";

char *SparkDamage10 = "����ũ������ �ΰ�:/r";

char *RequirLevel3 = "(����䱸����:%d)/r";

char *RecvItemTT = "�������� �޾ҽ��ϴ�";
char *PuzzleEvent5 = "�ϼ��� ������";
char *BabelHorn = "�ٺ��� ����";
char *ChristMas = "��¦��¦ ���縦";
char *NineTailFoxItem = "�������� �ƹ�����";
char *MyShopItemSell5 = "�ǸŰ���: /r";
char *CristalItem = "�ϰ����� ũ����Ż��";


char *WatermelonItem = "7���� ������";

char *PumpkinItem = "7���� ȣ����";

char *StarItem = "7���� ��������";

char *ValentineItem = "7���� ���ݸ���";

char *PristonAlphabetItem = "7���� ���ĺ�����";

char *CandydaysItem = "7���� ĵ��";

char *MagicalGreenEmeraldItem = "7���� ���޶��带";

char *MagicalGreenJadeItem = "7���� ���븦";

char *TearOfKaraItem = "7���� ī���� ������";

char *ExpireItem = "�Ⱓ����";

char *FindinvestigatorItem = "���ξƹķ���";
char *FindinvestigatorNineItem = "���ξƹķ���";
char *FindinvestigatorTaleItem = "���Ͼƹķ���";



char *InventoryFull = "�κ��丮 ��������";
char *InvenSpaceSecure = "������ Ȯ���� �ּ���";

char *sinDeadCandyEat = "�̷�! �����Ǽ� ���� ���ݾ�!";

char *PotionCntDoc2 = "���� �Ǹ� �ݾ���";
char *ItemPrice7 = "��������  �ݾ���";

char *BuyMyShopItem7 = "%s���� %s �������� %d���� %d�� ��̽��ϴ�";
char *BuyMyShopItem8 = "%s���� %s �������� %d���� %d�� ��̽��ϴ�";
char *ItemName87 = "��������";

char *sinEvent87 = "������ �������� ��Ź";
char *sinAddDamageItem = "�߰����ݷ�: /r";
char *sinContinueTimeItem = "�����ð�: /r";
char *BuyPotionMoney = "�ݾ�:";
char *BuyPotionKind = "����:";
char *SheltomName2[] = { "������","�극�׿�","�ĵ���","����Ű","���̴�Ʈ","Ʈ�����з�","��Ű","�����","��������", "�̶���", "���丣��", "�̴ϱ׸�" };


char *BillingMagicForceName = "���� ����";


char *MagicForceName = "����";

char *AddPercentDamage3 = "���ݷ°���: /r";

char *sinLuckyBox = "���ָӴ�";

char *SodMessage_Etc[] = {
"����Ʈ�� � Ŭ������ ���޵Ǵ� ���ͱ���",
"�ش� Ŭ���� Ŭ��Ĩ�� ã�� �� �ֽ��ϴ�.",
"���� ������ %d%s �Դϴ�.",
};

char *SodMessage_Clan[] = {
"���� ����Ʈ�󿡼� �츮 Ŭ������ ����",
"�� ���ͱ��� ������ �����ϴ�.",
};

char *SodMessage_Master[] = {
"���� ����Ʈ�󿡼� �츮 Ŭ������ ����",
"�� ���ͱ��� ������ �����ϴ�.",
"���� ã���ðڽ��ϱ�?",
};

char *OtherClanMaster[] = {
"��� �� �Ϻΰ� �����ֽ��ϴ�",
"ã���÷��� �ݾ��� �Է��ϼ���.",

};

char *SiegeMessage_Taxrates[] = {
	"������ %d%s�� ���� �մϴ�.",
	"���ݱ��� ������ �����Ѿ�",
	"���Դϴ�.",
	"������ ã���ðڽ��ϱ�?.",
	"���� ������ %d%s�Դϴ�.",
};
char *SiegeMessage_MercenrayA[] = {
	"��ī���� �κ���",
	"�κ����� ������ ����������",
	"���� ��� �������� ���� ���մϴ�.",
	"�ִ�뺴�ο�:20 / %d",
};
char *SiegeMessage_MercenrayB[] = {
	"��ī���� ����",
	"���� ���� ��� ���� �ִ� �뺴����",
	"�߰����� �������� �����ϴ�.",
	"�ִ�뺴�ο�:20 / %d",
};
char *SiegeMessage_MercenrayC[] = {
	"������ձ� ����",
	"���� ������ ��� �뺴���μ�",
	"�������� ���� ���մϴ�.",
	"�ִ�뺴�ο�:20 / %d",
};
char *SiegeMessage_TowerIce[] = {
	"���̽� ũ����Ż",
	"���̽� �Ӽ����� �̵��ӵ� �϶��",
	"���ݼӵ� �϶��� Ư�� �ɷ��� �ֽ��ϴ�.",
	"���̽� ũ����Ż �����Ϸ�",
};
char *SiegeMessage_TowerFire[] = {
	"���̾� ũ����Ż",
	"���̾� �Ӽ����� ���ݷ��� ���� ������",
	"Ư���� �ɷ��� �����ϴ�.",
	"���̾� ũ����Ż �����Ϸ�",
};
char *SiegeMessage_TowerLighting[] = {
	"����Ʈ�� ũ����Ż",
	"����Ʈ�� �Ӽ����� �̵��ӵ� �϶�� ",
	"���ݼӵ� �϶��� Ư�� �ɷ��� �ֽ��ϴ�.",
	"����Ʈ�� ũ����Ż �����Ϸ�",
};
char *SiegeMessage_MerMoney = "�뺴 ���: ";
char *SiegeMessage_TowerMoney = "Ÿ�� ���: ";

char *sinClanMaster7 = "Ŭ����";
char *sinPrize7 = "���";

char *sinLevelQuestMonster[] = { "�ٰ�","����","���̾� ��","Ÿ��Ǭ","����","�׷��׽�ũ","���̾� �ǽ�Ʈ" };
char *sinLevelQusetDoc = "����Ʈ> %s %d�������";
char *sinTeleportDoc[] = { "�������� �Ա�","�������� �Ա�","���ֹ��� ��","�ݴ��� ��" };
char *sinLevelQuest90_2Mon[] = { "���ް�","��-�ӽ�","����ƾ" };
char *sinLevelQusetDoc2 = "<����Ʈ> %s %d������ ����Ͻÿ�";
char *sinLevelQusetDoc3 = "<����Ʈ> %s �κ��� �������� ȹ���Ͻÿ�";
char *sinLimitTiem2 = "���ѽð�: /r";
char *sinMinute2 = "��";
char *sinDay4 = "��";
char *sinHour4 = "��";
char *sinLevelQusetDoc4 = "<����Ʈ> %s %d������� �Ϸ�";

sinSTRING sinTestMsg7("�޷� ������ �ϵ�� ��!");

char *sinMonCodeName[] = {
	0,
	"�ٰ�",
	"���̷��� ������",
	"��� Ŀ��",
	"�Ƹӵ� ��Ʋ",
	"���̷��� ������",
	"Ÿ��ź",
	"����",
	"���̺�",
	"Ÿ��Ǭ",
	"����",
	"�׷ε���ũ",
	"���̾� �ǽ�Ʈ",
	"���ް�",
	"�� �ӽ�",
	"����ƾ",
	"���̷��� ����Ʈ",
	"�ָ��� ������",
	"�� ũ�ο﷯",
	"�ӹ�",
	"�� ����",
	"���̰�",
	"���� ���̾�Ʈ",
	"���� ���",
	"���̾� ����",
	"�ƺ���",
	"�ƺ�����ũ-L",
	"���",
	"�Ϸ��� ����Ʈ",
	"����Ʈ �޾�",
	"��ġ",
	"��� ����",
	"�ٿ츰",
	"��Ƽ����",
	"��ť����",
	"ü�� ���",
	"��ũ ������",
	"��Ʈ��",
	"�ƺ�����ũ �ε�",
	"����ũ",
	"����Ͻ�",
	0,
};

char *ChargingTime4 = "��¡�ӵ����:/r";
char *AddIceDamage4 = "���Ӽ� �߰����ݷ�:/r";
char *IceTime4 = "����ð�:/r";
char *sinAttackType = "����Ÿ��:/r";
char *sinAttackType2[2] = { "����/r","�ڵ�/r" };

char *Won = "��";


char *CastlItemInfo[] = {
	{"�����ð� �������°� ������/r"},
	{"ũ��Ƽ�� Ȯ����/r"},
	{"�߰� ȸ�� Ȯ����/r"},
	{"��Ƽ���� �����,���,�ٷ�/r"},
	{"����� ��� ��Ȱ�Ѵ�./r"},
	{"ũ��ƼŻ Ÿ���� ������/r"},
	{"Ÿ�� ���ݽ� ���ݷ��� 20%/r"},
	0,
};
char *CastlItemInfo2[] = {
	{"���ݷ��� 1/2�� �����Ѵ�./r"},
	{"5% �����Ѵ�./r"},
	{"����Ѵ�./r"},
	{"�� ��� ȸ���Ѵ�./r"},
	{"/r"},
	{"50%����Ѵ�./r"},
	{"���� ��Ų��./r"},
	0,
};
char *CharClassTarget[8] = {
	"Ÿ��(������)/r",
	"Ÿ��(��ī�ϼ�)/r",
	"Ÿ��(����ũ��)/r",
	"Ÿ��(����)/r",
	"Ÿ��(����Ʈ)/r",
	"Ÿ��(��Ż��Ÿ)/r",
	"Ÿ��(������)/r",
	"Ÿ��(������Ƽ��)/r",
};
char *AttributeTower[4] = {
	"�Ӽ�(���̾�)/r",
	"�Ӽ�(���̽�)/r",
	"�Ӽ�(����Ʈ��)/r",
	"�����ð�:%d��/r",
};

char *SiegeMessage_MerComplete = "�뺴���� �Ϸ�";


char *haQuestMonsterName[] = {
	"ȣ��",
	"����",
	"�����",
	"�뾲����",
	"���̷���",
	"Ŀ��Ʈ",
	"����Ŭ�ӽ�",
	"�ٰ�",
	0,
};
char *ha100LVQuestItemName[] = {
	"�̳��� �׽�",
	"�ͽ�Ʈ�� Ż��",
	"�巡�� �� �ظ�",
	"�����̾� ���̵�",
	"������ ����",
	"�̸�Ż �ҵ�",
	"�����Ǵ� ����",
	"��� ������",
	0,
};

char *ha100LVQuestMonterName[] = {
	"����� ���� ����",
	"�ַ��� ��� ����",
	0,
};
char *ha100LVQuestName = "ǻ���� ȯ��";


char *PremiumItemDoc[][2] = {
	{"���� ����Ʈ��/r"     ,"��й� �Ѵ�./r"},
	{"��ų ����Ʈ��/r"     ," ��й� �Ѵ�./r"},
	{"����/��ų ����Ʈ��/r"," ��й��Ѵ�./r"},
	{"���� ��ġ���� /r"    ,"��Ȱ�Ѵ�./r"},
	{"�����ð� �������°� �ȴ�./r","1/2 ���ݷ� ����/r"},
	{"ũ��Ƽ�� ���� Ȯ���� /r"    ,"15% �����Ѵ�./r"},
	{"�̺��̵� Ȯ���� /r"         ,"15% �����Ѵ�./r"},
};
char *UpKeepItemDoc[] = {
	"�ܿ��ð� : %d��",
	"�߰�������ȹ�� %d%s����",
	"�߰�����ġȹ�� %d%s����",
	"���ݷ�%d%s����",
	"���� ����� �Ϻ�ȸ��",
	"���� ��� �Ϻ�ȸ��",
	"������ %d����",
	"��¼Ҹ� %d%s����",
	"�� ����� �� �׸�",
	"�� ����� �� �ܽý�",
	"�� ����� �� �̿�",
	"�� ����� �� ��Ʈ",
	"���� ����� �Ϻ�ȸ��",
	"�ٷ¼Ҹ� %d%s����",
	0,
};
char *UpKeepItemName[] = {
	"��� ������",
	"����ġ���� ����",
	"���Ǹ� Ŀ����",
	"���� ����¡ ����",
	"����Ʈ ���� ����",
	"���� ���ེ ����",
	"�Ǵн���",
	"����� ��(�׸�)",
	"����� ��(�ܽý�)",
	"����� ��(�̿�)",
	"����� ��(��Ʈ)",
	"���Ǹ� Ŀ����EX",
	"�ٷ� ���ེ ����",
	0,
};



char *BoosterItemInfo[] = {
	{"�����ð����� �������/r"},
	{"�����ð����� �����/r"},
	{"�����ð����� �ٷ���/r"},
	0,
};
char *BoosterItemInfo2[] = {
	{"15% ��½�Ų��./r"},
	{"15% ��½�Ų��./r"},
	{"15% ��½�Ų��./r"},
	0,
};


char *SkillDelayItemInfo[] = {
	{"�����ð����� ��ų ����̸�/r"},
	0,
};
char *SkillDelayItemInfo2[] = {
	{"50% ���� ��Ų��./r"},
	0,
}

#endif

#ifdef	_LANGUAGE_CHINESE
#include "Language/chinese/C_sinMsg.h"
#endif

#ifdef	_LANGUAGE_JAPANESE
#include "Language/Japanese/J_sinMsg.h"
#endif

#ifdef	_LANGUAGE_TAIWAN
#include "Language/Taiwan/T_sinMsg.h"
#endif

#ifdef	_LANGUAGE_ENGLISH
#include "Language/English/E_sinMsg.h"
#endif

#ifdef	_LANGUAGE_THAI
#include "Language/THAI/TH_sinMsg.h"
#endif

#ifdef _LANGUAGE_VEITNAM
#include "Language/VEITNAM/V_sinMsg.h"
#endif

#ifdef _LANGUAGE_BRAZIL
#include "Language/Brazil/b_sinMsg.h"
#endif

#ifdef _LANGUAGE_ARGENTINA
#include "Language/ARGENTINA/A_sinMsg.h"
#endif


