#!/usr/bin/env python3
"""PATCH 264: regression tests for actual legacy server ASE reader/token routines."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
READER = ROOT/'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'


class AseParserGuards264(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = READER.read_text()

    def test_actual_tokenizers_cpp(self):
        compiler = shutil.which('g++') or shutil.which('clang++')
        if compiler is None:
            self.skipTest('native C++ compiler not available')
        begin = self.source.index('char *GetWord(char *q, char *p)')
        end = self.source.index('char *SetDirectoryFromFile', begin)
        actual = self.source[begin:end]
        code = (r'''
#include <cassert>
#include <cstring>
#include <string>
''' + actual + r'''
int main() {
    char word[258]; std::memset(word, 'Z', sizeof(word));
    std::string longWord(500, 'Q'); longWord += " :END\n";
    char* next = GetWord(word + 1, longWord.data());
    assert(word[0]=='Z' && word[257]=='Z' && std::strlen(word+1)==255);
    assert(std::string(next)==" :END\n");
    char line[]=" \t:Model : Sample"; char buffer[256]{};
    assert(std::strcmp(GetWord(buffer,line)," : Sample")==0);
    assert(std::strcmp(buffer,"Model")==0);
    std::string longQuoted = " \"" + std::string(600, 'T') + "\" extra";
    std::memset(word,'Z',sizeof(word));
    next = GetString(word+1,longQuoted.data());
    assert(std::strlen(word+1)==255 && word[0]=='Z' && word[257]=='Z');
    assert(std::strcmp(next," extra")==0);
    char missing[]=" MISSING_QUOTE\n";
    next=GetString(buffer,missing);
    assert(buffer[0]==0 && *next=='\n');
    char ordinary[]=" \"some material.png\" X";
    next=GetString(buffer,ordinary);
    assert(std::strcmp(buffer,"some material.png")==0);
    assert(std::strcmp(next," X")==0);
}
''')
        with tempfile.TemporaryDirectory() as directory:
            cpp=Path(directory)/'reader.cpp'; exe=Path(directory)/'reader'
            cpp.write_text(code)
            cmd=[compiler,'-std=c++17','-Wall','-Wextra','-Werror','-O1',str(cpp),'-o',str(exe)]
            cp=subprocess.run(cmd,text=True,capture_output=True)
            self.assertEqual(cp.returncode,0,cp.stderr)
            run=subprocess.run([str(exe)],text=True,capture_output=True)
            self.assertEqual(run.returncode,0,run.stderr)

    def test_geometry_uses_guarded_materials_and_indices(self):
        section=self.source.split('FILE *ReadASE_GEOMOBJECT(',1)[1].split('void  smASE_SetPhysique',1)[0]
        self.assertIn('auto failImport = [&]() -> FILE*',section)
        self.assertIn('!smMatPattern',section)
        self.assertIn('MatrialRef >= aseMaterialCnt',section)
        self.assertIn('if (!lpTexVertex || TVertexCnt >= nTVertex)',section)
        self.assertIn('if (!lpTexFace || TFaceCnt >= nFace)',section)
        self.assertIn('FaceMatrial = new int[nFace]();',section)
        self.assertIn('if (TexCnt > 0 && (!tPoint || !tFace))',section)
        self.assertIn('if (TmRotCnt >= 4095)',section)
        self.assertIn('if (TmPosCnt >= 4095)',section)
        self.assertIn('if (TmScaleCnt >= 4095)',section)
        for name in ['FaceMatrial','tPoint','tFace','tLightMapPoint','tLightMapFace']:
            self.assertIn('delete[] '+name+';',section)
            self.assertNotRegex(section, r'\bdelete\s+'+name+r'\s*;')
        self.assertEqual(section.count('fclose(fp);'),1)

    def test_mesh_and_bone_readers_reject_failed_subreaders(self):
        mesh = self.source.split('smPAT3D *smASE_Read(',1)[1].split('smPAT3D *smASE_ReadBone(',1)[0]
        bone = self.source.split('smPAT3D *smASE_ReadBone(',1)[1].split('smPAT3D* smASE_TalkReadBone(',1)[0]
        self.assertIn('aseMaterialCnt = 0;',mesh)
        self.assertIn('smMatPattern = NULL;',mesh)
        # PATCH 268: skeleton imports explicitly bypass ASE material lookup;
        # both variants still fail closed when GEOMOBJECT rejects a node.
        for frag, call in [(mesh, r'ReadASE_GEOMOBJECT\(fp, obj\)'),
                           (bone, r'ReadASE_GEOMOBJECT\(fp, obj, 0, true\)')]:
            self.assertRegex(frag, r'fp = '+call+r';\s*if \(!fp\)')
            self.assertIn('delete obj;',frag)
            self.assertIn('delete pat;',frag)

    def test_material_parser_handles_missing_metadata(self):
        material=self.source.split('FILE* ReadASE_LIGHTMAP_MATERIAL(',1)[1].split('smPAT3D *smPhysique',1)[0]
        self.assertIn('int smCount = 0;',material)
        self.assertIn('int NewMatrial = 0;',material)
        self.assertIn('if (s_LightMapRefMax <= 0)',material)
        self.assertIn('if (BitmapCnt >= 7)',material)
        self.assertIn('if (BitmapCnt < 0 || BitmapCnt >= 8)',material)
        self.assertIn('MATRIAL_MAX * 2 - smCount',material)

if __name__=='__main__':
    unittest.main(verbosity=2)
