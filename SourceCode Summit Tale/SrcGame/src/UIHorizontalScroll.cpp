#include "UILinker.h"

UIHorizontalScroll::UIHorizontalScroll(int iID, int x, int y, int minValue, int maxValue, int startValue)
{
	setID(iID);
	setX(x);
	setY(y);

	ScrollBarBg = new UIImage(0, "game\\Settings\\scroll\\barbg.png", 0, 0);
	ScrollBar = new UIImage(0, "game\\Settings\\scroll\\bar.png", 0, 0);
	ScrollButton = new UIImage(0, "game\\Settings\\scroll\\icon.png", x - 10, y - 6);

	setWidth(ScrollBarBg->getWidth());
	setHeight(ScrollBarBg->getHeight());

	setMin(minValue);
	setMax(maxValue);

	if (startValue >= 0)
		setValue(startValue);
	else
		setValue(((getMax() - getMin()) / 2) + getMin());

	ScrollButton->setX(ScrollBar->getX() + ScrollBar->getWidth() - 10);
}

void UIHorizontalScroll::Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
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

		ScrollBarBg->Render(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
		ScrollBar->Render(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
		ScrollButton->Render(iRenderX, iRenderY, iRenderWidth, iRenderHeight, iSourceX, iSourceY);
	}
}

bool UIHorizontalScroll::OnMouseClick(CLICKS Type, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
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
			if (Type == Down)
			{
				bTrackX = true;

				if (pRealCursorPos.x >= iRenderX && pRealCursorPos.x <= iRenderX + iRenderWidth)
					updateButtonPos(pRealCursorPos.x - iX);
			}
			else if(Type == Up)
			{
				bTrackX = false;
			}
			return true;
		}

		if (Type == Up)
		{
			bTrackX = false;
		}
	}
	return false;
}

bool UIHorizontalScroll::OnMouseMove(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (isVisible())
	{
		if (iWidth <= 0 || iHeight <= 0)
			return false;

		if (bTrackX)
		{
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

			if(pRealCursorPos.x >= iRenderX && pRealCursorPos.x <= iRenderX + iRenderWidth)
				updateButtonPos(pRealCursorPos.x - iX);
		}
		return true;
	}
	return false;
}

void UIHorizontalScroll::updateBar()
{
	ScrollBar->setWidth((int)(ScrollBar->getTrueWidth() * ((getValue() - getMin()) / static_cast<float>(getMax() - getMin()))));
}

void UIHorizontalScroll::updateButtonPos(int iX)
{
	ScrollButton->setX(iX-10);

	int interval = iX - getX();
	float percent = interval / (float)getWidth();
	setValue(((getMax() - getMin()) * percent) + getMin());
}




