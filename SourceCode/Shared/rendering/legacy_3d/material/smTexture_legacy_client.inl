#include <string>
#include <set>
#include <cstdlib>
#include <cstdint>
#include <new>
#include <atomic>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include "core/concurrency/JobSystem.h"

#include "rendering/assets/v2/AssetLoadRegistry.h"

#include "systems/rendering/legacy_3d/smd3d.h"
#include <io.h>




int		g_IsReadTextures = 1;

BYTE	g_DibImageDateBuff[(1024 * 768) * 4];
int		g_DibLastReadWidth;
int		g_DibLastReadHeight;
int		g_TgaLastBitCnt;
int		g_TgaLastSwapHeight;

int		new_smCreateTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap = 0);
int     AssetLoadV3RequestTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap, int priority);
void    AssetLoadV3PumpTextureUploads();
void    AssetLoadV3ResetAsyncState();
bool    AssetLoadV3IsTexturePending(smTEXTUREHANDLE *lpTexHandle);

#define ERROR_LIST_MAX		32
#define ERROR_LIST_MASK		(ERROR_LIST_MAX-1)

char szErrorList[ERROR_LIST_MAX][64];
int ErrorListCnt = 0;


int AddErrorList(char *szErrMsg)
{
	return ErrorListCnt;
}

int DisplayError(HDC hdc, int x, int y)
{
	int cnt, mcnt;
	int len;
	int dy;

	dy = y;

	for (cnt = ErrorListCnt - ERROR_LIST_MASK; cnt < ErrorListCnt; cnt++) {
		if (cnt >= 0) {
			mcnt = cnt & ERROR_LIST_MASK;
			len = lstrlen(szErrorList[mcnt]);
			
			dy += 14;
		}
	}

	return TRUE;
}





DWORD	VramTotal = 0;




#define	MAX_MAP_BLINK_COLOR_TIME				8

const char *szBlinkTimeScript[MAX_MAP_BLINK_COLOR_TIME] =
{
	"BLINK_TIME1:",
	"BLINK_TIME2:",
	"BLINK_TIME3:",
	"BLINK_TIME4:",
	"BLINK_TIME5:",
	"BLINK_TIME6:",
	"BLINK_TIME7:",
	"BLINK_TIME8:",
};

const DWORD dwBlinkTimeCode[MAX_MAP_BLINK_COLOR_TIME] =
{
	9, 10, 11, 12, 13, 14, 15, 16,
};





int MipMapModeCreate;						
int TextureSwapMode = TRUE;					
static int TexturHalfSize = FALSE;			




static char MipMapFileListBuff[MIPMAP_FILELIST][16] = { "", "_mm1", "_mm2", "_mm3", "_mm4", };

static int  MipMapCount = 0;
static char MipMapFileBuff[MIPMAP_FILELIST][126] = { 0, };
static int  MipMapCountA = 0;
static char MipMapFileABuff[MIPMAP_FILELIST][126] = { 0, };
static char FileTypeBuff[16] = { 0, };



// High-resolution texture memory policy.
// The client is a 32-bit process. Large uncompressed managed textures can
// exhaust its virtual address space even when the GPU has enough VRAM.
// For large, block-compatible assets we keep D3DPOOL_MANAGED semantics but
// request DXT compression, reducing the persistent managed copy by 4x (DXT5)
// or 8x (DXT1). If conversion is unsupported/fails, loading falls back to the
// legacy D3DFMT_FROM_FILE path.
// Asset Load System V2 keeps all state outside smTEXTUREHANDLE so the legacy
// ABI/disk assumptions remain untouched.  The registry is the authority for
// retry policy, resident-byte accounting, demand loading and LRU eviction.
typedef summit::rendering::assets::v2::AssetLoadRegistry<MAX_TEXTURE> SummitAssetRegistryType;
static SummitAssetRegistryType SummitAssetRegistry;
static std::set<std::string> SummitMissingTexturePaths;

enum SummitTextureAsyncState
{
    SUMMIT_TEX_ASYNC_IDLE = 0,
    SUMMIT_TEX_ASYNC_PREPARING = 1,
    SUMMIT_TEX_ASYNC_READY = 2
};

static std::atomic<unsigned char> SummitTextureAsyncStates[MAX_TEXTURE];
static std::atomic<std::uint32_t> SummitTextureAsyncGenerations[MAX_TEXTURE];
static std::atomic<std::uint32_t> SummitTextureAsyncInFlight(0);
static std::atomic<std::uint64_t> SummitTextureStagingBytes(0);
static std::atomic<bool> SummitTextureLoadingBoost(false);

struct SummitTextureCriticalSectionGuard
{
    explicit SummitTextureCriticalSectionGuard(CRITICAL_SECTION *section) : section_(section)
    {
        EnterCriticalSection(section_);
    }
    ~SummitTextureCriticalSectionGuard()
    {
        LeaveCriticalSection(section_);
    }
    CRITICAL_SECTION *section_;
};

static bool SummitAssetTraceEnabled()
{
    const char *value = getenv("SUMMIT_ASSET_TRACE");
    return value && *value && *value != '0';
}

static bool SummitPrewarmHasHeadroom()
{
    const ULONGLONG budget = SummitAssetRegistry.Policy().residentBudgetBytes;
    if (!budget)
        return true;

    // External/UI textures are not part of the registry accounting, so leave
    // a generous reserve for demand loads instead of filling the full budget.
    const ULONGLONG prewarmCeiling = (budget * 3ULL) / 4ULL;
    return SummitAssetRegistry.ResidentBytes() < prewarmCeiling;
}


static void SummitConfigureAssetLoadPolicy()
{
    using summit::rendering::assets::v2::AssetLoadPolicy;
    AssetLoadPolicy policy = AssetLoadPolicy::Defaults();

    const char *budgetMb = getenv("SUMMIT_TEXTURE_BUDGET_MB");
    if (budgetMb && *budgetMb)
    {
        const unsigned long parsed = strtoul(budgetMb, NULL, 10);
        if (parsed >= 128ul && parsed <= 2048ul)
            policy.residentBudgetBytes = static_cast<std::uint64_t>(parsed) * 1024ull * 1024ull;
    }

    const char *maxDim = getenv("SUMMIT_TEXTURE_MAX_DIM");
    if (maxDim && *maxDim)
    {
        const unsigned long parsed = strtoul(maxDim, NULL, 10);
        if (parsed >= 512ul && parsed <= policy.maxSourceDimension)
        {
            policy.maxRuntimeDimension = static_cast<std::uint32_t>(parsed);
            policy.maxRuntimePixels = static_cast<std::uint64_t>(parsed) * static_cast<std::uint64_t>(parsed);
        }
    }

    const char *externalMaxDim = getenv("SUMMIT_EXTERNAL_TEXTURE_MAX_DIM");
    if (externalMaxDim && *externalMaxDim)
    {
        const unsigned long parsed = strtoul(externalMaxDim, NULL, 10);
        if (parsed >= 256ul && parsed <= policy.maxSourceDimension)
        {
            policy.maxExternalRuntimeDimension = static_cast<std::uint32_t>(parsed);
            policy.maxExternalRuntimePixels = static_cast<std::uint64_t>(parsed) * static_cast<std::uint64_t>(parsed);
        }
    }

    const char *prewarmLimit = getenv("SUMMIT_TEXTURE_PREWARM_LIMIT");
    if (prewarmLimit && *prewarmLimit)
    {
        const unsigned long parsed = strtoul(prewarmLimit, NULL, 10);
        if (parsed <= 512ul)
            policy.globalPrewarmLimit = static_cast<std::uint32_t>(parsed);
    }

    const char *scopedPrewarmLimit = getenv("SUMMIT_TEXTURE_SCOPED_PREWARM_LIMIT");
    if (scopedPrewarmLimit && *scopedPrewarmLimit)
    {
        const unsigned long parsed = strtoul(scopedPrewarmLimit, NULL, 10);
        if (parsed <= 1024ul)
            policy.scopedPrewarmLimit = static_cast<std::uint32_t>(parsed);
    }

    SummitAssetRegistry.SetPolicy(policy);
}

static D3DFORMAT SummitChooseLargeTextureFormat(const smTEXTUREHANDLE *lpTexHandle, const D3DXIMAGE_INFO &imageInfo)
{
    if (!lpTexHandle)
        return D3DFMT_FROM_FILE;

    if (imageInfo.Width < 4 || imageInfo.Height < 4)
        return D3DFMT_FROM_FILE;

    // DXT block compression requires complete 4x4 blocks at the top level.
    if ((imageInfo.Width & 3U) != 0 || (imageInfo.Height & 3U) != 0)
        return D3DFMT_FROM_FILE;

    const ULONGLONG rgbaBytes =
        static_cast<ULONGLONG>(imageInfo.Width) *
        static_cast<ULONGLONG>(imageInfo.Height) * 4ULL;

    if (rgbaBytes < SummitAssetRegistry.Policy().largeTextureRgbaBytes)
        return D3DFMT_FROM_FILE;

    // Existing loader marks TGA/PNG as opacity-capable and BMP/JPG as opaque.
    // Preserve alpha-capable content with DXT5; opaque content uses DXT1.
    return lpTexHandle->MapOpacity ? D3DFMT_DXT5 : D3DFMT_DXT1;
}

static void SummitLogLargeTexturePolicy(const char *name, const D3DXIMAGE_INFO &imageInfo, D3DFORMAT format, HRESULT hr)
{
    if (!SummitAssetTraceEnabled())
        return;
    if (format == D3DFMT_FROM_FILE)
        return;

    FILE *f = NULL;
    fopen_s(&f, "LOGS\\large_texture_memory_policy.txt", "a");
    if (!f)
        return;

    const char *formatName = format == D3DFMT_DXT1 ? "DXT1" : (format == D3DFMT_DXT5 ? "DXT5" : "OTHER");
    fprintf(
        f,
        "%s | %ux%u | %s | hr=0x%08lX\n",
        name ? name : "(null)",
        static_cast<unsigned>(imageInfo.Width),
        static_cast<unsigned>(imageInfo.Height),
        formatName,
        static_cast<unsigned long>(hr));
    fclose(f);
}

static void SummitLogTextureReject(const char *name, const char *reason, UINT width, UINT height)
{
    if (!SummitAssetTraceEnabled())
        return;
    FILE *f = NULL;
    fopen_s(&f, "LOGS\\texture_pipeline_rejects.txt", "a");
    if (!f)
        return;
    fprintf(
        f,
        "%s | %s | %ux%u\n",
        name ? name : "(null)",
        reason ? reason : "UNKNOWN",
        static_cast<unsigned>(width),
        static_cast<unsigned>(height));
    fclose(f);
}

int		   MaterialFrame;


int SetTextureSwapMode(int mode)
{
	int OldValue;

	OldValue = TextureSwapMode;

	TextureSwapMode = mode;

	return OldValue;
}


int smSetTextureLevel(int Level)
{
	switch (Level) {
	case 0:	
		
		TextureSwapMode = FALSE;
		TexturHalfSize = 0;
		break;

	case 1: 
		
		TextureSwapMode = TRUE;
		TexturHalfSize = 0;
		break;

	case 2: 
		
		TextureSwapMode = TRUE;
		TexturHalfSize = 1;
		break;

	case 3: 
		
		TextureSwapMode = TRUE;
		TexturHalfSize = 2;
		break;
	}

	return TRUE;
}



smTEXTURE::smTEXTURE()
{
	Init();
}

smTEXTURE::~smTEXTURE()
{
	
}

void smTEXTURE::Init()
{
	
	
	ZeroMemory(TexSpeedSum, sizeof(TexSpeedSum));
	

	ZeroMemory(Texture, sizeof(smTEXTUREHANDLE) * MAX_TEXTURE);
	ZeroMemory(SwapTexture, sizeof(smSWAPTEXTURE) * SWAPTEXTURE_MAX);
	SummitConfigureAssetLoadPolicy();
	SummitAssetRegistry.Reset();
	SummitMissingTexturePaths.clear();
	// Invalidate every async handle generation across material-system sessions.
	// Never reset generations back to a fixed value: a late completion from an
	// older GameMode session must not be able to match a newly reused slot.
	for (int asyncIndex = 0; asyncIndex < MAX_TEXTURE; ++asyncIndex)
	{
		SummitTextureAsyncGenerations[asyncIndex].fetch_add(1u, std::memory_order_acq_rel);
		SummitTextureAsyncStates[asyncIndex].store(SUMMIT_TEX_ASYNC_IDLE, std::memory_order_release);
	}
	SummitTextureAsyncInFlight.store(0u, std::memory_order_release);
	SummitTextureStagingBytes.store(0u, std::memory_order_release);
	TextureCnt = 0;
	SwapCount = 0;

	
	InitializeCriticalSection(&cTextureSection);

}

int smTEXTURE::Remove()
{
	int cnt;

	
	EnterCriticalSection(&cTextureSection);

	for (cnt = 0; cnt < SWAPTEXTURE_MAX; cnt++) {
		if (SwapTexture[cnt].SwapState == TRUE) {
			DeleteSwapTexture(cnt);
		}
	}

	for (cnt = 0; cnt < MAX_TEXTURE; cnt++) {
		if (Texture[cnt].UseCounter > 0) {
			if (Texture[cnt].lpD3DTexture) Texture[cnt].lpD3DTexture->Release();
			if (Texture[cnt].lpDDSurface) Texture[cnt].lpDDSurface->Release();
			if (Texture[cnt].lpDDSSysMemory) Texture[cnt].lpDDSSysMemory->Release();
			Texture[cnt].lpD3DTexture = NULL;
			Texture[cnt].lpDDSurface = NULL;
			Texture[cnt].lpDDSSysMemory = NULL;
			Texture[cnt].UseCounter = 0;
		}
	}
	SummitAssetRegistry.Reset();
	
	LeaveCriticalSection(&cTextureSection);

	
	DeleteCriticalSection(&cTextureSection);

	return TRUE;
}





int smTEXTURE::Find(DWORD dwCode, char *Name)
{
	int cnt;
	for (cnt = 0; cnt < MAX_TEXTURE; cnt++)
	{
		if (TexSpeedSum[cnt] == dwCode)
		{
			if (Texture[cnt].UseCounter && _stricmp(Texture[cnt].Name, Name) == 0)
				return cnt;
		}
	}

	return -1;
}




