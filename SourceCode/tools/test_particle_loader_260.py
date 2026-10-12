#!/usr/bin/env python3
"""Compile and execute the actual legacy particle resource-loader functions."""
from pathlib import Path
import subprocess
import tempfile
import unittest

BASE = Path(__file__).resolve().parents[1]
SOURCE = BASE/'Shared/rendering/effects/particles/HoNewParticleMgrLegacyCommon.inl'

class Particle260(unittest.TestCase):
    def test_real_legacy_particle_loader_cpp(self):
        source = SOURCE.read_text(encoding='latin-1')
        relevant = source[source.index('void HoNewParticleResMgr::init()'):source.index('void HoNewParticleMgr::Main(')]
        with tempfile.TemporaryDirectory() as tmp:
            tmp=Path(tmp)
            (tmp/'active.inl').write_text(relevant)
            (tmp/'test.cpp').write_text(r'''#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>
#include <strings.h>
#include <cstdlib>
#include <sys/stat.h>
using namespace std;
#define FALSE 0
#define TRUE 1
#define MAX_MATERIAL 300
#define MAX_TOKEN_VECTORS 500
#define SMMAT_BLEND_LAMP 1
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define INVALID_HANDLE_VALUE (-1)
#define stricmp strcasecmp
using HANDLE=int;
struct WIN32_FIND_DATA { int dwFileAttributes; char cFileName[260]; };
int findCalls=0, closeCalls=0, created=0, readTextures=0, failCreate=0;
HANDLE FindFirstFile(const char* path, WIN32_FIND_DATA* data) {
    ++findCalls;
    if(strcmp(path,"*.tga")) return INVALID_HANDLE_VALUE;
    data->dwFileAttributes=0;
    strcpy(data->cFileName,"ASH.tga");
    return 1;
}
int FindNextFile(HANDLE, WIN32_FIND_DATA*) {return 0;}
int FindClose(HANDLE) {++closeCalls;return 1;}
int CreateTextureMaterial(const char*,int,int,int,int,int) {
    ++created;
    return failCreate ? -1 : 100+created;
}
struct MaterialGroup {void ReadTextures() {++readTextures;}} material;
MaterialGroup* smMaterialGroup=&material;
void MakeUpperCase(string& s) {for(char& c:s)c=(char)toupper((unsigned char)c);}
namespace summit { namespace diagnostics {
void RuntimeAssetProbeAttempt(const char*,const char*,const char*){}
void RuntimeAssetProbeResult(const char*,const char*,const char*,bool){}
}}
struct Tokenizer {string value; void Tokenize(char* s) {value=s;}};
struct HoNewParticleScriptRes {string name;Tokenizer emitterTokenizer;};
struct HoNewParticleMatRes {int matNum;string matName;};
class HoNewParticleResMgr {
 int NumScriptRes=0;vector<HoNewParticleScriptRes> ScriptRes;
 int NumMaterial=0;vector<HoNewParticleMatRes> MatRes;
public:
 void init(); HoNewParticleResMgr(){init();}
 HoNewParticleScriptRes* FindScript(char*);
 bool LoadScript(char*);
 HoNewParticleScriptRes* AddScript(char*,char*);
 int SearchingAddTexture(const char*);
 int AddTexture(const char*);
};
#include "active.inl"
int main(int argc, char** argv) {
 if(argc!=2)return 1;
 HoNewParticleResMgr mgr;
 string path=string(argv[1])+"/Spark.part";
 FILE* fp=fopen(path.c_str(),"wb");if(!fp)return 2;
 fputs("test-particle-content",fp);fclose(fp);
 if(!mgr.LoadScript(path.data())) return 3;
 char key[]="SPARK";
 if(!mgr.FindScript(key))return 4;
 if(remove(path.c_str())!=0)return 5;
 if(!mgr.LoadScript(path.data()))return 6; // duplicate never reopens
 char missing[]="missing.part";
 if(mgr.LoadScript(missing))return 7;
 if(mgr.SearchingAddTexture("missing-dir"))return 8;
 if(findCalls!=1 || closeCalls!=0)return 9;
 if(!mgr.SearchingAddTexture("*.tga"))return 10;
 if(findCalls!=2 || closeCalls!=1 || created!=1 || readTextures!=1)return 11;
 const int mat=mgr.AddTexture("ash.tga");
 if(mat!=101 || created!=1)return 12; // case insensitive cache
 string longName(300,'X');
 if(mgr.AddTexture(longName.c_str())!=-1 || created!=1)return 13;
 failCreate=1;
 if(mgr.AddTexture("broken.tga")!=-1)return 14;
 failCreate=0;
 if(mgr.AddTexture("broken.tga")<0)return 15; // failed entries not cached
 if(created!=3)return 16;
 return 0;
}
''')
            binary=tmp/'test'
            run=subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',str(tmp/'test.cpp'),'-o',str(binary)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stderr)
            res=subprocess.run([str(binary),str(tmp)],capture_output=True,text=True)
            self.assertEqual(res.returncode,0,res.stderr+res.stdout)

    def test_no_permanent_missing_script_log(self):
        source=SOURCE.read_text(encoding='latin-1')
        self.assertNotIn('mingw_missing_particle_scripts.txt',source)
        self.assertEqual(source.count('FindFirstFile(directory, &wfd)'),1)
        self.assertIn('if (hSrch == INVALID_HANDLE_VALUE) return FALSE;',source)

if __name__=='__main__':unittest.main(verbosity=2)
