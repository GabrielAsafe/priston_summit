#pragma once

#include <mutex>

class smCHAR;
class scITEM;

// Shared guard for the legacy smPATTERN arrays. Expensive model parsing never
// runs while this mutex is held; only cache lookup/publication/refcount does.
std::recursive_mutex &SummitPatternCacheMutexV3();

int SummitRequestCharacterModelV3(smCHAR *character, const char *primaryName, const char *secondaryName);
int SummitRequestItemModelV3(scITEM *item, const char *modelName);
int SummitRequestToolModelV3(smCHAR *character, unsigned long itemCode, int toolPosition, const char *modelName);
// Fast path used by legacy callers that can consume an already-resident model
// immediately. It never performs disk I/O or parsing. The returned cache entry
// owns one reference and must eventually be released through smDPAT_Delete /
// AddRemoveDynPattern just like smPATTERN::LoadCharactor().
struct smDPAT;
smDPAT *SummitAcquireCachedModelV3(const char *modelName);
void SummitCancelCharacterModelV3(smCHAR *character);
void SummitCancelItemModelV3(scITEM *item);
void SummitCancelToolModelV3(smCHAR *character, int toolPosition);
void SummitPumpModelStreamingV3();
void SummitResetModelStreamingV3();
void SummitShutdownModelStreamingV3();