smTEXTUREHANDLE *smTEXTURE::Add(char *name, char *nameA)
{
    if (!name || !name[0])
        return NULL;

    SummitTextureCriticalSectionGuard guard(&cTextureSection);

    extern DWORD GetSpeedSum(const char *szName);
    const DWORD dwCode = GetSpeedSum(name);
    const int existing = Find(dwCode, name);

    if (existing >= 0)
    {
        LastTextureCnt = existing;
        Texture[existing].UseCounter++;
        if (!summit::core::concurrency::JobSystem::Instance().IsWorkerThread())
            SummitAssetRegistry.Register(static_cast<std::size_t>(existing), summit::rendering::assets::v2::ASSET_OWNER_GLOBAL);
        return &Texture[existing];
    }

    int cnt = 0;
    for (; cnt < MAX_TEXTURE; ++cnt)
        if (Texture[cnt].UseCounter == 0)
            break;

    if (cnt == MAX_TEXTURE)
        return NULL;

    TextureCnt = cnt;
    lpLastTextureSurface = &Texture[TextureCnt].lpDDSurface;
    lpLastTexture = &Texture[TextureCnt].lpD3DTexture;

    Texture[TextureCnt].lpDDSurface = NULL;
    Texture[TextureCnt].lpD3DTexture = NULL;
    Texture[TextureCnt].lpDDSSysMemory = NULL;
    Texture[TextureCnt].lpD3DTextureLarge = NULL;

    strcpy_s(Texture[TextureCnt].Name, sizeof(Texture[TextureCnt].Name), name);
    if (nameA)
        strcpy_s(Texture[TextureCnt].NameA, sizeof(Texture[TextureCnt].NameA), nameA);
    else
        Texture[TextureCnt].NameA[0] = 0;

    Texture[TextureCnt].TexSwapMode = TextureSwapMode;
    Texture[TextureCnt].UseCounter = 1;
    Texture[TextureCnt].loadingFailed = 0;
    TexSpeedSum[TextureCnt] = dwCode;

    SummitTextureAsyncGenerations[TextureCnt].fetch_add(1u, std::memory_order_acq_rel);
    SummitTextureAsyncStates[TextureCnt].store(SUMMIT_TEX_ASYNC_IDLE, std::memory_order_release);

    // Background stage/model parsers are allowed to register texture handles,
    // but the V2 residency registry remains game-thread owned.
    if (!summit::core::concurrency::JobSystem::Instance().IsWorkerThread())
        SummitAssetRegistry.Register(static_cast<std::size_t>(TextureCnt), summit::rendering::assets::v2::ASSET_OWNER_GLOBAL);

    return &Texture[TextureCnt];
}


int smTEXTURE::Delete(smTEXTUREHANDLE *lpTex)
{
    if (!lpTex)
        return FALSE;

    SummitTextureCriticalSectionGuard guard(&cTextureSection);
    if (lpTex->UseCounter <= 0)
        return FALSE;

    lpTex->UseCounter--;
    if (lpTex->UseCounter > 0)
        return lpTex->UseCounter;

    // DeleteSwapTexture() also acquires cTextureSection, so perform the same
    // cleanup inline while this lock is held instead of recursively locking it.
    if (lpTex->lpD3DTextureLarge)
    {
        for (int cnt = 0; cnt < SWAPTEXTURE_MAX; ++cnt)
        {
            if (!SwapTexture[cnt].SwapState || SwapTexture[cnt].smTextureMaster != lpTex)
                continue;
            if (SwapTexture[cnt].lpDDSurface)
            {
                SwapTexture[cnt].lpDDSurface->Release();
                SwapTexture[cnt].lpDDSurface = 0;
            }
            if (SwapTexture[cnt].lpD3DTexture)
            {
                SwapTexture[cnt].lpD3DTexture->Release();
                SwapTexture[cnt].lpD3DTexture = 0;
            }
            SwapTexture[cnt].SwapState = FALSE;
            SwapTexture[cnt].smTextureMaster = 0;
            lpTex->lpD3DTextureLarge = NULL;
            break;
        }
    }

    if (lpTex->lpDDSurface) lpTex->lpDDSurface->Release();
    if (lpTex->lpD3DTexture) lpTex->lpD3DTexture->Release();
    if (lpTex->lpDDSSysMemory) lpTex->lpDDSSysMemory->Release();

    lpTex->lpD3DTexture = 0;
    lpTex->lpDDSurface = 0;
    lpTex->lpDDSSysMemory = 0;

    for (int cnt = 0; cnt < MAX_TEXTURE; ++cnt)
    {
        if (&Texture[cnt] != lpTex)
            continue;

        SummitTextureAsyncGenerations[cnt].fetch_add(1u, std::memory_order_acq_rel);
        SummitTextureAsyncStates[cnt].store(SUMMIT_TEX_ASYNC_IDLE, std::memory_order_release);
        if (!summit::core::concurrency::JobSystem::Instance().IsWorkerThread())
            SummitAssetRegistry.MarkReleased(static_cast<std::size_t>(cnt));
        break;
    }

    const DWORD releasedBytes = static_cast<DWORD>(lpTex->Width * lpTex->Height * 2);
    VramTotal = VramTotal > releasedBytes ? VramTotal - releasedBytes : 0;
    return 0;
}

int smTEXTURE::DeleteSwapTexture(int TexNum)
{
	
	EnterCriticalSection(&cTextureSection);

	if (SwapTexture[TexNum].lpDDSurface) {
		SwapTexture[TexNum].lpDDSurface->Release();
		SwapTexture[TexNum].lpDDSurface = 0;
	}

	if (SwapTexture[TexNum].lpD3DTexture) {
		SwapTexture[TexNum].lpD3DTexture->Release();
		SwapTexture[TexNum].lpD3DTexture = 0;
	}

	SwapTexture[TexNum].smTextureMaster->lpD3DTextureLarge = NULL;
	SwapTexture[TexNum].SwapState = FALSE;

	
	VramTotal -= SwapTexture[TexNum].smTextureMaster->Width *
		SwapTexture[TexNum].smTextureMaster->Height * 2;

	
	LeaveCriticalSection(&cTextureSection);

	return TRUE;
}

int smTEXTURE::SetLargeTexture(smTEXTUREHANDLE *TexHandle)
{
	return TRUE;
}


int smTEXTURE::ReadTexture()
{
    if (g_IsReadTextures == 0)
        return 0;

    int ReadCount = 0;
    char szErrMsg[128];

    // Asset Load V2 changes the old global sweep into a bounded prewarm: only
    // registrations marked pending are attempted here. Explicit material-group
    // loads and renderer demand-loads can still materialize evicted resources.
    const std::uint32_t prewarmLimit = SummitAssetRegistry.Policy().globalPrewarmLimit;
    if (prewarmLimit == 0)
        return 0;

    for (int cnt = 0; cnt < MAX_TEXTURE && static_cast<std::uint32_t>(ReadCount) < prewarmLimit; ++cnt)
    {
        if (!SummitPrewarmHasHeadroom())
            break;
        if (!Texture[cnt].UseCounter || Texture[cnt].lpD3DTexture)
            continue;
        if (!SummitAssetRegistry.IsPendingGlobal(static_cast<std::size_t>(cnt)))
            continue;
        if (!SummitAssetRegistry.CanAttempt(static_cast<std::size_t>(cnt)))
            continue;

        const int Result = AssetLoadV3RequestTexture(&Texture[cnt], 0, 2);
        if (Result <= 0 && !AssetLoadV3IsTexturePending(&Texture[cnt]))
        {
            wsprintf(szErrMsg, "Falha TEX_V3 [%s]", Texture[cnt].Name);
            AddErrorList(szErrMsg);
        }
        ++ReadCount;
        if ((ReadCount & 0x7) == 0)
            smPlayWindowMessage();
    }

    return ReadCount;
}


int smTEXTURE::Restore()
{
    for (int cnt = 0; cnt < SWAPTEXTURE_MAX; ++cnt)
    {
        if (SwapTexture[cnt].SwapState == TRUE)
            DeleteSwapTexture(cnt);
    }

    EnterCriticalSection(&cTextureSection);
    for (int cnt = 0; cnt < MAX_TEXTURE; ++cnt)
    {
        if (!Texture[cnt].UseCounter)
            continue;

        if (Texture[cnt].lpD3DTexture) Texture[cnt].lpD3DTexture->Release();
        if (Texture[cnt].lpDDSurface) Texture[cnt].lpDDSurface->Release();
        if (Texture[cnt].lpDDSSysMemory) Texture[cnt].lpDDSSysMemory->Release();
        Texture[cnt].lpD3DTexture = 0;
        Texture[cnt].lpDDSurface = 0;
        Texture[cnt].lpDDSSysMemory = 0;
        SummitAssetRegistry.MarkDeviceRestoreNeeded(static_cast<std::size_t>(cnt));
    }
    LeaveCriticalSection(&cTextureSection);

    ReadTexture();
    AssetLoadV2DumpTelemetry("DEVICE_RESTORE");
    return TRUE;
}










smTEXTURE smTexture;

static int SummitTextureHandleIndex(const smTEXTUREHANDLE *lpTexHandle)
{
    if (!lpTexHandle)
        return -1;

    const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(&smTexture.Texture[0]);
    const std::uintptr_t end = reinterpret_cast<std::uintptr_t>(&smTexture.Texture[MAX_TEXTURE]);
    const std::uintptr_t value = reinterpret_cast<std::uintptr_t>(lpTexHandle);
    if (value < base || value >= end)
        return -1;

    const std::uintptr_t offset = value - base;
    if ((offset % sizeof(smTEXTUREHANDLE)) != 0)
        return -1;

    const std::size_t index = static_cast<std::size_t>(offset / sizeof(smTEXTUREHANDLE));
    return index < MAX_TEXTURE ? static_cast<int>(index) : -1;
}


static ULONGLONG SummitEstimateSurfaceBytes(UINT width, UINT height, D3DFORMAT format)
{
    if (!width || !height)
        return 0;

    switch (format)
    {
    case D3DFMT_DXT1:
        return static_cast<ULONGLONG>((width + 3U) / 4U) *
               static_cast<ULONGLONG>((height + 3U) / 4U) * 8ULL;
    case D3DFMT_DXT2:
    case D3DFMT_DXT3:
    case D3DFMT_DXT4:
    case D3DFMT_DXT5:
        return static_cast<ULONGLONG>((width + 3U) / 4U) *
               static_cast<ULONGLONG>((height + 3U) / 4U) * 16ULL;
    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_A4R4G4B4:
        return static_cast<ULONGLONG>(width) * height * 2ULL;
    case D3DFMT_A8:
    case D3DFMT_L8:
        return static_cast<ULONGLONG>(width) * height;
    default:
        return static_cast<ULONGLONG>(width) * height * 4ULL;
    }
}

static ULONGLONG SummitEstimateTextureBytes(LPDIRECT3DTEXTURE9 texture, UINT *residentWidth = NULL, UINT *residentHeight = NULL)
{
    if (!texture)
        return 0;

    ULONGLONG bytes = 0;
    const UINT levels = texture->GetLevelCount();
    for (UINT level = 0; level < levels; ++level)
    {
        D3DSURFACE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        if (FAILED(texture->GetLevelDesc(level, &desc)))
            continue;
        if (level == 0)
        {
            if (residentWidth) *residentWidth = desc.Width;
            if (residentHeight) *residentHeight = desc.Height;
        }
        bytes += SummitEstimateSurfaceBytes(desc.Width, desc.Height, desc.Format);
    }
    return bytes;
}

static void SummitLogAssetEvent(const char *eventName, const smTEXTUREHANDLE *texture, ULONGLONG bytes = 0, const char *detail = NULL)
{
    if (!SummitAssetTraceEnabled())
        return;
    FILE *f = NULL;
    fopen_s(&f, "LOGS\\asset_load_v2_events.txt", "a");
    if (!f)
        return;
    fprintf(
        f,
        "%s | %s | bytes=%llu | %s\n",
        eventName ? eventName : "EVENT",
        texture ? texture->Name : "(null)",
        static_cast<unsigned long long>(bytes),
        detail ? detail : "-");
    fclose(f);
}

static void SummitTextureMarkPermanentFailure(smTEXTUREHANDLE *lpTexHandle, const char *reason, UINT width = 0, UINT height = 0)
{
    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index >= 0)
        SummitAssetRegistry.MarkPermanentFailure(static_cast<std::size_t>(index), width, height);
    SummitLogTextureReject(lpTexHandle ? lpTexHandle->Name : NULL, reason, width, height);
}

static bool SummitIsDeviceUnavailableHRESULT(HRESULT hr)
{
    return hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICENOTRESET;
}

static void SummitTextureMarkWaitingForDevice(smTEXTUREHANDLE *lpTexHandle, UINT width = 0, UINT height = 0)
{
    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index < 0)
        return;
    SummitAssetRegistry.MarkWaitingForDevice(static_cast<std::size_t>(index), width, height);
    SummitLogAssetEvent("WAIT_DEVICE", lpTexHandle, 0, "retry_after_restore");
}

static void SummitTextureRecordCreateFailure(smTEXTUREHANDLE *lpTexHandle, HRESULT hr, UINT width, UINT height)
{
    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index < 0)
        return;

    if (SummitIsDeviceUnavailableHRESULT(hr))
    {
        SummitTextureMarkWaitingForDevice(lpTexHandle, width, height);
        return;
    }

    const bool permanent = SummitAssetRegistry.MarkTransientFailure(static_cast<std::size_t>(index), width, height);
    if (permanent)
    {
        char reason[64] = { 0 };
        wsprintf(reason, "CREATE_FAILED_0x%08lX", static_cast<unsigned long>(hr));
        SummitLogTextureReject(lpTexHandle->Name, reason, width, height);
    }
}

static void SummitTextureRecordCreateSuccess(smTEXTUREHANDLE *lpTexHandle, UINT sourceWidth, UINT sourceHeight)
{
    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index < 0 || !lpTexHandle || !lpTexHandle->lpD3DTexture)
        return;

    UINT residentWidth = 0;
    UINT residentHeight = 0;
    const ULONGLONG bytes = SummitEstimateTextureBytes(lpTexHandle->lpD3DTexture, &residentWidth, &residentHeight);
    SummitAssetRegistry.MarkLoaded(
        static_cast<std::size_t>(index),
        sourceWidth,
        sourceHeight,
        residentWidth,
        residentHeight,
        bytes);
}

static void SummitAssetEvictIndex(std::size_t index, const char *reason)
{
    if (index >= MAX_TEXTURE)
        return;

    smTEXTUREHANDLE &texture = smTexture.Texture[index];
    const ULONGLONG bytes = SummitAssetRegistry.Record(index).residentBytes;

    if (texture.lpD3DTexture)
    {
        texture.lpD3DTexture->Release();
        texture.lpD3DTexture = NULL;
    }
    if (texture.lpDDSurface)
    {
        texture.lpDDSurface->Release();
        texture.lpDDSurface = NULL;
    }
    if (texture.lpDDSSysMemory)
    {
        texture.lpDDSSysMemory->Release();
        texture.lpDDSSysMemory = NULL;
    }

    SummitAssetRegistry.MarkEvicted(index);
    SummitLogAssetEvent("EVICT", &texture, bytes, reason);
}

