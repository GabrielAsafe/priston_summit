int	LoadOftenMeshPattern( char *szFileName )
{
	DynTempPat[DynTempPatCnt] = smPattern.LoadCharactor( szFileName );
	if ( DynTempPat[DynTempPatCnt] )
	{
		DynTempPat[DynTempPatCnt]->UseCount = 99;
		DynTempPatCnt++;
		return TRUE;
	}

	return FALSE;
}