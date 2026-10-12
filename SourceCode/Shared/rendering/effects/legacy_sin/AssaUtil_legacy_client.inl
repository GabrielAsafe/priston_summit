

#include "compat/legacy/sin/sinLinkHeader.h"
#include <new>
#include <deque>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include "core/concurrency/JobSystem.h"
#include "core/concurrency/SourceAssetLock.h"
#include "rendering/effects/legacy_sin/SummitAssaCatalog251.h"
#include "rendering/effects/legacy_sin/SummitAssaLookup252.h"


#define MAX_RES_PATH	300
#define MAX_RES_DRIVE   200
#define MAX_FIND_FILE   300
char FindFileName[MAX_FIND_FILE];

char *AssaErrorFile = "AssaErrorFile.txt";
char *AssaLoadingFile = "AssaLoadingFile.txt";

BOOL FindFileRecursive(char *path, char *fileName)
{
	HANDLE hSrch;
	WIN32_FIND_DATA wfd;
	BOOL bResult = TRUE;
	char drive[MAX_RES_DRIVE] = { 0 };
	char dir[MAX_RES_PATH] = { 0 };
	char newpath[MAX_RES_PATH] = { 0 };

	hSrch = FindFirstFile(path, &wfd);
	if(hSrch == INVALID_HANDLE_VALUE)
		return FALSE;
	while(bResult)
	{
		_splitpath(path, drive, dir, NULL, NULL);
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(wfd.cFileName[0] != '.')
			{
                if (!summit::effects::JoinCatalogPath251(newpath, drive, dir,
                    wfd.cFileName, "//*.*"))
                {
                    bResult = FindNextFile(hSrch, &wfd);
                    continue;
                }
				if(FindFileRecursive(newpath, fileName))
				{
					FindClose(hSrch);
					return TRUE;
				}
			}
		}
		else
		{
			if(stricmp(wfd.cFileName, fileName) == 0)
			{
                if (summit::effects::JoinCatalogPath251(FindFileName,
                    drive, dir, wfd.cFileName))
                {
                    FindClose(hSrch);
                    return TRUE;
                }
			}
		}
		bResult = FindNextFile(hSrch, &wfd);
	}

	FindClose(hSrch);
	return FALSE;
}


char *FindFile(char *fileName)
{
	char Path[MAX_RES_PATH] = { 0 };
    const DWORD size = GetCurrentDirectory(MAX_RES_PATH, Path);
    if (!size || size >= MAX_RES_PATH ||
        !summit::effects::AppendCatalogPath251(Path, "//image//Sinimage//TestEffect//*.*"))
    {
        FindFileName[0] = 0;
        return FindFileName;
    }


	memset(FindFileName, 0, sizeof(FindFileName));
	FindFileRecursive(Path, fileName);
	return FindFileName;
}

#define MAX_RES_NAME	  64
#define MAX_RES_FULL_NAME 512
#define MAX_RES_BUFFER    1000			

#define TYPE_MAT        100
#define TYPE_ASE        200
#define TYPE_BONE       300
#define TYPE_BONE_ANI   400				

struct cASSARES
{
	void *res;
	char resName[MAX_RES_NAME];
	char resFullName[MAX_RES_FULL_NAME];
	int  type;
	BOOL LoadFlag;
};


cASSARES AssaRes[MAX_RES_BUFFER];
int		 AssaResCount;

// Cache the first matching catalog index, preserving the legacy first-match
// rule for names duplicated across effect directories.
static summit::effects::AssaLookup252 summitAssaLookup252;

static int SummitFindAssaRes252(const char *name)
{
    if (!name || !*name) return -1;
    if (summitAssaLookup252.Count() != AssaResCount)
        summitAssaLookup252.Rebuild(AssaRes, AssaResCount);
    const int found = summitAssaLookup252.Find(name);
    if (found >= 0 && found < AssaResCount &&
        stricmp(AssaRes[found].resName, name) == 0)
        return found;
    // Preserve Win32 stricmp behavior for non-ASCII filenames and unexpected
    // locale mappings absent from the ASCII index.
    for (int index = 0; index < AssaResCount; ++index)
        if (stricmp(AssaRes[index].resName, name) == 0)
            return index;
    return -1;
}



// SUMMIT_ASSA_ASYNC_256_BEGIN
namespace
{
struct SummitAssaJob256
{
    int slot;
    int type;
    unsigned long long generation;
    std::string meshPath;
    std::string bonePath;
    smPAT3D *mesh;
    smPAT3D *bone;
    SummitAssaJob256(int s, int t, unsigned long long g, const std::string &m, const std::string &b)
        : slot(s), type(t), generation(g), meshPath(m), bonePath(b), mesh(NULL), bone(NULL) {}
};
struct SummitAssaState256
{
    std::mutex mutex;
    std::set<int> pending;
    std::set<int> failed;
    std::deque<std::shared_ptr<SummitAssaJob256> > complete;
    unsigned long long generation;
    int cursor;
    SummitAssaState256() : generation(1ull), cursor(0) {}
};
static SummitAssaState256 &SummitAssaState()
{
    static SummitAssaState256 *state = new SummitAssaState256();
    return *state;
}
static void SummitAssaDiscardOnOwner(SummitAssaJob256 &job)
{
    // Destroy the mesh before the bone; worker may have attached mesh to bone.
    delete job.mesh;
    job.mesh = NULL;
    delete job.bone;
    job.bone = NULL;
}
}

void SummitResetAssaEffectModels256()
{
    SummitAssaState256 &state = SummitAssaState();
    std::deque<std::shared_ptr<SummitAssaJob256> > discarded;
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        ++state.generation;
        state.pending.clear();
        state.failed.clear();
        state.cursor = 0;
        discarded.swap(state.complete);
    }
    for (std::size_t i = 0; i < discarded.size(); ++i)
        SummitAssaDiscardOnOwner(*discarded[i]);
}

