#!/usr/bin/env python3
"""PATCH 270: test real *MESH_FACE reader against missing per-face MTLIDs."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'

class FaceSlots270(unittest.TestCase):
    def test_actual_reader_cpp(self):
        text=SOURCE.read_text(encoding='latin1')
        start=text.index('\t\tif(lstrcmp(decode, "*MESH_FACE") == 0)', text.index('FILE *ReadASE_GEOMOBJECT'))
        end=text.index('\n\n\n\t\tif(lstrcmp(decode, "*MESH_TVERT") == 0)', start)
        block=text[start:end]
        self.assertIn('FaceMatrial[FaceCnt - 1] = atoi(strBuff)',block)
        compiler=shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        mock=r'''
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
struct Object { int faces=0; void AddFace(int,int,int) { faces++; } };
int lstrcmp(const char *a, const char *b) { return std::strcmp(a,b); }
char *GetWord(char *q, char *p) {
  while(*p==' ' || *p=='\t' || *p==':') ++p;
  int n=0;
  while(*p && *p!=' ' && *p!='\t' && *p!='\r' && *p!='\n' && *p!=':') {
    if(n < 255) q[n++]=*p;
    ++p;
  }
  q[n]=0;
  return p;
}
char decode[]="*MESH_FACE";
int nFace=3, nVertex=8, FaceCnt=0, cnt=0, a=0,b=0,c=0;
int FaceMatrialBuf[3]={0,0,0};
int *FaceMatrial=FaceMatrialBuf;
char strBuff[256];
Object object; Object* obj=&object;
const char* failure=nullptr;
int readFace(char *line) {
    char *p=line;
    auto failImportWith=[](const char *s) { failure=s; return -1; };
''' + block + r'''
    return 0;
}
int main(){
    char a[]="0: A: 0 B: 1 C: 2 AB: 1 BC: 1 CA: 1 *MESH_SMOOTHING 1 *MESH_MTLID 4";
    char b[]="1: A: 1 B: 2 C: 3 AB: 1 BC: 1 CA: 1 *MESH_SMOOTHING 1";
    char c[]="2: A: 2 B: 3 C: 4 AB: 1 BC: 1 CA: 1 *MESH_SMOOTHING 1 *MESH_MTLID 6";
    assert(readFace(a)==0 && readFace(b)==0 && readFace(c)==0);
    assert(FaceCnt==3 && object.faces==3);
    assert(FaceMatrial[0]==4 && FaceMatrial[1]==0 && FaceMatrial[2]==6);
    char d[]="3: A: 0 B: 1 C: 2 *MESH_MTLID 10";
    assert(readFace(d)==-1 && strcmp(failure,"geometry_face_count")==0);
}
'''
        with tempfile.TemporaryDirectory() as td:
            cpp=Path(td)/'real.cpp';exe=Path(td)/'real';cpp.write_text(mock)
            result=subprocess.run([compiler,'-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            run=subprocess.run([str(exe)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)

    def test_only_mesh_reader_changes(self):
        text=SOURCE.read_text(encoding='latin1')
        mesh=text.split('FILE *ReadASE_GEOMOBJECT',1)[1].split('FILE *smSTAGE3D_ReadASE_GEOMOBJECT',1)[0]
        self.assertNotIn('FaceMatrial[mCount++]',mesh)
        self.assertIn('FaceMatrial[FaceCnt - 1]',mesh)
        self.assertIn('geometry_face_material',mesh)
        self.assertIn('summit_ase_face269::Resolve(',mesh)
        # Legacy map/stage import is not silently changed by a model-only patch.
        self.assertIn('FaceMatrial[mCount++]',text)

if __name__=='__main__':unittest.main(verbosity=2)
