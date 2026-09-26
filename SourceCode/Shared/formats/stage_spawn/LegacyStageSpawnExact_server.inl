#include "content/v1/stage_spawn/legacy/LegacyStageSpawnAdapter.h"

int DecodeOpenMonster(char *szCharFile, rsSTG_MONSTER_LIST *lpStgMonList, smCHAR_INFO *lpCharInfoList, int ListCnt)
{
	return summit::content::stage_spawn::DecodeOpenMonsterViaContent(
		szCharFile, lpStgMonList, lpCharInfoList, ListCnt) ? TRUE : FALSE;
}



int GetRandomPos(int Min, int Max)
{
	int smaller = 0;
	int bigger = 0;

	if(Min <= Max)
	{
		smaller = Min;
		bigger = Max;
	}
	else
	{
		smaller = Max;
		bigger = Min;
	}

	int sb = (Max + 1) - Min;

	if(sb == 0)
		return Max;
	else
	{
		int rnd = rand() % sb;
		return Min + rnd;
	}
}


float GetRandomFloatPos(float Min, float Max)
{
	float smaller = 0.0f;
	float bigger = 0.0f;

	if(Min <= Max)
	{
		smaller = Min;
		bigger = Max;
	}
	else
	{
		smaller = Max;
		bigger = Min;
	}


	int sb = (int)((bigger - smaller) * 100.0f);

	if(sb == 0)
		return Max;
	else
	{
		int rnd = rand() % (sb + 1);
		return Min + ((float)((float)rnd / 100.0f));
	}
}


float GetRandomFloatPos2(float Min, float Max)
{
	float smaller = 0.0f;
	float bigger = 0.0f;

	if(Min <= Max)
	{
		smaller = Min;
		bigger = Max;
	}
	else
	{
		smaller = Max;
		bigger = Min;
	}

	int sb = (int)((bigger - smaller) * 10.0f);

	if(sb == 0)
		return Max;
	else
	{
		int rnd = rand() % (sb + 1);

		return Min + ((float)((float)rnd / 10.0f));
	}
}


int GetMiddlePos(int Min, int Max)
{
	int sb;

	sb = (Max - Min) / 2;

	return Min + sb;
}



static int	Random_NormalMode = 0;


int GetRandomPos_Normal(int inMin, int inMax)
{
	int sb, sc;
	int rnd;
	int	Min, Max;

	if(!Random_NormalMode)
		return GetRandomPos(inMin, inMax);

	sc = inMax - inMin;
	sb = sc / 4;

	if(sb == 0 && sc >= 2) sb = 1;

	Min = inMin + sb;
	Max = inMax - sb;

	sb = (Max + 1) - Min;

	if(sb == 0) return Max;

	rnd = rand() % sb;
	return Min + rnd;
}

// Class-specific item requirement revisions are owned by PlayableClassDefinition.