void SummitPumpAssaEffectModels256(unsigned int maxCommits)
{
    SummitAssaState256 &state = SummitAssaState();
    if (maxCommits > 2u) maxCommits = 2u;
    for (unsigned int i = 0; i < maxCommits; ++i)
    {
        std::shared_ptr<SummitAssaJob256> job;
        bool current = false;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            if (state.complete.empty()) break;
            job = state.complete.front();
            state.complete.pop_front();
            current = (job->generation == state.generation);
            if (current)
            {
                state.pending.erase(job->slot);
                if (!job->mesh || (job->type == TYPE_BONE && !job->bone))
                    state.failed.insert(job->slot);
            }
        }
        if (current && job->mesh && (job->type != TYPE_BONE || job->bone) &&
            job->slot < AssaResCount && job->slot >= 0 &&
            AssaRes[job->slot].type == job->type &&
            !AssaRes[job->slot].LoadFlag && !AssaRes[job->slot].res &&
            job->meshPath == AssaRes[job->slot].resFullName)
        {
            bool canPublish = true;
            if (job->type == TYPE_BONE)
                canPublish = job->slot > 0 &&
                    AssaRes[job->slot - 1].type == TYPE_BONE_ANI &&
                    !AssaRes[job->slot - 1].LoadFlag && !AssaRes[job->slot - 1].res &&
                    job->bonePath == AssaRes[job->slot - 1].resFullName;
            if (canPublish)
            {
                if (job->type == TYPE_BONE)
                {
                    AssaRes[job->slot - 1].res = job->bone;
                    AssaRes[job->slot - 1].LoadFlag = TRUE;
                    job->bone = NULL;
                }
                AssaRes[job->slot].res = job->mesh;
                AssaRes[job->slot].LoadFlag = TRUE;
                job->mesh = NULL;
                smPAT3D *resident = (smPAT3D *)AssaRes[job->slot].res;
                if (resident->smMaterialGroup)
                    AssetLoadV2ReadMaterialGroup(resident->smMaterialGroup, SMASSET_OWNER_EFFECT);
            }
        }
        SummitAssaDiscardOnOwner(*job);
    }
}

void SummitPrewarmAssaEffectModelsStep256(unsigned int maxRequests)
{
    if (AssaResCount <= 0 || !maxRequests) return;
    if (maxRequests > 2u) maxRequests = 2u;
    SummitAssaState256 &state = SummitAssaState();
    unsigned int submitted = 0;
    int scanned = 0;
    // Scan is bounded too: never walk the whole effect catalog on one frame.
    while (submitted < maxRequests && scanned < 48 && scanned < AssaResCount)
    {
        int index = 0;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            if (state.cursor >= AssaResCount) state.cursor = 0;
            index = state.cursor++;
        }
        ++scanned;
        const int type = AssaRes[index].type;
        if ((type != TYPE_ASE && type != TYPE_BONE) ||
            AssaRes[index].LoadFlag || AssaRes[index].res)
            continue;
        if (type == TYPE_BONE && (index == 0 || AssaRes[index - 1].type != TYPE_BONE_ANI ||
            AssaRes[index - 1].LoadFlag || AssaRes[index - 1].res))
            continue;

        const std::string mesh = AssaRes[index].resFullName;
        const std::string bone = type == TYPE_BONE ? AssaRes[index - 1].resFullName : "";
        if (mesh.empty() || (type == TYPE_BONE && bone.empty())) continue;
        unsigned long long generation = 0ull;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            if (state.pending.size() >= 8u || state.failed.count(index) ||
                !state.pending.insert(index).second)
                continue;
            generation = state.generation;
        }
        const std::shared_ptr<SummitAssaJob256> job(
            new(std::nothrow) SummitAssaJob256(index, type, generation, mesh, bone));
        if (!job)
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.pending.erase(index);
            continue;
        }
        const bool queued = summit::core::concurrency::JobSystem::Instance().Submit(
            [job]()
            {
                smASE_SetPhysique(NULL);
                if (!job->bonePath.empty())
                {
                    summit::core::concurrency::SourceAssetLock lock(job->bonePath.c_str());
                    char boneBuffer[MAX_RES_FULL_NAME];
                    if (job->bonePath.size() < sizeof(boneBuffer))
                    {
                        memcpy(boneBuffer, job->bonePath.c_str(), job->bonePath.size() + 1u);
                        job->bone = smASE_ReadBone(boneBuffer);
                    }
                    if (job->bone) smASE_SetPhysique(job->bone);
                }
                if (job->bonePath.empty() || job->bone)
                {
                    summit::core::concurrency::SourceAssetLock lock(job->meshPath.c_str());
                    char meshBuffer[MAX_RES_FULL_NAME];
                    if (job->meshPath.size() < sizeof(meshBuffer))
                    {
                        memcpy(meshBuffer, job->meshPath.c_str(), job->meshPath.size() + 1u);
                        job->mesh = smASE_Read(meshBuffer);
                        if (job->mesh) job->mesh->ZeroNormals();
                    }
                }
                smASE_SetPhysique(NULL);
                SummitAssaState256 &state = SummitAssaState();
                std::lock_guard<std::mutex> lock(state.mutex);
                state.complete.push_back(job);
            }, summit::core::concurrency::JobPriority::Low);
        if (!queued)
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.pending.erase(index);
        }
        else
            ++submitted;
    }
}
// SUMMIT_ASSA_ASYNC_256_END

bool GetLoadedFile(char *searchFileName)
{
	for(int index = 0; index < AssaResCount; index++)
	{
		if(AssaRes[index].type == TYPE_ASE || AssaRes[index].type == TYPE_BONE || AssaRes[index].type == TYPE_BONE_ANI)
		{
			if(stricmp(searchFileName, AssaRes[index].resName) == 0)
				return true;
		}
	}

	return false;
}


bool ExistAseFile(char *directory)
{
	HANDLE hSrch;
	WIN32_FIND_DATA wfd;
	BOOL bResult = TRUE;

	hSrch = FindFirstFile(directory, &wfd);
	if(hSrch == INVALID_HANDLE_VALUE)
		return false;
	while(bResult)
	{
		const char *tempExtName = strrchr(wfd.cFileName, '.');
		if(tempExtName) tempExtName++;
		else tempExtName = "";

		if(stricmp(tempExtName, "ase") == 0 || stricmp(tempExtName, "smd") == 0)
		{
			FindClose(hSrch);
			return true;
		}

		bResult = FindNextFile(hSrch, &wfd);
	}
	FindClose(hSrch);

	return false;
}

