

#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
HoAnimDataMgr	AnimDataMgr;




BOOL GetDirectoryToFileName(const char *directoryName, char *getFileName, size_t getFileNameSize)
{
	if(!directoryName || !getFileName || getFileNameSize == 0) return FALSE;
	getFileName[0] = 0;
	const char *name = strrchr(directoryName, '/');
	const char *name2 = strrchr(directoryName, '\\');
	if(!name || (name2 && name2 > name)) name = name2;
	name = name ? name + 1 : directoryName;
	if(!name[0]) return FALSE;
	strncpy(getFileName, name, getFileNameSize - 1);
	getFileName[getFileNameSize - 1] = 0;
	return TRUE;
}

enum HoAnimValueType
{
	HO_ANIM_IMAGE = 0,
	HO_ANIM_DELAY,
	HO_ANIM_ALPHA,
	HO_ANIM_SIZE,
	HO_ANIM_ANGLE
};

static BOOL ParseAnimFrameValues(const char *buffer, hoAnimFrame *frames, int frameCount, HoAnimValueType valueType)
{
	if(!buffer || !buffer[0] || !frames || frameCount <= 0) return FALSE;
	const char *token = buffer;
	int frame = 0;

	while(TRUE)
	{
		const char *comma = strchr(token, ',');
		const char *tokenEnd = comma ? comma : token + strlen(token);
		const size_t tokenLength = (size_t)(tokenEnd - token);
		if(frame >= frameCount || tokenLength == 0 || tokenLength >= 64) return FALSE;
		char num[64] = {};
		memcpy(num, token, tokenLength);
		const int value = atoi(num);

		switch(valueType)
		{
		case HO_ANIM_IMAGE: frames[frame].ImageNum = (unsigned short)value; break;
		case HO_ANIM_DELAY: frames[frame].Delay = (unsigned short)value; break;
		case HO_ANIM_ALPHA: frames[frame].AlphaValue = (BYTE)value; break;
		case HO_ANIM_SIZE: frames[frame].SizeWidth = (unsigned short)value; break;
		case HO_ANIM_ANGLE: frames[frame].Angle = float(ANGLE_360 * (float(value) / 360.f)); break;
		default: return FALSE;
		}

		++frame;
		if(!comma) break;
		token = comma + 1;
	}
	return frame > 0;
}

HoAnimSequenceData::HoAnimSequenceData()
{
	AnimFrameCount = 0;
	AnimFrame = NULL;
	DataFileIndex = -1;
	BlendType = 0;
	InfoFlag = 0;

}
HoAnimSequenceData::~HoAnimSequenceData()
{
	Init();
}



int HoAnimSequenceData::Load(char *iniName, HoAnimDataMgr *animationData)
{
	if(iniName == NULL || animationData == NULL) return FALSE;
	Init();
	DataFileIndex = -1;
	BlendType = 0;

	if(!GetDirectoryToFileName(iniName, IniName, sizeof(IniName))) return FALSE;

	char buffer[512] = {};
	GetPrivateProfileString("ANIMATION", "DataFile", NULL, buffer, sizeof(buffer), iniName);
	DataFileIndex = animationData->GetSearchAnimImageData(buffer);
	StartBlendValue = GetPrivateProfileInt("ANIMATION", "StartBlendValue", 0, iniName);
	BlendType = GetPrivateProfileInt("ANIMATION", "BlendType", 0, iniName);

	ZeroMemory(buffer, sizeof(buffer));
	GetPrivateProfileString("ANIMATION", "ImageNum", NULL, buffer, sizeof(buffer), iniName);
	if(!buffer[0]) return FALSE;

	AnimFrameCount = 1;
	for(const char *p = buffer; *p; ++p) if(*p == ',') ++AnimFrameCount;
	if(AnimFrameCount <= 0 || AnimFrameCount > (int)(sizeof(buffer) / 2)) return FALSE;
	AnimFrame = new hoAnimFrame[AnimFrameCount];
	memset(AnimFrame, 1, sizeof(hoAnimFrame) * AnimFrameCount);
	if(!ParseAnimFrameValues(buffer, AnimFrame, AnimFrameCount, HO_ANIM_IMAGE)) { Init(); return FALSE; }
	InfoFlag |= INFO_IMAGE;

	ZeroMemory(buffer, sizeof(buffer));
	GetPrivateProfileString("ANIMATION", "Delay", NULL, buffer, sizeof(buffer), iniName);
	if(!ParseAnimFrameValues(buffer, AnimFrame, AnimFrameCount, HO_ANIM_DELAY)) { Init(); return FALSE; }
	InfoFlag |= INFO_DELAY;

	ZeroMemory(buffer, sizeof(buffer));
	GetPrivateProfileString("ANIMATION", "BlendValue", NULL, buffer, sizeof(buffer), iniName);
	if(!ParseAnimFrameValues(buffer, AnimFrame, AnimFrameCount, HO_ANIM_ALPHA)) { Init(); return FALSE; }
	InfoFlag |= INFO_ALPHA;

	ZeroMemory(buffer, sizeof(buffer));
	GetPrivateProfileString("ANIMATION", "Size", NULL, buffer, sizeof(buffer), iniName);
	if(buffer[0])
	{
		if(!ParseAnimFrameValues(buffer, AnimFrame, AnimFrameCount, HO_ANIM_SIZE)) { Init(); return FALSE; }
		InfoFlag |= INFO_SIZEWIDTH;
	}

	ZeroMemory(buffer, sizeof(buffer));
	GetPrivateProfileString("ANIMATION", "Angle", NULL, buffer, sizeof(buffer), iniName);
	if(buffer[0])
	{
		if(!ParseAnimFrameValues(buffer, AnimFrame, AnimFrameCount, HO_ANIM_ANGLE)) { Init(); return FALSE; }
		InfoFlag |= INFO_ANGLE;
	}

	return TRUE;
}

