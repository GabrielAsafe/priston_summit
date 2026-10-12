#include "systems/rendering/effects/legacy_new/core/HoEffectResMgr.h"
#ifndef _SERVER
#include "core/concurrency/JobSystem.h"
#include "core/concurrency/SourceAssetLock.h"
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <new>
#include <set>
#endif
HoEffectResTexture::HoEffectResTexture()
    : m_iMatNum(-1)
{
}

HoEffectResTexture::~HoEffectResTexture()
{
#ifndef _SERVER
    if(m_iMatNum >= 0 && smMaterialGroup)
    {
        AssetLoadV2ReleaseMaterial(smMaterialGroup, m_iMatNum);
        m_iMatNum = -1;
    }
#endif
}

bool HoEffectResTexture::LoadingRes(const std::string &fileName)
{
    // Do not truncate a requested texture into a different resource identity.
    if (fileName.size() >= 128u)
        return false;
    char tempBuffer[128];
    strcpy(tempBuffer, fileName.c_str());

    m_iMatNum = CreateTextureMaterial(tempBuffer, 0, 0, 0, 0, SMMAT_BLEND_LAMP);
    if(m_iMatNum < 0 || !smMaterialGroup)
        return false;
#ifndef _SERVER
    AssetLoadV2ReadMaterialGroup(smMaterialGroup, SMASSET_OWNER_EFFECT);
#else
    smMaterialGroup->ReadTextures();
#endif

    return true;
}

int HoEffectResTexture::GetRes()
{
    return m_iMatNum;
}

HoEffectResMesh::HoEffectResMesh():m_Mesh(NULL),
                                   m_Bone(NULL)
{
}

HoEffectResMesh::~HoEffectResMesh()
{
    if(m_Mesh != NULL)
    {
        delete m_Mesh;
        m_Mesh = NULL;
    }

    if(m_Bone != NULL)
    {
        delete m_Bone;
        m_Bone = NULL;
    }
}

bool HoEffectResMesh::LoadingRes(const std::string &meshName, bool deferMaterialRead)
{
    if (meshName.size() >= 128u)
        return false;
    char tempBuffer[128];
    strcpy(tempBuffer, meshName.c_str());

#ifndef _SERVER
    smASE_SetPhysique(NULL);
    {
        summit::core::concurrency::SourceAssetLock sourceLock(tempBuffer);
        m_Mesh = smASE_Read(tempBuffer);
    }
#else
    smASE_SetPhysique(NULL);
    m_Mesh = smASE_Read(tempBuffer);
#endif
    if(m_Mesh == NULL)
        return false;

    m_Mesh->ZeroNormals();

    if(m_Mesh->smMaterialGroup)
    {
#ifndef _SERVER
        // Worker decode must not publish materials or touch Direct3D.
        if (!deferMaterialRead)
            AssetLoadV2ReadMaterialGroup(m_Mesh->smMaterialGroup, SMASSET_OWNER_EFFECT);
#else
        (void)deferMaterialRead;
        m_Mesh->smMaterialGroup->ReadTextures();
#endif
        int matCount = m_Mesh->smMaterialGroup->MaterialCount;
        for(int i = 0; i < matCount; i++)
        {
            m_Mesh->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
        }
    }
    return true;
}

#ifndef _SERVER
void HoEffectResMesh::FinishMaterialReadOnOwnerThread()
{
    if (m_Mesh && m_Mesh->smMaterialGroup)
        AssetLoadV2ReadMaterialGroup(m_Mesh->smMaterialGroup, SMASSET_OWNER_EFFECT);
}
#endif

smPAT3D *HoEffectResMesh::GetRes()
{
    if(m_Mesh == NULL)
        return NULL;

    return m_Mesh;
}

smPAT3D *HoEffectResMesh::GetBoneRes()
{
	if(m_Bone == NULL)
		return NULL;

	return m_Bone;
}

