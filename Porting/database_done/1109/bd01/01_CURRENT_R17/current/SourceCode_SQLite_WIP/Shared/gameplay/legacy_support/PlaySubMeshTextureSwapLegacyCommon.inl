int CheckOftenMeshTextureSwap()
{
	return TRUE;

	for (int cnt = 0; cnt < DynTempPatCnt; cnt++)
	{
		if (DynTempPat[cnt] && DynTempPat[cnt]->Pat && DynTempPat[cnt]->Pat->smMaterialGroup)
		{
			DynTempPat[cnt]->Pat->smMaterialGroup->CheckMatreialTextureSwap();
		}
	}
	return TRUE;
}
