
#ifndef PRIME_ITEM_TIME_1H
#define PRIME_ITEM_TIME_1H   (60 * 60)
#endif
#ifndef PRIME_ITEM_TIME_6H
#define PRIME_ITEM_TIME_6H   (6 * 60 * 60)
#endif
#ifndef PRIME_ITEM_TIME_12H
#define PRIME_ITEM_TIME_12H  (12 * 60 * 60)
#endif
#ifndef PRIME_ITEM_TIME_24H
#define PRIME_ITEM_TIME_24H  (24 * 60 * 60)
#endif
#ifndef PRIME_ITEM_TIME_72H
#define PRIME_ITEM_TIME_72H  (72 * 60 * 60)
#endif
#ifndef PRIME_ITEM_TIME_7D
#define PRIME_ITEM_TIME_7D   (7 * 24 * 60 * 60)
#endif

#include "global_def.h"
#include "smwsock.h"
#ifndef	_SMPACKET_HEADER

#define _SMPACKET_HEADER

#include	"sinbaram/sinItem.h"
// #include <atltime.h> // removed for MinGW

//���Ӱ��� ���� Lib
#ifdef	_npGAME_GUARD_AUTH
#include "nProtect/CSAuth.h"
#pragma comment(lib,"nprotect//CSAuth.lib")
#endif

#define	smTRANSCODE_OPENLIVE		0x38000000

#define	smTRANSCODE_SYSTEM			0x48400000

#define	smTRANSCODE_CONNECTED		0x48470080
#define smTRANSCODE_RECORDDATA		0x48470081
#define smTRANSCODE_GETRECORDDATA	0x48470082
#define smTRANSCODE_FAILRECORDDATA	0x48470083
#define smTRANSCODE_RECORDRESULT	0x48470084

//ID�� ����� ĳ���� ���� �Լ�
#define smTRANSCODE_ID_GETUSERINFO	0x48470085
#define smTRANSCODE_ID_SETUSERINFO	0x48470086
#define smTRANSCODE_DELRECORDDATA	0x48470087
#define smTRANSCODE_INSRECORDDATA	0x48470088
#define smTRANSCODE_ISRECORDDATA	0x48470089

//���� Ȯ�� �޼���
#define smTRANSCODE_VERSION			0x4847008A
//��Ʈ��ũ ýũ �޼���
#define smTRANSCODE_CHECK_NETSTATE	0x4847008B
//������ ��� ����
#define smTRANSCODE_ADMINMODE		0x4847008C
//������Ʈ ������ ��ȣ ���� 
#define smTRANSCODE_SET_OBJSERIAL	0x4847008D
//��Ʈ��ũ ýũ �޼���(���� ������)
#define smTRANSCODE_CHECK_NETSTATE2	0x4847008E
#define smTRANSCODE_CHECK_NETSTATE3	0x4847008F
#define smTRANSCODE_CHECK_NETSTATE4	0x48470090
#define smTRANSCODE_CHECK_NETSTATE5	0x48470091

#define smTRANSCODE_IPLIST			0x48471000
#define smTRANSCODE_CHATMESSAGE		0x48471001
#define smTRANSCODE_POSITION		0x48471002
#define smTRANSCODE_PLAYLIST		0x48471003
#define smTRANSCODE_CONTINUEDATA	0x48471004
#define smTRANSCODE_WHISPERMESSAGE	0x48471005
#define smTRANSCODE_COUPLERING		0x48471006			// pluto Ŀ�ø�

#define smTRANSCODE_PLAYDATA1		0x48470010
#define smTRANSCODE_PLAYDATA2		0x48470011
#define smTRANSCODE_PLAYDATA3		0x48470012
#define smTRANSCODE_TRANSPLAYDATA	0x48470013
#define smTRANSCODE_PLAYDATAGROUP	0x48470014
#define smTRANSCODE_PLAY_COMMAND	0x48470018

#define smTRANSCODE_PARTY_CHATMSG	0x48471015

#define smTRANSCODE_ADDEXP_PARTY	0x48470029
#define smTRANSCODE_ATTACKDATA		0x48470030
#define smTRANSCODE_ADDEXP			0x48470031
#define smTRANSCODE_PLAYERINFO2		0x48470032
#define smTRANSCODE_SET_BLACKLIST	0x48470033
#define smTRANSCODE_SET_ITEMCOPYLIST 0x48470034
#define smTRANSCODE_SKIL_ATTACKDATA	0x48470035
#define smTRANSCODE_FINDCRACK		0x48470036
#define smTRANSCODE_MODULE_WARNIG	0x48470037
#define smTRANSCODE_UPDATE_CINFO	0x48470038
#define smTRANSCODE_OPENMONSTER		0x48470039
#define smTRANSCODE_WARPFIELD		0x48470040
#define smTRANSCODE_TRADEITEMS		0x48470041
#define smTRANSCODE_TRADE_REQUST	0x48470042
#define smTRANSCODE_TRADE_ITEMKEY	0x48470043
#define smTRANSCODE_TRADE_READYITEM	0x48470044
#define smTRANSCODE_SKILL_MENU		0x48470045
#define smTRANSCODE_FINDCRACK2		0x48470046
#define smTRANSCODE_WAREHOUSE		0x48470047

#define smTRANSCODE_OPEN_WAREHOUSE	0x48470048
#define smTRANSCODE_OPEN_MIXITEM	0x48470049
#define smTRANSCODE_OPEN_COLLECT	0x4847004A
#define smTRANSCODE_COLLECTMONEY	0x4847004B
#define	smTRANSCODE_PLAYERINFO_Q	0x4847004C
#define smTRANSCODE_OPEN_AGING		0x4847004D

#define smTRANSCODE_SKIL_ATTACKUSER	0x4847004E

#define smTRANSCODE_OPEN_EVENTGIFT	0x4847004F

#define smTRANSCODE_PLAYERINFO		0x48470020
#define smTRANSCODE_GETPLAYERINFO	0x48470021
#define smTRANSCODE_FAILCONNECT		0x48470023


#define smTRANSCODE_PLAYITEM		0x48470050
#define smTRANSCODE_DELITEM			0x48470051
#define smTRANSCODE_PUTITEM			0x48470052
#define smTRANSCODE_THROWITEM		0x48470053
#define smTRANSCODE_SHOP_ITEMLIST	0x48470054
#define smTRANSCODE_SAVETHROWITEM	0x48470055
#define smTRANSCODE_SAVETHROWMONEY	0x48470056
#define smTRANSCODE_SAVETHROWITEM2	0x48470057
#define smTRANSCODE_SAVETHROWITEMS	0x48470058
#define smTRANSCODE_PLAYERKILLING	0x48470059
#define smTRANSCODE_GETITEM			0x4847005A
#define smTRANSCODE_USEITEM_CODE	0x4847005B
#define smTRANSCODE_COMMAND_USER	0x4847005C
#define smTRANSCODE_WING_NPC		0x4847005D
#define smTRANSCODE_PAYMENT_MONEY	0x4847005E
#define smTRANSCODE_MEDIA_NPC		0x4847005F

#define smTRANSCODE_ADD_STARTPOSI	0x48470060
#define smTRANSCODE_DEL_STARTPOSI	0x48470061
#define smTRANSCODE_ADD_NPC			0x48470070
#define smTRANSCODE_DEL_NPC			0x48470071
#define smTRANSCODE_ADMIN_COPYCHAR	0x48470062

#define	smTRANSCODE_PARTY_UPDATE		0x484700A0
#define	smTRANSCODE_PARTY_REQUEST		0x484700A1
#define	smTRANSCODE_PARTY_JOIN			0x484700A2
#define	smTRANSCODE_PARTY_PLAYUPDATE	0x484700A3
#define	smTRANSCODE_PARTY_RELEASE		0x484700A4
#define	smTRANSCODE_PARTY_COMMAND		0x484700A5
#define smTRANSCODE_OPEN_MIXTURE_RESET	0x484700A6	// ������ - �ͽ��� ���� �޽��� ����

#define smTRANSCODE_PCBANG_PET			0x484700B0
#define smTRANSCODE_PHENIX_PET			0x484700B1	// pluto ��(�ؿ�) ����
#define smTRANSCODE_HELP_PET			0x484700B2	// ����� - ��� ����� �� �߰�

#define	smTRANSCODE_SERVER_INFO			0x484700C0

#ifndef smTRANSCODE_MECH_INFO
#define smTRANSCODE_MECH_INFO      0x484700C1
#define smTRANSCODE_FIGHTER_INFO   0x484700C2
#define smTRANSCODE_PIKE_INFO      0x484700C3
#define smTRANSCODE_ARCHER_INFO    0x484700C4
#define smTRANSCODE_KINA_INFO      0x484700C5
#define smTRANSCODE_ATA_INFO       0x484700C6
#define smTRANSCODE_SACER_INFO     0x484700C7
#define smTRANSCODE_MAGE_INFO      0x484700C8
#endif


#define	smTRANSCODE_PARTY_POTION		0x484700D0
//����
#define	smTRANSCODE_HEALING				0x484700D2
#define	smTRANSCODE_HOLYMIND			0x484700D3
#define	smTRANSCODE_GRAND_HEALING		0x484700D4

#define	smTRANSCODE_VAMPRIC_CUSPID		0x484700D6
#define	smTRANSCODE_VAMPRIC_CUSPID_EX		0x484700D7		// �庰 - ���Ǹ� Ŀ���� EX
#define	smTRANSCODE_MANA_RECHARGING		0x484700D8

//������ �ŷ� ����
#define	smTRANSCODE_RECORD_TRADE		0x48480010


//��Ʈ��ũ ǰ�� ��� ����
#define smTRANSCODE_NETWORK_QUALITY	0x484700E0
#define smTRANSCODE_PROCESSINFO		0x484700E4
#define	smTRANSCODE_CLOSECLIENT		0x484700E6
#define smTRANSCODE_CHECKIP			0x484700E7

//Ŭ���̾�Ʈ�� ����Ÿ ���� �䱸
#define	smTRANSCODE_SAVECLIENT		0x484700E8


#define smTRANSCODE_EFFECT_ITEM		0x48478000
#define smTRANSCODE_EFFECT_WEAPON	0x48478002
#define smTRANSCODE_EFFECT_STUN		0x48478004
#define smTRANSCODE_EFFECT_CURSE	0x48478006
#define smTRANSCODE_EFFECT_ICE		0x48478008
#define smTRANSCODE_EFFECT_POISION	0x4847800A
#define smTRANSCODE_EFFECT_VANISH	0x4847800C
#define smTRANSCODE_EFFECT_PERSHOP	0x4847800E
#define smTRANSCODE_EFFECT_LOWPET	0x4847800F
#define smTRANSCODE_EFFECT_CLANMGR	0x48479000
#define smTRANSCODE_EFFECT_FORCEORB	0x48479010
#define smTRANSCODE_EFFECT_CURSE_QUEST	0x48479011
#define smTRANSCODE_EFFECT_BLESS_SCORE	0x48479012
#define smTRANSCODE_EFFECT_KEEPSKILL	0x48479020


//������Ʈ ������ ��ȣ ���� 
#define smTRANSCODE_CONT_OBJSERIAL	0x48478010

//�޼����ڽ� ���
#define smTRANSCODE_MESSAGEBOX		0x48478100

//��� ���������� ����
#define	smTRANSCODE_FUNCERROR		0x48478200

//Ŭ���̾�Ʈ ��Ǹ޸� ����
#define smTRANSCODE_FUNCTION_MEM2	0x48478300

//��ŷ���� ��� 2
#define	smTRANSCODE_SET_BLACKLIST2	0x48478400
//��ŷ���� ��� 3
#define smTRANSCODE_SET_BLACKLIST3	0x48478500

//�ҷ� ���� ����
#define	smTRANSCODE_BAD_PLAYER		0x48478600

//�߸��� ���� ��ġ ���� ��
#define	smTRANSCODE_FIX_ATTACK		0x48478700
//ĳ���� ������ ���̱�
#define	smTRANSCODE_DEAD_PLAYER		0x48478800

//Ŭ���̾�Ʈ ��ġ ����
#define smTRANSCODE_CLIENT_ERROR	0x48478900

//�κ��丮 ������ ����
#define	smTRANSCODE_INVEN_ERR_ITEM	0x48478910
//�κ��丮 �� ����
#define	smTRANSCODE_INVEN_ERR_MONEY	0x48478920

//�κ��丮 ������ ��ġ ����
#define	smTRANSCODE_INVEN_POSITION	0x48478930
#define	smTRANSCODE_INVEN_POSITION2	0x48478931


//��ȣȭ�� ��Ŷ
#define smTRANSCODE_ENCODE_PACKET	0x80010000
#define smTRANSCODE_ENCODE_PACKET2	0x90010000

//Ŭ�� �޴�
#define smTRANSCODE_OPEN_CLANMENU	0x48478A00
//�� ����Ʈ ����
#define smTRANSCODE_OPEN_STARPOINT	0x48478A10
#define smTRANSCODE_STARPOINT		0x48478A12
//����� ��Ŷ
#define smTRANSCODE_OPEN_GIVEMONEY	0x48478A14
#define smTRANSCODE_GIVEMONEY		0x48478A16
//Ŭ���Ӵ� ������Ŷ
#define smTRANSCODE_CLANMONEY		0x48478A18


//������ ���
#define smTRANSCODE_ITEM_EXPRESS	0x48478A80

//���λ��� ����
#define smTRANSCODE_OPEN_MYSHOP		0x48478A90
//���λ��� �θ���
#define smTRANSCODE_CALL_MYSHOP		0x48478AA0
//���λ��� ������
#define	smTRANSCODE_MYSHOP_ITEM		0x48478AB0
//���λ��� �޼��� ���
#define	smTRANSCODE_MYSHOP_MESSAGE	0x48478AC0
//���λ��� ���� ����
#define	smTRANSCODE_MYSHOP_TRADE	0x48478AD0