static bool SummitAssetEnsureBudget(ULONGLONG incomingBytes, std::size_t excludeIndex)
{
    const ULONGLONG budget = SummitAssetRegistry.Policy().residentBudgetBytes;
    if (!budget || incomingBytes > budget)
        return incomingBytes <= budget;

    while (SummitAssetRegistry.ResidentBytes() + incomingBytes > budget)
    {
        std::size_t victim = SummitAssetRegistry.SelectEvictionCandidate(excludeIndex, false);
        if (victim >= MAX_TEXTURE)
            victim = SummitAssetRegistry.SelectEvictionCandidate(excludeIndex, true);
        if (victim >= MAX_TEXTURE)
            return false;
        SummitAssetEvictIndex(victim, "BUDGET");
    }
    return true;
}

void AssetLoadV2TouchTexture(smTEXTUREHANDLE *lpTexHandle)
{
    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index >= 0)
        SummitAssetRegistry.Touch(static_cast<std::size_t>(index));
}

void AssetLoadV2AddOwner(smTEXTUREHANDLE *lpTexHandle, DWORD ownerMask)
{
    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index >= 0)
        SummitAssetRegistry.Register(static_cast<std::size_t>(index), ownerMask);
}

int AssetLoadV2EnsureTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap)
{
    if (!lpTexHandle || lpTexHandle->UseCounter <= 0)
        return FALSE;

    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index < 0)
        return FALSE;

    if (lpTexHandle->lpD3DTexture)
    {
        SummitAssetRegistry.Touch(static_cast<std::size_t>(index));
        return TRUE;
    }

    // V3 is request-based: a cache miss never performs file I/O or D3DX image
    // creation on the caller. CPU preparation goes to the worker pool and the
    // render thread later commits a bounded number of ready uploads.
    AssetLoadV3RequestTexture(lpTexHandle, MipMap, 0);
    return lpTexHandle->lpD3DTexture ? TRUE : FALSE;
}

LPDIRECT3DTEXTURE9 AssetLoadV2GetResidentTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap)
{
    if (!lpTexHandle)
        return NULL;
    AssetLoadV2EnsureTexture(lpTexHandle, MipMap);
    AssetLoadV2TouchTexture(lpTexHandle);
    return lpTexHandle->lpD3DTexture;
}

void AssetLoadV2DumpTelemetry(const char *reason)
{
    const summit::rendering::assets::v2::AssetLoadStats stats = SummitAssetRegistry.Stats();
    std::size_t referenced = 0;
    std::size_t residentHandles = 0;
    for (std::size_t i = 0; i < MAX_TEXTURE; ++i)
    {
        if (smTexture.Texture[i].UseCounter > 0)
            ++referenced;
        if (smTexture.Texture[i].lpD3DTexture)
            ++residentHandles;
    }

    FILE *f = NULL;
    fopen_s(&f, "LOGS\\asset_load_v2_telemetry.txt", "a");
    if (!f)
        return;
    fprintf(
        f,
        "%s | registered=%u referenced=%u loaded=%u resident_handles=%u transient=%u permanent=%u waiting_device=%u evicted=%u bytes=%llu budget=%llu runtime_max=%u external_max=%u prewarm_limit=%u scoped_prewarm_limit=%u serial=%llu\n",
        reason ? reason : "SNAPSHOT",
        static_cast<unsigned>(stats.registeredCount),
        static_cast<unsigned>(referenced),
        static_cast<unsigned>(stats.loadedCount),
        static_cast<unsigned>(residentHandles),
        static_cast<unsigned>(stats.transientFailureCount),
        static_cast<unsigned>(stats.permanentFailureCount),
        static_cast<unsigned>(stats.waitingDeviceCount),
        static_cast<unsigned>(stats.evictedCount),
        static_cast<unsigned long long>(stats.residentBytes),
        static_cast<unsigned long long>(SummitAssetRegistry.Policy().residentBudgetBytes),
        static_cast<unsigned>(SummitAssetRegistry.Policy().maxRuntimeDimension),
        static_cast<unsigned>(SummitAssetRegistry.Policy().maxExternalRuntimeDimension),
        static_cast<unsigned>(SummitAssetRegistry.Policy().globalPrewarmLimit),
        static_cast<unsigned>(SummitAssetRegistry.Policy().scopedPrewarmLimit),
        static_cast<unsigned long long>(stats.serial));
    fclose(f);
}

void AssetLoadV2TrimToBudget(const char *reason)
{
    const ULONGLONG budget = SummitAssetRegistry.Policy().residentBudgetBytes;
    const ULONGLONG target = budget * 3ULL / 4ULL;
    while (SummitAssetRegistry.ResidentBytes() > target)
    {
        std::size_t victim = SummitAssetRegistry.SelectEvictionCandidate(MAX_TEXTURE, false);
        if (victim >= MAX_TEXTURE)
            victim = SummitAssetRegistry.SelectEvictionCandidate(MAX_TEXTURE, true);
        if (victim >= MAX_TEXTURE)
            break;
        SummitAssetEvictIndex(victim, reason ? reason : "TRIM");
    }
    AssetLoadV2DumpTelemetry(reason ? reason : "TRIM");
}

int AssetLoadV2ReadMaterialGroup(smMATERIAL_GROUP *group, DWORD ownerMask, int MipMap)
{
    if (!group)
        return 0;

    for (DWORD materialIndex = 0; materialIndex < group->MaterialCount; ++materialIndex)
    {
        smMATERIAL &material = group->smMaterial[materialIndex];
        if (!material.InUse)
            continue;
        for (DWORD textureIndex = 0; textureIndex < material.TextureCounter; ++textureIndex)
            if (material.smTexture[textureIndex])
                AssetLoadV2AddOwner(material.smTexture[textureIndex], ownerMask);
        for (DWORD textureIndex = 0; textureIndex < material.AnimTexCounter; ++textureIndex)
            if (material.smAnimTexture[textureIndex])
                AssetLoadV2AddOwner(material.smAnimTexture[textureIndex], ownerMask);
    }

    return group->ReadTextures(MipMap);
}

int AssetLoadV2ReleaseMaterial(smMATERIAL_GROUP *group, int materialIndex)
{
    if (!group || materialIndex < 0 || static_cast<DWORD>(materialIndex) >= group->MaterialCount)
        return FALSE;

    smMATERIAL &material = group->smMaterial[materialIndex];
    if (!material.InUse)
        return FALSE;

    for (DWORD textureIndex = 0; textureIndex < material.TextureCounter; ++textureIndex)
    {
        if (material.smTexture[textureIndex])
            smTexture.Delete(material.smTexture[textureIndex]);
    }
    for (DWORD textureIndex = 0; textureIndex < material.AnimTexCounter; ++textureIndex)
    {
        if (material.smAnimTexture[textureIndex])
            smTexture.Delete(material.smAnimTexture[textureIndex]);
    }
    ZeroMemory(&material, sizeof(material));
    return TRUE;
}




smMATERIAL_GROUP::smMATERIAL_GROUP()
{

}

smMATERIAL_GROUP::smMATERIAL_GROUP(int MaterialMax)
{
	Init(MaterialMax);
}

smMATERIAL_GROUP::~smMATERIAL_GROUP()
{
	Close();
}


void smMATERIAL_GROUP::Init(int MaterialMax)
{

	MaterialCount = 0;
	MaxMaterial = MaterialMax;
	ReformTexture = 0;

	smMaterial = new smMATERIAL[MaterialMax];
	ZeroMemory(smMaterial, sizeof(smMATERIAL)*MaterialMax);

}


int smMATERIAL_GROUP::Close()
{
	DWORD cnt, tcnt;

	for (cnt = 0; cnt < MaterialCount; cnt++) {
		if (smMaterial[cnt].InUse) {
			
			for (tcnt = 0; tcnt < smMaterial[cnt].TextureCounter; tcnt++)
				if (smMaterial[cnt].smTexture[tcnt])
					smTexture.Delete(smMaterial[cnt].smTexture[tcnt]);

			for (tcnt = 0; tcnt < smMaterial[cnt].AnimTexCounter; tcnt++)
				if (smMaterial[cnt].smAnimTexture[tcnt])
					smTexture.Delete(smMaterial[cnt].smAnimTexture[tcnt]);

		}
	}

	delete[] smMaterial;

	return TRUE;
}

#define TEXFILENAME_SIZE	64


int smMATERIAL_GROUP::GetSaveSize()
{
	int size;
	DWORD cnt, tcnt;
	int	len, alen;

	size = sizeof(smMATERIAL_GROUP);

	for (cnt = 0; cnt < MaterialCount; cnt++) {
		size += sizeof(smMATERIAL);
		if (smMaterial[cnt].InUse) {
			size += sizeof(int);					
			for (tcnt = 0; tcnt < smMaterial[cnt].TextureCounter; tcnt++) {
				len = lstrlen(smMaterial[cnt].smTexture[tcnt]->Name) + 1;
				alen = lstrlen(smMaterial[cnt].smTexture[tcnt]->NameA) + 1;
				size += len;
				size += alen;
			}

			for (tcnt = 0; tcnt < smMaterial[cnt].AnimTexCounter; tcnt++) {
				len = lstrlen(smMaterial[cnt].smAnimTexture[tcnt]->Name) + 1;
				alen = lstrlen(smMaterial[cnt].smAnimTexture[tcnt]->NameA) + 1;
				size += len;
				size += alen;
			}

		}
	}
	return size;
}


int smMATERIAL_GROUP::SaveFile(HANDLE hFile)
{
	DWORD	dwAcess;
	DWORD cnt, tcnt;
	int	len;
	int size;

	size = WriteFile(hFile, &Head, sizeof(smMATERIAL_GROUP), &dwAcess, NULL);

	for (cnt = 0; cnt < MaterialCount; cnt++) {
		
		size += WriteFile(hFile, &smMaterial[cnt], sizeof(smMATERIAL), &dwAcess, NULL);

		if (smMaterial[cnt].InUse) {

			
			len = 0;
			for (tcnt = 0; tcnt < smMaterial[cnt].TextureCounter; tcnt++) {
				len += lstrlen(smMaterial[cnt].smTexture[tcnt]->Name) + 1;
				len += lstrlen(smMaterial[cnt].smTexture[tcnt]->NameA) + 1;
			}
			for (tcnt = 0; tcnt < smMaterial[cnt].AnimTexCounter; tcnt++) {
				len += lstrlen(smMaterial[cnt].smAnimTexture[tcnt]->Name) + 1;
				len += lstrlen(smMaterial[cnt].smAnimTexture[tcnt]->NameA) + 1;
			}
			
			size += WriteFile(hFile, &len, sizeof(int), &dwAcess, NULL);

			
			for (tcnt = 0; tcnt < smMaterial[cnt].TextureCounter; tcnt++) {
				len = lstrlen(smMaterial[cnt].smTexture[tcnt]->Name) + 1;
				size += WriteFile(hFile, smMaterial[cnt].smTexture[tcnt]->Name, len, &dwAcess, NULL);
				len = lstrlen(smMaterial[cnt].smTexture[tcnt]->NameA) + 1;
				size += WriteFile(hFile, smMaterial[cnt].smTexture[tcnt]->NameA, len, &dwAcess, NULL);
			}

			for (tcnt = 0; tcnt < smMaterial[cnt].AnimTexCounter; tcnt++) {
				len = lstrlen(smMaterial[cnt].smAnimTexture[tcnt]->Name) + 1;
				size += WriteFile(hFile, smMaterial[cnt].smAnimTexture[tcnt]->Name, len, &dwAcess, NULL);
				len = lstrlen(smMaterial[cnt].smAnimTexture[tcnt]->NameA) + 1;
				size += WriteFile(hFile, smMaterial[cnt].smAnimTexture[tcnt]->NameA, len, &dwAcess, NULL);
			}

		}
	}

	return size;
}


int smMATERIAL_GROUP::LoadFile(HANDLE hFile)
{
	DWORD	dwAcess;
	DWORD	cnt, tcnt;
	int		StrLen;
	int		size;
	char	szNameBuff[4096];
	char	*lpNameBuff;
	char	*szName, *szNameA;

	
	size = ReadFile(hFile, &Head, sizeof(smMATERIAL_GROUP), &dwAcess, NULL);

	
	smMaterial = new smMATERIAL[MaterialCount];

	for (cnt = 0; cnt < MaterialCount; cnt++) {
		
		size += ReadFile(hFile, &smMaterial[cnt], sizeof(smMATERIAL), &dwAcess, NULL);

		if (smMaterial[cnt].InUse) {
			
			size += ReadFile(hFile, &StrLen, sizeof(int), &dwAcess, NULL);
			
			size += ReadFile(hFile, szNameBuff, StrLen, &dwAcess, NULL);

			lpNameBuff = szNameBuff;

			
			for (tcnt = 0; tcnt < smMaterial[cnt].TextureCounter; tcnt++) {
				szName = lpNameBuff;
				lpNameBuff += lstrlen(szName) + 1;
				szNameA = lpNameBuff;
				lpNameBuff += lstrlen(szNameA) + 1;

				if (szNameA[0])
					smMaterial[cnt].smTexture[tcnt] = smTexture.Add(szName, szNameA);
				else
					smMaterial[cnt].smTexture[tcnt] = smTexture.Add(szName);
			}

			
			for (tcnt = 0; tcnt < smMaterial[cnt].AnimTexCounter; tcnt++) {
				szName = lpNameBuff;
				lpNameBuff += lstrlen(szName) + 1;
				szNameA = lpNameBuff;
				lpNameBuff += lstrlen(szNameA) + 1;

				if (szNameA[0])
					smMaterial[cnt].smAnimTexture[tcnt] = smTexture.Add(szName, szNameA);
				else
					smMaterial[cnt].smAnimTexture[tcnt] = smTexture.Add(szName);
			}
		}

	}

	return size;
}


int smMATERIAL_GROUP::DeleteUnuseMaterial()
{
	DWORD cnt, dcnt, tcnt;

	dcnt = 0;
	for (cnt = 0; cnt < MaterialCount; cnt++) {
		if (smMaterial[cnt].InUse == TRUE) {
			
			for (tcnt = 0; tcnt < smMaterial[cnt].TextureCounter; tcnt++)
				smTexture.Delete(smMaterial[cnt].smTexture[tcnt]);

			for (tcnt = 0; tcnt < smMaterial[cnt].AnimTexCounter; tcnt++)
				smTexture.Delete(smMaterial[cnt].smAnimTexture[tcnt]);

			ZeroMemory(&smMaterial[cnt], sizeof(smMATERIAL));
			
			dcnt++;
		}
	}

	return dcnt;
}



