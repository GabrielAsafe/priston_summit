int smCHAR::SetLoadPattern( char *szName )
{
	Pattern = 0;
	Flag = 1;
	return AddLoaderPattern( (smCHAR *)&Head , szName );

}