//��öȣ : 2005-11-17 ���� 8:41:39
//PC�� �� ��������Ʈ
#define smTRANSCODE_EFFECT_PCBANGPET	0x48478AE0


//������ ��� ����
#define smTRANSCODE_ADMINMODE2			0x5047108C
//��Ʈ�� ýũ
#define smTRANSCODE_CHECK_NETSTATE20	0x5047008E

//����ġ �� ��������Ÿ Ȯ��
#define smTRANSCODE_CHECK_EXPMONEY		0x50320010
//����ġ ����Ÿ Ȯ��
#define smTRANSCODE_CHECK_EXPDATA		0x50320020
//�������� ���� ����Ÿ ��������
#define smTRANSCODE_FORCEORB_DATA		0x50320030
//��������� ���� ����Ÿ ��������
#define smTRANSCODE_PRIMEITEM_DATA		0x50320031

// pluto ���� ���ེ
// pluto ��������� ���� ����Ÿ ��������
#define smTRANSCODE_PRIMEITEM_DATA2	0x50320032

// ����� - ��� ����� ��, ���Ǹ� Ŀ���� EX, �ٷ� ���ེ ����
#define smTRANSCODE_PRIMEITEM_DATA3	0x50320033

// �庰 - ���Ǹ� Ŀ���� EX
#define smTRANSCODE_PRIMEITEM_DATA4	0x50320036

// ����� - �ν��� ������(�����, ���, ü��)
#define smTRANSCODE_BOOSTER_DATA		0x50320034

// �庰 - ����¡ ġƮ ������ ����
#define smTRANSCODE_AGING_DATA			0x50320035

// �庰 - ��ų �����
#define smTRANSCODE_SKILLDELAY_DATA			0x50320036

//����Ÿ ������ Ŭ���̾�Ʈ ���� ����
#define smTRANSCODE_SEND_DATASERVER		0x50320100
//���� ������ Ŭ���̾�Ʈ ���� ����
#define smTRANSCODE_SEND_GAMESERVER		0x50320110
//���� ������ Ŭ���̾�Ʈ ���� ����
#define smTRANSCODE_SEND_AREASERVER		0x50320120
//����/���� ������ Ŭ���̾�Ʈ ���� ����
#define smTRANSCODE_SEND_AGAME_SERVER	0x50320130

//pluto ���� ���ེ
//����/���� ������ Ŭ���̾�Ʈ ���� ����
#define smTRANSCODE_SEND_AGAME_PRIMEITEM_DATA2	0x50320140

// ����� - ��� ����� ��, ���Ǹ� Ŀ���� EX, �ٷ� ���ེ ���� ������ �߰��� �����.
// ����/���� ������ Ŭ���̾�Ʈ ���� ����
#define smTRANSCODE_SEND_AGAME_PRIMEITEM_DATA3	0x50320150

// �庰 - ���Ǹ� Ŀ���� EX
#define smTRANSCODE_SEND_AGAME_PRIMEITEM_DATA4	0x50320160


//�ͽ��� ����Ÿ ����
#define smTRANSCODE_CRAFTITEM			0x50320200
//����¡ ����Ÿ ����
#define smTRANSCODE_AGINGITEM			0x50320202
//����¡ ���׷��̵�
#define	smTRANSCODE_AGING_UPGRADE		0x50320204
//��ũ�ھ� ���
#define	smTRANSCODE_MAKE_LINKCORE		0x50320205
//��ũ�ھ� ���
#define	smTRANSCODE_USE_LINKCORE		0x50320206
//�� ������ ����Ÿ ����
#define smTRANSCODE_WINGITEM			0x50320208

//���� ����Ÿ ����
#define smTRANSCODE_SHOPTITEM			0x50320210
//���� ����Ÿ �Ǹ�
#define smTRANSCODE_SHOP_SELLITEM		0x50320212

//�������� ������ ����
#define	smTRANSCODE_ITEM_FORCEORB		0x50320214

//����Ʈ ���
#define	smTRANSCODE_QUEST_COMMAND		0x50320220
//����� Web URL ����
#define	smTRANSCODE_EXITINFO_URL		0x50320230


//������ Ȯ��
#define smTRANSCODE_CHECKITEM			0x50320300
//������ ���� Ȯ��
#define smTRANSCODE_ERRORITEM			0x50320310
//���� ���� ���� Ȯ�� - ���� ��ģ���� 1�� �ʱ�ȭ 
#define smTRANSCODE_CLEARPOTION			0x50320320

#ifdef _CHECK_MESH
//�����۸޽� ���۰˻� ýũ����
#define smTRANSCODE_CHECK_ITEMMESH		0x50320330
#endif

//Ŭ���̾�Ʈ ��������� ����
#define smTRANSCODE_CLIENT_FUNCPOS		0x50320400
//Ŭ���̾�Ʈ ��������� ���� �䱸 ( �ʱ⼳���� ���� )
#define smTRANSCODE_GETCLIENT_FUNCPOS	0x50320410

//Ŭ���̾�Ʈ ��Ǹ޸� ����
#define smTRANSCODE_FUNCTION_MEM		0x50320420
//Ŭ���̾�Ʈ ��������� ��� ��
#define smTRANSCODE_FUNCTON_RESULT		0x50320430

//Ŭ���̾�Ʈ ���μ��� Ÿ�� �ִ�ġ
#define smTRANSCODE_PROCESS_TIMEMAX		0x50320500

//nSprite �߱��� �������α׷�
#define smTRANSCODE_NSPRITE				0x50320600
//nProtect
#define smTRANSCODE_NPROTECT			0x50320700
#define smTRANSCODE_GAMEGUARD_AUTH		0x50320710


//���� ��� �� �䱸
#define	smTRANSCODE_REC_DAMAGEDATA		0x50320800

//SoD ��� ����
#define	smTRANSCODE_SOD_RESULT			0x50320900
#define	smTRANSCODE_SOD_RESULT2			0x50320901

//��ų ���� ����
#define	smTRANSCODE_PARTY_SKILL			0x50320A00


//��ų ���� ���
#define	smTRANSCODE_CANCEL_SKILL		0x50320A10
//��ų ���� �Ϲ�
#define	smTRANSCODE_PROCESS_SKILL		0x50320A20
#define smTRANSCODE_PROCESS_SKILL2		0x50320A30
#define smTRANSCODE_PROCESS_CLAN_SKILL	0x50320A35
//��ų ���� ������Ʈ
#define	smTRANSCODE_UPDATEL_SKILL		0x50320A40
//�߰�ȸ�� ��������
#define	smTRANSCODE_SUCCESS_EVATION		0x50320A50
//�Ӽ�����
#define	smTRANSCODE_RESISTANCE			0x50320A60

//��������
#define	smTRANSCODE_PUBLIC_POLLING		0x50320A70
//������� �ڽ�
#define	smTRANSCODE_NOTICE_BOX			0x50320A80
//��ŷ�� ����
#define	smTRANSCODE_HACKTRAP			0x50320A90


//����,���,�ٷ� ����
#define	smTRANSCODE_VIRTURAL_POTION		0x50320B00

//Ŭ�� ���� �޼���
#define	smTRANSCODE_CLAN_SERVICE		0x50320C00
//Ŭ�� ������Ʈ �䱸 �޼���
#define	smTRANSCODE_CLAN_UPDATE			0x50320C10

//����Ʈ ó�� �䱸 �޼���
#define	smTRANSCODE_QUEST_MESSAGE		0x50320D00

//ĳ���� �״�
#define	smTRANSCODE_DEAD_USER			0x50320E00
//��ȣ ���� ����
#define	smTRANSCODE_YAHOO_MOTION		0x50320E10
//�̺�Ʈ ���� ����
#define	smTRANSCODE_CHANGE_EVENT		0x50320F00

//Ŭ���̾�Ʈ ����
#define	smTRANSCODE_CLIENTINFO			0x50321000

//���� ��� ���
#define	smTRANSCODE_WARNING_BLINK_ATT	0x50321010


//�� ���� ��Ŷ
#define smTRANSCODE_ATTACK_DAMAGE		0x50322000
#define smTRANSCODE_RANGE_DAMAGE		0x50322010
#define smTRANSCODE_ATTACK_RESULT		0x50322020
#define smTRANSCODE_DISP_DAMAGE			0x50322030
#define smTRANSCODE_ATTACKDATA2			0x50322040

#define smTRANSCODE_LIMIT_DAMAGE		0x50322050

//Ŭ���̾�Ʈ ���� ���
#define smTRANSCODE_WARNING_CLIENT		0x50322060
#define smTRANSCODE_WARNING_SKILL		0x50322080
//�� Ŀ���� ��Ŷ
#define smTRANSCODE_COMIC_SKIN			0x50322070
//������ ��� ���
#define	smTRANSCODE_REC_ADMIN_COMMAND	0x50322090

//������ ��� ���� Ȯ��
#define smTRANSCODE_CHECK_ADMINMODE		0x503220A0

//SoD���� ������
#define smTRANSCODE_SOD_INFOMATION		0x503220B0

//�����ĳ�� ��������
#define smTRANSCODE_BLESSCASTLE_INFO	0x503220C0
#define smTRANSCODE_BLESSCASTLE_TAX		0x503220D0

//������ ������ų �ֺ� ĳ���� ����
#define smTRANSCODE_ATTACK_SLASH_LIST	0x503220E0

//�ٸ������� ���� ����� �� ����
#define smTRANSCODE_UPDATE_SERVER_PARAM	0x503220F0
#define smUPDATE_PARAM_LOWUSER_PRESENT	0x100

//���μ����� �������� ���� (��Ʈ��)
#define smTRANSCODE_USER_ID				0x50322100



//������ ��Ŷ ��ȣȭ ���
#define smTRANSCODE_DAMAGE_ENCODE_MEM	0x50322F00
#define smTRANSCODE_DAMAGE_ENCODE_MEM2	0x50322EC0

//��Ŷ ���� ��ȣȭ ���
#define	smTRANSCODE_PACKET_DYNFUNC		0x50323000


//���� ���� ����
#define	smTRANSCODE_AREA_SERVER_MAP		0x50324000
// pluto ������ ����
#define smTRANSCODE_OPEN_SMELTING		0x50325000
#define smTRANSCODE_SMELTINGITEM		0x50325001
// pluto ������ ����
#define smTRANSCODE_OPEN_MANUFACTURE	0x50326000
#define smTRANSCODE_MANUFACTURE			0x50326001
// pluto ���� ������ ����, ���� ���� ���� �÷���
#define smTRANSCODE_MANUFACTURE_WEIGHT_PRICE 0x50326002

// pluto ��������
#define smTRANSCODE_GIFTBOX_MONEY		0x50326003

//����� - ��� ������ �ʱ�ȭ ġƮ
#define smTRANSCODE_PREMIUMITEM_INIT	0x50326004

// ������ - �ͽ��� ���� ó��
#define smTRANSCODE_MIXTURE_RESET_ITEM	0x50326006

//�̺�Ʈ �÷����
#define	rsHARDCORE_EVENT_FLAG		0x100

#define smCODE_SOD_EFFECT		100			//�ֹ� �ý��� �߰� - SOD ����Ʈ ����

// �庰 - ��й� �̺�Ʈ
#define smTRANSCODE_EVENT_GAME		0x50326007
#define smCODE_EVENT_STATE_INIT		100

// �庰 - �Է�â
#define smTRANSCODE_ITEMDOC			0x50326008


#define TRANS_BUFF_SIZE		4096

#define	smCHAR_STATE_NPC		0
#define	smCHAR_STATE_ENEMY		1
#define	smCHAR_STATE_USER		0x80

#define	smCHAR_MONSTER_NATURAL	0x80
#define	smCHAR_MONSTER_GOOD		0x81
#define	smCHAR_MONSTER_EVIL		0x82

#define	smCHAR_MONSTER_NORMAL	0x00
#define	smCHAR_MONSTER_UNDEAD	0x90
#define	smCHAR_MONSTER_MUTANT	0x91
#define	smCHAR_MONSTER_DEMON	0x92
#define	smCHAR_MONSTER_MECHANIC	0x93

#define	smCHAR_MONSTER_USER		0xA0

#define	smCOMMNAD_USER_WARP		0x100
#define	smCOMMNAD_USER_AGINGUP	0x110
#define	smCOMMNAD_USER_HOLY_INCANTATION	0x120
#define	smCOMMNAD_USER_RESURRECTION	0x130
#define	smCOMMNAD_USER_FORCEORB	0x140
#define	smCOMMNAD_USER_VALENTO	0x150
#define	smCOMMNAD_USER_FIREFLOWER	0x160

#define	smCOMMAND_PLAY_BABEL		0x200
#define	smCOMMAND_PLAY_BELSS_CASTLE	0x210
#define	smCOMMAND_PLAY_PK_FIELD		0x220
#define	smCOMMAND_PLAY_KELVEZU		0x230

class	smCHAR;

class	rsRECORD_DBASE;

#define	FALLITEM_MAX			200
#define	FALLITEM2_MAX			3

struct _FALLITEM{
	DWORD	dwItemCode;
	int		Percentage;
	short	sPrice[2];
};

#define	NPC_MESSAGE_MAX		20

/*----------------------------------------------------------------------------*
*				(����) ���� ����ü
*-----------------------------------------------------------------------------*/
struct smCHAR_MONSTER_INFO {
	char	szName[32];			//�̸�

	short	SkillDamage[2];		//��ų ���ݷ�
	int		SkillRange;			//��ų ���� ����
	int		SkillRating;		//��ų ���ݷ�
	int		SkillDistance;		//���� ��ų�� �Ÿ� 
	RECT	SkillRangeRect;		//���� ��ų����� ��ġ
	int		SkillCurse;			//���� �ɱ�

