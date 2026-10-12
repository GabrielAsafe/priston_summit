#!/usr/bin/env python3
"""PATCH 273: compile actual header and test offline-only call wiring and read-only ASE inventory."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from inspect_ase_bone_hierarchy_273 import bone_hierarchy

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT/'Shared/rendering/legacy_3d/ase/OfflineBoneHierarchy273.h'
READER = ROOT/'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
COOKER = ROOT/'SrcServer/src/systems/rendering/legacy_3d/NativeModelCook259.h'


class Tests273(unittest.TestCase):
    def test_compiled_actual_linker_and_non_mutating_failure(self):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        program = r'''
#include <cassert>
#include <cstring>
#include "ase/OfflineBoneHierarchy273.h"
struct Obj { char NodeName[32]; char NodeParent[32]; Obj* pParent;
 Obj(const char* n, const char* p="") : pParent(nullptr) {
   std::strcpy(NodeName,n); std::strcpy(NodeParent,p);
 }
};
struct Pat { int nObj3d; Obj* obj3d[129]; int TmSort[128];
 Pat() : nObj3d(0) { for(int &v: TmSort) v=77; }
 void add(Obj* o){ obj3d[nObj3d++]=o; }
};
int main(){
 Obj a("BipRoot"), b("BipChild","BipRoot"), c("BipGrand","BipChild"),
     d("BipSide","BIPROOT");
 Pat normal; normal.add(&c); normal.add(&d); normal.add(&a); normal.add(&b);
 assert(summit_ase_bone273::LinkAndSort(&normal));
 assert(normal.TmSort[0]==2 && normal.TmSort[1]==1 && normal.TmSort[2]==3 && normal.TmSort[3]==0);
 assert(c.pParent==&b && b.pParent==&a && d.pParent==&a);
 Obj missing("BipNoParent","unexported bone"); Pat orphan; orphan.add(&missing);
 assert(summit_ase_bone273::LinkAndSort(&orphan));
 assert(missing.pParent==nullptr && orphan.TmSort[0]==0);
 Obj x("BipX","BipY"),y("BipY","BipX"); Pat cycle; cycle.add(&x);cycle.add(&y);
 assert(!summit_ase_bone273::LinkAndSort(&cycle));
 assert(cycle.TmSort[0]==77 && cycle.TmSort[1]==77 && x.pParent==nullptr);
 Obj self("BipSelf","BipSelf"); Pat selfPattern; selfPattern.add(&self);
 assert(!summit_ase_bone273::LinkAndSort(&selfPattern));
 Pat over; over.nObj3d=129; assert(!summit_ase_bone273::LinkAndSort(&over));
 assert(!summit_ase_bone273::LinkAndSort<Pat>(nullptr));
}
'''
        with tempfile.TemporaryDirectory() as td:
            source = Path(td)/'test.cpp'; exe = Path(td)/'test'
            source.write_text(program)
            compile_ = subprocess.run([compiler, '-std=c++17','-Wall','-Wextra','-Werror',
                                       '-I', str(HEADER.parent.parent), str(source), '-o',str(exe)],
                                      capture_output=True,text=True)
            self.assertEqual(compile_.returncode,0,compile_.stdout+compile_.stderr)
            run = subprocess.run([str(exe)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)

    def test_offline_only_switch_and_success_checks(self):
        reader = READER.read_text(encoding='latin1')
        header = COOKER.read_text()
        section=reader.split('smPAT3D *smASE_ReadBone(char *file)',1)[1].split('smPAT3D* smASE_TalkReadBone(',1)[0]
        mesh=reader.split('smPAT3D *smASE_Read(char *file',1)[1].split('smPAT3D *smASE_ReadBone(char *file)',1)[0]
        self.assertIn('g_SummitOfflineBoneHierarchy273 = false', reader)
        self.assertIn('summit_ase_bone273::LinkAndSort(pat)',section)
        self.assertIn('bone_hierarchy_invalid',section)
        self.assertIn('bone_object_limit',section)
        self.assertIn('bone_save_failed',section)
        self.assertIn('if (!g_SummitOfflineBoneHierarchy273) pat->SaveFile(szFile);',section)
        self.assertIn('OfflineBoneScope273',header)
        self.assertIn('smASE_EnableOfflineBoneHierarchy273(true)',header)
        self.assertIn('smASE_EnableOfflineBoneHierarchy273(false)',header)
        self.assertNotIn('LinkAndSort(pat)',mesh)

    def test_inspector_valid_and_cycles(self):
        content=b'''*GEOMOBJECT {\n *NODE_NAME "BipRoot"\n}\n*GEOMOBJECT {\n *NODE_NAME "BipChild"\n *NODE_PARENT "BipRoot"\n}\n'''
        valid=bone_hierarchy(content)
        self.assertEqual((valid['geometry'],valid['bones'],valid['roots'],valid['problems']), (2,2,1,[]))
        content=b'''*GEOMOBJECT {\n *NODE_NAME "BipA"\n *NODE_PARENT "BipB"\n}\n*GEOMOBJECT {\n *NODE_NAME "BipB"\n *NODE_PARENT "BipA"\n}\n'''
        invalid=bone_hierarchy(content)
        self.assertIn('bone_hierarchy_invalid', invalid['problems'])
        content=b'''*GEOMOBJECT {\n *NODE_NAME "BipA"\n *NODE_PARENT "Missing"\n}\n'''
        orphan=bone_hierarchy(content)
        self.assertFalse(orphan['problems'])
        self.assertEqual(len(orphan['missing']),1)

    def test_inspector_respects_non_bip_and_128(self):
        objects=[f'*GEOMOBJECT {{\n *NODE_NAME "Bip{i}"\n}}\n'.encode() for i in range(129)]
        too_many=bone_hierarchy(b''.join(objects))
        self.assertIn('bone_object_limit',too_many['problems'])
        one=bone_hierarchy(b'*GEOMOBJECT {\n *NODE_NAME "foo"\n}\n')
        self.assertIn('bone_none',one['problems'])


if __name__ == '__main__':
    unittest.main(verbosity=2)
