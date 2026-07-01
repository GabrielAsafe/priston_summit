#pragma once

class UITitle : public Element
{
public:
	UITitle(int iID, char* left, char* middle, char* right, int x, int y, char* text, bool bold = false, DWORD dwColor = D3DCOLOR_ARGB(255, 255, 255, 255));

	void Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);

	void setText(string text);

private:
	UIImage* Left;
	UIImage* Right;
	UIImage* Middle;
	UIText* Text;
};