	int		AttackPattern;		//���� ����
	int		ActiveHour;			//Ȱ���ð�
	int		IQ;					//��������
	int		Nature;				//ǰ��
	int		Undead;				//�𵥵� ����
	int		GenerateGroup[2];	//���ÿ� ������ ( �ּ� / �ִ� )
	int		GetExp;				//�׾����� ����ġ �ο���
	int		SpAttackPercetage;	//Ư�� ���� ��� Ȯ��
	int		MoveRange;			//�̵�����

	int		DamageStunPers;		//���ݹ����� ����Ȯ��

	int		PotionCount;		//���� ������
	int		PotionPercent;		//���� ������

	int		AllSeeItem;			//�������� �ٺ��δ�
	int		FallItemCount;		//����߸� �����۵��� ī����
	int		FallItemPerMax;		//������ ���� Ȯ�� �ִ�ġ
	int		FallItemMax;		//���ÿ� ����߸� ������ �ִ�ġ
	int		FallItemPlusCount;	//����߸� �����۵��� ī���� (�߰�)

	int		ClassCode;			//���� ���� �ڵ�

	int		UseEventModel;		//�̺�Ʈ �� ��������

	int		Real_Sight;			//���� �þ� �Ÿ� ( ������ �ƴ� )

	_FALLITEM	FallItems[FALLITEM_MAX];
	_FALLITEM	FallItems_Plus[FALLITEM2_MAX];		//�߰�������


	//NPC ���� �Ǹ� ����
	int		SellAttackItemCount;
	void	*SellAttackItem[32];	//���� ������
	int		SellDefenceItemCount;
	void	*SellDefenceItem[32];	//��� ������
	int		SellEtcItemCount;
	void	*SellEtcItem[32];		//��Ÿ ������

	int		SkillMaster;			//��ų ���� NPC
	int		SkillChangeJob;			//���� ���� NPC
	int		WareHouseMaster;		//������ ���� NPC
	int		ItemMix;				//������ ���� NPC
	int		ItemAging;				//������ ����¡
	int		CollectMoney;			//����� NPC
	int		EventGift;				//��ǰ �̺�Ʈ NPC
	int		Smelting;				// pluto	���� NPC
	int		Manufacture;			// pluto	���� NPC
	int		MixtureReset;			// ������ - �ͽ��� ���� NPC


#ifdef _LANGUAGE_VEITNAM
	int		WowEvent;
#endif

	int		EventCode;				//�̺�Ʈ �ڵ�
	int		EventInfo;				//�̺�Ʈ ����
	DWORD	dwEvnetItem;			//�̺�Ʈ ������
	int		EventNPC;				//�̺�Ʈ NPC ����
	int		ClanNPC;				//Ŭ�� NPC
	int		GiftExpress;			//��ǰ ���
	int		WingQuestNpc;			//�� ����Ʈ NPC
	int		StarPointNpc;			//�� ����Ʈ NPC
	int		GiveMoneyNpc;			//�� ��� NPC
	int		TelePortNpc;			//�ڷ���Ʈ NPC
	int		BlessCastleNPC;			//����� ĳ�� NPC
	int		PollingNpc;				//�������� NPC

	char	*szMediaPlayNPC_Title;		//������ NPC
	char	*szMediaPlayNPC_Path;		//������ NPC

	int		QuestCode;				//����Ʈ ó���ڵ�
	int		QuestParam;				//����Ʈ �Ķ����

	short	OpenCount[2];			//���� ���� ��ȣ

	DWORD	dwAutoCharCode;			//�ڵ�ĳ���� ���� �ڵ� (SpeedSum ��)

	char	*lpNpcMessage[NPC_MESSAGE_MAX];	//NPC ��ȭ �޼���
	int		NpcMsgCount;					//NPC ��ȭ ī����

};

/*----------------------------------------------------------------------------*
*				(ĳ���� �������ͽ�)ĳ���� ���� ����ü
*-----------------------------------------------------------------------------*/

struct smCHAR_INFO {
	char	szName[32];				//�̸�
	char	szModelName[64];		//�� �̸�
	char	szModelName2[60];		//�� �̸�2  // szModelName2[64] -> szModelName2[60] ��ū�� ���� ����
	DWORD	ModelNameCode2;			//�� �ڵ� �߰� (�����) // ��ū�� ���� ����

	DWORD	dwObjectSerial;			//��ü�� ������

	//int		AutoPlayer;				//�ڵ� �Ӽ�
	int		ClassClan;				//�Ҽ�Ŭ��
	int		State;					//��ü�� �Ӽ� ( smCHAR_STATE_  NPC / ENEMY / USER )
	int		SizeLevel;				//ũ�� �ܰ� ( 0-���� ~ 3-���� )
	DWORD	dwCharSoundCode;		//���� ĳ���� �ڵ�

	/*-------------------------*
	*			�ɷ�ġ 
	*--------------------------*/
	DWORD JOB_CODE;			//���� �ڵ� 
	int   Level;			//���� 
	int   Strength;			//�� 
	int   Spirit;			//���ŷ� 
	int   Talent;			//��� 
	int   Dexterity;		//��ø�� 
	int   Health;			//�ǰ� 

	/*-------------------------*
	*			��������
	*--------------------------*/
	int	  Accuracy;			//����� ���߷�
	int   Attack_Rating;	//���߷�
	int   Attack_Damage[2];	//Ÿ�ݼ�ġ ( �ּ� / �ִ� )
	int	  Attack_Speed;		//���ݼӵ�
	int	  Shooting_Range;	//�����Ÿ� ( �߻������� )
	int	  Critical_Hit;		//ũ��Ƽ���� ( 1.5�� ������ Ȯ�� )

	int   Defence;			//����� 			
	int   Chance_Block;		//����� 
	int   Absorption;		//����� 

	int   Move_Speed;		//�̵� �ӵ� 
	int   Sight;			//�þ�
	short Weight[2];			//��� �ɷ� 

	/*-------------------------*
	*			���׷� 
	*--------------------------*/
	short	Resistance[8];			//���ҿ����� ���׷� 
	short	Attack_Resistance[8];	//���ҿ����� ���ݷ� 

	/*-------------------------*
	*			...
	*--------------------------*/
	short   Life[2];			//0 ���� 1 �ִ�ġ 
	short   Mana[2];			//0 ���� 1 �ִ�ġ 
	short   Stamina[2];			//0 ���� 1 �ִ�ġ

	float	Life_Regen;			//����ɷ� 
	float	Mana_Regen;			//����ɷ� 
	float	Stamina_Regen;		//����ɷ� 

	int		Exp;
	int		Next_Exp;

	int		Money;				//������ ��

	smCHAR_MONSTER_INFO	*lpMonInfo;	//���� ����ü ������

	DWORD	Brood;				//����

	int		StatePoint;			//�ɷ�ġ ����Ʈ
	BYTE	bUpdateInfo[4];		//���� ���� ī����
	short	ArrowPosi[2];		//�κ��丮 ȭ��ǥ ��ġ
	int		Potion_Space;		//���� �ָӴ� ũ��

	int     LifeFunction;		//����� �Լ� 
	int     ManaFunction;		//��� �Լ� 
	int     StaminaFunction;	//�ٷ� �Լ� 
	short   DamageFunction[2];  //0 ���� 1 ���Ÿ�   

	DWORD	RefomCode;			//���� �ڵ�

	DWORD	ChangeJob;
	DWORD	JobBitMask;

	WORD	wPlayerKilling[2];	//�������� ����
	WORD	wPlayClass[2];		//��� ( ������ �������� )

	int		Exp_High;			//����ġ ����4��Ʈ
	DWORD	dwEventTime_T;		//�̺�Ʈ ���� �ð� - ( ��ū�� �ð� )
	short	sEventParam[2];		//�̺�Ʈ �Ķ���Ϳ� [0] �̺�Ʈ�ڵ� [1] �̺�Ʈ �Ķ����

	short	sPresentItem[2];	//�쿬�� ������ �ʿ����� ������ ����

	// �庰 - �׶��Ƽ ��ũ��
	short GravityScroolCheck[2];

	DWORD	dwTemp[11];			//���� ���� // �庰 - �׶��Ƽ ��ũ��(12->11)

	DWORD	dwLoginServerIP;		//�α����� ������ IP
	DWORD	dwLoginServerSafeKey;	//�α����� �������� ���� ����Ű

	WORD	wVersion[2];		//�� ����ü�� ����

	// �庰 - �������
//	DWORD	dwTemp2[2000];
};

//ĳ���� ���� ���� ũ��
#define	smCHAR_INFO_QUICK_SIZE		192
#define	smCHAR_INFO_QUICK_SIZE2		352

#define	THROW_ITEM_INFO_MAX		64
#define	THROW_ITEM_INFO_MASK	63
#define	INVEN_ITEM_INFO_MAX		300

//������ ������ ���� ���� ( ������ ���� ���� )
struct	sTHROW_ITEM_INFO {

	DWORD	dwCode;
	DWORD	dwKey;
	DWORD	dwSum;
};


//ĳ���� ��ǥ ����
struct rsPLAYPOS {
	DWORD	Area;		// Low->Wolrd High->Area	: ���� �����ϰ� �ִ� ������ ��ġ �ڵ�
	int		x,y,z;		// ��ǥ
};

//���� : �ڽ��� ��ġ�� ������ ������ ����
//��� : Ŭ���̾�Ʈ -> ����
//�ڵ� : smTRANSCODE_POSITION
//10�� �������� ������ �������� �÷��̾� ��ġ ����
struct TRANS_PLAYPOS {
	int size,code;
	
	rsPLAYPOS	rsPlayPos;
	short Hp[2];
};



//���� : ������� ���� ������ ������
//��� : Ŭ���̾�Ʈ -> ����  Ŭ���̾�Ʈ -> Ŭ���̾�Ʈ
//�ڵ� : smTRANSCODE_ATTACKDATA
struct TRANS_ATTACKDATA {
	int size,code;

	DWORD	dwDestObjectSerial;	//�����ϴ� ĳ���� ������ȣ
	DWORD	dwTarObjectSerial;	//���ݴ�� ĳ���� ������ȣ

	int x,y,z;			// ��ǥ
	int AttackState;	// �Ӽ�
	int AttackSize;		// ���� ����
	int Power;			// ���� �Ŀ�
	DWORD	dwChkSum;

	short	sRating[2];	//[0]-���� [1]-���߷�
	DWORD	dwDamageChkSum;
};

struct TRANS_ATTACKDATA2 {
	int size,code;

	DWORD	dwChkSum;

	DWORD	dwDestObjectSerial;	//�����ϴ� ĳ���� ������ȣ
	DWORD	dwTarObjectSerial;	//���ݴ�� ĳ���� ������ȣ

	int		x,y,z;			// ��ǥ
	int		AttackState;	// �Ӽ�
	int		AttackSize;		// ���� ����
	short	Power[2];		// ���� �Ŀ�
	short	Power2[2];		// ���� �Ŀ� ( �߰� ���� )
	short	Critical[2];	// ũ��Ƽ��
	int		SkillCode;		// ��ų�ڵ�
	DWORD	dwTime;			// Ŭ���̾�Ʈ �ð�
	int		AttackCount;	// ���� ī����
	short	MotionCount[2];	// ���� ��� ī����
	DWORD	dwWeaponCode;	// �� ���� ����
	short	Area[2];		// ����
	int		Temp[4];		// ����
};


// �庰 - ������ ��ų( 32 -> 42 )
#define SKIL_ATTACK_CHAR_MAX		42

struct TRANS_SKIL_ATTACKDATA {
	int size,code;

	DWORD	dwDestObjectSerial;	//�����ϴ� ĳ���� ������ȣ

	int x,y,z;			// ��ǥ
	int AttackState;	// �Ӽ�
	int AttackSize;		// ���� ����
	int Power;			// ���� �Ŀ�

	DWORD	dwChkSum;	//���� ���� �ڵ�

	int	TargetCount;
	DWORD	dwTarObjectSerial[SKIL_ATTACK_CHAR_MAX];	//���ݴ�� ĳ���� ������ȣ
};

struct TRANS_SKIL_ATTACKDATA2 {
	int size,code;

	DWORD	dwDestObjectSerial;	//�����ϴ� ĳ���� ������ȣ

	DWORD	dwChkSum;

	int		x,y,z;			// ��ǥ
	int		AttackState;	// �Ӽ�
	int		AttackSize;		// ���� ����
	short	Power[2];		// ���� �Ŀ�
	short	Power2[2];		// ���� �Ŀ� ( �߰� ���� )
	short	Critical[2];	// ũ��Ƽ��
	int		SkillCode;		// ��ų�ڵ�
	DWORD	dwTime;			// Ŭ���̾�Ʈ �ð�
	int		AttackCount;	// ���� ī����
	short	MotionCount[2];	// ���� ��� ī����
	DWORD	dwWeaponCode;	// �� ���� ����
	short	Area[2];		// ����
	int		Temp[4];		// ����

	DWORD	dwMainTargetObject;		//�� Ÿ��

	int	TargetCount;
	DWORD	dwTarObjectSerial[SKIL_ATTACK_CHAR_MAX];	//���ݴ�� ĳ���� ������ȣ
};



//���� : ������ �����ϴ� �ٸ� �÷��̾� ���� ��� ����
//��� : ���� -> Ŭ���̾�Ʈ
//�ڵ� : smTRANSCODE_IPLIST
struct TRANS_AREAINFO_HEADER {
	int		size;
	int		code;
	DWORD	dwIP;
	int		Stats;					//���� �Ϸ�� TRUE ���� �� ���� ����Ÿ�� ������ FALSE
	int		TransListCnt;			//�����ϴ� TRANS_IPLIST ����
};

struct TRANS_IPLIST {
	DWORD	dwIP;
	rsPLAYPOS	Pos;
};

/////////////// ������ �ù� ���� /////////////////
#define	POST_ITEM_MAX				500

struct _POST_BOX_ITEM {
	int		Flag;
	char	szCharName[32];
	char	szItemCode[32];
	char	szSpeJob[32];
	char	szDoc[128];
	char	szFormCode[64];
	char	szPassCode[16];

