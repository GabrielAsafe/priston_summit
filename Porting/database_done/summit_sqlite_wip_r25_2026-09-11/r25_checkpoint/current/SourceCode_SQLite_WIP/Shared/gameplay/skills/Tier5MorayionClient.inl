// Client-only Priestess/Magician Tier 5 activation helpers imported from fixes 159-171.

int F_Priestess_T5_Ice_Elemental()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	// EU CIceElementalHandler leaves its timer registration disabled; the summon lifetime is server-authoritative.
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Priestess_T5_Lightning_Surge()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Priestess_T5_Heavenly_Light()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Priestess_T5_Consecration()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	// EU exposes Defense/Absorption but not the legacy duration handler. Summit uses a
	// documented 300-second compatibility duration until a canonical duration is recovered.
	pUseSkill->UseTime = 300;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar();
	return TRUE;
}

int F_Magician_T5_Wizard_Trance()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	// EU Wizard Trance requires a Wand in the right hand and an Orb in the left hand.
	if(!sInven[0].ItemIndex || !sInven[1].ItemIndex) return FALSE;
	if((cInvenTory.InvenItem[sInven[0].ItemIndex - 1].CODE & sinITEM_MASK2) != sinWM1) return FALSE;
	if((cInvenTory.InvenItem[sInven[1].ItemIndex - 1].CODE & sinITEM_MASK2) != sinOM1) return FALSE;
	pUseSkill->UseTime = Magician_T5_WizardTrance_Duration[pUseSkill->Point - 1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Magician_T5_Stone_Skin()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	pUseSkill->UseTime = Magician_T5_StoneSkin_Duration[pUseSkill->Point - 1];
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Magician_T5_Red_Ray()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Magician_T5_Cataclysm()
{
	if(!pUseSkill || pUseSkill->Point < 1 || pUseSkill->Point > 10 || !pUseSkill->Skill_Info.UseMana) return FALSE;
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