void LoadResFile(char *path, bool aseExistFlag)
{
	HANDLE hSrch;
	WIN32_FIND_DATA wfd;
	BOOL bResult = TRUE;
	char drive[MAX_RES_DRIVE] = { 0 };
	char dir[MAX_RES_PATH] = { 0 };
	char newpath[MAX_RES_PATH] = { 0 };
	hSrch = FindFirstFile(path, &wfd);
	if(hSrch == INVALID_HANDLE_VALUE)
		return;

	while(bResult)
	{
		_splitpath(path, drive, dir, NULL, NULL);
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(wfd.cFileName[0] != '.')
			{
                if (summit::effects::JoinCatalogPath251(newpath, drive, dir,
                    wfd.cFileName, "//*.*"))
                    LoadResFile(newpath, ExistAseFile(newpath));
			}
		}
		else
		{
			if(strlen(wfd.cFileName) > 4)
			{
                if (AssaResCount >= MAX_RES_BUFFER)
                    break; // release FindFirstFile handle below

				char fileDirectoryName[1024];
				memset(fileDirectoryName, 0, sizeof(fileDirectoryName));
                if (strlen(wfd.cFileName) >= MAX_RES_NAME ||
                    !summit::effects::JoinCatalogPath251(fileDirectoryName,
                        drive, dir, wfd.cFileName))
                {
                    bResult = FindNextFile(hSrch, &wfd);
                    continue;
                }
                if (strlen(fileDirectoryName) >= MAX_RES_FULL_NAME)
                {
                    bResult = FindNextFile(hSrch, &wfd);
                    continue;
                }

				const char *tempExtName = strrchr(wfd.cFileName, '.');
				if(tempExtName) tempExtName++;
				else tempExtName = "";

				if(aseExistFlag == false)
				{

					if(stricmp(tempExtName, "bmp") == 0 || stricmp(tempExtName, "tga") == 0)
					{

						strcpy(AssaRes[AssaResCount].resName, wfd.cFileName);

						AssaRes[AssaResCount].type = TYPE_MAT;
						strcpy(AssaRes[AssaResCount].resFullName, fileDirectoryName);


#ifdef _EFFECT_RES_LOADING_
						if(HoEffectResLoadingFlag)
						{
							if(AssaRes[AssaResCount].LoadFlag == FALSE)
							{
								int *matNum = new int;
								(*matNum) = CreateTextureMaterial(AssaRes[AssaResCount].resFullName, 0, 0, 0, 0, SMMAT_BLEND_LAMP);
								AssaRes[AssaResCount].res = matNum;
								AssaRes[AssaResCount].LoadFlag = TRUE;
								AssetLoadV2ReadMaterialGroup(smMaterialGroup, SMASSET_OWNER_EFFECT);
							}
						}
#endif
						AssaResCount++;
					}
				}


				if(stricmp(tempExtName, "ase") == 0 || stricmp(tempExtName, "smd") == 0)
				{

					char boneName[8];
					memset(boneName, 0, sizeof(boneName));
					memcpy(boneName, wfd.cFileName, 2);

					char aseName[256];
					memset(aseName, 0, sizeof(aseName));
					strcpy(aseName, wfd.cFileName);

					for(int index = strlen(aseName) - 1; index > 0; index--)
					{
						if(aseName[index] == '.')
						{
							aseName[index] = '\0';
							strcat(aseName, ".ase");
							break;
						}
					}

					memset(fileDirectoryName, 0, sizeof(fileDirectoryName));
                    if (!summit::effects::JoinCatalogPath251(fileDirectoryName,
                        drive, dir, aseName) ||
                        strlen(fileDirectoryName) >= MAX_RES_FULL_NAME)
                    {
                        bResult = FindNextFile(hSrch, &wfd);
                        continue;
                    }

					if(GetLoadedFile(aseName) == true)
					{
						bResult = FindNextFile(hSrch, &wfd);
						continue;
					}


                    if(stricmp(boneName, "b_") == 0)
                    {
                        // Bone + paired mesh occupy two adjacent catalog slots.
                        if (AssaResCount + 1 >= MAX_RES_BUFFER)
                        {
                            bResult = FindNextFile(hSrch, &wfd);
                            continue;
                        }
						strcpy(AssaRes[AssaResCount].resName, aseName);
						strcpy(AssaRes[AssaResCount].resFullName, fileDirectoryName);
						AssaRes[AssaResCount].type = TYPE_BONE_ANI;
						AssaResCount++;


						char tempName[64];
						memset(tempName, 0, sizeof(tempName));
						strcpy(tempName, aseName);
						for(int index = 0; index < (int)strlen(tempName); index++)
						{
							if(tempName[index] == '_')
							{
								strcpy(AssaRes[AssaResCount].resName, &tempName[index + 1]);
								break;
							}
						}
						strcpy(AssaRes[AssaResCount].resFullName, fileDirectoryName);

#ifdef _EFFECT_RES_LOADING_
						if(HoEffectResLoadingFlag)
						{
							if((AssaResCount - 1) >= 0)
							{

								if(AssaRes[AssaResCount - 1].LoadFlag == FALSE)
								{

									AssaRes[AssaResCount - 1].res = (void *)smASE_ReadBone(AssaRes[AssaResCount - 1].resFullName);
									AssaRes[AssaResCount - 1].LoadFlag = TRUE;
									smASE_SetPhysique(((smPAT3D *)AssaRes[AssaResCount - 1].res));
								}


								AssaRes[AssaResCount].res = (void *)smASE_Read(AssaRes[AssaResCount].resFullName);
								((smPAT3D *)AssaRes[AssaResCount].res)->ZeroNormals();
								smASE_SetPhysique(NULL);

								AssaRes[AssaResCount].LoadFlag = TRUE;
								AssetLoadV2ReadMaterialGroup(((smPAT3D *)AssaRes[AssaResCount].res)->smMaterialGroup, SMASSET_OWNER_EFFECT);
							}
						}

#endif
						AssaRes[AssaResCount].type = TYPE_BONE;
						AssaResCount++;
					}
					else
					{
						strcpy(AssaRes[AssaResCount].resName, aseName);
						strcpy(AssaRes[AssaResCount].resFullName, fileDirectoryName);

#ifdef _EFFECT_RES_LOADING_
						if(HoEffectResLoadingFlag)
						{
							if(AssaRes[AssaResCount].LoadFlag == FALSE)
							{
								smASE_SetPhysique(NULL);
								AssaRes[AssaResCount].res = (void *)smASE_Read(AssaRes[AssaResCount].resFullName);

								((smPAT3D *)AssaRes[AssaResCount].res)->ZeroNormals();
								AssaRes[AssaResCount].LoadFlag = TRUE;
								AssetLoadV2ReadMaterialGroup(((smPAT3D *)AssaRes[AssaResCount].res)->smMaterialGroup, SMASSET_OWNER_EFFECT);
							}
						}
#endif
						AssaRes[AssaResCount].type = TYPE_ASE;
						AssaResCount++;
					}
				}
			}
		}
		bResult = FindNextFile(hSrch, &wfd);
	}
	FindClose(hSrch);
}


void InitAssaRes()
{
    summitAssaLookup252.Invalidate();
    SummitResetAssaEffectModels256();
	AssaResCount = 0;
	memset(AssaRes, 0, sizeof(AssaRes));
}


void CloseAssaRes()
{
    summitAssaLookup252.Invalidate();
    SummitResetAssaEffectModels256();
    for(int index = 0; index < AssaResCount; index++)
    {
        if(AssaRes[index].res == NULL)
            continue;

        if(AssaRes[index].type == TYPE_MAT)
        {
            int *matNum = (int *)AssaRes[index].res;
            if(matNum && smMaterialGroup)
                AssetLoadV2ReleaseMaterial(smMaterialGroup, *matNum);
            delete matNum;
        }
        else if(AssaRes[index].type == TYPE_ASE ||
                AssaRes[index].type == TYPE_BONE ||
                AssaRes[index].type == TYPE_BONE_ANI)
        {
            delete ((smPAT3D *)AssaRes[index].res);
        }

        AssaRes[index].res = NULL;
        AssaRes[index].LoadFlag = FALSE;
    }

    // LoadAssaRes rescans the directories. Keeping the old count here caused
    // duplicate catalog entries and unbounded growth on every reload.
    AssaResCount = 0;
    memset(AssaRes, 0, sizeof(AssaRes));
    AssetLoadV2DumpTelemetry("ASSARES_CLEAR");
}



