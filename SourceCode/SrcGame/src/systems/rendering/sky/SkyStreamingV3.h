#pragma once

struct skyResInfo;

int SummitRequestSkyModelV3(skyResInfo *target, const char *sourceName, bool clippingFlag);
void SummitCancelSkyModelV3(skyResInfo *target);
void SummitPumpSkyStreamingV3();
void SummitResetSkyStreamingV3();
