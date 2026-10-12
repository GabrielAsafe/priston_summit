#!/usr/bin/env python3
"""PATCH 262: native cooker CPU material initialization / clean fallback."""
import pathlib, subprocess, tempfile, unittest, shutil, sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
HEADER = ROOT / 'SrcServer/src/systems/rendering/legacy_3d/NativeModelCook259.h'
sys.path.insert(0, str(ROOT / 'tools'))
from cook_native_models_259 import _failure_excerpt

class OfflineMaterialTests(unittest.TestCase):
    def test_material_initialization_compiled_cpp(self):
        cxx = shutil.which('g++') or shutil.which('clang++')
        if not cxx:
            self.skipTest('C++ compiler unavailable')
        cpp = r"""
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
using DWORD = unsigned long;
const DWORD INVALID_FILE_ATTRIBUTES = (DWORD)-1;
const DWORD FILE_ATTRIBUTE_DIRECTORY = 0x10;
const DWORD FILE_ATTRIBUTE_REPARSE_POINT = 0x400;
const int FALSE = 0;
static std::string seq;
static bool hasOutput = false;
static bool failInit = false;
static bool failParse = false;
static bool materialReady = false;
struct MAT{};
static MAT group;
static MAT* smMaterialGroup = nullptr;
static MAT* smMaterial = nullptr;
DWORD GetFileAttributesA(const char* path) {
    if(std::strstr(path, ".ASE")) return 0;
    return hasOutput ? 0 : INVALID_FILE_ATTRIBUTES;
}
struct smPAT3D { ~smPAT3D(){seq += "D";} };
void initSinCos(){ seq += "T"; }
int g_IsReadTextures = 1;
int InitMaterial(){seq += "I"; if(failInit) return FALSE; materialReady=true; smMaterialGroup=&group;smMaterial=&group;return 1;}
void CloseMaterial(){seq += "C";smMaterialGroup=nullptr;materialReady=false;}
void smASE_SetPhysique(smPAT3D*) {seq += "P";}
void smASE_EnableOfflineBoneHierarchy273(bool) {}
void smASE_EnableOfflineEmptyMaterials274(bool) {}
smPAT3D* smASE_Read(char*){
    seq += "M";
    if(!materialReady || g_IsReadTextures || failParse) return nullptr;
    hasOutput = true;
    return new smPAT3D;
}
smPAT3D* smASE_ReadBone(char*){
    seq += "B";
    if(!materialReady || !smMaterial || g_IsReadTextures || failParse) return nullptr;
    hasOutput = true;
    return new smPAT3D;
}
smPAT3D* smReadModel(char*, char*){seq += "R";return materialReady && hasOutput ? new smPAT3D : nullptr;}
smPAT3D* smReadModel_Bip(char*){seq += "S";return materialReady && hasOutput ? new smPAT3D : nullptr;}
#include "NativeModelCook259.h"
int main(){
    using summit_model_cook_259::RunOne;
    char a0[]="Server.exe", a1[]="--summit-cook-model-one", a2[]="mesh", a3[]="Effect/Test.ASE";
    char* argv[]={a0,a1,a2,a3};
    assert(RunOne(4,argv)==0);
    assert(seq.find("I")<seq.find("M"));
    assert(seq.find("R")<seq.rfind("C"));
    assert(smMaterialGroup==nullptr && smMaterial==nullptr);
    seq.clear();hasOutput=false;g_IsReadTextures=1;
    std::strcpy(a2,"bone");
    assert(RunOne(4,argv)==0);
    assert(seq.find("I")<seq.find("B"));
    assert(seq.find("S")<seq.rfind("C"));
    seq.clear();hasOutput=false;failParse=true;
    assert(RunOne(4,argv)==3);
    assert(seq.find("C")!=std::string::npos);
    assert(smMaterialGroup==nullptr);
    seq.clear();failParse=false;failInit=true;
    assert(RunOne(4,argv)==3);
    assert(seq.find("B")==std::string::npos);
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            src=pathlib.Path(tmp)/'test.cpp';exe=pathlib.Path(tmp)/'test'
            src.write_text(cpp)
            result=subprocess.run([cxx,'-std=c++17','-Wall','-Wextra','-Werror','-I',str(HEADER.parent),'-I',str(ROOT/'SrcServer/src'),str(src),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            run=subprocess.run([str(exe)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stderr)
    def test_real_source_initialization_before_parser(self):
        s=HEADER.read_text()
        self.assertLess(s.index('InitMaterial() != FALSE'),s.index('smASE_Read(args[3])'))
        self.assertIn('if (smMaterialGroup) CloseMaterial()',s)
        self.assertIn('g_IsReadTextures = 0;',s)
        self.assertIn('MODEL_COOK_259=FAIL material_init',s)
    def test_error_excerpt_retains_crash_head_and_tail(self):
        msg='Unhandled page fault'+'x'*2500+'Wine build 11.0'
        excerpt=_failure_excerpt(msg,'')
        self.assertIn('Unhandled page fault',excerpt)
        self.assertIn('Wine build 11.0',excerpt)
        self.assertLessEqual(len(excerpt),2400)
    def test_existing_orchestrator_remains_fail_closed(self):
        s=(ROOT/'tools/cook_native_models_259.py').read_text()
        self.assertIn('out.exists()',s)
        self.assertIn('client_result.returncode != 42',s)
        self.assertIn('native conversion failed',s)
        self.assertNotIn('open(\"LOG',s)

if __name__ == '__main__':
    unittest.main(verbosity=2)
