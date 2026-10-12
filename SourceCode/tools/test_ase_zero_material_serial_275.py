#!/usr/bin/env python3
"""PATCH 275: exercise the exact zero-material group cleanup and native SMD layout.

No client assets, Wine, network, mutable project content, or logs needed.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
ASE = ROOT / 'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
OBJ = ROOT / 'Shared/rendering/legacy_3d/object/smObj3d_legacy_server.inl'
COOK = ROOT / 'SrcServer/src/systems/rendering/legacy_3d/NativeModelCook259.h'
MARKER = '// PATCH 275: with MATERIAL_COUNT 0 the native SMD writer uses'


def actual_cleanup():
    source = ASE.read_text(encoding='latin1')
    start = source.index('if (offlineEmptyRegistry276) {', source.index(MARKER))
    end = source.index('} else {', start) + 1
    return source[start:end]


class EmptySerial275(unittest.TestCase):
    def compile_run(self, snippet):
        compiler = shutil.which('g++') or shutil.which('clang++')
        self.assertIsNotNone(compiler, 'host C++ compiler required')
        with tempfile.TemporaryDirectory() as temp:
            src = Path(temp) / 'main.cpp'
            exe = Path(temp) / 'main'
            src.write_text(snippet)
            build = subprocess.run([compiler, '-std=c++17', '-O1', '-Wall', '-Wextra',
                                    '-Werror', str(src), '-o', str(exe)],
                                   capture_output=True, text=True, timeout=20)
            self.assertEqual(build.returncode, 0, build.stderr)
            done = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
            self.assertEqual(done.returncode, 0, done.stderr)

    def test_real_cleanup_cpp_and_native_layout(self):
        # The block is extracted from the actual production parser, not retyped
        # into the test. Simulate the documented writer/reader disagreement and
        # show it disappears with the material-group deletion.
        cleanup = actual_cleanup()
        self.compile_run(r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
struct smMATERIAL_GROUP {
    static int alive;
    void* smMaterial = nullptr;
    smMATERIAL_GROUP() { ++alive; }
    ~smMATERIAL_GROUP() { --alive; }
};
int smMATERIAL_GROUP::alive = 0;
static void add(std::vector<unsigned char>& bytes, uint32_t value) {
    for (int i=0;i<4;i++) bytes.push_back((value >> (8*i)) & 255);
}
static uint32_t get(const std::vector<unsigned char>& bytes, size_t offset) {
    assert(offset+4<=bytes.size());
    uint32_t x=0;
    for(int i=0;i<4;i++) x |= (uint32_t)bytes[offset+i] << (8*i);
    return x;
}
static std::vector<unsigned char> save(const smMATERIAL_GROUP* group) {
    std::vector<unsigned char> data;
    add(data, 0); // the real writer uses MatCounter=MaterialCount=0
    if (group) { // the native SaveFile writes the group, even if empty!
        add(data, 0xFFFFFFFF); // opaque material-group header
        add(data, 0xFFFFFFFF);
    }
    add(data, 0x41424344); // first smOBJ3D Head
    add(data, 3); // first smOBJ3D nVertex
    return data;
}
static bool read(const std::vector<unsigned char>& data) {
    const uint32_t matCount=get(data,0);
    size_t offset=4;
    if(matCount) offset+=8; // native reader skips group if MatCounter=0
    return get(data,offset)==0x41424344 && get(data,offset+4)==3;
}
static void run(bool offline, bool explicitEmpty, int count) {
    bool g_SummitOfflineEmptyMaterials274=offline;
    bool g_SummitASEEmptyMaterialTable274=explicitEmpty;
    int aseMaterialCnt=count;
    const bool offlineEmptyRegistry276 =
      g_SummitOfflineEmptyMaterials274 && g_SummitASEEmptyMaterialTable274 && aseMaterialCnt==0;
    struct PAT { smMATERIAL_GROUP* smMaterialGroup=nullptr; } pattern;
    PAT* pat=&pattern;
    void* smMatPattern=reinterpret_cast<void*>(1);
    smMATERIAL_GROUP* lpMaterialGroup=new smMATERIAL_GROUP;
    auto original=save(lpMaterialGroup);
    assert(!read(original)); // reproduces the layout mismatch
''' + cleanup + r'''
    if(offline && explicitEmpty && count==0) {
        assert(lpMaterialGroup==nullptr);
        assert(pat->smMaterialGroup==nullptr && smMatPattern==nullptr);
        assert(read(save(lpMaterialGroup)));
    } else {
        assert(lpMaterialGroup!=nullptr);
        // This patch does not modify non-offline or nonempty groups.
        assert(!read(save(lpMaterialGroup)));
    }
    delete lpMaterialGroup;
    assert(smMATERIAL_GROUP::alive==0);
}
int main() {
    run(true,true,0);
    run(false,true,0);
    run(true,false,0);
    run(true,true,1);
}
''')

    def test_native_writer_reader_contract_and_guard_scope(self):
        obj = OBJ.read_text(encoding='latin1')
        writer = obj.split('int\tsmPAT3D::SaveFile(char *szFile)', 1)[1].split('int\tsmPAT3D::LoadFile', 1)[0]
        reader = obj.split('int\tsmPAT3D::LoadFile(char *szFile, char *szNodeName)', 1)[1]
        self.assertIn('FileHeader.MatCounter = smMaterialGroup->MaterialCount', writer)
        self.assertIn('if(smMaterialGroup) smMaterialGroup->SaveFile(hFile);', writer)
        self.assertIn('if(FileHeader.MatCounter)', reader)
        self.assertIn('smMaterialGroup->LoadFile(hFile);', reader)
        self.assertIn('obj->LoadFile(hFile, BipPat);', reader)
        mesh = ASE.read_text(encoding='latin1').split('smPAT3D *smASE_Read(char *file, char *szModelName)', 1)[1].split('smPAT3D *smASE_ReadBone(char *file)', 1)[0]
        self.assertEqual(mesh.count(MARKER),1)
        self.assertIn('delete lpMaterialGroup;', actual_cleanup())
        self.assertIn('lpMaterialGroup = NULL;', actual_cleanup())
        self.assertIn('g_SummitOfflineEmptyMaterials274 &&', ASE.read_text(encoding='latin1').split(MARKER,1)[1])
        self.assertIn('g_SummitASEEmptyMaterialTable274 && aseMaterialCnt == 0', ASE.read_text(encoding='latin1').split(MARKER,1)[1])
        self.assertLess(mesh.index(MARKER), mesh.index('pat->smMaterialGroup = lpMaterialGroup;'))
        self.assertIn('smASE_EnableOfflineEmptyMaterials274(true)', COOK.read_text())

    def test_untouched_client_and_cache_strategy(self):
        snippet=actual_cleanup()
        self.assertNotIn('cliente/',snippet)
        self.assertNotIn('Content/canonical',snippet)
        self.assertNotIn('smASE_ReadBone',snippet)
        self.assertIn('pat->SaveFile(szFile);', ASE.read_text(encoding='latin1'))


if __name__ == '__main__':
    unittest.main(verbosity=2)
