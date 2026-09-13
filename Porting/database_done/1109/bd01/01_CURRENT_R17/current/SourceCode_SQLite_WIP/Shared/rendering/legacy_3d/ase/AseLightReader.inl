#pragma once

struct AseLightParseContext
{
	AseLightParseContext() : intensity(0), blockDepth(0) {}
	int intensity;
	int blockDepth;
};

AseLineStatus ParseLightName(const char *tag, const char *payload, smASE_LIGHT *light)
{
	if(!summit::formats::ase::TagEquals(tag, "*NODE_NAME"))
		return AseLineStatus::NotHandled;

	char nodeName[256];
	summit::formats::ase::LineCursor cursor(payload);
	if(!cursor.NextQuoted(nodeName, sizeof(nodeName)))
		return AseLineStatus::Error;

	nodeName[31] = '\0';
	if(strstr(nodeName, "dynamic:"))
		light->Type |= smLIGHT_TYPE_DYNAMIC;
	if(strstr(nodeName, "night:"))
		light->Type |= smLIGHT_TYPE_DYNAMIC | smLIGHT_TYPE_NIGHT;
	if(strstr(nodeName, "lens:"))
		light->Type |= smLIGHT_TYPE_DYNAMIC | smLIGHT_TYPE_LENS;
	if(strstr(nodeName, "obj:"))
		light->Type |= smLIGHT_TYPE_DYNAMIC | SMLIGHT_TYPE_OBJ;
	return AseLineStatus::Handled;
}

AseLineStatus ParseLightColor(const char *tag, const char *payload, smASE_LIGHT *light)
{
	if(!summit::formats::ase::TagEquals(tag, "*LIGHT_COLOR"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	float red = 0.0f;
	float green = 0.0f;
	float blue = 0.0f;
	if(!cursor.NextFloat(&red) || !cursor.NextFloat(&green) || !cursor.NextFloat(&blue))
		return AseLineStatus::Error;

	light->r = static_cast<int>(red * 255.0f);
	light->g = static_cast<int>(green * 255.0f);
	light->b = static_cast<int>(blue * 255.0f);
	return AseLineStatus::Handled;
}

AseLineStatus ParseLightIntensity(const char *tag, const char *payload, AseLightParseContext &context)
{
	if(!summit::formats::ase::TagEquals(tag, "*LIGHT_INTENS"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	float intensity = 0.0f;
	if(!cursor.NextFloat(&intensity))
		return AseLineStatus::Error;
	context.intensity = static_cast<int>(intensity * fONE);
	return AseLineStatus::Handled;
}

AseLineStatus ParseLightPosition(const char *tag, const char *payload, smASE_LIGHT *light)
{
	if(!summit::formats::ase::TagEquals(tag, "*TM_POS"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	float sourceX = 0.0f;
	float sourceY = 0.0f;
	float sourceZ = 0.0f;
	if(!cursor.NextFloat(&sourceX) || !cursor.NextFloat(&sourceY) || !cursor.NextFloat(&sourceZ))
		return AseLineStatus::Error;

	light->x = static_cast<int>(sourceX * fONE);
	light->z = static_cast<int>(sourceY * fONE);
	light->y = static_cast<int>(sourceZ * fONE);
	return AseLineStatus::Handled;
}

AseLineStatus ParseLightRange(const char *tag, const char *payload, smASE_LIGHT *light)
{
	if(!summit::formats::ase::TagEquals(tag, "*LIGHT_MAPRANGE"))
		return AseLineStatus::NotHandled;

	summit::formats::ase::LineCursor cursor(payload);
	float range = 0.0f;
	if(!cursor.NextFloat(&range))
		return AseLineStatus::Error;
	light->Range = static_cast<int>((range * fONE) / 4 * fONE);
	return AseLineStatus::Handled;
}

AseLineStatus ParseLightProperties(const char *tag, const char *payload, AseLightParseContext &context, smASE_LIGHT *light)
{
	AseLineStatus result = ParseLightColor(tag, payload, light);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseLightIntensity(tag, payload, context);
	if(result != AseLineStatus::NotHandled) return result;
	result = ParseLightPosition(tag, payload, light);
	if(result != AseLineStatus::NotHandled) return result;
	return ParseLightRange(tag, payload, light);
}

void ApplyLightIntensity(smASE_LIGHT *light, int intensity)
{
	if(!intensity)
		return;

	if(intensity >= 0 && intensity < 64)
		intensity = 64;
	if(intensity < 0 && intensity > -64)
		intensity = -64;

	light->r = (light->r * intensity) >> FLOATNS;
	light->g = (light->g * intensity) >> FLOATNS;
	light->b = (light->b * intensity) >> FLOATNS;
}

} // anonymous namespace

FILE *smReadASE_LIGHTOBJECT(FILE *fp, smASE_LIGHT *smLight)
{
	if(!fp || !smLight)
		return FALSE;

	AseLightParseContext context;
	smLight->Type = 0;
	char inputLine[256];
	bool objectFinished = false;

	while(fgets(inputLine, sizeof(inputLine), fp) != NULL)
	{
		summit::formats::ase::LineCursor lineCursor(inputLine);
		char tag[64];
		if(!lineCursor.NextToken(tag, sizeof(tag)))
			continue;

		const char *payload = lineCursor.Position();
		AseLineStatus status = ParseLightName(tag, payload, smLight);
		if(status == AseLineStatus::NotHandled)
			status = ParseLightProperties(tag, payload, context, smLight);
		if(status == AseLineStatus::Error)
			return FALSE;

		if(UpdateBlockDepth(tag, payload, context.blockDepth) == AseLineStatus::FinishObject)
		{
			objectFinished = true;
			break;
		}
	}

	if(!objectFinished)
		return FALSE;

	ApplyLightIntensity(smLight, context.intensity);
	return fp;
}
