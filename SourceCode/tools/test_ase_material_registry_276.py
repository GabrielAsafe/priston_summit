#!/usr/bin/env python3
"""PATCH 276: execute the actual MATERIAL_LIST -> smPAT3D registry branch.

Host C++ tests do not run the MinGW/Wine server or read user assets. They
exercise the extracted production code, including its ownership on failures.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
READER = ROOT / 'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
COOK = ROOT / 'SrcServer/src/systems/rendering/legacy_3d/NativeModelCook259.h'
OBJ = ROOT / 'Shared/rendering/legacy_3d/object/smObj3d_legacy_server.inl'
START = 'const bool offlineEmptyRegistry276 ='
END = '\n\t\t}\n\n'


def actual_branch():
    text = READER.read_text(encoding='latin1')
    begin = text.index(START)
    end = text.index(END, begin)
    branch = text[begin:end]
    assert branch.count('smMatPattern = NULL;') == 1
    assert branch.count('smMatPattern = lpMaterialGroup->smMaterial;') == 1
    return branch


class Registry276(unittest.TestCase):
    def compile_run(self, snippet):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler, 'host C++ compiler required')
        with tempfile.TemporaryDirectory() as td:
            source, output = Path(td) / 'registry.cpp', Path(td) / 'registry'
            source.write_text(snippet)
            built = subprocess.run([compiler, '-std=c++17', '-O1', '-g',
                                    '-Wall', '-Wextra', '-Werror',
                                    str(source), '-o', str(output)],
                                   text=True, capture_output=True, timeout=25)
            self.assertEqual(built.returncode, 0, built.stderr)
            run = subprocess.run([str(output)], text=True, capture_output=True, timeout=10)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)

    def test_real_branch_zero_nonzero_and_failclosed_cpp(self):
        branch = actual_branch()
        self.compile_run(r'''#include <cassert>
#include <cstdio>
#include <cstring>
struct smMATERIAL { int v; };
smMATERIAL activeMaterial{1};
struct smMATERIAL_GROUP {
    static int alive;
    smMATERIAL* smMaterial;
    explicit smMATERIAL_GROUP(bool hasMaterial):smMaterial(hasMaterial ? &activeMaterial : nullptr) { ++alive; }
    ~smMATERIAL_GROUP() { --alive; }
};
int smMATERIAL_GROUP::alive=0;
struct smPAT3D {
    static int alive;
    smMATERIAL_GROUP* smMaterialGroup=nullptr;
    smPAT3D() { ++alive; }
    ~smPAT3D() { delete smMaterialGroup; --alive; }
};
int smPAT3D::alive=0;
#define FALSE 0
int importRegistry(bool offline, bool emptyTable, int count,
                   bool hasGroup, bool hasMaterial, bool* neutral) {
    bool g_SummitOfflineEmptyMaterials274=offline;
    bool g_SummitASEEmptyMaterialTable274=emptyTable;
    int aseMaterialCnt=count;
    smMATERIAL_GROUP* lpMaterialGroup=hasGroup ? new smMATERIAL_GROUP(hasMaterial) : nullptr;
    smPAT3D* pat=new smPAT3D;
    smMATERIAL* smMatPattern=&activeMaterial; // guard against inherited globals
    const char* g_SummitASEFailure265="none";
    FILE* fp=std::tmpfile();
    assert(fp);
''' + branch + r'''
    *neutral=(pat->smMaterialGroup==nullptr && smMatPattern==nullptr);
    if (offline && emptyTable && count==0) {
        assert(*neutral);
        assert(lpMaterialGroup==nullptr);
    } else {
        assert(!*neutral);
        assert(lpMaterialGroup==pat->smMaterialGroup);
        assert(smMatPattern==&activeMaterial);
    }
    assert(std::strcmp(g_SummitASEFailure265,"none")==0);
    std::fclose(fp);
    delete pat;
    return 1;
}
int main() {
    bool neutral=false;
    assert(importRegistry(true,true,0,true,false,&neutral)==1 && neutral);
    assert(smMATERIAL_GROUP::alive==0 && smPAT3D::alive==0);
    // In the empty-material mode there may also be no temporary group.
    assert(importRegistry(true,true,0,false,false,&neutral)==1 && neutral);
    assert(importRegistry(false,true,0,true,true,&neutral)==1 && !neutral);
    assert(importRegistry(true,false,0,true,true,&neutral)==1 && !neutral);
    assert(importRegistry(true,true,1,true,true,&neutral)==1 && !neutral);
    // Truly invalid registries are rejected in every nonempty case.
    assert(importRegistry(false,false,1,false,false,&neutral)==0);
    assert(importRegistry(false,false,1,true,false,&neutral)==0);
    assert(importRegistry(true,true,1,true,false,&neutral)==0);
    assert(smMATERIAL_GROUP::alive==0 && smPAT3D::alive==0);
}
''')

    def test_writes_no_null_material_group_and_smd_header_agrees(self):
        source = READER.read_text(encoding='latin1')
        block = actual_branch()
        self.assertIn('pat->smMaterialGroup = NULL;', block)
        self.assertIn('smMatPattern = NULL;', block)
        self.assertIn('delete lpMaterialGroup;', block)
        self.assertIn('if (!lpMaterialGroup || !lpMaterialGroup->smMaterial)', block)
        self.assertIn('g_SummitASEFailure265 = "material_registry_empty";', block)
        self.assertNotIn('smMatPattern = pat->smMaterialGroup->smMaterial;', block)
        writer = OBJ.read_text(encoding='latin1').split('int\tsmPAT3D::SaveFile(char *szFile)', 1)[1]
        self.assertIn('if(smMaterialGroup)', writer)
        self.assertIn('FileHeader.MatCounter = 0;', writer)
        self.assertIn('if(smMaterialGroup) smMaterialGroup->SaveFile(hFile);', writer)
        reader = writer.split('int\tsmPAT3D::LoadFile(char *szFile, char *szNodeName)',1)[1]
        self.assertIn('if(FileHeader.MatCounter)', reader)
        self.assertIn('smMaterialGroup->LoadFile(hFile);', reader)
        self.assertIn('if (materiallessMesh || emptyTableUnbound274)', source)

    def test_strict_scope_and_reconvert(self):
        block = actual_branch()
        cook = COOK.read_text()
        self.assertIn('g_SummitOfflineEmptyMaterials274 &&', block)
        self.assertIn('g_SummitASEEmptyMaterialTable274 && aseMaterialCnt == 0', block)
        self.assertIn('smASE_EnableOfflineEmptyMaterials274(true)', cook)
        self.assertIn('smASE_EnableOfflineEmptyMaterials274(false)', cook)
        self.assertIn('mesh ? smASE_Read(args[3]) : smASE_ReadBone(args[3])', cook)
        self.assertIn('smPAT3D* verified = mesh ? smReadModel(cache, nullptr)', cook)
        self.assertNotIn('Content/canonical',block)
        self.assertNotIn('LOG(',block)

if __name__ == '__main__':
    unittest.main(verbosity=2)
