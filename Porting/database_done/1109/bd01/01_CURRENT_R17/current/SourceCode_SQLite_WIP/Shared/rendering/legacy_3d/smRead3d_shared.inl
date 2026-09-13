#pragma once

#include <cstring>
#include <limits>
#include <memory>
#include <vector>

#include "io/LegacyFile.h"
#include "io/LegacyPath.h"
#include "io/LegacyParse.h"
#include "formats/ase/AseLineCursor.h"
#include "formats/ase/AseMeshRecords.h"
#include "rendering/legacy_3d/ase/AseImportContext.h"

using summit::rendering::legacy3d::AseImportContext;

static const int ASE_MATERIAL_CAPACITY = summit::rendering::legacy3d::AseMaterialCapacity;
static const int ASE_TEXTURE_SLOT_MAX = summit::rendering::legacy3d::AseTextureSlotMax;

char DataDirectory[128];


static int smMeshReload = TRUE;
static int smMeshSave = 0;

struct smTFACE
{
	DWORD	t[3];
};

struct smFCPOINT
{
	float r, g, b;
};











char *GetWord(char *q, char *p)
{


	while((*p == 32) || (*p == 9) || (*p == 58))
	{
		p++;
	}

	while((*p != 32) && (*p != 9) && (*p != 58))
	{
		// MINGW_GETWORD_LINE_TERMINATOR_FIX_V1
		if(*p == 0 || *p == '\n' || *p == '\r') break;
		*q++ = *p++;
	}
	*q++ = 0;



	return p;
}

char *GetString(char *q, char *p)
{
	// MINGW_GETSTRING_BOUNDS_FIX_V1
	while(*p != 34)
	{
		if(*p == 0 || *p == '\r' || *p == '\n')
		{
			*q = 0;
			return p;
		}
		p++;
	}

	p++;

	while(*p != 34)
	{
		if(*p == 0 || *p == '\r' || *p == '\n')
		{
			*q = 0;
			return p;
		}

		*q++ = *p++;
	}

	if(*p == 34)
		p++;

	*q = 0;

	return p;
}


char *SetDirectoryFromFile(char *filename)
{
	if(!summit::io::CopyFileDirectory(filename, DataDirectory, sizeof(DataDirectory)))
		DataDirectory[0] = 0;

	return DataDirectory;
}


char szChangeFileExt[64];


char *ChangeFileExt(char *filename, char *FileExt)
{
	if(!summit::io::ReplaceFileExtension(
		filename, FileExt, szChangeFileExt, sizeof(szChangeFileExt)))
		szChangeFileExt[0] = 0;

	return szChangeFileExt;
}

#include "rendering/legacy_3d/ase/AseMaterialReader.inl"
#include "rendering/legacy_3d/ase/AseLightMapMaterialReader.inl"
#include "rendering/legacy_3d/ase/AseGeometryReader.inl"
#include "rendering/legacy_3d/ase/AseModelReader.inl"
#include "rendering/legacy_3d/ase/AseBoneMerge.inl"
#include "rendering/legacy_3d/ase/AseStageGeometryReader.inl"
#include "rendering/legacy_3d/ase/AseLightReader.inl"
#include "rendering/legacy_3d/ase/AseStageReader.inl"