void LoadAssaRes()
{

	CloseAssaRes();

	char Path[MAX_RES_PATH] = { 0 };


	strcpy(Path, "image//Sinimage//AssaEffect//*.*");
	LoadResFile(Path, false);

	strcpy(Path, "Effect//AssaEffect//*.*");
	LoadResFile(Path, false);


}

void SummitPrewarmAssaEffectTexturesStep(int maxAttempts, int requestPriority)
{
    static int cursor = 0;
    if (maxAttempts <= 0 || AssaResCount <= 0 || !smMaterialGroup)
        return;

    int attempts = 0;
    int scanned = 0;
    while (attempts < maxAttempts && scanned < AssaResCount)
    {
        if (cursor >= AssaResCount)
            cursor = 0;
        const int index = cursor++;
        ++scanned;
        if (AssaRes[index].type != TYPE_MAT)
            continue;

        if (AssaRes[index].LoadFlag == FALSE)
        {
            int *matNum = new(std::nothrow) int;
            if (!matNum)
                continue;
            *matNum = CreateTextureMaterial(AssaRes[index].resFullName, 0, 0, 0, 0, SMMAT_BLEND_LAMP);
            if (*matNum < 0)
            {
                delete matNum;
                continue;
            }
            AssaRes[index].res = matNum;
            AssaRes[index].LoadFlag = TRUE;
        }

        if (!AssaRes[index].res)
            continue;
        const int matNum = *((int *)AssaRes[index].res);
        if (matNum < 0 || matNum >= static_cast<int>(smMaterialGroup->MaterialCount))
            continue;
        smMATERIAL &material = smMaterialGroup->smMaterial[matNum];
        if (!material.InUse || material.TextureCounter <= 0 || !material.smTexture[0])
            continue;

        AssetLoadV2RequestTexture(material.smTexture[0], SMASSET_OWNER_EFFECT, 1, requestPriority);
        ++attempts;
    }
}

void *AssaSearchRes(char *resName, int blendType)
{
    const int index = SummitFindAssaRes252(resName);
    if (index >= 0)
    {
			if(AssaRes[index].type == TYPE_ASE)
			{
				if(AssaRes[index].LoadFlag == FALSE)
				{
					smASE_SetPhysique(NULL);
					{
                        summit::core::concurrency::SourceAssetLock lock(AssaRes[index].resFullName);
                        AssaRes[index].res = (void *)smASE_Read(AssaRes[index].resFullName);
                    }
                    if (!AssaRes[index].res)
                        return NULL;

					((smPAT3D *)AssaRes[index].res)->ZeroNormals();
					AssaRes[index].LoadFlag = TRUE;
					AssetLoadV2ReadMaterialGroup(((smPAT3D *)AssaRes[index].res)->smMaterialGroup, SMASSET_OWNER_EFFECT);

				}

				if(((smPAT3D *)AssaRes[index].res)->smMaterialGroup)
				{
					int matCount = ((smPAT3D *)AssaRes[index].res)->smMaterialGroup->MaterialCount;
					for(int i = 0; i < matCount; i++)
						((smPAT3D *)AssaRes[index].res)->smMaterialGroup->smMaterial[i].BlendType = blendType;
				}

				return AssaRes[index].res;
			}
			else if(AssaRes[index].type == TYPE_BONE)
			{
                if (index == 0)
                    return NULL;
				if(AssaRes[index].LoadFlag == FALSE)
				{
					if((index - 1) >= 0)
					{

						if(AssaRes[index - 1].LoadFlag == FALSE)
						{

							{
                                summit::core::concurrency::SourceAssetLock lock(AssaRes[index - 1].resFullName);
                                AssaRes[index - 1].res = (void *)smASE_ReadBone(AssaRes[index - 1].resFullName);
                            }
                            if (!AssaRes[index - 1].res)
                                return NULL;
							AssaRes[index - 1].LoadFlag = TRUE;
						}

                        if (!AssaRes[index - 1].res)
                            return NULL;
						smASE_SetPhysique(((smPAT3D *)AssaRes[index - 1].res));


						{
                        summit::core::concurrency::SourceAssetLock lock(AssaRes[index].resFullName);
                        AssaRes[index].res = (void *)smASE_Read(AssaRes[index].resFullName);
                    }
                        smASE_SetPhysique(NULL);
                        if (!AssaRes[index].res)
                            return NULL;
						((smPAT3D *)AssaRes[index].res)->ZeroNormals();

						AssaRes[index].LoadFlag = TRUE;
						AssetLoadV2ReadMaterialGroup(((smPAT3D *)AssaRes[index].res)->smMaterialGroup, SMASSET_OWNER_EFFECT);

					}
				}

				if(((smPAT3D *)AssaRes[index].res)->smMaterialGroup)
				{
					int matCount = ((smPAT3D *)AssaRes[index].res)->smMaterialGroup->MaterialCount;
					for(int i = 0; i < matCount; i++)
						((smPAT3D *)AssaRes[index].res)->smMaterialGroup->smMaterial[i].BlendType = blendType;
				}

				return AssaRes[index].res;
			}
			else if(AssaRes[index].type == TYPE_BONE_ANI)
			{
                if (index == 0)
                    return NULL;
				if(AssaRes[index - 1].LoadFlag == FALSE)
				{


					{
                                summit::core::concurrency::SourceAssetLock lock(AssaRes[index - 1].resFullName);
                                AssaRes[index - 1].res = (void *)smASE_ReadBone(AssaRes[index - 1].resFullName);
                            }
                    if (!AssaRes[index - 1].res)
                        return NULL;
					AssaRes[index - 1].LoadFlag = TRUE;
				}
				return AssaRes[index].res;
			}
			else if(AssaRes[index].type == TYPE_MAT)
			{
				if(AssaRes[index].LoadFlag == FALSE)
				{
					int *matNum = new int;
					(*matNum) = CreateTextureMaterial(AssaRes[index].resFullName, 0, 0, 0, 0, SMMAT_BLEND_LAMP);
                    if (*matNum < 0 || !smMaterialGroup)
                    {
                        delete matNum;
                        return NULL;
                    }
					AssaRes[index].res = matNum;
					AssaRes[index].LoadFlag = TRUE;
				}

                if (!AssaRes[index].res || !smMaterialGroup)
                    return NULL;
				int matNum = *((int *)AssaRes[index].res);
                if (matNum < 0 || matNum >= static_cast<int>(smMaterialGroup->MaterialCount))
                    return NULL;
				smMATERIAL &material = smMaterialGroup->smMaterial[matNum];
				material.TwoSide = TRUE;
				material.BlendType = blendType;
				// A first-use effect must not render a white quad while its material
				// waits behind background streaming. Progressive prewarm normally
				// makes this a cache hit; if it did not, take over this exact texture
				// synchronously on the owner thread and invalidate any stale async job.
				if(material.TextureCounter > 0 && material.smTexture[0])
					AssetLoadV2LoadTextureNow(material.smTexture[0], SMASSET_OWNER_EFFECT, 1);
				return AssaRes[index].res;
			}

    }
    return NULL;
}


