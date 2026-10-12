#!/usr/bin/env python3
"""Actual PATCH 269 C++ material resolver + real caller branch, read-only inspector."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / 'Shared/rendering/legacy_3d/ase/FaceMaterialPolicy269.h'
SOURCE = ROOT / 'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'


class FaceMaterials269(unittest.TestCase):
    def build_and_run(self, code):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler, 'native C++ compiler required')
        with tempfile.TemporaryDirectory() as td:
            src = Path(td) / 'test.cpp'
            exe = Path(td) / 'test'
            src.write_text(code)
            cc = subprocess.run([compiler, '-std=c++17', '-O1', '-Wall', '-Wextra',
                                 '-Werror', '-I', str(HEADER.parent.parent),
                                 str(src), '-o', str(exe)], text=True, capture_output=True)
            self.assertEqual(cc.returncode, 0, cc.stderr)
            run = subprocess.run([str(exe)], text=True, capture_output=True)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)

    def test_native_resolver_compiled(self):
        self.build_and_run(r'''
#include <cassert>
#include "ase/FaceMaterialPolicy269.h"
int main() {
  int n=-77;
  using summit_ase_face269::Resolve;
  assert(Resolve(1,3,2,0,5,&n) && n==3);
  assert(Resolve(1,3,2,1,5,&n) && n==4);
  assert(Resolve(1,3,2,6,5,&n) && n==3);
  assert(Resolve(1,3,2,7,5,&n) && n==4);
  assert(Resolve(0,0,0,65535,5,&n) && n==0);
  assert(!Resolve(1,3,2,-1,5,&n));
  assert(!Resolve(1,4,2,3,5,&n));
  assert(!Resolve(1,3,0,3,5,&n));
  assert(!Resolve(1,3,-2,3,5,&n));
  assert(!Resolve(5,0,0,0,5,&n));
  assert(!Resolve(1,3,2,1,5,nullptr));
  assert(!Resolve(1,0,1,2,5,&n));
}
''')

    def test_real_caller_branch_compiled(self):
        source = SOURCE.read_text(encoding='latin1')
        start = source.index('int subCount = 0;', source.index('// PATCH 269:'))
        stop = source.index('\n\n\t\tmNum = ', start)
        snippet = source[start:stop]
        self.assertIn('s_LightMapRefBuff[MatrialRef].MaxSubMtl', snippet)
        self.assertIn('summit_ase_face269::Resolve(', snippet)
        self.build_and_run(r'''
#include <cassert>
#include <cstring>
#include "ase/FaceMaterialPolicy269.h"
struct Material { int SubPoint; } aseMaterial[8] = {};
struct Light { int MaxSubMtl; bool bLightMap; } s_LightMapRefBuff[4] = {};
int MatrialRef=0, aseMaterialCnt=0, cnt=0, FaceMatrial[2]={}, s_LightMapRefMax=0, fmat=-1;
const char* failure = nullptr;
int check() {
    auto failImportWith = [](const char* msg) { failure = msg; return -1; };
''' + snippet + r'''
    return fmat;
}
int main() {
    MatrialRef=1; aseMaterialCnt=5; s_LightMapRefMax=2;
    aseMaterial[1].SubPoint=3; s_LightMapRefBuff[1].MaxSubMtl=2;
    FaceMatrial[0]=6;
    assert(check()==3 && !failure);
    FaceMatrial[0]=7;
    assert(check()==4 && !failure);
    FaceMatrial[0]=-1;
    assert(check()==-1 && strcmp(failure,"geometry_face_material")==0);
    FaceMatrial[0]=6; failure=nullptr;
    s_LightMapRefBuff[1].MaxSubMtl=3;
    assert(check()==-1 && failure);
    s_LightMapRefBuff[1].MaxSubMtl=2;
    s_LightMapRefBuff[1].bLightMap=true;
    failure=nullptr;
    assert(check()==3); // lightmap count describes copied colour slots
    s_LightMapRefBuff[1].bLightMap=false;
    aseMaterial[1].SubPoint=0; failure=nullptr;
    assert(check()==1 && !failure); // ordinary material
}
''')

    def test_read_only_inspector(self):
        from inspect_ase_face_material_269 import inspect_material_faces
        sample = b'''*MATERIAL_LIST {\n*MATERIAL_COUNT 1\n*MATERIAL 0 {\n*MATERIAL_NAME "Multi"\n*NUMSUBMTLS 2\n*SUBMATERIAL 0 {\n*MATERIAL_NAME "M0"\n}\n*SUBMATERIAL 1 {\n*MATERIAL_NAME "M1"\n}\n}\n}\n*GEOMOBJECT {\n*NODE_NAME "dk"\n*MESH {\n*MESH_NUMFACES 2\n*MESH_FACE 0: A: 0 B: 1 C: 2 *MESH_MTLID 3\n*MESH_FACE 1: A: 0 B: 2 C: 3 *MESH_MTLID 4\n}\n*MATERIAL_REF 0\n}\n'''
        rows, counts = inspect_material_faces(sample)
        self.assertEqual(counts, {0: 2})
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['wrapped'], 2)
        self.assertEqual(rows[0]['status'], 'MULTISUB_WRAP')
        self.assertEqual(rows[0]['range'], (3,4))

    def test_invalid_face_id_remains_visible(self):
        from inspect_ase_face_material_269 import inspect_material_faces
        data = b'*GEOMOBJECT {\n*MESH_FACE 0: A: 0 B: 1 C: 2 *MESH_MTLID -1\n*MATERIAL_REF 0\n}\n'
        rows, _ = inspect_material_faces(data)
        self.assertEqual(rows[0]['status'], 'INVALID')

    def test_source_client_and_bones_unchanged(self):
        server = SOURCE.read_text(encoding='latin1')
        self.assertIn('ReadASE_GEOMOBJECT(fp, obj, 0, true);', server)
        self.assertIn('geometry_face_material', server)
        self.assertIn('geometry_partial_uv', server)
        self.assertIn('return failImportWith("geometry_face_material")', server)
        self.assertIn('#include "ase/FaceMaterialPolicy269.h"', server)


if __name__ == '__main__':
    unittest.main(verbosity=2)
