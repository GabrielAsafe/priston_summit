#pragma once

class UIPlaceholder : public Element
{
public:
	UIPlaceholder(function<void()> func)
	{
		fRender = func;
		setX(0);
		setY(0);
		setWidth(0);
		setHeight(0);
	}
	void Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY) 
	{
		if (isVisible() && fRender)
			fRender();
	}

private:
	function<void()> fRender;
};