#define FLOATS_EM				(FLOATNS+3)
#define SM_DIST_NEARZ			16
#define SMFLOAT_DIST_NEARZ		(SM_DIST_NEARZ<<FLOATNS)

bool AssaGetCameraCoord(POINT3D &world)
{
	int rx, ry, rz;


	world.x -= smRender.CameraPosiX;
	world.y -= smRender.CameraPosiY;
	world.z -= smRender.CameraPosiZ;

	rx = world.x * smRender.mCamera._11 +
		world.y * smRender.mCamera._21 +
		world.z * smRender.mCamera._31;

	ry = world.x * smRender.mCamera._12 +
		world.y * smRender.mCamera._22 +
		world.z * smRender.mCamera._32;

	rz = world.x * smRender.mCamera._13 +
		world.y * smRender.mCamera._23 +
		world.z * smRender.mCamera._33;

	world.x = rx >> FLOATS_EM;
	world.y = ry >> FLOATS_EM;
	world.z = rz >> FLOATS_EM;

	if(world.z < SMFLOAT_DIST_NEARZ)
		return FALSE;
	return TRUE;
}


bool AssaAddFace2D(smFACE2D *face, int ZPosiFlag, int angle)
{
	int x, y, z;
	int rx, ry, rz;
	int width, height;

	smRENDVERTEX *v[4];
	smRENDFACE	 *rf;
	short		sColor[4];

	sColor[SMC_A] = face->Transparency;
	sColor[SMC_R] = face->r;
	sColor[SMC_G] = face->g;
	sColor[SMC_B] = face->b;

	x = face->x;
	y = face->y;
	z = face->z;


	x -= smRender.CameraPosiX;
	y -= smRender.CameraPosiY;
	z -= smRender.CameraPosiZ;

	rx = x * smRender.mCamera._11 +
		y * smRender.mCamera._21 +
		z * smRender.mCamera._31;

	ry = x * smRender.mCamera._12 +
		y * smRender.mCamera._22 +
		z * smRender.mCamera._32;

	rz = x * smRender.mCamera._13 +
		y * smRender.mCamera._23 +
		z * smRender.
		mCamera._33;

	x = rx >> FLOATS_EM;
	y = ry >> FLOATS_EM;
	z = rz >> FLOATS_EM;


	if(ZPosiFlag)
		z -= 24 * fONE;


	if(z < SMFLOAT_DIST_NEARZ) return false;

	width = face->width >> 1;
	height = face->height >> 1;

	if(angle != 0)
	{

		int		ex[4], ey[4];
		int		mx, my;
		int		sin, cos;

		sin = GetSin[angle&ANGCLIP] >> FLOATNS;
		cos = GetCos[angle&ANGCLIP] >> FLOATNS;

		mx = -width;	my = -height;
		ex[0] = (mx * cos - my * sin) >> FLOATNS;
		ey[0] = (mx * sin + my * cos) >> FLOATNS;

		mx = width;	my = -height;
		ex[1] = (mx * cos - my * sin) >> FLOATNS;
		ey[1] = (mx * sin + my * cos) >> FLOATNS;

		mx = -width;	my = +height;
		ex[2] = (mx * cos - my * sin) >> FLOATNS;
		ey[2] = (mx * sin + my * cos) >> FLOATNS;

		mx = width;	my = height;
		ex[3] = (mx * cos - my * sin) >> FLOATNS;
		ey[3] = (mx * sin + my * cos) >> FLOATNS;



		v[0] = smRender.AddRendVertex(x + ex[0], y + ey[0], z, sColor);
		v[1] = smRender.AddRendVertex(x + ex[1], y + ey[1], z, sColor);
		v[2] = smRender.AddRendVertex(x + ex[2], y + ey[2], z, sColor);
		v[3] = smRender.AddRendVertex(x + ex[3], y + ey[3], z, sColor);
	}
	else
	{

		v[0] = smRender.AddRendVertex(x - width, y - height, z, sColor);
		v[1] = smRender.AddRendVertex(x + width, y - height, z, sColor);
		v[2] = smRender.AddRendVertex(x - width, y + height, z, sColor);
		v[3] = smRender.AddRendVertex(x + width, y + height, z, sColor);
	}


	rf = &smRender.RendFace[smRender.nRendFace++];
	rf->lpRendVertex[0] = v[0];
	rf->lpRendVertex[1] = v[1];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial = face->MatNum;
	rf->ClipStatus = v[0]->ClipStatus | v[1]->ClipStatus | v[2]->ClipStatus;
	rf->lpTexLink = 0;


	smRender.AddRendTempTexLink(rf, 0, face->TexRect.left, face->TexRect.bottom,
		face->TexRect.right, face->TexRect.bottom,
		face->TexRect.left, face->TexRect.top);



	rf = &smRender.RendFace[smRender.nRendFace++];
	rf->lpRendVertex[0] = v[1];
	rf->lpRendVertex[1] = v[3];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial = face->MatNum;
	rf->ClipStatus = v[1]->ClipStatus | v[2]->ClipStatus | v[3]->ClipStatus;
	rf->lpTexLink = 0;


	smRender.AddRendTempTexLink(rf, 0, face->TexRect.right, face->TexRect.bottom,
		face->TexRect.right, face->TexRect.top,
		face->TexRect.left, face->TexRect.top);


	return true;
}