int smMATERIAL_GROUP::AddMaterial(ASE_MATERIAL *aseMaterial, char *szScript)
{
	int cnt, len;
	int	AnimCount, FrameSpeed;
	int MatNum;
	char	*lpAnimFileList[16];
	char	*szDirectory;
	char	*lpScript;
	char	szTemp[64];

	MatNum = MaterialCount;
	MaterialCount++;

	smMaterial[MatNum].InUse = TRUE;
	smMaterial[MatNum].Shade = TRUE;
	smMaterial[MatNum].BlendType = SMMAT_BLEND_ALPHA;
	smMaterial[MatNum].TwoSide = aseMaterial->TwoSide;
	smMaterial[MatNum].ReformTexture = 0;

	
	if (!aseMaterial->MAP_OPACITY[0]) {
		for (cnt = 0; cnt < aseMaterial->TextureCounter; cnt++) {
			smMaterial[MatNum].smTexture[cnt] = smTexture.Add(aseMaterial->BITMAP[cnt]);
			smMaterial[MatNum].TextureStageState[cnt] = aseMaterial->BitmapStateState[cnt];
			smMaterial[MatNum].TextureFormState[cnt] = aseMaterial->BitmapFormState[cnt];
			smMaterial[MatNum].ReformTexture += aseMaterial->BitmapFormState[cnt];
		}
		smMaterial[MatNum].MapOpacity = FALSE;
	}
	else {
		
		
		if (aseMaterial->TextureCounter > 1)
		{
			for (cnt = 0; cnt < aseMaterial->TextureCounter - 1; cnt++)
			{
				smMaterial[MatNum].smTexture[cnt] = smTexture.Add(aseMaterial->BITMAP[cnt], aseMaterial->MAP_OPACITY);
				smMaterial[MatNum].TextureStageState[cnt] = aseMaterial->BitmapStateState[cnt];
				smMaterial[MatNum].TextureFormState[cnt] = aseMaterial->BitmapFormState[cnt];
				smMaterial[MatNum].ReformTexture += aseMaterial->BitmapFormState[cnt];
			}

			smMaterial[MatNum].smTexture[cnt] = smTexture.Add(aseMaterial->BITMAP[cnt]);
			smMaterial[MatNum].TextureStageState[cnt] = aseMaterial->BitmapStateState[cnt];
			smMaterial[MatNum].TextureFormState[cnt] = aseMaterial->BitmapFormState[cnt];
			smMaterial[MatNum].ReformTexture += aseMaterial->BitmapFormState[cnt];
		}
		else
		{
			for (cnt = 0; cnt < aseMaterial->TextureCounter; cnt++)
			{
				smMaterial[MatNum].smTexture[cnt] = smTexture.Add(aseMaterial->BITMAP[cnt], aseMaterial->MAP_OPACITY);
				smMaterial[MatNum].TextureStageState[cnt] = aseMaterial->BitmapStateState[cnt];
				smMaterial[MatNum].TextureFormState[cnt] = aseMaterial->BitmapFormState[cnt];
				smMaterial[MatNum].ReformTexture += aseMaterial->BitmapFormState[cnt];
			}
		}

		smMaterial[MatNum].MapOpacity = TRUE;
		
	}

	
	if (smMaterial[MatNum].ReformTexture)	ReformTexture++;

	smMaterial[MatNum].TextureCounter = aseMaterial->TextureCounter;	
	smMaterial[MatNum].Transparency = aseMaterial->Transparency;		
	smMaterial[MatNum].SelfIllum = aseMaterial->SelfIllum;		
	smMaterial[MatNum].Diffuse = aseMaterial->Diffuse;			

	smMaterial[MatNum].UseState = aseMaterial->ScriptState;
	smMaterial[MatNum].TextureType = SMTEX_TYPE_MULTIMIX;

	if (aseMaterial->BlendType)
		smMaterial[MatNum].BlendType = aseMaterial->BlendType;

	
	if (smMaterial[MatNum].Transparency == 0)
		smMaterial[MatNum].MeshState = SMMAT_STAT_CHECK_FACE;

	
	if ((aseMaterial->ScriptState&sMATS_SCRIPT_WIND)) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDZ1;
		smMaterial[MatNum].MeshState = 0;
	}
	if ((aseMaterial->ScriptState&sMATS_SCRIPT_WINDX1)) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDX1;
		smMaterial[MatNum].MeshState = 0;
	}
	if ((aseMaterial->ScriptState&sMATS_SCRIPT_WINDX2)) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDX2;
		smMaterial[MatNum].MeshState = 0;
	}
	if ((aseMaterial->ScriptState&sMATS_SCRIPT_WINDZ1)) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDZ1;
		smMaterial[MatNum].MeshState = 0;
	}
	if ((aseMaterial->ScriptState&sMATS_SCRIPT_WINDZ2)) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDZ2;
		smMaterial[MatNum].MeshState = 0;
	}

	if ((aseMaterial->ScriptState&sMATS_SCRIPT_WINDZ2)) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDZ2;
		smMaterial[MatNum].MeshState = 0;
	}

	if ((aseMaterial->ScriptState&sMATS_SCRIPT_WATER)) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WATER;
		smMaterial[MatNum].MeshState = 0;
	}

	
	if ((aseMaterial->ScriptState&sMATS_SCRIPT_NOTPASS)) {
		smMaterial[MatNum].MeshState = SMMAT_STAT_CHECK_FACE;	
	}
	else {
		if ((aseMaterial->ScriptState&sMATS_SCRIPT_PASS)) {
			smMaterial[MatNum].MeshState = 0;					
		}
	}
	
	if ((aseMaterial->ScriptState&sMATS_SCRIPT_RENDLATTER)) {
		smMaterial[MatNum].MeshState |= sMATS_SCRIPT_RENDLATTER;
	}

	
	
	if ((aseMaterial->ScriptState & sMATS_SCRIPT_CHECK_ICE))
		smMaterial[MatNum].MeshState |= sMATS_SCRIPT_CHECK_ICE;
	

	
	
	if ((aseMaterial->ScriptState & sMATS_SCRIPT_ORG_WATER))
		smMaterial[MatNum].MeshState = sMATS_SCRIPT_ORG_WATER;
	

	
	AnimCount = 0;
	if (szScript) {
		if (aseMaterial->ScriptState&sMATS_SCRIPT_ANIM2) AnimCount = 2;
		if (aseMaterial->ScriptState&sMATS_SCRIPT_ANIM4) AnimCount = 4;
		if (aseMaterial->ScriptState&sMATS_SCRIPT_ANIM8) AnimCount = 8;
		if (aseMaterial->ScriptState&sMATS_SCRIPT_ANIM16) AnimCount = 16;
	}

	if (AnimCount) {
		len = lstrlen(szScript);

		for (cnt = 0; cnt < len; cnt++) {
			if (szScript[cnt] == 'a' && szScript[cnt + 1] == 'n' &&
				szScript[cnt + 2] == 'i' && szScript[cnt + 3] == 'm') {
				for (cnt = cnt; cnt < len; cnt++) {
					if (szScript[cnt] == ':') break;
				}
				break;
			}
		}

		if (cnt < len) {
			cnt++;
			lpScript = &szScript[cnt];
			FrameSpeed = 0;

			for (cnt = cnt; cnt < len; cnt++) {
				if (szScript[cnt] == ':') {
					FrameSpeed = atoi(&szScript[cnt + 1]);
					szScript[cnt] = 0;
					break;
				}
			}

			szDirectory = SetDirectoryFromFile(aseMaterial->BITMAP[0]);

			if (szDirectory) {
				for (cnt = 0; cnt < AnimCount; cnt++) {
					lpAnimFileList[cnt] = new char[64];
					lstrcpy(szTemp, szDirectory);
					lstrcat(szTemp, lpScript);
					wsprintf(lpAnimFileList[cnt], szTemp, cnt);
				}

				ChangeMaterialToAnimation(&smMaterial[MatNum], lpAnimFileList, AnimCount, smMaterial[MatNum].BlendType, TRUE);
				if (FrameSpeed) smMaterial[MatNum].Shift_FrameSpeed = FrameSpeed;			

				smMaterial[MatNum].Transparency = aseMaterial->Transparency;		

				for (cnt = AnimCount - 1; cnt >= 0; cnt--) delete lpAnimFileList[cnt];
			}
		}
	}

	
	
	if ((aseMaterial->ScriptState & sMATS_SCRIPT_BLINK_COLOR) && smMaterial[MatNum].WindMeshBottom == 0)
	{
		for (cnt = 0; cnt < MAX_MAP_BLINK_COLOR_TIME; cnt++)
		{
			if (strstr(szScript, szBlinkTimeScript[cnt]) != 0)
				break;
		}

		if (cnt >= MAX_MAP_BLINK_COLOR_TIME)
			smMaterial[MatNum].WindMeshBottom = dwBlinkTimeCode[0];
		else
			smMaterial[MatNum].WindMeshBottom = dwBlinkTimeCode[cnt];
	}
	

	aseMaterial->Regist++;												
	aseMaterial->RegistNum = MatNum;

	return MatNum;
}


int smMATERIAL_GROUP::CreateTextureMaterial(char *szBitmap, char *szOpacity, int TwoSide, int Shade, DWORD TexSwap, DWORD BlendType)
{
	int MatNum = -1;

	// Asset Load V2 allows effect/UI material wrappers to release their slot.
	// Reuse holes so repeated cache clear/reload cycles do not exhaust the
	// fixed 5000-entry legacy material table.
	for (DWORD index = 0; index < MaterialCount; ++index)
	{
		if (!smMaterial[index].InUse)
		{
			MatNum = static_cast<int>(index);
			break;
		}
	}
	if (MatNum < 0)
	{
		if (MaterialCount >= static_cast<DWORD>(MaxMaterial))
			return -1;
		MatNum = static_cast<int>(MaterialCount++);
	}

	ZeroMemory(&smMaterial[MatNum], sizeof(smMATERIAL));
	smMaterial[MatNum].InUse = TRUE;
	smMaterial[MatNum].Shade = Shade;
	smMaterial[MatNum].TwoSide = TwoSide;

	if (szOpacity)
		smMaterial[MatNum].smTexture[0] = smTexture.Add(szBitmap, szOpacity);
	else
		smMaterial[MatNum].smTexture[0] = smTexture.Add(szBitmap);

	if (!smMaterial[MatNum].smTexture[0])
	{
		ZeroMemory(&smMaterial[MatNum], sizeof(smMATERIAL));
		return -1;
	}

	smMaterial[MatNum].ReformTexture = 0;

	smMaterial[MatNum].TextureStageState[0] = 0;
	smMaterial[MatNum].TextureFormState[0] = 0;

	smMaterial[MatNum].smTexture[0]->TexSwapMode = TexSwap;
	smMaterial[MatNum].MapOpacity = FALSE;

	smMaterial[MatNum].TextureCounter = 1;			
	smMaterial[MatNum].Transparency = 0;			


	smMaterial[MatNum].UseState = 0;
	smMaterial[MatNum].TextureType = SMTEX_TYPE_MULTIMIX;
	smMaterial[MatNum].BlendType = BlendType;


	smMaterial[MatNum].InUse++;

	return MatNum;
}


int	smMATERIAL_GROUP::AddTexture(int MatNum, char *szBitmap, char *szOpacity)
{
	int cnt;

	cnt = smMaterial[MatNum].TextureCounter;			

	if (szOpacity)
		smMaterial[MatNum].smTexture[cnt] = smTexture.Add(szBitmap, szOpacity);
	else
		smMaterial[MatNum].smTexture[cnt] = smTexture.Add(szBitmap);

	if (smMaterial[MatNum].smTexture[cnt])
		smMaterial[MatNum].TextureCounter++;

	return TRUE;
}


smMATERIAL *smMATERIAL_GROUP::FindNextMaterialFromTextureName()
{
	DWORD cnt, cnt2;

	char *szName;

	szName = szLastSearchName;


	
	for (cnt = LastSearchMaterial; cnt < MaterialCount; cnt++) {
		if (smMaterial[cnt].InUse) {

			for (cnt2 = 0; cnt2 < smMaterial[cnt].TextureCounter; cnt2++) {
				if (lstrcmpi(smMaterial[cnt].smTexture[cnt2]->Name, szName) == 0) {
					return	&smMaterial[cnt];
				}
			}

			for (cnt2 = 0; cnt2 < smMaterial[cnt].AnimTexCounter; cnt2++) {
				if (lstrcmpi(smMaterial[cnt].smAnimTexture[cnt2]->Name, szName) == 0) {
					return	&smMaterial[cnt];
				}
			}

		}
	}

	return NULL;
}


smMATERIAL *smMATERIAL_GROUP::FindMaterialFromTextureName(char *szName)
{
	LastSearchMaterial = 0;

	lstrcpy(szLastSearchName, szName);

	return FindNextMaterialFromTextureName();
}


int smMATERIAL_GROUP::SetMaterialAnimFrame(smMATERIAL *lpMaterial, int frame)
{
	int OldFrame;

	OldFrame = lpMaterial->AnimationFrame;
	lpMaterial->AnimationFrame = frame;

	return OldFrame;
}




int smMATERIAL_GROUP::ChangeMaterialToAnimation(smMATERIAL *lpMaterial, char **szBitmap, int NumTex, DWORD BlendType, int AutoAnimMode)
{
	int cnt;

	for (cnt = 0; cnt < NumTex; cnt++) {
		lpMaterial->smAnimTexture[cnt] = smTexture.Add(szBitmap[cnt]);
		lpMaterial->smAnimTexture[cnt]->TexSwapMode = FALSE;
	}


	lpMaterial->MapOpacity = FALSE;
	lpMaterial->TextureCounter = 1;
	lpMaterial->Transparency = 0;			

	lpMaterial->AnimTexCounter = NumTex;		
	lpMaterial->FrameMask = NumTex - 1;	
	lpMaterial->Shift_FrameSpeed = 6;			

	lpMaterial->UseState = 0;
	lpMaterial->TextureType = SMTEX_TYPE_ANIMATION;
	lpMaterial->BlendType = BlendType;

	
	if (AutoAnimMode)
		lpMaterial->AnimationFrame = SMTEX_AUTOANIMATION;
	else
		lpMaterial->AnimationFrame = 0;

	lpMaterial->InUse++;

	return TRUE;
}



int smMATERIAL_GROUP::CreateAnimationMaterial(char **szBitmap, int NumTex, DWORD BlendType, int AutoAnimMode)
{
	int cnt;

	smMATERIAL *lpMaterial;

	cnt = MaterialCount;
	MaterialCount++;

	lpMaterial = &smMaterial[cnt];

	lpMaterial->InUse = TRUE;
	lpMaterial->Shade = FALSE;
	lpMaterial->TwoSide = TRUE;

	ChangeMaterialToAnimation(lpMaterial, szBitmap, NumTex, BlendType, AutoAnimMode);

	return cnt;
}




