#pragma once

#include <cstring>
#include <fstream>
#include <string>

#include "formats/legacy_config/LegacyLineCursor.h"

namespace summit
{
namespace formats
{
namespace server_config
{

struct MainConfigServices
{
	int (*applyServerSelect)(char *command, char *values);
	int (*applyServerConfig)(char *command, char *values);
	void (*initializeServerConfig)(rsSERVER_CONFIG &config);
	void (*reportOpenFailure)(const char *path);
};

template <std::size_t DecodeCapacity>
inline smCONFIG *ReadMainConfig(
	const char *iniFile,
	smCONFIG &config,
	TRANS_SERVER_LIST &serverList,
	rsSERVER_CONFIG &serverConfig,
	int &adminCommandCount,
	char (&decodeBuffer)[DecodeCapacity],
	const MainConfigServices &services)
{
	adminCommandCount = 0;

	std::memset(&config, 0, sizeof(config));
	std::memset(&serverList, 0, sizeof(serverList));
	std::memset(&serverConfig, 0, sizeof(serverConfig));

	config.StageObjCnt = 0;
	config.MapBright = DEFAULT_BRIGHT;
	config.MapContrast = DEFAULT_CONTRAST;
	config.MapLightVector.x = fONE;
	config.MapLightVector.y = -fONE;
	config.MapLightVector.z = fONE / 2;

	if(services.initializeServerConfig)
		services.initializeServerConfig(serverConfig);

	std::ifstream file(iniFile, std::ifstream::binary);
	if(file.fail())
	{
		if(services.reportOpenFailure)
			services.reportOpenFailure(iniFile);
		return 0;
	}

	for(std::string sourceLine; std::getline(file, sourceLine);)
	{
		char *values = summit::formats::legacy_config::ReadWordTruncated(
			decodeBuffer, const_cast<char *>(sourceLine.c_str()));

#ifdef _W_SERVER
		if(services.applyServerSelect && services.applyServerSelect(decodeBuffer, values))
			continue;
		if(services.applyServerConfig && services.applyServerConfig(decodeBuffer, values))
			continue;
#else
		(void)values;
#endif
	}

	file.close();

	smMATRIX rotateX, rotateY, vectorMatrix;
	int vectorX, vectorY, vectorZ;

	const int mapLightAngleY = 128 * 16;
	int mapLightAngleX = (128 - 10) * 16;
	mapLightAngleX = (mapLightAngleX - ANGLE_90) & ANGCLIP;

	const int x = 0;
	const int y = 0;
	const int z = static_cast<int>(1.5 * fONE);

	smIdentityMatrix(rotateX);
	smIdentityMatrix(rotateY);
	smRotateYMatrix(rotateY, mapLightAngleY);
	smRotateXMatrix(rotateX, mapLightAngleX);
	smMatrixMult(vectorMatrix, rotateX, rotateY);

	vectorX = x * vectorMatrix._11 + y * vectorMatrix._21 + z * vectorMatrix._31;
	vectorY = x * vectorMatrix._12 + y * vectorMatrix._22 + z * vectorMatrix._32;
	vectorZ = x * vectorMatrix._13 + y * vectorMatrix._23 + z * vectorMatrix._33;

	vectorX >>= FLOATNS;
	vectorY >>= FLOATNS;
	vectorZ >>= FLOATNS;

	config.MapLightVector.x = vectorX;
	config.MapLightVector.y = vectorY;
	config.MapLightVector.z = vectorZ;
	config.MapBright = 130;
	config.MapContrast = 300;

	return &config;
}

} // namespace server_config
} // namespace formats
} // namespace summit
