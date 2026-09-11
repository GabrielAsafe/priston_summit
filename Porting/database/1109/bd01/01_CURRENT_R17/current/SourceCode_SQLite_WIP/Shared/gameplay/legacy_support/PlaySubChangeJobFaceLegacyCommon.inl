int ChangeJobFace()
{
	int cnt1;


	cnt1 = 0;

	StartSkill(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, SKILL_UP1);
	SkillPlaySound(SKILL_SOUND_LEARN, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);

	CheckCharForm();


	ChangeModelSkin(lpCurPlayer, 0, (lpCurPlayer->smCharInfo.ChangeJob > CHARACTER_RANK4) ? CHARACTER_RANK4 : lpCurPlayer->smCharInfo.ChangeJob, 0, 0);
	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	ReformCharForm();
	SendSetObjectSerial(lpCurPlayer->dwObjectSerial);

	return FALSE;
}
