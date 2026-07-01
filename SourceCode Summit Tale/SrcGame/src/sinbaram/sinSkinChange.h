struct sSKIN_CHANGE
{
	sITEM SkinChangeItem[2];

	sSKIN_CHANGE()
	{
		ZeroMemory(&SkinChangeItem, sizeof(SkinChangeItem));
	}
};


struct sSKIN_CHANGE_SERVER
{
	int		size, code;
	int		DocIndex;
	int		Result;
	int		Index;

	sITEM	DesSkinChangeItem;

	DWORD	dwSkinChangeStoneItemCode;
	DWORD	dwSkinChangeStoneHead;
	DWORD	dwSkinChangeStoneCheckSum;

	sSKIN_CHANGE_SERVER()
	{
		size = 0;
		code = 0;
		DocIndex = 0;
		Result = 0;
		Index = 0;

		dwSkinChangeStoneItemCode = 0;
		dwSkinChangeStoneHead = 0;
		dwSkinChangeStoneCheckSum = 0;
	}
};

class cSKINSYSTEM
{
public:

	int	OpenFlag;
	int	SkinChangeItemCheckSum;
	int	HaveSkinChangeStone;

public:

	int	SetSkinChangeAreaCheck(sITEM* pItem);
	int PickUpSkinChangeItem(int x, int y, int PickUpFlag = 0, int Kind = 0);
	int LastSetSkinChangeItem(sITEM* pItem, int kind);
	int	CancelSkinChange(int Flag = 0);
	int	CheckSkinChangeItemForm();
	int	ReformSkinChangeItem();

	int	sinSendSkinChange();
	int	sinRecvSkinChange(sSKIN_CHANGE_SERVER* pSkinChangeItem);
	int	sinRecvSkinChangeResult(sSKIN_CHANGE_SERVER* pSkinChangeItem_Server);
};

extern cSKINSYSTEM cSkinChange;
extern sSKIN_CHANGE	sSkinChangeItem;
extern sSKIN_CHANGE sSkinChangeItemBackUp;
extern int	SkinChangeCheckFlag;
extern sSKIN_CHANGE_SERVER	sSkinChange_Send;
extern sSKIN_CHANGE_SERVER	sSkinChange_Recv;