bool HoEffectResMesh::LoadingRes(const std::string &meshName, const std::string &boneName, bool deferMaterialRead)
{
    if (meshName.size() >= 128u || boneName.size() >= 128u)
        return false;
    char tempBuffer1[128];
    char tempBuffer2[128];
    strcpy(tempBuffer1, meshName.c_str());
    strcpy(tempBuffer2, boneName.c_str());

    // The old overload accidentally loaded the mesh path as the bone resource.
#ifndef _SERVER
    {
        summit::core::concurrency::SourceAssetLock sourceLock(tempBuffer2);
        m_Bone = smASE_ReadBone(tempBuffer2);
    }
#else
    m_Bone = smASE_ReadBone(tempBuffer2);
#endif
    if(m_Bone == NULL)
        return false;
    smASE_SetPhysique(m_Bone);

#ifndef _SERVER
    {
        summit::core::concurrency::SourceAssetLock sourceLock(tempBuffer1);
        m_Mesh = smASE_Read(tempBuffer1);
    }
#else
    m_Mesh = smASE_Read(tempBuffer1);
#endif
    smASE_SetPhysique(NULL); // Paired-bone state must not leak into later loads.
    if(m_Mesh == NULL)
        return false;
    m_Mesh->ZeroNormals();

    if(m_Mesh->smMaterialGroup)
    {
#ifndef _SERVER
        // Worker decode must not publish materials or touch Direct3D.
        if (!deferMaterialRead)
            AssetLoadV2ReadMaterialGroup(m_Mesh->smMaterialGroup, SMASSET_OWNER_EFFECT);
#else
        (void)deferMaterialRead;
        m_Mesh->smMaterialGroup->ReadTextures();
#endif
        int matCount = m_Mesh->smMaterialGroup->MaterialCount;
        for(int i = 0; i < matCount; i++)
        {
            m_Mesh->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
        }
    }

    return true;
}

HoEffectResManager::HoEffectResManager()
{
}

HoEffectResManager::~HoEffectResManager()
{
    ResTextureMapIT M_stIT	= m_ResTextureMap.begin();
	ResTextureMapIT	M_endIT	= m_ResTextureMap.end();

	for(;M_stIT != M_endIT;)
	{
		delete M_stIT->second;
		++M_stIT;
	}

    ResMeshMapIT B_stIT	 = m_ResMeshMap.begin();
	ResMeshMapIT B_endIT = m_ResMeshMap.end();

	for(;B_stIT != B_endIT;)
	{
		delete B_stIT->second;
		++B_stIT;
	}

	m_ResTextureMap.clear();
	m_ResMeshMap.clear();
}

HoEffectResManager* HoEffectResManager::m_Instance = 0;
HoEffectResManager* HoEffectResManager::GetInstance()
{
    if(m_Instance == 0)
        m_Instance = new HoEffectResManager;
    return m_Instance;
}

smPAT3D *HoEffectResManager::GetResMesh(const std::string &meshName, const std::string &boneName)
{
    if (meshName.empty() || meshName.size() >= 128u || boneName.size() >= 128u)
        return NULL;
    const ResMeshKey key = summit::effects::MakeEffectMeshKey249(meshName, boneName);
    const ResMeshMapIT existing = m_ResMeshMap.find(key);
    if (existing != m_ResMeshMap.end())
        return existing->second->GetRes();

    HoEffectResMesh *resMesh = new HoEffectResMesh;
    const bool loaded = boneName.empty()
        ? resMesh->LoadingRes(meshName)
        : resMesh->LoadingRes(meshName, boneName);
    if (!loaded || !resMesh->GetRes())
    {
        delete resMesh;
        return NULL;
    }
    m_ResMeshMap[key] = resMesh;
    return resMesh->GetRes();
}

