int LoadOftenNpc()
{
#ifdef _W_SERVER
	LoadFolder_Ini("Char//Npc", "*.inx", "ini", TRUE);
#else
	int cnt = 0;
	while (1)
	{
		if (!szOftenNPC[cnt])
			break;
		LoadOftenMeshPattern(szOftenNPC[cnt]);
		cnt++;
	}
#endif

	return TRUE;
}