	DWORD	dwItemCode;
	DWORD	dwJobCode;
	DWORD	dwFormCode;
	DWORD	dwPassCode;
	DWORD	dwParam[4];
};

struct	rsPOST_BOX_ITEM {
	DWORD	dwHead;
	int		ItemCounter;

	_POST_BOX_ITEM	PostItem[POST_ITEM_MAX];
};

struct	TRANS_POST_ITEM {
	int		size,code;

	char	szItemName[32];
	char	szDoc[128];
	DWORD	dwItemCode;
	DWORD	dwItemJobCode;
	DWORD	dwParam[4];

	DWORD	dwItemFlag;
	int		Weight;			// pluto �ؿ� ��� ������ ����
};
//	sITEM_INFO	sItemInfo;


///////////////////////////////////////////////////////


#define	srPLAYBUFF_MAX		4
#define	srPLAYBUFF_MASK		3

#define	PARTY_PLAYER_MAX		6

// �庰 - �������
#define	srRECORD_DEFAULT_SIZE	16384
//#define	srRECORD_DEFAULT_SIZE	32768
#define	srRECORD_MEMORY_SIZE	24576
//#define	srRECORD_MEMORY_SIZE	49152

#define SKILL_POINT_COLUM_MAX	16

struct	TRANS_PARTY_PLAYINFO;

struct	npGAME_GUARD_AUTH_INFO {
	 DWORD	Auth_Value;		//���Ӱ��� Ȯ���ڵ� ( ���� ���� ���� )
	 DWORD	Auth_Time;		//���Ӱ��� Ȯ�νð� ( ����ó�� �ð� )
	 DWORD	Auth_Count;		//���Ӱ��� Ȯ��ī���� ( �� ���� Ƚ�� )
	 DWORD	Auth_Retry;		//���Ӱ��� Ȯ�� ��õ� ( ������ 60�� �ȿ� �ȿ��� ������� ��õ� )
};

//������ ���� ī����
struct	sMYSHOP_SELL_INFO {
	DWORD	dwCode;
	DWORD	dwHead;
	DWORD	dwChkSum;
	int		Price;
};

#define	MYSHOP_LIST_MAX		32
#define	MYSHOP_LIST_MASK	31


struct	sCASTLE_MASTER {
	int		Flag;
	DWORD	dwCheckTime;
	int		SelectSkill;
};

//����Ÿ ���� ��Ʈ �÷�
#define	rsDATA_ERROR_WAREHOUSE			1


//������ �ڸ�� ���ۿ� ����
struct smTRANS_COMMAND
{
	int	size;
	int code;

	int LParam;
	int WParam;
	int SParam;
	int EParam;
};

//����� - XignCode
#define PACKET_SIZE 512


//����� - XignCode
typedef struct _XIGNCODE_PACKET {
	int size;
	int code;
	BYTE data[PACKET_SIZE]; //����� - XignCode
} XIGNCODE_PACKET;

typedef struct _XIGNCODE_TempData{
	BOOL m_type;
	BOOL m_ceComplete;
	DWORD	dwWaitTime_NC;
	DWORD	dwWaitTime_CE;
	BOOL   dwClientReady;
	HANDLE xigncode_handle;
	int m_pcount;
} XIGNCODE_TempData;

//����� - XignCode
#define smTRANSCODE_XIGNCODE_PACKET		0x48470400
#define smTRANSCODE_XIGNCODE_START		0x48470500


//////////////////// ����Ʈ�� ////////////////////
#ifdef _xTrap_GUARD

#include "XTrapSvrSrc/XTrapCC.h"
//smPacket.h
#define	smTRANSCODE_XTRAP_PACKET		0x48470300
#define	smXTRAP_COMMAND_LOGIN			0x00001000
#define	smXTRAP_COMMAND_SECURITY		0x00001010

struct	smTRANS_XTRAP_ADDR {
	smTRANS_COMMAND	smTransCommand;
	char Addr[528];				/* Client CREATEKF_IN_SIZE == Server XC_ADDR_SIZE */

};
struct	smTRANS_XTRAP_SECURITY {

	smTRANS_COMMAND	smTransCommand;
	char serialkey[1088];		/* Client CREATEKF_OUT_SIZE == Server XC_CLIENTBUF_SIZE */

};

struct	smXTRAP_PLAYINFO {
	//XC_CMF				Cmf[2];
	XC_KEYVALUE			KeyValue[2];
	DWORD				dwKeyGenTime;
	int					KeyGenWaitCount;
};

#ifdef _W_SERVER
#define _xTrap_GUARD_SERVER
#endif

#endif
////////////////////////////////////////////////////

//////////////////// ����Ʈ�� ////////////////////
#ifdef _XTRAP_GUARD_4_

#ifdef _W_SERVER

#define _XTRAP_GUARD_4_SERVER
#include "XTrapSrcD5/Server/XTrap4Server.h"	//XTrapD5

#else

#define _XTRAP_GUARD_4_CLIENT

#endif

#include "XTrapSrcD5/Server/MessageProtocol.h"
//smPacket.h
#define	smTRANSCODE_XTRAP_PACKET		0x48470300
#define	smXTRAP_COMMAND_LOGIN			0x00001000
#define	smXTRAP_COMMAND_SECURITY		0x00001010


struct	smTRANS_XTRAP_ADDR {
	smTRANS_COMMAND	smTransCommand;
	char PacketBuf[XTRAP_CC4_BUFFERSIZE_COMMUNICATION_BASE];
};


#endif
////////////////////////////////////////////////////


/////////////////////// ���������� �÷��̾� ���� ����ü ////////////////////////////////
class rsPLAYINFO {
public:

	char			szServerID[32];			//����ID

	char			szID[32];				//���� ID
	char			szName[32];				//���� ĳ���� �̸�
	DWORD			dwCode_ID;				//ID�� ���� ã������ �ڵ�
	DWORD			dwCode_Name;			//Name�� ���� ã�� ���� �ڵ�

	int				AdminOperMode;			//������ ���� �ε���

	DWORD			dwDataError;			//����Ÿ ���� ��Ʈ �÷�

	int				TransInfoFlag;			//��� �÷� ( 0-�غ��� 1-��밡�� )
	int				ChatBuffCnt;

	int				SystemLine;				//�ý��� ��� - �ٸ� �������� �����

	int				CharLoadingCount;		//ĳ���� �ε� �õ� ī���� (�Ѱ��� ������ ������)

	rsPLAYPOS		Position;
	rsPLAYINFO		**lpAreaPlayInfo;		//������ �÷��̾� ���� ��ũ������ 

	int				SendListCnt;

	DWORD			dwDecPacketCode;		//��Ŷ ��ȣ �ڵ�
	BYTE			bPacketXor[2];			//��Ŷ ��ȣ �ص��� ��
	DWORD			dwDecPacketTime;		//��Ŷ ��ȣ ���� �ð�
	DWORD			dwDecPacketTime2;		//��Ŷ ��ȣ ���� ���� �ð�

	smCHAR_INFO		smCharInfo;				//ĳ���� ����

	int				CharLevelMax;			//ĳ���� �ְ� ���� ( ���� ������ ������ �ִ� ĳ������ �ְ���� )
	DWORD			dwNoticeCount;			//�������� �޼��� ī����

	char			srPlayBuff[srPLAYBUFF_MAX+1][TRANS_BUFF_SIZE];		//������ ����Ÿ ���� ����
	int				srPlayBuffSize[srPLAYBUFF_MAX+1];					//������ ����Ÿ ���� ���� ũ��

	int				PlayBuffSleep;									//���� ����Ÿ ����

	sTHROW_ITEM_INFO	ThrowItemInfo[THROW_ITEM_INFO_MAX];			//������ ������ ����

	sTHROW_ITEM_INFO	InvenItemInfo[INVEN_ITEM_INFO_MAX+16];			//�κ��丮 ������ ����
	sTHROW_ITEM_INFO	WareHouseItemInfo[120];						//â�� ������ ����
	int				OpenWarehouseInfoFlag;								//â�� ������ ���� ����
	//sTHROW_ITEM_INFO	*lpWareHouseItemInfo;						//â�� ������ ���� ����

	DWORD			dwTradePotionInfo[16];							//���� �ŷ� ���� �ӽ�����
	int				TradePotionInfoCount;							//���� �ŷ� ���� ���� ī����

	sMYSHOP_SELL_INFO	sMyShopList[ MYSHOP_LIST_MAX ];					//���� ���� �ŷ� ��� ����
	int				MyShopListCount;								//���� ���� �ŷ� �õ� ��� ī����
	DWORD			dwMyShopTradeTime;								//���λ��� �ŷ� ��� �ð�

	int				ThrowItemCount;											//������ ������ ī����

	int				UnsaveMoney;									//������� ���� ��

	int				ServerMoney;									//�������� ���� ����� ��
	int				WareHouseMoney;									//â�� �ݾ�

	DWORD			dwRecvTimePlayBuff;								//���۹��� �ð�
	DWORD			srSendPlayBuffCnt;								//���� ���� ī����
	DWORD			srRecvPlayBuffCnt;								//���� ���� ī����

	DWORD			dwObjectSerial;	//��ü�� ������

	int				ChatMsgMode;			//ä�� �޼��� ���
	char			szChatMessage[256];		//ä�� �޼���

	//����
	char			szTradeMessage[128];	//���� �޼��� ���ڿ�
	DWORD			dwTradeMsgCode;			//���� �޼��� �ڵ�

	smWINSOCK		*lpsmSock;

	DWORD			dwLastRecvTime;
	DWORD			dwLastMotionRecvTime;

	int				UpdateInfo_WaitCounter;	//������Ʈ ���� ��� ī����
	int				UpdateInfo_WarnigCounter;	//������Ʈ ���� ��� (�����Ǹ� ���� ���� )

	rsRECORD_DBASE	*lpRecordBase;			//���� Ŭ���� ������
	rsPOST_BOX_ITEM	*lpPostBoxItem;			//������ ��� ����ü
	int				PostPassFailCount;		//������ ��� ��й�ȣ ���� Ƚ��

	int				AdminMode;				//������ ���
	int				HideMode;				//���� ��� ( �����ڰ� ������ �Ⱥ��̰� �ٴҶ� )
	int				AdminIP;				//������ ������ �ּҷ� ����

        const char* getAccount() const { return szID; }
        const char* getName() const { return smCharInfo.szName; }
        AccountTypes getSecurity() const { return (AccountTypes)AdminMode; }
        void setAccount(const char* v) { if(v) strcpy_s(szID, v); }
        void setAccount(char v) { szID[0]=v; }
        void setAccountId(int v) { dwCode_ID=v; }
        void setPassword(char v) { }
        void setName(const char* v) { if(v) strcpy_s(smCharInfo.szName, v); }


	DWORD			dwConnectTime;			//������ �ð�

	int				LastTransPlayUserCount;	//���������� ���۵� �ٸ� �÷��̾� ��ȣ
	int				LastTransAutoPlayCount;	//���������� ���۵� �ٸ� �ڵ� ĳ���� ��ȣ

	DWORD			dwConnectCount;			//��ü ���� ī����

	int				frame;
	DWORD			SendPlayGroupCount;		//���� ����Ÿ �׷� ī����

	DWORD			MonTranCount;			//����/NPC ���� �غ�
	DWORD			CharTransCount;			//ĳ���� ���� �غ�

	int				HitMonsterCount;		//������ ���� ī����
	smCHAR			*lpChrTarget;			//������ �ֱ� ������ ���� ���(�켱���� ����)
	DWORD			dwTarObjSerial;			//������ �ֱ� ������ ���� ������ȣ

	int				UserPacketLoop;			//���� 1���ν� ���۵� ��Ŷ ��ġ ( ������� ��ġ )
	int				BefUserPacketLoop;		//���� 1���ν� ���۵� ��Ŷ ��ġ ( ������� ��ġ )

	int				AttackCount;			//���� ī����			
	DWORD			dwAttackTime;			//���� �ð�
	DWORD			dwLastAttackTime;		//�ֱ� ���ݽð�
	int				DamagePacketCount;		//���� ��Ŷ ī����
	DWORD			dwDamagePacketTime;		//������ ��Ŷ�� ���� Ŭ���̾�Ʈ �� �ð�

	int				MonsterKillCounter;		//��Ʈ ���� ī����
	DWORD			dwLastTradeChksum;		//������ �ŷ��� ýũ��
	DWORD			dwLastTradeTime;		//������ �ŷ��� �ð�

	//////////// ������ ���� ///////////////
	short			sLowLevel_PresentItem[2];	//������ ������ ����

	DWORD			dwStartUser_PresentItem_Time;	//�������� ������ ����
	DWORD			dwStartUser_PresentItem_Count;	//�������� ������ ����



	////////////////////////////////////////////////
	// ���� �ۼ��� ����Ÿ

	int				Send_AttackCount;
	int				Send_AttackDamage;
	int				Recv_AttackCount;
	int				Recv_AttackDamage;

	/////////////// ��ũ�� ���� ////////////////////

	DWORD			dwMacroTime;
	int				Macro_PosX;
	int				Macro_PosZ;
	int				Macro_AngY;
	DWORD			dwMacroAngTime;
	DWORD			dwMacroChkTime;


	/////////////// ��Ÿ ///////////////////////////
	DWORD			dwSendFrames[CONNECTMAX];	//�ٸ� ĳ���͵� ����Ÿ ���� ������

	int				CrackWarning;				//ũ�� ��� ����
	DWORD			dwTime_Disconnect;			//������� ����
	int				BadPlayer;					//���� �÷��̾� ( ��� ���� ��ȿ )
	DWORD			dwPacketAttackCount;		//������ ��Ŷ���� ���� ���� ���� 
	DWORD			dwPacketAttackTime;			//������ ��Ű���� �������� �ð�
	int				WarningCount;				//��� ī��Ʈ

