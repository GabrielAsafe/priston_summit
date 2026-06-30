#include "UILinker.h"
//#include "Hook\\Hook.h"
extern int DrawFontRect(RECT rc, char* lpszString, int nCount, UINT uFormat);
extern void SetBold(bool bold);
extern int __stdcall SetFontColor(DWORD textcolor);
UIText::UIText(int iID, char* Text, int x, int y, bool bold, D3DCOLOR d3dColor)
{
	setID(iID);
	setX(x);
	setY(y);
	bBold = bold;
	TextColor = d3dColor;

	setOrientation(EORIENT_Left);

	ZeroMemory(szText, 255);
	lstrcpy(szText, Text);

	update();
}

void UIText::Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (iWidth <= 0 || iHeight <= 0)
			return;

		int cX = getX();
		int cY = getY();
		int cWidth = getWidth();
		int cHeight = getHeight();
		int iStartX = cX < iSourceX ? abs(cX - iSourceX) : 0;
		int iStartY = cY < iSourceY ? abs(cY - iSourceY) : 0;

		int iRenderX = cX + iX + iStartX;
		int iRenderY = cY + iY + iStartY;
		int iRenderWidth = low(cWidth, iWidth);
		int iRenderHeight = low(cHeight, iHeight);

		if (cX + iRenderWidth > iWidth)
			iRenderWidth -= (cX + iRenderWidth) - iWidth;

		if (cY + iRenderHeight > iHeight)
			iRenderHeight -= (cY + iRenderHeight) - iHeight;

		if (iStartY < iRenderHeight && iStartX < iRenderWidth)
		{
			UINT uFormat = 0;

			if (iStartX > 0)
			{
				uFormat |= DT_RIGHT;
			}
			else if (iRenderWidth < cWidth)
			{
				if (eOrientation == EORIENT_Left)
					uFormat |= DT_LEFT;
				else
					uFormat |= DT_RIGHT;
			}

			if (iStartY > 0)
			{
				uFormat |= DT_BOTTOM;
			}

			SetFontColor(TextColor);
			SetBold(true);
			DrawFontRect(RECT({ iRenderX, iRenderY, iRenderX + iRenderWidth - iStartX, iRenderY + iRenderHeight - iStartY }), szText, lstrlen(szText), uFormat);
			SetBold(false);
		}
	}
}

void UIText::AddChar(char Code)
{
	if (lstrlen(szText) < 254)
	{
		szText[lstrlen(szText)] = Code;
		szText[lstrlen(szText)] = 0;

		update();
	}
}

void UIText::RemoveChar(int count)
{
	if (lstrlen(szText) >= count && count > 0)
	{
		szText[lstrlen(szText) - count] = 0;

		update();
	}
}

void UIText::setText(string text)
{
	lstrcpy(szText, text.c_str());

	update();
}

void UIText::append(string text)
{
	lstrcat(szText, text.c_str());

	update();
}

void UIText::update()
{
	SetBold(true);

	SIZE sz = {};
	GetTextExtentPoint(0, szText, lstrlen(szText), &sz);
	setWidth(sz.cx);
	setHeight(sz.cy);

	SetBold(false);
}

