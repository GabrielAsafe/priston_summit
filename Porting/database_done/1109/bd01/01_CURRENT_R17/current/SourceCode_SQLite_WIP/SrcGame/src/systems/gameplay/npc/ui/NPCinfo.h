#pragma once
#pragma once
#include <windows.h>

#define smTRANSCODE_OPEN_NPCinfo  0x49470022

class NPCinfo
{
public:
	static              NPCinfo*	GetInstance() { static NPCinfo instance; return &instance; }
	bool                MouseAction(int x, int y, int w, int h);
	bool                IsOpen() { return fOpen; }
	void                Open();
	void                Init();
	void                Draw();
	void                Close();
	void                Button();
	int                 GetSlot();
	int                 GetScroll();
	void                ButtonScroll(short Dist);
private:
	int                Image[110];
	bool               fOpen;
	int                Scroll;
	int                Tag;
	int                SubTag;
};






