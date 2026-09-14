// Client-only Priestess/Magician Tier 5 metadata and timer routing from fixes 159-171.

static const char *SummitT5CanonicalTimerPath(DWORD code)
{
	switch(code)
	{
	case SKILL_T5_LETHAL_SIGHT: return "game\\images\\skills\\timers\\Ac5_LethalSight.tga";
	case SKILL_T5_HOLY_CONVICTION: return "game\\images\\skills\\timers\\Kn5_HolyConviction.tga";
	case SKILL_T5_SUMMON_TIGER: return "game\\images\\skills\\timers\\Ata5_SummonTiger.tga";
	case SKILL_T5_ICE_ELEMENTAL: return "game\\images\\skills\\timers\\Pr5_IceElemental.tga";
	case SKILL_T5_WIZARD_TRANCE: return "game\\images\\skills\\timers\\Mg5_WizardTrance.tga";
	}
	return 0;
}

static const char *SummitT5SkillFolder(DWORD code)
{
	switch(code & sinSKILL_MASK1)
	{
	case GROUP_MECHANICIAN: return "Mechanician";
	case GROUP_FIGHTER: return "Fighter";
	case GROUP_PIKEMAN: return "Pikeman";
	case GROUP_ARCHER: return "Archer";
	case GROUP_KNIGHT: return "Knight";
	case GROUP_ATALANTA: return "Atalanta";
	case GROUP_PRIESTESS: return "Priestess";
	case GROUP_MAGICIAN: return "Magician";
	}
	return 0;
}