int smMATERIAL_GROUP::IncTextureFrame()
{
	return TRUE;
}




int smMATERIAL_GROUP::CheckMatreialTextureSwap()
{
	
	
	return TRUE;
	

	DWORD cnt, cnt2;

	
	for (cnt = 0; cnt < MaterialCount; cnt++)
		if (smMaterial[cnt].InUse) {
			if (smMaterial[cnt].TextureType == SMTEX_TYPE_MULTIMIX) {
				for (cnt2 = 0; cnt2 < smMaterial[cnt].TextureCounter; cnt2++) {
					if (smMaterial[cnt].smTexture[cnt2] && smMaterial[cnt].smTexture[cnt2]->lpDDSSysMemory) {
						smMaterial[cnt].TextureSwap = TRUE;
						break;
					}
				}
			}

			if (smMaterial[cnt].TextureType == SMTEX_TYPE_ANIMATION) {
				for (cnt2 = 0; cnt2 < smMaterial[cnt].AnimTexCounter; cnt2++) {
					if (smMaterial[cnt].smAnimTexture[cnt2] && smMaterial[cnt].smAnimTexture[cnt2]->lpDDSSysMemory) {
						smMaterial[cnt].TextureSwap = TRUE;
						break;
					}
				}
			}

		}

	return TRUE;
}



int smMATERIAL_GROUP::ReSwapMaterial(smMATERIAL *lpMaterial)
{
	
	
	return TRUE;
	

	DWORD cnt;
	int result;

	lpMaterial->MatFrame = MaterialFrame;

	lpMaterial->TextureClip = FALSE;

	if (lpMaterial->TextureSwap) {
		result = 0;
		for (cnt = 0; cnt < lpMaterial->TextureCounter; cnt++) {
			result |= smTexture.SetLargeTexture(lpMaterial->smTexture[cnt]);
		}

		if (result) {
			lpMaterial->TextureClip = TRUE;
			return TRUE;
		}
	}

	return FALSE;
}




int smMATERIAL_GROUP::ReadTextures(int MipMap)
{
    if (g_IsReadTextures == 0)
        return 0;

    const std::uint32_t prewarmLimit = SummitAssetRegistry.Policy().scopedPrewarmLimit;
    if (prewarmLimit == 0)
        return 0;

    int ReadCount = 0;
    std::uint32_t AttemptCount = 0;
    for (DWORD cnt = 0; cnt < MaterialCount && AttemptCount < prewarmLimit; ++cnt)
    {
        if (!SummitPrewarmHasHeadroom())
            break;
        if (!smMaterial[cnt].InUse)
            continue;

        for (DWORD tcnt = 0;
             tcnt < smMaterial[cnt].TextureCounter && AttemptCount < prewarmLimit;
             ++tcnt)
        {
            smTEXTUREHANDLE *lpTexture = smMaterial[cnt].smTexture[tcnt];
            if (lpTexture && lpTexture->UseCounter && !lpTexture->lpD3DTexture)
            {
                if (!SummitPrewarmHasHeadroom())
                    break;
                const int index = SummitTextureHandleIndex(lpTexture);
                if (index >= 0 && SummitAssetRegistry.CanAttempt(static_cast<std::size_t>(index)) &&
                    !AssetLoadV3IsTexturePending(lpTexture))
                {
                    AssetLoadV3RequestTexture(lpTexture, MipMap, 2);
                    if (AssetLoadV3IsTexturePending(lpTexture))
                    {
                        ++AttemptCount;
                        ++ReadCount;
                    }
                }
            }
        }

        for (DWORD tcnt = 0;
             tcnt < smMaterial[cnt].AnimTexCounter && AttemptCount < prewarmLimit;
             ++tcnt)
        {
            smTEXTUREHANDLE *lpTexture = smMaterial[cnt].smAnimTexture[tcnt];
            if (lpTexture && lpTexture->UseCounter && !lpTexture->lpD3DTexture)
            {
                if (!SummitPrewarmHasHeadroom())
                    break;
                const int index = SummitTextureHandleIndex(lpTexture);
                if (index >= 0 && SummitAssetRegistry.CanAttempt(static_cast<std::size_t>(index)) &&
                    !AssetLoadV3IsTexturePending(lpTexture))
                {
                    AssetLoadV3RequestTexture(lpTexture, MipMap, 2);
                    if (AssetLoadV3IsTexturePending(lpTexture))
                    {
                        ++AttemptCount;
                        ++ReadCount;
                    }
                }
            }
        }
    }
    return ReadCount;
}

// Incremental stage prewarm used by the two-stage field cache. This keeps
// high-resolution map textures, but limits GPU decode/upload work to a tiny
// number of textures per game tick instead of a full material-group burst.
int AssetLoadV2PrewarmMaterialGroupStep(smMATERIAL_GROUP *group, DWORD ownerMask,
    int MipMap, std::uint32_t maxAttempts, int requestPriority)
{
    if (!group || g_IsReadTextures == 0)
        return 0;

    for (DWORD materialIndex = 0; materialIndex < group->MaterialCount; ++materialIndex)
    {
        smMATERIAL &material = group->smMaterial[materialIndex];
        if (!material.InUse)
            continue;
        for (DWORD textureIndex = 0; textureIndex < material.TextureCounter; ++textureIndex)
            if (material.smTexture[textureIndex])
                AssetLoadV2AddOwner(material.smTexture[textureIndex], ownerMask);
        for (DWORD textureIndex = 0; textureIndex < material.AnimTexCounter; ++textureIndex)
            if (material.smAnimTexture[textureIndex])
                AssetLoadV2AddOwner(material.smAnimTexture[textureIndex], ownerMask);
    }

    std::uint32_t attempts = 0;
    int remaining = 0;
    for (DWORD materialIndex = 0; materialIndex < group->MaterialCount; ++materialIndex)
    {
        smMATERIAL &material = group->smMaterial[materialIndex];
        if (!material.InUse)
            continue;

        for (DWORD textureIndex = 0; textureIndex < material.TextureCounter; ++textureIndex)
        {
            smTEXTUREHANDLE *texture = material.smTexture[textureIndex];
            if (!texture || !texture->UseCounter || texture->lpD3DTexture)
                continue;
            const int index = SummitTextureHandleIndex(texture);
            if (index < 0)
                continue;
            if (AssetLoadV3IsTexturePending(texture))
            {
                ++remaining;
                continue;
            }
            if (!SummitAssetRegistry.CanAttempt(static_cast<std::size_t>(index)))
                continue;
            if (attempts < maxAttempts && SummitPrewarmHasHeadroom())
            {
                AssetLoadV3RequestTexture(texture, MipMap, requestPriority);
                if (AssetLoadV3IsTexturePending(texture))
                    ++attempts;
            }
            if (!texture->lpD3DTexture &&
                (AssetLoadV3IsTexturePending(texture) ||
                 SummitAssetRegistry.CanAttempt(static_cast<std::size_t>(index))))
                ++remaining;
        }

        for (DWORD textureIndex = 0; textureIndex < material.AnimTexCounter; ++textureIndex)
        {
            smTEXTUREHANDLE *texture = material.smAnimTexture[textureIndex];
            if (!texture || !texture->UseCounter || texture->lpD3DTexture)
                continue;
            const int index = SummitTextureHandleIndex(texture);
            if (index < 0)
                continue;
            if (AssetLoadV3IsTexturePending(texture))
            {
                ++remaining;
                continue;
            }
            if (!SummitAssetRegistry.CanAttempt(static_cast<std::size_t>(index)))
                continue;
            if (attempts < maxAttempts && SummitPrewarmHasHeadroom())
            {
                AssetLoadV3RequestTexture(texture, MipMap, requestPriority);
                if (AssetLoadV3IsTexturePending(texture))
                    ++attempts;
            }
            if (!texture->lpD3DTexture &&
                (AssetLoadV3IsTexturePending(texture) ||
                 SummitAssetRegistry.CanAttempt(static_cast<std::size_t>(index))))
                ++remaining;
        }
    }
    return remaining;
}


















smMATERIAL *smMaterial;

smMATERIAL_GROUP	*smMaterialGroup;


int InitMaterial()
{
	
	smMaterialGroup = new smMATERIAL_GROUP(MATRIAL_MAX);
	smMaterial = smMaterialGroup->smMaterial;

	MaterialFrame = 0;

	return TRUE;
}


int CloseMaterial()
{
	
	delete smMaterialGroup;
	return TRUE;
}


int DeleteUnuseMaterial()
{
	return smMaterialGroup->DeleteUnuseMaterial();
}


int AddMaterial(ASE_MATERIAL *aseMaterial)
{
	return smMaterialGroup->AddMaterial(aseMaterial);
}


int CreateTextureMaterial(char *szBitmap, char *szOpacity, int TwoSide, int Shade, DWORD TexSwap, DWORD BlendType)
{
	return smMaterialGroup->CreateTextureMaterial(szBitmap, szOpacity, TwoSide, Shade, TexSwap, BlendType);
}


int	AddTexture(int MatNum, char *szBitmap, char *szOpacity)
{
	return smMaterialGroup->AddTexture(MatNum, szBitmap, szOpacity);
}


smMATERIAL *FindNextMaterialFromTextureName()
{
	return smMaterialGroup->FindNextMaterialFromTextureName();
}


smMATERIAL *FindMaterialFromTextureName(char *szName)
{
	return smMaterialGroup->FindMaterialFromTextureName(szName);
}


int SetMaterialAnimFrame(smMATERIAL *lpMaterial, int frame)
{
	return smMaterialGroup->SetMaterialAnimFrame(lpMaterial, frame);
}



int ChangeMaterialToAnimation(smMATERIAL *lpMaterial, char **szBitmap, int NumTex, DWORD BlendType, int AutoAnimMode)
{
	return smMaterialGroup->ChangeMaterialToAnimation(lpMaterial, szBitmap, NumTex, BlendType, AutoAnimMode);
}



int CreateAnimationMaterial(char **szBitmap, int NumTex, DWORD BlendType, int AutoAnimMode)
{
	return smMaterialGroup->CreateAnimationMaterial(szBitmap, NumTex, BlendType, AutoAnimMode);
}



int IncTextureFrame()
{
	MaterialFrame++;
	return smTexture.SwapCount++;
}


int CheckMatreialTextureSwap()
{
	return smMaterialGroup->CheckMatreialTextureSwap();
}



int ReSwapMaterial(smMATERIAL *lpMaterial)
{
	return smMaterialGroup->ReSwapMaterial(lpMaterial);
}



int InitTexture()
{
	smTexture.Init();

	return TRUE;
}

int CloseTexture()
{
	smTexture.Remove();

	return TRUE;
}

int ReadTextures()
{
	
	
	if (g_IsReadTextures == 0)
		return 0;
	

	int val;

	val = smTexture.ReadTexture();
	CheckMatreialTextureSwap();

	return val;
}


int RestoreTextures()
{
	return smTexture.Restore();
}













LPDIRECT3DTEXTURE9 GetTextureHandle(int num)
{
	if (num < 0 || num >= MAX_TEXTURE)
		return NULL;
	return AssetLoadV2GetResidentTexture(&smTexture.Texture[num]);
}

LPDIRECT3DTEXTURE9 GetTextureChildHandle(int num)
{
	if (num < 0 || num >= MAX_TEXTURE)
		return NULL;
	if (smTexture.Texture[num].TexChild)
		return AssetLoadV2GetResidentTexture(smTexture.Texture[num].TexChild);
	return NULL;
}



LPDIRECT3DTEXTURE9 *GetLastTextureSurface()
{
	return smTexture.lpLastTextureSurface;
}

LPDIRECT3DTEXTURE9 *GetLastTexture()
{
	return smTexture.lpLastTexture;
}


void smEnterTextureCriticalSection()
{
	EnterCriticalSection(&smTexture.cTextureSection);
}


void smLeaveTextureCriticalSection()
{
	LeaveCriticalSection(&smTexture.cTextureSection);
}


#define smTEXFILETYPE_BMP		0
#define smTEXFILETYPE_JPG		1
#define smTEXFILETYPE_TGA		2
#define smTEXFILETYPE_PNG		3

int GetTextureFileType(char *Filename)
{
	int len;
	int cnt;
	char *szFileExt = 0;
	len = lstrlen(Filename);
	for (cnt = len - 1; cnt >= 0; cnt--)
	{
		if (Filename[cnt] == '.')
		{
			szFileExt = Filename + cnt + 1;
			break;
		}
	}
	if (!szFileExt)
		return -1;
	if ((szFileExt[0] == 'j' || szFileExt[0] == 'J') &&
		(szFileExt[1] == 'p' || szFileExt[1] == 'P') &&
		(szFileExt[2] == 'g' || szFileExt[2] == 'G'))
		return smTEXFILETYPE_JPG;
	if ((szFileExt[0] == 'b' || szFileExt[0] == 'B') &&
		(szFileExt[1] == 'm' || szFileExt[1] == 'M') &&
		(szFileExt[2] == 'p' || szFileExt[2] == 'P'))
		return smTEXFILETYPE_BMP;
	if ((szFileExt[0] == 't' || szFileExt[0] == 'T') &&
		(szFileExt[1] == 'g' || szFileExt[1] == 'G') &&
		(szFileExt[2] == 'a' || szFileExt[2] == 'A'))
		return smTEXFILETYPE_TGA;
	if ((szFileExt[0] == 'p' || szFileExt[0] == 'P') &&
		(szFileExt[1] == 'n' || szFileExt[1] == 'N') &&
		(szFileExt[2] == 'g' || szFileExt[2] == 'G'))
		return smTEXFILETYPE_PNG;
	if ((szFileExt[0] == 's' || szFileExt[0] == 'S') &&
		(szFileExt[1] == 't' || szFileExt[1] == 'T') &&
		(szFileExt[2] == 'g' || szFileExt[2] == 'G'))
		return smTEXFILETYPE_JPG;
	if ((szFileExt[0] == 's' || szFileExt[0] == 'S') &&
		(szFileExt[1] == 't' || szFileExt[1] == 'T') &&
		(szFileExt[2] == 'a' || szFileExt[2] == 'A'))
		return smTEXFILETYPE_TGA;
	return -1;
}

BOOL DecryptBMP(char * pBuffer, unsigned int uBufferSize)
{
	if (uBufferSize < 14)
		return FALSE;

	if ((pBuffer[0] != 'B') || (pBuffer[1] != 'M'))
	{
		pBuffer[0] = 'B';
		pBuffer[1] = 'M';

		for (unsigned char c = 2; c < 14; c++)
			pBuffer[c] -= (c * c);
	}

	pBuffer[2] = 0;
	pBuffer[3] = 0;
	pBuffer[4] = 0;
	pBuffer[5] = 0;

	return TRUE;
}

