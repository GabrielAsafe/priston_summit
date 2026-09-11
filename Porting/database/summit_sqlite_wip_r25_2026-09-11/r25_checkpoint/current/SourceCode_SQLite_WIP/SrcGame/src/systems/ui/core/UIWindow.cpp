#include "systems/ui/core/UILinker.h"

UIWindow::UIWindow(int ID, char * path, int x, int y, bool fixed)
{
	Background = new UIImage(ID, path, 0, 0);
	if (!Background)
		return;

	setID(ID);

	setX(x);
	setY(y);

	setWidth(Background->getWidth());
	setHeight(Background->getHeight());

	setType(1);
	setFixed(fixed);
}

UIWindow::UIWindow(int ID, char * top, char * mid, char * bot, int x, int y, bool fixed)
{
	bgSplit[0] = new UIImage(0, top, 0, 0);
	if (!bgSplit[0])
		return;

	bgSplit[1] = new UIImage(0, mid, 0, bgSplit[0]->getHeight());
	if (!bgSplit[1])
		return;

	bgSplit[2] = new UIImage(0, bot, 0, bgSplit[0]->getHeight() + bgSplit[1]->getHeight());
	if (!bgSplit[2])
		return;

	setID(ID);

	setX(x);
	setY(y);

	setWidth(bgSplit[0]->getWidth());
	setHeight(bgSplit[0]->getHeight() + bgSplit[1]->getHeight() + bgSplit[2]->getHeight());

	setType(2);
	setFixed(fixed);
}

UIWindow::UIWindow(int ID, int x, int y, int w, int h, bool fixed)
{
	setID(ID);
	setX(x);
	setY(y);
	setWidth(w);
	setHeight(h);
	setFixed(fixed);

	setType(1);
}

UIWindow::~UIWindow()
{
	for (auto element : vElements)
		if (element)
			delete element;

	if (Background)
		delete Background;

	if (getType() == 2)
	{
		for (int i = 0; i < 3; i++)
			if (bgSplit[i])
				delete bgSplit[i];
	}
}

void UIWindow::Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (iWidth <= 0 || iHeight <= 0)
			return;

		int cX = getX();
		int cY = getY();
		int cWidth = getWidth();
		int cHeight = getHeight();

		int iScroll = pVerticalScroll ? pVerticalScroll->getValue() : 0;

		int iStartX = cX < iSourceX && cX > 0 ? abs(cX - iSourceX) : 0;
		int iStartY = cY < iSourceY && cY > 0 ? abs(cY - iSourceY) : 0;
		int iRenderWidth = low(cWidth, iWidth);
		int iRenderHeight = low(cHeight, iHeight);
		int iRenderX = cX + iX + iStartX;
		int iRenderY = cY + iY + iStartY;

		if(cX + iRenderWidth > iWidth)
			iRenderWidth -= (cX+ iRenderWidth) - iWidth;

		if (cY + iRenderHeight > iHeight)
			iRenderHeight -= (cY + iRenderHeight) - iHeight;

		if (getType() == 2)
		{
			for (int i = 0; i < 3; i++)
				bgSplit[i]->Render(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
		}
		else
		{
			if (Background)
				Background->Render(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
		}

		for (auto element : vElements)
		{
			element->Render(iRenderX, iRenderY - iScroll, iRenderWidth, iRenderHeight + iScroll, iSourceX, iSourceY + iScroll);
		}

		if(pVerticalScroll)
			pVerticalScroll->Render(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
	}
}

bool UIWindow::OnMouseClick(CLICKS Type, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (iWidth <= 0 || iHeight <= 0)
			return false;

		int cX = getX();
		int cY = getY();
		int cWidth = getWidth();
		int cHeight = getHeight();

		int iScroll = pVerticalScroll ? pVerticalScroll->getValue() : 0;

		int iStartX = cX < iSourceX && cX > 0 ? abs(cX - iSourceX) : 0;
		int iStartY = cY < iSourceY && cY > 0 ? abs(cY - iSourceY) : 0;
		int iRenderX = cX + iX + iStartX;
		int iRenderY = cY + iY + iStartY;
		int iRenderWidth = low(cWidth, iWidth);
		int iRenderHeight = low(cHeight, iHeight);

		if(cX + iRenderWidth > iWidth)
			iRenderWidth -= (cX+ iRenderWidth) - iWidth;

		if (cY + iRenderHeight > iHeight)
			iRenderHeight -= (cY + iRenderHeight) - iHeight;

		if (isMousePos(iRenderX, iRenderY - iStartY, iRenderWidth, iRenderHeight) || Type == Up)
		{
			bool bRet = false;
			for (auto element : vElements)
				if (element->OnMouseClick(Type, iRenderX, iRenderY - iScroll, iRenderWidth, iRenderHeight + iScroll, iSourceX, iSourceY + iScroll))
					bRet = true;

			if (pVerticalScroll)
				if (pVerticalScroll->OnMouseClick(Type, iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY))
					bRet = true;

			if (!bRet && !getFixed() && getRectMove() && Type == Down)
			{
				bMouseToggle = true;
				iPaddingX = pRealCursorPos.x - getX();
				iPaddingY = pRealCursorPos.y - getY();
				bRet = true;
			}
			else if (Type == Up)                      
			{
				bMouseToggle = false;
				iPaddingX = 0;
				iPaddingY = 0;
			}

			return Type != Up;
		}
	}
	return false;
}

