#!/usr/bin/env python3
"""Compile the actual PATCH 255 legacy .inl against minimal portable engine stubs."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
STUB = r'''
#pragma once
#include <string>
#include <map>
#include <deque>
#include <set>
#include <cstring>
#include <utility>
#define SMMAT_BLEND_LAMP 3
#define SMASSET_OWNER_EFFECT 2
namespace summit { namespace effects {
typedef std::pair<std::string,std::string> EffectMeshKey249;
inline EffectMeshKey249 MakeEffectMeshKey249(const std::string &a,const std::string &b) { return {a,b}; }
}}
struct smMATERIAL { int BlendType=0; };
struct smMATERIAL_GROUP { int MaterialCount=1; smMATERIAL smMaterial[1]; void ReadTextures() {} };
struct smPAT3D { smMATERIAL_GROUP* smMaterialGroup; smPAT3D(); ~smPAT3D(); void ZeroNormals() {} };
extern smMATERIAL_GROUP *smMaterialGroup;
int CreateTextureMaterial(char *, int,int,int,int,int);
void AssetLoadV2ReadMaterialGroup(smMATERIAL_GROUP*,int);
void AssetLoadV2ReleaseMaterial(smMATERIAL_GROUP*,int);
smPAT3D* smASE_Read(char*); smPAT3D* smASE_ReadBone(char*); void smASE_SetPhysique(smPAT3D*);
class HoEffectResTexture { int m_iMatNum; public: HoEffectResTexture(); ~HoEffectResTexture(); bool LoadingRes(const std::string&); int GetRes(); };
class HoEffectResMesh {
    smPAT3D *m_Mesh, *m_Bone;
public:
    HoEffectResMesh(); ~HoEffectResMesh();
    bool LoadingRes(const std::string&, bool deferMaterialRead=false);
    bool LoadingRes(const std::string&,const std::string&, bool deferMaterialRead=false);
    void FinishMaterialReadOnOwnerThread(); smPAT3D* GetRes(); smPAT3D* GetBoneRes();
};
class HoEffectResManager {
public: HoEffectResManager(); ~HoEffectResManager();
protected:
    typedef std::map<std::string,HoEffectResTexture*> ResTextureMap;
    typedef ResTextureMap::iterator ResTextureMapIT;
    typedef summit::effects::EffectMeshKey249 ResMeshKey;
    typedef std::map<ResMeshKey,HoEffectResMesh*> ResMeshMap;
    typedef ResMeshMap::iterator ResMeshMapIT;
private:
    ResMeshMap m_ResMeshMap; ResTextureMap m_ResTextureMap;
    std::deque<std::string> m_PrewarmTextureQueue257;
    std::set<std::string> m_PrewarmTextureNames257;
    static HoEffectResManager* m_Instance;
public:
    static HoEffectResManager* GetInstance();
    smPAT3D* GetResMesh(const std::string&,const std::string&);
    void RequestResMesh(const std::string&,const std::string&);
    smPAT3D* GetReadyResMesh(const std::string&,const std::string&);
    void PumpPreparedResMeshes(unsigned int maxCommits=2u);
    smPAT3D* GetResBone(const std::string&);
    int GetResTexture(const std::string&);
    void RequestResTexturePrewarm(const std::string&);
    void PumpResTexturePrewarm(unsigned int maxLoads=2u);
    void Clear();
};
'''
CPP = r'''
#include <atomic>
#include <cassert>
#include <thread>
#include <cstring>
#include <iostream>
#include "systems/rendering/effects/legacy_new/core/HoEffectResMgr.h"
#include "core/concurrency/JobSystem.h"
static std::thread::id owner = std::this_thread::get_id();
static std::atomic<int> reads{0}, bones{0}, published{0}, destroyed{0};
static thread_local smPAT3D* physique = nullptr;
static smMATERIAL_GROUP globalGroup;
smMATERIAL_GROUP* smMaterialGroup = &globalGroup;
smPAT3D::smPAT3D():smMaterialGroup(new smMATERIAL_GROUP){}
smPAT3D::~smPAT3D() { assert(std::this_thread::get_id()==owner); delete smMaterialGroup; ++destroyed; }
void smASE_SetPhysique(smPAT3D* b) { physique=b; }
smPAT3D* smASE_Read(char* s) { ++reads; if (!std::strcmp(s,"missing")) return nullptr; return new smPAT3D; }
smPAT3D* smASE_ReadBone(char*) { ++bones; return new smPAT3D; }
int CreateTextureMaterial(char*,int,int,int,int,int) { return 1; }
void AssetLoadV2ReadMaterialGroup(smMATERIAL_GROUP*,int) { assert(std::this_thread::get_id()==owner); ++published; }
void AssetLoadV2ReleaseMaterial(smMATERIAL_GROUP*,int) { assert(std::this_thread::get_id()==owner); }
#include "rendering/effects/legacy_new/HoEffectResMgrLegacyCommon.inl"
int main() {
    auto& pool=summit::core::concurrency::JobSystem::Instance();
    auto* mgr=HoEffectResManager::GetInstance();
    mgr->RequestResMesh("effectA","boneA");
    mgr->RequestResMesh("effectA","boneA");
    assert(mgr->GetReadyResMesh("effectA","boneA")==nullptr);
    pool.WaitIdle(); mgr->PumpPreparedResMeshes(2);
    auto* first=mgr->GetReadyResMesh("effectA","boneA");
    assert(first && reads==1 && bones==1 && published==1);
    mgr->RequestResMesh("effectA","boneB");
    pool.WaitIdle(); mgr->PumpPreparedResMeshes(2);
    auto* second=mgr->GetReadyResMesh("effectA","boneB");
    assert(second && first!=second && reads==2 && bones==2 && published==2);
    mgr->RequestResMesh("missing", "");
    pool.WaitIdle(); mgr->PumpPreparedResMeshes(2);
    assert(!mgr->GetReadyResMesh("missing", ""));
    assert(reads==3);
    mgr->RequestResMesh("late", ""); mgr->Clear();
    pool.WaitIdle(); mgr->PumpPreparedResMeshes(4);
    assert(published==2);
    mgr->RequestResMesh("next", ""); pool.WaitIdle(); mgr->PumpPreparedResMeshes(2);
    assert(mgr->GetReadyResMesh("next", "")!=nullptr);
    mgr->Clear(); pool.WaitIdle(); mgr->PumpPreparedResMeshes(4);
    assert(destroyed>0);
    std::cout<<"SUMMIT_EFFECT_MESH_STREAM_255_CPP=PASS\n";
}
'''
class TestEffectMesh255(unittest.TestCase):
    def test_real_legacy_inl_cpp(self):
        if not shutil.which('g++'):
            self.skipTest('g++ not found')
        with tempfile.TemporaryDirectory() as tmp:
            temp=Path(tmp)
            header=temp/'systems/rendering/effects/legacy_new/core/HoEffectResMgr.h'
            header.parent.mkdir(parents=True)
            header.write_text(STUB)
            source=temp/'test.cpp'; source.write_text(CPP)
            cmd=['g++','-std=c++17','-pthread','-Wall','-Wextra','-Werror','-O1',
                 '-I',str(temp),'-I',str(ROOT/'SrcGame/src'),'-I',str(ROOT/'Shared'),
                 str(source),str(ROOT/'SrcGame/src/core/concurrency/JobSystem.cpp'),
                 str(ROOT/'SrcGame/src/core/concurrency/SourceAssetLock.cpp'),'-o',str(temp/'test')]
            build=subprocess.run(cmd,capture_output=True,text=True)
            self.assertEqual(build.returncode,0,'g++:\n'+build.stderr)
            executed=subprocess.run([str(temp/'test')],capture_output=True,text=True)
            self.assertEqual(executed.returncode,0,'test:\n'+executed.stdout+'\n'+executed.stderr)
            self.assertIn('SUMMIT_EFFECT_MESH_STREAM_255_CPP=PASS',executed.stdout)
    def test_render_loop_nonblocking(self):
        model=(ROOT/'Shared/rendering/effects/legacy_new/HoEffectModel_legacy_client.inl').read_text(errors='replace')
        body=model[model.index('bool HoEffectMeshModel::UpdateView'):]
        self.assertIn('GetReadyResMesh(',body[:500])
        self.assertNotIn('GetResMesh(',body[:500])
    def test_pumps_and_generation(self):
        win=(ROOT/'SrcGame/src/platform/windows/application/Winmain.cpp').read_text(errors='replace')
        self.assertGreaterEqual(win.count('PumpPreparedResMeshes(2u)'),2)
        inl=(ROOT/'Shared/rendering/effects/legacy_new/HoEffectResMgrLegacyCommon.inl').read_text(errors='replace')
        self.assertIn('if (!deferMaterialRead)',inl)
        self.assertIn('++state.generation;',inl)
        self.assertIn('state.failed.insert(job->key)',inl)
if __name__=='__main__': unittest.main(verbosity=2)