BOOL DecryptTGA(char * pBuffer, unsigned int uBufferSize)
{
	if (uBufferSize < 18)
		return FALSE;

	if ((pBuffer[0] == 'G') && (pBuffer[1] == '8'))
	{
		pBuffer[0] = 0;
		pBuffer[1] = 0;

		for (unsigned char c = 2; c < 18; c++)
			pBuffer[c] -= (c * c);
	}

	return TRUE;
}

std::string GetFileExtension(const std::string & strFilePath)
{
	std::string::size_type stSlashIndex;
	std::string::size_type stBackslashIndex;
	std::string::size_type stDotIndex;

	stSlashIndex = strFilePath.rfind('/');
	stBackslashIndex = strFilePath.rfind('\\');
	stDotIndex = strFilePath.rfind('.');

	if (stDotIndex != std::string::npos)
	{
		if (stSlashIndex != std::string::npos)
		{
			if (stSlashIndex > stDotIndex)
				return "";
		}

		if (stBackslashIndex != std::string::npos)
		{
			if (stBackslashIndex > stDotIndex)
				return "";
		}

		char szFileExtension[MAX_PATH] = { 0 };
		lstrcpy(szFileExtension, strFilePath.substr(stDotIndex + 1).c_str());

		for (int i = 0, j = lstrlen(szFileExtension); i < j; i++)
			szFileExtension[i] = tolower(szFileExtension[i]);

		return szFileExtension;
	}

	return "";
}


namespace
{

enum SummitTexturePrepareResult
{
    SUMMIT_PREPARE_OK = 0,
    SUMMIT_PREPARE_FILE_NOT_FOUND,
    SUMMIT_PREPARE_OPEN_FAILED,
    SUMMIT_PREPARE_EMPTY_FILE,
    SUMMIT_PREPARE_READ_FAILED,
    SUMMIT_PREPARE_INVALID_HEADER,
    SUMMIT_PREPARE_INVALID_IMAGE_INFO,
    SUMMIT_PREPARE_INVALID_DIMENSIONS,
    SUMMIT_PREPARE_STAGING_BUDGET
};

struct SummitPreparedTexture
{
    SummitPreparedTexture()
        : index(-1), generation(0), mipMap(0), result(SUMMIT_PREPARE_OPEN_FAILED),
          mapOpacity(FALSE), stagingBytes(0)
    {
        ZeroMemory(&imageInfo, sizeof(imageInfo));
        target.width = 0;
        target.height = 0;
        target.downscaled = false;
    }

    ~SummitPreparedTexture()
    {
        if (stagingBytes)
            SummitTextureStagingBytes.fetch_sub(stagingBytes, std::memory_order_acq_rel);
    }

    int index;
    std::uint32_t generation;
    int mipMap;
    SummitTexturePrepareResult result;
    std::string path;
    std::vector<char> bytes;
    D3DXIMAGE_INFO imageInfo;
    summit::rendering::assets::v2::AssetTargetDimensions target;
    BOOL mapOpacity;
    std::uint64_t stagingBytes;
};

static std::mutex SummitPreparedTextureMutex;
static std::deque<std::shared_ptr<SummitPreparedTexture> > SummitPreparedTextureQueue;

static std::uint32_t SummitParseBoundedEnv(const char *name, std::uint32_t fallback,
    std::uint32_t minimum, std::uint32_t maximum)
{
    const char *value = getenv(name);
    if (!value || !*value)
        return fallback;
    char *end = NULL;
    const unsigned long parsed = strtoul(value, &end, 10);
    if (!end || end == value || *end != 0)
        return fallback;
    std::uint32_t result = static_cast<std::uint32_t>(parsed);
    if (result < minimum) result = minimum;
    if (result > maximum) result = maximum;
    return result;
}

static std::uint32_t SummitTextureMaxInFlight()
{
    static const std::uint32_t configured = []() -> std::uint32_t
    {
        const std::size_t workers = summit::core::concurrency::JobSystem::Instance().WorkerCount();
        const std::uint32_t automatic = static_cast<std::uint32_t>(workers > 0 ? workers : 2u);
        return SummitParseBoundedEnv("SUMMIT_TEXTURE_IO_INFLIGHT",
            automatic > 6u ? 6u : automatic, 1u, 16u);
    }();
    return configured;
}

static std::uint64_t SummitTextureStagingBudgetBytes()
{
    static const std::uint64_t configured = []() -> std::uint64_t
    {
        const char *value = getenv("SUMMIT_TEXTURE_STAGING_BUDGET_MB");
        unsigned long mb = 128ul;
        if (value && *value)
        {
            const unsigned long parsed = strtoul(value, 0, 10);
            if (parsed >= 32ul && parsed <= 512ul)
                mb = parsed;
        }
        return static_cast<std::uint64_t>(mb) * 1024ull * 1024ull;
    }();
    return configured;
}

static bool SummitReserveTextureStaging(std::uint64_t bytes)
{
    const std::uint64_t budget = SummitTextureStagingBudgetBytes();
    std::uint64_t current = SummitTextureStagingBytes.load(std::memory_order_acquire);
    while (current + bytes <= budget)
    {
        if (SummitTextureStagingBytes.compare_exchange_weak(
            current, current + bytes, std::memory_order_acq_rel, std::memory_order_acquire))
            return true;
    }
    return false;
}

static std::uint32_t SummitTextureUploadsPerFrame()
{
    // Explicit user tuning always wins.  Otherwise bootstrap gets a wider
    // completion burst so the first world does not visibly fill in at two
    // textures per frame.  Gameplay remains deliberately conservative.
    const char *value = getenv("SUMMIT_TEXTURE_UPLOADS_PER_FRAME");
    if (value && *value)
        return SummitParseBoundedEnv("SUMMIT_TEXTURE_UPLOADS_PER_FRAME", 2u, 1u, 8u);
    return SummitTextureLoadingBoost.load(std::memory_order_acquire) ? 8u : 2u;
}

static std::uint32_t SummitTextureUploadBudgetMicros()
{
    const char *value = getenv("SUMMIT_TEXTURE_UPLOAD_BUDGET_US");
    if (value && *value)
    {
        const unsigned long parsed = strtoul(value, 0, 10);
        if (parsed < 250ul) return 250u;
        if (parsed > 12000ul) return 12000u;
        return static_cast<std::uint32_t>(parsed);
    }
    return SummitTextureLoadingBoost.load(std::memory_order_acquire) ? 10000u : 1800u;
}

static void SummitQueuePreparedTexture(const std::shared_ptr<SummitPreparedTexture> &prepared)
{
    if (!prepared)
        return;
    if (prepared->index < 0 || prepared->index >= MAX_TEXTURE)
        return;

    const std::uint32_t currentGeneration = SummitTextureAsyncGenerations[prepared->index].load(std::memory_order_acquire);
    if (currentGeneration != prepared->generation)
    {
        SummitTextureAsyncInFlight.fetch_sub(1u, std::memory_order_acq_rel);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(SummitPreparedTextureMutex);
        SummitPreparedTextureQueue.push_back(prepared);
    }
    SummitTextureAsyncStates[prepared->index].store(SUMMIT_TEX_ASYNC_READY, std::memory_order_release);
}

static void SummitPrepareTextureOnWorker(const std::shared_ptr<SummitPreparedTexture> &prepared,
    summit::rendering::assets::v2::AssetLoadPolicy policy, unsigned qualityShift)
{
    if (!prepared)
        return;

    FILE *file = NULL;
    fopen_s(&file, prepared->path.c_str(), "rb");
    if (!file)
    {
        prepared->result = SUMMIT_PREPARE_FILE_NOT_FOUND;
        SummitQueuePreparedTexture(prepared);
        return;
    }

    fseek(file, 0, SEEK_END);
    const long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (fileSize <= 0)
    {
        fclose(file);
        prepared->result = SUMMIT_PREPARE_EMPTY_FILE;
        SummitQueuePreparedTexture(prepared);
        return;
    }

    if (!SummitReserveTextureStaging(static_cast<std::uint64_t>(fileSize)))
    {
        fclose(file);
        prepared->result = SUMMIT_PREPARE_STAGING_BUDGET;
        SummitQueuePreparedTexture(prepared);
        return;
    }
    prepared->stagingBytes = static_cast<std::uint64_t>(fileSize);

    try
    {
        prepared->bytes.resize(static_cast<std::size_t>(fileSize));
    }
    catch (...)
    {
        fclose(file);
        prepared->result = SUMMIT_PREPARE_OPEN_FAILED;
        SummitQueuePreparedTexture(prepared);
        return;
    }

    const size_t readCount = fread(prepared->bytes.data(), static_cast<std::size_t>(fileSize), 1, file);
    fclose(file);
    if (readCount != 1)
    {
        prepared->bytes.clear();
        prepared->result = SUMMIT_PREPARE_READ_FAILED;
        SummitQueuePreparedTexture(prepared);
        return;
    }

    const int fileType = GetTextureFileType(const_cast<char *>(prepared->path.c_str()));
    if (fileType == smTEXFILETYPE_TGA)
    {
        if (!DecryptTGA(prepared->bytes.data(), static_cast<unsigned int>(prepared->bytes.size())))
        {
            prepared->result = SUMMIT_PREPARE_INVALID_HEADER;
            SummitQueuePreparedTexture(prepared);
            return;
        }
        prepared->mapOpacity = TRUE;
    }
    else if (fileType == smTEXFILETYPE_BMP)
    {
        if (!DecryptBMP(prepared->bytes.data(), static_cast<unsigned int>(prepared->bytes.size())))
        {
            prepared->result = SUMMIT_PREPARE_INVALID_HEADER;
            SummitQueuePreparedTexture(prepared);
            return;
        }
        prepared->mapOpacity = FALSE;
    }
    else if (fileType == smTEXFILETYPE_PNG)
    {
        prepared->mapOpacity = TRUE;
    }
    else
    {
        prepared->mapOpacity = FALSE;
    }

    if (FAILED(D3DXGetImageInfoFromFileInMemory(
        prepared->bytes.data(), static_cast<UINT>(prepared->bytes.size()), &prepared->imageInfo)))
    {
        prepared->result = SUMMIT_PREPARE_INVALID_IMAGE_INFO;
        SummitQueuePreparedTexture(prepared);
        return;
    }

    if (!summit::rendering::assets::v2::IsStructurallyValidSource(
        prepared->imageInfo.Width, prepared->imageInfo.Height, policy))
    {
        prepared->result = SUMMIT_PREPARE_INVALID_DIMENSIONS;
        SummitQueuePreparedTexture(prepared);
        return;
    }

    prepared->target = summit::rendering::assets::v2::ComputeTargetDimensions(
        prepared->imageInfo.Width, prepared->imageInfo.Height, policy, qualityShift, true);
    prepared->result = SUMMIT_PREPARE_OK;
    SummitQueuePreparedTexture(prepared);
}

static void SummitCompletePreparedFailure(smTEXTUREHANDLE *handle,
    const std::shared_ptr<SummitPreparedTexture> &prepared)
{
    if (!handle || !prepared)
        return;

    switch (prepared->result)
    {
    case SUMMIT_PREPARE_FILE_NOT_FOUND:
    {
        const bool firstMissing = SummitMissingTexturePaths.insert(prepared->path).second;
        if (firstMissing && SummitAssetTraceEnabled())
        {
            FILE *f = NULL;
            fopen_s(&f, "LOGS\\mingw_missing_texture_paths.txt", "a");
            if (f)
            {
                fprintf(f, "%s\n", prepared->path.c_str());
                fclose(f);
            }
        }
        SummitTextureMarkPermanentFailure(handle, "FILE_NOT_FOUND");
        break;
    }
    case SUMMIT_PREPARE_EMPTY_FILE:
        SummitTextureMarkPermanentFailure(handle, "EMPTY_FILE");
        break;
    case SUMMIT_PREPARE_INVALID_HEADER:
        SummitTextureMarkPermanentFailure(handle, "INVALID_IMAGE_HEADER");
        break;
    case SUMMIT_PREPARE_INVALID_IMAGE_INFO:
        SummitTextureMarkPermanentFailure(handle, "INVALID_IMAGE_INFO");
        break;
    case SUMMIT_PREPARE_INVALID_DIMENSIONS:
        SummitTextureMarkPermanentFailure(handle, "IMPLAUSIBLE_DIMENSIONS",
            prepared->imageInfo.Width, prepared->imageInfo.Height);
        break;
    case SUMMIT_PREPARE_STAGING_BUDGET:
    case SUMMIT_PREPARE_READ_FAILED:
    case SUMMIT_PREPARE_OPEN_FAILED:
    default:
    {
        const int index = SummitTextureHandleIndex(handle);
        if (index >= 0)
            SummitAssetRegistry.MarkTransientFailure(static_cast<std::size_t>(index));
        break;
    }
    }
}

static void SummitCommitPreparedTexture(const std::shared_ptr<SummitPreparedTexture> &prepared)
{
    if (!prepared || prepared->index < 0 || prepared->index >= MAX_TEXTURE)
        return;

    const int index = prepared->index;
    if (SummitTextureAsyncGenerations[index].load(std::memory_order_acquire) != prepared->generation)
        return;

    smTEXTUREHANDLE *handle = &smTexture.Texture[index];
    if (handle->UseCounter <= 0 || _stricmp(handle->Name, prepared->path.c_str()) != 0)
        return;

    if (prepared->result != SUMMIT_PREPARE_OK)
    {
        SummitCompletePreparedFailure(handle, prepared);
        return;
    }

    if (!d3ddev)
    {
        // Keep the prepared bytes queued until the device is available. The
        // caller only pumps uploads when D3D is alive, so this is defensive.
        SummitTextureMarkWaitingForDevice(handle, prepared->imageInfo.Width, prepared->imageInfo.Height);
        return;
    }

    handle->Width = prepared->imageInfo.Width;
    handle->Height = prepared->imageInfo.Height;
    handle->MapOpacity = prepared->mapOpacity;

    summit::rendering::assets::v2::AssetTargetDimensions target = prepared->target;
    summit::rendering::assets::v2::AssetLoadPolicy runtimePolicy = SummitAssetRegistry.Policy();

    D3DCAPS9 caps;
    ZeroMemory(&caps, sizeof(caps));
    if (SUCCEEDED(d3ddev->GetDeviceCaps(&caps)))
    {
        const UINT deviceMax = caps.MaxTextureWidth < caps.MaxTextureHeight ? caps.MaxTextureWidth : caps.MaxTextureHeight;
        if (deviceMax && (target.width > deviceMax || target.height > deviceMax))
        {
            const double sx = static_cast<double>(deviceMax) / static_cast<double>(target.width);
            const double sy = static_cast<double>(deviceMax) / static_cast<double>(target.height);
            const double scale = sx < sy ? sx : sy;
            target.width = summit::rendering::assets::v2::AlignDown4(static_cast<std::uint32_t>(target.width * scale));
            target.height = summit::rendering::assets::v2::AlignDown4(static_cast<std::uint32_t>(target.height * scale));
            target.downscaled = true;
        }
    }

    D3DXIMAGE_INFO residentInfo = prepared->imageInfo;
    residentInfo.Width = target.width;
    residentInfo.Height = target.height;
    D3DFORMAT preferredFormat = SummitChooseLargeTextureFormat(handle, residentInfo);
    ULONGLONG estimatedBytes = SummitEstimateSurfaceBytes(
        target.width, target.height,
        preferredFormat == D3DFMT_FROM_FILE ? D3DFMT_A8R8G8B8 : preferredFormat);

    while (!SummitAssetEnsureBudget(estimatedBytes, static_cast<std::size_t>(index)) &&
           (target.width > 512u || target.height > 512u))
    {
        target.width = target.width > 512u ? summit::rendering::assets::v2::AlignDown4(target.width / 2u) : target.width;
        target.height = target.height > 512u ? summit::rendering::assets::v2::AlignDown4(target.height / 2u) : target.height;
        residentInfo.Width = target.width;
        residentInfo.Height = target.height;
        preferredFormat = SummitChooseLargeTextureFormat(handle, residentInfo);
        estimatedBytes = SummitEstimateSurfaceBytes(
            target.width, target.height,
            preferredFormat == D3DFMT_FROM_FILE ? D3DFMT_A8R8G8B8 : preferredFormat);
        target.downscaled = true;
    }

    if (!SummitAssetEnsureBudget(estimatedBytes, static_cast<std::size_t>(index)))
    {
        SummitTextureRecordCreateFailure(handle, E_OUTOFMEMORY,
            prepared->imageInfo.Width, prepared->imageInfo.Height);
        return;
    }

    HRESULT textureHr = E_FAIL;
    if (preferredFormat != D3DFMT_FROM_FILE)
    {
        textureHr = D3DXCreateTextureFromFileInMemoryEx(
            d3ddev,
            prepared->bytes.data(),
            static_cast<UINT>(prepared->bytes.size()),
            target.width,
            target.height,
            1,
            0,
            preferredFormat,
            D3DPOOL_MANAGED,
            D3DX_FILTER_POINT,
            D3DX_DEFAULT,
            0xFFF0FFFF,
            &prepared->imageInfo,
            NULL,
            &handle->lpD3DTexture);
        SummitLogLargeTexturePolicy(handle->Name, residentInfo, preferredFormat, textureHr);
    }

    if (preferredFormat == D3DFMT_FROM_FILE || FAILED(textureHr))
    {
        if (handle->lpD3DTexture)
        {
            handle->lpD3DTexture->Release();
            handle->lpD3DTexture = NULL;
        }

        ULONGLONG fallbackBytes = SummitEstimateSurfaceBytes(target.width, target.height, D3DFMT_A8R8G8B8);
        while (!SummitAssetEnsureBudget(fallbackBytes, static_cast<std::size_t>(index)) &&
               (target.width > 512u || target.height > 512u))
        {
            target.width = target.width > 512u ? summit::rendering::assets::v2::AlignDown4(target.width / 2u) : target.width;
            target.height = target.height > 512u ? summit::rendering::assets::v2::AlignDown4(target.height / 2u) : target.height;
            fallbackBytes = SummitEstimateSurfaceBytes(target.width, target.height, D3DFMT_A8R8G8B8);
        }

        if (SummitAssetEnsureBudget(fallbackBytes, static_cast<std::size_t>(index)))
        {
            textureHr = D3DXCreateTextureFromFileInMemoryEx(
                d3ddev,
                prepared->bytes.data(),
                static_cast<UINT>(prepared->bytes.size()),
                target.width,
                target.height,
                D3DX_FROM_FILE,
                0,
                D3DFMT_FROM_FILE,
                D3DPOOL_MANAGED,
                D3DX_FILTER_POINT,
                D3DX_DEFAULT,
                0xFFF0FFFF,
                &prepared->imageInfo,
                NULL,
                &handle->lpD3DTexture);
        }
        else
        {
            textureHr = E_OUTOFMEMORY;
        }
    }

    if (FAILED(textureHr) || !handle->lpD3DTexture)
    {
        handle->lpD3DTexture = NULL;
        SummitTextureRecordCreateFailure(handle, textureHr,
            prepared->imageInfo.Width, prepared->imageInfo.Height);
        return;
    }

    SummitTextureRecordCreateSuccess(handle, prepared->imageInfo.Width, prepared->imageInfo.Height);
}

} // anonymous namespace