#ifndef _SERVER
namespace
{
struct SummitEffectMeshJob255
{
    summit::effects::EffectMeshKey249 key;
    unsigned long long generation;
    HoEffectResMesh *prepared;
    SummitEffectMeshJob255(const summit::effects::EffectMeshKey249 &k, unsigned long long g)
        : key(k), generation(g), prepared(NULL) {}
};

struct SummitEffectMeshState255
{
    std::mutex mutex;
    std::set<summit::effects::EffectMeshKey249> pending;
    std::set<summit::effects::EffectMeshKey249> failed;
    std::deque<std::shared_ptr<SummitEffectMeshJob255> > completed;
    unsigned long long generation;
    SummitEffectMeshState255() : generation(1ull) {}
};

static SummitEffectMeshState255 &SummitEffectMeshState()
{
    // Process-lifetime: a queued job cannot outlive its state on shutdown.
    static SummitEffectMeshState255 *state = new SummitEffectMeshState255();
    return *state;
}
}

void HoEffectResManager::RequestResMesh(const std::string &meshName, const std::string &boneName)
{
    if (meshName.empty() || meshName.size() >= 128u || boneName.size() >= 128u)
        return;
    const ResMeshKey key = summit::effects::MakeEffectMeshKey249(meshName, boneName);
    if (m_ResMeshMap.find(key) != m_ResMeshMap.end())
        return;

    SummitEffectMeshState255 &state = SummitEffectMeshState();
    unsigned long long generation = 0ull;
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        // Bound both queued and completed work: gameplay never floods the pool.
        if (state.pending.size() >= 24u || state.failed.count(key) || !state.pending.insert(key).second)
            return;
        generation = state.generation;
    }

    const std::shared_ptr<SummitEffectMeshJob255> job(new(std::nothrow) SummitEffectMeshJob255(key, generation));
    if (!job)
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.pending.erase(key);
        return;
    }
    const bool submitted = summit::core::concurrency::JobSystem::Instance().Submit(
        [job]()
        {
            HoEffectResMesh *mesh = new(std::nothrow) HoEffectResMesh;
            if (mesh)
            {
                // Same source locks and thread-local physique used by model V3.
                smASE_SetPhysique(NULL);
                const bool loaded = job->key.second.empty()
                    ? mesh->LoadingRes(job->key.first, true)
                    : mesh->LoadingRes(job->key.first, job->key.second, true);
                smASE_SetPhysique(NULL);
                // Worker-side destruction can touch texture/device state.
                // Even partial/failed models are reclaimed by the owner.
                job->prepared = mesh;
                (void)loaded;
            }
            SummitEffectMeshState255 &state = SummitEffectMeshState();
            std::lock_guard<std::mutex> lock(state.mutex);
            state.completed.push_back(job);
        }, summit::core::concurrency::JobPriority::Normal);
    if (!submitted)
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.pending.erase(key);
    }
}

smPAT3D *HoEffectResManager::GetReadyResMesh(const std::string &meshName, const std::string &boneName)
{
    if (meshName.empty() || meshName.size() >= 128u || boneName.size() >= 128u)
        return NULL;
    const ResMeshKey key = summit::effects::MakeEffectMeshKey249(meshName, boneName);
    ResMeshMapIT existing = m_ResMeshMap.find(key);
    if (existing != m_ResMeshMap.end())
        return existing->second->GetRes();
    RequestResMesh(meshName, boneName);
    return NULL;
}

void HoEffectResManager::PumpPreparedResMeshes(unsigned int maxCommits)
{
    SummitEffectMeshState255 &state = SummitEffectMeshState();
    if (maxCommits > 4u) maxCommits = 4u;
    for (unsigned int i = 0; i < maxCommits; ++i)
    {
        std::shared_ptr<SummitEffectMeshJob255> job;
        bool current = false;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            if (state.completed.empty()) break;
            job = state.completed.front();
            state.completed.pop_front();
            current = job->generation == state.generation;
            if (current)
            {
                state.pending.erase(job->key);
                if (!job->prepared || !job->prepared->GetRes())
                    state.failed.insert(job->key);
            }
        }
        if (job->prepared)
        {
            if (current && job->prepared->GetRes() &&
                m_ResMeshMap.find(job->key) == m_ResMeshMap.end())
            {
                job->prepared->FinishMaterialReadOnOwnerThread();
                m_ResMeshMap[job->key] = job->prepared;
                job->prepared = NULL;
            }
            // Owner-thread teardown even for obsolete/failed jobs.
            delete job->prepared;
            job->prepared = NULL;
        }
    }
}
#endif