bool AssaAddFace2DLine(smFACE2D *face, POINT3D *localDestPos, BOOL centerFlag)
{
	int x, y, z;
	int width;

	smRENDVERTEX *v[4];
	smRENDFACE	 *rf;
	short		sColor[4];

	sColor[SMC_A] = face->Transparency;
	sColor[SMC_R] = face->r;
	sColor[SMC_G] = face->g;
	sColor[SMC_B] = face->b;

	x = face->x;
	y = face->y;
	z = face->z;

	POINT3D currentPos;
	POINT3D destPos;
	width = face->width / 2;

	currentPos.x = x;
	currentPos.y = y;
	currentPos.z = z;

	destPos.x = localDestPos->x;
	destPos.y = localDestPos->y;
	destPos.z = localDestPos->z;

	if(centerFlag == FALSE)
	{
		destPos.x = currentPos.x + destPos.x;
		destPos.y = currentPos.y + destPos.y;
		destPos.z = currentPos.z + destPos.z;
	}
	else
	{
		POINT3D centerDestPos;
		memcpy(&centerDestPos, &destPos, sizeof(centerDestPos));
		centerDestPos.x = centerDestPos.x / 2;
		centerDestPos.y = centerDestPos.y / 2;
		centerDestPos.z = centerDestPos.z / 2;

		destPos.x = currentPos.x + centerDestPos.x;
		destPos.y = currentPos.y + centerDestPos.y;
		destPos.z = currentPos.z + centerDestPos.z;

		currentPos.x = currentPos.x - (centerDestPos.x);
		currentPos.y = currentPos.y - (centerDestPos.y);
		currentPos.z = currentPos.z - (centerDestPos.z);
	}



	if(AssaGetCameraCoord(currentPos) == FALSE)
		return FALSE;

	if(AssaGetCameraCoord(destPos) == FALSE)
		return FALSE;




	sColor[SMC_A] = face->Transparency;
	sColor[SMC_R] = face->r;
	sColor[SMC_G] = face->g;
	sColor[SMC_B] = face->b;

	smTEXRECT texRect;
	texRect.left = 0;
	texRect.top = 0;
	texRect.right = 1;
	texRect.bottom = 1;

	float dx = float(destPos.x - currentPos.x);
	float dy = float(destPos.y - currentPos.y);
	float length = (float)sqrt(dx*dx + dy*dy);

	dx = dx / length*width;
	dy = dy / length*width;

	D3DXVECTOR3 persp;
	persp.x = -dy;
	persp.y = +dx;
	persp.z = 0;

	if(abs((int)dx) > 1.0f) currentPos.x -= (int)dx;
	if(abs((int)dy) > 1.0f) currentPos.y -= (int)dy;

	v[0] = smRender.AddRendVertex(int(destPos.x - persp.x), int(destPos.y - persp.y), int(destPos.z), sColor);
	v[1] = smRender.AddRendVertex(int(destPos.x + persp.x), int(destPos.y + persp.y), int(destPos.z), sColor);
	v[2] = smRender.AddRendVertex(int(currentPos.x - persp.x), int(currentPos.y - persp.y), int(currentPos.z), sColor);
	v[3] = smRender.AddRendVertex(int(currentPos.x + persp.x), int(currentPos.y + persp.y), int(currentPos.z), sColor);


	rf = &smRender.RendFace[smRender.nRendFace++];
	rf->lpRendVertex[0] = v[0];
	rf->lpRendVertex[1] = v[1];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial = face->MatNum;
	rf->ClipStatus = v[0]->ClipStatus | v[1]->ClipStatus | v[2]->ClipStatus;
	rf->lpTexLink = 0;


	smRender.AddRendTempTexLink(rf, 0, texRect.left, texRect.bottom,
		texRect.right, texRect.bottom,
		texRect.left, texRect.top);



	rf = &smRender.RendFace[smRender.nRendFace++];
	rf->lpRendVertex[0] = v[1];
	rf->lpRendVertex[1] = v[3];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial = face->MatNum;
	rf->ClipStatus = v[1]->ClipStatus | v[2]->ClipStatus | v[3]->ClipStatus;
	rf->lpTexLink = 0;


	smRender.AddRendTempTexLink(rf, 0, texRect.right, texRect.bottom,
		texRect.right, texRect.top,
		texRect.left, texRect.top);
	return true;
}



bool AssaAddFaceWorld(smFACE2D *face, POINT3D *inAngle)
{
	int x, y, z;
	int width, height;

	smRENDVERTEX *v[4];
	smRENDFACE	 *rf;
	short		sColor[4];

	sColor[SMC_A] = face->Transparency;
	sColor[SMC_R] = face->r;
	sColor[SMC_G] = face->g;
	sColor[SMC_B] = face->b;

	x = face->x;
	y = face->y;
	z = face->z;

	POINT3D inVertex[4];
	POINT3D outVertex[4];

	width = face->width / 2;
	height = face->height / 2;

	inVertex[0].x = x - width;
	inVertex[0].y = y;
	inVertex[0].z = z + height;

	inVertex[1].x = x + width;
	inVertex[1].y = y;
	inVertex[1].z = z + height;

	inVertex[2].x = x - width;
	inVertex[2].y = y;
	inVertex[2].z = z - height;

	inVertex[3].x = x + width;
	inVertex[3].y = y;
	inVertex[3].z = z - height;

	int index = 0;
	if(inAngle != NULL)
	{
		inVertex[0].x = -width;
		inVertex[0].y = 0;
		inVertex[0].z = height;

		inVertex[1].x = width;
		inVertex[1].y = 0;
		inVertex[1].z = height;

		inVertex[2].x = -width;
		inVertex[2].y = 9;
		inVertex[2].z = -height;

		inVertex[3].x = width;
		inVertex[3].y = 0;
		inVertex[3].z = -height;

		POINT3D angle;
		angle.x = (int)inAngle->x;
		angle.y = (int)inAngle->y;
		angle.z = (int)inAngle->z;

		angle.x = angle.x & ANGCLIP;
		angle.y = angle.y & ANGCLIP;
		angle.z = angle.z & ANGCLIP;

		smMATRIX inRotXMatrix;
		smMATRIX inRotYMatrix;
		smMATRIX inRotZMatrix;

		smMATRIX outMatrix;
		smIdentityMatrix(outMatrix);
		smIdentityMatrix(inRotXMatrix);
		smIdentityMatrix(inRotYMatrix);
		smIdentityMatrix(inRotZMatrix);

		smRotateXMatrix(inRotXMatrix, angle.x);
		smRotateYMatrix(inRotYMatrix, angle.y);
		smRotateZMatrix(inRotZMatrix, angle.z);

		smMatrixMult(outMatrix, inRotXMatrix, inRotYMatrix);
		smMatrixMult(outMatrix, outMatrix, inRotZMatrix);

		for(index = 0; index < 4; index++)
		{
			outVertex[index].x = inVertex[index].x * outMatrix._11 +
				inVertex[index].y * outMatrix._21 +
				inVertex[index].z * outMatrix._31;


			outVertex[index].y = inVertex[index].x * outMatrix._12 +
				inVertex[index].y * outMatrix._22 +
				inVertex[index].z * outMatrix._32;


			outVertex[index].z = inVertex[index].x * outMatrix._13 +
				inVertex[index].y * outMatrix._23 +
				inVertex[index].z * outMatrix._33;

			outVertex[index].x = outVertex[index].x / fONE + x;
			outVertex[index].y = outVertex[index].y / fONE + y;
			outVertex[index].z = outVertex[index].z / fONE + z;

		}

		memcpy(inVertex, outVertex, sizeof(inVertex));
	}


	for(index = 0; index < 4; index++)
	{
		inVertex[index].x -= smRender.CameraPosiX;
		inVertex[index].y -= smRender.CameraPosiY;
		inVertex[index].z -= smRender.CameraPosiZ;
	}

	for(index = 0; index < 4; index++)
	{
		outVertex[index].x = inVertex[index].x*smRender.mCamera._11 +
			inVertex[index].y*smRender.mCamera._21 +
			inVertex[index].z*smRender.mCamera._31;

		outVertex[index].y = inVertex[index].x*smRender.mCamera._12 +
			inVertex[index].y*smRender.mCamera._22 +
			inVertex[index].z*smRender.mCamera._32;

		outVertex[index].z = inVertex[index].x*smRender.mCamera._13 +
			inVertex[index].y*smRender.mCamera._23 +
			inVertex[index].z*smRender.mCamera._33;

		outVertex[index].x = outVertex[index].x >> FLOATS_EM;
		outVertex[index].y = outVertex[index].y >> FLOATS_EM;
		outVertex[index].z = outVertex[index].z >> FLOATS_EM;

		if(outVertex[index].z < SMFLOAT_DIST_NEARZ)
			return false;
	}



	v[0] = smRender.AddRendVertex(outVertex[0].x, outVertex[0].y, outVertex[0].z, sColor);
	v[1] = smRender.AddRendVertex(outVertex[1].x, outVertex[1].y, outVertex[1].z, sColor);
	v[2] = smRender.AddRendVertex(outVertex[2].x, outVertex[2].y, outVertex[2].z, sColor);
	v[3] = smRender.AddRendVertex(outVertex[3].x, outVertex[3].y, outVertex[3].z, sColor);


	rf = &smRender.RendFace[smRender.nRendFace++];
	rf->lpRendVertex[0] = v[0];
	rf->lpRendVertex[1] = v[1];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial = face->MatNum;
	rf->ClipStatus = v[0]->ClipStatus | v[1]->ClipStatus | v[2]->ClipStatus;
	rf->lpTexLink = 0;


	smRender.AddRendTempTexLink(rf, 0, face->TexRect.left, face->TexRect.bottom,
		face->TexRect.right, face->TexRect.bottom,
		face->TexRect.left, face->TexRect.top);



	rf = &smRender.RendFace[smRender.nRendFace++];
	rf->lpRendVertex[0] = v[1];
	rf->lpRendVertex[1] = v[3];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial = face->MatNum;
	rf->ClipStatus = v[1]->ClipStatus | v[2]->ClipStatus | v[3]->ClipStatus;
	rf->lpTexLink = 0;


	smRender.AddRendTempTexLink(rf, 0, face->TexRect.right, face->TexRect.bottom,
		face->TexRect.right, face->TexRect.top,
		face->TexRect.left, face->TexRect.top);
	return true;
}



