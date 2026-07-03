#include "sinbaram/sinLinkHeader.h"
#include "KillHandler.h"

cKILL cKill;

void cKILL::Init()
{
	CurrentStreak = 0;
	for (int i = 1; i <= 8; i++)
	{
		char path[255];
		wsprintf(path, "game\\kill\\class%d.png", i);
		lpClass[i-1] = LoadDibSurfaceOffscreen(path);
	}
	
	for (int i = 1; i <= 9; i++)
	{
		char path[255];
		wsprintf(path, "game\\kill\\kt%d.png", i);
		lpKills[i-1] = LoadDibSurfaceOffscreen(path);
	}
}

void cKILL::Draw()
{
	DWORD Time = GetCurrentTime() - sKillInfo.initTime;
	if (Time < sKillInfo.Duration)
	{
		DrawSprite((WinSizeX - 400) / 2, 150, lpKills[sKillInfo.Kind - 1], 0, 0, 400, 120);
		DrawSprite(((WinSizeX - 400) / 2) - 90, 150 + ((120 - 90) / 2), lpClass[lpCurPlayer->smCharInfo.JOB_CODE - 1], 0, 0, 90, 90);
		DrawSprite((WinSizeX / 2) + 200, 150 + ((120 - 90) / 2), lpClass[sKillInfo.Class - 1], 0, 0, 90, 90);
	}
}

void cKILL::recvKill(int Streak, int Class)
{
	CurrentStreak++;

	bool showstreak = false;

	if (Streak == 1 || Streak > 5 || (sKillInfo.initTime && sKillInfo.initTime + 20000 < GetCurrentTime()))
	{
		/*if (CurrentStreak == 3)
		{
			sKillInfo.Kind = KILLINGSPREE;
			showstreak = true;
		}
		else 
		else if (CurrentStreak == 7)
		{
			sKillInfo.Kind = LEGENDARY;
			showstreak = true;
		}*/

		if (CurrentStreak == 5)
		{
			sKillInfo.Kind = RAMPAGE;
			showstreak = true;
		}

		if (CurrentStreak >= 6)
		{
			sKillInfo.Kind = UNSTOPPABLE;
			showstreak = true;
		}
	}
	else
	{
		if (Streak == 2)
		{
			sKillInfo.Kind = DOUBLEKILL;
			showstreak = true;
		}
		else if (Streak == 3)
		{
			sKillInfo.Kind = TRIPLEKILL;
			showstreak = true;
		}
		else if (Streak == 4)
		{
			sKillInfo.Kind = QUADRAKILL;
			showstreak = true;
		}
		else if (Streak == 5)
		{
			sKillInfo.Kind = PENTAKILL;
			showstreak = true;
		}
	}

	if (showstreak)
	{
		sKillInfo.initTime = GetCurrentTime();
		sKillInfo.Duration = 4000;
		sKillInfo.Class = Class;

		PlayWav2(12 + sKillInfo.Kind, 0, 400, 4410);
	}
}

void cKILL::handleDeath()
{
	CurrentStreak = 0;
}