smPAT3D *HoEffectResManager::GetResBone(const std::string &meshName)
{
    // Legacy one-argument interface: an ambiguous mesh cannot identify which
    // of several skeletons the caller intended. Fail closed instead of using
    // an unrelated animation. Current client has no active call sites.
    smPAT3D *selected = NULL;
    for (ResMeshMapIT it = m_ResMeshMap.begin(); it != m_ResMeshMap.end(); ++it)
    {
        if (it->first.first != meshName)
            continue;
        smPAT3D *bone = it->second->GetBoneRes();
        if (!bone)
            continue;
        if (selected && selected != bone)
            return NULL;
        selected = bone;
    }
    return selected;
}

#ifndef _SERVER
void HoEffectResManager::RequestResTexturePrewarm(const std::string &textureName)
{
    // This queue is only accessed from the render/owner thread. It defers
    // material registration instead of doing CPU/GPU work during effect Init.
    if (textureName.empty() || textureName.size() >= 128u ||
        m_ResTextureMap.find(textureName) != m_ResTextureMap.end() ||
        m_PrewarmTextureNames257.size() >= 128u)
        return;
    if (m_PrewarmTextureNames257.insert(textureName).second)
        m_PrewarmTextureQueue257.push_back(textureName);
}

void HoEffectResManager::PumpResTexturePrewarm(unsigned int maxLoads)
{
    // No unbounded global sweep. Material creation stays on the owner thread;
    // AssetLoadV2ReadMaterialGroup enqueues asynchronous GPU preparation.
    if (maxLoads > 4u) maxLoads = 4u;
    for (unsigned int i = 0; i < maxLoads && !m_PrewarmTextureQueue257.empty(); ++i)
    {
        const std::string name = m_PrewarmTextureQueue257.front();
        m_PrewarmTextureQueue257.pop_front();
        m_PrewarmTextureNames257.erase(name);
        if (m_ResTextureMap.find(name) == m_ResTextureMap.end())
            GetResTexture(name);
    }
}
#endif

int HoEffectResManager::GetResTexture(const std::string &fileName)
{
    if(m_ResTextureMap.find(fileName) == m_ResTextureMap.end())
    {
        HoEffectResTexture *resTexture = new HoEffectResTexture;
        if(!resTexture->LoadingRes(fileName) || resTexture->GetRes() < 0)
        {
            delete resTexture;
            return -1;
        }
        m_ResTextureMap[fileName] = resTexture;
        return resTexture->GetRes();
    }
    else
        return m_ResTextureMap[fileName]->GetRes();
    return -1;
}

void HoEffectResManager::Clear()
{
#ifndef _SERVER
    m_PrewarmTextureQueue257.clear();
    m_PrewarmTextureNames257.clear();
#endif
#ifndef _SERVER
    {
        SummitEffectMeshState255 &state = SummitEffectMeshState();
        std::lock_guard<std::mutex> lock(state.mutex);
        ++state.generation;
        state.pending.clear();
        state.failed.clear();
    }
#endif
    ResTextureMapIT M_stIT	= m_ResTextureMap.begin();
	ResTextureMapIT	M_endIT	= m_ResTextureMap.end();

	for(;M_stIT != M_endIT;)
	{
		delete M_stIT->second;
		++M_stIT;
	}

    ResMeshMapIT B_stIT	 = m_ResMeshMap.begin();
	ResMeshMapIT B_endIT = m_ResMeshMap.end();

	for(;B_stIT != B_endIT;)
	{
		delete B_stIT->second;
		++B_stIT;
	}

	m_ResTextureMap.clear();
	m_ResMeshMap.clear();
}





 