int HoAnimSequenceData::Init()
{
	AnimFrameCount = 0;
	InfoFlag = 0;
	if(AnimFrame != NULL)
	{
		delete[] AnimFrame;
		AnimFrame = NULL;
	}
	memset(IniName, 0, sizeof(IniName));

	return TRUE;
}

HoAnimImageData::HoAnimImageData()
{
	AnimationCount = 0;
	MaterialNum = -1;
}

HoAnimImageData::~HoAnimImageData()
{

}

int HoAnimImageData::Load(char *iniName)
{
	if(!iniName) return FALSE;
	Init();
	if(!GetDirectoryToFileName(iniName, IniName, sizeof(IniName))) return FALSE;

	char dataName[512] = {};
	GetPrivateProfileString("IMAGE", "Name", NULL, dataName, sizeof(dataName), iniName);
	const int dataCount = GetPrivateProfileInt("IMAGE", "Count", 0, iniName);
	return Load(dataName, dataCount);
}

int HoAnimImageData::Load(char *textureName, int count)
{
	if(!textureName || !textureName[0] || count <= 0 || count > 30) return FALSE;

	const char *dot = strrchr(textureName, '.');
	if(!dot || dot == textureName || !dot[1]) return FALSE;
	const size_t baseLen = (size_t)(dot - textureName);
	if(baseLen >= 240) return FALSE;

	char fileName[256] = {};
	char fileExt[64] = {};
	memcpy(fileName, textureName, baseLen);
	fileName[baseLen] = '\0';
	strncpy(fileExt, dot + 1, sizeof(fileExt) - 1);
	fileExt[sizeof(fileExt) - 1] = '\0';

	const BOOL isBmp = stricmp(fileExt, "bmp") == 0;
	const BOOL isTga = stricmp(fileExt, "tga") == 0;
	if(!isBmp && !isTga) return FALSE;

	char *szEffect[30] = {};
	for(int cnt = 0; cnt < count; ++cnt)
	{
		szEffect[cnt] = new char[256];
		const int written = snprintf(szEffect[cnt], 256, "%s%d.%s", fileName, cnt + 1, isBmp ? "bmp" : "tga");
		if(written < 0 || written >= 256)
		{
			for(int i = 0; i <= cnt; ++i) { delete[] szEffect[i]; szEffect[i] = NULL; }
			return FALSE;
		}
	}

	const int blendMode = isBmp ? SMMAT_BLEND_COLOR : SMMAT_BLEND_ALPHA;
	MaterialNum = CreateTextureMaterial(szEffect[0], 0, 0, 0, 0, blendMode);
	if(MaterialNum < 0 || smMaterial[MaterialNum].smTexture == NULL)
	{
		for(int i = 0; i < count; ++i) { delete[] szEffect[i]; szEffect[i] = NULL; }
		MaterialNum = -1;
		return FALSE;
	}

	smMaterialGroup->ChangeMaterialToAnimation(&smMaterial[MaterialNum], szEffect, count, blendMode, FALSE);
	for(int i = 0; i < count; ++i) { delete[] szEffect[i]; szEffect[i] = NULL; }

	AnimationCount = count;
	smMaterial[MaterialNum].TwoSide = TRUE;
	return TRUE;
}

int	HoAnimImageData::Init()
{
	memset(IniName, 0, sizeof(IniName));
	MaterialNum = 0;
	AnimationCount = 0;
	return TRUE;
}

HoAnimDataMgr::HoAnimDataMgr()
{
	Init();

}

HoAnimDataMgr::~HoAnimDataMgr()
{
	Init();
}

int HoAnimDataMgr::SequenceLoad(char *iniName)
{
	if(SequenceDataBufferCount < MAX_SEQUENCEDATA_BUFFER)
	{
		if(!SequenceDataBuffer[SequenceDataBufferCount].Load(iniName, this))
			return FALSE;

		SequenceDataBufferCount++;
		return TRUE;
	}
	return FALSE;
}

int HoAnimDataMgr::ImageLoad(char *iniName)
{

	if(ImageDataBufferCount < MAX_IMAGEDATA_BUFFER)
	{
		if(!ImageDataBuffer[ImageDataBufferCount].Load(iniName))
			return FALSE;
		ImageDataBufferCount++;
		return TRUE;
	}

	return FALSE;
}

int HoAnimDataMgr::Init()
{

	for(int index = 0; index < MAX_IMAGEDATA_BUFFER; index++)
	{
		ImageDataBuffer[index].Init();
	}

	for(int index = 0; index < MAX_SEQUENCEDATA_BUFFER; index++)
	{
		SequenceDataBuffer[index].Init();
	}

	ImageDataBufferCount = 0;
	SequenceDataBufferCount = 0;

	return TRUE;
}


int HoAnimDataMgr::GetSearchSequenceData(char *iniName)
{
	for(int index = 0; index < SequenceDataBufferCount; index++)
	{
		if(stricmp(SequenceDataBuffer[index].IniName, iniName) == 0)
			return index;
	}

	return -1;
}

int HoAnimDataMgr::GetSearchAnimImageData(char *iniName)
{
	for(int index = 0; index < ImageDataBufferCount; index++)
	{
		if(stricmp(ImageDataBuffer[index].IniName, iniName) == 0)
			return index;
	}

	return -1;
}