	char			*lpRecordDataBuff;			//���ڵ嵥��Ÿ ���� ( �������� �޸𸮹��� )
	DWORD			dwRecordDataTime;			//�ֱ� ��Ͻð�

	int				EventMode;					//�̺�Ʈ ����
	int				EventScore;					//�̺�Ʈ ����
	int				EventCount;					//���� ī��Ʈ
	DWORD			dwEventTime;				//�̺�Ʈ ���� �ð�
	short			sEventParam[2];				//�̺�Ʈ ���� �Ķ����

	int				AreaOutCount;				//����ħ�� ī����

	int				BlockTime;					//���� �����ų �ð�

	int				WhisperMode;				//�Ӹ� ���

	int				Event_StarTicketLevel;		//�� ��ǰ�� �߻� ����


	////////////// ���� ���� ��ġ ��� //////////////////////

	INT64			spExp_Start;				//���۽� ����ġ
	int				spMoney_Start;				//���۽� ��
	int				spLevel_Start;				//���۽� ����

	int				spExp_InOut_Total;			//����ġ ��� ��Ż
	int				spMoney_In_Total;			//�� �Ա� ��Ż
	int				spMoney_InOut_Total;		//�� ��� ��Ż

	int				spEXP_Out;					//����ġ ���
	int				spMoney_In;					//�� �Ա�
	int				spMoney_Out;				//�� ���

	DWORD			dwLastExpMoneyCheckTime;	//���������� ����ġ �ݾ� ���� �ð�
	DWORD			dwLastExpMoneyTransServer;	//�������� ����ġ �ݾ� ��ġ ������ �ð�

	DWORD			dwHopeDisconnectTime;		//���������� ������ �ð��� �Ǹ� ���� ����

	int				Client_Version;				//������� Ŭ���̾�Ʈ�� ���� ����

	DWORD			dwLastSaveTime;				//���������� ������ �ð�

	DWORD			dwGameServerExp[8];			//���Ӽ��� ����ġ ����

	DWORD			dwNSpriteTime;				//nSprite �˻��� �ð�

	//Ŭ���̾�Ʈ ��� ýũ
	DWORD			dwClientFuncChk_1;			//ýũ�� ���1
	DWORD			dwClientFuncChk_2;			//��ũ�� ���2
	int				FuncChkFailCount1;			//����
	int				FuncChkFailCount2;			//����
	int				FuncChkSucessCount1;		//����
	int				FuncChkSucessCount2;		//����
	DWORD			dwFuncChkErrorFlag;			//����
	int				ClientFunChk1_Count;
	int				ClientFunChk2_Count;

	DWORD			dwClientFunChk_NextTime;	//���� ýũ�� �ð�

	int				RecordWarningCount;			//���� ������ ��� ī����
	int				RecordWarningExp;			//��� ���� ����ġ

	DWORD			dwClentOS;					//Ŭ���̾�Ʈ OS
	WORD			wClentOSVersion[2];			//Ŭ���̾�Ʈ ���� [0]-���� [1]-����ȣ
	DWORD			dwClentModulChkSum;			//Ŭ���̾�Ʈ ��� ýũ��
	int				ClentModuleCount;			//Ŭ���̾�Ʈ ��ƫ ����

	DWORD			dwDamagePacketKey[2];		//������ ��Ŷ ��ȣ�� Ű ( ���ſ� )
	DWORD			dwDamagePacketKey2[2];		//������ ��Ŷ ��ȣ�� Ű ( �۽ſ� )


	////////////// ��Ƽ ���� ���� ///////////////////
	DWORD			PartyUpdateFlag;					//��Ƽ ���� ������Ʈ �䱸
	DWORD			dwPartyInfo;						//��Ƽ ����
	rsPLAYINFO		*lpPartyMaster;						//��Ƽ�� ����Ʈ
	DWORD			PartyMasterObjectCode;				//��Ƽ�� ������Ʈ �ڵ�
	rsPLAYINFO		*lpPartyPlayers[PARTY_PLAYER_MAX];	//��Ƽ�� ����Ʈ
	DWORD			dwPartyObjectCode[PARTY_PLAYER_MAX];//��Ƽ���� ������Ʈ �ڵ�
	DWORD			PartyLevelAverage;					//��Ƽ�� ���� ���
	int				PartyUserCount;						//��Ƽ�� ī��Ʈ

	///////////// Ŭ�� ���� ���� ////////////////////
	DWORD			dwClanCode;							//������ Ŭ�� �ڵ�
	int				ClanTicketCode;						//Ŭ�� Ƽ�� �ڵ� ( Clan.DLL ���� �߱� )
	int				SucessCheck_ClanCode;				//Ŭ�� �ڵ� Ȯ�� ����


	//////////////////// ��� ���� ���� /////////////////////////

	char	szGuid[128];			//�α��� �ĺ���
	char	szPwd[32];			//�н�����
	char	szGPCode[16];		//��ǰ�ڵ�
	time_t	DisuseDay;			//���� ��¥
	int		UsePeriod;			//�����ð� ��
	int		GpCode;				//��ǰ�ڵ�
	int		GpType;				//��ǰŸ�� ( ���� / PC�� )
	DWORD	dwPeriodTime;		//���� �ð�
	int		Bl_RNo;				//PC�� ��� ����
	int		Bl_Meter;			//������
	DWORD	dwPeriodTime2;		//������ ����ð� ( �±��̼����� ��� )


	////////////////// ������ �ۼ��� ���� �ڵ� /////////////////////

	int		SndItemSecCodeCount;			//������ ���� ���� �ڵ� ī����
	int		RcvItemSecCodeBuff[8][16];		//���� ������ ���� ī���� ���� ���� ����

	/////////////////////////////////////////////////////////////////

	short	sLimitDamage[2];				//���ݷ� ����
	short	sLimitCritical[2];				//ũ��Ƽ�� ����

	/////////////////////////////////////////////////////////////////

	int		ServerPotion[3][4];				//���హ�� Ȯ�ι���

	//////////////////////////// ��ų ó�� //////////////////////////////

	//������̼�
	DWORD	dwSkill_AutoMation_Time;
	int		dwSkill_AutoMation_Param;
	//�ƽø�����
	DWORD	dwSkill_Maximize_Time;
	int		dwSkill_Maximize_Param;
	//������
	DWORD	dwSkill_PhysicalAbsorb_Time;
	int		dwSkill_PhysicalAbsorb_Param;
	//�ͽ�Ʈ�� ����
	DWORD	dwSkill_ExtreamShield_Time;
	int		dwSkill_ExtreamShield_Param;

	//Ȧ�� �ٵ�
	DWORD	dwSkill_HolyBody_Time;
	int		dwSkill_HolyBody_Param;
	//Ȧ�� �뷯
	DWORD	dwSkill_HolyValor_Time;
	int		dwSkill_HolyValor_Param;
	//Ȧ�� ���÷���
	DWORD	dwSkill_HolyReflection_Time;
	int		dwSkill_HolyReflection_Param;
	//��þƮ ����
	DWORD	dwSkill_EnchantWeapon_Time;
	int		dwSkill_EnchantWeapon_Param;


	//��Ż�Ƹ�
	DWORD	dwSkill_MetalArmor_Time;
	int		dwSkill_MetalArmor_Param;

	//����ũ ����
	DWORD	dwSkill_SparkShield_Time;
	int		dwSkill_SparkShield_Param;

	//���� �ø���
	DWORD	dwSKill_Falcon_Time;
	short	dwSKill_Falcon_Damage[2];

	//����� ��Ȧ���̼�
	DWORD	dwSkill_DivineInhalation_Time;
	DWORD	dwSkill_DivineInhalation_Param;

	//Ʈ���̾��� ���Ҷ�
	DWORD	dwSkill_TriumphOfValhalla_Time;
	DWORD	dwSkill_TriumphOfValhalla_Param;

	//����� ������
	DWORD	dwSkill_VirtualLife_Time;
	DWORD	dwSkill_VirtualLife_Param;

	//����������
	DWORD	dwSkill_EnergyShield_Time;
	DWORD	dwSkill_EnergyShield_Param;

	//���Ǹ�������Ż
	DWORD	dwSkill_SpiritElemental_Time;
	DWORD	dwSkill_SpiritElemental_Param;

	//��̼ҵ�
	DWORD	dwSkill_DancingSword_Time;
	DWORD	dwSkill_DancingSword_Param;
	DWORD	dwSkill_DancingSword_DelayTime;

	//��Ͻ�
	DWORD	dwSkill_Vanish_Time;
	DWORD	dwSkill_Vanish_Param;

	//��Ͻ�
	DWORD	dwSkill_Compulsion_Time;
	DWORD	dwSkill_Compulsion_Param;

	//���׳�ƽ���Ǿ�
	DWORD	dwSkill_MagneticSphere_Time;
	DWORD	dwSkill_MagneticSphere_Param;

	//����Ŀ
	DWORD	dwSkill_Berserker_Time;
	DWORD	dwSkill_Berserker_Param;

	//��ž���
	DWORD	dwSkill_AssassinEye_Time;
	DWORD	dwSkill_AssassinEye_Param;

	//����
	DWORD	dwSkill_Vague_Time;
	DWORD	dwSkill_Vague_Param;

	//�������������
	DWORD	dwSkill_ForceOfNature_Time;
	WORD	wSkill_ForceOfNature_Param[2];

	//���鸮 ����
	DWORD	dwSkill_GodlyShied_Time;
	DWORD	dwSkill_GodlyShied_Param;

	//���������
	DWORD	dwSkill_GodsBless_Time;
	DWORD	dwSkill_GodsBless_Param;

	//Ȧ������Ҷ�
	DWORD	dwSkill_HallOfValhalla_Time;
	WORD	wSkill_HallOfValhalla_Param[2];

	//���ν�Ʈ����
	DWORD	dwSkill_FrostJavelin_Time;
	DWORD	dwSkill_FrostJavelin_Param;

	//�����ʷ��ʵ�
	DWORD	dwSkill_RegenerationField_Time;

	//���� ������
	DWORD	dwSkill_SummonMuspell_Time;
	DWORD	dwSkill_SummonMuspell_Param;


	//////////////////////////// ���� ó�� //////////////////////////////
	//��ť���� �ø��� ����
	DWORD	dwCurse_Attack_Time;		//���� ����
	DWORD	dwCurse_Attack_Param;		//���� ����

	DWORD	dwCurse_Defence_Time;		//��� ����
	DWORD	dwCurse_Defence_Param;		//��� ����

	////////////////////////// ��ų ����Ʈ ���� //////////////////////

	BYTE	bSkillPoint[SKILL_POINT_COLUM_MAX];

	////////////////////////////////////////////////////////////////


	//////////////////////////// �������� ////////////////////////////

	DWORD	dwForceOrb_Code;			//�������� ������ �����ڵ�
	DWORD	dwForceOrb_Time;			//�������� �����ð�
	DWORD	dwForceOrb_Damage;			//�������� ������

	DWORD	dwForceOrb_SaveCode;		//�������� ������ �����
	DWORD	dwForceOrb_SaveTime;		//�������� ������ �����
	DWORD	dwForceOrb_SaveDamage;		//�������� ������ �����
	DWORD	dwForceOrb_NextSetTime;		//�������� ������� ������ ����ð�

	// ����� - �ν��� ������(�����)
	DWORD	dwLifeBooster_Code;			//�ν��� ������(�����) �����ڵ�
	DWORD	dwLifeBooster_Time;			//�ν��� ������(�����) �����ð�
	DWORD	dwLifeBooster_Data;			//�ν��� ������(�����) ���� �ۼ�Ʈ

	DWORD	dwLifeBooster_SaveCode;		//�ν��� ������(�����) ������ �����
	DWORD	dwLifeBooster_SaveTime;		//�ν��� ������(�����) ������ �����
	DWORD	dwLifeBooster_SaveData;		//�ν��� ������(�����) ������ ����� ���� �ۼ�Ʈ
	DWORD	dwLifeBooster_NextSetTime;	//�ν��� ������(�����) ������� ������ ����ð�

	// ����� - �ν��� ������(���)
	DWORD	dwManaBooster_Code;			//�ν��� ������(���) �����ڵ�
	DWORD	dwManaBooster_Time;			//�ν��� ������(���) �����ð�
	DWORD	dwManaBooster_Data;			//�ν��� ������(���) ���� �ۼ�Ʈ

	DWORD	dwManaBooster_SaveCode;		//�ν��� ������(���) ������ �����
	DWORD	dwManaBooster_SaveTime;		//�ν��� ������(���) ������ �����
	DWORD	dwManaBooster_SaveData;		//�ν��� ������(���) ������ ����� ���� �ۼ�Ʈ
	DWORD	dwManaBooster_NextSetTime;	//�ν��� ������(���) ������� ������ ����ð�

	// ����� - �ν��� ������(ü��)
	DWORD	dwStaminaBooster_Code;			//�ν��� ������(�ٷ�) �����ڵ�
	DWORD	dwStaminaBooster_Time;			//�ν��� ������(�ٷ�) �����ð�
	DWORD	dwStaminaBooster_Data;			//�ν��� ������(�ٷ�) ���� �ۼ�Ʈ

	DWORD	dwStaminaBooster_SaveCode;		//�ν��� ������(�ٷ�) ������ �����
	DWORD	dwStaminaBooster_SaveTime;		//�ν��� ������(�ٷ�) ������ �����
	DWORD	dwStaminaBooster_SaveData;		//�ν��� ������(�ٷ�) ������ ����� ���� �ۼ�Ʈ
	DWORD	dwStaminaBooster_NextSetTime;	//�ν��� ������(�ٷ�) ������� ������ ����ð�


	//�ð��� ��� ������ ����
	DWORD	dwPrimeItem_NextSetTime;	//�ð��� ��������� ���μ����뺸 ����ð�
	DWORD	dwTime_PrimeItem_X2;		//������ 2�� ���
	DWORD	dwTime_PrimeItem_ExpUp;		//����ġ�� ����
	DWORD	dwPrimeItem_PackageCode;	//��Ű�������� �ڵ�

