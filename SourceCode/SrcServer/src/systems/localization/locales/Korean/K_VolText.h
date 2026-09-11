
#include "systems/localization/config/language.h"

#ifdef	_LANGUAGE_KOREAN

char *parkTextVO = "음성 채팅 옵션";
char *parkTextMIC = "마이크";
char *parkTextMicON = "On";
char *parkTextMicOFF = "Off";

char *parkTextVol = "볼륨 : ";
char *parkTextVolLow = "저";
char *parkTextVolHigh = "고";

char *parkTextVolErr1 = "윈도우 내에서 마이크";
char *parkTextVolErr2 = "볼륨 조절 해주세요.";


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