bool UIWindow::OnMouseMove(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (iWidth <= 0 || iHeight <= 0)
			return false;

		if (bMouseToggle)
		{
			setX(pRealCursorPos.x - iPaddingX);
			setY(pRealCursorPos.y - iPaddingY);
		}

		int cX = getX();
		int cY = getY();
		int cWidth = getWidth();
		int cHeight = getHeight();

		int iScroll = pVerticalScroll ? pVerticalScroll->getValue() : 0;

		int iStartX = cX < iSourceX && cX > 0 ? abs(cX - iSourceX) : 0;
		int iStartY = cY < iSourceY && cY > 0 ? abs(cY - iSourceY) : 0;
		int iRenderX = cX + iX + iStartX;
		int iRenderY = cY + iY + iStartY;
		int iRenderWidth = low(cWidth, iWidth);
		int iRenderHeight = low(cHeight, iHeight);

		if(cX + iRenderWidth > iWidth)
			iRenderWidth -= (cX+ iRenderWidth) - iWidth;

		if (cY + iRenderHeight > iHeight)
			iRenderHeight -= (cY + iRenderHeight) - iHeight;

		if (pVerticalScroll)
			pVerticalScroll->OnMouseMove(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);

		if (isMousePos(iRenderX, iRenderY, iRenderWidth, iRenderHeight))
		{
			for (auto element : vElements)
				element->OnMouseMove(iRenderX, iRenderY - iScroll, iRenderWidth, iRenderHeight + iScroll, iSourceX, iSourceY + iScroll);

			return true;
		}
	}
	return false;
}

bool UIWindow::OnMouseScroll(int zDelta, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (iWidth <= 0 || iHeight <= 0)
			return false;

		int cX = getX();
		int cY = getY();
		int cWidth = getWidth();
		int cHeight = getHeight();

		int iScroll = pVerticalScroll ? pVerticalScroll->getValue() : 0;

		int iStartX = cX < iSourceX && cX > 0 ? abs(cX - iSourceX) : 0;
		int iStartY = cY < iSourceY && cY > 0 ? abs(cY - iSourceY) : 0;
		int iRenderX = cX + iX + iStartX;
		int iRenderY = cY + iY + iStartY;
		int iRenderWidth = low(cWidth, iWidth);
		int iRenderHeight = low(cHeight, iHeight);

		if(cX + iRenderWidth > iWidth)
			iRenderWidth -= (cX+ iRenderWidth) - iWidth;

		if (cY + iRenderHeight > iHeight)
			iRenderHeight -= (cY + iRenderHeight) - iHeight;

		if (isMousePos(iRenderX, iRenderY, iRenderWidth, iRenderHeight))
		{
			for (auto element : vElements)
				element->OnMouseScroll(zDelta, iRenderX, iRenderY - iScroll, iRenderWidth, iRenderHeight + iScroll, iSourceX, iSourceY + iScroll);

			if (pVerticalScroll)
				pVerticalScroll->OnMouseScroll(zDelta, iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
			return true;
		}
	}
	return false;
}