	DWORD	dwTime_PrimeItem_VampCuspid;	//���Ǹ�Ŀ����
	DWORD	dwTime_PrimeItem_ManaRecharg;	//��������¡
	DWORD	dwTime_PrimeItem_MightofAwell;	// pluto ����Ʈ ���� ����
	DWORD	dwTime_PrimeItem_MightofAwell2;	// pluto ����Ʈ ���� ����
	DWORD	dwTime_PrimeItem_ManaReduce;	// pluto ���� ���ེ
	DWORD	dwTime_PrimeItem_PhenixPet;		// pluto ��(�ؿ�)
	DWORD	dwTime_PrimeItem_HelpPet;	// ����� - ��� ����� �� �߰�
	DWORD	dwTime_PrimeItem_VampCuspid_EX;	// �庰 - ���Ǹ� Ŀ���� EX
	DWORD	dwTime_PrimeItem_StaminaReduce;	// ����� - �ٷ� ���ེ ����

	//pC�� ��
	DWORD	dwPCBang_Pet_AttackTime;	//PC�� �� ����Ÿ��
	DWORD	dwPCBang_Pet_NextSetTime;	//PC�� �� ���� �����뺸 ����ð�

	// ����� - ��� ����� �� �߰�
	DWORD	dwHelp_Pet_AttackTime;	//��� ����� �� ����Ÿ��
	DWORD	dwHelp_Pet_NextSetTime;	//��� ����� �� ���� �����뺸 ����ð�

	// �庰 - ��ų �����
	DWORD	dwSkillDelay_Code;			
	DWORD	dwSkillDelay_Time;			
	DWORD	dwSkillDelay_Data;			

	DWORD	dwSkillDelay_SaveCode;		
	DWORD	dwSkillDelay_SaveTime;		
	DWORD	dwSkillDelay_SaveData;		
	DWORD	dwSkillDelay_NextSetTime;



	//////////////////////////// ��ȯ�� ���� ///////////////////////
	smCHAR	*lpLinkChar;
	DWORD	dwLinkCharCode;

	/////////////////////////// ���� ���� //////////////////////////
	short	sBlessCastle_Damage[2];		//0-Damage 1-Killing Count
	DWORD	dwCheck_MasterClanTime;		//���� Ŭ�� Ȯ�� �ð�


	sCASTLE_MASTER	CastleMaster;		//���� ���� ����
	DWORD	dwBlessSkill_Code;			//��ų ���� �ڵ�
	DWORD	dwBlessSkill_Time;			//��ų ���� �ð�
	DWORD	dwBlessSkill_Param;			//��ų ���� �Ķ����

	DWORD	dwSiegeItem_Scroll_Time;	//���� ������ ��ũ��
	DWORD	dwSiegeItem_Scroll_Code;
	DWORD	dwSiegeItem_Scroll_Param;

	DWORD	dwSiegeItem_Stone1_Time;		//���� ������ ���׼�
	DWORD	dwSiegeItem_Stone1_Code;
	DWORD	dwSiegeItem_Stone1_Param;

	DWORD	dwSiegeItem_Stone2_Time;		//���� ������ ��ȭ��
	DWORD	dwSiegeItem_Stone2_Code;
	DWORD	dwSiegeItem_Stone2_Param;

	short	sCheckPlayField_Count[2];		//���� �ʵ� ��ġ Ȯ�� ī����


	//////////// Quest ���� �ڵ� ���ȿ� //////////////

	DWORD	dwHaQuestItemCount;					//������Ʈ ������ ���� ī��Ʈ


	///////////// ��Ʈ�� ����ġ �й� ���� /////////////////////
	char	szUserExpTimeID[32];		//���� ID(��Ʈ�� ���μ���)
	char	szUserExpTimeName[32];		//���� Name(��Ʈ�� ���μ���)
	DWORD	dwExpGameTime;				//�÷��̽ð�
	DWORD	dwExpGameTimeCheck;			//�÷��̽ð�ýũ��



//���Ӱ��� ���� Lib
#ifdef	_npGAME_GUARD_AUTH
	 CCSAuth csa;
	npGAME_GUARD_AUTH_INFO	npGameGuard;
#endif

#ifdef _xTrap_GUARD
	smXTRAP_PLAYINFO	smXTrapInfo;
#endif

#ifdef _XTRAP_GUARD_4_SERVER
	//smXTRAP_PLAYINFO	smXTrapInfo;
	XTRAPCS_SESSION_BUF		XTrapCS_Session_Buf;
#endif

// ����� - XignCode
#ifdef _XIGNCODE_SERVER
	XIGNCODE_TempData	Xigncode_tempdata;
	int CE_code_count;
#endif


	////////////////////////////////////////////////////////////////


	//��Ƽ��
	int	CreateParty( rsPLAYINFO *lpPlayInfo );			//��Ƽ���� - ( �ʱ� ��Ƽ�� ���� )
	int	JoinPartyPlayer( rsPLAYINFO *lpPlayInfo );		//��Ƽ�� ����
	int	DeletePartyPlayer( rsPLAYINFO *lpPlayInfo );	//��Ƽ���� Ż��
	int ChangePartyPlayer( rsPLAYINFO *lpOldPlayInfo , rsPLAYINFO *lpNewPlayInfo );	//��Ƽ�� ����
	int	UpdatePartyPlayer();							//��Ƽ�� ������Ʈ
	int	UpdatePartyPlayInfo();							//��Ƽ�÷��� ������Ʈ
	int GetPartyMoney( sITEMINFO *lpDefItemInfo , rsPLAYINFO *lpPlayInfo );	//��Ƽ�� �� ȹ��
	int GetPartyExp( smCHAR *lpChar , rsPLAYINFO *lpPlayInfo );				//��Ƽ�� ����ġ ȹ��
	int PartyChatting( rsPLAYINFO *lpPlayInfo );			//��Ƽ�� ä�ø޼��� ����

	int	UpdateServerPartyPlayer( TRANS_PARTY_PLAYINFO *lpTransPartyPlayInfo );	//���� ������ ��Ƽ�� ���� ������Ʈ /smTRANSCODE_PARTY_PLAYUPDATE


	int	LeavePartyMaster( rsPLAYINFO *lpPlayInfo );		//��Ƽ�� ����
	int	ReleaseParty();									//��Ƽ �ػ�

	//��Ƽ������ �޼��� ����
	int SendPartyMessage( char *szMessage , DWORD dwCode );		//��Ƽ�������� ������ �޼���
	//��Ƽ������ ����Ÿ ����
	int SendPartyData( rsPLAYINFO *lpPlayInfo_From , char *szData  );

	//��Ƽ��
	int	JoinParty( rsPLAYINFO *lpPlayMaster );			//��Ƽ ���� ��û
	int	SecedeParty ();									//��Ƽ Ż��
	int	DisconnectPartyUser();							//��Ƽ�� ���� ������


	int	AddServerMoney( int Money , int WhereParam=0 );		//���� �Ӵ� �߰�
	int	SubServerMoney( int Money , int WhereParam=0 );		//���� �Ӵ� ����
	int	SetServerMoney( int Money , int WhereParam=0 );		//���� �Ӵ� ����

	int	CallBack_ClanMarkNum( int ClanMarkNum );			//Ŭ����ũ ��ȣ �˷���


};
/////////////////////////////// ��� ���� ///////////////////////////////


// ���ϰ�: 	-1 : ���ӻ�ǰ ����
//		 0 : ����		- ��¥
//		 1 : ����(�ð�)       	- �����ð�
//		 2 : ����
//		 3 : ����		- �д� �� ( ��Ż - ���� )
//	PG :   "0" : ����
// 	       "1" : PC��

#define BILLING_GPTYPE_DAY		0
#define BILLING_GPTYPE_TIME		1
#define BILLING_GPTYPE_FREE		2
#define BILLING_GPTYPE_POTAL	3

#define BILLING_PGTYPE_PERSONAL	0
#define BILLING_PGTYPE_PCROOM	0




/////////////////////////////////////////////////////////////////////////////////////////

//���� : ä�� ���ڿ� ���� �ڵ�
//��� : Ŭ���̾�Ʈ <-> ����
//�ڵ� : smTRANSCODE_CHATMESSAGE
struct TRANS_CHATBUFF {
	int		size;
	int		code;
	DWORD	dwIP;
};

struct CHATBUFF {
	DWORD	dwIP;
	char szMessage[256];
};

struct TRANS_CHATMESSAGE {
	int		size;
	int		code;
	DWORD	dwIP;
	DWORD	dwObjectSerial;	//��ü�� ������

	char szMessage[256];
};

struct smPLAYDATA_GROUP {
	int		size,code;
	int		PlayDataCnt;
	DWORD	dwSendCode;
};


#define smPLAYBUFF_FRAME_NONE			0xFFFF0000

#define	smPLAYBUFF_ITEM_BODY			0
#define	smPLAYBUFF_ITEM_LEFT			1
#define	smPLAYBUFF_ITEM_RIGHT			2
#define	smPLAYBUFF_ITEM_SKIL			3

struct smPLAYBUFF {
	int		x,y,z;
	DWORD	frame;
	short	angle[4];

	short	wStickItems[4];				//���� ������ ��ȣ
	DWORD	dwTargetSerial;				//��ǥ ������Ʈ �ڵ�
};


//������ ���� / ���� ���� / ��ǥ������Ʈ

struct smPLAYDATA {
	int	size;
	int code;

	DWORD	dwObjectSerial;	//��ü�� ������

	int x,y,z;
	short angle[4];		//012->xyz 3->action code

	DWORD	frame;
	DWORD	dwAutoCharCode;		//���� ĳ���� �ڵ�

};

struct smPLAYDATA2 {
	int	size;
	int code;

	DWORD	dwObjectSerial;	//��ü�� ������

	short	BuffCount[2];			//PlayBuff Time Posi
	smPLAYBUFF	PlayBuff[2];		//0-Begin 1-last
	DWORD	dwTarget;				//��ǥ ���� ���� ����

};

struct smPLAYDATA3 {
	int	size;
	int code;

	DWORD	dwObjectSerial;	//��ü�� ������

	short	BuffCount[3];			//PlayBuff Time Posi
	smPLAYBUFF	PlayBuff[3];		//0-Begin 1-last 2-change
	DWORD	dwTarget;				//��ǥ ���� ���� ����
};


//������ ���� �� ������ ��ü
struct smTRANS_PLAYBUFF {
	int		BuffCount;
	smPLAYBUFF	PlayBuff;
};

#define TRANS_PLAYDATA_EVENT_LEVEL		0
#define TRANS_PLAYDATA_EVENT_POTION		1
#define TRANS_PLAYDATA_EVENT_SKIL		2
#define TRANS_PLAYDATA_EVENT_EXT		3

//������ ���� �� ����
struct smTRANS_PLAYDATA
{
	int	size;
	int code;

	DWORD	dwObjectSerial;		//��ü�� ������
	DWORD	dwTarget;			//��ǥ ���� ���� ����

	int		PlayBuffCnt;		//�ڿ� ����� smTRANS_PLAYBUFF ����
	int		StartPosi;			//PlayBuff  ���� ��ġ

	short	Hp[2];				//ü�°� 
	DWORD	dwAutoCharCode;		//���� ĳ���� �ڵ�
	short	wArea[2];			//���� �ڵ� //int		Etc2;
	BYTE	bUpdateInfo[4];		//���� ���� ī����
	BYTE	bEventInfo[4];		//��� �̺�Ʈ ���� ( 0-Level 1-Potion��� 2-��ų��� 3-���� )
};

//�������� ����Ʈ ����
struct smEFFECT_ITEM {
	int size,code;

	DWORD	dwItemCode;

	int		ColorBlink;
	short	sColors[4];
	DWORD	DispEffect;
	DWORD	BlinkScale;
};

//ĳ���� ���ۿ䱸
struct smTRANS_CHAR_COMMAND
{
	int	size;
	int code;

	int LParam;
	int WParam;
	int SParam;

	char	szName[32];
};

//ĳ���� ���ۿ䱸
struct smTRANS_CHAR_COMMAND2
{
	int	size;
	int code;

	int LParam;
	int WParam;
	int SParam;

	char	szId[32];
	char	szName[32];
};

//���� : �ٸ� �÷��̾���� ������ ���� ������ �ְ� ����
//��� : Ŭ���̾�Ʈ <-> Ŭ���̾�Ʈ
//�ڵ� : smTRANSCODE_PLAYERINFO
struct smTRNAS_PLAYERINFO
{
	int	size;
	int code;

	smCHAR_INFO	smCharInfo;

	DWORD	dwObjectSerial;

	int	x,y,z;				//���� ��ǥ
	int ax,ay,az;			//���� ����
	int state;				//�Ӽ�
};

struct smTRNAS_PLAYERINFO_QUICK
{
	int	size;
	int code;

	DWORD	dwObjectSerial;

	int	x,y,z;				//���� ��ǥ
	int ax,ay,az;			//���� ����
	int state;				//�Ӽ�

	smCHAR_INFO	smCharInfo;
};

//������ �ڸ�� ���ۿ� ����
struct smTRANS_COMMAND_DWORD
{
	int	size;
	int code;

	DWORD LParam;
	DWORD WParam;
	DWORD SParam;
	DWORD EParam;
};

struct smTRANS_COMMAND_EX
{
	int	size;
	int code;

	int LParam;
	int WParam;
	int SParam;
	int EParam;

	int LxParam;
	int WxParam;
	int SxParam;
	int ExParam;

};

struct smTRANS_COMMAND_BUFF {
	smTRANS_COMMAND		smTransCommand;
	char	*Buff[256];
};

struct smTRANS_COMMAND_SOD {
	smTRANS_COMMAND		smTransCommand;
	int		MyScore;
	int		MyTeam;
	int		TeamScore[4];
	int		Temp[4];
};

