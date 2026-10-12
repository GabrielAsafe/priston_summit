#!/usr/bin/env python3
"""Test that explicit offline failures identify guarded parser stage, no persistent logs."""
import pathlib, re, subprocess, tempfile, unittest, sys
ROOT = pathlib.Path(__file__).resolve().parents[1]
SERVER = ROOT / 'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl'
HEADER = ROOT / 'SrcServer/src/systems/rendering/legacy_3d/NativeModelCook259.h'
WINMAIN = ROOT / 'SrcServer/src/platform/windows/application/Winmain.cpp'

class Tests(unittest.TestCase):
    def test_offline_mode_only_and_reason_markers(self):
        inl=SERVER.read_text(encoding='latin1')
        header=HEADER.read_text()
        main=WINMAIN.read_text(encoding='latin1')
        self.assertIn('const char* smASEFailureReason265()',inl)
        self.assertIn('#ifdef SUMMIT_NATIVE_ASE_REASON_265',header)
        self.assertIn('#define SUMMIT_NATIVE_ASE_REASON_265 1',main)
        self.assertIn('reason=%s',header)
        self.assertIn('MODEL_COOK_259=FAIL ase_parse\\n',header)
        for status in ['material_unterminated','material_sub_without_name',
                       'geometry_missing_uv','geometry_material_reference',
                       'geometry_face_material','geometry_unterminated']:
            self.assertIn('"'+status+'"',inl)
        self.assertNotIn('fopen("log',inl.lower())
        self.assertNotIn('SUMMIT_ASSET_TRACE',header)

    def test_native_header_reason_works_compiled(self):
        # Reuse the mock with real native header from PATCH 262, activating
        # the failure-reason path explicitly (normal stubs remain unchanged).
        prior=(ROOT/'tools/test_offline_model_material_262.py').read_text()
        cpp=prior.split('cpp = r"""',1)[1].split('"""',1)[0]
        cpp=cpp.replace('#include "NativeModelCook259.h"',
            '#define SUMMIT_NATIVE_ASE_REASON_265 1\n'
            'const char* smASEFailureReason265(){return "geometry_missing_uv";}\n'
            '#include "NativeModelCook259.h"')
        with tempfile.TemporaryDirectory() as td:
            path=pathlib.Path(td)/'test.cpp';binary=pathlib.Path(td)/'test'
            path.write_text(cpp)
            result=subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',
                '-I',str(HEADER.parent),'-I',str(ROOT/'SrcServer/src'),
                str(path),'-o',str(binary)],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            run=subprocess.run([str(binary)],capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stderr)
            self.assertIn('MODEL_COOK_259=FAIL ase_parse reason=geometry_missing_uv',run.stderr)

    def test_exact_reason_getter_compiles(self):
        source=SERVER.read_text(encoding='latin1')
        getter=re.search(r'static const char\* g_SummitASEFailure265.*?\nconst char\* smASEFailureReason265\(\)\s*\{[^}]+\}',source,re.S)
        self.assertIsNotNone(getter)
        code='#include <cstring>\n'+getter.group(0)+'''\nint main(){
 if(std::strcmp(smASEFailureReason265(),"none")!=0) return 1;
 g_SummitASEFailure265 = "material_sub_count";
 return std::strcmp(smASEFailureReason265(),"material_sub_count") != 0;
}\n'''
        with tempfile.TemporaryDirectory() as td:
            path=pathlib.Path(td)/'test.cpp';binary=pathlib.Path(td)/'test'
            path.write_text(code)
            c=subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',str(path),'-o',str(binary)],capture_output=True,text=True)
            self.assertEqual(c.returncode,0,c.stderr)
            self.assertEqual(subprocess.run([str(binary)]).returncode,0)

if __name__=='__main__':unittest.main(verbosity=2)