void AssetLoadV3ResetAsyncState()
{
    SummitTextureLoadingBoost.store(false, std::memory_order_release);
    // Must be called after JobSystem::WaitIdle() during a session teardown.
    // No worker can append after this point, so dropping the completion queue
    // releases all staging buffers without racing CPU preparation.
    std::deque<std::shared_ptr<SummitPreparedTexture> > abandoned;
    {
        std::lock_guard<std::mutex> lock(SummitPreparedTextureMutex);
        abandoned.swap(SummitPreparedTextureQueue);
    }

    for (int index = 0; index < MAX_TEXTURE; ++index)
    {
        SummitTextureAsyncGenerations[index].fetch_add(1u, std::memory_order_acq_rel);
        SummitTextureAsyncStates[index].store(SUMMIT_TEX_ASYNC_IDLE, std::memory_order_release);
    }
    SummitTextureAsyncInFlight.store(0u, std::memory_order_release);

    // Destruction of abandoned prepared objects releases their accounted bytes.
    abandoned.clear();
    SummitTextureStagingBytes.store(0u, std::memory_order_release);
}

void AssetLoadV3SetLoadingBoost(bool enabled)
{
    SummitTextureLoadingBoost.store(enabled, std::memory_order_release);
}

bool AssetLoadV3IsTexturePending(smTEXTUREHANDLE *lpTexHandle)
{
    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index < 0)
        return false;
    return SummitTextureAsyncStates[index].load(std::memory_order_acquire) != SUMMIT_TEX_ASYNC_IDLE;
}

int AssetLoadV3RequestTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap, int priority)
{
    if (!lpTexHandle || lpTexHandle->UseCounter <= 0)
        return FALSE;

    const int index = SummitTextureHandleIndex(lpTexHandle);
    if (index < 0)
        return FALSE;
    if (lpTexHandle->lpD3DTexture)
        return TRUE;

    SummitAssetRegistry.Register(static_cast<std::size_t>(index), summit::rendering::assets::v2::ASSET_OWNER_GLOBAL);
    if (!SummitAssetRegistry.CanAttempt(static_cast<std::size_t>(index)))
        return FALSE;

    if (SummitTextureAsyncInFlight.load(std::memory_order_acquire) >= SummitTextureMaxInFlight())
        return FALSE;

    unsigned char expected = SUMMIT_TEX_ASYNC_IDLE;
    if (!SummitTextureAsyncStates[index].compare_exchange_strong(
        expected, SUMMIT_TEX_ASYNC_PREPARING, std::memory_order_acq_rel))
        return FALSE;

    SummitTextureAsyncInFlight.fetch_add(1u, std::memory_order_acq_rel);
    SummitAssetRegistry.BeginLoad(static_cast<std::size_t>(index));

    std::shared_ptr<SummitPreparedTexture> prepared(new SummitPreparedTexture());
    prepared->index = index;
    prepared->generation = SummitTextureAsyncGenerations[index].load(std::memory_order_acquire);
    prepared->mipMap = MipMap;
    prepared->path = lpTexHandle->Name;

    const summit::rendering::assets::v2::AssetLoadPolicy policy = SummitAssetRegistry.Policy();
    unsigned qualityShift = TexturHalfSize > 0 ? static_cast<unsigned>(TexturHalfSize) : 0u;
    if (qualityShift > 2u) qualityShift = 2u;

    summit::core::concurrency::JobPriority jobPriority = summit::core::concurrency::JobPriority::Normal;
    if (priority <= 0) jobPriority = summit::core::concurrency::JobPriority::High;
    else if (priority >= 2) jobPriority = summit::core::concurrency::JobPriority::Low;

    if (!summit::core::concurrency::JobSystem::Instance().Submit(
        [prepared, policy, qualityShift]() {
            SummitPrepareTextureOnWorker(prepared, policy, qualityShift);
        },
        jobPriority))
    {
        SummitTextureAsyncStates[index].store(SUMMIT_TEX_ASYNC_IDLE, std::memory_order_release);
        SummitTextureAsyncInFlight.fetch_sub(1u, std::memory_order_acq_rel);
        SummitAssetRegistry.MarkTransientFailure(static_cast<std::size_t>(index));
        return FALSE;
    }

    return FALSE;
}

void AssetLoadV3PumpTextureUploads()
{
    if (!d3ddev)
        return;

    const std::uint32_t uploadLimit = SummitTextureUploadsPerFrame();
    const std::uint32_t budgetMicros = SummitTextureUploadBudgetMicros();
    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (std::uint32_t uploaded = 0; uploaded < uploadLimit; ++uploaded)
    {
        if (uploaded > 0)
        {
            const std::uint64_t elapsed = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now() - begin).count());
            if (elapsed >= budgetMicros)
                break;
        }

        std::shared_ptr<SummitPreparedTexture> prepared;
        {
            std::lock_guard<std::mutex> lock(SummitPreparedTextureMutex);
            if (SummitPreparedTextureQueue.empty())
                break;
            prepared = SummitPreparedTextureQueue.front();
            SummitPreparedTextureQueue.pop_front();
        }

        if (!prepared)
            continue;

        const int index = prepared->index;
        const bool generationMatches = index >= 0 && index < MAX_TEXTURE &&
            SummitTextureAsyncGenerations[index].load(std::memory_order_acquire) == prepared->generation;

        if (generationMatches)
            SummitCommitPreparedTexture(prepared);

        if (generationMatches)
            SummitTextureAsyncStates[index].store(SUMMIT_TEX_ASYNC_IDLE, std::memory_order_release);
        SummitTextureAsyncInFlight.fetch_sub(1u, std::memory_order_acq_rel);
    }
}


LPDIRECT3DTEXTURE9 LoadDibSurfaceOffscreenWithInfo(char *Filename, UINT *pSourceWidth, UINT *pSourceHeight)
{
    if (pSourceWidth) *pSourceWidth = 0;
    if (pSourceHeight) *pSourceHeight = 0;
    if (!Filename || !PathFileExists(Filename))
        return NULL;

    FILE *pFile = NULL;
    fopen_s(&pFile, Filename, "rb");
    if (!pFile)
        return NULL;

    fseek(pFile, 0, SEEK_END);
    const int iSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    if (iSize <= 0)
    {
        fclose(pFile);
        SummitLogTextureReject(Filename, "OFFSCREEN_EMPTY_FILE", 0, 0);
        return NULL;
    }

    char *pBuffer = new (std::nothrow) char[iSize];
    if (!pBuffer)
    {
        fclose(pFile);
        SummitLogTextureReject(Filename, "OFFSCREEN_DECODE_BUFFER_OOM", 0, 0);
        return NULL;
    }
    const size_t readCount = fread(pBuffer, iSize, 1, pFile);
    fclose(pFile);
    if (readCount != 1)
    {
        delete[] pBuffer;
        SummitLogTextureReject(Filename, "OFFSCREEN_READ_FAILED", 0, 0);
        return NULL;
    }

    if (GetFileExtension(Filename).compare("bmp") == 0 && !DecryptBMP(pBuffer, iSize))
    {
        delete[] pBuffer;
        SummitLogTextureReject(Filename, "OFFSCREEN_INVALID_BMP", 0, 0);
        return NULL;
    }

    D3DXIMAGE_INFO imageInfo;
    ZeroMemory(&imageInfo, sizeof(imageInfo));
    if (FAILED(D3DXGetImageInfoFromFileInMemory(pBuffer, iSize, &imageInfo)))
    {
        delete[] pBuffer;
        SummitLogTextureReject(Filename, "OFFSCREEN_INVALID_IMAGE_INFO", 0, 0);
        return NULL;
    }

    // Keep the file dimensions separate from the resident GPU dimensions.
    // UI assets are authored/upscaled at higher resolution and can still be
    // runtime-downscaled by the asset policy (for example 2240 -> 2048).
    if (pSourceWidth) *pSourceWidth = imageInfo.Width;
    if (pSourceHeight) *pSourceHeight = imageInfo.Height;

    using namespace summit::rendering::assets::v2;
    AssetLoadPolicy runtimePolicy = SummitAssetRegistry.Policy();
    if (!IsStructurallyValidSource(imageInfo.Width, imageInfo.Height, runtimePolicy))
    {
        SummitLogTextureReject(Filename, "OFFSCREEN_IMPLAUSIBLE_DIMENSIONS", imageInfo.Width, imageInfo.Height);
        delete[] pBuffer;
        return NULL;
    }

    runtimePolicy.maxRuntimeDimension = runtimePolicy.maxExternalRuntimeDimension;
    runtimePolicy.maxRuntimePixels = runtimePolicy.maxExternalRuntimePixels;

    D3DCAPS9 caps;
    ZeroMemory(&caps, sizeof(caps));
    if (d3ddev && SUCCEEDED(d3ddev->GetDeviceCaps(&caps)))
    {
        const UINT deviceMax = caps.MaxTextureWidth < caps.MaxTextureHeight ? caps.MaxTextureWidth : caps.MaxTextureHeight;
        if (deviceMax && runtimePolicy.maxRuntimeDimension > deviceMax)
        {
            runtimePolicy.maxRuntimeDimension = deviceMax;
            runtimePolicy.maxRuntimePixels = static_cast<ULONGLONG>(deviceMax) * deviceMax;
        }
    }

    unsigned qualityShift = TexturHalfSize > 0 ? static_cast<unsigned>(TexturHalfSize) : 0u;
    if (qualityShift > 2u) qualityShift = 2u;
    const AssetTargetDimensions target = ComputeTargetDimensions(
        imageInfo.Width, imageInfo.Height, runtimePolicy, qualityShift, false);

    if (target.downscaled)
    {
        char detail[96] = { 0 };
        wsprintf(detail, "OFFSCREEN_DOWNSCALE_%ux%u_TO_%ux%u", imageInfo.Width, imageInfo.Height, target.width, target.height);
        SummitLogAssetEvent("ADAPT", NULL, 0, detail);
    }

    LPDIRECT3DTEXTURE9 image = NULL;
    const int externalFileType = GetTextureFileType(Filename);
    D3DFORMAT externalFormat = D3DFMT_FROM_FILE;
    const ULONGLONG targetRgbaBytes =
        static_cast<ULONGLONG>(target.width) * static_cast<ULONGLONG>(target.height) * 4ULL;
    if (target.width >= 4u && target.height >= 4u &&
        (target.width & 3u) == 0 && (target.height & 3u) == 0 &&
        targetRgbaBytes >= runtimePolicy.largeTextureRgbaBytes)
    {
        externalFormat = (externalFileType == smTEXFILETYPE_JPG) ? D3DFMT_DXT1 : D3DFMT_DXT5;
    }

    const D3DCOLOR colorKey = imageInfo.ImageFileFormat == D3DXIFF_BMP ? D3DCOLOR_ARGB(255, 0, 0, 0) : 0;
    HRESULT hr = E_FAIL;
    if (externalFormat != D3DFMT_FROM_FILE)
    {
        hr = D3DXCreateTextureFromFileInMemoryEx(
            d3ddev, pBuffer, iSize, target.width, target.height, 1, 0,
            externalFormat, D3DPOOL_MANAGED, D3DX_FILTER_POINT, D3DX_DEFAULT,
            colorKey, &imageInfo, NULL, &image);
        SummitLogLargeTexturePolicy(Filename, imageInfo, externalFormat, hr);
    }

    if (externalFormat == D3DFMT_FROM_FILE || FAILED(hr))
    {
        if (image)
        {
            image->Release();
            image = NULL;
        }
        hr = D3DXCreateTextureFromFileInMemoryEx(
            d3ddev, pBuffer, iSize, target.width, target.height, 1, 0,
            D3DFMT_FROM_FILE, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
            colorKey, &imageInfo, NULL, &image);
    }

    if (SUCCEEDED(hr) && image)
    {
        UINT residentWidth = 0;
        UINT residentHeight = 0;
        const ULONGLONG residentBytes = SummitEstimateTextureBytes(image, &residentWidth, &residentHeight);
        char detail[128] = { 0 };
        wsprintf(detail, "EXTERNAL_%ux%u", residentWidth, residentHeight);
        SummitLogAssetEvent("EXTERNAL_LOAD", NULL, residentBytes, detail);
    }

    delete[] pBuffer;
    return SUCCEEDED(hr) ? image : NULL;
}

