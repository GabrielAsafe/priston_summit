#pragma once

class cHELLSGATE
{
public:
	void RecvOpenNpc();
	void ConfirmEnterHg();
	void Draw();
	void RecvUpdateHg(smTRANS_COMMAND* psPacket);
	void RecvLeaveHg();

private:
	bool bInEvent;
	DWORD dwTime;
	int iState;
};

extern cHELLSGATE cHg;