static int SummitT5ResolveTimerMaterial(DWORD code)
{
	if((code & 0x00FF0000) != CHANGE_JOB5) return 0;

	for(int j = SIN_TIER5_SKILL_START; j < SIN_MAX_USE_SKILL; j++)
	{
		sSKILL *skill = &sinSkill.UseSkill[j];
		if(skill->CODE != code || !skill->FileName[0]) continue;
		if(skill->MatIcon) return skill->MatIcon;

		char path[260] = { 0 };
		const char *canonical = SummitT5CanonicalTimerPath(code);
		if(canonical && GetFileAttributesA(canonical) != INVALID_FILE_ATTRIBUTES)
			strcpy_s(path, canonical);
		else
		{
			const char *folder = SummitT5SkillFolder(code);
			if(!folder) return 0;
			wsprintf(path, "image//Sinimage//skill//%s//Button//%s.bmp", folder, skill->FileName);
		}

		skill->MatIcon = CreateTextureMaterial(path, 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
		if(skill->MatIcon) ReadTextures();
		return skill->MatIcon;
	}
	return 0;
}

static void InitSummitTier5PriestessSkillMetadata()
{
	if(sinChar->JOB_CODE != JOBCODE_PRIESTESS) return;
	sSKILL *skill = &sinSkill.UseSkill[17];
	if(skill->CODE != SKILL_T5_ICE_ELEMENTAL) ZeroMemory(skill, sizeof(sSKILL));
	strcpy_s(skill->sSkillName, "Ice Elemental"); strcpy_s(skill->FileName, "MP100 Ice_Elemental"); skill->CODE=SKILL_T5_ICE_ELEMENTAL; skill->Flag=1;
	skill->Skill_Info.RequireLevel=100; skill->Skill_Info.UseStamina[0]=45; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=169; skill->Skill_Info.RequireMastery[1]=0; skill->Skill_Info.Element[0]=2; skill->Skill_Info.Element[1]=0; skill->Skill_Info.Element[2]=0;
	for(int i=0;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Ice Elemental"); strcpy_s(skill->Skill_Info.SkillDoc,"Summons an Ice Elemental to help you in battle.");
	skill->Skill_Info.FuncPointer=F_Priestess_T5_Ice_Elemental; skill->Skill_Info.CODE=SKILL_T5_ICE_ELEMENTAL; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Priestess_T5_IceElemental_UseMana; skill->Skill_Info.SkillNum=16;

	skill=&sinSkill.UseSkill[18]; if(skill->CODE != SKILL_T5_LIGHTNING_SURGE) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Lightning Surge"); strcpy_s(skill->FileName,"MP103 Lightning_Surge"); skill->CODE=SKILL_T5_LIGHTNING_SURGE; skill->Flag=1; skill->Skill_Info.RequireLevel=103; skill->Skill_Info.UseStamina[0]=50; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=180; skill->Skill_Info.RequireMastery[1]=4; skill->Skill_Info.Element[0]=2; skill->Skill_Info.UseWeaponCode[0]=sinWM1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Lightning Surge"); strcpy_s(skill->Skill_Info.SkillDoc,"Discharges a great sum of electricity into your target.");
	skill->Skill_Info.FuncPointer=F_Priestess_T5_Lightning_Surge; skill->Skill_Info.CODE=SKILL_T5_LIGHTNING_SURGE; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Priestess_T5_LightningSurge_UseMana; skill->Skill_Info.SkillNum=17;

	skill=&sinSkill.UseSkill[19]; if(skill->CODE != SKILL_T5_HEAVENLY_LIGHT) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Heavenly Light"); strcpy_s(skill->FileName,"MP106 Heavenly_Light"); skill->CODE=SKILL_T5_HEAVENLY_LIGHT; skill->Flag=1; skill->Skill_Info.RequireLevel=106; skill->Skill_Info.UseStamina[0]=54; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=203; skill->Skill_Info.RequireMastery[1]=0; skill->Skill_Info.Element[0]=2; skill->Skill_Info.UseWeaponCode[0]=sinWM1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Heavenly Light"); strcpy_s(skill->Skill_Info.SkillDoc,"Summons a divine light to purify allies and blind enemies.");
	skill->Skill_Info.FuncPointer=F_Priestess_T5_Heavenly_Light; skill->Skill_Info.CODE=SKILL_T5_HEAVENLY_LIGHT; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Priestess_T5_HeavenlyLight_UseMana; skill->Skill_Info.SkillNum=18;

	skill=&sinSkill.UseSkill[20]; if(skill->CODE != SKILL_T5_CONSECRATION) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Consecration"); strcpy_s(skill->FileName,"MP110 Consecration"); skill->CODE=SKILL_T5_CONSECRATION; skill->Flag=1; skill->Skill_Info.RequireLevel=110; skill->Skill_Info.UseStamina[0]=60; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=203; skill->Skill_Info.RequireMastery[1]=0; skill->Skill_Info.Element[0]=2; skill->Skill_Info.UseWeaponCode[0]=sinWM1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Consecration"); strcpy_s(skill->Skill_Info.SkillDoc,"Blesses party members with God's protection.");
	skill->Skill_Info.FuncPointer=F_Priestess_T5_Consecration; skill->Skill_Info.CODE=SKILL_T5_CONSECRATION; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Priestess_T5_Consecration_UseMana; skill->Skill_Info.SkillNum=19;
}