LPDIRECT3DTEXTURE9 LoadDibSurfaceOffscreen(char *Filename)
{
    return LoadDibSurfaceOffscreenWithInfo(Filename, NULL, NULL);
}


int new_smCreateTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap)
{
    if (!lpTexHandle)
        return -3;

    const int assetIndex = SummitTextureHandleIndex(lpTexHandle);
    if (assetIndex < 0)
        return -3;

    if (!d3ddev)
    {
        SummitTextureMarkWaitingForDevice(lpTexHandle);
        return 0;
    }

    const std::string texturePath = lpTexHandle->Name;
    if (SummitMissingTexturePaths.find(texturePath) != SummitMissingTexturePaths.end())
    {
        SummitTextureMarkPermanentFailure(lpTexHandle, "FILE_NOT_FOUND_CACHED");
        return -1;
    }

    if (!PathFileExists(lpTexHandle->Name))
    {
        const bool firstMissing = SummitMissingTexturePaths.insert(texturePath).second;
        if (firstMissing)
        {
            FILE *f = NULL;
            fopen_s(&f, "LOGS\\mingw_missing_texture_paths.txt", "a");
            if (f)
            {
                fprintf(f, "%s\n", lpTexHandle->Name);
                fclose(f);
            }
        }
        SummitTextureMarkPermanentFailure(lpTexHandle, "FILE_NOT_FOUND");
        return -1;
    }

    FILE *pFile = NULL;
    fopen_s(&pFile, lpTexHandle->Name, "rb");
    if (!pFile)
    {
        const bool permanent = SummitAssetRegistry.MarkTransientFailure(static_cast<std::size_t>(assetIndex));
        if (permanent)
            SummitLogTextureReject(lpTexHandle->Name, "OPEN_FAILED_RETRY_LIMIT", 0, 0);
        return 0;
    }

    fseek(pFile, 0, SEEK_END);
    const int iSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    if (iSize <= 0)
    {
        fclose(pFile);
        SummitTextureMarkPermanentFailure(lpTexHandle, "EMPTY_FILE");
        return -3;
    }

    char *pBuffer = new (std::nothrow) char[iSize];
    if (!pBuffer)
    {
        fclose(pFile);
        const bool permanent = SummitAssetRegistry.MarkTransientFailure(static_cast<std::size_t>(assetIndex));
        if (permanent)
            SummitLogTextureReject(lpTexHandle->Name, "DECODE_BUFFER_OOM_RETRY_LIMIT", 0, 0);
        return 0;
    }
    const size_t readCount = fread(pBuffer, iSize, 1, pFile);
    fclose(pFile);
    if (readCount != 1)
    {
        delete[] pBuffer;
        const bool permanent = SummitAssetRegistry.MarkTransientFailure(static_cast<std::size_t>(assetIndex));
        if (permanent)
            SummitLogTextureReject(lpTexHandle->Name, "READ_FAILED_RETRY_LIMIT", 0, 0);
        return 0;
    }

    // Preserve the authored mip lookup behavior; D3DX is still responsible for
    // actual mip generation/import. The assignment bug in the legacy gate is fixed.
    UINT uMipLevels = 0;
    if (MipMap == true)
    {
        char szFilePath[MAX_PATH] = { 0 };
        lstrcpy(szFilePath, lpTexHandle->Name);
        const char *iDot = strrchr(szFilePath, '.');
        if (iDot != NULL)
        {
            char szBaseFile[MAX_PATH] = { 0 };
            char szFileExtension[MAX_PATH] = { 0 };
            const int iLenToDot = static_cast<int>(iDot - szFilePath);
            lstrcpy(szBaseFile, szFilePath);
            szBaseFile[iLenToDot] = 0;
            lstrcpy(szFileExtension, iDot + 1);
            for (int i = 1; i <= 4; ++i)
            {
                char szBuffer[MAX_PATH] = { 0 };
                wsprintf(szBuffer, "%s_mm%d.%s", szBaseFile, i, szFileExtension);
                uMipLevels += PathFileExistsA(szBuffer) ? 1 : 0;
            }
        }
    }
    (void)uMipLevels;

    const int fileType = GetTextureFileType(lpTexHandle->Name);
    if (fileType == smTEXFILETYPE_TGA)
    {
        if (!DecryptTGA(pBuffer, iSize))
        {
            delete[] pBuffer;
            SummitTextureMarkPermanentFailure(lpTexHandle, "INVALID_TGA_HEADER");
            return -3;
        }
        lpTexHandle->MapOpacity = TRUE;
    }
    else if (fileType == smTEXFILETYPE_BMP)
    {
        if (!DecryptBMP(pBuffer, iSize))
        {
            delete[] pBuffer;
            SummitTextureMarkPermanentFailure(lpTexHandle, "INVALID_BMP_HEADER");
            return -3;
        }
        lpTexHandle->MapOpacity = FALSE;
    }
    else if (fileType == smTEXFILETYPE_JPG)
    {
        lpTexHandle->MapOpacity = FALSE;
    }
    else if (fileType == smTEXFILETYPE_PNG)
    {
        lpTexHandle->MapOpacity = TRUE;
    }

    D3DXIMAGE_INFO imageInfo;
    ZeroMemory(&imageInfo, sizeof(imageInfo));
    if (FAILED(D3DXGetImageInfoFromFileInMemory(pBuffer, iSize, &imageInfo)))
    {
        delete[] pBuffer;
        SummitTextureMarkPermanentFailure(lpTexHandle, "INVALID_IMAGE_INFO");
        return -3;
    }

    lpTexHandle->Width = imageInfo.Width;
    lpTexHandle->Height = imageInfo.Height;

    using namespace summit::rendering::assets::v2;
    AssetLoadPolicy runtimePolicy = SummitAssetRegistry.Policy();
    if (!IsStructurallyValidSource(imageInfo.Width, imageInfo.Height, runtimePolicy))
    {
        delete[] pBuffer;
        SummitTextureMarkPermanentFailure(lpTexHandle, "IMPLAUSIBLE_DIMENSIONS", imageInfo.Width, imageInfo.Height);
        return -3;
    }

    D3DCAPS9 caps;
    ZeroMemory(&caps, sizeof(caps));
    if (d3ddev && SUCCEEDED(d3ddev->GetDeviceCaps(&caps)))
    {
        const UINT deviceMax = caps.MaxTextureWidth < caps.MaxTextureHeight ? caps.MaxTextureWidth : caps.MaxTextureHeight;
        if (deviceMax && runtimePolicy.maxRuntimeDimension > deviceMax)
        {
            runtimePolicy.maxRuntimeDimension = deviceMax;
            runtimePolicy.maxRuntimePixels = static_cast<ULONGLONG>(deviceMax) * deviceMax;
        }
    }

    unsigned qualityShift = TexturHalfSize > 0 ? static_cast<unsigned>(TexturHalfSize) : 0u;
    if (qualityShift > 2u) qualityShift = 2u;
    AssetTargetDimensions target = ComputeTargetDimensions(
        imageInfo.Width, imageInfo.Height, runtimePolicy, qualityShift, true);

    D3DXIMAGE_INFO residentInfo = imageInfo;
    residentInfo.Width = target.width;
    residentInfo.Height = target.height;
    D3DFORMAT preferredFormat = SummitChooseLargeTextureFormat(lpTexHandle, residentInfo);

    ULONGLONG estimatedBytes = SummitEstimateSurfaceBytes(
        target.width,
        target.height,
        preferredFormat == D3DFMT_FROM_FILE ? D3DFMT_A8R8G8B8 : preferredFormat);

    // If the current working set leaves no room, evict LRU entries. If the
    // single texture itself still cannot fit, progressively reduce its runtime
    // size while preserving the enhanced source file on disk.
    while (!SummitAssetEnsureBudget(estimatedBytes, static_cast<std::size_t>(assetIndex)) &&
           (target.width > 512u || target.height > 512u))
    {
        target.width = target.width > 512u ? summit::rendering::assets::v2::AlignDown4(target.width / 2u) : target.width;
        target.height = target.height > 512u ? summit::rendering::assets::v2::AlignDown4(target.height / 2u) : target.height;
        residentInfo.Width = target.width;
        residentInfo.Height = target.height;
        preferredFormat = SummitChooseLargeTextureFormat(lpTexHandle, residentInfo);
        estimatedBytes = SummitEstimateSurfaceBytes(
            target.width,
            target.height,
            preferredFormat == D3DFMT_FROM_FILE ? D3DFMT_A8R8G8B8 : preferredFormat);
        target.downscaled = true;
    }

    if (!SummitAssetEnsureBudget(estimatedBytes, static_cast<std::size_t>(assetIndex)))
    {
        delete[] pBuffer;
        SummitTextureRecordCreateFailure(lpTexHandle, E_OUTOFMEMORY, imageInfo.Width, imageInfo.Height);
        SummitLogTextureReject(lpTexHandle->Name, "RESIDENT_BUDGET_EXHAUSTED", target.width, target.height);
        return 0;
    }

    if (target.downscaled)
    {
        char detail[128] = { 0 };
        wsprintf(detail, "DOWNSCALE_%ux%u_TO_%ux%u", imageInfo.Width, imageInfo.Height, target.width, target.height);
        SummitLogAssetEvent("ADAPT", lpTexHandle, estimatedBytes, detail);
    }

    HRESULT textureHr = E_FAIL;
    if (preferredFormat != D3DFMT_FROM_FILE)
    {
        textureHr = D3DXCreateTextureFromFileInMemoryEx(
            d3ddev,
            pBuffer,
            iSize,
            target.width,
            target.height,
            1,
            0,
            preferredFormat,
            D3DPOOL_MANAGED,
            D3DX_FILTER_POINT,
            D3DX_DEFAULT,
            0xFFF0FFFF,
            &imageInfo,
            NULL,
            &lpTexHandle->lpD3DTexture);
        SummitLogLargeTexturePolicy(lpTexHandle->Name, residentInfo, preferredFormat, textureHr);
    }

    if (preferredFormat == D3DFMT_FROM_FILE || FAILED(textureHr))
    {
        if (lpTexHandle->lpD3DTexture)
        {
            lpTexHandle->lpD3DTexture->Release();
            lpTexHandle->lpD3DTexture = NULL;
        }

        ULONGLONG fallbackBytes = SummitEstimateSurfaceBytes(target.width, target.height, D3DFMT_A8R8G8B8);
        while (!SummitAssetEnsureBudget(fallbackBytes, static_cast<std::size_t>(assetIndex)) &&
               (target.width > 512u || target.height > 512u))
        {
            target.width = target.width > 512u ? summit::rendering::assets::v2::AlignDown4(target.width / 2u) : target.width;
            target.height = target.height > 512u ? summit::rendering::assets::v2::AlignDown4(target.height / 2u) : target.height;
            fallbackBytes = SummitEstimateSurfaceBytes(target.width, target.height, D3DFMT_A8R8G8B8);
            target.downscaled = true;
        }

        if (SummitAssetEnsureBudget(fallbackBytes, static_cast<std::size_t>(assetIndex)))
        {
            textureHr = D3DXCreateTextureFromFileInMemoryEx(
                d3ddev,
                pBuffer,
                iSize,
                target.width,
                target.height,
                D3DX_FROM_FILE,
                0,
                D3DFMT_FROM_FILE,
                D3DPOOL_MANAGED,
                D3DX_FILTER_POINT,
                D3DX_DEFAULT,
                0xFFF0FFFF,
                &imageInfo,
                NULL,
                &lpTexHandle->lpD3DTexture);
        }
        else
        {
            textureHr = E_OUTOFMEMORY;
        }
    }

    delete[] pBuffer;

    if (FAILED(textureHr) || !lpTexHandle->lpD3DTexture)
    {
        lpTexHandle->lpD3DTexture = NULL;
        SummitTextureRecordCreateFailure(lpTexHandle, textureHr, imageInfo.Width, imageInfo.Height);
        return 0;
    }

    SummitTextureRecordCreateSuccess(lpTexHandle, imageInfo.Width, imageInfo.Height);
    if (!SummitAssetEnsureBudget(0, static_cast<std::size_t>(assetIndex)))
        SummitLogAssetEvent("BUDGET_OVERSHOOT", lpTexHandle, SummitAssetRegistry.Record(static_cast<std::size_t>(assetIndex)).residentBytes, "no_evictable_victim");
    return TRUE;
}







