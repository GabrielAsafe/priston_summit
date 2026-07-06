#pragma once

class cMixGuide
{
public:
	static cMixGuide * GetInstance() { static cMixGuide instance; return &instance; };
	void Open() { cOpen = true; };
	void Close() { cOpen = false; };
	bool IsOpen() { return cOpen; };
	void Init();
	void Draw();
	void Button();
	void DrawImage(int Mat, int x, int y, int w, int h);
	bool MouseAction(int x, int y, int w, int h);
	void DescriptionsMix();
	void DrawComboBox();
	void Weapons();
	void ArmorsRobe();
	void Bracelet();
	void Gauntlet();
	void Boot();
	void ShieldOrb();
	void Ring();
	void Amulet();

private:
	bool cOpen;
	int cImages[50];
	SIZE size;
	
};



