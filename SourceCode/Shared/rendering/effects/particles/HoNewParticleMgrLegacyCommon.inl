#include "diagnostics/RuntimeAssetProbe.h"
HoNewParticleResMgr  g_NewParticleResMgr;
HoNewParticleMgr     g_NewParticleMgr;

void HoNewParticleResMgr::init()
{
	MatRes.clear();
	ScriptRes.clear();

	MatRes.reserve(MAX_MATERIAL);
	ScriptRes.reserve(MAX_TOKEN_VECTORS);

	NumScriptRes = 0;
	NumMaterial = 0;

}



HoNewParticleScriptRes *HoNewParticleResMgr::FindScript(char *find)
{
    if (!find || !find[0]) return NULL;
    // Compare the registered string directly; the old temporary[256] copy
    // could overflow for an externally registered name.
    for (size_t index = 0; index < ScriptRes.size(); ++index)
        if (stricmp(ScriptRes[index].name.c_str(), find) == 0)
            return &ScriptRes[index];
    return NULL;
}

bool HoNewParticleResMgr::LoadScript(char *scr)
{
    if (!scr || !scr[0]) return false;
    const size_t pathLength = strlen(scr);
    if (pathLength > 1023u) return false;

    const char *nameStart = scr;
    for (const char *cursor = scr; *cursor; ++cursor)
        if (*cursor == '/' || *cursor == '\\') nameStart = cursor + 1;
    const char *extension = strrchr(nameStart, '.');
    const size_t keyLength = extension ? static_cast<size_t>(extension - nameStart) : strlen(nameStart);
    if (keyLength == 0 || keyLength >= 256u) return false;
    char existingKey[256];
    memcpy(existingKey, nameStart, keyLength);
    existingKey[keyLength] = 0;
    if (FindScript(existingKey)) return true;

    summit::diagnostics::RuntimeAssetProbeAttempt("PARTICLE_SCRIPT", "OPEN", scr);
    FILE *fp = fopen(scr, "rb");
    summit::diagnostics::RuntimeAssetProbeResult("PARTICLE_SCRIPT", "OPEN", scr, fp != NULL);
    // Missing scripts are normal on some legacy content paths. Do not
    // synchronously open an additional permanent diagnostics file.
    if (!fp) return false;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return false; }
    const long scriptSize = ftell(fp);
    if (scriptSize < 0 || scriptSize > 8L * 1024L * 1024L || fseek(fp, 0, SEEK_SET) != 0)
    {
        fclose(fp);
        return false;
    }
    std::vector<char> particleBuffer(static_cast<size_t>(scriptSize) + 1u, 0);
    if (scriptSize > 0 && fread(particleBuffer.data(), 1, static_cast<size_t>(scriptSize), fp) != static_cast<size_t>(scriptSize))
    {
        fclose(fp);
        return false;
    }
    fclose(fp);
    return AddScript(particleBuffer.data(), existingKey) != NULL;
}

HoNewParticleScriptRes *HoNewParticleResMgr::AddScript(char *scr, char *indexName)
{
    if (!scr || !indexName || !indexName[0] || MAX_TOKEN_VECTORS <= NumScriptRes)
        return NULL;
    if (FindScript(indexName)) return NULL;

    HoNewParticleScriptRes particleScriptRes;
    particleScriptRes.name = indexName;
    particleScriptRes.emitterTokenizer.Tokenize(scr);
    ScriptRes.push_back(particleScriptRes);
    NumScriptRes = static_cast<int>(ScriptRes.size());
    return &ScriptRes.back();
}

int HoNewParticleResMgr::SearchingAddTexture(const char *directory)
{
    if (!directory || !directory[0]) return FALSE;
    WIN32_FIND_DATA wfd;
    HANDLE hSrch = FindFirstFile(directory, &wfd);
    // The legacy function opened the same search twice and leaked the first
    // handle, then read uninitialized wfd if FindFirstFile failed.
    if (hSrch == INVALID_HANDLE_VALUE) return FALSE;
    do
    {
        if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strlen(wfd.cFileName) > 3u)
        {
            std::string texBuffer = wfd.cFileName;
            MakeUpperCase(texBuffer);
            AddTexture(texBuffer.c_str());
        }
    } while (FindNextFile(hSrch, &wfd));
    FindClose(hSrch);
    return TRUE;
}

int HoNewParticleResMgr::AddTexture(const char *mat)
{
    if (!mat || !mat[0]) return -1;
    const size_t nameLength = strlen(mat);
    // The registered name and the material path must both be representable.
    if (nameLength >= 256u || nameLength + sizeof("effect//particle//") > 500u)
        return -1;
    std::string normalized = mat;
    MakeUpperCase(normalized);
    for (size_t index = 0; index < MatRes.size(); ++index)
        if (MatRes[index].matName == normalized)
            return MatRes[index].matNum;
    if (MAX_MATERIAL <= NumMaterial) return -1;

    char fullDirectoryName[500];
    const int written = snprintf(fullDirectoryName, sizeof(fullDirectoryName),
                                 "%s%s", "effect//particle//", mat);
    if (written < 0 || static_cast<size_t>(written) >= sizeof(fullDirectoryName))
        return -1;
    const int materialNum = CreateTextureMaterial(fullDirectoryName, 0, 0, 0, 0, SMMAT_BLEND_LAMP);
    if (materialNum < 0) return -1; // Do not leave a failed entry in MatRes.
    HoNewParticleMatRes resource;
    resource.matName = normalized;
    resource.matNum = materialNum;
    MatRes.push_back(resource);
    NumMaterial = static_cast<int>(MatRes.size());
    if (smMaterialGroup) smMaterialGroup->ReadTextures();
    return materialNum;
}

