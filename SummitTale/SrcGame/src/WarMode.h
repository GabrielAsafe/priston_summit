#pragma once

class MODOWAR
{
public:
	void Init();
	void Draw();
	void Open() { fOpen = true; };
	void Close() { fOpen = false; };
	bool IsOpen() { return fOpen; };
	BOOL IsMousePos(int x, int y, int w, int h);
	void Button();
	void DrawImage(int Image, int x, int y, int w, int h);


	int WarModeImg[10];
	int fOpen;
	int BaseX, BaseY;
};
extern MODOWAR WarMode;
