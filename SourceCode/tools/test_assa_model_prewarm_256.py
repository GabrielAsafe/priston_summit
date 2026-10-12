#!/usr/bin/env python3
"""PATCH256: compile/run actual async C++ fragment against real JobSystem and SourceAssetLock."""
from pathlib import Path
import shutil,subprocess,tempfile,unittest
R=Path(__file__).resolve().parents[1]
F=R/'Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl'
HEAD=r'''
#include <atomic>
#include <cassert>
#include <cstring>
#include <thread>
#include <iostream>
#include <string>
#include <memory>
#include <deque>
#include <set>
#include <mutex>
#include <new>
#include "core/concurrency/JobSystem.h"
#include "core/concurrency/SourceAssetLock.h"
#define TYPE_ASE 200
#define TYPE_BONE 300
#define TYPE_BONE_ANI 400
#define TYPE_MAT 100
#define SMASSET_OWNER_EFFECT 7
#define MAX_RES_FULL_NAME 512
#define TRUE 1
#define FALSE 0
struct smMATERIAL_GROUP {};
static std::thread::id owner=std::this_thread::get_id();
static std::atomic<int> meshReads{0},boneReads{0},published{0},destroyed{0};
struct smPAT3D {
 smMATERIAL_GROUP *smMaterialGroup;
 smPAT3D():smMaterialGroup(new smMATERIAL_GROUP) {}
 ~smPAT3D(){assert(std::this_thread::get_id()==owner);++destroyed;delete smMaterialGroup;}
 void ZeroNormals() {}
};
static thread_local smPAT3D *physique=nullptr;
void smASE_SetPhysique(smPAT3D *p){physique=p;}
smPAT3D *smASE_Read(char *path){++meshReads;return !strcmp(path,"missing")?nullptr:new smPAT3D;}
smPAT3D *smASE_ReadBone(char *path){++boneReads;return !strcmp(path,"missing")?nullptr:new smPAT3D;}
void AssetLoadV2ReadMaterialGroup(smMATERIAL_GROUP*,int){assert(std::this_thread::get_id()==owner);++published;}
struct cASSARES{void *res=nullptr; char resName[64]={};char resFullName[512]={};int type=0;int LoadFlag=0;};
static cASSARES AssaRes[1000];
static int AssaResCount=0;
'''
MAIN=r'''
int main(){
 AssaResCount=4;
 AssaRes[0].type=TYPE_ASE;strcpy(AssaRes[0].resFullName,"solo");
 AssaRes[1].type=TYPE_BONE_ANI;strcpy(AssaRes[1].resFullName,"pairbone");
 AssaRes[2].type=TYPE_BONE;strcpy(AssaRes[2].resFullName,"pairmesh");
 AssaRes[3].type=TYPE_ASE;strcpy(AssaRes[3].resFullName,"missing");
 auto &pool=summit::core::concurrency::JobSystem::Instance();
 SummitPrewarmAssaEffectModelsStep256(2u);
 SummitPrewarmAssaEffectModelsStep256(2u);
 pool.WaitIdle();SummitPumpAssaEffectModels256(2u);SummitPumpAssaEffectModels256(2u);
 assert(AssaRes[0].res&&AssaRes[0].LoadFlag);
 assert(AssaRes[1].res&&AssaRes[1].LoadFlag);
 assert(AssaRes[2].res&&AssaRes[2].LoadFlag);
 assert(meshReads==3&&boneReads==1&&published==2);
 for(int i=0;i<8;++i)SummitPrewarmAssaEffectModelsStep256(2u);
 pool.WaitIdle();SummitPumpAssaEffectModels256(2u);SummitPumpAssaEffectModels256(2u);
 assert(!AssaRes[3].res&&meshReads==3);
 for(int i=0;i<8;++i)SummitPrewarmAssaEffectModelsStep256(2u);
 pool.WaitIdle();SummitPumpAssaEffectModels256(2u);SummitPumpAssaEffectModels256(2u);
 assert(meshReads==3);
 SummitResetAssaEffectModels256();
 AssaRes[0].res=nullptr;AssaRes[0].LoadFlag=FALSE;
 SummitPrewarmAssaEffectModelsStep256(1u);
 SummitResetAssaEffectModels256();
 pool.WaitIdle();SummitPumpAssaEffectModels256(2u);SummitPumpAssaEffectModels256(2u);
 assert(!AssaRes[0].res&&published==2);
 delete static_cast<smPAT3D*>(AssaRes[2].res);
 delete static_cast<smPAT3D*>(AssaRes[1].res);
 std::cout<<"SUMMIT_ASSA_MODEL_PREWARM_256_CPP=PASS\n";
}
'''
class Check(unittest.TestCase):
 def test_cpp_actual_fragment(self):
  if not shutil.which('g++'):self.skipTest('g++ absent')
  inl=F.read_text(errors='replace');fragment=inl.split('// SUMMIT_ASSA_ASYNC_256_BEGIN',1)[1].split('// SUMMIT_ASSA_ASYNC_256_END',1)[0]
  with tempfile.TemporaryDirectory() as d:
   t=Path(d);(t/'fragment.h').write_text(fragment);(t/'test.cpp').write_text(HEAD+'\n#include "fragment.h"\n'+MAIN)
   cmd=['g++','-std=c++17','-pthread','-O1','-Wall','-Wextra','-Werror','-I',str(t),'-I',str(R/'SrcGame/src'),str(t/'test.cpp'),str(R/'SrcGame/src/core/concurrency/JobSystem.cpp'),str(R/'SrcGame/src/core/concurrency/SourceAssetLock.cpp'),'-o',str(t/'test')]
   r=subprocess.run(cmd,capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stderr)
   r=subprocess.run([str(t/'test')],capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stdout+'\n'+r.stderr)
   self.assertIn('PASS',r.stdout)
 def test_default_frame_paths(self):
  p=(R/'SrcGame/src/platform/windows/application/Winmain.cpp').read_text(errors='replace')
  self.assertEqual(p.count('SummitPumpAssaEffectModels256(1u);'),2)
  self.assertEqual(p.count('SummitPrewarmAssaEffectModelsStep256(1u);'),2)
  a=F.read_text(errors='replace')
  self.assertGreaterEqual(a.count('SummitResetAssaEffectModels256();'),2)
  self.assertGreaterEqual(a.count('SourceAssetLock lock('),4)
if __name__=='__main__':unittest.main(verbosity=2)
