







#ifndef _HO_EFFECT_RES_H_
#define _HO_EFFECT_RES_H_

#include "systems/rendering/effects/legacy_new/core/HoNewEffectHeader.h"
#ifndef _HO_NEW_WIN32_
#include "systems/rendering/legacy_3d/smd3d.h"
#endif

#ifdef  _HO_NEW_WIN32_
#include "systems/rendering/legacy_3d/smd3d.h"
#endif



#include <string>
#include <map>
#include <deque>
#include <set>
#include "rendering/effects/legacy_new/SummitEffectMeshKey249.h"

class HoEffectResTexture
{
private:
    int     m_iMatNum;
public:
    HoEffectResTexture();
    ~HoEffectResTexture();

    bool    LoadingRes(const std::string &fileName);
    int     GetRes();
};


class HoEffectResMesh
{
private:
    smPAT3D *m_Mesh;
    smPAT3D *m_Bone;
public:
    HoEffectResMesh();
    ~HoEffectResMesh();

    bool    LoadingRes(const std::string &meshName, bool deferMaterialRead = false);
    bool    LoadingRes(const std::string &meshName, const std::string &boneName, bool deferMaterialRead = false);
#ifndef _SERVER
    void    FinishMaterialReadOnOwnerThread();
#endif
    smPAT3D *GetRes();
	smPAT3D *GetBoneRes();
};


class HoEffectResManager
{
public:
    HoEffectResManager();
    ~HoEffectResManager();
protected:
    typedef std::map<std::string, HoEffectResTexture *> ResTextureMap;
    typedef std::map<std::string, HoEffectResTexture *>::iterator ResTextureMapIT;

    // A mesh with different bones is a different resource; meshName alone
    // could silently return another skill's skeleton.
    typedef summit::effects::EffectMeshKey249 ResMeshKey;
    typedef std::map<ResMeshKey, HoEffectResMesh *> ResMeshMap;
    typedef ResMeshMap::iterator ResMeshMapIT;

private:
    ResMeshMap          m_ResMeshMap;
    ResTextureMap       m_ResTextureMap;
#ifndef _SERVER
    // Owner-thread-only, bounded texture look-ahead: never enqueue jobs here.
    std::deque<std::string> m_PrewarmTextureQueue257;
    std::set<std::string> m_PrewarmTextureNames257;
#endif

    static HoEffectResManager*              m_Instance;
public:
    static HoEffectResManager*              GetInstance();

    smPAT3D *GetResMesh(const std::string &meshName, const std::string &boneName);
#ifndef _SERVER
    // Nonblocking effect path: never parse an ASE from UpdateView.
    void RequestResMesh(const std::string &meshName, const std::string &boneName);
    smPAT3D *GetReadyResMesh(const std::string &meshName, const std::string &boneName);
    void PumpPreparedResMeshes(unsigned int maxCommits = 2u);
#endif
    smPAT3D *GetResBone(const std::string &meshName);

	int      GetResTexture(const std::string &textureName);
#ifndef _SERVER
    void RequestResTexturePrewarm(const std::string &textureName);
    void PumpResTexturePrewarm(unsigned int maxLoads = 2u);
#endif
	

    void    Clear();
};

#endif