bool AssaAddFaceTrace(smFACE2D *face, cASSATrace *assaTrace)
{
	if(face == NULL || assaTrace == NULL)
		return false;

	if(assaTrace->TraceList.size() < 2)
		return FALSE;

	POINT3D destPos;
	POINT3D currentPos;

	std::list<POINT3D>::iterator i;
	i = assaTrace->TraceList.begin();
	destPos.x = i->x;
	destPos.y = i->y;
	destPos.z = i->z;

	int texSizeCount = 0;
	int test = assaTrace->TraceList.size();
	float cellSize = 1.f / (assaTrace->TraceList.size() - 1);
	++i;
	POINT3D OldVertex[2];
	memset(OldVertex, 0, sizeof(OldVertex));

	for(; i != assaTrace->TraceList.end(); i++)
	{
		currentPos.x = i->x;
		currentPos.y = i->y;
		currentPos.z = i->z;

		if(AssaGetCameraCoord(currentPos) == FALSE)
			return FALSE;

		if(AssaGetCameraCoord(destPos) == FALSE)
			return FALSE;

		smRENDVERTEX *v[4];
		smRENDFACE	 *rf;

		short		sColor[4];

		sColor[SMC_R] = face->r;
		sColor[SMC_G] = face->g;
		sColor[SMC_B] = face->b;
		sColor[SMC_A] = face->Transparency;

		smTEXRECT texRect;
		texRect.left = 0;
		texRect.top = (float)texSizeCount*cellSize;
		texRect.right = 1;
		texRect.bottom = (float)(texSizeCount + 1)*cellSize;


		int size;
		size = (int)face->width;

		size = size >> 1;

		float dx = float(destPos.x - currentPos.x);
		float dy = float(destPos.y - currentPos.y);
		float length = (float)sqrt(dx*dx + dy*dy);

		if(length == 0)
			length = 1;

		dx = dx / length*size;
		dy = dy / length*size;

		D3DXVECTOR3 persp;
		persp.x = -dy;
		persp.y = +dx;
		persp.z = 0;

		if(texSizeCount == 0)
		{
			v[0] = smRender.AddRendVertex(int(destPos.x - persp.x), int(destPos.y - persp.y), int(destPos.z), sColor);
			v[1] = smRender.AddRendVertex(int(destPos.x + persp.x), int(destPos.y + persp.y), int(destPos.z), sColor);
		}
		else
		{
			v[0] = smRender.AddRendVertex(OldVertex[0].x, OldVertex[0].y, OldVertex[0].z, sColor);
			v[1] = smRender.AddRendVertex(OldVertex[1].x, OldVertex[1].y, OldVertex[1].z, sColor);
		}

		OldVertex[0].x = int(currentPos.x - persp.x);
		OldVertex[0].y = int(currentPos.y - persp.y);
		OldVertex[0].z = int(currentPos.z);

		OldVertex[1].x = int(currentPos.x + persp.x);
		OldVertex[1].y = int(currentPos.y + persp.y);
		OldVertex[1].z = int(currentPos.z);

		destPos.x = int(i->x);
		destPos.y = int(i->y);
		destPos.z = int(i->z);

		v[2] = smRender.AddRendVertex(int(currentPos.x - persp.x), int(currentPos.y - persp.y), int(currentPos.z), sColor);
		v[3] = smRender.AddRendVertex(int(currentPos.x + persp.x), int(currentPos.y + persp.y), int(currentPos.z), sColor);


		rf = &smRender.RendFace[smRender.nRendFace++];
		rf->lpRendVertex[0] = v[0];
		rf->lpRendVertex[1] = v[1];
		rf->lpRendVertex[2] = v[2];
		rf->Matrial = face->MatNum;
		rf->ClipStatus = v[0]->ClipStatus | v[1]->ClipStatus | v[2]->ClipStatus;
		rf->lpTexLink = 0;


		smRender.AddRendTempTexLink(rf, 0, texRect.left, texRect.top,
			texRect.right, texRect.top,
			texRect.left, texRect.bottom);



		rf = &smRender.RendFace[smRender.nRendFace++];
		rf->lpRendVertex[0] = v[1];
		rf->lpRendVertex[1] = v[3];
		rf->lpRendVertex[2] = v[2];
		rf->Matrial = face->MatNum;
		rf->ClipStatus = v[1]->ClipStatus | v[2]->ClipStatus | v[3]->ClipStatus;
		rf->lpTexLink = 0;


		smRender.AddRendTempTexLink(rf, 0, texRect.right, texRect.top,
			texRect.right, texRect.bottom,
			texRect.left, texRect.bottom);

		texSizeCount++;
	}
	return true;
}



