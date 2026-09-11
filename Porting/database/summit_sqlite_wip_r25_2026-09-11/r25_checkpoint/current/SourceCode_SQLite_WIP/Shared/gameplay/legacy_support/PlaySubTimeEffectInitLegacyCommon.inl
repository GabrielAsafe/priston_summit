int InitTimeEffect()
{
	int cnt;
	for (cnt = 0; cnt < TIME_EFFECT_BUFF_MAX; cnt++)
	{
		TimeEffectBuff[cnt].dwCode = 0;
	}

	return TRUE;
}
