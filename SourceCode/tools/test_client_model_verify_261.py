#!/usr/bin/env python3
"""Compiles actual Game.exe native verifier header with C++ runtime stubs."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
CLIENT=ROOT/'SrcGame/src/platform/windows/application/Winmain.cpp'

class ClientNativeVerify261(unittest.TestCase):
    def test_real_client_verifier_cpp(self):
        source=ROOT/'SrcGame/src/systems/rendering/legacy_3d/NativeModelVerify261.h'
        with tempfile.TemporaryDirectory() as directory:
            temp=Path(directory)
            p=temp/'main.cpp'
            p.write_text(r'''#include <cstdio>
#include <cstring>
#include <string>
using DWORD=unsigned long;
#define INVALID_FILE_ATTRIBUTES (~DWORD(0))
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_REPARSE_POINT 0x400
DWORD GetFileAttributesA(const char* path) {
    FILE* f=fopen(path,"rb");
    if(!f)return INVALID_FILE_ATTRIBUTES;
    fclose(f);return 0;
}
struct smPAT3D {};
int g_IsReadTextures=1, meshCalls=0,boneCalls=0;
void initSinCos(){}
void smASE_SetPhysique(smPAT3D*){}
smPAT3D* smReadModel(char*,char*) {++meshCalls;return new smPAT3D;}
smPAT3D* smReadModel_Bip(char*) {++boneCalls;return new smPAT3D;}
#include "systems/rendering/legacy_3d/NativeModelVerify261.h"
int main(int argc, char** argv) {
 if(argc!=2)return 1;
 char name[256];snprintf(name,sizeof(name),"%s/cache.smd",argv[1]);
 FILE* f=fopen(name,"wb");if(!f)return 2;
 fputs("native",f);fclose(f);
 std::string rel="--summit-verify-model mesh ";rel+=name;
 // Absolute paths are not allowed even if the file exists.
 if(summit_offline_model_261::RunFromCommandLine(rel.c_str())==0)return 3;
 if(summit_offline_model_261::RunFromCommandLine("--summit-verify-model mesh ../bad.smd")==0)return 4;
 if(summit_offline_model_261::RunFromCommandLine("--summit-verify-model bone x.smd")==0)return 5;
 if(summit_offline_model_261::RunFromCommandLine("--summit-verify-model mesh notfound.smd")==0)return 6;
 if(summit_offline_model_261::RunFromCommandLine("--summit-verify-model mesh cache.smd extra")==0)return 7;
 char* cwd=nullptr;(void)cwd;
 return 0;
}
''')
            exe=temp/'test'
            build=subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',
                '-I',str(ROOT/'SrcGame/src'),'-I',str(ROOT/'Shared'),str(p),'-o',str(exe)],
                capture_output=True,text=True)
            self.assertEqual(build.returncode,0,build.stderr)
            run=subprocess.run([str(exe),str(temp)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stderr+run.stdout)
            # A successful *relative* client native reload, without real Win32 dependencies.
            (temp/'cache.smd').write_bytes(b'native')
            success=(temp/'success.cpp')
            success.write_text(r'''#include <cstdio>
#include <cstring>
using DWORD=unsigned long;
#define INVALID_FILE_ATTRIBUTES (~DWORD(0))
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_REPARSE_POINT 0x400
DWORD GetFileAttributesA(const char* path) {FILE* f=fopen(path,"rb");if(!f)return INVALID_FILE_ATTRIBUTES;fclose(f);return 0;}
struct smPAT3D {};
int g_IsReadTextures=1, meshCalls=0, boneCalls=0;
void initSinCos(){}
void smASE_SetPhysique(smPAT3D*){}
smPAT3D* smReadModel(char*,char*) {++meshCalls;return new smPAT3D;}
smPAT3D* smReadModel_Bip(char*) {++boneCalls;return new smPAT3D;}
#include "systems/rendering/legacy_3d/NativeModelVerify261.h"
int main(){
 if(summit_offline_model_261::RunFromCommandLine("--summit-verify-model mesh cache.smd")!=42) return 1;
 FILE* f=fopen("cache.smb","wb");if(!f)return 2;fputs("native",f);fclose(f);
 if(summit_offline_model_261::RunFromCommandLine("--summit-verify-model bone cache.smb")!=42) return 3;
 if(meshCalls!=1||boneCalls!=1||g_IsReadTextures!=0)return 4;
 return 0;
}
''')
            exe2=temp/'success'
            build2=subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',
                '-I',str(ROOT/'SrcGame/src'),'-I',str(ROOT/'Shared'),str(success),'-o',str(exe2)],
                capture_output=True,text=True)
            self.assertEqual(build2.returncode,0,build2.stderr)
            run2=subprocess.run([str(exe2)],cwd=temp,capture_output=True,text=True)
            self.assertEqual(run2.returncode,0,run2.stderr+run2.stdout)
            self.assertEqual(run2.stdout.count('MODEL_VERIFY_261=PASS'),2)

    def test_before_game_initialization_no_trace(self):
        body=CLIENT.read_text(errors='replace')
        self.assertIn('summit_offline_model_261::RunFromCommandLine(lpCmdLine)',body)
        self.assertLess(body.index('summit_offline_model_261::RunFromCommandLine(lpCmdLine)'),body.index('pcAntiCheat = new AntiCheat'))
        self.assertNotIn('CreateFileA("LOGS\\\\mingw_winmain_reached.txt"',body)
        orchestrator=(ROOT/'tools/cook_native_models_259.py').read_text()
        self.assertIn('client_result.returncode != 42',orchestrator)
        self.assertIn("'client_readback': True",orchestrator)

if __name__ == '__main__':unittest.main(verbosity=2)