bool AssaSearchObjPos(smCHAR *character, smOBJ3D *obj, POINT3D *outObjPos)
{
	if(character == NULL ||
		obj == NULL)
		return false;

	smMATRIX *mWorld;
	mWorld = &obj->mWorld;
	POINT3D angle;
	memcpy(&angle, &character->Angle, sizeof(POINT3D));
	angle.y = (-angle.y + ANGLE_180)&ANGCLIP;
	AnimObjectTree(obj, character->frame, angle.x, angle.y, angle.z);

	outObjPos->x = character->pX + mWorld->_41;
	outObjPos->z = character->pZ + mWorld->_42;
	outObjPos->y = character->pY + mWorld->_43;

	return true;
}


bool AssaSearchObjPos(smCHAR *character, smOBJ3D *obj, POINT3D *outObjPos, int length)
{
	if(character == NULL ||
		obj == NULL)
		return false;

	smMATRIX *mWorld;
	mWorld = &obj->mWorld;
	POINT3D angle;
	memcpy(&angle, &character->Angle, sizeof(POINT3D));
	angle.y = (-angle.y + ANGLE_180)&ANGCLIP;
	AnimObjectTree(obj, character->frame, angle.x, angle.y, angle.z);

	POINT3D pBot;
	pBot.x = 0;
	pBot.y = 0;
	pBot.z = length;

	int rx, ry, rz;
	rx = pBot.x * mWorld->_11 +
		pBot.y * mWorld->_21 +
		pBot.z * mWorld->_31;

	ry = pBot.x * mWorld->_12 +
		pBot.y * mWorld->_22 +
		pBot.z * mWorld->_32;

	rz = pBot.x * mWorld->_13 +
		pBot.y * mWorld->_23 +
		pBot.z * mWorld->_33;

	outObjPos->x = character->pX + (rx >> FLOATNS) + mWorld->_41;
	outObjPos->z = character->pZ + (ry >> FLOATNS) + mWorld->_42;
	outObjPos->y = character->pY + (rz >> FLOATNS) + mWorld->_43;





	return true;
}


bool AssaSearchWeaponPos(smCHAR *character, POINT3D *outWeaponPos, float u)
{
	if(character == NULL)
		return false;

	smMATRIX *mWorld;
	mWorld = &character->HvRightHand.ObjBip->mWorld;

	POINT3D pBot;
	pBot.x = 0;
	pBot.y = 0;

	if(character->ShootingFlag)
	{
		outWeaponPos->x = character->ShootingPosi.x;
		outWeaponPos->y = character->ShootingPosi.y;
		outWeaponPos->z = character->ShootingPosi.z;
		return true;
	}

	if((character->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWT1)
	{
		pBot.z = -character->HvRightHand.SizeMax + int((float)character->HvRightHand.SizeMax*u);
	}
	else
	{
		pBot.z = character->HvRightHand.SizeMax - int((float)character->HvRightHand.SizeMax*u);
	}

	POINT3D angle;
	memcpy(&angle, &character->Angle, sizeof(POINT3D));
	angle.y = (-angle.y + ANGLE_180)&ANGCLIP;

	AnimObjectTree(character->HvRightHand.ObjBip, character->frame, angle.x, angle.y, angle.z);
	int rx, ry, rz;
	rx = pBot.x * mWorld->_11 +
		pBot.y * mWorld->_21 +
		pBot.z * mWorld->_31;

	ry = pBot.x * mWorld->_12 +
		pBot.y * mWorld->_22 +
		pBot.z * mWorld->_32;

	rz = pBot.x * mWorld->_13 +
		pBot.y * mWorld->_23 +
		pBot.z * mWorld->_33;

	outWeaponPos->x = character->pX + (rx >> FLOATNS) + mWorld->_41;
	outWeaponPos->z = character->pZ + (ry >> FLOATNS) + mWorld->_42;
	outWeaponPos->y = character->pY + (rz >> FLOATNS) + mWorld->_43;

	return true;
}



static char sinDecode[512] = { 0 };
static char sinLine[512] = { 0 };
int AssaTestEffectImage()
{

	char *szFilePath = "image//Sinimage//AssaEffect//AssaTest.INI";

	FILE	*fp;
	char strBuff[64] = { 0 };

	char *p;
	int Count = 0;
	int NextFlag = 0;
	int Index = 0;
	int sinMeshCount = 0;

	fp = fopen(szFilePath, "rb");
	if(fp == NULL) return FALSE;


	while(!feof(fp))
	{
		NextFlag = 0;
		if(fgets(sinLine, 255, fp) == NULL)
			break;

		p = GetWord(sinDecode, sinLine);



		if(!NextFlag && lstrcmp(sinDecode, "*�����̸�") == 0)
		{
			char boneName2[8];
			char MeshName[32];
			memset(boneName2, 0, sizeof(boneName2));
			memcpy(boneName2, strBuff, 2);
			p = GetWord(strBuff, p);
			if(stricmp(boneName2, "b_") == 0)
			{
				cAssaEffect[Index]->Bone = 1;
				Index = SetAssaEffect(0, strBuff, lpCurPlayer, 0, 0);
				strcpy_s(MeshName, &strBuff[2]);
				Index = SetAssaEffect(0, MeshName, lpCurPlayer, 0, 0);

			}
			else
			{
				Index = SetAssaEffect(0, strBuff, lpCurPlayer, 0, 0);
			}


			cAssaEffect[Index]->Angle.y = (-(lpCurPlayer->Angle.y&ANGCLIP) + ANGLE_180)&ANGCLIP;
			NextFlag = TRUE;

		}

		if(!NextFlag && lstrcmp(sinDecode, "*������") == 0)
		{
			p = GetWord(strBuff, p);
			cAssaEffect[Index]->AniMaxCount = atoi(strBuff);
			NextFlag = TRUE;

		}

		if(!NextFlag && lstrcmp(sinDecode, "*�����ӵ�����") == 0)
		{
			p = GetWord(strBuff, p);
			cAssaEffect[Index]->AniDelayTime = atoi(strBuff);
			NextFlag = TRUE;

		}
		if(!NextFlag && lstrcmp(sinDecode, "*����") == 0)
		{
			p = GetWord(strBuff, p);
			cAssaEffect[Index]->Posi.y += atoi(strBuff);
			NextFlag = TRUE;

		}

	}
	if(fp) fclose(fp);
	return TRUE;

}