struct smTRANS_COMMAND_POLLING {
	smTRANS_COMMAND		smTransCommand;
	BYTE	bPolling[10];
};

//�Ӽ� ȣ��
struct smTRANS_RESISTANCE {
	int	size;
	int code;
	int	Param;
	short Resistance[8];
	int	Absorb;
};

//����
struct smTRANS_BLESSCASTLE {
	smTRANS_COMMAND		smTransCommand;
	int				TaxRate;
	short			Tower[12];		//Ÿ������	
	unsigned char	MercenaryNum[4];	//int		MercenaryType;	//�뺴����
	int				ClanSkill;		//Ŭ����ų
	int				Price;
	DWORD			dwMasterClan;	//����Ŭ��
	int				Temp[4];
};

//���ݼ��� ��10
struct	rsUSER_LIST_TOP10 {
	//SYSTEM HEADER
	int		size,code;

	int		tCode;
	DWORD	dwObjectSerial;
	DWORD	dwNameCode;
	int		x,y,z;

	//BODY

	DWORD	dwCharCode;
	DWORD	dwUpdateTime;
	DWORD	dwTotalDamage;
	char	bCrystalTowerCount[4];
	DWORD	dwTemp[2];

	DWORD	dwUserCode[10];
	int		Damage[10];
	int		Counter[10];
};

/////////////// �������� �׷����� ��� ���� /////////////////////

/*
/////������ ������ ������ ����ü 
struct sCRAFTITEM_SERVER{
	int size, code;
	int DocIndex;
	int Result;
	sITEM DesCraftItem;
	DWORD SheltomCode[12];
	DWORD Head[12];
	DWORD CheckSum[12];
	int Index;
	int Money;
};
*/


//������ �׷����� ����
#define	TRANS_GROUPITEM_MAX		16
#define	TRANS_GROUPITEM_HEADER_SIZE		(14*4)

//������ Ǭ���
struct	TRANS_ITEMINFO_GROUP {
	int	size,code;

	int Index;
	int Result;
	int Money;
	int	ItemCount;

	DWORD	dwTemp[8];

	sITEMINFO	sItemInfo[TRANS_GROUPITEM_MAX];
};

//��������
struct	TRANS_ITEMINFO_GROUP2 {
	int	size,code;

	int Index;
	int Result;
	int Money;
	int	ItemCount;

	DWORD	dwTemp[8];

	char	szBuff[ sizeof(sITEMINFO) * TRANS_GROUPITEM_MAX ];
};

//���� ������ ����
struct TRANS_ITEMINFO {
	//------------------------------------------------------------------< _ignore_bug_fix_0x00000000_ //�ؿ�
	/*
	TRANS_ITEMINFO()  // ������
	{
		size = 0,code = 0;x=0,y=0,z=0,
		ZeroMemory(&Item,sizeof(Item));
		for(int i=0;i<4;i++)
		{
			dwSeCode[i] = 0;
		}
		
	};
	*/
	//------------------------------------------------------------------  _ignore_bug_fix_0x00000000_ >
	int	size,code;

	//char	szItemName[32];
	//DWORD	ItemCode;

	sITEMINFO	Item;

	int x,y,z;

	DWORD	dwSeCode[4];		//���� �ڵ�
};

struct	TRANS_ITEM_CODE {
	int size,code;

	DWORD	dwFlag;

	DWORD	dwItemCode;
	DWORD	dwHead;
	DWORD	dwChkSum;
	DWORD	dwTime;
};

struct TRANS_ITEM {
	int			state;

	int			type;
	int			ItemCode;

	int			x,y,z;
	char		szName[64];
	void		*lpStgArea;
	//char szModelName[64];
};

struct _TRANS_THROW_ITEM {
	DWORD	dwCode;
	DWORD	dwHead;
	DWORD	dwChkSum;
};

#define	THROW_ITEMS_MAX				32
//����ü ��� ������
#define	TRANS_THROW_ITEMS_DEFSIZE	(sizeof(int)*4)

struct TRANS_THROW_ITEMS {
	int		size,code;
	int		ThrowItemCount;
	int		Temp;

	_TRANS_THROW_ITEM	ThrowItems[THROW_ITEMS_MAX];
};

//�ŷ��� ������ ����
struct _TRANS_RECORD_TRADE_ITEM {
	DWORD	dwCode;
	DWORD	dwHead;
	DWORD	dwChkSum;
	int		State;
};

//sinTrade.h ���� ������
#define _MAX_TRADE_ITEM			20

struct TRANS_TRADE_ITEMKEY	{
	int size,code;
	DWORD	dwSender;
	DWORD	dwRecver;
	DWORD	dwSum;

	DWORD	dwItemCode[_MAX_TRADE_ITEM];
	DWORD	dwItemKey[_MAX_TRADE_ITEM];
	DWORD	dwItemSum[_MAX_TRADE_ITEM];

	int		Money;
	int		Coin;
};

#define	TRADE_RECV_ITEMS_MAX				32

//����ü ��� ������
#define	TRANS_RECORD_TRADE_DEFSIZE		(sizeof(int)*16)

struct TRANS_RECORD_TRADE_ITEMS {
	int		size,code;
	int		Money;
	int		Coin;
	int		ThrowItemCount;
	char	szTradeUserName[32];			//���� ĳ����
	int		Temp[4];

	_TRANS_RECORD_TRADE_ITEM	Items[TRADE_RECV_ITEMS_MAX];
};

#define TRANS_ITEM_MAX			16

//������ ����
struct smTRANS_ITEMS
{
	int size,code;

	int state;
	int Count;

	TRANS_ITEM	Items[ TRANS_ITEM_MAX ];
};

//������ ����
struct smTRANS_ACITON_ITEM
{
	int size,code;

	int state;

	int			x,y,z;
	void		*lpStgArea;
};

//���� ������ ����
struct smTRANS_SELL_ITEM {
	int size,code;
	int	ItemKind;					//���� / ��� /��Ÿ ����� ����
	int	ItemCounter;				//�Ǹ� �������� ��
	int	Ext[4];						//����
};

//������ ��ȯ â
#define TRANS_TRADE_BUFF_SIZE		4000
struct TRANS_TRADEITEMS {
	int size,code;
	DWORD	dwSender;
	DWORD	dwRecver;
	DWORD	dwSum;
	DWORD	Temp[4];

	BYTE	TradeBuff[TRANS_TRADE_BUFF_SIZE];
};

#define	SOD_RESULT_MAX	32

struct	_SOD_RESULT {
	char		szName[32];
	rsPLAYINFO	*lpPlayInfo;
	int			JobCode;
	int			Class;
	int			Score;
	int			Monster;
	DWORD		dwTemp[4];
};

//SoD ���� ����
struct	TRANS_SOD_RESULT {
	int	size,code;

	DWORD	dwGameCode;
	DWORD	dwGameTime;
	DWORD	dwDisplayMode;
	short	TeamRound[2];

	DWORD	dwTemp[15];

	int		ResultCount;

	_SOD_RESULT	Result[SOD_RESULT_MAX];
};

//////// ������ Ŭ���̾�Ʈ�� ��,����ġ ��ġ ��ȸ //////////
struct	sSERVER_MONEY {
	int	InputMoney;
	int	Counter;
	int	TotalX;
	int	TotalY;
	int	TotalZ;
};

struct sSERVER_EXP {
	int	Counter;
	int	InputExp;
	int	Total1;
	int	Total2;
};

struct TRANS_TOTAL_EXPMONEY {
	int	size,code;

	sSERVER_MONEY	sServerMoney;
	sSERVER_EXP		sServerExp;
};

//����Ÿ ������ Ŭ���̾�Ʈ ���� ����
struct	TRANS_SEND_DATASERVER {
	int size,code;
	char	szBuff[1024];
};

//������ ���Կ� ����ü
struct	TRANS_BUY_SHOPITEM {
	int	size,code;

	sITEM	sItem;			//�����ϴ� ������
	int		ItemCount;		//�����ϴ� ������ ����
};

//�ŷ��� ������ ������ ���� �������� �κ��� ���� �ϴ��� Ȯ��
struct	_TRANS_USINGITEM {
	DWORD	dwCode;
	short	Performance[8];
};

struct	TRANS_USINGITEM {
	int	size,code;
	int	ItemListCount;
	_TRANS_USINGITEM	ItemList[16];
};

//Ʈ���̵� ������ Ű ����ü

//ũ�������� ��ġ ����ü
#define	FIND_CRACK_LIST_MAX		64

struct	_CRACK_NAME {
	char	szName1[12];
	char	szName2[12];
	char	szClass[12];
};

struct	TRANS_FIND_CRACKER {
	int		size,code;

	int		FindCounter;

	_CRACK_NAME	CrackName[FIND_CRACK_LIST_MAX];

	char	szTemp[128];
};

#define	PARTY_STATE_NONE		0
#define	PARTY_STATE_MASTER		1
#define	PARTY_STATE_MEMBER		2


struct _PARTY_USER {
	char	szName[32];				//�̸�
	char	szModelName[64];		//�� �̸�
	char	szModelName2[64];		//�� �̸�2

	DWORD	dwObjectSerial;			//��ü ������ȣ
	DWORD	Level;
	DWORD	JobCode;
	short	Life[2];				//�����
	short	Mana[2];
	int		x,z;
};

struct TRANS_PARTY_USERS {
	int		size,code;

	DWORD	dwObjectSerial;
	DWORD	dwTemp[8];

	int		PartyUserCount;

	_PARTY_USER		PartyUser[PARTY_PLAYER_MAX];
};

struct _PARTY_PLAYINFO {
	DWORD	dwObjectSerial;			//��ü ������ȣ
	DWORD	Level;					//����
	short	Life[2];				//�����
	short	Mana[2];
	int		x,z;
};

struct	TRANS_PARTY_PLAYINFO {
	int		size,code;

	DWORD	dwObjectSerial;
	DWORD	dwTemp;

	int		PartyUserCount;

	_PARTY_PLAYINFO	PlayInfo[PARTY_PLAYER_MAX];
};

struct	TRANS_PARTY_SKILL {
	int		size,code;
	DWORD	dwSkillCode;
	int		Point;
	int		wParam;
	int		lParam;
	int		sParam;
	int		eParam;
	int		PartyCount;
	DWORD	dwPartyUser[8];
};

struct TRANS_CLAN_COMMAND_USER {
	int size,code;
	int	Param[4];
	int	UserCount;

	char	szUserBuff[2048];
};

#define	CLAN_USER_MAX	64

struct _CLAN_USER_INFO {
	char		szName[32];
	DWORD		dwSpeedSum;
	DWORD		dwObjectSerial;
	rsPLAYINFO	*lpPlayInfo;
};

struct	rsCLAN_INFOMATION {
	DWORD	dwClanCode;
	DWORD	dwTemp[32];

	_CLAN_USER_INFO	ClanUserInfo[CLAN_USER_MAX];
};

//â�� ���� ���� 
struct	TRANS_WAREHOUSE {
	int size,code;

	DWORD	dwChkSum;
	WORD	wVersion[2];
	int		WareHouseMoney;
	int		UserMoney;
	DWORD	dwTemp[5];

	int		DataSize;
	BYTE	Data[ sizeof( sITEM )*100+256 ];
};

#define	TRANS_SERVER_MAX		8

//���� ����
struct _TRANS_SERVER_INFO {
	char	szServerName[32];
	char	szServerIp1[20];
	char	szServerIp2[20];
	char	szServerIp3[20];

	DWORD	dwServerPort1;
	DWORD	dwServerPort2;
	DWORD	dwServerPort3;

	DWORD	ConnState;
};

//���� ���� ��� 
struct TRANS_SERVER_LIST {
	int size,code;

	char	szServerName[16];

	DWORD	dwServerTime_t;
	int		ClanTicket;
	int	dwTemp[1];
	int	ClanServerCount;
	int	ServerCount;
	_TRANS_SERVER_INFO ServerInfo[TRANS_SERVER_MAX];
};

#define	SERVER_MAP_MAX		64

struct	TRANS_SEVER_MAP {
	int size,code;

	DWORD	dwTemp[30];

	int		IP_Count;
	DWORD	dwIP[SERVER_MAP_MAX];
};

struct	TT_DATA_SERVER_INFO {
	char	szServerID[16];
	char	szServerPath[128];
};


#define	SYSTEM_IP_MAX			32
#define	LOGIN_IP_MAX			16
#define	DISABLE_IP_MAX			500
#define	ENABLE_IP_MAX			100
#define	ADMIN_IP_MAX			16
#define CLAN_SERVER_MAX			3
#define ENABLE_MODULE_CHECK_MAX	64	
#define	TT_DATA_SERVER_MAX		16

//���� ���� ����
struct rsSERVER_CONFIG	{

	DWORD	ServerCode;
	DWORD	AutoPlayer;
	DWORD	DisableParty;		//�������
	DWORD	EnablePlayerKill;	//PK�㰡
	DWORD	ConnectUserMax;		//�ִ� ���� �㰡 �ο�

	int		TestSeverMode;		//�׽�Ʈ ������

	int		UseRecorMemory;		//���嵥��Ÿ �޸� ���ۻ��
	int		PotionMonitor;		//���� ����

	int		ClientCheckSumCount;	//Ŭ���̾�Ʈ üũ�� ī����
	int		ClientCheckOut;			//���� Ŭ���̾�Ʈ ���� ����
	DWORD	dwClientCheckSum[32];	//���� ������ üũ�� ���

	int		SafeClientCodeCount;	//Ŭ���̾�Ʈ�� ���ýũ �ڵ�ī��Ʈ
	int		ClientCodeCheckOut;		//��ǹ��� Ŭ���̾�Ʈ ���� ����
	DWORD	dwSafeClientCode[32];	//Ŭ���̾�Ʈ�� ���ýũ �ڵ�

