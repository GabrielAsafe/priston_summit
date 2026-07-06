#pragma once

enum EOrientation
{
	EORIENT_Left = 1,
	EORIENT_Right = 2,
};

class UIText : public Element
{
public:
	UIText(int iID, char* Text, int x, int y, bool bold = false, DWORD dwColor = RGB(255, 255, 255));

	void Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);
	void AddChar(char Code);
	void RemoveChar(int count = 1);

	void setText(string text);
	string getText() { return string(szText); }

	void append(string text);

	int Length() { return lstrlen(szText); }
	void setOrientation(EOrientation orientation) { eOrientation = orientation; }

private:
	char szText[255];
	bool bBold;
	DWORD TextColor;
	EOrientation eOrientation;
	void update();
};



