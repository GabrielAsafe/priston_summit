#pragma once

#include "UILinker.h"
#include "roll.h"

class CRollWindow
{
public:
	CRollWindow(DWORD iRollID, int x, int y, sITEMINFO* lpItemInfo);
	~CRollWindow();

	bool isInside();

	UIWindow* GetWindow() { return pWindow1; }

protected:
	void OnClickRollButton();
	void OnClickSkipButton();
	void OnHoverItemImage();

private:
	UIWindow* pWindow1;
	sITEM Item;
};