	int		LimitVersion;			//���� ������ ���� ����
	int		LimitVersionMax;		//���� ������ �ֻ� ����
	char	szAdminPassword[3][32];	//������ ���� �ο� �н�����
	char	szAdminName[32];		//������ �̸�

	char	szAdminIP[16];			//������ IP
	char	szAdminLogPassword[32];	//������ Password

	char	szServerName[64];		//���� �̸�
	int		Enable_PK;				//PK�㰡
	int		CheckProtect;			//nProtect ���÷� �˻�����
	DWORD	RemoveNProtectTime;		//�����߿� nProtect���� �ð�

	char	szDebugIP[10][32];		//����� IP
	int		DebugIP_Count;			//����� IP ī����

	char	szDebugID[32][32];		//������ ID
	int		DebugID_Count;			//������ ID ī����

	int		BlockLevel;				//ID �ӽ� ����� ����
	int		BlockLevel_LogOut;		//�α� �ƿ�ÿ��� ����( �߱��� �䱸 )
	int		ItemCodeOut;			//������ �ڵ� ������ ����

	int		ClientFuncCheckMode;	//Ŭ���̾�Ʈ ��� �˻� ���
	int		Enable_PartyItem;		//���ᰡ ȹ���� ������ ���

	
	int		Event_Mandoo;			//����(����) �̺�Ʈ
	int		Event_HardCore;			//�ϵ��ھ� �̺�Ʈ
	int		Event_DamageOff;		//���ݷ� Ȱ�� �̺�Ʈ
//�ؿܹ�����/////////////////////////////////////////////////////////////////////////////
	//�ؿ�
//#ifdef _LANGUAGE_PHILIPIN
	int		Event_DamageOff1;		//���� ���ݷ� ���� �̺�Ʈ1 (�ʸ��ɿ�)
	int		Event_DamageOff2;		//���� ���ݷ� ���� �̺�Ʈ2 (�ʸ��ɿ�)
//#endif

	int		Event_DefenseOff;		//���� ���� ��ȭ �̺�Ʈ
/////////////////////////////////////////////////////////////////////////////////////////
	int		Event_Child;			//��� �̺�Ʈ
	int		Event_Cristal;			//ũ����Ż �̺�Ʈ ( Ư�� �ð��뿡 ũ����Ż ���� ��ȭ )
	int		Event_Potion;			//���� �̺�Ʈ ( ���� ���ÿ� �������� ���� ���� )
	int		Event_StarPointTicket;	//������Ʈ �̺�Ʈ ( ����ǰ�� �߻� )
	int		Event_ValentineDay;		//�߷�Ÿ�� ���� �̺�Ʈ
	int		Event_WhiteDay;			//ȭ��Ʈ ���� �̺�Ʈ
	int		Event_MorifSheltom;		//������ ���� Ż�� �̺�Ʈ
	int		Event_MollyWolf;		//����ʹ��� �̺�Ʈ
	int		Event_Ghost;			//���Ʈ �̺�Ʈ
	int		Event_SantaGoblin;		//��Ÿ���� �̺�Ʈ
	int		Event_WorldCup;			//������ �̺�Ʈ
	int		Event_ExpUp;			//����ġ�� �̺�Ʈ
	int     Event_FastExpUp1To2;    // Panzer - speed up the exp for player while his level is >= 1 and <= 20 
	int		Event_CoupleRing;		// pluto Ŀ�ø� �̺�Ʈ
	int		Event_Watermelon;		// ����� - ���� �̺�Ʈ
	int		Event_Star;				// ����� - ���ϴ��� �ҿ��̺�Ʈ
	int		Event_Valentine;		// �庰 - �߷�Ÿ�� �̺�Ʈ
	int		Event_PristonAlphabet;	// ����� - ���ĺ� ���� �̺�Ʈ
	int		Event_WhiteContrast;	// �庰 - ȭ��Ʈ��Ʈ��Ʈ �̺�Ʈ
	int		Event_Candydays;		// �庰 - ĵ������
	int		Event_BlackSmith;		// �庰 - ���������� ȥ
	int		Event_MagicalGreen;	// �庰 - �����ñ׸�
	int		Event_TearOfKara;		// �庰 - ī���� ����
	int		Event_WarerMelonMonster;		// �庰 - ���ڸ��� �̺�Ʈ
	int		Event_Chichen_Day;		// ����� - ���� �̺�Ʈ(�Ʊ� ���� ���Ͱ� ��Ÿ����)
	int		Event_MovieBloodyBattle; // �庰 - ��ȭ ����


#ifdef _LANGUAGE_ARGENTINA
	int		Event_ExpUp_latin;		//����ġ�� �̺�Ʈ(�Ƹ���Ƽ��) //�ؿ� Kyle
#endif

	int		Event_SummerChicken;	//����� �̺�Ʈ
	int		Event_GiftBox;			// pluto �������� �̺�Ʈ

	int		BlessCastleMode;			//�����ĳ�� ����
	short	sBlessCastle_StartTime[2];	//�����ĳ�� ���� �ð� ���� [0]���� [1]�ð�

	int		Record_JobChanger;		//�������� ���
	int		ForeverBlockMode;		//������ ���

	int		Permit_Exp;				//������ ����ġ
	int		Permit_Money;			//������ ��

	int		ClientProcessTimeChk;	//Ŭ���̾�Ʈ ���μ��� �ð����� �ִ�ġ
	int		ClientProcessTimeChkOut;//Ŭ���̾�Ʈ ���μ��� �ð� �ʰ��� ��������
	int		Dis_PotionError;		//���� �������� ���� ����

	int		ExpFixMode;				//���� ����ġ ����

	char	szBlockUserPath[64];	//���� ��� ����Ÿ ���

	int		DispBillingTime;		//��� �����ð� ǥ��

	int		Disable_LogIn;			//�α��� ����
	int		Disable_LogQue;			//�α�ť ó�� ����
	int		Disable_CheckLogOut;	//�α׾ƿ� ť�˻� ����

	int		Disable_LogIn2;			//�α��� ����2��° �ű� �α��θ� ����
	int		Disable_PCBang;			//PC�� ���� ���

	int		ExpGameTimeMode;		//����ġ �ð��� �й�(��Ʈ��)

	int		ServerInventoryMode;	//������ �����κ��丮�� ������ ����

	DWORD	DisableIP[DISABLE_IP_MAX][2];		//���� ���� IP
	int		DisableIP_Count;					//���� ���� IP ���� ��

	DWORD	EnableIP[ENABLE_IP_MAX][2];			//���� ���� IP�� �㰡�Ǵ� IP
	int		EnableIP_Count;						//���� ���� IP ���� ��

	DWORD	AdminIP[ADMIN_IP_MAX][2];			//������ IP ���
	int		AdminIP_Count;						//������ IP ���� ��

	DWORD	SystemIP[SYSTEM_IP_MAX][2];			//�ý��� IP ���
	int		SystemIP_Count;						//�ý��� IP ���� ��

	DWORD	LoginServerIP[LOGIN_IP_MAX][2];		//�α伭�� IP ���
	int		LoginServerIP_Count;				//�α伭�� IP ���� ��

	char	szBackupPath[80];					//*��������̸�

	char	szClanServerIP[CLAN_SERVER_MAX][20];	//Ŭ�� ����	�ּ�
	int		ClanServerPort[CLAN_SERVER_MAX];		//Ŭ�� ���� ��Ʈ
	int		ClanServerCount;


	DWORD	Enable_ClientModule[ENABLE_MODULE_CHECK_MAX];	//Ŭ���̾�Ʈ ��ƫýũ��
	int		Enable_ClientModuleCount;						//Ŭ���̾�Ʈ ��� ýũ�� ī����

	int		Disable_DecSkillPoint;							//��ų ����Ʈ ���� ýũ ó��

	int		Disable_Quest;									//���� ����Ʈ ����

	int		CrazyPacketMode;							//���� ��Ŷ ���

	char	szNoticeURL[256];							//Notice URL ����


	TRANS_SEVER_MAP	AreaServer;							//������ ���� ����

	char	szLinkServerIP[32];							//��ũ�� ���� IP

	int		RecordAdminCommand;							//��� ��� ���

	int		FreeLevel;									//���� ����
	char	szFreeGpCode[32];							//������ GPCODE
	int		PresentLevel;								//�����ִ� ����
	int		Event_ComicBirthDay;						//���ϼ��� ��ū��
	int		Event_Puzzle;								//���� ���߱� �̺�Ʈ
	int		Event_BabelHorn;							//�ٺ����̺�Ʈ
	int		Event_NineFox;								//����ȣ�̺�Ʈ
	int		EVENT_Christmas;							//ũ�������� �̺�Ʈ
	int		EventPlay_BitMask;							//�̺�Ʈ �÷��� ��Ʈ����ũ
	int		Event_PCBangPet;							//PC�� �� �̺�Ʈ

	int		StartUser_PresentItem;						//������������ ���� �ֱ�

	int		GameGuard_Auth;								//���Ӱ��� ���� ����

#ifdef _XTRAP_GUARD_4_
	int		XTRAP_Timer;								//XTRAP �˻��ֱ�		XTRAP_GUARD
	int		XTRAP_MaxDealy;								//XTRAP �˻���ü ����ġ  XTRAP_GUARD
#endif

	//��ʸ�Ʈ���� ���� ����
	int		TT_DataServer_Count;									//��ʸ�Ʈ���� ����Ÿ ��� ����
	TT_DATA_SERVER_INFO		TT_DataServer[TT_DATA_SERVER_MAX];		//��ʸ�Ʈ���� ����Ÿ ����

	int		FixedStartField;							//���� �ʵ� ���� ������Ű��
	int		DisableThrowItem;							//������ ���� ������

	int		CheckPlayField;								//Ŭ���̾�Ʈ �� ������  �ʵ� ���� �� �˻�

	int		Enable_PKField;								// PK��� �ʵ� ��ȣ

	int		Enable_PKField_All;							// pluto PK��� �ʵ� ����

	//��Ų�� ����
	int		CheckModelFile;								//������ Ȯ�� �ɼ�


	#ifdef LOG_THE_PK
		//PK_LOG���� //�ؿ�
		bool	bPK_LOG;
		rsSERVER_CONFIG()
		{
			bPK_LOG = false;
		};
	#endif

};

//���� �⵿ 
extern int	ServerMode;

//������ ����/���� �Լ�
//�� Ŭ���̾�Ʈ ����
int Serv_ConnectPlayer( smWINSOCK *lpsmSock );
//Ŭ���̾�Ʈ ����
int Serv_DisconnectPlayer( smWINSOCK *lpsmSock );
//�ٸ� ĳ���Ϳ��� ��ġ ��ġ���� Ȯ��
smCHAR *Serv_CheckOtherPlayPosi( int x, int y, int z );

//����ġ�� ���� ��� �Ѱ�ġ ( ���Ѱ踦 �Ѿ�� ������ �ȵȴ� )
#define	TOTAL_CHECK_MONEY_MAX		52000
#define	TOTAL_CHECK_EXP_MAX			100000




/////////////////// Ŭ���̾�Ʈ ��� ��ġ ����ü ////////////////////////
#define	dwFUNC_VALUE_MAX	64

struct	sFUNC_VALUE {
	DWORD	dwFunc;
	DWORD	dwLen;
	DWORD	dwChkSum;
};

struct	TRANS_CLIENT_FUNPOS {
	int	size,code;

	int	ClientVersion;

	DWORD	dwFunc_CheckMemSum;
	int		FuncCount;

	sFUNC_VALUE	dwFuncValue[dwFUNC_VALUE_MAX];
};

struct	TRANS_FUNC_MEMORY	{
	int		size,code;
	int		Param[8];
	char	szData[6000];
};

struct	sCHK_POTION_COUNT  {
	int	Ps101;
	int	Ps102;
	int	Ps103;
	int	Ps104;

	int	Pm101;
	int	Pm102;
	int	Pm103;
	int	Pm104;

	int	Pl101;
	int	Pl102;
	int	Pl103;
	int	Pl104;
};

//SOD ���� ���
struct	sSOD_SCORE {
	int		Flag;
	DWORD	dwDispTime;
	int		Score;
	int		MyTeam;
	int		TeamScore[4];
	int		Round;
	DWORD	dwPlayTime;

	/////// SOD ���� �̺�Ʈ /////////

	DWORD	dwSoD_NextStageTime;		//���� �������� �Ѿ�� �ð�
	int		SodNextStageNum;			//�Ѿ �������� ��ȣ
	int		NextRound;
	DWORD	dwSoD_CloseStageTime;		//ȭ���� �ݴ� ����Ʈ ����ð�

	///////// SOD ���� ���� //////////////
	int		EffectStartCount;			//����Ʈ ���۱����� ī����
	int		EffectCode;					//����Ʈ �ڵ�

	//////// SOD ���� ȿ�� /////////////
	int		ScoreEffectCount;				//���� ȿ��

};

#define MAX_JOBNUM					8 //Num of Jobs

#define	JOBCODE_FIGHTER				1
#define	JOBCODE_MECHANICIAN			2
#define	JOBCODE_ARCHER				3
#define	JOBCODE_PIKEMAN				4
#define	JOBCODE_ATALANTA			5
#define	JOBCODE_KNIGHT				6
#define	JOBCODE_MAGICIAN			7
#define	JOBCODE_PRIESTESS			8

#define	CHAR_EVENT_STARPOINT		300

#define	ITEM_UNIQUE_CODE_NORMAL		1
#define	ITEM_UNIQUE_CODE_QUEST		2


#define BIMASK_PRORATE_USER			1
#define BIMASK_FREE_USER			2
#define BIMASK_VIP_USER				4
#define BIMASK_BIRTHDAY_USER		8
#define BIMASK_TIMELEVEL_USER		16
#define BIMASK_ADULT_USER			32


#define	MONSTER_CLASS_NORMAL		0
#define	MONSTER_CLASS_BOSS			1
#define	MONSTER_CLASS_HAMMER		200
#define	MONSTER_CLASS_GHOST			300

#endif




















