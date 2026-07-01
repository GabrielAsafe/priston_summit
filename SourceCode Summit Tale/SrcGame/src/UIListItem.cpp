#include "UILinker.h"

UIListItem::UIListItem(int ID, char * path, char * path_, int x, int y, bool hover, bool selected)
{
	Image = new UIImage(ID, path, 0, 0);

	if (!Image)
		return;

	Image_ = new UIImage(ID, path_, 0, 0);
	setWidth(Image->getWidth());
	setHeight(Image->getHeight());

	setID(ID);
	setX(x);
	setY(y);

	setHover(hover);
	setSelected(selected);
}

void UIListItem::Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
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

		if(cX + iRenderWidth > iWidth)
			iRenderWidth -= (cX+ iRenderWidth) - iWidth;

		if (cY + iRenderHeight > iHeight)
			iRenderHeight -= (cY + iRenderHeight) - iHeight;

		if (((isMousePos(iRenderX, iRenderY, iRenderWidth, iRenderHeight) && getHover()) || getSelected()) && Image_)
		{
			Image_->Render(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
		}
		else
		{
			Image->Render(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
		}
	}
}

void UIListItem::RenderWindows(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (!getSelected())
			return;

		for (auto pElement : vElements)
			pElement->Render(iX, iY, iWidth, iHeight, iSourceX, iSourceY);
	}
}

bool UIListItem::OnMouseClick(CLICKS Type, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (iWidth <= 0 || iHeight <= 0)
			return false;

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

		if(cX + iRenderWidth > iWidth)
			iRenderWidth -= (cX+ iRenderWidth) - iWidth;

		if (cY + iRenderHeight > iHeight)
			iRenderHeight -= (cY + iRenderHeight) - iHeight;

		if (isMousePos(iRenderX, iRenderY - iStartY, iRenderWidth, iRenderHeight))
		{
			return true;
		}
	}
	return false;
}

void UIListItem::OnMouseClickWindows(CLICKS Type, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (!getSelected())
			return;

		for (auto pElement : vElements)
			pElement->OnMouseClick(Type, iX, iY, iWidth, iHeight, iSourceX, iSourceY);
	}
}

bool UIListItem::OnMouseMoveWindows(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (!getSelected())
			return false;

		for (auto pElement : vElements)
			pElement->OnMouseMove(iX, iY, iWidth, iHeight, iSourceX, iSourceY);
	}
	return false;
}

bool UIListItem::OnMouseScrollWindows(int zDelta, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (!getSelected())
			return false;

		for (auto pElement : vElements)
			pElement->OnMouseScroll(zDelta, iX, iY, iWidth, iHeight, iSourceX, iSourceY);
	}
	return false;
}

bool UIListItem::OnKeyDown(char Code)
{
	bool bRet = false;
	for (auto pElement : vElements)
		if (pElement->OnKeyDown(Code))
			bRet = true;
	return bRet;
}

bool UIListItem::OnKeyChar(char Code)
{
	bool bRet = false;
	for (auto pElement : vElements)
		if (pElement->OnKeyChar(Code))
			bRet = true;
	return bRet;
}

void UIListItem::RegisterEvent(ListEvents eEvent, std::function<void()> callback)
{
	switch (eEvent)
	{
	case LISTEVENT_Select:
		SelectCallback = callback;
		break;

	case LISTEVENT_Unselect:
		UnselectCallback = callback;
		break;
	}
}

void UIListItem::OnSelect()
{
	if (getSelected())
		return;

	setSelected(true);

	for (auto pElement : vElements)
		pElement->show();

	if (SelectCallback)
		SelectCallback();
}

void UIListItem::OnUnselect()
{
	if (!getSelected())
		return;

	setSelected(false);

	for (auto pElement : vElements)
		pElement->hide();

	if (UnselectCallback)
		UnselectCallback();
}

void UIListItem::AddElement(Element * pElement)
{
	vElements.push_back(pElement);
}
