#pragma once

struct smTEXTUREHANDLE;

// Asset Streaming V3 public bridge. CPU preparation runs on the worker pool;
// GPU publication is pumped from the render thread with a bounded budget.
int  AssetLoadV3RequestTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap = 0, int priority = 1);
bool AssetLoadV3IsTexturePending(smTEXTUREHANDLE *lpTexHandle);
void AssetLoadV3PumpTextureUploads();
void AssetLoadV3ResetAsyncState();
// The initial world load may use a wider render-thread upload budget.  This
// never moves D3D work to workers; it only avoids visible one-texture-at-a-time
// bootstrap while gameplay keeps the conservative frame budget.
void AssetLoadV3SetLoadingBoost(bool enabled);
