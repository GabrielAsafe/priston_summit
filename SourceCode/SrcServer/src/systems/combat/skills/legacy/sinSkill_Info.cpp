/*Tabela de Skills
Organizada e reestruturada por Zaraki.*/

#include "systems/combat/skills/server_support/SkillSub.h"

/*__________________________________________________________________________________________________________________________

Tabela de Skills
Organizada e reestruturada por Zaraki.

__________________________________________________________________________________________________________________________*/


#include "systems/combat/skills/legacy/sinSkill_InfoIni.h"
#include "modules/File/File.h"


char* GetWords(char* q, char* p)
{

	//	while ( *p != '\n' ) {
	if (*p == 0) { *q = 0; return p; }

	while ((*p == 32) || (*p == 9) || (*p == 58)) // SPACE or TAB or ':'´Â Á¦¿Ü ½ÃÅ´
	{
		p++;
	}

	while ((*p != 32) && (*p != 9) && (*p != 58) && (*p != 0)) // SPACE or TAB
	{
		if (*p == '\n' || *p == '\r') break;
		*q++ = *p++;
	}
	*q++ = 0;	// end of one string, put Null character
	//	}
	//	*q = '\n';

	return p;
}



#include <string>
#include <sstream>

int ReadSkillFloat(float* Skill, char* szFileName, char* szSkillName)
{
#ifdef _W_SERVER
	int i;
	char Path[255], decode[255], line[255];
	FILE* file;
	char* skill;

	GetCurrentDirectoryA(sizeof(Path), Path);
	wsprintf(Path, "%s//Configs//Skill//%s", Path, szFileName);

	file = fopen(Path, "rb");

	//fopen(&file, Path, "rb");

	if (file == NULL)
		return FALSE;

	while (!feof(file)) {

		if (fgets(line, 255, file) == NULL) break;

		skill = GetWords(decode, line);
		if (lstrcmpi(decode, szSkillName) == 0) {
			for (i = 0; i < 10; i++)
			{

				while (*skill == ',' || *skill == ' ' || *skill == '=' || *skill == '.')
					if (*skill == '.')
						skill += 2;
					else
						skill++;

				if (*skill == ';') break;

				std::istringstream in(skill);

				std::string type;
				in >> type;

				Skill[i] = (float)atof(skill);
				//	Skill[i] = std::stof(type, &sz);

				while (*skill != ',')
				{
					skill++;
					if (*skill == ';') break;
				}
			}
		}
	}
	fclose(file);
#endif
	return TRUE;
}

int ReadSkillInt(int* Skill, char* szFileName, char* szSkillName)
{
	int i;
	char Path[255], decode[255], line[255];
	FILE* file;
	char* skill;

	GetCurrentDirectoryA(sizeof(Path), Path);
	wsprintf(Path, "%s//Configs//Skill//%s", Path, szFileName);

	file = fopen(Path, "rb");


	if (file == NULL)
		return FALSE;

	while (!feof(file)) {

		if (fgets(line, 255, file) == NULL) break;

		skill = GetWords(decode, line);
		if (lstrcmpi(decode, szSkillName) == 0) {
			for (i = 0; i < 10; i++) {
				while (*skill == ',' || *skill == ' ' || *skill == '=')
					skill++;

				if (*skill == ';') break;

				Skill[i] = atoi(skill);

				while (*skill != ',')
				{
					skill++;
					if (*skill == ';') break;
				}
			}
			break;
		}
	}
	fclose(file);
	return TRUE;
}

int ReadSkillInt2(int Skill[][2], char* szFileName, char* szSkillName)
{
	int i;
	char Path[255], decode[255], line[255];
	FILE* file;
	char* skill;

	GetCurrentDirectoryA(sizeof(Path), Path);
	wsprintf(Path, "%s//Configs//Skill//%s", Path, szFileName);

	file = fopen(Path, "rb");
	//fopen_s(&file, Path, "rb");

	if (file == NULL)
		return FALSE;

	while (!feof(file)) {

		if (fgets(line, 255, file) == NULL) break;

		skill = GetWords(decode, line);
		if (lstrcmpi(decode, szSkillName) == 0) {
			for (i = 0; i < 10; i++) {
				for (int j = 0; j < 2; j++) {

					while (*skill == ',' || *skill == ' ' || *skill == '=' || *skill == '(' || *skill == ')')
						skill++;

					if (*skill == ';') break;

					Skill[i][j] = atoi(skill);

					while (*skill != ',')
					{
						skill++;
						if (*skill == ';') break; // testar.
					}
				}
			}
			break;
		}
	}
	fclose(file);
	return TRUE;
}

