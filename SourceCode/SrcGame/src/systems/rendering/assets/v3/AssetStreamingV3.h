#pragma once

struct smTEXTUREHANDLE;

enum AssetLoadV3TextureReadiness
{
    ASSET_LOAD_V3_TEXTURE_RETRYABLE = 0,
    ASSET_LOAD_V3_TEXTURE_PENDING = 1,
    ASSET_LOAD_V3_TEXTURE_RESIDENT = 2,
    ASSET_LOAD_V3_TEXTURE_FAILED_PERMANENT = 3
};

// Asset Streaming V3 public bridge. CPU preparation runs on the worker pool;
// GPU publication is pumped from the render thread with a bounded budget.
int  AssetLoadV3RequestTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap = 0, int priority = 1);
bool AssetLoadV3IsTexturePending(smTEXTUREHANDLE *lpTexHandle);
int  AssetLoadV3GetTextureReadiness(smTEXTUREHANDLE *lpTexHandle);
void AssetLoadV3PumpTextureUploads();
void AssetLoadV3ResetAsyncState();
// The initial world load may use a wider render-thread upload budget.  This
// never moves D3D work to workers; it only avoids visible one-texture-at-a-time
// bootstrap while gameplay keeps the conservative frame budget.
void AssetLoadV3SetLoadingBoost(bool enabled);
// Opening/login UI is a correctness boundary: these source textures must be
// resident before the first interactive frame. This owner-thread helper bypasses
// cooked DDS and loads only the small opening UI set synchronously.
int  AssetLoadV3LoadOpeningUiNow();
// True while login/character-selection UI textures still have V3 work pending.
// Used as a transition barrier so the selection screen is never exposed half-resident.
bool AssetLoadV3HasPendingInteractiveLoginTextures();
