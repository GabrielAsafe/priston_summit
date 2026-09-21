#define SWAPTEXTURE_MAX		64

extern int		g_IsReadTextures;

extern int		g_DibLastReadWidth;
extern int		g_DibLastReadHeight;




#define		MIPMAP_FILELIST				5
#define		SQUARE_MODE					1
#define		NOSQUARE_MODE				2

extern int MipMapModeCreate;
extern int TextureSwapMode;



class smTEXTURE {
public:

	
	
	DWORD			TexSpeedSum[ MAX_TEXTURE ];
	

	smTEXTUREHANDLE Texture[MAX_TEXTURE];
    LPDIRECT3DTEXTURE9 *lpLastTexture;

	CRITICAL_SECTION	cTextureSection;			

	int LastTextureCnt;
	int TextureCnt;


	smTEXTURE();
	~smTEXTURE();

	void Init();
	int	 Remove();

	
	
	
	int Find( DWORD dwCode, char *Name );
	

	smTEXTUREHANDLE *Add( char *name , char *nameA=0 );

	
	

	
	int Delete( smTEXTUREHANDLE *lpTex );

	
	int ReadTexture();



};

class smMATERIAL_GROUP {
	DWORD	Head;
public:
	smMATERIAL *smMaterial;
	DWORD MaterialCount;

	int ReformTexture;		

	int MaxMaterial;

	int LastSearchMaterial;
	char szLastSearchName[64];

	smMATERIAL_GROUP();
	smMATERIAL_GROUP( int MaterialMax );
	~smMATERIAL_GROUP();

	
	void Init( int MaterialMax );
	
	int Close();

	
	int GetSaveSize();
	
	int SaveFile( HANDLE hFile );
	
	int LoadFile( HANDLE hFile );

	
	int DeleteUnuseMaterial();
	
	int AddMaterial( ASE_MATERIAL *aseMaterial , char *szScript=0 );
	
	int CreateTextureMaterial( char *szBitmap , char *szOpacity , int TwoSide ,int Shade , DWORD TexSwap , DWORD BlendType );
	
	int	AddTexture( int MatNum , char *szBitmap , char *szOpacity=0 );

	
	int ReSwapMaterial( smMATERIAL *lpMaterial );
	
	int CheckMatreialTextureSwap();
	
	int IncTextureFrame();
	
	int CreateAnimationMaterial( char **szBitmap , int NumTex , DWORD BlendType , int AutoAnimMode );
	
	int ChangeMaterialToAnimation( smMATERIAL *lpMaterial , char **szBitmap , int NumTex , DWORD BlendType , int AutoAnimMode );
	
	int SetMaterialAnimFrame( smMATERIAL *lpMaterial , int frame );
	
	smMATERIAL *FindMaterialFromTextureName( char *szName );
	
	smMATERIAL *FindNextMaterialFromTextureName();

	
	
	
	int ReadTextures( int MipMap = 0 );
	

};



#define MATRIAL_MAX			5000


extern smMATERIAL *smMaterial;
extern smMATERIAL_GROUP	*smMaterialGroup;

extern smTEXTURE smTexture;



extern DWORD	VramTotal;



int InitMaterial();

int CloseMaterial();


int AddMaterial( ASE_MATERIAL *aseMaterial );

int DeleteUnuseMaterial();

int CreateTextureMaterial( char *szBitmap , char *szOpacity , int TwoSide ,int Shade , DWORD TexSwap , DWORD BlendType );

int	AddTexture( int MatNum , char *szBitmap , char *szOpacity=0 );

int CreateAnimationMaterial( char **szBitmap , int NumTex , DWORD BlendType , int AutoAnimMode=TRUE );

int ChangeMaterialToAnimation( smMATERIAL *lpMaterial , char **szBitmap , int NumTex , DWORD BlendType , int AutoAnimMode=TRUE );

int SetMaterialAnimFrame( smMATERIAL *lpMaterial , int frame );


int CheckMatreialTextureSwap();

int ReSwapMaterial( smMATERIAL *lpMaterial );

extern int MaterialFrame;

int IncTextureFrame();


int SetTextureSwapMode( int mode );


int smSetTextureLevel( int Level );



smMATERIAL *FindMaterialFromTextureName( char *szName );

smMATERIAL *FindNextMaterialFromTextureName();







int InitTexture();

int CloseTexture();


int ReadTextures();

int RestoreTextures();

// Asset Load System V2 legacy adapter. These APIs intentionally keep
// smTEXTUREHANDLE unchanged and make residency a sidecar concern.
enum smASSET_OWNER_V2
{
	SMASSET_OWNER_GLOBAL    = 1u << 0,
	SMASSET_OWNER_STAGE     = 1u << 1,
	SMASSET_OWNER_EFFECT    = 1u << 2,
	SMASSET_OWNER_UI        = 1u << 3,
	SMASSET_OWNER_CHARACTER = 1u << 4,
	SMASSET_OWNER_SKY       = 1u << 5
};

int  AssetLoadV2EnsureTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap = 0);
LPDIRECT3DTEXTURE9 AssetLoadV2GetResidentTexture(smTEXTUREHANDLE *lpTexHandle, int MipMap = 0);
void AssetLoadV2TouchTexture(smTEXTUREHANDLE *lpTexHandle);
void AssetLoadV2AddOwner(smTEXTUREHANDLE *lpTexHandle, DWORD ownerMask);
int  AssetLoadV2ReadMaterialGroup(smMATERIAL_GROUP *group, DWORD ownerMask, int MipMap = 0);
int  AssetLoadV2ReleaseMaterial(smMATERIAL_GROUP *group, int materialIndex);
void AssetLoadV2TrimToBudget(const char *reason);
void AssetLoadV2DumpTelemetry(const char *reason);


void smLeaveTextureCriticalSection();

void smEnterTextureCriticalSection();


LPDIRECT3DTEXTURE9 *GetLastTextureSurface();
LPDIRECT3DTEXTURE9 *GetLastTexture();

LPDIRECT3DTEXTURE9 LoadDibSurfaceOffscreen( char *Filename );
LPDIRECT3DTEXTURE9 LoadDibSurfaceOffscreenWithInfo( char *Filename, UINT *pSourceWidth, UINT *pSourceHeight );


LPDIRECT3DTEXTURE9 GetTextureHandle( int num );
LPDIRECT3DTEXTURE9 GetTextureChildHandle( int num );


int AddErrorList( char *szErrMsg );






