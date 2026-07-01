#pragma once

class UIButton : public Element
{
public:
	UIButton(int ID, char* path, char* path_, int x, int y, bool hover = true, bool selected = false, std::function<void()> callback = 0);

	void Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);
	bool OnMouseClick(CLICKS Type, int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY);

private:
	UIImage* Image;
	UIImage* Image_;

	std::function<void()> Activation;
};
