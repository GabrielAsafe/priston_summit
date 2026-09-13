int sinCheckContinueSkill()
{


	return TRUE;
}

int sinSwitchSkill(sSKILL *pSkill)
{


	return TRUE;
}




int F_Sword_Blast()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_HolyBody()
{
	pUseSkill->UseTime = HolyBody_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	sinSwitchSkill(pUseSkill);
	return TRUE;
}
int F_Physical_Traning()
{

	return TRUE;
}
int F_Double_Crash()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_S_Strike()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Farina()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_D_Mastery()
{


	return TRUE;
}
int F_V_Spear()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Healing()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_HolyBolt()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_M_Spark()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_HolyMind()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Agony()
{
	short TempLife, TempLife2, TempMana;
	sinUseManaStamina(pUseSkill);
	TempLife = (short)(((float)sinChar->Life[1] / 100.0f)*(float)Agony_ConvLife[pUseSkill->Point - 1]);
	TempMana = (short)(((float)sinChar->Mana[1] / 100.0f)*(float)Agony_RecoveryMana[pUseSkill->Point - 1]);
	if((sinGetLife() - TempLife) > 0)
	{
		TempLife2 = sinGetLife() - TempLife;
		sinSetLife(TempLife2);
		TempMana = sinGetMana() + TempMana;
		sinSetMana(TempMana);

	};




	return TRUE;
}
int F_FireBolt()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Zenith()
{

	pUseSkill->UseTime = Zenith_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_ZenithFromServer(int Time)
{
	for(int i = 0; i < MAX_CONTINUE_SKILL; i++)
	{
		if(!ContinueSkill[i].Flag)
		{
			ContinueSkill[i].Flag = 1;
			ContinueSkill[i].CODE = SKILL_ZENITH;
			ContinueSkill[i].Time[1] = Time;
			ContinueSkill[i].PartyFlag = 1;
			break;

		}
	}
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_FireBall()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Holy_Valor()
{

	pUseSkill->UseTime = Holy_Valor_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	sinSwitchSkill(pUseSkill);
	return TRUE;
}

int F_Brandish()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Piercing()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Drastic_Spirit()
{
	pUseSkill->UseTime = Drastic_Spirit_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	sinSwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Windy()
{

	pUseSkill->UseTime = Windy_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Twist_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Soul_Sucker()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Fire_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Meditation()
{
	return TRUE;
}

int F_Divine_Lightning()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Holy_Reflection()
{
	pUseSkill->UseTime = Holy_Reflection_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Grand_Healing()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Mental_Mastery()
{
	return TRUE;
}

int F_Watornado()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Enchant_Weapon()
{



	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Dead_Ray()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}



int F_S_Mastery()
{

	return TRUE;
}

int F_D_Inhalation()
{
	pUseSkill->UseTime = D_Inhalation_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Holy_Incantation()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Grand_Cross()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;

}


int F_Split_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_T_Of_Valhalla()
{


	sinUseManaStamina(pUseSkill);



	if(!pUseSkill->SkillTaget_CODE)
		SetT_Of_ValhallaFlag2 = 1;


	return TRUE;
}
int F_Lightning_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Storm_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Vigor_Ball()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Resurrection()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Extinction()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Virtual_Life()
{


	sinUseManaStamina(pUseSkill);


	return TRUE;
}


int F_Energy_Shield()
{
	pUseSkill->UseTime = Energy_Shield_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Diastrophism()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Spirit_Elemental()
{
	pUseSkill->UseTime = Spirit_Elemental_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_D_Sword()
{
	pUseSkill->UseTime = Dancing_Sword_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}


int F_Divine_Piercing()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Godly_Shield()
{
	pUseSkill->UseTime = Godly_Shield_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	return TRUE;
}
int F_God_Bless()
{
	pUseSkill->UseTime = God_Bless_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Sword_Of_Justice()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Hall_Of_Valhalla()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_X_Rage()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Frost_Javelin()
{
	pUseSkill->UseTime = Frost_Javelin_Time[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}
int F_Vengeance()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Glacial_Spike()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Regeneration_Field()
{

	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Chain_Lightning()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Summon_Muspell()
{
	pUseSkill->UseTime = Summon_Muspell_Time[pUseSkill->Point - 1];
	sinUseManaStamina(pUseSkill);
	sinContinueSkillSet(pUseSkill);
	return TRUE;
}


int F_Fire_Elemental()
{
	pUseSkill->UseTime = 6 * 60;;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SummonSkillCodeIndex[1] = 1;
	return TRUE;
}
int F_Flame_Wave()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Distortion()
{
	pUseSkill->UseTime = Distortion_Time[pUseSkill->Point - 1];

	sinUseManaStamina(pUseSkill);


	return TRUE;
}
int F_Meteo()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


// Summit Tier 5 bugfix stack 091-158

int F_Atalanta_T5_Summon_Tiger()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	pUseSkill->UseTime = Atalanta_T5_SummonTiger_Duration[pUseSkill->Point - 1];
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Atalanta_T5_Golden_Apple() { return TRUE; }

int F_Atalanta_T5_Plague_Javelin()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Atalanta_T5_Combo_Javelin()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Knight_T5_Holy_Conviction()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	pUseSkill->UseTime = Knight_T5_HolyConviction_Duration[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Knight_T5_Divine_Inquisition()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Knight_T5_Glorious_Shield()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	pUseSkill->UseTime = Knight_T5_GloriousShield_Duration[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Knight_T5_Divine_Cross()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