static void InitSummitTier5MagicianSkillMetadata()
{
	if(sinChar->JOB_CODE != JOBCODE_MAGICIAN) return;
	sSKILL *skill=&sinSkill.UseSkill[17];
	if(skill->CODE != SKILL_T5_WIZARD_TRANCE) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Wizard's Trance"); strcpy_s(skill->FileName,"MM100 WizardTrance"); skill->CODE=SKILL_T5_WIZARD_TRANCE; skill->Flag=1; skill->Skill_Info.RequireLevel=100; skill->Skill_Info.UseStamina[0]=54; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=203; skill->Skill_Info.RequireMastery[1]=0; skill->Skill_Info.Element[0]=2; skill->Skill_Info.UseWeaponCode[0]=sinWM1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Wizard's Trance"); strcpy_s(skill->Skill_Info.SkillDoc,"Increases Orb effectiveness by focusing on defense, at the cost of attack rate.");
	skill->Skill_Info.FuncPointer=F_Magician_T5_Wizard_Trance; skill->Skill_Info.CODE=SKILL_T5_WIZARD_TRANCE; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Magician_T5_WizardTrance_UseMana; skill->Skill_Info.SkillNum=16;

	skill=&sinSkill.UseSkill[18]; if(skill->CODE != SKILL_T5_STONE_SKIN) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Stone Skin"); strcpy_s(skill->FileName,"MM103 Stone_Skin"); skill->CODE=SKILL_T5_STONE_SKIN; skill->Flag=1; skill->Skill_Info.RequireLevel=103; skill->Skill_Info.UseStamina[0]=45; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=203; skill->Skill_Info.RequireMastery[1]=0; skill->Skill_Info.Element[0]=2; skill->Skill_Info.UseWeaponCode[0]=sinWM1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Stone Skin"); strcpy_s(skill->Skill_Info.SkillDoc,"Becomes immune to damage briefly, while reducing movement speed.");
	skill->Skill_Info.FuncPointer=F_Magician_T5_Stone_Skin; skill->Skill_Info.CODE=SKILL_T5_STONE_SKIN; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Magician_T5_StoneSkin_UseMana; skill->Skill_Info.SkillNum=17;

	skill=&sinSkill.UseSkill[19]; if(skill->CODE != SKILL_T5_RED_RAY) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Elemental Ray"); strcpy_s(skill->FileName,"MM106 RedRay"); skill->CODE=SKILL_T5_RED_RAY; skill->Flag=1; skill->Skill_Info.RequireLevel=106; skill->Skill_Info.UseStamina[0]=50; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=70; skill->Skill_Info.RequireMastery[1]=3; skill->Skill_Info.Element[0]=2; skill->Skill_Info.UseWeaponCode[0]=sinWM1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Elemental Ray"); strcpy_s(skill->Skill_Info.SkillDoc,"Launches an elemental ray that defeats the enemy.");
	skill->Skill_Info.FuncPointer=F_Magician_T5_Red_Ray; skill->Skill_Info.CODE=SKILL_T5_RED_RAY; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Magician_T5_RedRay_UseMana; skill->Skill_Info.SkillNum=18;

	skill=&sinSkill.UseSkill[20]; if(skill->CODE != SKILL_T5_CATACLYSM) ZeroMemory(skill,sizeof(sSKILL));
	strcpy_s(skill->sSkillName,"Cataclysm"); strcpy_s(skill->FileName,"MM110 Cataclysm"); skill->CODE=SKILL_T5_CATACLYSM; skill->Flag=1; skill->Skill_Info.RequireLevel=110; skill->Skill_Info.UseStamina[0]=60; skill->Skill_Info.UseStamina[1]=2; skill->Skill_Info.RequireMastery[0]=180; skill->Skill_Info.RequireMastery[1]=4; skill->Skill_Info.Element[0]=2; skill->Skill_Info.UseWeaponCode[0]=sinWM1; for(int i=1;i<8;i++) skill->Skill_Info.UseWeaponCode[i]=0; strcpy_s(skill->Skill_Info.SkillName,"Cataclysm"); strcpy_s(skill->Skill_Info.SkillDoc,"Invokes the wrath of nature in a series of powerful attacks.");
	skill->Skill_Info.FuncPointer=F_Magician_T5_Cataclysm; skill->Skill_Info.CODE=SKILL_T5_CATACLYSM; skill->Skill_Info.USECODE=SIN_SKILL_USE_RIGHT; skill->Skill_Info.UseMana=Magician_T5_Cataclysm_UseMana; skill->Skill_Info.SkillNum=19;
}

void LegacySinSkillPolicy::InitAdditionalTier5Metadata()
{
	InitSummitTier5PriestessSkillMetadata();
	InitSummitTier5MagicianSkillMetadata();
}

void LegacySinSkillPolicy::OnContinueIconResolved(int index, sSKILL *skill)
{
	SummitT5TimerProbeSkill("icon-material-assigned", index, skill);
}

int LegacySinSkillPolicy::ResolveTier5ContinueIcon(int index, sSKILL *skill)
{
	if(!skill) return FALSE;
	skill->MatIcon = SummitT5ResolveTimerMaterial(skill->CODE);
	if(!skill->MatIcon) return FALSE;
	SummitT5TimerProbeSkill("icon-material-assigned-t5", index, skill);
	return TRUE;
}
