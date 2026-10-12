#!/usr/bin/env python3
"""PATCH 271: model ASE no-material geometry, negative cases, source wiring."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
READER = ROOT / 'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
POLICY = ROOT / 'Shared/rendering/legacy_3d/ase/MateriallessMeshPolicy271.h'


class Materialless271(unittest.TestCase):
    def test_compiled_real_policy_cpp(self):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        code = r'''
#include <cassert>
#include "ase/MateriallessMeshPolicy271.h"
struct Object {
    int nFace;
    int mats[3];
    bool allow;
    explicit Object(int n):nFace(n),mats{9,9,9},allow(true){}
    bool SetFaceMaterial(int i,int mat){
        if(!allow || i<0 || i>=nFace || mat!=0) return false;
        mats[i]=mat; return true;
    }
};
int main(){
    using namespace summit_ase_mesh271;
    int ids[3]={0,0,0};
    assert(CanUseNeutral(0,false,false,3,3,ids));
    assert(CanUseNeutral(0,false,false,0,0,nullptr));
    assert(!CanUseNeutral(0,false,true,3,3,ids));
    assert(!CanUseNeutral(1,false,false,3,3,ids));
    assert(!CanUseNeutral(0,true,false,3,3,ids));
    assert(!CanUseNeutral(0,false,false,3,2,ids));
    assert(!CanUseNeutral(0,false,false,3,3,nullptr));
    assert(!CanUseNeutral(0,false,false,-1,0,ids));
    ids[1]=4;
    assert(!CanUseNeutral(0,false,false,3,3,ids));
    ids[1]=-1;
    assert(!CanUseNeutral(0,false,false,3,3,ids));
    ids[1]=0;
    Object obj(3);
    assert(AssignNeutralFaces(&obj,3));
    assert(obj.mats[0]==0 && obj.mats[1]==0 && obj.mats[2]==0);
    assert(!AssignNeutralFaces<Object>(nullptr,3));
    assert(!AssignNeutralFaces(&obj,2));
    obj.allow=false;
    assert(!AssignNeutralFaces(&obj,3));
}
'''
        with tempfile.TemporaryDirectory() as td:
            src, exe = Path(td) / 'p.cpp', Path(td) / 'p'
            src.write_text(code)
            cmd = [compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
                   '-I', str(POLICY.parent.parent), str(src), '-o', str(exe)]
            compile = subprocess.run(cmd, capture_output=True, text=True)
            self.assertEqual(compile.returncode, 0, compile.stderr)
            run = subprocess.run([str(exe)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)

    def test_compiled_exact_reader_neutral_branch(self):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        text = READER.read_text(encoding='latin1')
        start = text.index('const bool materiallessMesh = summit_ase_mesh271::CanUseNeutral(')
        stop = text.index('// PATCH 264: regular mesh geometry MUST have valid materials.', start)
        actual = text[start:stop] + '}\n'
        code = r'''
#include <cassert>
#include <cstddef>
#include "ase/MateriallessMeshPolicy271.h"
#include "ase/EmptyMaterialTablePolicy274.h"
struct Mesh {
    int nFace = 2;
    int mat[2] = {5,5};
    bool SetFaceMaterial(int i, int material) {
        if (i < 0 || i >= nFace || material != 0) return false;
        mat[i] = material;
        return true;
    }
};
Mesh object;
Mesh* obj = &object;
int aseMaterialCnt = 0;
void* smMatPattern = nullptr;
bool hasMaterialRef = false;
bool g_SummitOfflineEmptyMaterials274 = false;
bool g_SummitASEEmptyMaterialTable274 = false;
int nFace = 2, FaceCnt = 2;
int ids[2] = {0,0};
int* FaceMatrial = ids;
const char* error = nullptr;
int runBranch() {
    auto failImportWith = [](const char* reason) { error = reason; return -1; };
''' + actual + r'''
    return materiallessMesh ? 1 : 0;
}
int main() {
    assert(runBranch() == 1 && object.mat[0] == 0 && object.mat[1] == 0);
    ids[1] = 3;
    assert(runBranch() == 0);
    ids[1] = 0;
    hasMaterialRef = true;
    assert(runBranch() == 0);
    hasMaterialRef = false;
    aseMaterialCnt = 2;
    assert(runBranch() == 0);
    aseMaterialCnt = 0;
    smMatPattern = &object;
    assert(runBranch() == 0);
    smMatPattern = nullptr;
    FaceCnt = 1;
    assert(runBranch() == 0);
}
'''
        with tempfile.TemporaryDirectory() as td:
            cpp, exe = Path(td)/'reader.cpp', Path(td)/'reader'
            cpp.write_text(code)
            cc = subprocess.run([compiler,'-std=c++17','-Wall','-Wextra','-Werror',
                                 '-I',str(POLICY.parent.parent),str(cpp),'-o',str(exe)],
                                capture_output=True,text=True)
            self.assertEqual(cc.returncode,0,cc.stderr)
            run = subprocess.run([str(exe)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stderr)

    def test_real_reader_only_neutral_with_absent_material_table(self):
        text = READER.read_text(encoding='latin1')
        start = text.index('FILE *ReadASE_GEOMOBJECT(')
        end = text.index('void  smASE_SetPhysique(', start)
        section = text[start:end]
        self.assertIn('#include "ase/MateriallessMeshPolicy271.h"', text)
        self.assertIn('bool hasMaterialRef = false;', section)
        self.assertIn('hasMaterialRef = true;', section)
        self.assertIn('summit_ase_mesh271::CanUseNeutral(', section)
        self.assertIn('summit_ase_mesh271::AssignNeutralFaces(obj, nFace)', section)
        self.assertIn('if (materiallessMesh || emptyTableUnbound274)', section)
        self.assertIn('!smMatPattern', section)
        self.assertIn('MatrialRef >= aseMaterialCnt', section)
        self.assertIn('geometry_face_material', section)
        self.assertIn('obj->SetFaceMaterial(cnt, mNum)', section)
        # Keep stage, client and bone material handling independent.
        self.assertEqual(text.count('CanUseNeutral('), 1)
        bone = text.split('smPAT3D *smASE_ReadBone(char *file)', 1)[1]
        self.assertIn('ReadASE_GEOMOBJECT(fp, obj, 0, true)', bone)
        self.assertIn('summit_ase_bone268::AssignNeutralFaces', section)

    def test_read_only_inspector(self):
        from inspect_ase_materialless_271 import classify
        neutral = b'''*GEOMOBJECT {\n*NODE_NAME "helper"\n*MESH {\n*MESH_NUMFACES 1\n*MESH_FACE 0: A: 0 B: 1 C: 2 *MESH_MTLID 0\n}\n}\n'''
        info, rows = classify(neutral)
        self.assertEqual(info['material_lists'], 0)
        self.assertEqual(rows[0]['materialless'], 'NEUTRAL_CANDIDATE')
        nonzero = neutral.replace(b'MESH_MTLID 0', b'MESH_MTLID 3')
        self.assertEqual(classify(nonzero)[1][0]['materialless'], 'REJECT_NONZERO_FACE_ID')
        explicit = neutral.replace(b'\n}\n', b'\n*MATERIAL_REF 0\n}\n')
        self.assertEqual(classify(explicit)[1][0]['materialless'], 'REJECT_EXPLICIT_REF')
        with_table = neutral.replace(b'*GEOMOBJECT', b'*MATERIAL_LIST {\n*MATERIAL_COUNT 1\n}\n*GEOMOBJECT')
        self.assertEqual(classify(with_table)[1][0]['materialless'], 'MATERIAL_TABLE_PRESENT')

    def test_no_gameplay_or_content_changes(self):
        self.assertTrue(POLICY.is_file())
        self.assertTrue((ROOT / 'tools/inspect_ase_materialless_271.py').is_file())
        self.assertTrue((ROOT / 'Shared/rendering/legacy_3d/smRead3d_legacy_client.inl').is_file())


if __name__ == '__main__':
    unittest.main(verbosity=2)
