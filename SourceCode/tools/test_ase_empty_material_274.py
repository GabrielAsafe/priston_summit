#!/usr/bin/env python3
"""PATCH 274: zero-material ASE face slots, actual reader branch and safety."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
READER = ROOT/'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
HEADER = ROOT/'Shared/rendering/legacy_3d/ase/EmptyMaterialTablePolicy274.h'
COOKER = ROOT/'SrcServer/src/systems/rendering/legacy_3d/NativeModelCook259.h'


class EmptyMaterialTable274(unittest.TestCase):
    def compile_run(self, source):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as d:
            p = Path(d)
            (p/'test.cpp').write_text(source)
            command = [compiler,'-std=c++17','-Wall','-Wextra','-Werror','-O1',
                       '-I',str(HEADER.parent.parent),str(p/'test.cpp'),'-o',str(p/'test')]
            compiled = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            ran = subprocess.run([str(p/'test')], capture_output=True, text=True)
            self.assertEqual(ran.returncode, 0, ran.stderr)

    def test_cpp_strict_policy(self):
        self.compile_run(r'''#include <cassert>
#include "ase/EmptyMaterialTablePolicy274.h"
int main() {
    using summit_ase_mesh274::CanNormalizeUnbound;
    int slots[4]={0,5,1,65535};
    assert(CanNormalizeUnbound(true,true,0,false,4,4,slots));
    assert(!CanNormalizeUnbound(false,true,0,false,4,4,slots));
    assert(!CanNormalizeUnbound(true,false,0,false,4,4,slots));
    assert(!CanNormalizeUnbound(true,true,1,false,4,4,slots));
    assert(!CanNormalizeUnbound(true,true,0,true,4,4,slots));
    assert(!CanNormalizeUnbound(true,true,0,false,4,3,slots));
    assert(!CanNormalizeUnbound(true,true,0,false,-1,0,slots));
    assert(!CanNormalizeUnbound(true,true,0,false,4,4,nullptr));
    assert(CanNormalizeUnbound(true,true,0,false,0,0,nullptr));
    slots[1]=-1;
    assert(!CanNormalizeUnbound(true,true,0,false,4,4,slots));
    slots[1]=65536;
    assert(!CanNormalizeUnbound(true,true,0,false,4,4,slots));
}
''')

    def test_cpp_exact_reader_branch(self):
        text=READER.read_text(encoding='latin1')
        begin=text.index('const bool emptyTableUnbound274 = summit_ase_mesh274::CanNormalizeUnbound(')
        end=text.index('// PATCH 264: regular mesh geometry MUST have valid materials.',begin)
        actual=text[begin:end]+'}\n'
        self.compile_run(r'''#include <cassert>
#include "ase/EmptyMaterialTablePolicy274.h"
#include "ase/MateriallessMeshPolicy271.h"
struct Object {
  int nFace = 2;
  int mat[2]={9,9};
  bool SetFaceMaterial(int i, int material) {
    if(i<0 || i>=2 || material!=0) return false;
    mat[i]=material; return true;
  }
};
Object object;
Object* obj=&object;
bool materiallessMesh=false;
bool g_SummitOfflineEmptyMaterials274=true;
bool g_SummitASEEmptyMaterialTable274=true;
int aseMaterialCnt=0;
bool hasMaterialRef=false;
int nFace=2,FaceCnt=2;
int ids[]={5,2};
int* FaceMatrial=ids;
const char* failure=nullptr;
int run() {
  auto failImportWith=[](const char* reason){failure=reason;return -1;};
''' + actual + r'''
  return emptyTableUnbound274 ? 1:0;
}
int main() {
  assert(run()==1 && object.mat[0]==0 && object.mat[1]==0);
  g_SummitOfflineEmptyMaterials274=false;
  assert(run()==0);
  g_SummitOfflineEmptyMaterials274=true;
  hasMaterialRef=true;
  assert(run()==0);
  hasMaterialRef=false;
  aseMaterialCnt=1;
  assert(run()==0);
  aseMaterialCnt=0;
  ids[1]=-1;
  assert(run()==0);
  ids[1]=2;
  g_SummitASEEmptyMaterialTable274=false;
  assert(run()==0);
}
''')

    def test_exact_material_parser_flags_and_offline_only(self):
        text=READER.read_text(encoding='latin1')
        cook=COOKER.read_text()
        material=text.split('FILE* ReadASE_LIGHTMAP_MATERIAL(',1)[1].split('smPAT3D *smPhysique',1)[0]
        mesh=text.split('smPAT3D *smASE_Read(char *file, char *szModelName)',1)[1].split('smPAT3D *smASE_ReadBone(',1)[0]
        self.assertIn('if (emptyTable274) *emptyTable274 = false;',material)
        self.assertIn('mCount == 0 && !sawMaterialDefinitions274',material)
        self.assertIn('aseMaterialCnt == 0 && s_LightMapRefMax == 0',material)
        self.assertIn('ReadASE_LIGHTMAP_MATERIAL(fp, &lpMaterialGroup,',mesh)
        self.assertIn('&g_SummitASEEmptyMaterialTable274',mesh)
        self.assertIn('g_SummitASEEmptyMaterialTable274 = false;',mesh)
        self.assertIn('if (materiallessMesh || emptyTableUnbound274)',text)
        self.assertIn('smASE_EnableOfflineEmptyMaterials274(true)',cook)
        self.assertIn('smASE_EnableOfflineEmptyMaterials274(false)',cook)
        self.assertIn('if (boneOnly)',text)
        self.assertNotIn('SourceCode/Content/canonical',cook)

    def test_read_only_inspector(self):
        from inspect_ase_empty_material_274 import classify
        data = b'''*MATERIAL_LIST {
*MATERIAL_COUNT 0
}
*GEOMOBJECT {
*NODE_NAME "test01"
*MESH {
*MESH_NUMFACES 2
*MESH_FACE 0: A: 0 B: 1 C: 2 *MESH_MTLID 1
*MESH_FACE 1: A: 0 B: 1 C: 2 *MESH_MTLID 5
}
}
'''
        info,rows=classify(data)
        self.assertEqual(info['material_count'],0)
        self.assertEqual(rows[0]['empty_table_274'],'EMPTY_TABLE_CANDIDATE')
        extra=data.replace(b'*NODE_NAME "test01"',b'*NODE_NAME "test01"\n*MATERIAL_REF 0')
        # Use literal newline in the ASE input, not an escaped slash-n.
        extra=extra.replace(b'\\n',b'\n')
        self.assertEqual(classify(extra)[1][0]['empty_table_274'],'EXPLICIT_MATERIAL_REF')
        nonzero=data.replace(b'*MATERIAL_COUNT 0',b'*MATERIAL_COUNT 1')
        self.assertEqual(classify(nonzero)[1][0]['empty_table_274'],'NOT_EXPLICIT_EMPTY_TABLE')

    def test_staging_and_no_other_assets(self):
        self.assertTrue(HEADER.is_file())
        self.assertTrue((ROOT/'tools/inspect_ase_empty_material_274.py').is_file())
        self.assertTrue((ROOT/'tools/cook_native_models_incremental_263.py').is_file())


if __name__=='__main__':
    unittest.main(verbosity=2)