void HoNewParticleMgr::Main(float elapsedTime, float timeDelta)
{
	std::vector<HoNewParticleEmitter>::iterator i;
	for(i = Emitters.begin(); i != Emitters.end(); ++i)
	{
		if((*i).IsRunning())
		{
			(*i).Main(elapsedTime, timeDelta);
		}
	}
}

void HoNewParticleMgr::Draw(int x, int y, int z, int ax, int ay, int az)
{
	std::vector<HoNewParticleEmitter>::iterator i;



	for(i = Emitters.begin(); i != Emitters.end(); ++i)
	{
		if((*i).IsRunning())
		{
			(*i).Draw(x, y, z, ax, ay, az);
		}
	}
}

int HoNewParticleMgr::Start(char *fileName, POINT3D pos, float startDelay)
{

	HoNewParticleScriptRes *scriptRes = g_NewParticleResMgr.FindScript(fileName);
	if(scriptRes != NULL)
	{
		return Start(scriptRes, pos, startDelay);
	}

	return -1;
}

int HoNewParticleMgr::Start(HoNewParticleScriptRes *tokenRes, POINT3D pos, float startDelay)
{
	LegacyHoNewParticleMgrPolicy::Vector3 particlePos;
	particlePos.x = (float)pos.x;
	particlePos.y = (float)pos.y;
	particlePos.z = (float)pos.z;

	if(Emitters.size() >= MAX_EMITTER)
	{
		int count = 0;
		vector<HoNewParticleEmitter>::iterator i;
		for(i = Emitters.begin(); i != Emitters.end(); ++i)
		{
			if(!(*i).IsRunning())
			{
				(*i).AddScript(tokenRes);
			#ifdef _HO_WIN_32_
				(*i).SetPos(particlePos);
			#endif
				(*i).Start(startDelay);

				return count;
			}
			count++;
		}
	}

	HoNewParticleEmitter emitter;
	emitter.AddScript(tokenRes);
#ifdef _HO_WIN_32_
	emitter.SetPos(particlePos);
#endif
	emitter.Start(startDelay);

	if(NumEmitters < MAX_EMITTER)
	{
		Emitters.push_back(emitter);
		NumEmitters++;
		return (NumEmitters - 1);
	}
	return (-1);
}

void HoNewParticleMgr::SetPos(int partID, LegacyHoNewParticleMgrPolicy::Vector3 pos)
{
	if(partID < 0 || (size_t)partID >= Emitters.size())
		return;

	if(Emitters[partID].IsRunning())
		Emitters[partID].SetPos(pos);
}

void HoNewParticleMgr::SetAngle(int partID, POINT3D angle)
{
	if(partID < 0 || (size_t)partID >= Emitters.size())
		return;

	if(Emitters[partID].IsRunning())
	{
		LegacyHoNewParticleMgrPolicy::Vector3 tempAngle;
		tempAngle.x = (float)angle.x;
		tempAngle.y = (float)angle.y;
		tempAngle.z = (float)angle.z;

		Emitters[partID].SetAngle(tempAngle);
	}
}

bool HoNewParticleMgr::GetRunning(int partID)
{
	if(partID < 0 || (size_t)partID >= Emitters.size())
		return false;
	if(Emitters[partID].IsRunning())
		return true;

	return false;
}


void HoNewParticleMgr::SetAttachPos(int partID, LegacyHoNewParticleMgrPolicy::Vector3 pos)
{
	if(partID < 0 || (size_t)partID >= Emitters.size())
		return;

	if(Emitters[partID].IsRunning())
		Emitters[partID].SetAttachPos(pos);
}

void HoNewParticleMgr::SetStop(int partID)
{
	if(partID < 0 || (size_t)partID >= Emitters.size())
		return;
	if(Emitters[partID].IsRunning())
		Emitters[partID].FadeStop();
}

void HoNewParticleMgr::SetFastStop(int partID)
{
	if(partID < 0 || (size_t)partID >= Emitters.size())
		return;
	if(Emitters[partID].IsRunning())
		Emitters[partID].FastStop();
}

void HoNewParticleMgr::SetRendering(int partID, BOOL flag)
{
	if(partID < 0 || (size_t)partID >= Emitters.size())
		return;
	if(Emitters[partID].IsRunning())
	{
		Emitters[partID].SetRendering(flag);
	}
}

void HoNewParticleMgr::Init()
{
	NumEmitters = 0;
	Emitters.clear();
	Emitters.reserve(MAX_EMITTER + 1);
	Running = false;
}

void HoNewParticleMgr::Clear()
{
	vector<HoNewParticleEmitter>::iterator i;
	for(i = Emitters.begin(); i != Emitters.end(); ++i)
	{
		(*i).init();
	}
}

void HoNewParticleMgr::Destroy()
{

	vector<HoNewParticleEmitter>::iterator i;
	for(i = Emitters.begin(); i != Emitters.end(); ++i)
	{
		(*i).init();
	}
}
