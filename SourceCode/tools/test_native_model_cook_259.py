#!/usr/bin/env python3
"""Offline native cook C++ path gates and fail-closed staging tests."""
from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import hashlib

SRC = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SRC / 'tools'))
from cook_native_models_259 import prepare, FORMAT

class NativeCookTests259(unittest.TestCase):
    def test_cpp_path_validator_real_header(self):
        header = SRC / 'SrcServer/src/systems/rendering/legacy_3d/ModelCookPath259.h'
        with tempfile.TemporaryDirectory() as d:
            cpp = Path(d) / 'main.cpp'
            cpp.write_text('''#include "ModelCookPath259.h"
int main() {
using summit_model_cook_259::SafeRelativeASE;
if(!SafeRelativeASE("Effect/NewEffect/Res/Object/At_4-3a.ASE")) return 1;
if(!SafeRelativeASE("char/tmABCD/MmbA16.ASE")) return 2;
const char* bad[] = {"", "/a.ASE", "../a.ASE", "a/../b.ASE", "C:/a.ASE", "a\\\\b.ASE", "a//b.ASE", "x.smd", "a/./b.ASE", "a;a.ASE", "long/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.ASE"};
for(const char* p:bad) if(SafeRelativeASE(p)) return 3;
return 0;
}
''')
            exe = Path(d) / 'test'
            subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I', str(header.parent), str(cpp), '-o', str(exe)], check=True, capture_output=True, text=True)
            subprocess.run([str(exe)], check=True)

    def fixture(self, tmp, native_exit=0):
        p = Path(tmp)
        client = p / 'cliente'; client.mkdir()
        (client / 'Game.exe').write_bytes(b'client')
        rel = 'Effect/Test.ASE'
        ase = client / rel
        ase.parent.mkdir()
        ase.write_text('*GEOMOBJECT {\n *NODE_NAME "NonBip"\n}\n')
        serverdir = p / 'Servidor'; serverdir.mkdir()
        server = serverdir / 'Server.exe'; server.write_bytes(b'server')
        census = p / 'census.json'; roles = p / 'roles.json'
        census.write_text(json.dumps({'format':'SUMMIT_MODEL_CACHE_CENSUS_245_V1',
          'counts':{'sources_ase':1}, 'entries':[{'source':rel, 'source_sha256':hashlib.sha256(ase.read_bytes()).hexdigest(),
            'cache':{'smd':{'status':'missing','path':'Effect/Test.smd'},'smb':{'status':'missing','path':'Effect/Test.smb'}}}]}))
        sys.path.insert(0, str(SRC / 'tools'))
        from audit_model_roles_246 import audit
        audit(client, census, roles)
        runner = p / 'fakewine'
        runner.write_text('''#!/usr/bin/env python3
import pathlib, sys
if len(sys.argv)!=5:sys.exit(7)
role,relative=sys.argv[3:]
p=pathlib.Path(relative)
if sys.argv[2]=='--summit-verify-model' and sys.argv[1].endswith('Game.exe'):
  if not p.is_file() or not p.read_bytes().startswith(b'SMD Model data Ver 0.62'):sys.exit(8)
  print('MODEL_VERIFY_261=PASS')
  sys.exit(42)
if sys.argv[2]!='--summit-cook-model-one':sys.exit(9)
if %d:sys.exit(1)
p.parent.mkdir(exist_ok=True,parents=True)
p.with_suffix('.smd' if role=='mesh' else '.smb').write_bytes(b'SMD Model data Ver 0.62\\0'+bytes(256))
print('MODEL_COOK_259=PASS')
''' % native_exit)
        runner.chmod(0o755)
        return client, server, census, roles, runner

    def test_publish_only_separate_output(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.fixture(d)
            before = sorted(x.as_posix() for x in a[0].rglob('*'))
            out = Path(d) / 'artifacts'
            report = prepare(*a[:4], out, str(a[4]))
            self.assertEqual(report['count'], 1)
            self.assertEqual(report['format'], FORMAT)
            self.assertTrue(report['artifacts'][0]['client_readback'])
            self.assertTrue((out/'Effect/Test.smd').is_file())
            self.assertFalse((a[0]/'Effect/Test.smd').exists())
            self.assertEqual(before, sorted(x.as_posix() for x in a[0].rglob('*')))
            with self.assertRaisesRegex(ValueError, 'must not exist'):
                prepare(*a[:4], out, str(a[4]))

    def test_pilot_only_and_invalid_source(self):
        with tempfile.TemporaryDirectory() as d:
            a=self.fixture(d)
            with self.assertRaisesRegex(ValueError, 'pilot source not found'):
                prepare(*a[:4], Path(d)/'nothing', str(a[4]), 'not-an-asset.ASE')
            self.assertFalse((Path(d)/'nothing').exists())
            report=prepare(*a[:4],Path(d)/'pilot',str(a[4]),'Effect/Test.ASE')
            self.assertEqual(report['scope'],'pilot_only')
            self.assertEqual(report['count'],1)

    def test_subprocess_failure_refuses_publish(self):
        with tempfile.TemporaryDirectory() as d:
            a=self.fixture(d, native_exit=1)
            out=Path(d)/'artifacts'
            with self.assertRaisesRegex(ValueError, 'native conversion failed'):
                prepare(*a[:4], out, str(a[4]))
            self.assertFalse(out.exists())

    def test_source_change_refused(self):
        with tempfile.TemporaryDirectory() as d:
            a=self.fixture(d)
            (a[0]/'Effect/Test.ASE').write_text('changed')
            with self.assertRaisesRegex(ValueError, 'source changed'):
                prepare(*a[:4], Path(d)/'artifacts', str(a[4]))

    def test_no_client_output_or_symlink(self):
        with tempfile.TemporaryDirectory() as d:
            a=self.fixture(d)
            with self.assertRaisesRegex(ValueError, 'outside client'):
                prepare(*a[:4], a[0]/'cooked', str(a[4]))
            target=Path(d)/'outside'; target.mkdir()
            (Path(d)/'redirect').symlink_to(target, target_is_directory=True)
            with self.assertRaisesRegex(ValueError, 'symlink'):
                prepare(*a[:4], Path(d)/'redirect'/'cooked', str(a[4]))

    def test_winmain_early_branch_before_sqlite(self):
        win=(SRC/'SrcServer/src/platform/windows/application/Winmain.cpp').read_text(errors='replace')
        self.assertIn('summit_model_cook_259::RunOne(argv, argc)',win)
        self.assertLess(win.index('summit_model_cook_259::RunOne(argv, argc)'),win.index('openDatabase();'))
        native=(SRC/'SrcServer/src/systems/rendering/legacy_3d/NativeModelCook259.h').read_text()
        self.assertIn('smASE_ReadBone(args[3])',native)
        self.assertIn('smReadModel_Bip(cache)',native)
        self.assertIn('g_IsReadTextures = 0',native)
        self.assertIn('native_readback', (SRC/'tools/cook_native_models_259.py').read_text())

if __name__ == '__main__':
    unittest.main(verbosity=2)
