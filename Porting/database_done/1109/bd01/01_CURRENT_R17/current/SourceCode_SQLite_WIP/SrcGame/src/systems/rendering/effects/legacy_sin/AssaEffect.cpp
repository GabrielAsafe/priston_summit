#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/world/field/field.h"

struct LegacyAssaEffectPolicy
{
	static void InitializeEffects()
	{
	{ FILE *f=fopen("LOGS\\mingw_assa_effect_init.txt","a"); if(f){ fprintf(f,
        "ENTER InitAssaEffect sizeof_class=%u sizeof_array=%u max=%d ptr=%u\n",
        (unsigned)sizeof(cASSAEFFECT),
        (unsigned)sizeof(cAssaEffect),
        (int)cASSAEFFECT_MAX,
        (unsigned)sizeof(void *)); fclose(f);} }

	{ FILE *f=fopen("LOGS\\mingw_assa_effect_init.txt","a"); if(f){ fprintf(f,"before InitAssaRes\n"); fclose(f);} }
	InitAssaRes();
	{ FILE *f=fopen("LOGS\\mingw_assa_effect_init.txt","a"); if(f){ fprintf(f,"after InitAssaRes\n"); fclose(f);} }

	{ FILE *f=fopen("LOGS\\mingw_assa_effect_init.txt","a"); if(f){ fprintf(f,"before LoadAssaRes\n"); fclose(f);} }
	LoadAssaRes();
	{ FILE *f=fopen("LOGS\\mingw_assa_effect_init.txt","a"); if(f){ fprintf(f,"after LoadAssaRes\n"); fclose(f);} }

	memset(cAssaEffect, 0, sizeof(cAssaEffect));
	{ FILE *f=fopen("LOGS\\mingw_assa_effect_init.txt","a"); if(f){ fprintf(f,"after memset cAssaEffect\n"); fclose(f);} }

	{ FILE *f=fopen("LOGS\\mingw_assa_effect_init.txt","a"); if(f){ fprintf(f,"LEAVE InitAssaEffect\n"); fclose(f);} }

	}
	static void BeginEffectScene() {}
	static void EndEffectScene() {}
};

#include "rendering/effects/AssaEffectLegacyCommon.inl"
