
#include "systems/localization/config/language.h"

#ifdef	_LANGUAGE_KOREAN

char *parkTextVO = "���� ä�� �ɼ�";
char *parkTextMIC = "����ũ";
char *parkTextMicON = "On";
char *parkTextMicOFF = "Off";

char *parkTextVol = "���� : ";
char *parkTextVolLow = "��";
char *parkTextVolHigh = "��";

char *parkTextVolErr1 = "������ ������ ����ũ";
char *parkTextVolErr2 = "���� ���� ���ּ���.";


#endif



#ifdef	_LANGUAGE_CHINESE
#include "../chinese/C_VolText.h"
#endif

#ifdef	_LANGUAGE_JAPANESE
#include "systems/localization/locales/Japanese/J_VolText.h"
#endif

#ifdef	_LANGUAGE_TAIWAN
#include "../Taiwan/T_VolText.h"
#endif

#ifdef	_LANGUAGE_ENGLISH
#include "systems/localization/locales/English/E_VolText.h"
#endif

#ifdef	_LANGUAGE_THAI
#include "../Thai/Th_VolText.h"
#endif

#ifdef	_LANGUAGE_VEITNAM
#include "systems/localization/locales/veitnam/v_VolText.h"
#endif

#ifdef	_LANGUAGE_BRAZIL
#include "systems/localization/locales/Brazil/b_VolText.h"
#endif

#ifdef	_LANGUAGE_ARGENTINA
#include "../ARGENTINA/a_VolText.h"
#endif



