#pragma once

#include <map>

class cKILL
{
public:
	void Init();
	void Draw();
	void recvKill(int Streak, int Class);
	void handleDeath();

private:
	std::map<int, LPDIRECT3DTEXTURE9> lpClass;
	LPDIRECT3DTEXTURE9 lpKills[9];
	LPDIRECT3DTEXTURE9 GetClassIcon(int legacyJobCode) const;

	enum KILLS
	{
		DOUBLEKILL = 1,
		TRIPLEKILL,
		QUADRAKILL,
		PENTAKILL,
		SHUTDOWN,
		RAMPAGE,
		UNSTOPPABLE,
		LEGENDARY,
		KILLINGSPREE,
	};

	struct SHOWKILL_INFO
	{
		int Kind;
		DWORD initTime;
		DWORD Duration;
		int Class;
	};

	SHOWKILL_INFO sKillInfo;

	int CurrentStreak;
};

extern cKILL cKill;



