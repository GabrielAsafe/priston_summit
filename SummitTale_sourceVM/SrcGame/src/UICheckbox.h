#pragma once

class UICheckbox : public UIListItem
{
public:
	UICheckbox(int ID, int x, int y, char* szText, bool free = true, bool selected = false, bool hover = false, bool bold = false);
	UICheckbox(int ID, int x, int y, char* path, char* path_, char* szText, bool free = true, bool selected = false, bool hover = false, bool bold = false);

	void Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);
	bool OnMouseClick(CLICKS Type, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);

	void setActivation(std::function<void()> callback) { Activation = callback; }
	void setHoverFunc(std::function<void()> callback) { fHover = callback; }

private:
	UIText* pText = nullptr;
	std::function<void()> Activation;
	std::function<void()> fHover;
};