bool UIWindow::OnKeyDown(char Code)
{
	bool bRet = false;
	for (auto pElement : vElements)
		if (pElement->OnKeyDown(Code))
			bRet = true;
	return bRet;
}

bool UIWindow::OnKeyChar(char Code)
{
	bool bRet = false;
	for (auto pElement : vElements)
		if (pElement->OnKeyChar(Code))
			bRet = true;
	return bRet;
}

void UIWindow::setMidHeight(int height)
{
	if (getType() == 2)
	{
		/*bgSplit[1]->setHeight(height);
		bgSplit[1]->setY(bgSplit[2]->getY() - height);
		bgSplit[0]->setY(bgSplit[1]->getY() - bgSplit[0]->getHeight());

		setHeight(bgSplit[0]->getHeight() + bgSplit[1]->getHeight() + bgSplit[2]->getHeight());
		setY(bgSplit[0]->getY());*/

		int baseY = getY() + getHeight();
		bgSplit[1]->setHeight(height);
		bgSplit[0]->setY(0);
		bgSplit[1]->setY(bgSplit[0]->getHeight());
		bgSplit[2]->setY(bgSplit[0]->getHeight() + bgSplit[1]->getHeight());
		setHeight(bgSplit[0]->getHeight() + bgSplit[1]->getHeight() + bgSplit[2]->getHeight());
		setY(baseY - getHeight());
	}
}

void UIWindow::ExpandMidWindow()
{
	int oldHeight = getHeight();
	if (iExpandCount < iExpandLimit || iExpandLimit == -1)
	{
		setMidHeight(getMidHeight() + iExpandRate);
		iExpandCount++;
	}
	else
	{
		setMidHeight(iDefaultMidHeight);
		iExpandCount = 0;
	}

	for (auto pElement : vElements)
	{
		if (pElement->getAlign() == EALIGN_BOT)
		{
			int oldDistY = oldHeight - pElement->getY();
			pElement->setY(getHeight() - oldDistY);
		}
	}
}

void UIWindow::AdjustScrollBar()
{
	if (pVerticalScroll)
	{
		if (iMaxY > getHeight())
		{
			pVerticalScroll->setHeight(getHeight() - 5);
			pVerticalScroll->setMax(iMaxY - getHeight());
			pVerticalScroll->setValue(pVerticalScroll->getMax());
			pVerticalScroll->updateButtonPosByValue();
			pVerticalScroll->show();
		}
		else
		{
			pVerticalScroll->setValue(0);
			pVerticalScroll->hide();
		}
	}
}

void UIWindow::AddElement(Element * pElement)
{
	if (pElement->getY() + pElement->getHeight() > iMaxY && pElement->getID() != 500)
	{
		iMaxY = pElement->getY() + pElement->getHeight();

		if (iMaxY > getHeight() && bScroll)
		{
			if (!pVerticalScroll)
			{
				pVerticalScroll = new UIVerticalScroll(500, getWidth() - 10, 10, -1, getHeight() - 20, 0, iMaxY - getHeight());
				//pVerticalScroll->setValue(iMaxY - getHeight());
				pVerticalScroll->setValue(0);
				pVerticalScroll->updateButtonPosByValue();
				pVerticalScroll->show();
			}
			else
			{
				pVerticalScroll->setMax(iMaxY - getHeight());
				pVerticalScroll->setValue(0);
				pVerticalScroll->setHeight(getHeight() - 20);
				pVerticalScroll->updateButtonPosByValue();
				pVerticalScroll->show();
			}
		}
	}

	vElements.push_back(pElement);
}

void UIWindow::RemoveElementByID(int ID)
{
	for (auto it = vElements.begin(); it < vElements.end(); it++)
	{
		auto pElement = (*it);
		if (pElement->getID() == ID)
		{
			delete pElement;
			vElements.erase(it);
			it--;
		}
	}
}