int LoadSkillInfo()
{

#ifdef _W_SERVER
	ReadSkillInt(&E_Shield_BlockRate[0], "Mech.ini", "E_Shield_BlockRate");
	ReadSkillInt(&E_Shield_UseTime[0], "Mech.ini", "E_Shield_UseTime");
	ReadSkillInt(&E_Shield_UseMana[0], "Mech.ini", "E_Shield_UseMana");

	ReadSkillInt(&Mechanic_Bomb_DamagePercent[0], "Mech.ini", "Mechanic_Bomb_DamagePercent");
	ReadSkillInt(&Mechanic_Bomb_Attack_Range[0], "Mech.ini", "Mechanic_Bomb_Attack_Range");
	ReadSkillInt(&Mechanic_Bomb_UseMana[0], "Mech.ini", "Mechanic_Bomb_UseMana");

	ReadSkillInt2(P_Absorb, "Mech.ini", "P_Absorb");
	ReadSkillInt(&P_Absorb_UseTime[0], "Mech.ini", "P_Absorb_UseTime");
	ReadSkillInt(&P_Absor_bUseMana[0], "Mech.ini", "P_Absor_bUseMana");

	ReadSkillInt(&PlusPoison[0], "Mech.ini", "PlusPoison");

	ReadSkillInt(&G_Smash_Attack_Rate[0], "Mech.ini", "G_Smash_Attack_Rate");
	ReadSkillInt(&G_Smash_Attack_RagePercent[0], "Mech.ini", "G_Smash_Attack_RagePercent");
	ReadSkillInt(&G_Smash_UseMana[0], "Mech.ini", "G_Smash_UseMana");

	ReadSkillInt(&Maximize_Damage[0], "Mech.ini", "Maximize_Damage");
	ReadSkillInt(&Maximize_Time[0], "Mech.ini", "Maximize_Time");
	ReadSkillInt(&Maximize_UseMana[0], "Mech.ini", "Maximize_UseMana");

	ReadSkillInt(&Automation_Damage[0], "Mech.ini", "Automation_Damage");
	ReadSkillInt(&Automation_Speed[0], "Mech.ini", "Automation_Speed");
	ReadSkillInt(&Automation_Time[0], "Mech.ini", "Automation_Time");
	ReadSkillInt(&Automation_UseMana[0], "Mech.ini", "Automation_UseMana");

	ReadSkillInt(&Spark_DamagePercent[0], "Mech.ini", "Spark_DamagePercent");
	ReadSkillInt(&Spark_Area[0], "Mech.ini", "Spark_Area");
	ReadSkillInt(&Spark_UseMana[0], "Mech.ini", "Spark_UseMana");

	ReadSkillInt(&Metal_Armor_Defense[0], "Mech.ini", "Metal_Armor_Defense");
	ReadSkillInt(&Metal_Armor_Time[0], "Mech.ini", "Metal_Armor_Time");
	ReadSkillInt(&Metal_Armor_UseMana[0], "Mech.ini", "Metal_Armor_UseMana");

	ReadSkillInt(&Grand_Smash_AttackRate[0], "Mech.ini", "Grand_Smash_AttackRate");
	ReadSkillInt(&Grand_Smash_Damage[0], "Mech.ini", "Grand_Smash_Damage");
	ReadSkillInt(&Grand_Smash_UseMana[0], "Mech.ini", "Grand_Smash_UseMana");

	ReadSkillInt(&M_Weapon_Mastey[0], "Mech.ini", "M_Weapon_Mastey");

	ReadSkillInt(&Spark_Shield_Damage[0], "Mech.ini", "Spark_Shield_Damage");
	ReadSkillInt(&Spark_Shield_Defense[0], "Mech.ini", "Spark_Shield_Defense");
	ReadSkillInt(&Spark_Shield_Time[0], "Mech.ini", "Spark_Shield_Time");
	ReadSkillInt(&Spark_Shield_UseMana[0], "Mech.ini", "Spark_Shield_UseMana");

	ReadSkillInt(&Impulsion_LightingDamagePercent[0], "Mech.ini", "Impulsion_LightingDamagePercent");
	ReadSkillInt(&Impulsion_Range[0], "Mech.ini", "Impulsion_Range");
	ReadSkillInt(&Impulsion_LightingNum[0], "Mech.ini", "Impulsion_LightingNum");
	ReadSkillInt(&Impulsion_UseMana[0], "Mech.ini", "Impulsion_UseMana");

	ReadSkillInt(&Compulsion_AddAbsorb[0], "Mech.ini", "Compulsion_AddAbsorb");
	ReadSkillInt(&Compulsion_Area[0], "Mech.ini", "Compulsion_Area");
	ReadSkillInt(&Compulsion_Time[0], "Mech.ini", "Compulsion_Time");
	ReadSkillInt(&Compulsion_UseMana[0], "Mech.ini", "Compulsion_UseMana");

	ReadSkillInt(&Magnetic_Sphere_AddDamagePercent[0], "Mech.ini", "Magnetic_Sphere_AddDamagePercent");
	ReadSkillInt(&Magnetic_Sphere_Area[0], "Mech.ini", "Magnetic_Sphere_Area");
	ReadSkillInt(&Magnetic_Sphere_AttackDelay[0], "Mech.ini", "Magnetic_Sphere_AttackDelay");
	ReadSkillInt(&Magnetic_Sphere_Time[0], "Mech.ini", "Magnetic_Sphere_Time");
	ReadSkillInt(&Magnetic_Sphere_UseMana[0], "Mech.ini", "Magnetic_Sphere_UseMana");

	ReadSkillInt2(Metal_Golem_Damage, "Mech.ini", "Metal_Golem_Damage");
	ReadSkillInt(&Metal_Golem_Hit[0], "Mech.ini", "Metal_Golem_Hit");
	ReadSkillInt(&Metal_Golem_Defense[0], "Mech.ini", "Metal_Golem_Defense");
	ReadSkillInt(&Metal_Golem_Life[0], "Mech.ini", "Metal_Golem_Life");
	ReadSkillInt(&Metal_Golem_UseMana[0], "Mech.ini", "Metal_Golem_UseMana");

	ReadSkillInt(&Rupture_DamageBoost[0], "Mech.ini", "Rupture_DamageBoost");
	ReadSkillInt(&Rupture_UseMana[0], "Mech.ini", "Rupture_UseMana");
	ReadSkillInt(&Parasit_Shot_DamageBoost[0], "Mech.ini", "Parasit_Shot_DamageBoost");
	ReadSkillInt(&Parasit_Shot_PoisonDuration[0], "Mech.ini", "Parasit_Shot_PoisonDuration");
	ReadSkillInt(&Parasit_Shot_PoisonDamagePercent[0], "Mech.ini", "Parasit_Shot_PoisonDamagePercent");
	ReadSkillInt(&Parasit_Shot_UseMana[0], "Mech.ini", "Parasit_Shot_UseMana");
	ReadSkillInt(&Hardening_Shield_DamageReduction[0], "Mech.ini", "Hardening_Shield_DamageReduction");
	ReadSkillInt(&Hardening_Shield_AttackRateReduction[0], "Mech.ini", "Hardening_Shield_AttackRateReduction");
	ReadSkillInt(&Hardening_Shield_Time[0], "Mech.ini", "Hardening_Shield_time");
	ReadSkillInt(&Hardening_Shield_UseMana[0], "Mech.ini", "Hardening_Shield_UseMana");
	ReadSkillInt(&Magnetic_Discharge_DamageBoost[0], "Mech.ini", "Magnetic_Discharge_DamageBoost");
	ReadSkillInt(&Magnetic_Discharge_UseMana[0], "Mech.ini", "Magnetic_Discharge_UseMana");

	ReadSkillInt(&Melee_Mastery_DamagePercent[0], "Fighter.ini", "Melee_Mastery_DamagePercent");

	ReadSkillInt(&PlusFire[0], "Fighter.ini", "PlusFire");

	ReadSkillInt(&Raving_Damage[0], "Fighter.ini", "Raving_Damage");
	ReadSkillInt(&Ravind_Speed[0], "Fighter.ini", "Ravind_Speed");
	ReadSkillFloat(&Raving_UseLife[0], "Fighter.ini", "Raving_UseLife");
	ReadSkillInt(&Raving_UseMana[0], "Fighter.ini", "Raving_UseMana");

	ReadSkillInt(&Impact_Attack_Rating[0], "Fighter.ini", "Impact_Attack_Rating");
	ReadSkillInt(&Impact_Damage[0], "Fighter.ini", "Impact_Damage");
	ReadSkillInt(&Impact_UseMana[0], "Fighter.ini", "Impact_UseMana");

	ReadSkillInt(&T_Impact_Damage[0], "Fighter.ini", "T_Impact_Damage");
	ReadSkillInt(&T_Impact_Hit[0], "Fighter.ini", "T_Impact_Hit");
	ReadSkillInt(&T_Impact_UseMana[0], "Fighter.ini", "T_Impact_UseMana");

	ReadSkillInt(&B_Swing_Damage[0], "Fighter.ini", "B_Swing_Damage");
	ReadSkillInt(&B_Swing_Critical[0], "Fighter.ini", "B_Swing_Critical");
	ReadSkillInt(&B_Swing_UseMana[0], "Fighter.ini", "B_Swing_UseMana");

	ReadSkillInt(&Roar_Range[0], "Fighter.ini", "Roar_Range");
	ReadSkillInt(&Roar_Time[0], "Fighter.ini", "Roar_Time");
	ReadSkillInt(&Roar_UseMana[0], "Fighter.ini", "Roar_UseMana");

	ReadSkillInt(&R_Zecram_Damage[0], "Fighter.ini", "R_Zecram_Damage");
	ReadSkillInt(&R_Zecram_UseMana[0], "Fighter.ini", "R_Zecram_UseMana");

	ReadSkillInt(&Concentration_AttackRate[0], "Fighter.ini", "Concentration_AttackRate");
	ReadSkillInt(&Concentration_Time[0], "Fighter.ini", "Concentration_Time");
	ReadSkillInt(&Concentration_UseMana[0], "Fighter.ini", "Concentration_UseMana");

	ReadSkillInt(&A_Crash_Damage[0], "Fighter.ini", "A_Crash_Damage");
	ReadSkillInt(&A_Crash_AttackRate[0], "Fighter.ini", "A_Crash_AttackRate");
	ReadSkillInt(&A_Crash_UseMana[0], "Fighter.ini", "A_Crash_UseMana");

	ReadSkillInt(&Swift_Axe_Speed[0], "Fighter.ini", "Swift_Axe_Speed");
	ReadSkillInt(&Swift_Axe_Time[0], "Fighter.ini", "Swift_Axe_Time");
	ReadSkillInt(&Swift_Axe_UseMana[0], "Fighter.ini", "Swift_Axe_UseMana");

	ReadSkillInt(&B_Crash_Damage[0], "Fighter.ini", "B_Crash_Damage");
	ReadSkillInt(&B_Crash_DemonDamage[0], "Fighter.ini", "B_Crash_DemonDamage");
	ReadSkillInt(&B_Crash_UseMana[0], "Fighter.ini", "B_Crash_UseMana");

	ReadSkillInt(&Destoryer_DamagePercent[0], "Fighter.ini", "Destoryer_DamagePercent");
	ReadSkillInt(&Destoryer_AddCritical[0], "Fighter.ini", "Destoryer_AddCritical");
	ReadSkillInt(&Destoryer_UseMana[0], "Fighter.ini", "Destoryer_UseMana");

	ReadSkillInt(&Berserker_AddAttack[0], "Fighter.ini", "Berserker_AddAttack");
	ReadSkillInt(&Berserker_SubAbsorb[0], "Fighter.ini", "Berserker_SubAbsorb");
	ReadSkillInt(&Berserker_Time[0], "Fighter.ini", "Berserker_Time");
	ReadSkillInt(&Berserker_UseMana[0], "Fighter.ini", "Berserker_UseMana");

	ReadSkillInt(&Cyclone_Strike_DamagePercent[0], "Fighter.ini", "Cyclone_Strike_DamagePercent");
	ReadSkillInt2(Cyclone_Strike_AreaDamage, "Fighter.ini", "Cyclone_Strike_AreaDamage");
	ReadSkillInt(&Cyclone_Strike_AttackNum[0], "Fighter.ini", "Cyclone_Strike_AttackNum");
	ReadSkillInt(&Cyclone_Strike_Area[0], "Fighter.ini", "Cyclone_Strike_Area");
	ReadSkillInt(&Cyclone_Strike_UseMana[0], "Fighter.ini", "Cyclone_Strike_UseMana");

	ReadSkillInt(&Boost_Health_Life[0], "Fighter.ini", "Boost_Health_Life");
	ReadSkillInt(&Fighter_T5_Survival_Life[0], "Fighter.ini", "Fighter_T5_Survival_Life");
	ReadSkillInt(&Fighter_T5_Survival_Regen[0], "Fighter.ini", "Fighter_T5_Survival_Regen");
	ReadSkillInt(&Fighter_T5_Seismic_Damage[0], "Fighter.ini", "Fighter_T5_Seismic_Damage");
	ReadSkillInt(&Fighter_T5_Seismic_Area[0], "Fighter.ini", "Fighter_T5_Seismic_Area");
	ReadSkillInt(&Fighter_T5_Seismic_UseMana[0], "Fighter.ini", "Fighter_T5_Seismic_UseMana");
	ReadSkillFloat(&Fighter_T5_Ruthless_HPDecrease[0], "Fighter.ini", "Fighter_T5_Ruthless_HPDecrease");
	ReadSkillInt(&Fighter_T5_Ruthless_HitsBoost[0], "Fighter.ini", "Fighter_T5_Ruthless_HitsBoost");
	ReadSkillInt(&Fighter_T5_Ruthless_DamageBoost[0], "Fighter.ini", "Fighter_T5_Ruthless_DamageBoost");
	ReadSkillInt(&Fighter_T5_Ruthless_UseMana[0], "Fighter.ini", "Fighter_T5_Ruthless_UseMana");
	ReadSkillInt(&Fighter_T5_Whirlwind_DamageBoost[0], "Fighter.ini", "Fighter_T5_Whirlwind_DamageBoost");
	ReadSkillInt(&Fighter_T5_Whirlwind_Duration[0], "Fighter.ini", "Fighter_T5_Whirlwind_Duration");
	ReadSkillInt(&Fighter_T5_Whirlwind_MPDrain[0], "Fighter.ini", "Fighter_T5_Whirlwind_MPDrain");
	ReadSkillInt(&Fighter_T5_Whirlwind_SPDrain[0], "Fighter.ini", "Fighter_T5_Whirlwind_SPDrain");

	ReadSkillInt(&Pike_Wind_Throw_Height[0], "Pike.ini", "Pike_Wind_Throw_Height");
	ReadSkillInt(&Pike_Wind_DamagePercent[0], "Pike.ini", "Pike_Wind_DamagePercent");
	ReadSkillInt(&Pike_Wind_Push_Lenght[0], "Pike.ini", "Pike_Wind_Push_Lenght");
	ReadSkillInt(&Pike_Wind_UseMana[0], "Pike.ini", "Pike_Wind_UseMana");

	ReadSkillInt(&PlusIce[0], "Pike.ini", "PlusIce");

	ReadSkillInt(&Critical_Hit_Critical[0], "Pike.ini", "Critical_Hit_Critical");
	ReadSkillInt(&Critical_Hit_UseMana[0], "Pike.ini", "Critical_Hit_UseMana");

	ReadSkillInt(&Jumping_Crash_UseMana[0], "Pike.ini", "Jumping_Crash_UseMana");
	ReadSkillInt(&Jumping_Crash_Attack_Rating[0], "Pike.ini", "Jumping_Crash_Attack_Rating");
	ReadSkillInt(&Jumping_Crash_Damage[0], "Pike.ini", "Jumping_Crash_Damage");

	ReadSkillInt(&G_Pike_DamagePercent[0], "Pike.ini", "G_Pike_DamagePercent");
	ReadSkillInt(&G_Pike_Range[0], "Pike.ini", "G_Pike_Range");
	ReadSkillInt(&G_Pike_UseMana[0], "Pike.ini", "G_Pike_UseMana");
	ReadSkillInt(&G_Pike_Time[0], "Pike.ini", "G_Pike_Time");

	ReadSkillInt(&Tornado_Damage[0], "Pike.ini", "Tornado_Damage");
	ReadSkillInt(&Tornado_Stun[0], "Pike.ini", "Tornado_Stun");
	ReadSkillInt(&Tornado_Range[0], "Pike.ini", "Tornado_Range");
	ReadSkillInt(&Tornado_UseMana[0], "Pike.ini", "Tornado_UseMana");

	ReadSkillInt(&W_D_Mastery_Block[0], "Pike.ini", "W_D_Mastery_Block");

	ReadSkillInt(&Expansion_Size[0], "Pike.ini", "Expansion_Size");
	ReadSkillInt(&Expansion_Damage[0], "Pike.ini", "Expansion_Damage");
	ReadSkillInt(&Expansion_UseMana[0], "Pike.ini", "Expansion_UseMana");

	ReadSkillInt(&VenomSpear_DamagePercent[0], "Pike.ini", "VenomSpear_DamagePercent");
	ReadSkillInt(&VenomSpear_Num[0], "Pike.ini", "VenomSpear_Num");
	ReadSkillInt(&VenomSpear_Time[0], "Pike.ini", "VenomSpear_Time");
	ReadSkillInt(&VenomSpear_Range[0], "Pike.ini", "VenomSpear_Range");
	ReadSkillInt(&VenomSpear_UseMana[0], "Pike.ini", "VenomSpear_UseMana");

	ReadSkillFloat(&Vanish_Speed[0], "Pike.ini", "Vanish_Speed");
	ReadSkillInt(&Vanish_Damage[0], "Pike.ini", "Vanish_Damage");
	ReadSkillInt(&Vanish_Range[0], "Pike.ini", "Vanish_Range");
	ReadSkillInt(&Vanish_Time[0], "Pike.ini", "Vanish_Time");
	ReadSkillInt(&Vanish_UseMana[0], "Pike.ini", "Vanish_UseMana");

	ReadSkillInt(&Critical_Mastery_Critical[0], "Pike.ini", "Critical_Mastery_Critical");

	ReadSkillInt(&Chain_Lance_Damage[0], "Pike.ini", "Chain_Lance_Damage");
	ReadSkillInt(&Chain_Lance_UseMana[0], "Pike.ini", "Chain_Lance_UseMana");

	ReadSkillInt(&Assassin_Eye_AddCritical[0], "Pike.ini", "Assassin_Eye_AddCritical");
	ReadSkillInt(&Assassin_Eye_M_SubCritical[0], "Pike.ini", "Assassin_Eye_M_SubCritical");
	ReadSkillInt(&Assassin_Eye_Time[0], "Pike.ini", "Assassin_Eye_Time");
	ReadSkillInt(&Assassin_Eye_UseMana[0], "Pike.ini", "Assassin_Eye_UseMana");

	ReadSkillInt(&Charging_Strike_DamagePercent[0], "Pike.ini", "Charging_Strike_DamagePercent");
	ReadSkillInt(&Charging_Strike_CharingDamagePercent[0], "Pike.ini", "Charging_Strike_CharingDamagePercent");
	ReadSkillInt(&Charging_Strike_Time[0], "Pike.ini", "Charging_Strike_Time");
	ReadSkillInt(&Charging_Strike_UseMana[0], "Pike.ini", "Charging_Strike_UseMana");

	ReadSkillInt(&Vague_EvasionPercent[0], "Pike.ini", "Vague_EvasionPercent");
	ReadSkillInt(&Vague_Time[0], "Pike.ini", "Vague_Time");
	ReadSkillInt(&Vague_UseMana[0], "Pike.ini", "Vague_UseMana");

	ReadSkillInt(&Shadow_Master_DamagePercent[0], "Pike.ini", "Shadow_Master_DamagePercent");
	ReadSkillInt(&Shadow_Master_ShadowNum[0], "Pike.ini", "Shadow_Master_ShadowNum");
	ReadSkillInt(&Shadow_Master_UseMana[0], "Pike.ini", "Shadow_Master_UseMana");

	ReadSkillInt(&Pike_T5_Ring_Duration[0], "Pike.ini", "Pike_T5_Ring_Duration");
	ReadSkillInt(&Pike_T5_Ring_UseMana[0], "Pike.ini", "Pike_T5_Ring_UseMana");
	ReadSkillInt(&Pike_T5_Focus_MinAttackReduction[0], "Pike.ini", "Pike_T5_Focus_MinAttackReduction");
	ReadSkillInt(&Pike_T5_Focus_MaxAttackIncrease[0], "Pike.ini", "Pike_T5_Focus_MaxAttackIncrease");
	ReadSkillInt(&Pike_T5_Focus_UseMana[0], "Pike.ini", "Pike_T5_Focus_UseMana");
	ReadSkillInt(&Pike_T5_Death_DamageBoost[0], "Pike.ini", "Pike_T5_Death_DamageBoost");
	ReadSkillInt(&Pike_T5_Death_AttackRateBoost[0], "Pike.ini", "Pike_T5_Death_AttackRateBoost");
	ReadSkillInt(&Pike_T5_Death_UseMana[0], "Pike.ini", "Pike_T5_Death_UseMana");
	ReadSkillInt(&Pike_T5_Twister_DamageBoost[0], "Pike.ini", "Pike_T5_Twister_DamageBoost");
	ReadSkillInt(&Pike_T5_Twister_UseMana[0], "Pike.ini", "Pike_T5_Twister_UseMana");

	ReadSkillInt(&Scout_Hawk_Attack_Rate[0], "Archer.ini", "Scout_Hawk_Attack_Rate");
	ReadSkillInt(&Scout_Hawk_UseTime[0], "Archer.ini", "Scout_Hawk_UseTime");
	ReadSkillInt(&Scout_Hawk_UseMana[0], "Archer.ini", "Scout_Hawk_UseMana");

	ReadSkillInt(&S_Mastery_DamagePercent[0], "Archer.ini", "S_Mastery_DamagePercent");

	ReadSkillInt(&Wind_Arrow_Speed[0], "Archer.ini", "Wind_Arrow_Speed");
	ReadSkillInt(&Wind_Arrow_Damage[0], "Archer.ini", "Wind_Arrow_Damage");
	ReadSkillInt(&Wind_Arrow_UseMana[0], "Archer.ini", "Wind_Arrow_UseMana");

	ReadSkillInt(&Perfect_Aim_Attack_Rate[0], "Archer.ini", "Perfect_Aim_Attack_Rate");
	ReadSkillInt(&Perfect_Aim_DamagePercent[0], "Archer.ini", "Perfect_Aim_DamagePercent");
	ReadSkillInt(&Perfect_Aim_Damage_LV[0], "Archer.ini", "Perfect_Aim_Damage_LV");
	ReadSkillInt(&Perfect_Aim_UseMana[0], "Archer.ini", "Perfect_Aim_UseMana");

	ReadSkillInt(&D_Eye_Attack_Rate[0], "Archer.ini", "D_Eye_Attack_Rate");

	ReadSkillInt2(Falcon_Damage, "Archer.ini", "Falcon_Damage");
	ReadSkillInt(&Falcon_Time[0], "Archer.ini", "Falcon_Time");
	ReadSkillInt(&Falcon_UseMana[0], "Archer.ini", "Falcon_UseMana");

	ReadSkillInt(&A_Rage_ArrowNum[0], "Archer.ini", "A_Rage_ArrowNum");
	ReadSkillInt(&A_Rage_Damage[0], "Archer.ini", "A_Rage_Damage");
	ReadSkillInt(&A_Rage_Area[0], "Archer.ini", "A_Rage_Area");
	ReadSkillInt(&A_Rage_UseMana[0], "Archer.ini", "A_Rage_UseMana");

	ReadSkillInt(&Avalanche_Speed[0], "Archer.ini", "Avalanche_Speed");
	ReadSkillInt(&Avalanche_Damage[0], "Archer.ini", "Avalanche_Damage");
	ReadSkillInt(&Avalanche_ArrowNum[0], "Archer.ini", "Avalanche_ArrowNum");
	ReadSkillInt(&Avalanche_UseMana[0], "Archer.ini", "Avalanche_UseMana");

	ReadSkillInt2(Elemental_Shot_Ice, "Archer.ini", "Elemental_Shot_Ice");
	ReadSkillInt2(Elemental_Shot_Fire, "Archer.ini", "Elemental_Shot_Fire");
	ReadSkillInt2(Elemental_Shot_Lightning, "Archer.ini", "Elemental_Shot_Lightning");
	ReadSkillInt(&Elemental_ShotPercent[0], "Archer.ini", "Elemental_ShotPercent");
	ReadSkillInt(&Elemental_Shot_UseMana[0], "Archer.ini", "Elemental_Shot_UseMana");

	ReadSkillInt(&Golden_Falcon_Damage[0], "Archer.ini", "Golden_Falcon_Damage");
	ReadSkillFloat(&Golden_Falcon_LifeRegen[0], "Archer.ini", "Golden_Falcon_LifeRegen");
	ReadSkillInt(&Golden_Falcon_Time[0], "Archer.ini", "Golden_Falcon_Time");
	ReadSkillInt(&Golden_Falcon_UseMana[0], "Archer.ini", "Golden_Falcon_UseMana");

	ReadSkillInt(&BombShot_Damage[0], "Archer.ini", "BombShot_Damage");
	ReadSkillInt(&BombShot_DemonDamage[0], "Archer.ini", "BombShot_DemonDamage");
	ReadSkillInt2(BombShot_AddDamage, "Archer.ini", "BombShot_AddDamage");
	ReadSkillInt(&BombShot_Area[0], "Archer.ini", "BombShot_Area");
	ReadSkillInt(&BombShot_UseMana[0], "Archer.ini", "BombShot_UseMana");

	ReadSkillInt(&Perforation_Damage[0], "Archer.ini", "Perforation_Damage");
	ReadSkillInt(&Perforation_Critical[0], "Archer.ini", "Perforation_Critical");
	ReadSkillInt(&Perforation_UseMana[0], "Archer.ini", "Perforation_UseMana");
	ReadSkillInt(&Perforation_Attack_Range[0], "Archer.ini", "Perforation_Attack_Range");

	ReadSkillInt2(Recall_Wolverin_Damage, "Archer.ini", "Recall_Wolverin_Damage");
	ReadSkillInt(&Recall_Wolverin_Hit[0], "Archer.ini", "Recall_Wolverin_Hit");
	ReadSkillInt(&Recall_Wolverin_Defense[0], "Archer.ini", "Recall_Wolverin_Defense");
	ReadSkillInt(&Recall_Wolverin_Life[0], "Archer.ini", "Recall_Wolverin_Life");
	ReadSkillInt(&Recall_Wolverin_UseMana[0], "Archer.ini", "Recall_Wolverin_UseMana");

	ReadSkillInt(&Evasion_Mastery_AddPercent[0], "Archer.ini", "Evasion_Mastery_AddPercent");

	ReadSkillInt(&Phoenix_Shot_DamagePercent[0], "Archer.ini", "Phoenix_Shot_DamagePercent");
	ReadSkillInt(&Phoenix_Shot_Time[0], "Archer.ini", "Phoenix_Shot_Time");
	ReadSkillInt(&Phoenix_Shot_UseMana[0], "Archer.ini", "Phoenix_Shot_UseMana");

	ReadSkillInt(&Force_Of_Nature_AddDamage[0], "Archer.ini", "Force_Of_Nature_AddDamage");
	ReadSkillInt(&Force_Of_Nature_AddHit[0], "Archer.ini", "Force_Of_Nature_AddHit");
	ReadSkillInt(&Force_Of_Nature_FalconAddDamage[0], "Archer.ini", "Force_Of_Nature_FalconAddDamage");
	ReadSkillInt(&Force_Of_Nature_Time[0], "Archer.ini", "Force_Of_Nature_Time");
	ReadSkillInt(&Force_Of_Nature_UseMana[0], "Archer.ini", "Force_Of_Nature_UseMana");

	ReadSkillInt(&Sword_Blast_UseMana[0], "Knight.ini", "Sword_Blast_UseMana");
	ReadSkillInt(&Sword_Blast_DamagePercent[0], "Knight.ini", "Sword_Blast_DamagePercent");
	ReadSkillInt(&Sword_Blast_ShootingRange[0], "Knight.ini", "Sword_Blast_ShootingRange");

	ReadSkillInt(&HolyBody_UseMana[0], "Knight.ini", "HolyBody_UseMana");
	ReadSkillInt(&HolyBody_Absorb[0], "Knight.ini", "HolyBody_Absorb");
	ReadSkillInt(&HolyBody_Time[0], "Knight.ini", "HolyBody_Time");

	ReadSkillInt(&P_Traning_Stamina[0], "Knight.ini", "P_Traning_Stamina");

	ReadSkillInt(&D_Crash_UseMana[0], "Knight.ini", "D_Crash_UseMana");
	ReadSkillInt(&D_Crash_Damage[0], "Knight.ini", "D_Crash_Damage");
	ReadSkillInt(&D_Crash_Critical[0], "Knight.ini", "D_Crash_Critical");

	ReadSkillInt(&Holy_Valor_Damage[0], "Knight.ini", "Holy_Valor_Damage");
	ReadSkillInt(&Holy_Valor_Time[0], "Knight.ini", "Holy_Valor_Time");
	ReadSkillInt(&Holy_Valor_Mana[0], "Knight.ini", "Holy_Valor_Mana");

	ReadSkillInt(&Brandish_Range[0], "Knight.ini", "Brandish_Range");
	ReadSkillInt(&Brandish_Damage[0], "Knight.ini", "Brandish_Damage");
	ReadSkillInt(&Brandish_Mana[0], "Knight.ini", "Brandish_Mana");

	ReadSkillInt(&Piercing_Damage[0], "Knight.ini", "Piercing_Damage");
	ReadSkillInt(&Piercing_Attack_Rating[0], "Knight.ini", "Piercing_Attack_Rating");
	ReadSkillInt(&Piercing_Mana[0], "Knight.ini", "Piercing_Mana");

	ReadSkillInt(&Drastic_Spirit_Defense[0], "Knight.ini", "Drastic_Spirit_Defense");
	ReadSkillInt(&Drastic_Spirit_Time[0], "Knight.ini", "Drastic_Spirit_Time");
	ReadSkillInt(&Drastic_Spirit_Mana[0], "Knight.ini", "Drastic_Spirit_Mana");

	ReadSkillInt(&Sword_Mastery_DamagePercent[0], "Knight.ini", "Sword_Mastery_DamagePercent");

	ReadSkillInt(&D_Inhalation_Life[0], "Knight.ini", "D_Inhalation_Life");
	ReadSkillInt(&D_Inhalation_Block[0], "Knight.ini", "D_Inhalation_Block");
	ReadSkillInt(&D_Inhalation_Time[0], "Knight.ini", "D_Inhalation_Time");
	ReadSkillInt(&D_Inhalation_UseMana[0], "Knight.ini", "D_Inhalation_UseMana");

	ReadSkillInt(&H_Incantation_Success[0], "Knight.ini", "H_Incantation_Success");
	ReadSkillInt(&H_Incantation_AddLife[0], "Knight.ini", "H_Incantation_AddLife");
	ReadSkillInt(&H_Incantation_Time[0], "Knight.ini", "H_Incantation_Time");
	ReadSkillInt(&H_Incantation_UseMana[0], "Knight.ini", "H_Incantation_UseMana");

	ReadSkillInt(&Grand_Cross_Damage[0], "Knight.ini", "Grand_Cross_Damage");
	ReadSkillInt(&Grand_Cross_AttackRate[0], "Knight.ini", "Grand_Cross_AttackRate");
	ReadSkillInt(&Grand_Cross_UndeadDamage[0], "Knight.ini", "Grand_Cross_UndeadDamage");
	ReadSkillInt(&Grand_Cross_UseMana[0], "Knight.ini", "Grand_Cross_UseMana");

	ReadSkillInt(&Godly_Shield_AbsorbPercent[0], "Knight.ini", "Godly_Shield_AbsorbPercent");
	ReadSkillInt(&Godly_Shield_Time[0], "Knight.ini", "Godly_Shield_Time");
	ReadSkillInt(&Godly_Shield_UseMana[0], "Knight.ini", "Godly_Shield_UseMana");

	ReadSkillInt(&God_Bless_AddDamage[0], "Knight.ini", "God_Bless_AddDamage");
	ReadSkillInt(&God_Bless_Time[0], "Knight.ini", "God_Bless_Time");
	ReadSkillInt(&God_Bless_UseMana[0], "Knight.ini", "God_Bless_UseMana");

	ReadSkillInt(&Sword_Of_Justice_DamagePercent[0], "Knight.ini", "Sword_Of_Justice_DamagePercent");
	ReadSkillInt(&Sword_Of_Justice_Area[0], "Knight.ini", "Sword_Of_Justice_Area");
	ReadSkillInt(&Sword_Of_Justice_UseMana[0], "Knight.ini", "Sword_Of_Justice_UseMana");

	ReadSkillInt(&Knight_T5_HolyConviction_UndeadBoost[0], "Knight.ini", "Knight_T5_HolyConviction_UndeadBoost");
	ReadSkillInt(&Knight_T5_HolyConviction_Duration[0], "Knight.ini", "Knight_T5_HolyConviction_Duration");
	ReadSkillInt(&Knight_T5_HolyConviction_UseMana[0], "Knight.ini", "Knight_T5_HolyConviction_UseMana");
	ReadSkillInt(&Knight_T5_DivineInquisition_DamageBoost[0], "Knight.ini", "Knight_T5_DivineInquisition_DamageBoost");
	ReadSkillInt(&Knight_T5_DivineInquisition_Area[0], "Knight.ini", "Knight_T5_DivineInquisition_Area");
	ReadSkillInt(&Knight_T5_DivineInquisition_UndeadDamage[0], "Knight.ini", "Knight_T5_DivineInquisition_UndeadDamage");
	ReadSkillInt(&Knight_T5_DivineInquisition_UseMana[0], "Knight.ini", "Knight_T5_DivineInquisition_UseMana");
	ReadSkillInt(&Knight_T5_GloriousShield_DefenseBoost[0], "Knight.ini", "Knight_T5_GloriousShield_DefenseBoost");
	ReadSkillInt(&Knight_T5_GloriousShield_Duration[0], "Knight.ini", "Knight_T5_GloriousShield_Duration");
	ReadSkillInt(&Knight_T5_GloriousShield_UseMana[0], "Knight.ini", "Knight_T5_GloriousShield_UseMana");
	ReadSkillInt(&Knight_T5_DivineCross_DamageBoost[0], "Knight.ini", "Knight_T5_DivineCross_DamageBoost");
	ReadSkillInt(&Knight_T5_DivineCross_UseMana[0], "Knight.ini", "Knight_T5_DivineCross_UseMana");


	ReadSkillInt(&Divine_Piercing_DamagePercent[0], "Knight.ini", "Divine_Piercing_DamagePercent");
	ReadSkillInt(&Divine_Piercing_AddHit[0], "Knight.ini", "Divine_Piercing_AddHit");
	ReadSkillInt(&Divine_Piercing_AttackNum[0], "Knight.ini", "Divine_Piercing_AttackNum");
	ReadSkillInt(&Divine_Piercing_UseMana[0], "Knight.ini", "Divine_Piercing_UseMana");

	ReadSkillInt(&S_Strike_UseMana[0], "Atalanta.ini", "S_Strike_UseMana");
	ReadSkillInt(&S_Strike_DamagePercent[0], "Atalanta.ini", "S_Strike_DamagePercent");
	int S_Strike_Stun[10] = {6,9,12,15,18,21,24,27,30,34};
ReadSkillInt(&S_Strike_Stun[0], "Atalanta.ini", "S_Strike_Stun");

	ReadSkillInt(&Farina_UseMana[0], "Atalanta.ini", "Farina_UseMana");
	ReadSkillInt(&Farina_Speed[0], "Atalanta.ini", "Farina_Speed");
	ReadSkillInt(&Farina_AttackRate[0], "Atalanta.ini", "Farina_AttackRate");

	ReadSkillInt(&T_Mastery_Damage[0], "Atalanta.ini", "T_Mastery_Damage");

	ReadSkillInt(&V_Spear_UseMana[0], "Atalanta.ini", "V_Spear_UseMana");
	ReadSkillInt(&V_Spear_DamagePercent[0], "Atalanta.ini", "V_Spear_DamagePercent");

	ReadSkillInt(&Windy_Attack_Rating[0], "Atalanta.ini", "Windy_Attack_Rating");
	ReadSkillInt(&Windy_Time[0], "Atalanta.ini", "Windy_Time");
	ReadSkillInt(&Windy_Mana[0], "Atalanta.ini", "Windy_Mana");

	ReadSkillInt(&Twist_Javelin_Damage[0], "Atalanta.ini", "Twist_Javelin_Damage");
	ReadSkillInt(&Twist_Javelin_Attack_Rating[0], "Atalanta.ini", "Twist_Javelin_Attack_Rating");
	ReadSkillInt(&Twist_Javelin_Mana[0], "Atalanta.ini", "Twist_Javelin_Mana");

	ReadSkillInt(&Soul_Sucker_Absorb[0], "Atalanta.ini", "Soul_Sucker_Absorb");
	ReadSkillInt(&Soul_Sucker_Mana[0], "Atalanta.ini", "Soul_Sucker_Mana");

	ReadSkillInt(&Fire_Javelin_DamagePercent[0], "Atalanta.ini", "Fire_Javelin_DamagePercent");
	ReadSkillInt(&Fire_Javelin_Mana[0], "Atalanta.ini", "Fire_Javelin_Mana");

	ReadSkillInt(&Split_Javelin_AttackNum[0], "Atalanta.ini", "Split_Javelin_AttackNum");
	ReadSkillInt(&Split_Javelin_Damage[0], "Atalanta.ini", "Split_Javelin_Damage");
	ReadSkillInt(&Split_Javelin_AttackRate[0], "Atalanta.ini", "Split_Javelin_AttackRate");
	ReadSkillInt(&Split_Javelin_UseMana[0], "Atalanta.ini", "Split_Javelin_UseMana");

	ReadSkillInt(&T_Of_Valhalla_Damage[0], "Atalanta.ini", "T_Of_Valhalla_Damage");
	ReadSkillInt(&T_Of_Valhalla_Time[0], "Atalanta.ini", "T_Of_Valhalla_Time");
	ReadSkillInt(&T_Of_Valhalla_UseMana[0], "Atalanta.ini", "T_Of_Valhalla_UseMana");

	ReadSkillInt(&Lightning_Javelin_DamagePercent[0], "Atalanta.ini", "Lightning_Javelin_DamagePercent");
	ReadSkillInt(&Lightning_Javelin_UseMana[0], "Atalanta.ini", "Lightning_Javelin_UseMana");

	ReadSkillInt(&Storm_Javelin_Damage[0], "Atalanta.ini", "Storm_Javelin_Damage");
	ReadSkillInt(&Storm_Javelin_UseMana[0], "Atalanta.ini", "Storm_Javelin_UseMana");

	ReadSkillInt(&Hall_Of_Valhalla_AddEvation[0], "Atalanta.ini", "Hall_Of_Valhalla_AddEvation");
	ReadSkillInt(&Hall_Of_Valhalla_Time[0], "Atalanta.ini", "Hall_Of_Valhalla_Time");
	ReadSkillInt(&Hall_Of_Valhalla_UseMana[0], "Atalanta.ini", "Hall_Of_Valhalla_UseMana");

	ReadSkillInt(&X_Rage_DamagePercent[0], "Atalanta.ini", "X_Rage_DamagePercent");
	ReadSkillInt(&X_Rage_Area[0], "Atalanta.ini", "X_Rage_Area");
	ReadSkillInt(&X_Rage_UseMana[0], "Atalanta.ini", "X_Rage_UseMana");

	ReadSkillInt2(Frost_Javelin_IceAddDamage, "Atalanta.ini", "Frost_Javelin_IceAddDamage");
	ReadSkillInt(&Frost_Javelin_SpeedSubPercent[0], "Atalanta.ini", "Frost_Javelin_SpeedSubPercent");
	ReadSkillInt(&Frost_Javelin_IceTime[0], "Atalanta.ini", "Frost_Javelin_IceTime");
	ReadSkillInt(&Frost_Javelin_Time[0], "Atalanta.ini", "Frost_Javelin_Time");
	ReadSkillInt(&Frost_Javelin_UseMana[0], "Atalanta.ini", "Frost_Javelin_UseMana");

	ReadSkillInt(&Vengeance_DamagePercent[0], "Atalanta.ini", "Vengeance_DamagePercent");
	ReadSkillInt(&Vengeance_Range[0], "Atalanta.ini", "Vengeance_Range");
	ReadSkillInt(&Vengeance_UseMana[0], "Atalanta.ini", "Vengeance_UseMana");
	ReadSkillInt(&Vengeance_AddCritical[0], "Atalanta.ini", "Vengeance_AddCritical");

	ReadSkillInt(&Atalanta_T5_SummonTiger_AttackPower[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_AttackPower");
	ReadSkillInt(&Atalanta_T5_SummonTiger_AttackRating[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_AttackRating");
	ReadSkillInt(&Atalanta_T5_SummonTiger_Life[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_Life");
	ReadSkillInt(&Atalanta_T5_SummonTiger_Duration[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_Duration");
	ReadSkillInt(&Atalanta_T5_SummonTiger_UseMana[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_UseMana");
	ReadSkillInt(&Atalanta_T5_GoldenApple_Speed[0], "Atalanta.ini", "Atalanta_T5_GoldenApple_Speed");
	ReadSkillInt(&Atalanta_T5_GoldenApple_Evade[0], "Atalanta.ini", "Atalanta_T5_GoldenApple_Evade");
	ReadSkillInt(&Atalanta_T5_Plague_DamageBoost[0], "Atalanta.ini", "Atalanta_T5_Plague_DamageBoost");
	ReadSkillInt(&Atalanta_T5_Plague_Targets[0], "Atalanta.ini", "Atalanta_T5_Plague_Targets");
	ReadSkillInt(&Atalanta_T5_Plague_PoisonDuration[0], "Atalanta.ini", "Atalanta_T5_Plague_PoisonDuration");
	ReadSkillInt(&Atalanta_T5_Plague_PoisonDamagePercent[0], "Atalanta.ini", "Atalanta_T5_Plague_PoisonDamagePercent");
	ReadSkillInt(&Atalanta_T5_Plague_UseMana[0], "Atalanta.ini", "Atalanta_T5_Plague_UseMana");
	ReadSkillInt(&Atalanta_T5_Combo_DamageBoost[0], "Atalanta.ini", "Atalanta_T5_Combo_DamageBoost");
	ReadSkillInt(&Atalanta_T5_Combo_UseMana[0], "Atalanta.ini", "Atalanta_T5_Combo_UseMana");

	//sacer
	ReadSkillInt(&Healing_UseMana[0], "Sacer.ini", "Healing_UseMana");
	ReadSkillInt2(Healing_Heal, "Sacer.ini", "Healing_Heal");

	ReadSkillInt(&HolyBolt_UseMana[0], "Sacer.ini", "HolyBolt_UseMana");
	ReadSkillInt(&HolyBolt_DamagePercent[0], "Sacer.ini", "HolyBolt_DamagePercent");

	ReadSkillInt(&M_Spark_UseMana[0], "Sacer.ini", "M_Spark_UseMana");
	ReadSkillInt(&M_Spark_Damage_Percent[0], "Sacer.ini", "M_Spark_Damage_Percent");
	ReadSkillInt(&M_Spark_Num[0], "Sacer.ini", "M_Spark_Num");

	ReadSkillInt(&HolyMind_UseMana[0], "Sacer.ini", "HolyMind_UseMana");
	ReadSkillInt(&HolyMind_DecDamage[0], "Sacer.ini", "HolyMind_DecDamage");
	ReadSkillInt(&HolyMind_Time[0], "Sacer.ini", "HolyMind_Time");

	ReadSkillFloat(&Meditation_Regen[0], "Sacer.ini", "Meditation_Regen");

	ReadSkillInt(&Divine_Lightning_Damage_Percent[0], "Sacer.ini", "Divine_Lightning_Damage_Percent");
	ReadSkillInt(&Divine_Lightning_Num[0], "Sacer.ini", "Divine_Lightning_Num");
	ReadSkillInt(&Divine_Lightning_Mana[0], "Sacer.ini", "Divine_Lightning_Mana");

	ReadSkillInt(&Holy_Reflection_Return_Damage[0], "Sacer.ini", "Holy_Reflection_Return_Damage");
	ReadSkillInt(&Holy_Reflection_Time[0], "Sacer.ini", "Holy_Reflection_Time");
	ReadSkillInt(&Holy_Reflection_Mana[0], "Sacer.ini", "Holy_Reflection_Mana");

	ReadSkillInt2(Grand_Healing, "Sacer.ini", "Grand_Healing");
	ReadSkillInt(&Grand_Healing_Range[0], "Sacer.ini", "Grand_Healing_Range");
	ReadSkillInt(&Grand_Healing_Mana[0], "Sacer.ini", "Grand_Healing_Mana");

	ReadSkillInt(&Vigor_Ball_Damage_Percent[0], "Sacer.ini", "Vigor_Ball_Damage_Percent");
	ReadSkillInt(&Vigor_Ball_UseMana[0], "Sacer.ini", "Vigor_Ball_UseMana");

	ReadSkillInt(&Resurrection_Percent[0], "Sacer.ini", "Resurrection_Percent");
	ReadSkillInt(&Resurrection_Exp[0], "Sacer.ini", "Resurrection_Exp");
	ReadSkillInt(&Resurrection_UseMana[0], "Sacer.ini", "Resurrection_UseMana");

	ReadSkillInt(&Extinction_Percent[0], "Sacer.ini", "Extinction_Percent");
	ReadSkillInt(&Extinction_Amount[0], "Sacer.ini", "Extinction_Amount");
	ReadSkillInt(&Extinction_UseMana[0], "Sacer.ini", "Extinction_UseMana");

	ReadSkillInt(&Virtual_Life_Percent[0], "Sacer.ini", "Virtual_Life_Percent");
	ReadSkillInt(&Virtual_Life_Time[0], "Sacer.ini", "Virtual_Life_Time");
	ReadSkillInt(&Virtual_Life_UseMana[0], "Sacer.ini", "Virtual_Life_UseMana");

	ReadSkillInt(&Glacial_Spike_Damage_Percent[0], "Sacer.ini", "Glacial_Spike_Damage_Percent");
	ReadSkillInt(&Glacial_Spike_Area[0], "Sacer.ini", "Glacial_Spike_Area");
	ReadSkillInt(&Glacial_Spike_UseMana[0], "Sacer.ini", "Glacial_Spike_UseMana");

	ReadSkillFloat(&Regeneration_Field_LifeRegen[0], "Sacer.ini", "Regeneration_Field_LifeRegen");
	ReadSkillFloat(&Regeneration_Field_ManaRegen[0], "Sacer.ini", "Regeneration_Field_ManaRegen");
	ReadSkillInt(&Regeneration_Field_Time[0], "Sacer.ini", "Regeneration_Field_Time");
	ReadSkillInt(&Regeneration_Field_Area[0], "Sacer.ini", "Regeneration_Field_Area");
	ReadSkillInt(&Regeneration_Field_UseMana[0], "Sacer.ini", "Regeneration_Field_UseMana");

	ReadSkillInt(&Chain_Lightning_Damage_Percent[0], "Sacer.ini", "Chain_Lightning_Damage_Percent");
	ReadSkillInt(&Chain_Lightning_Num[0], "Sacer.ini", "Chain_Lightning_Num");
	ReadSkillInt(&Chain_Lightning_Range[0], "Sacer.ini", "Chain_Lightning_Range");
	ReadSkillInt(&Chain_Lightning_UseMana[0], "Sacer.ini", "Chain_Lightning_UseMana");

	ReadSkillInt2(Summon_Muspell_Damage, "Sacer.ini", "Summon_Muspell_Damage");
	ReadSkillInt(&Summon_Muspell_Attack_Delay[0], "Sacer.ini", "Summon_Muspell_Attack_Delay");
	ReadSkillInt(&Summon_Muspell_UndeadAbsorbPercent[0], "Sacer.ini", "Summon_Muspell_UndeadAbsorbPercent");
	ReadSkillInt(&Summon_Muspell_BlockPercent[0], "Sacer.ini", "Summon_Muspell_BlockPercent");
	ReadSkillInt(&Summon_Muspell_Time[0], "Sacer.ini", "Summon_Muspell_Time");
	ReadSkillInt(&Summon_Muspell_UseMana[0], "Sacer.ini", "Summon_Muspell_UseMana");

	ReadSkillInt(&Priestess_T5_IceElemental_AttackPower[0], "Sacer.ini", "Priestess_T5_IceElemental_AttackPower");
	ReadSkillInt(&Priestess_T5_IceElemental_AttackRating[0], "Sacer.ini", "Priestess_T5_IceElemental_AttackRating");
	ReadSkillInt(&Priestess_T5_IceElemental_Life[0], "Sacer.ini", "Priestess_T5_IceElemental_Life");
	ReadSkillInt(&Priestess_T5_IceElemental_Duration[0], "Sacer.ini", "Priestess_T5_IceElemental_Duration");
	ReadSkillInt(&Priestess_T5_IceElemental_UseMana[0], "Sacer.ini", "Priestess_T5_IceElemental_UseMana");
	ReadSkillInt(&Priestess_T5_LightningSurge_DamageBoost[0], "Sacer.ini", "Priestess_T5_LightningSurge_DamageBoost");
	ReadSkillInt(&Priestess_T5_LightningSurge_Area[0], "Sacer.ini", "Priestess_T5_LightningSurge_Area");
	ReadSkillInt(&Priestess_T5_LightningSurge_ChargedArea[0], "Sacer.ini", "Priestess_T5_LightningSurge_ChargedArea");
	ReadSkillInt(&Priestess_T5_LightningSurge_UseMana[0], "Sacer.ini", "Priestess_T5_LightningSurge_UseMana");
	ReadSkillInt(&Priestess_T5_HeavenlyLight_AttackRateReduction[0], "Sacer.ini", "Priestess_T5_HeavenlyLight_AttackRateReduction");
	ReadSkillInt(&Priestess_T5_HeavenlyLight_Duration[0], "Sacer.ini", "Priestess_T5_HeavenlyLight_Duration");
	ReadSkillInt(&Priestess_T5_HeavenlyLight_Area[0], "Sacer.ini", "Priestess_T5_HeavenlyLight_Area");
	ReadSkillInt(&Priestess_T5_HeavenlyLight_UseMana[0], "Sacer.ini", "Priestess_T5_HeavenlyLight_UseMana");
	ReadSkillInt(&Priestess_T5_Consecration_DefenseBoost[0], "Sacer.ini", "Priestess_T5_Consecration_DefenseBoost");
	ReadSkillInt(&Priestess_T5_Consecration_Absorption[0], "Sacer.ini", "Priestess_T5_Consecration_Absorption");
	ReadSkillInt(&Priestess_T5_Consecration_UseMana[0], "Sacer.ini", "Priestess_T5_Consecration_UseMana");

	ReadSkillInt(&Agony_UseMana[0], "Mage.ini", "Agony_UseMana");
	ReadSkillInt(&Agony_ConvLife[0], "Mage.ini", "Agony_ConvLife");
	ReadSkillInt(&Agony_RecoveryMana[0], "Mage.ini", "Agony_RecoveryMana");

	ReadSkillInt(&FireBolt_UseMana[0], "Mage.ini", "FireBolt_UseMana");
	ReadSkillInt(&FireBolt_DamagePercent[0], "Mage.ini", "FireBolt_DamagePercent");

	ReadSkillInt(&Zenith_UseMana[0], "Mage.ini", "Zenith_UseMana");
	ReadSkillInt(&Zenith_Element[0], "Mage.ini", "Zenith_Element");
	ReadSkillInt(&Zenith_Time[0], "Mage.ini", "Zenith_Time");

	ReadSkillInt(&FireBall_UseMana[0], "Mage.ini", "FireBall_UseMana");
	ReadSkillInt(&FireBall_DamagePercent[0], "Mage.ini", "FireBall_DamagePercent");
	ReadSkillInt(&FireBall_Area[0], "Mage.ini", "FireBall_Area");
	ReadSkillInt(&FireBall_Range[0], "Mage.ini", "FireBall_Range");

	ReadSkillInt(&Mental_Mastery_Mana[0], "Mage.ini", "Mental_Mastery_Mana");

	ReadSkillInt(&Watornado_DamagePercent[0], "Mage.ini", "Watornado_DamagePercent");
	ReadSkillInt(&Watornado_Area[0], "Mage.ini", "Watornado_Area");
	ReadSkillInt(&Watornado_Range[0], "Mage.ini", "Watornado_Range");
	ReadSkillInt(&Watornado_Mana[0], "Mage.ini", "Watornado_Mana");

	ReadSkillInt2(Enchant_Weapon_Damage_Ice, "Mage.ini", "Enchant_Weapon_Damage_Ice");
	ReadSkillInt2(Enchant_Weapon_Damage_Fire, "Mage.ini", "Enchant_Weapon_Damage_Fire");
	ReadSkillInt2(Enchant_Weapon_Damage_Lightning, "Mage.ini", "Enchant_Weapon_Damage_Lightning");
	ReadSkillInt(&Enchant_Weapon_Damage[0], "Mage.ini", "Enchant_Weapon_Damage");
	ReadSkillInt(&Enchant_Weapon_Time[0], "Mage.ini", "Enchant_Weapon_Time");
	ReadSkillInt(&Enchant_Weapon_Mana[0], "Mage.ini", "Enchant_Weapon_Mana");

	ReadSkillInt(&Dead_Ray_Damage_Percent[0], "Mage.ini", "Dead_Ray_Damage_Percent");
	ReadSkillInt(&Dead_Ray_Range[0], "Mage.ini", "Dead_Ray_Range");
	ReadSkillInt(&Dead_Ray_Mana[0], "Mage.ini", "Dead_Ray_Mana");

	ReadSkillInt(&Energy_Shield_DecDamage[0], "Mage.ini", "Energy_Shield_DecDamage");
	ReadSkillInt(&Energy_Shield_Time[0], "Mage.ini", "Energy_Shield_Time");
	ReadSkillInt(&Energy_Shield_UseMana[0], "Mage.ini", "Energy_Shield_UseMana");

	ReadSkillInt(&Diastrophism_Damage_Percent[0], "Mage.ini", "Diastrophism_Damage_Percent");
	ReadSkillInt(&Diastrophism_Area[0], "Mage.ini", "Diastrophism_Area");
	ReadSkillInt(&Diastrophism_UseMana[0], "Mage.ini", "Diastrophism_UseMana");

	ReadSkillFloat(&Spirit_Elemental_RegenMana[0], "Mage.ini", "Spirit_Elemental_RegenMana");
	ReadSkillInt(&Spirit_Elemental_Damage[0], "Mage.ini", "Spirit_Elemental_Damage");
	ReadSkillInt(&Spirit_Elemental_Time[0], "Mage.ini", "Spirit_Elemental_Time");
	ReadSkillInt(&Spirit_Elemental_UseMana[0], "Mage.ini", "Spirit_Elemental_UseMana");

	ReadSkillInt2(Dancing_Sword_IceDamage, "Mage.ini", "Dancing_Sword_IceDamage");
	ReadSkillInt2(Dancing_Sword_FireDamage, "Mage.ini", "Dancing_Sword_FireDamage");
	ReadSkillInt2(Dancing_Sword_Damage, "Mage.ini", "Dancing_Sword_Damage");
	ReadSkillInt(&Dancing_Sword_AttackDelay[0], "Mage.ini", "Dancing_Sword_AttackDelay");
	ReadSkillInt(&Dancing_Sword_Time[0], "Mage.ini", "Dancing_Sword_Time");
	ReadSkillInt(&Dancing_Sword_UseMana[0], "Mage.ini", "Dancing_Sword_UseMana");

	ReadSkillInt2(Fire_Elemental_Damage, "Mage.ini", "Fire_Elemental_Damage");
	ReadSkillInt(&Fire_Elemental_Hit[0], "Mage.ini", "Fire_Elemental_Hit");
	ReadSkillInt(&Fire_Elemental_Life[0], "Mage.ini", "Fire_Elemental_Life");
	ReadSkillInt(&Fire_Elemental_UseMana[0], "Mage.ini", "Fire_Elemental_UseMana");

	ReadSkillInt(&Flame_Wave_FireDamage_Percent[0], "Mage.ini", "Flame_Wave_FireDamage_Percent");
	ReadSkillInt(&Flame_Wave_Area[0], "Mage.ini", "Flame_Wave_Area");
	ReadSkillInt(&Flame_Wave_UseMana[0], "Mage.ini", "Flame_Wave_UseMana");

	ReadSkillInt(&Distortion_SpeedSubPercent[0], "Mage.ini", "Distortion_SpeedSubPercent");
	ReadSkillInt(&Distortion_DamageSubPercent[0], "Mage.ini", "Distortion_DamageSubPercent");
	ReadSkillInt(&Distortion_Area[0], "Mage.ini", "Distortion_Area");
	ReadSkillInt(&Distortion_Time[0], "Mage.ini", "Distortion_Time");
	ReadSkillInt(&Distortion_UseMana[0], "Mage.ini", "Distortion_UseMana");

	ReadSkillInt(&Meteo_Damage_Percent[0], "Mage.ini", "Meteo_Damage_Percent");
	ReadSkillInt(&Meteo_Area[0], "Mage.ini", "Meteo_Area");
	ReadSkillInt(&Meteo_UseMana[0], "Mage.ini", "Meteo_UseMana");

	ReadSkillInt(&Magician_T5_WizardTrance_BlockBoost[0], "Mage.ini", "Magician_T5_WizardTrance_BlockBoost");
	ReadSkillInt(&Magician_T5_WizardTrance_AttackRateReduction[0], "Mage.ini", "Magician_T5_WizardTrance_AttackRateReduction");
	ReadSkillInt(&Magician_T5_WizardTrance_Duration[0], "Mage.ini", "Magician_T5_WizardTrance_Duration");
	ReadSkillInt(&Magician_T5_WizardTrance_UseMana[0], "Mage.ini", "Magician_T5_WizardTrance_UseMana");
	ReadSkillInt(&Magician_T5_StoneSkin_ImmortalWindow[0], "Mage.ini", "Magician_T5_StoneSkin_ImmortalWindow");
	ReadSkillInt(&Magician_T5_StoneSkin_Duration[0], "Mage.ini", "Magician_T5_StoneSkin_Duration");
	ReadSkillInt(&Magician_T5_StoneSkin_RunSpeedReduction[0], "Mage.ini", "Magician_T5_StoneSkin_RunSpeedReduction");
	ReadSkillInt(&Magician_T5_StoneSkin_UseMana[0], "Mage.ini", "Magician_T5_StoneSkin_UseMana");
	ReadSkillInt(&Magician_T5_RedRay_DamageBoost[0], "Mage.ini", "Magician_T5_RedRay_DamageBoost");
	ReadSkillInt(&Magician_T5_RedRay_AddRange[0], "Mage.ini", "Magician_T5_RedRay_AddRange");
	ReadSkillInt(&Magician_T5_RedRay_UseMana[0], "Mage.ini", "Magician_T5_RedRay_UseMana");
	ReadSkillInt(&Magician_T5_Cataclysm_DamageBoost[0], "Mage.ini", "Magician_T5_Cataclysm_DamageBoost");
	ReadSkillInt(&Magician_T5_Cataclysm_Duration[0], "Mage.ini", "Magician_T5_Cataclysm_Duration");
	ReadSkillInt(&Magician_T5_Cataclysm_MPDrain[0], "Mage.ini", "Magician_T5_Cataclysm_MPDrain");
	ReadSkillInt(&Magician_T5_Cataclysm_SPDrain[0], "Mage.ini", "Magician_T5_Cataclysm_SPDrain");
	ReadSkillInt(&Magician_T5_Cataclysm_UseMana[0], "Mage.ini", "Magician_T5_Cataclysm_UseMana");


#endif

	return TRUE;
}

int ReLoadSkillInfo(rsPLAYINFO* lpPlayInfo)
{

#ifdef _W_SERVER


	MECH_INFO MechSkills;
	FIGHTER_INFO FighterSkills;
	PIKE_INFO PikeSkills;
	ZeroMemory(&PikeSkills, sizeof(PikeSkills));
	ARCHER_INFO ArcherSkills;

	KINA_INFO KinaSkills;
	ATA_INFO AtaSkills;
	SACER_INFO SacerSkills;
	MAGE_INFO MageSkills;
	ZeroMemory(&KinaSkills, sizeof(KinaSkills));
	ZeroMemory(&AtaSkills, sizeof(AtaSkills));
	ZeroMemory(&SacerSkills, sizeof(SacerSkills));
	ZeroMemory(&MageSkills, sizeof(MageSkills));


	ReadSkillInt(&MechSkills.E_Shield_BlockRate[0], "Mech.ini", "E_Shield_BlockRate");
	ReadSkillInt(&MechSkills.E_Shield_UseTime[0], "Mech.ini", "E_Shield_UseTime");
	ReadSkillInt(&MechSkills.E_Shield_UseMana[0], "Mech.ini", "E_Shield_UseMana");

	ReadSkillInt(&MechSkills.Mechanic_Bomb_DamagePercent[0], "Mech.ini", "Mechanic_Bomb_DamagePercent");
	ReadSkillInt(&MechSkills.Mechanic_Bomb_Attack_Range[0], "Mech.ini", "Mechanic_Bomb_Attack_Range");
	ReadSkillInt(&MechSkills.Mechanic_Bomb_UseMana[0], "Mech.ini", "Mechanic_Bomb_UseMana");

	ReadSkillInt2(MechSkills.P_Absorb, "Mech.ini", "P_Absorb");
	ReadSkillInt(&MechSkills.P_Absorb_UseTime[0], "Mech.ini", "P_Absorb_UseTime");
	ReadSkillInt(&MechSkills.P_Absor_bUseMana[0], "Mech.ini", "P_Absor_bUseMana");

	ReadSkillInt(&MechSkills.PlusPoison[0], "Mech.ini", "PlusPoison");

	ReadSkillInt(&MechSkills.G_Smash_Attack_Rate[0], "Mech.ini", "G_Smash_Attack_Rate");
	ReadSkillInt(&MechSkills.G_Smash_Attack_RagePercent[0], "Mech.ini", "G_Smash_Attack_RagePercent");
	ReadSkillInt(&MechSkills.G_Smash_UseMana[0], "Mech.ini", "G_Smash_UseMana");

	ReadSkillInt(&MechSkills.Maximize_Damage[0], "Mech.ini", "Maximize_Damage");
	ReadSkillInt(&MechSkills.Maximize_Time[0], "Mech.ini", "Maximize_Time");
	ReadSkillInt(&MechSkills.Maximize_UseMana[0], "Mech.ini", "Maximize_UseMana");

	ReadSkillInt(&MechSkills.Automation_Damage[0], "Mech.ini", "Automation_Damage");
	ReadSkillInt(&MechSkills.Automation_Speed[0], "Mech.ini", "Automation_Speed");
	ReadSkillInt(&MechSkills.Automation_Time[0], "Mech.ini", "Automation_Time");
	ReadSkillInt(&MechSkills.Automation_UseMana[0], "Mech.ini", "Automation_UseMana");

	ReadSkillInt(&MechSkills.Spark_DamagePercent[0], "Mech.ini", "Spark_DamagePercent");
	ReadSkillInt(&MechSkills.Spark_Area[0], "Mech.ini", "Spark_Area");
	ReadSkillInt(&MechSkills.Spark_UseMana[0], "Mech.ini", "Spark_UseMana");

	ReadSkillInt(&MechSkills.Metal_Armor_Defense[0], "Mech.ini", "Metal_Armor_Defense");
	ReadSkillInt(&MechSkills.Metal_Armor_Time[0], "Mech.ini", "Metal_Armor_Time");
	ReadSkillInt(&MechSkills.Metal_Armor_UseMana[0], "Mech.ini", "Metal_Armor_UseMana");

	ReadSkillInt(&MechSkills.Grand_Smash_AttackRate[0], "Mech.ini", "Grand_Smash_AttackRate");
	ReadSkillInt(&MechSkills.Grand_Smash_Damage[0], "Mech.ini", "Grand_Smash_Damage");
	ReadSkillInt(&MechSkills.Grand_Smash_UseMana[0], "Mech.ini", "Grand_Smash_UseMana");

	ReadSkillInt(&MechSkills.M_Weapon_Mastey[0], "Mech.ini", "M_Weapon_Mastey");

	ReadSkillInt(&MechSkills.Spark_Shield_Damage[0], "Mech.ini", "Spark_Shield_Damage");
	ReadSkillInt(&MechSkills.Spark_Shield_Defense[0], "Mech.ini", "Spark_Shield_Defense");
	ReadSkillInt(&MechSkills.Spark_Shield_Time[0], "Mech.ini", "Spark_Shield_Time");
	ReadSkillInt(&MechSkills.Spark_Shield_UseMana[0], "Mech.ini", "Spark_Shield_UseMana");

	ReadSkillInt(&MechSkills.Impulsion_LightingDamagePercent[0], "Mech.ini", "Impulsion_LightingDamagePercent");
	ReadSkillInt(&MechSkills.Impulsion_Range[0], "Mech.ini", "Impulsion_Range");
	ReadSkillInt(&MechSkills.Impulsion_LightingNum[0], "Mech.ini", "Impulsion_LightingNum");
	ReadSkillInt(&MechSkills.Impulsion_UseMana[0], "Mech.ini", "Impulsion_UseMana");

	ReadSkillInt(&MechSkills.Compulsion_AddAbsorb[0], "Mech.ini", "Compulsion_AddAbsorb");
	ReadSkillInt(&MechSkills.Compulsion_Area[0], "Mech.ini", "Compulsion_Area");
	ReadSkillInt(&MechSkills.Compulsion_Time[0], "Mech.ini", "Compulsion_Time");
	ReadSkillInt(&MechSkills.Compulsion_UseMana[0], "Mech.ini", "Compulsion_UseMana");

	ReadSkillInt(&MechSkills.Magnetic_Sphere_AddDamagePercent[0], "Mech.ini", "Magnetic_Sphere_AddDamagePercent");
	ReadSkillInt(&MechSkills.Magnetic_Sphere_Area[0], "Mech.ini", "Magnetic_Sphere_Area");
	ReadSkillInt(&MechSkills.Magnetic_Sphere_AttackDelay[0], "Mech.ini", "Magnetic_Sphere_AttackDelay");
	ReadSkillInt(&MechSkills.Magnetic_Sphere_Time[0], "Mech.ini", "Magnetic_Sphere_Time");
	ReadSkillInt(&MechSkills.Magnetic_Sphere_UseMana[0], "Mech.ini", "Magnetic_Sphere_UseMana");

	ReadSkillInt2(MechSkills.Metal_Golem_Damage, "Mech.ini", "Metal_Golem_Damage");
	ReadSkillInt(&MechSkills.Metal_Golem_Hit[0], "Mech.ini", "Metal_Golem_Hit");
	ReadSkillInt(&MechSkills.Metal_Golem_Defense[0], "Mech.ini", "Metal_Golem_Defense");
	ReadSkillInt(&MechSkills.Metal_Golem_Life[0], "Mech.ini", "Metal_Golem_Life");
	ReadSkillInt(&MechSkills.Metal_Golem_UseMana[0], "Mech.ini", "Metal_Golem_UseMana");

	memcpy(MechSkills.Rupture_DamageBoost, Rupture_DamageBoost, sizeof(Rupture_DamageBoost));
	memcpy(MechSkills.Rupture_UseMana, Rupture_UseMana, sizeof(Rupture_UseMana));
	memcpy(MechSkills.Parasit_Shot_DamageBoost, Parasit_Shot_DamageBoost, sizeof(Parasit_Shot_DamageBoost));
	memcpy(MechSkills.Parasit_Shot_PoisonDuration, Parasit_Shot_PoisonDuration, sizeof(Parasit_Shot_PoisonDuration));
	memcpy(MechSkills.Parasit_Shot_PoisonDamagePercent, Parasit_Shot_PoisonDamagePercent, sizeof(Parasit_Shot_PoisonDamagePercent));
	memcpy(MechSkills.Parasit_Shot_UseMana, Parasit_Shot_UseMana, sizeof(Parasit_Shot_UseMana));
	memcpy(MechSkills.Hardening_Shield_DamageReduction, Hardening_Shield_DamageReduction, sizeof(Hardening_Shield_DamageReduction));
	memcpy(MechSkills.Hardening_Shield_AttackRateReduction, Hardening_Shield_AttackRateReduction, sizeof(Hardening_Shield_AttackRateReduction));
	memcpy(MechSkills.Hardening_Shield_Time, Hardening_Shield_Time, sizeof(Hardening_Shield_Time));
	memcpy(MechSkills.Hardening_Shield_UseMana, Hardening_Shield_UseMana, sizeof(Hardening_Shield_UseMana));
	memcpy(MechSkills.Magnetic_Discharge_DamageBoost, Magnetic_Discharge_DamageBoost, sizeof(Magnetic_Discharge_DamageBoost));
	memcpy(MechSkills.Magnetic_Discharge_UseMana, Magnetic_Discharge_UseMana, sizeof(Magnetic_Discharge_UseMana));
	ReadSkillInt(&MechSkills.Rupture_DamageBoost[0], "Mech.ini", "Rupture_DamageBoost");
	ReadSkillInt(&MechSkills.Rupture_UseMana[0], "Mech.ini", "Rupture_UseMana");
	ReadSkillInt(&MechSkills.Parasit_Shot_DamageBoost[0], "Mech.ini", "Parasit_Shot_DamageBoost");
	ReadSkillInt(&MechSkills.Parasit_Shot_PoisonDuration[0], "Mech.ini", "Parasit_Shot_PoisonDuration");
	ReadSkillInt(&MechSkills.Parasit_Shot_PoisonDamagePercent[0], "Mech.ini", "Parasit_Shot_PoisonDamagePercent");
	ReadSkillInt(&MechSkills.Parasit_Shot_UseMana[0], "Mech.ini", "Parasit_Shot_UseMana");
	ReadSkillInt(&MechSkills.Hardening_Shield_DamageReduction[0], "Mech.ini", "Hardening_Shield_DamageReduction");
	ReadSkillInt(&MechSkills.Hardening_Shield_AttackRateReduction[0], "Mech.ini", "Hardening_Shield_AttackRateReduction");
	ReadSkillInt(&MechSkills.Hardening_Shield_Time[0], "Mech.ini", "Hardening_Shield_time");
	ReadSkillInt(&MechSkills.Hardening_Shield_UseMana[0], "Mech.ini", "Hardening_Shield_UseMana");
	ReadSkillInt(&MechSkills.Magnetic_Discharge_DamageBoost[0], "Mech.ini", "Magnetic_Discharge_DamageBoost");
	ReadSkillInt(&MechSkills.Magnetic_Discharge_UseMana[0], "Mech.ini", "Magnetic_Discharge_UseMana");

	ReadSkillInt(&FighterSkills.Melee_Mastery_DamagePercent[0], "Fighter.ini", "Melee_Mastery_DamagePercent");

	ReadSkillInt(&FighterSkills.PlusFire[0], "Fighter.ini", "PlusFire");

	ReadSkillInt(&FighterSkills.Raving_Damage[0], "Fighter.ini", "Raving_Damage");
	ReadSkillInt(&FighterSkills.Ravind_Speed[0], "Fighter.ini", "Ravind_Speed");
	ReadSkillFloat(&FighterSkills.Raving_UseLife[0], "Fighter.ini", "Raving_UseLife");
	ReadSkillInt(&FighterSkills.Raving_UseMana[0], "Fighter.ini", "Raving_UseMana");

	ReadSkillInt(&FighterSkills.Impact_Attack_Rating[0], "Fighter.ini", "Impact_Attack_Rating");
	ReadSkillInt(&FighterSkills.Impact_Damage[0], "Fighter.ini", "Impact_Damage");
	ReadSkillInt(&FighterSkills.Impact_UseMana[0], "Fighter.ini", "Impact_UseMana");

	ReadSkillInt(&FighterSkills.T_Impact_Damage[0], "Fighter.ini", "T_Impact_Damage");
	ReadSkillInt(&FighterSkills.T_Impact_Hit[0], "Fighter.ini", "T_Impact_Hit");
	ReadSkillInt(&FighterSkills.T_Impact_UseMana[0], "Fighter.ini", "T_Impact_UseMana");

	ReadSkillInt(&FighterSkills.B_Swing_Damage[0], "Fighter.ini", "B_Swing_Damage");
	ReadSkillInt(&FighterSkills.B_Swing_Critical[0], "Fighter.ini", "B_Swing_Critical");
	ReadSkillInt(&FighterSkills.B_Swing_UseMana[0], "Fighter.ini", "B_Swing_UseMana");

	ReadSkillInt(&FighterSkills.Roar_Range[0], "Fighter.ini", "Roar_Range");
	ReadSkillInt(&FighterSkills.Roar_Time[0], "Fighter.ini", "Roar_Time");
	ReadSkillInt(&FighterSkills.Roar_UseMana[0], "Fighter.ini", "Roar_UseMana");

	ReadSkillInt(&FighterSkills.R_Zecram_Damage[0], "Fighter.ini", "R_Zecram_Damage");
	ReadSkillInt(&FighterSkills.R_Zecram_UseMana[0], "Fighter.ini", "R_Zecram_UseMana");

	ReadSkillInt(&FighterSkills.Concentration_AttackRate[0], "Fighter.ini", "Concentration_AttackRate");
	ReadSkillInt(&FighterSkills.Concentration_Time[0], "Fighter.ini", "Concentration_Time");
	ReadSkillInt(&FighterSkills.Concentration_UseMana[0], "Fighter.ini", "Concentration_UseMana");

	ReadSkillInt(&FighterSkills.A_Crash_Damage[0], "Fighter.ini", "A_Crash_Damage");
	ReadSkillInt(&FighterSkills.A_Crash_AttackRate[0], "Fighter.ini", "A_Crash_AttackRate");
	ReadSkillInt(&FighterSkills.A_Crash_UseMana[0], "Fighter.ini", "A_Crash_UseMana");

	ReadSkillInt(&FighterSkills.Swift_Axe_Speed[0], "Fighter.ini", "Swift_Axe_Speed");
	ReadSkillInt(&FighterSkills.Swift_Axe_Time[0], "Fighter.ini", "Swift_Axe_Time");
	ReadSkillInt(&FighterSkills.Swift_Axe_UseMana[0], "Fighter.ini", "Swift_Axe_UseMana");

	ReadSkillInt(&FighterSkills.B_Crash_Damage[0], "Fighter.ini", "B_Crash_Damage");
	ReadSkillInt(&FighterSkills.B_Crash_DemonDamage[0], "Fighter.ini", "B_Crash_DemonDamage");
	ReadSkillInt(&FighterSkills.B_Crash_UseMana[0], "Fighter.ini", "B_Crash_UseMana");


	ReadSkillInt(&FighterSkills.Destoryer_DamagePercent[0], "Fighter.ini", "Destoryer_DamagePercent");
	ReadSkillInt(&FighterSkills.Destoryer_AddCritical[0], "Fighter.ini", "Destoryer_AddCritical");
	ReadSkillInt(&FighterSkills.Destoryer_UseMana[0], "Fighter.ini", "Destoryer_UseMana");


	ReadSkillInt(&FighterSkills.Berserker_AddAttack[0], "Fighter.ini", "Berserker_AddAttack");
	ReadSkillInt(&FighterSkills.Berserker_SubAbsorb[0], "Fighter.ini", "Berserker_SubAbsorb");
	ReadSkillInt(&FighterSkills.Berserker_Time[0], "Fighter.ini", "Berserker_Time");
	ReadSkillInt(&FighterSkills.Berserker_UseMana[0], "Fighter.ini", "Berserker_UseMana");

	ReadSkillInt(&FighterSkills.Cyclone_Strike_DamagePercent[0], "Fighter.ini", "Cyclone_Strike_DamagePercent");
	ReadSkillInt2(FighterSkills.Cyclone_Strike_AreaDamage, "Fighter.ini", "Cyclone_Strike_AreaDamage");
	ReadSkillInt(&FighterSkills.Cyclone_Strike_AttackNum[0], "Fighter.ini", "Cyclone_Strike_AttackNum");
	ReadSkillInt(&FighterSkills.Cyclone_Strike_Area[0], "Fighter.ini", "Cyclone_Strike_Area");
	ReadSkillInt(&FighterSkills.Cyclone_Strike_UseMana[0], "Fighter.ini", "Cyclone_Strike_UseMana");

	ReadSkillInt(&FighterSkills.Boost_Health_Life[0], "Fighter.ini", "Boost_Health_Life");
	ReadSkillInt(&FighterSkills.Fighter_T5_Survival_Life[0], "Fighter.ini", "Fighter_T5_Survival_Life");
	ReadSkillInt(&FighterSkills.Fighter_T5_Survival_Regen[0], "Fighter.ini", "Fighter_T5_Survival_Regen");
	ReadSkillInt(&FighterSkills.Fighter_T5_Seismic_Damage[0], "Fighter.ini", "Fighter_T5_Seismic_Damage");
	ReadSkillInt(&FighterSkills.Fighter_T5_Seismic_Area[0], "Fighter.ini", "Fighter_T5_Seismic_Area");
	ReadSkillInt(&FighterSkills.Fighter_T5_Seismic_UseMana[0], "Fighter.ini", "Fighter_T5_Seismic_UseMana");
	ReadSkillFloat(&FighterSkills.Fighter_T5_Ruthless_HPDecrease[0], "Fighter.ini", "Fighter_T5_Ruthless_HPDecrease");
	ReadSkillInt(&FighterSkills.Fighter_T5_Ruthless_HitsBoost[0], "Fighter.ini", "Fighter_T5_Ruthless_HitsBoost");
	ReadSkillInt(&FighterSkills.Fighter_T5_Ruthless_DamageBoost[0], "Fighter.ini", "Fighter_T5_Ruthless_DamageBoost");
	ReadSkillInt(&FighterSkills.Fighter_T5_Ruthless_UseMana[0], "Fighter.ini", "Fighter_T5_Ruthless_UseMana");
	ReadSkillInt(&FighterSkills.Fighter_T5_Whirlwind_DamageBoost[0], "Fighter.ini", "Fighter_T5_Whirlwind_DamageBoost");
	ReadSkillInt(&FighterSkills.Fighter_T5_Whirlwind_Duration[0], "Fighter.ini", "Fighter_T5_Whirlwind_Duration");
	ReadSkillInt(&FighterSkills.Fighter_T5_Whirlwind_MPDrain[0], "Fighter.ini", "Fighter_T5_Whirlwind_MPDrain");
	ReadSkillInt(&FighterSkills.Fighter_T5_Whirlwind_SPDrain[0], "Fighter.ini", "Fighter_T5_Whirlwind_SPDrain");

	ReadSkillInt(&PikeSkills.Pike_Wind_Throw_Height[0], "Pike.ini", "Pike_Wind_Throw_Height");
	ReadSkillInt(&PikeSkills.Pike_Wind_DamagePercent[0], "Pike.ini", "Pike_Wind_DamagePercent");
	ReadSkillInt(&PikeSkills.Pike_Wind_Push_Lenght[0], "Pike.ini", "Pike_Wind_Push_Lenght");
	ReadSkillInt(&PikeSkills.Pike_Wind_UseMana[0], "Pike.ini", "Pike_Wind_UseMana");

	ReadSkillInt(&PikeSkills.PlusIce[0], "Pike.ini", "PlusIce");

	ReadSkillInt(&PikeSkills.Critical_Hit_Critical[0], "Pike.ini", "Critical_Hit_Critical");
	ReadSkillInt(&PikeSkills.Critical_Hit_UseMana[0], "Pike.ini", "Critical_Hit_UseMana");

	ReadSkillInt(&PikeSkills.Jumping_Crash_UseMana[0], "Pike.ini", "Jumping_Crash_UseMana");
	ReadSkillInt(&PikeSkills.Jumping_Crash_Attack_Rating[0], "Pike.ini", "Jumping_Crash_Attack_Rating");
	ReadSkillInt(&PikeSkills.Jumping_Crash_Damage[0], "Pike.ini", "Jumping_Crash_Damage");

	ReadSkillInt(&PikeSkills.G_Pike_DamagePercent[0], "Pike.ini", "G_Pike_DamagePercent");
	ReadSkillInt(&PikeSkills.G_Pike_Range[0], "Pike.ini", "G_Pike_Range");
	ReadSkillInt(&PikeSkills.G_Pike_UseMana[0], "Pike.ini", "G_Pike_UseMana");
	ReadSkillInt(&PikeSkills.G_Pike_Time[0], "Pike.ini", "G_Pike_Time");

	ReadSkillInt(&PikeSkills.Tornado_Damage[0], "Pike.ini", "Tornado_Damage");
	ReadSkillInt(&PikeSkills.Tornado_Stun[0], "Pike.ini", "Tornado_Stun");
	ReadSkillInt(&PikeSkills.Tornado_Range[0], "Pike.ini", "Tornado_Range");
	ReadSkillInt(&PikeSkills.Tornado_UseMana[0], "Pike.ini", "Tornado_UseMana");

	ReadSkillInt(&PikeSkills.W_D_Mastery_Block[0], "Pike.ini", "W_D_Mastery_Block");

	ReadSkillInt(&PikeSkills.Expansion_Size[0], "Pike.ini", "Expansion_Size");
	ReadSkillInt(&PikeSkills.Expansion_Damage[0], "Pike.ini", "Expansion_Damage");
	ReadSkillInt(&PikeSkills.Expansion_UseMana[0], "Pike.ini", "Expansion_UseMana");

	ReadSkillInt(&PikeSkills.VenomSpear_DamagePercent[0], "Pike.ini", "VenomSpear_DamagePercent");
	ReadSkillInt(&PikeSkills.VenomSpear_Num[0], "Pike.ini", "VenomSpear_Num");
	ReadSkillInt(&PikeSkills.VenomSpear_Time[0], "Pike.ini", "VenomSpear_Time");
	ReadSkillInt(&PikeSkills.VenomSpear_Range[0], "Pike.ini", "VenomSpear_Range");
	ReadSkillInt(&PikeSkills.VenomSpear_UseMana[0], "Pike.ini", "VenomSpear_UseMana");

	ReadSkillFloat(&PikeSkills.Vanish_Speed[0], "Pike.ini", "Vanish_Speed");
	ReadSkillInt(&PikeSkills.Vanish_Damage[0], "Pike.ini", "Vanish_Damage");
	ReadSkillInt(&PikeSkills.Vanish_Range[0], "Pike.ini", "Vanish_Range");
	ReadSkillInt(&PikeSkills.Vanish_Time[0], "Pike.ini", "Vanish_Time");
	ReadSkillInt(&PikeSkills.Vanish_UseMana[0], "Pike.ini", "Vanish_UseMana");

	ReadSkillInt(&PikeSkills.Critical_Mastery_Critical[0], "Pike.ini", "Critical_Mastery_Critical");

	ReadSkillInt(&PikeSkills.Chain_Lance_Damage[0], "Pike.ini", "Chain_Lance_Damage");
	ReadSkillInt(&PikeSkills.Chain_Lance_UseMana[0], "Pike.ini", "Chain_Lance_UseMana");

	ReadSkillInt(&PikeSkills.Assassin_Eye_AddCritical[0], "Pike.ini", "Assassin_Eye_AddCritical");
	ReadSkillInt(&PikeSkills.Assassin_Eye_M_SubCritical[0], "Pike.ini", "Assassin_Eye_M_SubCritical");
	ReadSkillInt(&PikeSkills.Assassin_Eye_Time[0], "Pike.ini", "Assassin_Eye_Time");
	ReadSkillInt(&PikeSkills.Assassin_Eye_UseMana[0], "Pike.ini", "Assassin_Eye_UseMana");

	ReadSkillInt(&PikeSkills.Charging_Strike_DamagePercent[0], "Pike.ini", "Charging_Strike_DamagePercent");
	ReadSkillInt(&PikeSkills.Charging_Strike_CharingDamagePercent[0], "Pike.ini", "Charging_Strike_CharingDamagePercent");
	ReadSkillInt(&PikeSkills.Charging_Strike_Time[0], "Pike.ini", "Charging_Strike_Time");
	ReadSkillInt(&PikeSkills.Charging_Strike_UseMana[0], "Pike.ini", "Charging_Strike_UseMana");

	ReadSkillInt(&PikeSkills.Vague_EvasionPercent[0], "Pike.ini", "Vague_EvasionPercent");
	ReadSkillInt(&PikeSkills.Vague_Time[0], "Pike.ini", "Vague_Time");
	ReadSkillInt(&PikeSkills.Vague_UseMana[0], "Pike.ini", "Vague_UseMana");

	ReadSkillInt(&PikeSkills.Shadow_Master_DamagePercent[0], "Pike.ini", "Shadow_Master_DamagePercent");
	ReadSkillInt(&PikeSkills.Shadow_Master_ShadowNum[0], "Pike.ini", "Shadow_Master_ShadowNum");
	ReadSkillInt(&PikeSkills.Shadow_Master_UseMana[0], "Pike.ini", "Shadow_Master_UseMana");

	memcpy(PikeSkills.Pike_T5_Ring_Duration, Pike_T5_Ring_Duration, sizeof(Pike_T5_Ring_Duration));
	memcpy(PikeSkills.Pike_T5_Ring_UseMana, Pike_T5_Ring_UseMana, sizeof(Pike_T5_Ring_UseMana));
	memcpy(PikeSkills.Pike_T5_Focus_MinAttackReduction, Pike_T5_Focus_MinAttackReduction, sizeof(Pike_T5_Focus_MinAttackReduction));
	memcpy(PikeSkills.Pike_T5_Focus_MaxAttackIncrease, Pike_T5_Focus_MaxAttackIncrease, sizeof(Pike_T5_Focus_MaxAttackIncrease));
	memcpy(PikeSkills.Pike_T5_Focus_UseMana, Pike_T5_Focus_UseMana, sizeof(Pike_T5_Focus_UseMana));
	memcpy(PikeSkills.Pike_T5_Death_DamageBoost, Pike_T5_Death_DamageBoost, sizeof(Pike_T5_Death_DamageBoost));
	memcpy(PikeSkills.Pike_T5_Death_AttackRateBoost, Pike_T5_Death_AttackRateBoost, sizeof(Pike_T5_Death_AttackRateBoost));
	memcpy(PikeSkills.Pike_T5_Death_UseMana, Pike_T5_Death_UseMana, sizeof(Pike_T5_Death_UseMana));
	memcpy(PikeSkills.Pike_T5_Twister_DamageBoost, Pike_T5_Twister_DamageBoost, sizeof(Pike_T5_Twister_DamageBoost));
	memcpy(PikeSkills.Pike_T5_Twister_UseMana, Pike_T5_Twister_UseMana, sizeof(Pike_T5_Twister_UseMana));
	ReadSkillInt(&PikeSkills.Pike_T5_Ring_Duration[0], "Pike.ini", "Pike_T5_Ring_Duration");
	ReadSkillInt(&PikeSkills.Pike_T5_Ring_UseMana[0], "Pike.ini", "Pike_T5_Ring_UseMana");
	ReadSkillInt(&PikeSkills.Pike_T5_Focus_MinAttackReduction[0], "Pike.ini", "Pike_T5_Focus_MinAttackReduction");
	ReadSkillInt(&PikeSkills.Pike_T5_Focus_MaxAttackIncrease[0], "Pike.ini", "Pike_T5_Focus_MaxAttackIncrease");
	ReadSkillInt(&PikeSkills.Pike_T5_Focus_UseMana[0], "Pike.ini", "Pike_T5_Focus_UseMana");
	ReadSkillInt(&PikeSkills.Pike_T5_Death_DamageBoost[0], "Pike.ini", "Pike_T5_Death_DamageBoost");
	ReadSkillInt(&PikeSkills.Pike_T5_Death_AttackRateBoost[0], "Pike.ini", "Pike_T5_Death_AttackRateBoost");
	ReadSkillInt(&PikeSkills.Pike_T5_Death_UseMana[0], "Pike.ini", "Pike_T5_Death_UseMana");
	ReadSkillInt(&PikeSkills.Pike_T5_Twister_DamageBoost[0], "Pike.ini", "Pike_T5_Twister_DamageBoost");
	ReadSkillInt(&PikeSkills.Pike_T5_Twister_UseMana[0], "Pike.ini", "Pike_T5_Twister_UseMana");

	ReadSkillInt(&ArcherSkills.Scout_Hawk_Attack_Rate[0], "Archer.ini", "Scout_Hawk_Attack_Rate");
	ReadSkillInt(&ArcherSkills.Scout_Hawk_UseTime[0], "Archer.ini", "Scout_Hawk_UseTime");
	ReadSkillInt(&ArcherSkills.Scout_Hawk_UseMana[0], "Archer.ini", "Scout_Hawk_UseMana");

	ReadSkillInt(&ArcherSkills.S_Mastery_DamagePercent[0], "Archer.ini", "S_Mastery_DamagePercent");

	ReadSkillInt(&ArcherSkills.Wind_Arrow_Speed[0], "Archer.ini", "Wind_Arrow_Speed");
	ReadSkillInt(&ArcherSkills.Wind_Arrow_Damage[0], "Archer.ini", "Wind_Arrow_Damage");
	ReadSkillInt(&ArcherSkills.Wind_Arrow_UseMana[0], "Archer.ini", "Wind_Arrow_UseMana");

	ReadSkillInt(&ArcherSkills.Perfect_Aim_Attack_Rate[0], "Archer.ini", "Perfect_Aim_Attack_Rate");
	ReadSkillInt(&ArcherSkills.Perfect_Aim_DamagePercent[0], "Archer.ini", "Perfect_Aim_DamagePercent");
	memcpy(ArcherSkills.Perfect_Aim_Damage, Perfect_Aim_Damage, sizeof(Perfect_Aim_Damage));
	ReadSkillInt(&ArcherSkills.Perfect_Aim_Damage[0], "Archer.ini", "Perfect_Aim_Damage");
	ReadSkillInt(&ArcherSkills.Perfect_Aim_Damage_LV[0], "Archer.ini", "Perfect_Aim_Damage_LV");
	ReadSkillInt(&ArcherSkills.Perfect_Aim_UseMana[0], "Archer.ini", "Perfect_Aim_UseMana");

	ReadSkillInt(&ArcherSkills.D_Eye_Attack_Rate[0], "Archer.ini", "D_Eye_Attack_Rate");

	ReadSkillInt2(ArcherSkills.Falcon_Damage, "Archer.ini", "Falcon_Damage");
	ReadSkillInt(&ArcherSkills.Falcon_Time[0], "Archer.ini", "Falcon_Time");
	ReadSkillInt(&ArcherSkills.Falcon_UseMana[0], "Archer.ini", "Falcon_UseMana");

	ReadSkillInt(&ArcherSkills.A_Rage_ArrowNum[0], "Archer.ini", "A_Rage_ArrowNum");
	ReadSkillInt(&ArcherSkills.A_Rage_Damage[0], "Archer.ini", "A_Rage_Damage");
	ReadSkillInt(&ArcherSkills.A_Rage_Area[0], "Archer.ini", "A_Rage_Area");
	ReadSkillInt(&ArcherSkills.A_Rage_UseMana[0], "Archer.ini", "A_Rage_UseMana");

	ReadSkillInt(&ArcherSkills.Avalanche_Speed[0], "Archer.ini", "Avalanche_Speed");
	ReadSkillInt(&ArcherSkills.Avalanche_Damage[0], "Archer.ini", "Avalanche_Damage");
	ReadSkillInt(&ArcherSkills.Avalanche_ArrowNum[0], "Archer.ini", "Avalanche_ArrowNum");
	ReadSkillInt(&ArcherSkills.Avalanche_UseMana[0], "Archer.ini", "Avalanche_UseMana");

	ReadSkillInt2(ArcherSkills.Elemental_Shot_Ice, "Archer.ini", "Elemental_Shot_Ice");
	ReadSkillInt2(ArcherSkills.Elemental_Shot_Fire, "Archer.ini", "Elemental_Shot_Fire");
	ReadSkillInt2(ArcherSkills.Elemental_Shot_Lightning, "Archer.ini", "Elemental_Shot_Lightning");
	ReadSkillInt(&ArcherSkills.Elemental_ShotPercent[0], "Archer.ini", "Elemental_ShotPercent");
	ReadSkillInt(&ArcherSkills.Elemental_Shot_UseMana[0], "Archer.ini", "Elemental_Shot_UseMana");

	ReadSkillInt(&ArcherSkills.Golden_Falcon_Damage[0], "Archer.ini", "Golden_Falcon_Damage");
	ReadSkillFloat(&ArcherSkills.Golden_Falcon_LifeRegen[0], "Archer.ini", "Golden_Falcon_LifeRegen");
	ReadSkillInt(&ArcherSkills.Golden_Falcon_Time[0], "Archer.ini", "Golden_Falcon_Time");
	ReadSkillInt(&ArcherSkills.Golden_Falcon_UseMana[0], "Archer.ini", "Golden_Falcon_UseMana");

	ReadSkillInt(&ArcherSkills.BombShot_Damage[0], "Archer.ini", "BombShot_Damage");
	ReadSkillInt(&ArcherSkills.BombShot_DemonDamage[0], "Archer.ini", "BombShot_DemonDamage");
	ReadSkillInt2(ArcherSkills.BombShot_AddDamage, "Archer.ini", "BombShot_AddDamage");
	ReadSkillInt(&ArcherSkills.BombShot_Area[0], "Archer.ini", "BombShot_Area");
	ReadSkillInt(&ArcherSkills.BombShot_UseMana[0], "Archer.ini", "BombShot_UseMana");

	ReadSkillInt(&ArcherSkills.Perforation_Damage[0], "Archer.ini", "Perforation_Damage");
	ReadSkillInt(&ArcherSkills.Perforation_Critical[0], "Archer.ini", "Perforation_Critical");
	ReadSkillInt(&ArcherSkills.Perforation_UseMana[0], "Archer.ini", "Perforation_UseMana");
	ReadSkillInt(&ArcherSkills.Perforation_Attack_Range[0], "Archer.ini", "Perforation_Attack_Range");

	ReadSkillInt2(ArcherSkills.Recall_Wolverin_Damage, "Archer.ini", "Recall_Wolverin_Damage");
	ReadSkillInt(&ArcherSkills.Recall_Wolverin_Hit[0], "Archer.ini", "Recall_Wolverin_Hit");
	ReadSkillInt(&ArcherSkills.Recall_Wolverin_Defense[0], "Archer.ini", "Recall_Wolverin_Defense");
	ReadSkillInt(&ArcherSkills.Recall_Wolverin_Life[0], "Archer.ini", "Recall_Wolverin_Life");
	ReadSkillInt(&ArcherSkills.Recall_Wolverin_UseMana[0], "Archer.ini", "Recall_Wolverin_UseMana");

	ReadSkillInt(&ArcherSkills.Evasion_Mastery_AddPercent[0], "Archer.ini", "Evasion_Mastery_AddPercent");

	ReadSkillInt(&ArcherSkills.Phoenix_Shot_DamagePercent[0], "Archer.ini", "Phoenix_Shot_DamagePercent");
	ReadSkillInt(&ArcherSkills.Phoenix_Shot_Time[0], "Archer.ini", "Phoenix_Shot_Time");
	ReadSkillInt(&ArcherSkills.Phoenix_Shot_UseMana[0], "Archer.ini", "Phoenix_Shot_UseMana");

	ReadSkillInt(&ArcherSkills.Force_Of_Nature_AddDamage[0], "Archer.ini", "Force_Of_Nature_AddDamage");
	ReadSkillInt(&ArcherSkills.Force_Of_Nature_AddHit[0], "Archer.ini", "Force_Of_Nature_AddHit");
	ReadSkillInt(&ArcherSkills.Force_Of_Nature_FalconAddDamage[0], "Archer.ini", "Force_Of_Nature_FalconAddDamage");
	ReadSkillInt(&ArcherSkills.Force_Of_Nature_Time[0], "Archer.ini", "Force_Of_Nature_Time");
	ReadSkillInt(&ArcherSkills.Force_Of_Nature_UseMana[0], "Archer.ini", "Force_Of_Nature_UseMana");

	memcpy(ArcherSkills.Archer_T5_Lethal_RangeAdd, Archer_T5_Lethal_RangeAdd, sizeof(Archer_T5_Lethal_RangeAdd));
	memcpy(ArcherSkills.Archer_T5_Lethal_CriticalRate, Archer_T5_Lethal_CriticalRate, sizeof(Archer_T5_Lethal_CriticalRate));
	memcpy(ArcherSkills.Archer_T5_Lethal_DamageBoost, Archer_T5_Lethal_DamageBoost, sizeof(Archer_T5_Lethal_DamageBoost));
	memcpy(ArcherSkills.Archer_T5_Lethal_UseMana, Archer_T5_Lethal_UseMana, sizeof(Archer_T5_Lethal_UseMana));
	memcpy(ArcherSkills.Archer_T5_Fierce_DamageBoost, Archer_T5_Fierce_DamageBoost, sizeof(Archer_T5_Fierce_DamageBoost));
	memcpy(ArcherSkills.Archer_T5_Fierce_CriticalDamageBoost, Archer_T5_Fierce_CriticalDamageBoost, sizeof(Archer_T5_Fierce_CriticalDamageBoost));
	memcpy(ArcherSkills.Archer_T5_Fierce_UseMana, Archer_T5_Fierce_UseMana, sizeof(Archer_T5_Fierce_UseMana));
	memcpy(ArcherSkills.Archer_T5_Roots_Duration, Archer_T5_Roots_Duration, sizeof(Archer_T5_Roots_Duration));
	memcpy(ArcherSkills.Archer_T5_Roots_UseMana, Archer_T5_Roots_UseMana, sizeof(Archer_T5_Roots_UseMana));
	memcpy(ArcherSkills.Archer_T5_Bombard_DamageBoost, Archer_T5_Bombard_DamageBoost, sizeof(Archer_T5_Bombard_DamageBoost));
	memcpy(ArcherSkills.Archer_T5_Bombard_Duration, Archer_T5_Bombard_Duration, sizeof(Archer_T5_Bombard_Duration));
	memcpy(ArcherSkills.Archer_T5_Bombard_MPDrain, Archer_T5_Bombard_MPDrain, sizeof(Archer_T5_Bombard_MPDrain));
	memcpy(ArcherSkills.Archer_T5_Bombard_SPDrain, Archer_T5_Bombard_SPDrain, sizeof(Archer_T5_Bombard_SPDrain));
	memcpy(ArcherSkills.Archer_T5_Bombard_UseMana, Archer_T5_Bombard_UseMana, sizeof(Archer_T5_Bombard_UseMana));

	ReadSkillInt(&ArcherSkills.Archer_T5_Lethal_RangeAdd[0], "Archer.ini", "Archer_T5_Lethal_RangeAdd");
	ReadSkillInt(&ArcherSkills.Archer_T5_Lethal_CriticalRate[0], "Archer.ini", "Archer_T5_Lethal_CriticalRate");
	ReadSkillInt(&ArcherSkills.Archer_T5_Lethal_DamageBoost[0], "Archer.ini", "Archer_T5_Lethal_DamageBoost");
	ReadSkillInt(&ArcherSkills.Archer_T5_Lethal_UseMana[0], "Archer.ini", "Archer_T5_Lethal_UseMana");
	ReadSkillInt(&ArcherSkills.Archer_T5_Fierce_DamageBoost[0], "Archer.ini", "Archer_T5_Fierce_DamageBoost");
	ReadSkillInt(&ArcherSkills.Archer_T5_Fierce_CriticalDamageBoost[0], "Archer.ini", "Archer_T5_Fierce_CriticalDamageBoost");
	ReadSkillInt(&ArcherSkills.Archer_T5_Fierce_UseMana[0], "Archer.ini", "Archer_T5_Fierce_UseMana");
	ReadSkillInt(&ArcherSkills.Archer_T5_Roots_Duration[0], "Archer.ini", "Archer_T5_Roots_Duration");
	ReadSkillInt(&ArcherSkills.Archer_T5_Roots_UseMana[0], "Archer.ini", "Archer_T5_Roots_UseMana");
	ReadSkillInt(&ArcherSkills.Archer_T5_Bombard_DamageBoost[0], "Archer.ini", "Archer_T5_Bombard_DamageBoost");
	ReadSkillInt(&ArcherSkills.Archer_T5_Bombard_Duration[0], "Archer.ini", "Archer_T5_Bombard_Duration");
	ReadSkillInt(&ArcherSkills.Archer_T5_Bombard_MPDrain[0], "Archer.ini", "Archer_T5_Bombard_MPDrain");
	ReadSkillInt(&ArcherSkills.Archer_T5_Bombard_SPDrain[0], "Archer.ini", "Archer_T5_Bombard_SPDrain");
	ReadSkillInt(&ArcherSkills.Archer_T5_Bombard_UseMana[0], "Archer.ini", "Archer_T5_Bombard_UseMana");

	ReadSkillInt(&KinaSkills.Sword_Blast_UseMana[0], "Knight.ini", "Sword_Blast_UseMana");
	ReadSkillInt(&KinaSkills.Sword_Blast_DamagePercent[0], "Knight.ini", "Sword_Blast_DamagePercent");
	ReadSkillInt(&KinaSkills.Sword_Blast_ShootingRange[0], "Knight.ini", "Sword_Blast_ShootingRange");

	ReadSkillInt(&KinaSkills.HolyBody_UseMana[0], "Knight.ini", "HolyBody_UseMana");
	ReadSkillInt(&KinaSkills.HolyBody_Absorb[0], "Knight.ini", "HolyBody_Absorb");
	ReadSkillInt(&KinaSkills.HolyBody_Time[0], "Knight.ini", "HolyBody_Time");

	ReadSkillInt(&KinaSkills.P_Traning_Stamina[0], "Knight.ini", "P_Traning_Stamina");

	ReadSkillInt(&KinaSkills.D_Crash_UseMana[0], "Knight.ini", "D_Crash_UseMana");
	ReadSkillInt(&KinaSkills.D_Crash_Damage[0], "Knight.ini", "D_Crash_Damage");
	ReadSkillInt(&KinaSkills.D_Crash_Critical[0], "Knight.ini", "D_Crash_Critical");

	ReadSkillInt(&KinaSkills.Holy_Valor_Damage[0], "Knight.ini", "Holy_Valor_Damage");
	ReadSkillInt(&KinaSkills.Holy_Valor_Time[0], "Knight.ini", "Holy_Valor_Time");
	ReadSkillInt(&KinaSkills.Holy_Valor_Mana[0], "Knight.ini", "Holy_Valor_Mana");

	ReadSkillInt(&KinaSkills.Brandish_Range[0], "Knight.ini", "Brandish_Range");
	ReadSkillInt(&KinaSkills.Brandish_Damage[0], "Knight.ini", "Brandish_Damage");
	ReadSkillInt(&KinaSkills.Brandish_Mana[0], "Knight.ini", "Brandish_Mana");

	ReadSkillInt(&KinaSkills.Piercing_Damage[0], "Knight.ini", "Piercing_Damage");
	ReadSkillInt(&KinaSkills.Piercing_Attack_Rating[0], "Knight.ini", "Piercing_Attack_Rating");
	ReadSkillInt(&KinaSkills.Piercing_Mana[0], "Knight.ini", "Piercing_Mana");

	ReadSkillInt(&KinaSkills.Drastic_Spirit_Defense[0], "Knight.ini", "Drastic_Spirit_Defense");
	ReadSkillInt(&KinaSkills.Drastic_Spirit_Time[0], "Knight.ini", "Drastic_Spirit_Time");
	ReadSkillInt(&KinaSkills.Drastic_Spirit_Mana[0], "Knight.ini", "Drastic_Spirit_Mana");

	ReadSkillInt(&KinaSkills.Sword_Mastery_DamagePercent[0], "Knight.ini", "Sword_Mastery_DamagePercent");

	ReadSkillInt(&KinaSkills.D_Inhalation_Life[0], "Knight.ini", "D_Inhalation_Life");
	ReadSkillInt(&KinaSkills.D_Inhalation_Block[0], "Knight.ini", "D_Inhalation_Block");
	ReadSkillInt(&KinaSkills.D_Inhalation_Time[0], "Knight.ini", "D_Inhalation_Time");
	ReadSkillInt(&KinaSkills.D_Inhalation_UseMana[0], "Knight.ini", "D_Inhalation_UseMana");

	ReadSkillInt(&KinaSkills.H_Incantation_Success[0], "Knight.ini", "H_Incantation_Success");
	ReadSkillInt(&KinaSkills.H_Incantation_AddLife[0], "Knight.ini", "H_Incantation_AddLife");
	ReadSkillInt(&KinaSkills.H_Incantation_Time[0], "Knight.ini", "H_Incantation_Time");
	ReadSkillInt(&KinaSkills.H_Incantation_UseMana[0], "Knight.ini", "H_Incantation_UseMana");

	ReadSkillInt(&KinaSkills.Grand_Cross_Damage[0], "Knight.ini", "Grand_Cross_Damage");
	ReadSkillInt(&KinaSkills.Grand_Cross_AttackRate[0], "Knight.ini", "Grand_Cross_AttackRate");
	ReadSkillInt(&KinaSkills.Grand_Cross_UndeadDamage[0], "Knight.ini", "Grand_Cross_UndeadDamage");
	ReadSkillInt(&KinaSkills.Grand_Cross_UseMana[0], "Knight.ini", "Grand_Cross_UseMana");

	ReadSkillInt(&KinaSkills.Godly_Shield_AbsorbPercent[0], "Knight.ini", "Godly_Shield_AbsorbPercent");
	ReadSkillInt(&KinaSkills.Godly_Shield_Time[0], "Knight.ini", "Godly_Shield_Time");
	ReadSkillInt(&KinaSkills.Godly_Shield_UseMana[0], "Knight.ini", "Godly_Shield_UseMana");

	ReadSkillInt(&KinaSkills.God_Bless_AddDamage[0], "Knight.ini", "God_Bless_AddDamage");
	ReadSkillInt(&KinaSkills.God_Bless_Time[0], "Knight.ini", "God_Bless_Time");
	ReadSkillInt(&KinaSkills.God_Bless_UseMana[0], "Knight.ini", "God_Bless_UseMana");

	ReadSkillInt(&KinaSkills.Sword_Of_Justice_DamagePercent[0], "Knight.ini", "Sword_Of_Justice_DamagePercent");
	ReadSkillInt(&KinaSkills.Sword_Of_Justice_Area[0], "Knight.ini", "Sword_Of_Justice_Area");
	ReadSkillInt(&KinaSkills.Sword_Of_Justice_UseMana[0], "Knight.ini", "Sword_Of_Justice_UseMana");

	memcpy(KinaSkills.Knight_T5_HolyConviction_UndeadBoost, Knight_T5_HolyConviction_UndeadBoost, sizeof(Knight_T5_HolyConviction_UndeadBoost));
	memcpy(KinaSkills.Knight_T5_HolyConviction_Duration, Knight_T5_HolyConviction_Duration, sizeof(Knight_T5_HolyConviction_Duration));
	memcpy(KinaSkills.Knight_T5_HolyConviction_UseMana, Knight_T5_HolyConviction_UseMana, sizeof(Knight_T5_HolyConviction_UseMana));
	memcpy(KinaSkills.Knight_T5_DivineInquisition_DamageBoost, Knight_T5_DivineInquisition_DamageBoost, sizeof(Knight_T5_DivineInquisition_DamageBoost));
	memcpy(KinaSkills.Knight_T5_DivineInquisition_Area, Knight_T5_DivineInquisition_Area, sizeof(Knight_T5_DivineInquisition_Area));
	memcpy(KinaSkills.Knight_T5_DivineInquisition_UndeadDamage, Knight_T5_DivineInquisition_UndeadDamage, sizeof(Knight_T5_DivineInquisition_UndeadDamage));
	memcpy(KinaSkills.Knight_T5_DivineInquisition_UseMana, Knight_T5_DivineInquisition_UseMana, sizeof(Knight_T5_DivineInquisition_UseMana));
	memcpy(KinaSkills.Knight_T5_GloriousShield_DefenseBoost, Knight_T5_GloriousShield_DefenseBoost, sizeof(Knight_T5_GloriousShield_DefenseBoost));
	memcpy(KinaSkills.Knight_T5_GloriousShield_Duration, Knight_T5_GloriousShield_Duration, sizeof(Knight_T5_GloriousShield_Duration));
	memcpy(KinaSkills.Knight_T5_GloriousShield_UseMana, Knight_T5_GloriousShield_UseMana, sizeof(Knight_T5_GloriousShield_UseMana));
	memcpy(KinaSkills.Knight_T5_DivineCross_DamageBoost, Knight_T5_DivineCross_DamageBoost, sizeof(Knight_T5_DivineCross_DamageBoost));
	memcpy(KinaSkills.Knight_T5_DivineCross_UseMana, Knight_T5_DivineCross_UseMana, sizeof(Knight_T5_DivineCross_UseMana));

	ReadSkillInt(&KinaSkills.Knight_T5_HolyConviction_UndeadBoost[0], "Knight.ini", "Knight_T5_HolyConviction_UndeadBoost");
	ReadSkillInt(&KinaSkills.Knight_T5_HolyConviction_Duration[0], "Knight.ini", "Knight_T5_HolyConviction_Duration");
	ReadSkillInt(&KinaSkills.Knight_T5_HolyConviction_UseMana[0], "Knight.ini", "Knight_T5_HolyConviction_UseMana");
	ReadSkillInt(&KinaSkills.Knight_T5_DivineInquisition_DamageBoost[0], "Knight.ini", "Knight_T5_DivineInquisition_DamageBoost");
	ReadSkillInt(&KinaSkills.Knight_T5_DivineInquisition_Area[0], "Knight.ini", "Knight_T5_DivineInquisition_Area");
	ReadSkillInt(&KinaSkills.Knight_T5_DivineInquisition_UndeadDamage[0], "Knight.ini", "Knight_T5_DivineInquisition_UndeadDamage");
	ReadSkillInt(&KinaSkills.Knight_T5_DivineInquisition_UseMana[0], "Knight.ini", "Knight_T5_DivineInquisition_UseMana");
	ReadSkillInt(&KinaSkills.Knight_T5_GloriousShield_DefenseBoost[0], "Knight.ini", "Knight_T5_GloriousShield_DefenseBoost");
	ReadSkillInt(&KinaSkills.Knight_T5_GloriousShield_Duration[0], "Knight.ini", "Knight_T5_GloriousShield_Duration");
	ReadSkillInt(&KinaSkills.Knight_T5_GloriousShield_UseMana[0], "Knight.ini", "Knight_T5_GloriousShield_UseMana");
	ReadSkillInt(&KinaSkills.Knight_T5_DivineCross_DamageBoost[0], "Knight.ini", "Knight_T5_DivineCross_DamageBoost");
	ReadSkillInt(&KinaSkills.Knight_T5_DivineCross_UseMana[0], "Knight.ini", "Knight_T5_DivineCross_UseMana");


	ReadSkillInt(&KinaSkills.Divine_Piercing_DamagePercent[0], "Knight.ini", "Divine_Piercing_DamagePercent");
	ReadSkillInt(&KinaSkills.Divine_Piercing_AddHit[0], "Knight.ini", "Divine_Piercing_AddHit");
	ReadSkillInt(&KinaSkills.Divine_Piercing_AttackNum[0], "Knight.ini", "Divine_Piercing_AttackNum");
	ReadSkillInt(&KinaSkills.Divine_Piercing_UseMana[0], "Knight.ini", "Divine_Piercing_UseMana");

	ReadSkillInt(&AtaSkills.S_Strike_UseMana[0], "Atalanta.ini", "S_Strike_UseMana");
	ReadSkillInt(&AtaSkills.S_Strike_DamagePercent[0], "Atalanta.ini", "S_Strike_DamagePercent");

	ReadSkillInt(&AtaSkills.Farina_UseMana[0], "Atalanta.ini", "Farina_UseMana");
	ReadSkillInt(&AtaSkills.Farina_Speed[0], "Atalanta.ini", "Farina_Speed");
	ReadSkillInt(&AtaSkills.Farina_AttackRate[0], "Atalanta.ini", "Farina_AttackRate");

	ReadSkillInt(&AtaSkills.T_Mastery_Damage[0], "Atalanta.ini", "T_Mastery_Damage");

	ReadSkillInt(&AtaSkills.V_Spear_UseMana[0], "Atalanta.ini", "V_Spear_UseMana");
	ReadSkillInt(&AtaSkills.V_Spear_DamagePercent[0], "Atalanta.ini", "V_Spear_DamagePercent");

	ReadSkillInt(&AtaSkills.Windy_Attack_Rating[0], "Atalanta.ini", "Windy_Attack_Rating");
	ReadSkillInt(&AtaSkills.Windy_Time[0], "Atalanta.ini", "Windy_Time");
	ReadSkillInt(&AtaSkills.Windy_Mana[0], "Atalanta.ini", "Windy_Mana");

	ReadSkillInt(&AtaSkills.Twist_Javelin_Damage[0], "Atalanta.ini", "Twist_Javelin_Damage");
	ReadSkillInt(&AtaSkills.Twist_Javelin_Attack_Rating[0], "Atalanta.ini", "Twist_Javelin_Attack_Rating");
	ReadSkillInt(&AtaSkills.Twist_Javelin_Mana[0], "Atalanta.ini", "Twist_Javelin_Mana");

	ReadSkillInt(&AtaSkills.Soul_Sucker_Absorb[0], "Atalanta.ini", "Soul_Sucker_Absorb");
	ReadSkillInt(&AtaSkills.Soul_Sucker_Mana[0], "Atalanta.ini", "Soul_Sucker_Mana");

	ReadSkillInt(&AtaSkills.Fire_Javelin_DamagePercent[0], "Atalanta.ini", "Fire_Javelin_DamagePercent");
	ReadSkillInt(&AtaSkills.Fire_Javelin_Mana[0], "Atalanta.ini", "Fire_Javelin_Mana");

	ReadSkillInt(&AtaSkills.Split_Javelin_AttackNum[0], "Atalanta.ini", "Split_Javelin_AttackNum");
	ReadSkillInt(&AtaSkills.Split_Javelin_Damage[0], "Atalanta.ini", "Split_Javelin_Damage");
	ReadSkillInt(&AtaSkills.Split_Javelin_AttackRate[0], "Atalanta.ini", "Split_Javelin_AttackRate");
	ReadSkillInt(&AtaSkills.Split_Javelin_UseMana[0], "Atalanta.ini", "Split_Javelin_UseMana");

	ReadSkillInt(&AtaSkills.T_Of_Valhalla_Damage[0], "Atalanta.ini", "T_Of_Valhalla_Damage");
	ReadSkillInt(&AtaSkills.T_Of_Valhalla_Time[0], "Atalanta.ini", "T_Of_Valhalla_Time");
	ReadSkillInt(&AtaSkills.T_Of_Valhalla_UseMana[0], "Atalanta.ini", "T_Of_Valhalla_UseMana");

	ReadSkillInt(&AtaSkills.Lightning_Javelin_DamagePercent[0], "Atalanta.ini", "Lightning_Javelin_DamagePercent");
	ReadSkillInt(&AtaSkills.Lightning_Javelin_UseMana[0], "Atalanta.ini", "Lightning_Javelin_UseMana");

	ReadSkillInt(&AtaSkills.Storm_Javelin_Damage[0], "Atalanta.ini", "Storm_Javelin_Damage");
	ReadSkillInt(&AtaSkills.Storm_Javelin_UseMana[0], "Atalanta.ini", "Storm_Javelin_UseMana");

	ReadSkillInt(&AtaSkills.Hall_Of_Valhalla_AddEvation[0], "Atalanta.ini", "Hall_Of_Valhalla_AddEvation");
	ReadSkillInt(&AtaSkills.Hall_Of_Valhalla_Time[0], "Atalanta.ini", "Hall_Of_Valhalla_Time");
	ReadSkillInt(&AtaSkills.Hall_Of_Valhalla_UseMana[0], "Atalanta.ini", "Hall_Of_Valhalla_UseMana");

	ReadSkillInt(&AtaSkills.X_Rage_DamagePercent[0], "Atalanta.ini", "X_Rage_DamagePercent");
	ReadSkillInt(&AtaSkills.X_Rage_Area[0], "Atalanta.ini", "X_Rage_Area");
	ReadSkillInt(&AtaSkills.X_Rage_UseMana[0], "Atalanta.ini", "X_Rage_UseMana");

	ReadSkillInt2(AtaSkills.Frost_Javelin_IceAddDamage, "Atalanta.ini", "Frost_Javelin_IceAddDamage");
	ReadSkillInt(&AtaSkills.Frost_Javelin_SpeedSubPercent[0], "Atalanta.ini", "Frost_Javelin_SpeedSubPercent");
	ReadSkillInt(&AtaSkills.Frost_Javelin_IceTime[0], "Atalanta.ini", "Frost_Javelin_IceTime");
	ReadSkillInt(&AtaSkills.Frost_Javelin_Time[0], "Atalanta.ini", "Frost_Javelin_Time");
	ReadSkillInt(&AtaSkills.Frost_Javelin_UseMana[0], "Atalanta.ini", "Frost_Javelin_UseMana");

	ReadSkillInt(&AtaSkills.Vengeance_DamagePercent[0], "Atalanta.ini", "Vengeance_DamagePercent");
	ReadSkillInt(&AtaSkills.Vengeance_Range[0], "Atalanta.ini", "Vengeance_Range");
	ReadSkillInt(&AtaSkills.Vengeance_UseMana[0], "Atalanta.ini", "Vengeance_UseMana");
	ReadSkillInt(&AtaSkills.Vengeance_AddCritical[0], "Atalanta.ini", "Vengeance_AddCritical");

	memcpy(AtaSkills.Atalanta_T5_SummonTiger_AttackPower, Atalanta_T5_SummonTiger_AttackPower, sizeof(Atalanta_T5_SummonTiger_AttackPower));
	memcpy(AtaSkills.Atalanta_T5_SummonTiger_AttackRating, Atalanta_T5_SummonTiger_AttackRating, sizeof(Atalanta_T5_SummonTiger_AttackRating));
	memcpy(AtaSkills.Atalanta_T5_SummonTiger_Life, Atalanta_T5_SummonTiger_Life, sizeof(Atalanta_T5_SummonTiger_Life));
	memcpy(AtaSkills.Atalanta_T5_SummonTiger_Duration, Atalanta_T5_SummonTiger_Duration, sizeof(Atalanta_T5_SummonTiger_Duration));
	memcpy(AtaSkills.Atalanta_T5_SummonTiger_UseMana, Atalanta_T5_SummonTiger_UseMana, sizeof(Atalanta_T5_SummonTiger_UseMana));
	memcpy(AtaSkills.Atalanta_T5_GoldenApple_Speed, Atalanta_T5_GoldenApple_Speed, sizeof(Atalanta_T5_GoldenApple_Speed));
	memcpy(AtaSkills.Atalanta_T5_GoldenApple_Evade, Atalanta_T5_GoldenApple_Evade, sizeof(Atalanta_T5_GoldenApple_Evade));
	memcpy(AtaSkills.Atalanta_T5_Plague_DamageBoost, Atalanta_T5_Plague_DamageBoost, sizeof(Atalanta_T5_Plague_DamageBoost));
	memcpy(AtaSkills.Atalanta_T5_Plague_Targets, Atalanta_T5_Plague_Targets, sizeof(Atalanta_T5_Plague_Targets));
	memcpy(AtaSkills.Atalanta_T5_Plague_PoisonDuration, Atalanta_T5_Plague_PoisonDuration, sizeof(Atalanta_T5_Plague_PoisonDuration));
	memcpy(AtaSkills.Atalanta_T5_Plague_PoisonDamagePercent, Atalanta_T5_Plague_PoisonDamagePercent, sizeof(Atalanta_T5_Plague_PoisonDamagePercent));
	memcpy(AtaSkills.Atalanta_T5_Plague_UseMana, Atalanta_T5_Plague_UseMana, sizeof(Atalanta_T5_Plague_UseMana));
	memcpy(AtaSkills.Atalanta_T5_Combo_DamageBoost, Atalanta_T5_Combo_DamageBoost, sizeof(Atalanta_T5_Combo_DamageBoost));
	memcpy(AtaSkills.Atalanta_T5_Combo_UseMana, Atalanta_T5_Combo_UseMana, sizeof(Atalanta_T5_Combo_UseMana));
	ReadSkillInt(&AtaSkills.Atalanta_T5_SummonTiger_AttackPower[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_AttackPower");
	ReadSkillInt(&AtaSkills.Atalanta_T5_SummonTiger_AttackRating[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_AttackRating");
	ReadSkillInt(&AtaSkills.Atalanta_T5_SummonTiger_Life[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_Life");
	ReadSkillInt(&AtaSkills.Atalanta_T5_SummonTiger_Duration[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_Duration");
	ReadSkillInt(&AtaSkills.Atalanta_T5_SummonTiger_UseMana[0], "Atalanta.ini", "Atalanta_T5_SummonTiger_UseMana");
	ReadSkillInt(&AtaSkills.Atalanta_T5_GoldenApple_Speed[0], "Atalanta.ini", "Atalanta_T5_GoldenApple_Speed");
	ReadSkillInt(&AtaSkills.Atalanta_T5_GoldenApple_Evade[0], "Atalanta.ini", "Atalanta_T5_GoldenApple_Evade");
	ReadSkillInt(&AtaSkills.Atalanta_T5_Plague_DamageBoost[0], "Atalanta.ini", "Atalanta_T5_Plague_DamageBoost");
	ReadSkillInt(&AtaSkills.Atalanta_T5_Plague_Targets[0], "Atalanta.ini", "Atalanta_T5_Plague_Targets");
	ReadSkillInt(&AtaSkills.Atalanta_T5_Plague_PoisonDuration[0], "Atalanta.ini", "Atalanta_T5_Plague_PoisonDuration");
	ReadSkillInt(&AtaSkills.Atalanta_T5_Plague_PoisonDamagePercent[0], "Atalanta.ini", "Atalanta_T5_Plague_PoisonDamagePercent");
	ReadSkillInt(&AtaSkills.Atalanta_T5_Plague_UseMana[0], "Atalanta.ini", "Atalanta_T5_Plague_UseMana");
	ReadSkillInt(&AtaSkills.Atalanta_T5_Combo_DamageBoost[0], "Atalanta.ini", "Atalanta_T5_Combo_DamageBoost");
	ReadSkillInt(&AtaSkills.Atalanta_T5_Combo_UseMana[0], "Atalanta.ini", "Atalanta_T5_Combo_UseMana");

	//sacer
	ReadSkillInt(&SacerSkills.Healing_UseMana[0], "Sacer.ini", "Healing_UseMana");
	ReadSkillInt2(SacerSkills.Healing_Heal, "Sacer.ini", "Healing_Heal");

	ReadSkillInt(&SacerSkills.HolyBolt_UseMana[0], "Sacer.ini", "HolyBolt_UseMana");
	ReadSkillInt(&SacerSkills.HolyBolt_DamagePercent[0], "Sacer.ini", "HolyBolt_DamagePercent");

	ReadSkillInt(&SacerSkills.M_Spark_UseMana[0], "Sacer.ini", "M_Spark_UseMana");
	ReadSkillInt(&SacerSkills.M_Spark_Damage_Percent[0], "Sacer.ini", "M_Spark_Damage_Percent");
	ReadSkillInt(&SacerSkills.M_Spark_Num[0], "Sacer.ini", "M_Spark_Num");

	ReadSkillInt(&SacerSkills.HolyMind_UseMana[0], "Sacer.ini", "HolyMind_UseMana");
	ReadSkillInt(&SacerSkills.HolyMind_DecDamage[0], "Sacer.ini", "HolyMind_DecDamage");
	ReadSkillInt(&SacerSkills.HolyMind_Time[0], "Sacer.ini", "HolyMind_Time");

	ReadSkillFloat(&SacerSkills.Meditation_Regen[0], "Sacer.ini", "Meditation_Regen");

	ReadSkillInt(&SacerSkills.Divine_Lightning_Damage_Percent[0], "Sacer.ini", "Divine_Lightning_Damage_Percent");
	ReadSkillInt(&SacerSkills.Divine_Lightning_Num[0], "Sacer.ini", "Divine_Lightning_Num");
	ReadSkillInt(&SacerSkills.Divine_Lightning_Mana[0], "Sacer.ini", "Divine_Lightning_Mana");

	ReadSkillInt(&SacerSkills.Holy_Reflection_Return_Damage[0], "Sacer.ini", "Holy_Reflection_Return_Damage");
	ReadSkillInt(&SacerSkills.Holy_Reflection_Time[0], "Sacer.ini", "Holy_Reflection_Time");
	ReadSkillInt(&SacerSkills.Holy_Reflection_Mana[0], "Sacer.ini", "Holy_Reflection_Mana");

	ReadSkillInt2(SacerSkills.Grand_Healing, "Sacer.ini", "Grand_Healing");
	ReadSkillInt(&SacerSkills.Grand_Healing_Range[0], "Sacer.ini", "Grand_Healing_Range");
	ReadSkillInt(&SacerSkills.Grand_Healing_Mana[0], "Sacer.ini", "Grand_Healing_Mana");

	ReadSkillInt(&SacerSkills.Vigor_Ball_Damage_Percent[0], "Sacer.ini", "Vigor_Ball_Damage_Percent");
	ReadSkillInt(&SacerSkills.Vigor_Ball_UseMana[0], "Sacer.ini", "Vigor_Ball_UseMana");

	ReadSkillInt(&SacerSkills.Resurrection_Percent[0], "Sacer.ini", "Resurrection_Percent");
	ReadSkillInt(&SacerSkills.Resurrection_Exp[0], "Sacer.ini", "Resurrection_Exp");
	ReadSkillInt(&SacerSkills.Resurrection_UseMana[0], "Sacer.ini", "Resurrection_UseMana");

	ReadSkillInt(&SacerSkills.Extinction_Percent[0], "Sacer.ini", "Extinction_Percent");
	ReadSkillInt(&SacerSkills.Extinction_Amount[0], "Sacer.ini", "Extinction_Amount");
	ReadSkillInt(&SacerSkills.Extinction_UseMana[0], "Sacer.ini", "Extinction_UseMana");

	ReadSkillInt(&SacerSkills.Virtual_Life_Percent[0], "Sacer.ini", "Virtual_Life_Percent");
	ReadSkillInt(&SacerSkills.Virtual_Life_Time[0], "Sacer.ini", "Virtual_Life_Time");
	ReadSkillInt(&SacerSkills.Virtual_Life_UseMana[0], "Sacer.ini", "Virtual_Life_UseMana");

	ReadSkillInt(&SacerSkills.Glacial_Spike_Damage_Percent[0], "Sacer.ini", "Glacial_Spike_Damage_Percent");
	ReadSkillInt(&SacerSkills.Glacial_Spike_Area[0], "Sacer.ini", "Glacial_Spike_Area");
	ReadSkillInt(&SacerSkills.Glacial_Spike_UseMana[0], "Sacer.ini", "Glacial_Spike_UseMana");

	ReadSkillFloat(&SacerSkills.Regeneration_Field_LifeRegen[0], "Sacer.ini", "Regeneration_Field_LifeRegen");
	ReadSkillFloat(&SacerSkills.Regeneration_Field_ManaRegen[0], "Sacer.ini", "Regeneration_Field_ManaRegen");
	ReadSkillInt(&SacerSkills.Regeneration_Field_Time[0], "Sacer.ini", "Regeneration_Field_Time");
	ReadSkillInt(&SacerSkills.Regeneration_Field_Area[0], "Sacer.ini", "Regeneration_Field_Area");
	ReadSkillInt(&SacerSkills.Regeneration_Field_UseMana[0], "Sacer.ini", "Regeneration_Field_UseMana");

	ReadSkillInt(&SacerSkills.Chain_Lightning_Damage_Percent[0], "Sacer.ini", "Chain_Lightning_Damage_Percent");
	ReadSkillInt(&SacerSkills.Chain_Lightning_Num[0], "Sacer.ini", "Chain_Lightning_Num");
	ReadSkillInt(&SacerSkills.Chain_Lightning_Range[0], "Sacer.ini", "Chain_Lightning_Range");
	ReadSkillInt(&SacerSkills.Chain_Lightning_UseMana[0], "Sacer.ini", "Chain_Lightning_UseMana");

	ReadSkillInt2(SacerSkills.Summon_Muspell_Damage, "Sacer.ini", "Summon_Muspell_Damage");
	ReadSkillInt(&SacerSkills.Summon_Muspell_Attack_Delay[0], "Sacer.ini", "Summon_Muspell_Attack_Delay");
	ReadSkillInt(&SacerSkills.Summon_Muspell_UndeadAbsorbPercent[0], "Sacer.ini", "Summon_Muspell_UndeadAbsorbPercent");
	ReadSkillInt(&SacerSkills.Summon_Muspell_BlockPercent[0], "Sacer.ini", "Summon_Muspell_BlockPercent");
	ReadSkillInt(&SacerSkills.Summon_Muspell_Time[0], "Sacer.ini", "Summon_Muspell_Time");
	ReadSkillInt(&SacerSkills.Summon_Muspell_UseMana[0], "Sacer.ini", "Summon_Muspell_UseMana");

	memcpy(SacerSkills.Priestess_T5_IceElemental_AttackPower, Priestess_T5_IceElemental_AttackPower, sizeof(Priestess_T5_IceElemental_AttackPower));
	memcpy(SacerSkills.Priestess_T5_IceElemental_AttackRating, Priestess_T5_IceElemental_AttackRating, sizeof(Priestess_T5_IceElemental_AttackRating));
	memcpy(SacerSkills.Priestess_T5_IceElemental_Life, Priestess_T5_IceElemental_Life, sizeof(Priestess_T5_IceElemental_Life));
	memcpy(SacerSkills.Priestess_T5_IceElemental_Duration, Priestess_T5_IceElemental_Duration, sizeof(Priestess_T5_IceElemental_Duration));
	memcpy(SacerSkills.Priestess_T5_IceElemental_UseMana, Priestess_T5_IceElemental_UseMana, sizeof(Priestess_T5_IceElemental_UseMana));
	memcpy(SacerSkills.Priestess_T5_LightningSurge_DamageBoost, Priestess_T5_LightningSurge_DamageBoost, sizeof(Priestess_T5_LightningSurge_DamageBoost));
	memcpy(SacerSkills.Priestess_T5_LightningSurge_Area, Priestess_T5_LightningSurge_Area, sizeof(Priestess_T5_LightningSurge_Area));
	memcpy(SacerSkills.Priestess_T5_LightningSurge_ChargedArea, Priestess_T5_LightningSurge_ChargedArea, sizeof(Priestess_T5_LightningSurge_ChargedArea));
	memcpy(SacerSkills.Priestess_T5_LightningSurge_UseMana, Priestess_T5_LightningSurge_UseMana, sizeof(Priestess_T5_LightningSurge_UseMana));
	memcpy(SacerSkills.Priestess_T5_HeavenlyLight_AttackRateReduction, Priestess_T5_HeavenlyLight_AttackRateReduction, sizeof(Priestess_T5_HeavenlyLight_AttackRateReduction));
	memcpy(SacerSkills.Priestess_T5_HeavenlyLight_Duration, Priestess_T5_HeavenlyLight_Duration, sizeof(Priestess_T5_HeavenlyLight_Duration));
	memcpy(SacerSkills.Priestess_T5_HeavenlyLight_Area, Priestess_T5_HeavenlyLight_Area, sizeof(Priestess_T5_HeavenlyLight_Area));
	memcpy(SacerSkills.Priestess_T5_HeavenlyLight_UseMana, Priestess_T5_HeavenlyLight_UseMana, sizeof(Priestess_T5_HeavenlyLight_UseMana));
	memcpy(SacerSkills.Priestess_T5_Consecration_DefenseBoost, Priestess_T5_Consecration_DefenseBoost, sizeof(Priestess_T5_Consecration_DefenseBoost));
	memcpy(SacerSkills.Priestess_T5_Consecration_Absorption, Priestess_T5_Consecration_Absorption, sizeof(Priestess_T5_Consecration_Absorption));
	memcpy(SacerSkills.Priestess_T5_Consecration_UseMana, Priestess_T5_Consecration_UseMana, sizeof(Priestess_T5_Consecration_UseMana));
	ReadSkillInt(&SacerSkills.Priestess_T5_IceElemental_AttackPower[0], "Sacer.ini", "Priestess_T5_IceElemental_AttackPower");
	ReadSkillInt(&SacerSkills.Priestess_T5_IceElemental_AttackRating[0], "Sacer.ini", "Priestess_T5_IceElemental_AttackRating");
	ReadSkillInt(&SacerSkills.Priestess_T5_IceElemental_Life[0], "Sacer.ini", "Priestess_T5_IceElemental_Life");
	ReadSkillInt(&SacerSkills.Priestess_T5_IceElemental_Duration[0], "Sacer.ini", "Priestess_T5_IceElemental_Duration");
	ReadSkillInt(&SacerSkills.Priestess_T5_IceElemental_UseMana[0], "Sacer.ini", "Priestess_T5_IceElemental_UseMana");
	ReadSkillInt(&SacerSkills.Priestess_T5_LightningSurge_DamageBoost[0], "Sacer.ini", "Priestess_T5_LightningSurge_DamageBoost");
	ReadSkillInt(&SacerSkills.Priestess_T5_LightningSurge_Area[0], "Sacer.ini", "Priestess_T5_LightningSurge_Area");
	ReadSkillInt(&SacerSkills.Priestess_T5_LightningSurge_ChargedArea[0], "Sacer.ini", "Priestess_T5_LightningSurge_ChargedArea");
	ReadSkillInt(&SacerSkills.Priestess_T5_LightningSurge_UseMana[0], "Sacer.ini", "Priestess_T5_LightningSurge_UseMana");
	ReadSkillInt(&SacerSkills.Priestess_T5_HeavenlyLight_AttackRateReduction[0], "Sacer.ini", "Priestess_T5_HeavenlyLight_AttackRateReduction");
	ReadSkillInt(&SacerSkills.Priestess_T5_HeavenlyLight_Duration[0], "Sacer.ini", "Priestess_T5_HeavenlyLight_Duration");
	ReadSkillInt(&SacerSkills.Priestess_T5_HeavenlyLight_Area[0], "Sacer.ini", "Priestess_T5_HeavenlyLight_Area");
	ReadSkillInt(&SacerSkills.Priestess_T5_HeavenlyLight_UseMana[0], "Sacer.ini", "Priestess_T5_HeavenlyLight_UseMana");
	ReadSkillInt(&SacerSkills.Priestess_T5_Consecration_DefenseBoost[0], "Sacer.ini", "Priestess_T5_Consecration_DefenseBoost");
	ReadSkillInt(&SacerSkills.Priestess_T5_Consecration_Absorption[0], "Sacer.ini", "Priestess_T5_Consecration_Absorption");
	ReadSkillInt(&SacerSkills.Priestess_T5_Consecration_UseMana[0], "Sacer.ini", "Priestess_T5_Consecration_UseMana");

	ReadSkillInt(&MageSkills.Agony_UseMana[0], "Mage.ini", "Agony_UseMana");
	ReadSkillInt(&MageSkills.Agony_ConvLife[0], "Mage.ini", "Agony_ConvLife");
	ReadSkillInt(&MageSkills.Agony_RecoveryMana[0], "Mage.ini", "Agony_RecoveryMana");
	
	ReadSkillInt(&MageSkills.FireBolt_UseMana[0], "Mage.ini", "FireBolt_UseMana");
	ReadSkillInt(&MageSkills.FireBolt_DamagePercent[0], "Mage.ini", "FireBolt_DamagePercent");

	ReadSkillInt(&MageSkills.Zenith_UseMana[0], "Mage.ini", "Zenith_UseMana");
	ReadSkillInt(&MageSkills.Zenith_Element[0], "Mage.ini", "Zenith_Element");
	ReadSkillInt(&MageSkills.Zenith_Time[0], "Mage.ini", "Zenith_Time");

	ReadSkillInt(&MageSkills.FireBall_UseMana[0], "Mage.ini", "FireBall_UseMana");
	ReadSkillInt(&MageSkills.FireBall_DamagePercent[0], "Mage.ini", "FireBall_DamagePercent");
	ReadSkillInt(&MageSkills.FireBall_Area[0], "Mage.ini", "FireBall_Area");
	ReadSkillInt(&MageSkills.FireBall_Range[0], "Mage.ini", "FireBall_Range");

	ReadSkillInt(&MageSkills.Mental_Mastery_Mana[0], "Mage.ini", "Mental_Mastery_Mana");

	ReadSkillInt(&MageSkills.Watornado_DamagePercent[0], "Mage.ini", "Watornado_DamagePercent");
	ReadSkillInt(&MageSkills.Watornado_Area[0], "Mage.ini", "Watornado_Area");
	ReadSkillInt(&MageSkills.Watornado_Range[0], "Mage.ini", "Watornado_Range");
	ReadSkillInt(&MageSkills.Watornado_Mana[0], "Mage.ini", "Watornado_Mana");

	ReadSkillInt2(MageSkills.Enchant_Weapon_Damage_Ice, "Mage.ini", "Enchant_Weapon_Damage_Ice");
	ReadSkillInt2(MageSkills.Enchant_Weapon_Damage_Fire, "Mage.ini", "Enchant_Weapon_Damage_Fire");
	ReadSkillInt2(MageSkills.Enchant_Weapon_Damage_Lightning, "Mage.ini", "Enchant_Weapon_Damage_Lightning");
	ReadSkillInt(&MageSkills.Enchant_Weapon_Damage[0], "Mage.ini", "Enchant_Weapon_Damage");
	ReadSkillInt(&MageSkills.Enchant_Weapon_Time[0], "Mage.ini", "Enchant_Weapon_Time");
	ReadSkillInt(&MageSkills.Enchant_Weapon_Mana[0], "Mage.ini", "Enchant_Weapon_Mana");

	ReadSkillInt(&MageSkills.Dead_Ray_Damage_Percent[0], "Mage.ini", "Dead_Ray_Damage_Percent");
	ReadSkillInt(&MageSkills.Dead_Ray_Range[0], "Mage.ini", "Dead_Ray_Range");
	ReadSkillInt(&MageSkills.Dead_Ray_Mana[0], "Mage.ini", "Dead_Ray_Mana");

	ReadSkillInt(&MageSkills.Energy_Shield_DecDamage[0], "Mage.ini", "Energy_Shield_DecDamage");
	ReadSkillInt(&MageSkills.Energy_Shield_Time[0], "Mage.ini", "Energy_Shield_Time");
	ReadSkillInt(&MageSkills.Energy_Shield_UseMana[0], "Mage.ini", "Energy_Shield_UseMana");

	ReadSkillInt(&MageSkills.Diastrophism_Damage_Percent[0], "Mage.ini", "Diastrophism_Damage_Percent");
	ReadSkillInt(&MageSkills.Diastrophism_Area[0], "Mage.ini", "Diastrophism_Area");
	ReadSkillInt(&MageSkills.Diastrophism_UseMana[0], "Mage.ini", "Diastrophism_UseMana");

	ReadSkillFloat(&MageSkills.Spirit_Elemental_RegenMana[0], "Mage.ini", "Spirit_Elemental_RegenMana");
	ReadSkillInt(&MageSkills.Spirit_Elemental_Damage[0], "Mage.ini", "Spirit_Elemental_Damage");
	ReadSkillInt(&MageSkills.Spirit_Elemental_Time[0], "Mage.ini", "Spirit_Elemental_Time");
	ReadSkillInt(&MageSkills.Spirit_Elemental_UseMana[0], "Mage.ini", "Spirit_Elemental_UseMana");

	ReadSkillInt2(MageSkills.Dancing_Sword_IceDamage, "Mage.ini", "Dancing_Sword_IceDamage");
	ReadSkillInt2(MageSkills.Dancing_Sword_FireDamage, "Mage.ini", "Dancing_Sword_FireDamage");
	ReadSkillInt2(MageSkills.Dancing_Sword_Damage, "Mage.ini", "Dancing_Sword_Damage");
	ReadSkillInt(&MageSkills.Dancing_Sword_AttackDelay[0], "Mage.ini", "Dancing_Sword_AttackDelay");
	ReadSkillInt(&MageSkills.Dancing_Sword_Time[0], "Mage.ini", "Dancing_Sword_Time");
	ReadSkillInt(&MageSkills.Dancing_Sword_UseMana[0], "Mage.ini", "Dancing_Sword_UseMana");

	ReadSkillInt2(MageSkills.Fire_Elemental_Damage, "Mage.ini", "Fire_Elemental_Damage");
	ReadSkillInt(&MageSkills.Fire_Elemental_Hit[0], "Mage.ini", "Fire_Elemental_Hit");
	ReadSkillInt(&MageSkills.Fire_Elemental_Life[0], "Mage.ini", "Fire_Elemental_Life");
	ReadSkillInt(&MageSkills.Fire_Elemental_UseMana[0], "Mage.ini", "Fire_Elemental_UseMana");

	ReadSkillInt(&MageSkills.Flame_Wave_FireDamage_Percent[0], "Mage.ini", "Flame_Wave_FireDamage_Percent");
	ReadSkillInt(&MageSkills.Flame_Wave_Area[0], "Mage.ini", "Flame_Wave_Area");
	ReadSkillInt(&MageSkills.Flame_Wave_UseMana[0], "Mage.ini", "Flame_Wave_UseMana");

	ReadSkillInt(&MageSkills.Distortion_SpeedSubPercent[0], "Mage.ini", "Distortion_SpeedSubPercent");
	ReadSkillInt(&MageSkills.Distortion_DamageSubPercent[0], "Mage.ini", "Distortion_DamageSubPercent");
	ReadSkillInt(&MageSkills.Distortion_Area[0], "Mage.ini", "Distortion_Area");
	ReadSkillInt(&MageSkills.Distortion_Time[0], "Mage.ini", "Distortion_Time");
	ReadSkillInt(&MageSkills.Distortion_UseMana[0], "Mage.ini", "Distortion_UseMana");


	ReadSkillInt(&MageSkills.Meteo_Damage_Percent[0], "Mage.ini", "Meteo_Damage_Percent");
	ReadSkillInt(&MageSkills.Meteo_Area[0], "Mage.ini", "Meteo_Area");
	ReadSkillInt(&MageSkills.Meteo_UseMana[0], "Mage.ini", "Meteo_UseMana");

	memcpy(MageSkills.Magician_T5_WizardTrance_BlockBoost, Magician_T5_WizardTrance_BlockBoost, sizeof(Magician_T5_WizardTrance_BlockBoost));
	memcpy(MageSkills.Magician_T5_WizardTrance_AttackRateReduction, Magician_T5_WizardTrance_AttackRateReduction, sizeof(Magician_T5_WizardTrance_AttackRateReduction));
	memcpy(MageSkills.Magician_T5_WizardTrance_Duration, Magician_T5_WizardTrance_Duration, sizeof(Magician_T5_WizardTrance_Duration));
	memcpy(MageSkills.Magician_T5_WizardTrance_UseMana, Magician_T5_WizardTrance_UseMana, sizeof(Magician_T5_WizardTrance_UseMana));
	memcpy(MageSkills.Magician_T5_StoneSkin_ImmortalWindow, Magician_T5_StoneSkin_ImmortalWindow, sizeof(Magician_T5_StoneSkin_ImmortalWindow));
	memcpy(MageSkills.Magician_T5_StoneSkin_Duration, Magician_T5_StoneSkin_Duration, sizeof(Magician_T5_StoneSkin_Duration));
	memcpy(MageSkills.Magician_T5_StoneSkin_RunSpeedReduction, Magician_T5_StoneSkin_RunSpeedReduction, sizeof(Magician_T5_StoneSkin_RunSpeedReduction));
	memcpy(MageSkills.Magician_T5_StoneSkin_UseMana, Magician_T5_StoneSkin_UseMana, sizeof(Magician_T5_StoneSkin_UseMana));
	memcpy(MageSkills.Magician_T5_RedRay_DamageBoost, Magician_T5_RedRay_DamageBoost, sizeof(Magician_T5_RedRay_DamageBoost));
	memcpy(MageSkills.Magician_T5_RedRay_AddRange, Magician_T5_RedRay_AddRange, sizeof(Magician_T5_RedRay_AddRange));
	memcpy(MageSkills.Magician_T5_RedRay_UseMana, Magician_T5_RedRay_UseMana, sizeof(Magician_T5_RedRay_UseMana));
	memcpy(MageSkills.Magician_T5_Cataclysm_DamageBoost, Magician_T5_Cataclysm_DamageBoost, sizeof(Magician_T5_Cataclysm_DamageBoost));
	memcpy(MageSkills.Magician_T5_Cataclysm_Duration, Magician_T5_Cataclysm_Duration, sizeof(Magician_T5_Cataclysm_Duration));
	memcpy(MageSkills.Magician_T5_Cataclysm_MPDrain, Magician_T5_Cataclysm_MPDrain, sizeof(Magician_T5_Cataclysm_MPDrain));
	memcpy(MageSkills.Magician_T5_Cataclysm_SPDrain, Magician_T5_Cataclysm_SPDrain, sizeof(Magician_T5_Cataclysm_SPDrain));
	memcpy(MageSkills.Magician_T5_Cataclysm_UseMana, Magician_T5_Cataclysm_UseMana, sizeof(Magician_T5_Cataclysm_UseMana));
	ReadSkillInt(&MageSkills.Magician_T5_WizardTrance_BlockBoost[0], "Mage.ini", "Magician_T5_WizardTrance_BlockBoost");
	ReadSkillInt(&MageSkills.Magician_T5_WizardTrance_AttackRateReduction[0], "Mage.ini", "Magician_T5_WizardTrance_AttackRateReduction");
	ReadSkillInt(&MageSkills.Magician_T5_WizardTrance_Duration[0], "Mage.ini", "Magician_T5_WizardTrance_Duration");
	ReadSkillInt(&MageSkills.Magician_T5_WizardTrance_UseMana[0], "Mage.ini", "Magician_T5_WizardTrance_UseMana");
	ReadSkillInt(&MageSkills.Magician_T5_StoneSkin_ImmortalWindow[0], "Mage.ini", "Magician_T5_StoneSkin_ImmortalWindow");
	ReadSkillInt(&MageSkills.Magician_T5_StoneSkin_Duration[0], "Mage.ini", "Magician_T5_StoneSkin_Duration");
	ReadSkillInt(&MageSkills.Magician_T5_StoneSkin_RunSpeedReduction[0], "Mage.ini", "Magician_T5_StoneSkin_RunSpeedReduction");
	ReadSkillInt(&MageSkills.Magician_T5_StoneSkin_UseMana[0], "Mage.ini", "Magician_T5_StoneSkin_UseMana");
	ReadSkillInt(&MageSkills.Magician_T5_RedRay_DamageBoost[0], "Mage.ini", "Magician_T5_RedRay_DamageBoost");
	ReadSkillInt(&MageSkills.Magician_T5_RedRay_AddRange[0], "Mage.ini", "Magician_T5_RedRay_AddRange");
	ReadSkillInt(&MageSkills.Magician_T5_RedRay_UseMana[0], "Mage.ini", "Magician_T5_RedRay_UseMana");
	ReadSkillInt(&MageSkills.Magician_T5_Cataclysm_DamageBoost[0], "Mage.ini", "Magician_T5_Cataclysm_DamageBoost");
	ReadSkillInt(&MageSkills.Magician_T5_Cataclysm_Duration[0], "Mage.ini", "Magician_T5_Cataclysm_Duration");
	ReadSkillInt(&MageSkills.Magician_T5_Cataclysm_MPDrain[0], "Mage.ini", "Magician_T5_Cataclysm_MPDrain");
	ReadSkillInt(&MageSkills.Magician_T5_Cataclysm_SPDrain[0], "Mage.ini", "Magician_T5_Cataclysm_SPDrain");
	ReadSkillInt(&MageSkills.Magician_T5_Cataclysm_UseMana[0], "Mage.ini", "Magician_T5_Cataclysm_UseMana");


	MechSkills.size = sizeof(MECH_INFO);
	MechSkills.code = smTRANSCODE_MECH_INFO;

	if (lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char*)&MechSkills, MechSkills.size, TRUE);

	FighterSkills.size = sizeof(FIGHTER_INFO);
	FighterSkills.code = smTRANSCODE_FIGHTER_INFO;

	if (lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char*)&FighterSkills, FighterSkills.size, TRUE);

	PikeSkills.size = sizeof(PIKE_INFO);
	PikeSkills.code = smTRANSCODE_PIKE_INFO;

	if (lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char*)&PikeSkills, PikeSkills.size, TRUE);

	ArcherSkills.size = sizeof(ARCHER_INFO);
	ArcherSkills.code = smTRANSCODE_ARCHER_INFO;

	if (lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char*)&ArcherSkills, ArcherSkills.size, TRUE);

	KinaSkills.size = sizeof(KINA_INFO);
	KinaSkills.code = smTRANSCODE_KINA_INFO;

	if (lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char*)&KinaSkills, KinaSkills.size, TRUE);

	AtaSkills.size = sizeof(ATA_INFO);
	AtaSkills.code = smTRANSCODE_ATA_INFO;

	if (lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char*)&AtaSkills, AtaSkills.size, TRUE);

	SacerSkills.size = sizeof(SACER_INFO);
	SacerSkills.code = smTRANSCODE_SACER_INFO;

	if (lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char*)&SacerSkills, SacerSkills.size, TRUE);

	MageSkills.size = sizeof(MAGE_INFO);
	MageSkills.code = smTRANSCODE_MAGE_INFO;

	if (lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char*)&MageSkills, MageSkills.size, TRUE);


#endif
	return TRUE;
}


