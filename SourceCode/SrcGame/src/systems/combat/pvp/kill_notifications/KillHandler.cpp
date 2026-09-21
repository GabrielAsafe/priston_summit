#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/combat/pvp/kill_notifications/KillHandler.h"
#include "gameplay/classes/v2/services/ClassService.h"

cKILL cKill;

void cKILL::Init()
{
	CurrentStreak = 0;
	lpClass.clear();
	const gameplay::classes::ClassService& classService = gameplay::classes::DefaultClassService();
	const std::vector<int> enabledJobs = classService.EnabledJobCodes();
	for (std::vector<int>::const_iterator jobCode = enabledJobs.begin(); jobCode != enabledJobs.end(); ++jobCode)
	{
		const std::string* iconPath = classService.KillNotificationIcon(*jobCode);
		if(iconPath && !iconPath->empty())
			lpClass[*jobCode] = LoadDibSurfaceOffscreen(const_cast<char*>(iconPath->c_str()));
	}
	
	for (int i = 1; i <= 9; i++)
	{
		char path[255];
		wsprintf(path, "game//kill//kt%d.png", i);
		lpKills[i-1] = LoadDibSurfaceOffscreen(path);
	}
}

LPDIRECT3DTEXTURE9 cKILL::GetClassIcon(int legacyJobCode) const
{
	std::map<int, LPDIRECT3DTEXTURE9>::const_iterator it = lpClass.find(legacyJobCode);
	return it != lpClass.end() ? it->second : 0;
}

void cKILL::Draw()
{
	DWORD Time = GetCurrentTime() - sKillInfo.initTime;
	if (Time < sKillInfo.Duration)
	{
		DrawSprite((WinSizeX - 400) / 2, 150, lpKills[sKillInfo.Kind - 1], 0, 0, 400, 120);
		LPDIRECT3DTEXTURE9 localClassIcon = GetClassIcon(lpCurPlayer->smCharInfo.JOB_CODE);
		LPDIRECT3DTEXTURE9 remoteClassIcon = GetClassIcon(sKillInfo.Class);
		if(localClassIcon)
			DrawSprite(((WinSizeX - 400) / 2) - 90, 150 + ((120 - 90) / 2), localClassIcon, 0, 0, 90, 90);
		if(remoteClassIcon)
			DrawSprite((WinSizeX / 2) + 200, 150 + ((120 - 90) / 2), remoteClassIcon, 0, 0, 90, 90);
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




