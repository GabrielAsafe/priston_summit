#pragma once

class UIHorizontalScroll : public Element
{
public:
	UIHorizontalScroll(int iID, int x, int y, int minValue, int maxValue, int startValue = -1);

	void Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);
	bool OnMouseClick(CLICKS Type, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);
	bool OnMouseMove(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);

	void setMin(int min) { Minimum = min; }
	int getMin() { return Minimum; }

	void setMax(int max) { Maximum = max; }
	int getMax() { return Maximum; }

	void setValue(int val) { val >= Maximum ? Value = Maximum : val <= Minimum ? Value = Minimum : Value = val;; updateBar(); updateButtonPos(); }
	int getValue() { return Value; }

private:
	UIImage* ScrollBar;
	UIImage* ScrollBarBg;
	UIImage* ScrollButton;

	int Minimum;
	int Maximum;
	int Value;

	bool bTrackX = false;;

	void updateBar();
	void updateButtonPos(int iX);
	void updateButtonPos() { ScrollButton->setX(ScrollBar->getX() + ScrollBar->getWidth() - 10); }
};
